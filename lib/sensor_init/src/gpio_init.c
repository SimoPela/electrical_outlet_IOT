/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "gpio_init.h"
#include "esp32_pinout.h"

#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "GPIO_INIT";

esp_err_t gpio_init_all(void)
{
    esp_err_t err;

    gpio_config_t pir_cfg = {
        .pin_bit_mask = (1ULL << PIN_PIR_OUT),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    err = gpio_config(&pir_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure PIR GPIO");
        return err;
    }

    gpio_config_t led_cfg = {
        .pin_bit_mask = (1ULL << PIN_LED_R) |
                        (1ULL << PIN_LED_G) |
                        (1ULL << PIN_LED_B),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    err = gpio_config(&led_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure RGB LED GPIOs");
        return err;
    }

    gpio_set_level(PIN_LED_R, 0);
    gpio_set_level(PIN_LED_G, 0);
    gpio_set_level(PIN_LED_B, 0);

    ESP_LOGI(TAG, "GPIO initialized");
    return ESP_OK;
}

esp_err_t gpio_restore(void)
{
    return gpio_init_all();
}