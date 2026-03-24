/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file i2s_init.h
 * @brief I2S standard Philips RX channel for the INMP441 digital microphone.
 */

#ifndef I2S_INIT_H
#define I2S_INIT_H

#include "esp_err.h"
#include "driver/i2s_std.h"

/**
 * @brief Create and enable I2S0 RX (24-bit mono, sample rate from @c i2s_init.c).
 * @return ESP_OK on success, or an ESP-IDF error code.
 */
esp_err_t i2s_init_all(void);

/**
 * @brief RX channel handle for @c i2s_channel_read (NULL if @ref i2s_init_all not called).
 * @return ESP-IDF I2S RX channel handle.
 */
i2s_chan_handle_t i2s_get_rx_channel(void);

/**
 * @brief Disable and delete the RX channel, then re-run @ref i2s_init_all.
 * @return ESP_OK on success, or an ESP-IDF error code.
 */
esp_err_t i2s_restore(void);

#endif /* I2S_INIT_H */
