/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * Wrapper over esp-idf-lib/scd4x managed component.
 */

#include "scd40.h"
#include "esp32_pinout.h"

#include "scd4x.h"
#include "esp_log.h"
#include "esp_check.h"
#include <stdbool.h>

static const char *TAG = "SCD40";

static i2c_dev_t g_scd40;
static bool g_scd40_initialized = false;

esp_err_t scd40_init(void)
{
    if (g_scd40_initialized)
        return ESP_OK;

    esp_err_t err = scd4x_init_desc(&g_scd40, I2C_NUM_0, PIN_I2C_SDA, PIN_I2C_SCL);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "scd4x_init_desc failed: %s", esp_err_to_name(err));
        return err;
    }

    err = scd4x_stop_periodic_measurement(&g_scd40);
    if (err != ESP_OK)
        ESP_LOGW(TAG, "stop_periodic_measurement: %s (may already be stopped)", esp_err_to_name(err));

    err = scd4x_start_periodic_measurement(&g_scd40);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "start_periodic_measurement failed: %s", esp_err_to_name(err));
        return err;
    }

    g_scd40_initialized = true;
    ESP_LOGI(TAG, "SCD40 initialized, periodic measurement started");
    return ESP_OK;
}

esp_err_t scd40_read(scd40_data_t *out)
{
    ESP_RETURN_ON_FALSE(out != NULL, ESP_ERR_INVALID_ARG, TAG, "out is NULL");
    ESP_RETURN_ON_FALSE(g_scd40_initialized, ESP_ERR_INVALID_STATE, TAG, "not initialized");

    bool ready = false;
    esp_err_t err = scd4x_get_data_ready_status(&g_scd40, &ready);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "get_data_ready_status failed: %s", esp_err_to_name(err));
        return err;
    }

    if (!ready)
        return ESP_ERR_NOT_FINISHED;

    uint16_t co2 = 0;
    float temperature = 0.0f;
    float humidity = 0.0f;

    err = scd4x_read_measurement(&g_scd40, &co2, &temperature, &humidity);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "read_measurement failed: %s", esp_err_to_name(err));
        return err;
    }

    out->co2_ppm          = (float)co2;
    out->temperature_c    = temperature;
    out->humidity_percent = humidity;

    return ESP_OK;
}
