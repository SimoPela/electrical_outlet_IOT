/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * Wrapper over the PMS7003 low-level driver (pms7003.h).
 *
 * In active mode the sensor pushes 32-byte frames every ~2.3 s.
 * pms7003_w_read() drains the UART RX buffer and parses the most
 * recent valid frame.
 */

#include "pms7003_w.h"
#include "pms7003.h"
#include "esp32_pinout.h"

#include <driver/uart.h>
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "PMS7003";

#define PMS7003_UART_PORT       UART_NUM_2
#define PMS7003_FRAME_LEN       32
#define PMS7003_READ_TIMEOUT_MS 150

static bool g_initialized = false;

esp_err_t pms7003_w_init(void)
{
    if (g_initialized)
        return ESP_OK;

    pms_config_t cfg = {
        .type       = PMS_TYPE_7003,
        .set_gpio   = PIN_PMS7003_SET,
        .reset_gpio = PIN_PMS7003_RESET,
        .uart_port  = PMS7003_UART_PORT,
    };

    esp_err_t err = pms_init(&cfg);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "pms_init failed: %s", esp_err_to_name(err));
        return err;
    }

    g_initialized = true;
    ESP_LOGI(TAG, "PMS7003 initialized (stabilizing ~30 s)");
    return ESP_OK;
}

esp_err_t pms7003_w_read(pms7003_data_t *out)
{
    ESP_RETURN_ON_FALSE(out != NULL,     ESP_ERR_INVALID_ARG,   TAG, "out is NULL");
    ESP_RETURN_ON_FALSE(g_initialized,   ESP_ERR_INVALID_STATE, TAG, "not initialized");

    if (pms_get_state() != PMS_STATE_ACTIVE)
        return ESP_ERR_NOT_FINISHED;

    uint8_t buf[128];
    int len = uart_read_bytes(PMS7003_UART_PORT, buf, sizeof(buf),
                              pdMS_TO_TICKS(PMS7003_READ_TIMEOUT_MS));
    if (len < PMS7003_FRAME_LEN)
        return ESP_ERR_NOT_FINISHED;

    esp_err_t err = ESP_FAIL;
    for (int i = len - PMS7003_FRAME_LEN; i >= 0; i--)
    {
        if (buf[i] == 0x42 && buf[i + 1] == 0x4D)
        {
            err = pms_parse_data(&buf[i], PMS7003_FRAME_LEN);
            if (err == ESP_OK)
                break;
        }
    }

    if (err != ESP_OK)
        return ESP_ERR_NOT_FINISHED;

    out->pm1_0_ug_m3 = (float)pms_get_data(PMS_FIELD_PM1_ATM);
    out->pm2_5_ug_m3 = (float)pms_get_data(PMS_FIELD_PM2_5_ATM);
    out->pm10_ug_m3  = (float)pms_get_data(PMS_FIELD_PM10_ATM);

    return ESP_OK;
}
