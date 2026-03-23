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

#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "SENSOR_INIT";

esp_err_t sensor_init_all(void)
{
    ESP_LOGI(TAG, "Initializing sensor peripherals");

    ESP_RETURN_ON_ERROR(gpio_init_all(), TAG, "GPIO init failed");
    ESP_RETURN_ON_ERROR(adc_init_all(), TAG, "ADC init failed");
    ESP_RETURN_ON_ERROR(i2s_init_all(), TAG, "I2S init failed");
    ESP_RETURN_ON_ERROR(uart_init_all(), TAG, "UART init failed");
    
    ESP_RETURN_ON_ERROR(mics5524_init(),  TAG, "mics5524 init failed");

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

    ESP_LOGI(TAG, "All sensor peripherals initialized");

    return ESP_OK;
}