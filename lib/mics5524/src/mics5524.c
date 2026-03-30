/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
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
#define MICS_NUM_SAMPLES 16

/* ------------------------------------------------------------------ */
/*  Sensor electrical model calibration                               */
/*  VCC  : voltage supply for the divider [V]                         */
/*  RL   : load resistor [Ohm]                                        */
/*  R0   : resistance in clean air [Ohm] — CALIBRATE in the field     */
/* ------------------------------------------------------------------ */
static const float MICS_VCC = 3.3f;
static const float MICS_RL  = 10000.0f;
static const float MICS_R0  = 75800.0f;

/* ------------------------------------------------------------------ */
/*  ADC calibration (optional, graceful fallback if unavailable)      */
/* ------------------------------------------------------------------ */
static adc_cali_handle_t s_cali_handle = NULL;
static bool              s_cali_ok     = false;

/* ------------------------------------------------------------------ */

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

/** L1: drop line-fitting cali and rebuild; keeps the oneshot ADC unit (cheap). */
static esp_err_t mics5524_restore_l1(void)
{
    if (s_cali_handle != NULL) {
        adc_cali_delete_scheme_line_fitting(s_cali_handle);
        s_cali_handle = NULL;
    }
    s_cali_ok = false;
    return mics5524_init();
}

/** Quick probe: one raw read on the MiCS channel (validates ADC path). */
static esp_err_t mics5524_adc_probe(void)
{
    adc_oneshot_unit_handle_t adc = adc_get_handle();
    if (adc == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    int raw = 0;
    return adc_oneshot_read(adc, MICS_ADC_CHANNEL, &raw);
}

esp_err_t mics5524_restore(void)
{
    ESP_LOGW(TAG, "restore L1: refresh calibration");
    esp_err_t err = mics5524_restore_l1();
    if (err != ESP_OK) {
        return err;
    }

    err = mics5524_adc_probe();
    if (err == ESP_OK) {
        return ESP_OK;
    }

    ESP_LOGW(TAG, "restore L1 insufficient, L2: adc_restore + recalibrate");
    err = adc_restore();
    if (err != ESP_OK) {
        return err;
    }
    return mics5524_restore_l1();
}

/* ------------------------------------------------------------------ */

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

    /* linear fallback */
    return ((float)avg_raw / 4095.0f) * MICS_VCC;
}

/* ------------------------------------------------------------------ */

float mics5524_read_ppm(void)
{
    float voltage = mics5524_read_voltage();
    if (voltage < 0.0f) {
        return -1.0f;
    }

    /* Prevent division by zero: if voltage is 0 or equals VCC
       the divider is open/short-circuited */
    if (voltage < 0.001f || voltage >= MICS_VCC) {
        ESP_LOGW(TAG, "Voltage out of range (%.3f V), sensor disconnected?", voltage);
        return -1.0f;
    }

    /* Rs from the resistor divider: Rs = ((VCC - V) / V) * RL */
    float rs    = ((MICS_VCC - voltage) / voltage) * MICS_RL;
    float ratio = rs / MICS_R0;

    /* Empirical CO curve from MICS5524 datasheet:
       ppm = 4.4 * (Rs/R0)^(-1.179)
       Calibrate R0 in clean air for accurate results. */
    float ppm = 4.4f * powf(ratio, -1.179f);

    return ppm;
}