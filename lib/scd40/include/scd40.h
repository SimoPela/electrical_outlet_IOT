/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file scd40.h
 * @brief Driver API for the Sensirion SCD40 CO₂ / RH / T sensor (I2C).
 */

#ifndef SCD40_H
#define SCD40_H

#include "esp_err.h"
#include <stdbool.h>

/** @brief One sample from the SCD40 periodic measurement. */
typedef struct
{
    float co2_ppm;           /**< CO₂ concentration [ppm]. */
    float temperature_c;   /**< Compensated temperature [°C]. */
    float humidity_percent; /**< Compensated relative humidity [%RH]. */
} scd40_data_t;

/**
 * @brief Initialize the SCD40 sensor and start periodic measurement.
 *
 * Call once at startup.
 */
esp_err_t scd40_init(void);

/**
 * @brief Stop periodic mode (best effort), remove the I2C device, and re-run @ref scd40_init.
 * @return ESP_OK on success, or an ESP-IDF error code.
 */
esp_err_t scd40_restore(void);

/**
 * @brief Read a new SCD40 sample if available.
 *
 * @param[out] out Pointer to output struct
 * @return
 *   - ESP_OK if a new sample was read
 *   - ESP_ERR_NOT_FINISHED if no new sample is ready yet
 *   - other ESP-IDF error code on failure
 */
esp_err_t scd40_read(scd40_data_t *out);

#endif /* SCD40_H */