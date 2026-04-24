/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * SCD40 driver — fully direct ESP-IDF I2C master API.
 *
 * All communication goes through a single raw device handle obtained
 * from the shared i2cdev bus.
 */

/**
 * @file scd40.c
 * @brief Sensirion SCD40 CO₂ / RH / T sensor — direct ESP-IDF I2C master driver implementation.
 */

#include "scd40.h"
#include "esp32_pinout.h"

#include <i2cdev.h>
#include <driver/i2c_master.h>
#include "esp_log.h"
#include "esp_check.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>

static const char *TAG = "SCD40";

/** SCD40 7-bit I2C address. */
#define SCD40_I2C_ADDR       0x62
#define SCD40_I2C_FREQ_HZ    100000
#define SCD40_I2C_TIMEOUT_MS 50

/** SCD40 command opcodes (datasheet §3). */
#define CMD_WAKE_UP              0x36F6
#define CMD_STOP_PERIODIC        0x3F86
#define CMD_REINIT               0x3646
#define CMD_GET_SERIAL           0x3682
#define CMD_START_PERIODIC       0x21B1
#define CMD_READ_MEASUREMENT     0xEC05
#define CMD_GET_DATA_READY       0xE4B8

/** Number of consecutive read failures before forcing a restart of periodic measurement. */
#define SCD40_MAX_CONSEC_FAIL    12

static i2c_master_dev_handle_t g_handle = NULL;
static bool g_initialized = false;
static int  g_consec_fail = 0;

/**
 * @brief Compute CRC-8 over @p len bytes using the Sensirion polynomial (0x31, init 0xFF).
 *
 * @param data Pointer to input bytes.
 * @param len  Number of bytes.
 * @return 8-bit CRC.
 */
static uint8_t scd40_crc8(const uint8_t *data, size_t len)
{
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (int b = 0; b < 8; b++)
            crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
    }
    return crc;
}

/**
 * @brief Transmit a 16-bit command word to the SCD40.
 *
 * @param cmd_code Command opcode (big-endian on the wire).
 * @return ESP-IDF I2C error code.
 */
static esp_err_t scd40_send_cmd(uint16_t cmd_code)
{
    uint8_t cmd[2] = { (uint8_t)(cmd_code >> 8),
                       (uint8_t)(cmd_code & 0xFF) };
    return i2c_master_transmit(g_handle, cmd, sizeof(cmd),
                               SCD40_I2C_TIMEOUT_MS);
}

/**
 * @brief Verify the CRC of each 2-byte word in a Sensirion response frame.
 *
 * @param buf     Response buffer (3 bytes per word: [MSB][LSB][CRC]).
 * @param n_words Number of words to check.
 * @return @c true if all CRCs pass, @c false on any mismatch.
 */
static bool scd40_check_crc(const uint8_t *buf, size_t n_words)
{
    for (size_t i = 0; i < n_words; i++)
    {
        if (scd40_crc8(&buf[i * 3], 2) != buf[i * 3 + 2])
            return false;
    }
    return true;
}

/**
 * @brief Send a command then read and CRC-verify @p n_words response words.
 *
 * @param cmd_code  Command opcode.
 * @param buf       Output buffer (must hold @p n_words * 3 bytes).
 * @param n_words   Number of 2-byte + CRC word triplets to receive.
 * @param delay_us  Microsecond delay between command and read (0 for none).
 * @return ESP_OK on success, @c ESP_ERR_INVALID_CRC on CRC mismatch, or I2C error.
 */
static esp_err_t scd40_cmd_read(uint16_t cmd_code, uint8_t *buf,
                                size_t n_words, uint32_t delay_us)
{
    esp_err_t err = scd40_send_cmd(cmd_code);
    if (err != ESP_OK)
        return err;

    if (delay_us > 0)
        esp_rom_delay_us(delay_us);

    size_t len = n_words * 3;
    err = i2c_master_receive(g_handle, buf, len, SCD40_I2C_TIMEOUT_MS);
    if (err != ESP_OK)
        return err;

    if (!scd40_check_crc(buf, n_words))
        return ESP_ERR_INVALID_CRC;

    return ESP_OK;
}

/**
 * @brief Wake, stop-periodic, reinit, read serial number, then start periodic measurement.
 *
 * Called by both @ref scd40_init (first time) and the consecutive-failure recovery path.
 *
 * @return ESP_OK on success, or an I2C / CRC error code.
 */
static esp_err_t scd40_start_periodic(void)
{
    esp_err_t err;

    err = scd40_send_cmd(CMD_WAKE_UP);
    if (err != ESP_OK)
        ESP_LOGW(TAG, "wake_up: %s", esp_err_to_name(err));
    vTaskDelay(pdMS_TO_TICKS(30));

    err = scd40_send_cmd(CMD_STOP_PERIODIC);
    if (err != ESP_OK)
        ESP_LOGW(TAG, "stop_periodic: %s", esp_err_to_name(err));
    vTaskDelay(pdMS_TO_TICKS(500));

    err = scd40_send_cmd(CMD_REINIT);
    if (err != ESP_OK)
        ESP_LOGW(TAG, "reinit: %s", esp_err_to_name(err));
    vTaskDelay(pdMS_TO_TICKS(30));

    uint8_t sbuf[9];
    err = scd40_cmd_read(CMD_GET_SERIAL, sbuf, 3, 1500);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "get_serial failed: %s", esp_err_to_name(err));
        return err;
    }
    uint16_t s0 = (sbuf[0] << 8) | sbuf[1];
    uint16_t s1 = (sbuf[3] << 8) | sbuf[4];
    uint16_t s2 = (sbuf[6] << 8) | sbuf[7];
    ESP_LOGI(TAG, "serial: 0x%04x%04x%04x", s0, s1, s2);

    err = scd40_send_cmd(CMD_START_PERIODIC);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "start_periodic failed: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "periodic measurement started (first sample in ~5 s)");
    return ESP_OK;
}

/** @copydoc scd40_init */
esp_err_t scd40_init(void)
{
    if (g_initialized)
        return ESP_OK;

    if (g_handle == NULL)
    {
        i2c_master_bus_handle_t bus = NULL;
        esp_err_t err = i2cdev_get_shared_handle(I2C_NUM_0, (void **)&bus);
        if (err != ESP_OK || bus == NULL)
        {
            ESP_LOGE(TAG, "get_shared_handle failed: %s", esp_err_to_name(err));
            return ESP_FAIL;
        }

        i2c_device_config_t cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address  = SCD40_I2C_ADDR,
            .scl_speed_hz    = SCD40_I2C_FREQ_HZ,
        };
        err = i2c_master_bus_add_device(bus, &cfg, &g_handle);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "bus_add_device failed: %s", esp_err_to_name(err));
            return err;
        }
    }

    esp_err_t err = scd40_start_periodic();
    if (err != ESP_OK)
        return err;

    g_consec_fail = 0;
    g_initialized = true;
    ESP_LOGI(TAG, "SCD40 initialized");
    return ESP_OK;
}

/** @copydoc scd40_read */
esp_err_t scd40_read(scd40_data_t *out)
{
    ESP_RETURN_ON_FALSE(out != NULL, ESP_ERR_INVALID_ARG, TAG, "out is NULL");
    ESP_RETURN_ON_FALSE(g_initialized, ESP_ERR_INVALID_STATE, TAG, "not initialized");

    uint8_t buf[9];
    esp_err_t err = scd40_cmd_read(CMD_READ_MEASUREMENT, buf, 3, 1500);
    if (err != ESP_OK)
    {
        g_consec_fail++;
        if (g_consec_fail >= SCD40_MAX_CONSEC_FAIL)
        {
            ESP_LOGW(TAG, "read failed %d times, restarting", g_consec_fail);
            g_consec_fail = 0;
            scd40_start_periodic();
        }
        return ESP_ERR_NOT_FINISHED;
    }

    uint16_t co2_raw  = (buf[0] << 8) | buf[1];
    int16_t  temp_raw = (int16_t)((buf[3] << 8) | buf[4]);
    uint16_t rh_raw   = (buf[6] << 8) | buf[7];

    if (co2_raw == 0)
        return ESP_ERR_NOT_FINISHED;

    g_consec_fail = 0;

    out->co2_ppm          = (float)co2_raw;
    out->temperature_c    = -45.0f + 175.0f * ((float)temp_raw / 65535.0f);
    out->humidity_percent = 100.0f * ((float)rh_raw / 65535.0f);

    return ESP_OK;
}
