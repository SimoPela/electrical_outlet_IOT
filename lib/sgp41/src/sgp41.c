/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * Ref. : Sensirion SGP41 Datasheet
 *   - §6.2  : sgp41_execute_conditioning (cmd 0x2612, 50 ms)
 *   - §6.3  : sgp41_measure_raw_signals (cmd 0x2619, 50 ms)
 *   - §6.6  : sgp41_turn_heater_off (cmd 0x3615)
 *   - CRC-8 poly 0x31, init 0xFF
 *
 * Le SGP41 retourne des valeurs brutes SRAW (pas des index VOC/NOx).
 * Pour obtenir des index 0-500, il faut le Gas Index Algorithm de
 * Sensirion (open-source, BSD-3).
 */

#include "sgp41.h"

#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <math.h>

static const char *TAG = "SGP41";

#define SGP41_I2C_ADDRESS           0x59
#define SGP41_I2C_PORT              I2C_NUM_0
#define SGP41_I2C_TIMEOUT           pdMS_TO_TICKS(100)

#define SGP41_CMD_CONDITIONING      0x2612
#define SGP41_CMD_MEASURE_RAW       0x2619
#define SGP41_CMD_HEATER_OFF        0x3615
#define SGP41_MEASURE_DELAY_MS      55     /* > 50 ms datasheet max */

/* Valeurs par défaut pour la compensation T/RH (25°C, 50%RH). */
#define SGP41_DEFAULT_RH            0x8000
#define SGP41_DEFAULT_T             0x6666

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

/* Construire le buffer de commande avec 2 mots d'arguments + CRC.
 * Format : CMD_MSB CMD_LSB  RH_MSB RH_LSB CRC_RH  T_MSB T_LSB CRC_T */
static void sgp41_build_cmd(uint8_t *buf, uint16_t cmd, uint16_t rh_ticks, uint16_t t_ticks)
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
    if (rh < 0.0f) rh = 0.0f;
    if (rh > 100.0f) rh = 100.0f;
    return (uint16_t)(rh * 65535.0f / 100.0f);
}

static uint16_t temp_to_ticks(float t)
{
    if (isnan(t)) return SGP41_DEFAULT_T;
    return (uint16_t)((t + 45.0f) * 65535.0f / 175.0f);
}

esp_err_t sgp41_init(void)
{
    /* Execute conditioning : démarre le heater, retourne 1 mot (SRAW_VOC). */
    uint8_t cmd[8];
    sgp41_build_cmd(cmd, SGP41_CMD_CONDITIONING, SGP41_DEFAULT_RH, SGP41_DEFAULT_T);

    esp_err_t ret = i2c_master_write_to_device(
        SGP41_I2C_PORT, SGP41_I2C_ADDRESS, cmd, 8, SGP41_I2C_TIMEOUT);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "conditioning write failed: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    vTaskDelay(pdMS_TO_TICKS(SGP41_MEASURE_DELAY_MS));

    /* Lire 3 octets : SRAW_VOC(2) + CRC (on ne l'utilise pas, juste validation) */
    uint8_t buf[3] = {0};
    ret = i2c_master_read_from_device(
        SGP41_I2C_PORT, SGP41_I2C_ADDRESS, buf, sizeof(buf), SGP41_I2C_TIMEOUT);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "conditioning read failed: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    if (sgp41_crc8(&buf[0], 2) != buf[2])
    {
        ESP_LOGE(TAG, "conditioning CRC mismatch");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "SGP41 initialized (conditioning started, NOx valid after ~10 s)");
    return ESP_OK;
}

esp_err_t sgp41_read(sgp41_data_t *out, float temperature, float humidity)
{
    if (out == NULL) return ESP_ERR_INVALID_ARG;

    /* 1) Construire commande measure_raw_signals avec compensation T/RH */
    uint8_t cmd[8];
    sgp41_build_cmd(cmd, SGP41_CMD_MEASURE_RAW, rh_to_ticks(humidity), temp_to_ticks(temperature));

    esp_err_t ret = i2c_master_write_to_device(
        SGP41_I2C_PORT, SGP41_I2C_ADDRESS, cmd, 8, SGP41_I2C_TIMEOUT);
    if (ret != ESP_OK) return ESP_FAIL;

    /* 2) Attendre mesure (50 ms max) */
    vTaskDelay(pdMS_TO_TICKS(SGP41_MEASURE_DELAY_MS));

    /* 3) Lire 6 octets : SRAW_VOC(2)+CRC, SRAW_NOx(2)+CRC */
    uint8_t buf[6] = {0};
    ret = i2c_master_read_from_device(
        SGP41_I2C_PORT, SGP41_I2C_ADDRESS, buf, sizeof(buf), SGP41_I2C_TIMEOUT);
    if (ret != ESP_OK) return ESP_FAIL;

    /* 4) CRC */
    if (sgp41_crc8(&buf[0], 2) != buf[2] ||
        sgp41_crc8(&buf[3], 2) != buf[5])
    {
        ESP_LOGW(TAG, "CRC mismatch");
        return ESP_FAIL;
    }

    /* 5) Extraire valeurs brutes */
    out->sraw_voc = ((uint16_t)buf[0] << 8) | buf[1];
    out->sraw_nox = ((uint16_t)buf[3] << 8) | buf[4];

    return ESP_OK;
}
