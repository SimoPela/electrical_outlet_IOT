/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file as312.c
 * @brief AS312 PIR motion sensor — GPIO input driver implementation.
 */

#include "as312.h"
#include "esp32_pinout.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "AS312";

/** @copydoc as312_init */
esp_err_t as312_init(void)
{
    gpio_config_t pir_cfg = {
        .pin_bit_mask = (1ULL << PIN_PIR_OUT),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
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

/** @copydoc as312_read_motion */
bool as312_read_motion(void)
{
    return gpio_get_level(PIN_PIR_OUT) != 0;
}
