/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file sensor_init.h
 * @brief One-shot initialization of GPIO, ADC, UART, I2S, and environmental sensor drivers.
 */

#ifndef SENSOR_INIT_H
#define SENSOR_INIT_H

#include "esp_err.h"

/**
 * @brief Initialize board peripherals and all sensors (best-effort where optional).
 *
 * Order: GPIO, ADC, I2S, UART, MiCS-5524, @c i2cdev / I2C, then I2C sensors.
 *
 * @return ESP_OK if core bring-up succeeded; sensor-specific failures may be logged as warnings.
 */
esp_err_t sensor_init_all(void);

#endif /* SENSOR_INIT_H */
