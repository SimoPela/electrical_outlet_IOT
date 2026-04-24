/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file sensor_init.c
 * @brief One-shot initialization of board peripherals and all sensor drivers.
 *
 * Critical peripherals (ADC, I2S, I2C, UART, MiCS-5524) abort on failure.
 * Optional sensors (AS312, RGB LED, SHT41, SCD40, SGP41, BMP280, AS7341,
 * PMS7003) log a warning and continue, allowing the system to run in
 * degraded mode when individual sensors are absent or faulty.
 */

#include "sensor_init.h"
#include "adc_init.h"
#include "uart_init.h"
#include "i2s_init.h"

#include "as312.h"
#include "mics5524.h"
#include "scd40.h"
#include "sht41.h"
#include "sgp41.h"
#include "bmp.h"
#include "as7341_w.h"
#include "pms7003_w.h"
#include "rgbled.h"

#include <i2cdev.h>
#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "SENSOR_INIT";

/** @copydoc sensor_init_all */
esp_err_t sensor_init_all(void)
{
    ESP_LOGI(TAG, "Initializing sensor peripherals");

    ESP_RETURN_ON_ERROR(adc_init_all(), TAG, "ADC init failed");
    ESP_RETURN_ON_ERROR(i2s_init_all(), TAG, "I2S init failed");
    ESP_RETURN_ON_ERROR(i2cdev_init(), TAG, "i2cdev init failed");
    ESP_RETURN_ON_ERROR(uart_init_all(), TAG, "UART init failed");

    ESP_RETURN_ON_ERROR(mics5524_init(),  TAG, "mics5524 init failed");

    /* Suppress noisy I2C master debug messages after the bus is up. */
    esp_log_level_set("i2c.master", ESP_LOG_NONE);

    esp_err_t err;

    err = as312_init();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "as312_init failed, continuing");
    }

    err = rgbled_init();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "rgbled_init failed, continuing");
    }

    err = sht41_init();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "sht41_init failed, continuing");
    }

    err = scd40_init();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "scd40_init failed, continuing");
    }

    err = sgp41_init();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "sgp41_init failed, continuing");
    }

    err = bmp_init();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "bmp_init failed, continuing");
    }

    err = as7341_w_init();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "as7341_w_init failed, continuing");
    }

    err = pms7003_w_init();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "pms7003_w_init failed, continuing");
    }

    ESP_LOGI(TAG, "All sensor peripherals initialized");

    return ESP_OK;
}
