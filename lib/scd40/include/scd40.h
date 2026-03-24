/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#ifndef SCD40_H
#define SCD40_H

#include "esp_err.h"
#include <stdbool.h>

typedef struct
{
    float co2_ppm;
    float temperature_c;
    float humidity_percent;
} scd40_data_t;

/**
 * @brief Initialize the SCD40 sensor and start periodic measurement.
 *
 * Call once at startup.
 */
esp_err_t scd40_init(void);

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