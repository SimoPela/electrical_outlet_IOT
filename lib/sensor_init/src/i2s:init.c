/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "i2s_init.h"
#include "esp32_pinout.h"

#include "driver/i2s.h"
#include "esp_log.h"

static const char *TAG = "I2S_INIT";

#define MIC_I2S_PORT            I2S_NUM_0
#define MIC_SAMPLE_RATE_HZ      16000
#define MIC_DMA_BUF_COUNT       4
#define MIC_DMA_BUF_LEN         256

esp_err_t i2s_init_all(void)
{
    esp_err_t err;

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = MIC_SAMPLE_RATE_HZ,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = MIC_DMA_BUF_COUNT,
        .dma_buf_len = MIC_DMA_BUF_LEN,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = PIN_I2S_SCK,
        .ws_io_num = PIN_I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = PIN_I2S_SD
    };

    err = i2s_driver_install(MIC_I2S_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install I2S driver");
        return err;
    }

    err = i2s_set_pin(MIC_I2S_PORT, &pin_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure I2S pins");
        i2s_driver_uninstall(MIC_I2S_PORT);
        return err;
    }

    err = i2s_zero_dma_buffer(MIC_I2S_PORT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to clear I2S DMA buffer");
        i2s_driver_uninstall(MIC_I2S_PORT);
        return err;
    }

    ESP_LOGI(TAG, "I2S initialized for INMP441 (port=%d, %d Hz)",
             MIC_I2S_PORT, MIC_SAMPLE_RATE_HZ);

    return ESP_OK;
}

i2s_port_t i2s_get_port(void)
{
    return MIC_I2S_PORT;
}