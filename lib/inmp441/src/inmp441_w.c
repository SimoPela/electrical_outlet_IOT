/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
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

/* Samples per read; must match I2S DMA expectations reasonably (multiple of frame). */
#define INMP441_READ_SAMPLES 512

static int32_t s_rx_buf[INMP441_READ_SAMPLES];

/*
 * ESP-IDF + INMP441: each DMA sample is a 32-bit little-endian word with the 24-bit
 * I2S payload left-aligned (valid bits in 31..8). Taking the low 24 bits is wrong and
 * breaks SPL (see esp-idf #15721). Arithmetic >> 8 yields a sign-extended 24-bit value.
 */
static int32_t sample_s24_from_i2s_word(int32_t raw)
{
    return raw >> 8;
}

/*
 * INMP441 (DS-INMP441-00): full-scale peak = 2^23 - 1. Scale to ~[-1, 1] peak.
 */
static float sample_to_float(int32_t raw)
{
    return (float)sample_s24_from_i2s_word(raw) / 8388608.0f; /* 2^23 */
}

/* Datasheet: 1 kHz, 94 dB SPL -> peak code ~420426; FS peak = 2^23 - 1. */
#define INMP441_FS_PEAK          8388607.0f
#define INMP441_PEAK_AT_94DB_SPL 420426.0f
#define INMP441_SQRT2            1.41421356f

/*
 * Additive trim (dB) after the DS formula. Calibrate: offset = L_meter - L_firmware_raw.
 * Bench: ~40 dB(A) sonomètre, firmware ~55 dB -> -15 dB.
 */
#ifndef INMP441_SPL_OFFSET_DB
#define INMP441_SPL_OFFSET_DB (-10.0f)
#endif

/*
 * SPL estimate (dB): base DS-INMP441 + offset utilisateur.
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

esp_err_t inmp441_w_init(void)
{
    /* Shared with sensor_init: idempotent. */
    return i2s_init_all();
}

esp_err_t inmp441_w_restore(void)
{
    i2s_chan_handle_t rx = i2s_get_rx_channel();
    if (rx != NULL) {
        ESP_LOGW(TAG, "restore L1: I2S RX disable / enable");
        esp_err_t err = i2s_channel_disable(rx);
        if (err == ESP_OK) {
            vTaskDelay(pdMS_TO_TICKS(10));
            err = i2s_channel_enable(rx);
            if (err == ESP_OK) {
                return ESP_OK;
            }
        }
        ESP_LOGW(TAG, "restore L1 failed, L2: i2s_restore (delete + rebuild)");
    }
    return i2s_restore();
}

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

    /* Float samples for this chunk */
    static float s_f[INMP441_READ_SAMPLES];
    if (n > INMP441_READ_SAMPLES) {
        n = INMP441_READ_SAMPLES;
    }

    double sum = 0.0;
    for (size_t i = 0; i < n; i++) {
        s_f[i] = sample_to_float(s_rx_buf[i]);
        sum += (double)s_f[i];
    }

    const float mean = (float)(sum / (double)n);

    /* AC RMS (remove DC); otherwise a DC offset reads as ~100+ dB SPL. */
    double acc = 0.0;
    for (size_t i = 0; i < n; i++) {
        double d = (double)s_f[i] - (double)mean;
        acc += d * d;
    }

    float rms = sqrtf((float)(acc / (double)n));
    out->noise_db = rms_to_spl_db_estimate(rms);

    return ESP_OK;
}
