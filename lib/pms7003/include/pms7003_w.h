/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file pms7003_w.h
 * @brief Project wrapper around @c pms7003.h: init/read API and atmospheric PM values.
 */

#ifndef PMS7003_W_H
#define PMS7003_W_H

#include "esp_err.h"

/** @brief PM mass concentrations (atmospheric environment fields from PMS frame). */
typedef struct
{
    float pm1_0_ug_m3;  /* PM1.0  atmospheric concentration [µg/m³] */
    float pm2_5_ug_m3;  /* PM2.5  atmospheric concentration [µg/m³] */
    float pm10_ug_m3;   /* PM10   atmospheric concentration [µg/m³] */
} pms7003_data_t;

/**
 * @brief Initialize the PMS7003 sensor.
 *
 * Configures the SET/RESET GPIOs, starts the internal state machine
 * and triggers a reset.  The sensor needs ~30 s to stabilize after
 * power-on; reads attempted before that return ESP_ERR_NOT_FINISHED.
 *
 * UART must already be initialized (uart_init_all).
 */
esp_err_t pms7003_w_init(void);

/**
 * @brief Read the latest PM data from the PMS7003.
 *
 * Reads any available UART data, finds the most recent valid 32-byte
 * frame, and extracts PM1.0, PM2.5 and PM10 atmospheric values.
 *
 * @param[out] out  Destination structure.
 * @return
 *      - ESP_OK              on success
 *      - ESP_ERR_INVALID_ARG if out is NULL
 *      - ESP_ERR_INVALID_STATE if not initialized
 *      - ESP_ERR_NOT_FINISHED if sensor is still stabilizing or
 *        no valid frame is available yet
 */
esp_err_t pms7003_w_read(pms7003_data_t *out);

#endif /* PMS7003_W_H */
