/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "sensor_init.h"
#include "gpio_init.h"
#include "adc_init.h"
#include "i2c_init.h"
#include "uart_init.h"
#include "i2s_init.h"

#include "mics5524.h"

#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "SENSOR_INIT";

esp_err_t sensor_init_all(void)
{
    ESP_LOGI(TAG, "Initializing sensor peripherals");

    ESP_RETURN_ON_ERROR(gpio_init_all(), TAG, "GPIO init failed");
    ESP_RETURN_ON_ERROR(adc_init_all(), TAG, "ADC init failed");
    ESP_RETURN_ON_ERROR(i2c_init_all(), TAG, "I2C init failed");
    ESP_RETURN_ON_ERROR(i2s_init_all(), TAG, "I2S init failed");
    ESP_RETURN_ON_ERROR(uart_init_all(), TAG, "UART init failed");
    
    ESP_LOGI(TAG, "All sensor peripherals initialized");

    return ESP_OK;
}