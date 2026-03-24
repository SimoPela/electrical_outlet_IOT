/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file inmp441_w.h
 * @brief TDK InvenSense INMP441 I2S MEMS microphone — SPL estimate wrapper.
 */

#ifndef INMP441_W_H
#define INMP441_W_H

#include "esp_err.h"

/** @brief Output of one microphone read (see @c inmp441_w.c for DSP and calibration). */
typedef struct
{
    /** @brief Estimated sound pressure level [dB]; datasheet mapping + @c INMP441_SPL_OFFSET_DB. */
    float noise_db;
} inmp441_data_t;

/**
 * @brief Ensure I2S RX is initialized (idempotent; shared with @c sensor_init).
 * @return ESP_OK on success, or an ESP-IDF error code.
 */
esp_err_t inmp441_w_init(void);

/**
 * @brief DMA-read one audio block, compute AC RMS, map to dB SPL.
 *
 * @param[out] out Filled on success; @c noise_db clamped to a safe display range in implementation.
 * @return ESP_OK on success, @c ESP_ERR_INVALID_ARG if @p out is NULL, or I2S error.
 */
esp_err_t inmp441_w_read(inmp441_data_t *out);

#endif /* INMP441_W_H */
