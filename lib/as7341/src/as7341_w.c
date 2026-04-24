/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * Wrapper over k0i05/esp_as7341 managed component.
 *
 * Header is as7341_w.h (not as7341.h) to avoid filename collision
 * with the managed component's as7341.h.
 */

/**
 * @file as7341_w.c
 * @brief AMS OSRAM AS7341 8-channel spectrometer — ESP-IDF I2C wrapper implementation.
 */

#include "as7341_w.h"
#include "as7341.h"
#include <i2cdev.h>

#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>

static const char *TAG = "AS7341";

/** Polling interval between data-ready checks [ms]. */
#define AS7341_DATA_READY_POLL_MS   10
/** Maximum number of polling attempts before timing out. */
#define AS7341_DATA_READY_TIMEOUT   50

static as7341_handle_t g_as7341 = NULL;
static bool g_as7341_initialized = false;

/** @copydoc as7341_w_init */
esp_err_t as7341_w_init(void)
{
    if (g_as7341_initialized)
        return ESP_OK;

    i2c_master_bus_handle_t bus = NULL;
    esp_err_t err = i2cdev_get_shared_handle(I2C_NUM_0, (void **)&bus);
    if (err != ESP_OK || bus == NULL)
    {
        ESP_LOGE(TAG, "i2cdev_get_shared_handle failed: %s", esp_err_to_name(err));
        return ESP_FAIL;
    }

    as7341_config_t cfg = I2C_AS7341_CONFIG_DEFAULT;

    err = as7341_init(bus, &cfg, &g_as7341);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "as7341_init failed: %s", esp_err_to_name(err));
        return err;
    }

    g_as7341_initialized = true;
    ESP_LOGI(TAG, "AS7341 initialized");
    return ESP_OK;
}

/** @copydoc as7341_w_read */
esp_err_t as7341_w_read(as7341_data_t *out)
{
    ESP_RETURN_ON_FALSE(out != NULL, ESP_ERR_INVALID_ARG, TAG, "out is NULL");
    ESP_RETURN_ON_FALSE(g_as7341_initialized, ESP_ERR_INVALID_STATE, TAG, "not initialized");

    esp_err_t err = as7341_enable_spectral_measurement(g_as7341);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "enable_spectral_measurement failed: %s", esp_err_to_name(err));
        return err;
    }

    bool ready = false;
    for (int i = 0; i < AS7341_DATA_READY_TIMEOUT; i++)
    {
        vTaskDelay(pdMS_TO_TICKS(AS7341_DATA_READY_POLL_MS));
        err = as7341_get_data_status(g_as7341, &ready);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "get_data_status failed: %s", esp_err_to_name(err));
            return err;
        }
        if (ready)
            break;
    }

    if (!ready)
    {
        ESP_LOGW(TAG, "data ready timeout");
        as7341_disable_spectral_measurement(g_as7341);
        return ESP_ERR_TIMEOUT;
    }

    as7341_channels_spectral_data_t raw = {0};
    err = as7341_get_spectral_measurements(g_as7341, &raw);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "get_spectral_measurements failed: %s", esp_err_to_name(err));
        return err;
    }

    as7341_channels_basic_counts_data_t bc = {0};
    err = as7341_get_basic_counts(g_as7341, raw, &bc);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "get_basic_counts failed: %s", esp_err_to_name(err));
        return err;
    }

    out->channels[0] = bc.f1;
    out->channels[1] = bc.f2;
    out->channels[2] = bc.f3;
    out->channels[3] = bc.f4;
    out->channels[4] = bc.f5;
    out->channels[5] = bc.f6;
    out->channels[6] = bc.f7;
    out->channels[7] = bc.f8;

    return ESP_OK;
}
