/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file mics5524.c
 * @brief MiCS-5524 combustible gas sensor — ADC voltage and CO ppm estimate implementation.
 *
 * Voltage is obtained by averaging @c MICS_NUM_SAMPLES ADC readings through the
 * ESP-IDF oneshot driver.  The ppm estimate uses the empirical CO curve from the
 * MiCS-5524 datasheet (Rs/R0 power law); accurate absolute ppm requires field
 * calibration of @c MICS_R0 in clean air.
 */

#include "mics5524.h"
#include "adc_init.h"

#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include <math.h>

static const char *TAG = "MICS5524";

/* ------------------------------------------------------------------ */
/*  Hardware configuration                                            */
/* ------------------------------------------------------------------ */
static const adc_channel_t MICS_ADC_CHANNEL = ADC_CHANNEL_6;
static const adc_unit_t    MICS_ADC_UNIT    = ADC_UNIT_1;
static const adc_atten_t   MICS_ADC_ATTEN   = ADC_ATTEN_DB_12;

/* ------------------------------------------------------------------ */
/*  Sampling configuration                                            */
/* ------------------------------------------------------------------ */

/** Number of ADC samples averaged per voltage reading. */
#define MICS_NUM_SAMPLES 16

/* ------------------------------------------------------------------ */
/*  Sensor electrical model                                           */
/*  VCC : supply voltage for the divider [V]                          */
/*  RL  : load resistor [Ω]                                           */
/*  R0  : sensor resistance in clean air [Ω] — calibrate in the field */
/* ------------------------------------------------------------------ */
static const float MICS_VCC = 3.3f;
static const float MICS_RL  = 10000.0f;
static const float MICS_R0  = 75800.0f;

/* ------------------------------------------------------------------ */
/*  ADC calibration (optional; graceful fallback if unavailable)     */
/* ------------------------------------------------------------------ */
static adc_cali_handle_t s_cali_handle = NULL;
static bool              s_cali_ok     = false;

/* ------------------------------------------------------------------ */

/** @copydoc mics5524_init */
esp_err_t mics5524_init(void)
{
    adc_cali_line_fitting_config_t cfg = {
        .unit_id  = MICS_ADC_UNIT,
        .atten    = MICS_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    esp_err_t err = adc_cali_create_scheme_line_fitting(&cfg, &s_cali_handle);

    if (err == ESP_OK) {
        s_cali_ok = true;
        ESP_LOGI(TAG, "ADC calibration active");
    } else {
        s_cali_ok = false;
        s_cali_handle = NULL;
        ESP_LOGW(TAG, "ADC calibration unavailable (%s), using linear fallback",
                 esp_err_to_name(err));
    }

    return ESP_OK; /* not fatal: the sensor can work without calibration */
}

/* ------------------------------------------------------------------ */

/** @copydoc mics5524_read_voltage */
float mics5524_read_voltage(void)
{
    adc_oneshot_unit_handle_t adc = adc_get_handle();
    if (adc == NULL) {
        ESP_LOGE(TAG, "ADC not initialized");
        return -1.0f;
    }

    int32_t sum = 0;
    for (int i = 0; i < MICS_NUM_SAMPLES; i++) {
        int raw = 0;
        esp_err_t err = adc_oneshot_read(adc, MICS_ADC_CHANNEL, &raw);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "ADC reading failed at sample %d: %s", i, esp_err_to_name(err));
            return -1.0f;
        }
        sum += raw;
    }

    int avg_raw = (int)(sum / MICS_NUM_SAMPLES);

    if (s_cali_ok && s_cali_handle != NULL) {
        int mv = 0;
        esp_err_t err = adc_cali_raw_to_voltage(s_cali_handle, avg_raw, &mv);
        if (err == ESP_OK) {
            return (float)mv / 1000.0f;
        }
        ESP_LOGW(TAG, "Calibrated conversion failed, using fallback");
    }

    /* Linear fallback: map raw [0, 4095] to [0, VCC]. */
    return ((float)avg_raw / 4095.0f) * MICS_VCC;
}

/* ------------------------------------------------------------------ */

/** @copydoc mics5524_read_ppm */
float mics5524_read_ppm(void)
{
    float voltage = mics5524_read_voltage();
    if (voltage < 0.0f) {
        return -1.0f;
    }

    /* Prevent division by zero when the divider is open- or short-circuited. */
    if (voltage < 0.001f || voltage >= MICS_VCC) {
        ESP_LOGW(TAG, "Voltage out of range (%.3f V), sensor disconnected?", voltage);
        return -1.0f;
    }

    /* Rs from the resistor divider: Rs = ((VCC - V) / V) * RL */
    float rs    = ((MICS_VCC - voltage) / voltage) * MICS_RL;
    float ratio = rs / MICS_R0;

    /* Empirical CO curve from MiCS-5524 datasheet:
       ppm = 4.4 * (Rs/R0)^(-1.179)
       Calibrate R0 in clean air for accurate absolute ppm. */
    float ppm = 4.4f * powf(ratio, -1.179f);

    return ppm;
}
