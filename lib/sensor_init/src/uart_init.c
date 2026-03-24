/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "uart_init.h"
#include "esp32_pinout.h"

#include "driver/uart.h"
#include "esp_log.h"

static const char *TAG = "UART_INIT";

#define PMS7003_UART_PORT       UART_NUM_2
#define PMS7003_BAUD_RATE       9600
#define PMS7003_RX_BUF_SIZE     256

static bool s_uart_installed = false;

esp_err_t uart_init_all(void)
{
    if (s_uart_installed) {
        return ESP_OK;
    }

    uart_config_t uart_config = {
        .baud_rate = PMS7003_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };

    esp_err_t err = uart_param_config(PMS7003_UART_PORT, &uart_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure UART");
        return err;
    }

    err = uart_set_pin(PMS7003_UART_PORT,
                       PIN_PMS7003_RX_SENSOR,   /* ESP32 TX → Sensor RX */
                       PIN_PMS7003_TX_SENSOR,    /* ESP32 RX ← Sensor TX */
                       UART_PIN_NO_CHANGE,
                       UART_PIN_NO_CHANGE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set UART pins");
        return err;
    }

    err = uart_driver_install(PMS7003_UART_PORT, PMS7003_RX_BUF_SIZE, 0, 0, NULL, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install UART driver");
        return err;
    }

    s_uart_installed = true;
    ESP_LOGI(TAG, "UART initialized");
    return ESP_OK;
}

esp_err_t uart_restore(void)
{
    if (s_uart_installed) {
        esp_err_t err = uart_driver_delete(PMS7003_UART_PORT);
        s_uart_installed = false;
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "uart_driver_delete failed: %s", esp_err_to_name(err));
            return err;
        }
    }
    return uart_init_all();
}