/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef BMP_H
#define BMP_H

#include "esp_err.h"

typedef struct
{
    float pressure_hpa;    /* Pressure [hPa]  */
    float temperature_c;   /* Temperature [°C] */
} bmp_data_t;

/**
 * @brief Initialize the BMP280 sensor (normal mode, 4× oversampling).
 *
 * Call once at startup.
 */
esp_err_t bmp_init(void);

/**
 * @brief Read pressure and temperature from the BMP280.
 *
 * @param[out] out Pointer to the output structure.
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if out is NULL
 *      - ESP_ERR_INVALID_STATE if not initialized
 *      - other error on I2C failure
 */
esp_err_t bmp_read(bmp_data_t *out);

#endif /* BMP_H */
