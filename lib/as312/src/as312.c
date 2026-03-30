/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "as312.h"
#include "esp32_pinout.h"

#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "AS312";

esp_err_t as312_init(void)
{
    gpio_config_t pir_cfg = {
        .pin_bit_mask = (1ULL << PIN_PIR_OUT),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,   // oppure ENABLE se ti serve
        .intr_type = GPIO_INTR_DISABLE
    };

    esp_err_t err = gpio_config(&pir_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure PIR GPIO: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "AS312 initialized");
    return ESP_OK;
}

bool as312_read_motion(void)
{
    return gpio_get_level(PIN_PIR_OUT) != 0;
}

esp_err_t as312_restore(void)
{
    return as312_init();
}