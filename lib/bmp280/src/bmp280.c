/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * Wrapper over esp-idf-lib/bmp280 managed component.
 *
 * The wrapper header is bmp.h (not bmp280.h) to avoid filename
 * collision with the managed component's bmp280.h.
 * Function prefix is bmp_ (not bmp280_) for the same reason.
 */

#include "bmp.h"
#include "esp32_pinout.h"

#include "bmp280.h"
#include "esp_log.h"
#include "esp_check.h"
#include <stdbool.h>

static const char *TAG = "BMP280";

static bmp280_t g_bmp;
static bool g_bmp_initialized = false;

esp_err_t bmp_init(void)
{
    if (g_bmp_initialized)
        return ESP_OK;

    esp_err_t err = bmp280_init_desc(&g_bmp, BMP280_I2C_ADDRESS_0,
                                     I2C_NUM_0, PIN_I2C_SDA, PIN_I2C_SCL);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "bmp280_init_desc failed: %s", esp_err_to_name(err));
        return err;
    }

    bmp280_params_t params;
    bmp280_init_default_params(&params);

    err = bmp280_init(&g_bmp, &params);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "bmp280_init failed: %s", esp_err_to_name(err));
        return err;
    }

    g_bmp_initialized = true;
    ESP_LOGI(TAG, "BMP280 initialized (chip id 0x%02X)", g_bmp.id);
    return ESP_OK;
}

esp_err_t bmp_restore(void)
{
    if (g_bmp_initialized) {
        ESP_LOGW(TAG, "restore L1: bmp280_init (soft re-init on same descriptor)");
        bmp280_params_t params;
        bmp280_init_default_params(&params);
        esp_err_t err = bmp280_init(&g_bmp, &params);
        if (err == ESP_OK) {
            return ESP_OK;
        }
        ESP_LOGW(TAG, "restore L1 failed, L2: bmp280_free_desc + init");
        (void)bmp280_free_desc(&g_bmp);
        g_bmp_initialized = false;
    }
    return bmp_init();
}

esp_err_t bmp_read(bmp_data_t *out)
{
    ESP_RETURN_ON_FALSE(out != NULL, ESP_ERR_INVALID_ARG, TAG, "out is NULL");
    ESP_RETURN_ON_FALSE(g_bmp_initialized, ESP_ERR_INVALID_STATE, TAG, "not initialized");

    float temperature = 0.0f;
    float pressure_pa = 0.0f;

    esp_err_t err = bmp280_read_float(&g_bmp, &temperature, &pressure_pa, NULL);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "bmp280_read_float failed: %s", esp_err_to_name(err));
        return err;
    }

    out->pressure_hpa  = pressure_pa / 100.0f;
    out->temperature_c = temperature;

    return ESP_OK;
}
