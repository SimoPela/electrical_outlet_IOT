/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file inmp441_w.c
 * @brief TDK InvenSense INMP441 I2S MEMS microphone — SPL estimate wrapper implementation.
 *
 * DSP pipeline per read:
 *   1. DMA-read @c INMP441_READ_SAMPLES 32-bit I2S words.
 *   2. Extract sign-extended 24-bit samples (valid bits in [31:8]).
 *   3. Normalise to [-1, 1] peak (full-scale = 2^23 - 1).
 *   4. Subtract DC mean, compute AC RMS.
 *   5. Map RMS to dB SPL via the INMP441 datasheet reference point
 *      (1 kHz, 94 dB SPL → peak code ~420426) plus a trim offset.
 */

#include "inmp441_w.h"
#include "i2s_init.h"

#include "driver/i2s_std.h"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>

static const char *TAG = "INMP441_W";

/** Number of 32-bit I2S words to read per call; must be a reasonable DMA multiple. */
#define INMP441_READ_SAMPLES 512

static int32_t s_rx_buf[INMP441_READ_SAMPLES];

/*
 * ESP-IDF + INMP441: each DMA sample is a 32-bit little-endian word with the 24-bit
 * I2S payload left-aligned (valid bits in 31..8). Taking the low 24 bits is wrong and
 * breaks SPL (see esp-idf #15721). Arithmetic >> 8 yields a sign-extended 24-bit value.
 */

/**
 * @brief Extract a sign-extended 24-bit sample from a 32-bit ESP-IDF I2S DMA word.
 *
 * @param raw  Raw 32-bit DMA word (payload in bits [31:8]).
 * @return     Signed 24-bit audio sample in the range [-8388608, 8388607].
 */
static int32_t sample_s24_from_i2s_word(int32_t raw)
{
    return raw >> 8;
}

/*
 * INMP441 (DS-INMP441-00): full-scale peak = 2^23 - 1. Scale to ~[-1, 1] peak.
 */

/**
 * @brief Normalise a raw 32-bit DMA word to a floating-point sample in [-1, 1].
 *
 * @param raw  Raw 32-bit DMA word.
 * @return     Normalised sample; full scale corresponds to ±1.0.
 */
static float sample_to_float(int32_t raw)
{
    return (float)sample_s24_from_i2s_word(raw) / 8388608.0f; /* 2^23 */
}

/** INMP441 full-scale peak count (2^23 - 1). */
#define INMP441_FS_PEAK          8388607.0f
/** RMS reference count at 94 dB SPL, 1 kHz (from datasheet). */
#define INMP441_PEAK_AT_94DB_SPL 420426.0f
#define INMP441_SQRT2            1.41421356f

/*
 * Additive trim [dB] applied after the datasheet formula.
 * Calibrate: offset = L_meter_reading - L_firmware_raw.
 * Bench measurement: ~40 dB(A) sound level meter, firmware raw ~55 dB → -15 dB initial estimate.
 */
#ifndef INMP441_SPL_OFFSET_DB
#define INMP441_SPL_OFFSET_DB (-10.0f)
#endif

/**
 * @brief Convert AC RMS (normalised) to estimated dB SPL.
 *
 * Uses the INMP441 datasheet reference point:
 *   SPL = 94 + 20*log10(rms / rms_ref_94)  + INMP441_SPL_OFFSET_DB
 *
 * Output is clamped to [30, 125] dB to avoid nonsensical display values.
 *
 * @param rms  AC RMS of the normalised audio block (0…1).
 * @return     Estimated SPL in dB; clamped to [30.0, 125.0].
 */
static float rms_to_spl_db_estimate(float rms)
{
    const float rms_ref_94 =
        (INMP441_PEAK_AT_94DB_SPL / INMP441_SQRT2) / INMP441_FS_PEAK;
    const float floor_rms = 1e-10f;
    float x = (rms < floor_rms) ? floor_rms : rms;
    float spl = 94.0f + 20.0f * log10f(x / rms_ref_94);
    spl += INMP441_SPL_OFFSET_DB;
    if (spl < 30.0f) {
        spl = 30.0f;
    }
    if (spl > 125.0f) {
        spl = 125.0f;
    }
    return spl;
}

/** @copydoc inmp441_w_init */
esp_err_t inmp441_w_init(void)
{
    /* Shared with sensor_init: idempotent. */
    return i2s_init_all();
}

/** @copydoc inmp441_w_read */
esp_err_t inmp441_w_read(inmp441_data_t *out)
{
    ESP_RETURN_ON_FALSE(out != NULL, ESP_ERR_INVALID_ARG, TAG, "out is NULL");

    i2s_chan_handle_t rx = i2s_get_rx_channel();
    ESP_RETURN_ON_FALSE(rx != NULL, ESP_ERR_INVALID_STATE, TAG, "I2S RX not initialized");

    size_t bytes_read = 0;
    esp_err_t err = i2s_channel_read(rx, s_rx_buf, sizeof(s_rx_buf), &bytes_read,
                                     pdMS_TO_TICKS(500));
    ESP_RETURN_ON_ERROR(err, TAG, "i2s_channel_read failed");

    size_t n = bytes_read / sizeof(int32_t);
    if (n == 0) {
        out->noise_db = 0.0f;
        return ESP_ERR_INVALID_SIZE;
    }

    static float s_f[INMP441_READ_SAMPLES];
    if (n > INMP441_READ_SAMPLES) {
        n = INMP441_READ_SAMPLES;
    }

    /* Compute mean for DC removal. */
    double sum = 0.0;
    for (size_t i = 0; i < n; i++) {
        s_f[i] = sample_to_float(s_rx_buf[i]);
        sum += (double)s_f[i];
    }

    const float mean = (float)(sum / (double)n);

    /* AC RMS (remove DC); a DC offset would otherwise read as ~100+ dB SPL. */
    double acc = 0.0;
    for (size_t i = 0; i < n; i++) {
        double d = (double)s_f[i] - (double)mean;
        acc += d * d;
    }

    float rms = sqrtf((float)(acc / (double)n));
    out->noise_db = rms_to_spl_db_estimate(rms);

    return ESP_OK;
}
