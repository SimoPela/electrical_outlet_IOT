/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file adc_init.h
 * @brief ESP-IDF ADC oneshot unit for analog sensors (e.g. MiCS-5524).
 */

#ifndef ADC_INIT_H
#define ADC_INIT_H

#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"

/**
 * @brief Create and configure the shared ADC1 oneshot unit.
 * @return ESP_OK on success, or an ESP-IDF error code.
 */
esp_err_t adc_init_all(void);

/**
 * @brief Return the global ADC oneshot handle (valid after @ref adc_init_all).
 * @return Handle owned by the sensor_init layer; do not deinit from application code.
 */
adc_oneshot_unit_handle_t adc_get_handle(void);

/**
 * @brief Tear down the oneshot unit and re-run @ref adc_init_all (recovery path).
 * @return ESP_OK on success, or an ESP-IDF error code.
 */
esp_err_t adc_restore(void);

#endif /* ADC_INIT_H */
