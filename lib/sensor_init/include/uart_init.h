/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file uart_init.h
 * @brief UART port used by the Plantower PMS7003 particulate sensor.
 */

#ifndef UART_INIT_H
#define UART_INIT_H

#include "esp_err.h"

/**
 * @brief Install UART driver and parameters for the PMS7003 link.
 * @return ESP_OK on success, or an ESP-IDF error code.
 */
esp_err_t uart_init_all(void);

/**
 * @brief Remove the UART driver (if installed) and reinstall it.
 * @return ESP_OK on success, or an ESP-IDF error code.
 */
esp_err_t uart_restore(void);

#endif /* UART_INIT_H */
