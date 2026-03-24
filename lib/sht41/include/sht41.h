/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file sht41.h
 * @brief Sensirion SHT41 humidity and temperature driver (I2C).
 */

#ifndef SHT41_H
#define SHT41_H

#include "esp_err.h"

/** @brief One high-precision measurement from SHT41. */
typedef struct
{
    float temperature_c;     /**< Temperature [°C]. */
    float humidity_percent; /**< Relative humidity [%RH]. */
} sht41_data_t;

/**
 * @brief Soft-reset the SHT41.
 *
 * @return ESP_OK on success, ESP_FAIL on I2C error.
 */
esp_err_t sht41_init(void);

/**
 * @brief Trigger a high-precision measurement and read the result.
 *
 * Blocks for ~10 ms while the sensor measures.
 *
 * @param[out] out  Pointer to the output structure.
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if out is NULL
 *      - ESP_FAIL on I2C or CRC error
 */
esp_err_t sht41_read(sht41_data_t *out);

#endif /* SHT41_H */
