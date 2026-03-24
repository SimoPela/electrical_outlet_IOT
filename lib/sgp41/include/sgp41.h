/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file sgp41.h
 * @brief Sensirion SGP41 VOC / NOx gas sensor with Sensirion Gas Index algorithm.
 */

#ifndef SGP41_H
#define SGP41_H

#include "esp_err.h"
#include <stdint.h>

/** @brief Processed indices and raw SRAW values from one measurement cycle. */
typedef struct
{
    int32_t  voc_index; /**< VOC index 0–500 (0 during ~45 s algorithm blackout; 100 = typical). */
    int32_t  nox_index; /**< NOx index 0–500 (0 during blackout; 1 = typical). */
    uint16_t sraw_voc;  /**< Raw VOC SRAW (diagnostics). */
    uint16_t sraw_nox;  /**< Raw NOx SRAW (diagnostics). */
} sgp41_data_t;

/**
 * @brief Initialize the SGP41 sensor and the Gas Index Algorithm.
 *
 * Executes the conditioning command (heater warm-up) and initializes
 * two instances of Sensirion's Gas Index Algorithm (VOC + NOx).
 * The sensor needs ~10 s before the NOx signal is valid; the algorithm
 * has a 45 s blackout period during which indices are 0.
 *
 * @return ESP_OK on success, ESP_FAIL on I2C/CRC error.
 */
esp_err_t sgp41_init(void);

/**
 * @brief Measure VOC / NOx indices.
 *
 * Reads raw SRAW signals, processes them through Sensirion's Gas Index
 * Algorithm, and returns both the computed indices (0-500) and raw values.
 *
 * Optionally pass ambient temperature and humidity for on-chip
 * compensation. Use NAN to skip compensation (defaults applied).
 *
 * @param[out] out         Pointer to the output structure.
 * @param      temperature Ambient temperature [°C] or NAN.
 * @param      humidity    Ambient humidity [%RH] or NAN.
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if out is NULL
 *      - ESP_FAIL on I2C or CRC error
 */
esp_err_t sgp41_read(sgp41_data_t *out, float temperature, float humidity);

#endif /* SGP41_H */
