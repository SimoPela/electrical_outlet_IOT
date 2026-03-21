/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * Ref. : Sensirion SHT4x Datasheet
 *   - §4.5 : Measure T & RH (high precision, cmd 0xFD, 8.2 ms max)
 *   - §4.9 : Soft reset (cmd 0x94, 1 ms)
 *   - CRC-8 poly 0x31, init 0xFF
 */

#include "sht41.h"

#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SHT41";

#define SHT41_I2C_ADDRESS       0x44
#define SHT41_I2C_PORT          I2C_NUM_0
#define SHT41_I2C_TIMEOUT       pdMS_TO_TICKS(100)

#define SHT41_CMD_MEASURE_HP    0xFD   /* high-precision, 8.2 ms */
#define SHT41_CMD_SOFT_RESET    0x94   /* 1 ms */
#define SHT41_MEASURE_DELAY_MS  10     /* > 8.2 ms datasheet max */

static uint8_t sht41_crc8(const uint8_t *data, size_t len)
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

esp_err_t sht41_init(void)
{
    uint8_t cmd = SHT41_CMD_SOFT_RESET;
    esp_err_t err = i2c_master_write_to_device(
        SHT41_I2C_PORT, SHT41_I2C_ADDRESS, &cmd, 1, SHT41_I2C_TIMEOUT);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "soft reset failed: %s", esp_err_to_name(err));
        return ESP_FAIL;
    }

    vTaskDelay(pdMS_TO_TICKS(2));
    ESP_LOGI(TAG, "SHT41 initialized");
    return ESP_OK;
}

esp_err_t sht41_read(sht41_data_t *out)
{
    if (out == NULL) return ESP_ERR_INVALID_ARG;

    /* 1) Envoyer commande mesure haute précision */
    uint8_t cmd = SHT41_CMD_MEASURE_HP;
    esp_err_t ret = i2c_master_write_to_device(
        SHT41_I2C_PORT, SHT41_I2C_ADDRESS, &cmd, 1, SHT41_I2C_TIMEOUT);
    if (ret != ESP_OK) return ESP_FAIL;

    /* 2) Attendre la mesure (8.2 ms max) */
    vTaskDelay(pdMS_TO_TICKS(SHT41_MEASURE_DELAY_MS));

    /* 3) Lire 6 octets : T(2)+CRC, RH(2)+CRC */
    uint8_t buf[6] = {0};
    ret = i2c_master_read_from_device(
        SHT41_I2C_PORT, SHT41_I2C_ADDRESS, buf, sizeof(buf), SHT41_I2C_TIMEOUT);
    if (ret != ESP_OK) return ESP_FAIL;

    /* 4) CRC */
    if (sht41_crc8(&buf[0], 2) != buf[2] ||
        sht41_crc8(&buf[3], 2) != buf[5])
    {
        ESP_LOGW(TAG, "CRC mismatch");
        return ESP_FAIL;
    }

    /* 5) Convertir (datasheet §4.5) */
    uint16_t t_raw  = ((uint16_t)buf[0] << 8) | buf[1];
    uint16_t rh_raw = ((uint16_t)buf[3] << 8) | buf[4];

    out->temperature_c    = -45.0f + 175.0f * ((float)t_raw / 65535.0f);
    float rh = -6.0f + 125.0f * ((float)rh_raw / 65535.0f);
    if (rh < 0.0f)   rh = 0.0f;
    if (rh > 100.0f) rh = 100.0f;
    out->humidity_percent = rh;

    return ESP_OK;
}
