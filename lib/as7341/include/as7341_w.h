/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file as7341_w.h
 * @brief AMS OSRAM AS7341 8-channel spectrometer driver wrapper (I2C).
 */

#ifndef AS7341_W_H
#define AS7341_W_H

#include "esp_err.h"
#include "device_state.h"

/**
 * @brief Initialize the AS7341 spectrometer.
 *
 * Obtains the shared I2C bus handle from i2cdev and configures the
 * sensor with default parameters (gain 32×, ATIME 29, ASTEP 599).
 *
 * Must be called after at least one i2cdev-based sensor has been
 * initialized (so the I2C bus is already set up).
 */
esp_err_t as7341_w_init(void);

/**
 * @brief Delete the driver handle and re-run @ref as7341_w_init.
 * @return ESP_OK on success, or an ESP-IDF / driver error code.
 */
esp_err_t as7341_w_restore(void);

/**
 * @brief Read the 8 spectral channels (F1-F8) as basic counts.
 *
 * Triggers a measurement, waits for data ready, then reads and
 * converts to basic counts (gain-normalized).
 *
 * @param[out] out Pointer to as7341_data_t (channels[0..7] = F1..F8).
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if out is NULL
 *      - ESP_ERR_INVALID_STATE if not initialized
 *      - other error on I2C / timeout failure
 */
esp_err_t as7341_w_read(as7341_data_t *out);

#endif /* AS7341_W_H */
