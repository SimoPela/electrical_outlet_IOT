/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "i2c_init.h"
#include "esp32_pinout.h"

#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG = "I2C_INIT";

#define I2C_MASTER_PORT         I2C_NUM_0
/* SCD4x : fréquence SCL max. 100 kHz (CD_DS_SCD40_SCD41_Datasheet_D1, Table 7). */
#define I2C_MASTER_FREQ_HZ      100000

esp_err_t i2c_init_all(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PIN_I2C_SDA,
        .scl_io_num = PIN_I2C_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };

    esp_err_t err = i2c_param_config(I2C_MASTER_PORT, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure I2C");
        return err;
    }

    err = i2c_driver_install(I2C_MASTER_PORT, conf.mode, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install I2C driver");
        return err;
    }

    ESP_LOGI(TAG, "I2C initialized");
    return ESP_OK;
}