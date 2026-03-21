/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef SGP41_H
#define SGP41_H

#include "esp_err.h"
#include <stdint.h>

typedef struct
{
    uint16_t sraw_voc; /* Raw VOC signal (higher = more VOC) */
    uint16_t sraw_nox; /* Raw NOx signal (higher = more NOx) */
} sgp41_data_t;

/**
 * @brief Execute the SGP41 conditioning command (heater warm-up).
 *
 * Must be called once after power-up. The sensor needs ~10 s of
 * conditioning before the NOx signal is valid.
 * During conditioning only sraw_voc is returned.
 *
 * @return ESP_OK on success, ESP_FAIL on I2C/CRC error.
 */
esp_err_t sgp41_init(void);

/**
 * @brief Measure raw VOC + NOx signals.
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
