/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "sht41.h"
#include "esp32_pinout.h"

#include "sht4x.h"
#include "esp_log.h"
#include "esp_check.h"
#include <stdbool.h>

static const char *TAG = "SHT41";

static sht4x_t g_sht41;
static bool g_sht41_initialized = false;

esp_err_t sht41_init(void)
{
    if (g_sht41_initialized)
        return ESP_OK;

    esp_err_t err = sht4x_init_desc(&g_sht41, I2C_NUM_0, PIN_I2C_SDA, PIN_I2C_SCL);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "sht4x_init_desc failed: %s", esp_err_to_name(err));
        return err;
    }

    err = sht4x_init(&g_sht41);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "sht4x_init failed: %s", esp_err_to_name(err));
        return err;
    }

    g_sht41_initialized = true;
    ESP_LOGI(TAG, "SHT41 initialized (serial 0x%08" PRIx32 ")",
             (uint32_t)g_sht41.serial);
    return ESP_OK;
}

esp_err_t sht41_restore(void)
{
    if (g_sht41_initialized) {
        esp_err_t err = sht4x_free_desc(&g_sht41);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "sht4x_free_desc: %s", esp_err_to_name(err));
        }
        g_sht41_initialized = false;
    }
    return sht41_init();
}

esp_err_t sht41_read(sht41_data_t *out)
{
    ESP_RETURN_ON_FALSE(out != NULL, ESP_ERR_INVALID_ARG, TAG, "out is NULL");
    ESP_RETURN_ON_FALSE(g_sht41_initialized, ESP_ERR_INVALID_STATE, TAG, "not initialized");

    float temperature = 0.0f;
    float humidity = 0.0f;

    esp_err_t err = sht4x_measure(&g_sht41, &temperature, &humidity);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "sht4x_measure failed: %s", esp_err_to_name(err));
        return err;
    }

    out->temperature_c    = temperature;
    out->humidity_percent = humidity;

    return ESP_OK;
}
