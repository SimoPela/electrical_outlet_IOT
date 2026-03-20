/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "adc_init.h"
#include "esp32_pinout.h"

#include "driver/adc.h"
#include "esp_log.h"

static const char *TAG = "ADC_INIT";

esp_err_t adc_init_all(void)
{
    esp_err_t err;

    err = adc1_config_width(ADC_WIDTH_BIT_12);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set ADC width");
        return err;
    }

    err = adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure ADC1 channel 6 for GPIO34");
        return err;
    }

    ESP_LOGI(TAG, "ADC initialized");
    return ESP_OK;
}