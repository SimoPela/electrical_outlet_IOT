/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file mics5524.h
 * @brief MiCS-5524 combustible gas sensor: ADC voltage and heuristic CO ppm estimate.
 */

#ifndef MICS5524_H
#define MICS5524_H

#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief Calibrate ADC curve and prepare MiCS channel.
 * @return ESP_OK on success, or an ESP-IDF error code.
 */
esp_err_t mics5524_init(void);

/**
 * @brief MiCS / ADC recovery: L1 recalibration only, L2 full ADC unit reset.
 * @return ESP_OK on success, or an ESP-IDF error code.
 */
esp_err_t mics5524_restore(void);

/**
 * @brief Average several ADC samples into a rail-referred voltage.
 * @return Voltage [V] in typical 0…3.3 range, or @c -1.0f on error.
 */
float mics5524_read_voltage(void);

/**
 * @brief Map sensor resistance ratio to an estimated CO ppm (uncalibrated baseline in @c .c).
 * @return Estimated ppm, or @c -1.0f on error.
 * @note Requires clean-air @c R0 calibration in the implementation for meaningful absolute ppm.
 */
float mics5524_read_ppm(void);

#endif /* MICS5524_H */
