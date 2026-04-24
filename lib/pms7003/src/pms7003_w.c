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
 *
 * Recovery strategy (pms7003_w_restore):
 *   Level 1 — soft reset (sleep → wake).  Fast, non-destructive.
 *   Level 2 — full deinit + reinit.  Used if level 1 fails or the
 *              driver is in an unrecoverable state.
 *
 * g_degraded is set when level 1 fails and cleared only after a
 * successful level-2 recovery AND a subsequent successful read.
 */

/**
 * @file pms7003_w.c
 * @brief Plantower PMS7003 particulate sensor — ESP-IDF wrapper implementation.
 */

#include "pms7003_w.h"
#include "pms7003.h"
#include "esp32_pinout.h"

#include <driver/uart.h>
#include <esp_log.h>
#include <esp_check.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>

static const char *TAG = "PMS7003_W";

#define PMS7003_UART_PORT       UART_NUM_2
#define PMS7003_FRAME_LEN       32
#define PMS7003_READ_TIMEOUT_MS 150

/*
 * After a reset/reinit the sensor needs ~30 s to stabilize.
 * pms7003_w_restore() does NOT block for that time — it just kicks
 * off the process and returns.  The caller should retry pms7003_w_read()
 * until it stops returning ESP_ERR_NOT_FINISHED.
 */

/** Maximum consecutive restore attempts before giving up and returning ESP_ERR_NOT_RECOVERABLE. */
#define PMS7003_MAX_RESTORE_ATTEMPTS 3

static bool     g_initialized       = false;
static bool     g_degraded          = false;  /**< Set after a failed level-1 reset. */
static uint8_t  g_restore_attempts  = 0;      /**< Lifetime counter; reset on successful read. */

/* ------------------------------------------------------------------ */
/* Public API                                                           */
/* ------------------------------------------------------------------ */

/** @copydoc pms7003_w_init */
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
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "pms_init failed: %s", esp_err_to_name(err));
        return err;
    }

    g_initialized       = true;
    g_degraded          = false;
    g_restore_attempts  = 0;
    ESP_LOGI(TAG, "PMS7003 initialized (stabilizing ~30 s)");
    return ESP_OK;
}

/** @copydoc pms7003_w_is_degraded */
bool pms7003_w_is_degraded(void)
{
    return g_degraded;
}

/** @copydoc pms7003_w_read */
esp_err_t pms7003_w_read(pms7003_data_t *out)
{
    ESP_RETURN_ON_FALSE(out != NULL,   ESP_ERR_INVALID_ARG,   TAG, "out is NULL");
    ESP_RETURN_ON_FALSE(g_initialized, ESP_ERR_INVALID_STATE, TAG, "not initialized");

    if (pms_get_state() != PMS_STATE_ACTIVE)
        return ESP_ERR_NOT_FINISHED;

    uint8_t buf[128];
    int len = uart_read_bytes(PMS7003_UART_PORT, buf, sizeof(buf),
                              pdMS_TO_TICKS(PMS7003_READ_TIMEOUT_MS));
    if (len < PMS7003_FRAME_LEN)
        return ESP_ERR_NOT_FINISHED;

    /*
     * Scan backwards: the most recent frame is at the tail of the buffer.
     * Stop at the first valid (checksum-passing) frame we find.
     */
    esp_err_t parse_err = ESP_FAIL;
    for (int i = len - PMS7003_FRAME_LEN; i >= 0; i--) {
        if (buf[i] == 0x42 && buf[i + 1] == 0x4D) {
            parse_err = pms_parse_data(&buf[i], PMS7003_FRAME_LEN);
            if (parse_err == ESP_OK)
                break;
        }
    }

    if (parse_err != ESP_OK)
        return ESP_ERR_NOT_FINISHED;

    uint16_t pm1 = 0, pm2_5 = 0, pm10 = 0;
    ESP_RETURN_ON_ERROR(pms_get_data(PMS_FIELD_PM1_ATM,   &pm1),  TAG, "get pm1 failed");
    ESP_RETURN_ON_ERROR(pms_get_data(PMS_FIELD_PM2_5_ATM, &pm2_5), TAG, "get pm2_5 failed");
    ESP_RETURN_ON_ERROR(pms_get_data(PMS_FIELD_PM10_ATM,  &pm10), TAG, "get pm10 failed");

    out->pm1_0_ug_m3 = (float)pm1;
    out->pm2_5_ug_m3 = (float)pm2_5;
    out->pm10_ug_m3  = (float)pm10;

    /*
     * Successful read: sensor confirmed healthy.
     * Clear degraded flag and reset the attempt counter so future
     * failures restart from level-1.
     */
    if (g_degraded) {
        ESP_LOGI(TAG, "sensor recovered successfully");
        g_degraded         = false;
        g_restore_attempts = 0;
    }

    return ESP_OK;
}
