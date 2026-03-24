/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "i2s_init.h"
#include "esp32_pinout.h"

#include "esp_log.h"
#include "esp_check.h"
#include "driver/i2s_std.h"
#include "driver/i2s_types.h"

static const char *TAG = "I2S_INIT";

#define MIC_SAMPLE_RATE_HZ 16000

static i2s_chan_handle_t s_rx_handle = NULL;

esp_err_t i2s_init_all(void)
{
    if (s_rx_handle != NULL) {
        return ESP_OK;
    }

    i2s_chan_config_t chan_cfg =
        I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);

    ESP_RETURN_ON_ERROR(
        i2s_new_channel(&chan_cfg, NULL, &s_rx_handle),
        TAG,
        "Failed to create I2S RX channel"
    );
 
    /*
     * INMP441 (DS-INMP441-00): I2S, 24-bit two's complement, MSB first with 1 SCK
     * delay vs half-frame (Philips I2S). Stereo frame = 64 SCK cycles (fSCK = 64 * fWS).
     *
     * ESP-IDF: with 24-bit slot data, mclk_multiple must be a multiple of 3
     * (see i2s_std.h / i2s_std_set_clock); default 256 is rejected.
     */
    i2s_std_clk_config_t clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(MIC_SAMPLE_RATE_HZ);
    clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_384;

    i2s_std_config_t std_cfg = {
        .clk_cfg = clk_cfg,
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
            I2S_DATA_BIT_WIDTH_24BIT,
            I2S_SLOT_MODE_MONO
        ),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = PIN_I2S_SCK,
            .ws   = PIN_I2S_WS,
            .dout = I2S_GPIO_UNUSED,
            .din  = PIN_I2S_SD,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
 
    ESP_RETURN_ON_ERROR(
        i2s_channel_init_std_mode(s_rx_handle, &std_cfg),
        TAG,
        "Failed to init I2S std mode"
    );

    ESP_RETURN_ON_ERROR(
        i2s_channel_enable(s_rx_handle),
        TAG,
        "Failed to enable I2S RX channel"
    );

    ESP_LOGI(TAG, "I2S RX ready (INMP441, %d Hz, Philips I2S 24-bit mono)", MIC_SAMPLE_RATE_HZ);
    return ESP_OK;
}

i2s_chan_handle_t i2s_get_rx_channel(void)
{
    return s_rx_handle;
}