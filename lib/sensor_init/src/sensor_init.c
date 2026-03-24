/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "sensor_init.h"
#include "gpio_init.h"
#include "adc_init.h"
#include "uart_init.h"
#include "i2s_init.h"

#include "mics5524.h"
#include "scd40.h"
#include "sht41.h"
#include "sgp41.h"
#include "bmp.h"
#include "as7341_w.h"
#include "pms7003_w.h"

#include <i2cdev.h>
#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "SENSOR_INIT";

esp_err_t sensor_init_all(void)
{
    ESP_LOGI(TAG, "Initializing sensor peripherals");

    ESP_RETURN_ON_ERROR(gpio_init_all(), TAG, "GPIO init failed");
    ESP_RETURN_ON_ERROR(adc_init_all(), TAG, "ADC init failed");
    /* I2S0 RX on PIN_I2S_WS / PIN_I2S_SCK / PIN_I2S_SD — INMP441 (see inmp441_w, audio_task). */
    ESP_RETURN_ON_ERROR(i2s_init_all(), TAG, "I2S init failed");
    ESP_RETURN_ON_ERROR(uart_init_all(), TAG, "UART init failed");
    
    ESP_RETURN_ON_ERROR(mics5524_init(),  TAG, "mics5524 init failed");

    ESP_RETURN_ON_ERROR(i2cdev_init(), TAG, "i2cdev init failed");

    esp_log_level_set("i2c.master", ESP_LOG_NONE);

    esp_err_t err;

    err = sht41_init();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "sht41_init failed (sensor not connected?), continuing");
    }
    
    err = scd40_init();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "scd40_init failed (sensor not connected?), continuing");
    }

    err = sgp41_init();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "sgp41_init failed (sensor not connected?), continuing");
    }

    err = bmp_init();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "bmp_init failed (sensor not connected?), continuing");
    }

    err = as7341_w_init();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "as7341_w_init failed (sensor not connected?), continuing");
    }

    err = pms7003_w_init();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "pms7003_w_init failed (sensor not connected?), continuing");
    }

    ESP_LOGI(TAG, "All sensor peripherals initialized");

    return ESP_OK;
}