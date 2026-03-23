/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * Driver for SGP41 VOC/NOx sensor using i2cdev.
 *
 * Ref. : Sensirion SGP41 Datasheet
 *   - §6.2  : sgp41_execute_conditioning (cmd 0x2612, 50 ms)
 *   - §6.3  : sgp41_measure_raw_signals  (cmd 0x2619, 50 ms)
 *   - §6.6  : sgp41_turn_heater_off      (cmd 0x3615)
 *   - CRC-8 poly 0x31, init 0xFF
 *
 * Returns raw SRAW values; for VOC/NOx indices (0-500),
 * Sensirion's Gas Index Algorithm (BSD-3) is needed.
 */

#include "sgp41.h"
#include "esp32_pinout.h"

#include <i2cdev.h>
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <math.h>
#include <stdbool.h>

static const char *TAG = "SGP41";

#define SGP41_I2C_ADDRESS       0x59
#define SGP41_I2C_FREQ_HZ      100000

#define SGP41_CMD_CONDITIONING  0x2612
#define SGP41_CMD_MEASURE_RAW   0x2619
#define SGP41_CMD_HEATER_OFF    0x3615
#define SGP41_MEASURE_DELAY_MS  55      /* > 50 ms datasheet max */

#define SGP41_DEFAULT_RH        0x8000  /* 50 %RH */
#define SGP41_DEFAULT_T         0x6666  /* 25 °C  */

static i2c_dev_t g_sgp41;
static bool g_sgp41_initialized = false;

static uint8_t sgp41_crc8(const uint8_t *data, size_t len)
{
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (int b = 0; b < 8; b++)
        {
            if (crc & 0x80) crc = (uint8_t)((crc << 1) ^ 0x31);
            else            crc = (uint8_t)(crc << 1);
        }
    }
    return crc;
}

static void build_cmd(uint8_t *buf, uint16_t cmd, uint16_t rh_ticks, uint16_t t_ticks)
{
    buf[0] = (uint8_t)(cmd >> 8);
    buf[1] = (uint8_t)(cmd & 0xFF);
    buf[2] = (uint8_t)(rh_ticks >> 8);
    buf[3] = (uint8_t)(rh_ticks & 0xFF);
    buf[4] = sgp41_crc8(&buf[2], 2);
    buf[5] = (uint8_t)(t_ticks >> 8);
    buf[6] = (uint8_t)(t_ticks & 0xFF);
    buf[7] = sgp41_crc8(&buf[5], 2);
}

static uint16_t rh_to_ticks(float rh)
{
    if (isnan(rh)) return SGP41_DEFAULT_RH;
    if (rh < 0.0f)   rh = 0.0f;
    if (rh > 100.0f) rh = 100.0f;
    return (uint16_t)(rh * 65535.0f / 100.0f);
}

static uint16_t temp_to_ticks(float t)
{
    if (isnan(t)) return SGP41_DEFAULT_T;
    return (uint16_t)((t + 45.0f) * 65535.0f / 175.0f);
}

static esp_err_t sgp41_init_desc(void)
{
    g_sgp41.port = I2C_NUM_0;
    g_sgp41.addr = SGP41_I2C_ADDRESS;
    g_sgp41.cfg.sda_io_num = PIN_I2C_SDA;
    g_sgp41.cfg.scl_io_num = PIN_I2C_SCL;
    g_sgp41.cfg.master.clk_speed = SGP41_I2C_FREQ_HZ;

    return i2c_dev_create_mutex(&g_sgp41);
}

esp_err_t sgp41_init(void)
{
    if (g_sgp41_initialized)
        return ESP_OK;

    esp_err_t err = sgp41_init_desc();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "init_desc failed: %s", esp_err_to_name(err));
        return err;
    }

    uint8_t cmd[8];
    build_cmd(cmd, SGP41_CMD_CONDITIONING, SGP41_DEFAULT_RH, SGP41_DEFAULT_T);

    I2C_DEV_TAKE_MUTEX(&g_sgp41);
    I2C_DEV_CHECK(&g_sgp41, i2c_dev_write(&g_sgp41, NULL, 0, cmd, sizeof(cmd)));
    vTaskDelay(pdMS_TO_TICKS(SGP41_MEASURE_DELAY_MS));

    uint8_t buf[3] = {0};
    I2C_DEV_CHECK(&g_sgp41, i2c_dev_read(&g_sgp41, NULL, 0, buf, sizeof(buf)));
    I2C_DEV_GIVE_MUTEX(&g_sgp41);

    if (sgp41_crc8(&buf[0], 2) != buf[2])
    {
        ESP_LOGE(TAG, "conditioning CRC mismatch");
        return ESP_FAIL;
    }

    g_sgp41_initialized = true;
    ESP_LOGI(TAG, "SGP41 initialized (NOx valid after ~10 s)");
    return ESP_OK;
}

esp_err_t sgp41_read(sgp41_data_t *out, float temperature, float humidity)
{
    ESP_RETURN_ON_FALSE(out != NULL, ESP_ERR_INVALID_ARG, TAG, "out is NULL");
    ESP_RETURN_ON_FALSE(g_sgp41_initialized, ESP_ERR_INVALID_STATE, TAG, "not initialized");

    uint8_t cmd[8];
    build_cmd(cmd, SGP41_CMD_MEASURE_RAW, rh_to_ticks(humidity), temp_to_ticks(temperature));

    I2C_DEV_TAKE_MUTEX(&g_sgp41);
    I2C_DEV_CHECK(&g_sgp41, i2c_dev_write(&g_sgp41, NULL, 0, cmd, sizeof(cmd)));
    vTaskDelay(pdMS_TO_TICKS(SGP41_MEASURE_DELAY_MS));

    uint8_t buf[6] = {0};
    I2C_DEV_CHECK(&g_sgp41, i2c_dev_read(&g_sgp41, NULL, 0, buf, sizeof(buf)));
    I2C_DEV_GIVE_MUTEX(&g_sgp41);

    if (sgp41_crc8(&buf[0], 2) != buf[2] ||
        sgp41_crc8(&buf[3], 2) != buf[5])
    {
        ESP_LOGW(TAG, "CRC mismatch");
        return ESP_FAIL;
    }

    out->sraw_voc = ((uint16_t)buf[0] << 8) | buf[1];
    out->sraw_nox = ((uint16_t)buf[3] << 8) | buf[4];

    return ESP_OK;
}
