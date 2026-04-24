/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file rgbled.h
 * @brief RGB status LED helper (GPIO output).
 */

#ifndef RGBLED_H
#define RGBLED_H

#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief Configure the three RGB LED GPIO pins as outputs and drive them low.
 * @return ESP_OK on success, or an ESP-IDF error code.
 */
esp_err_t rgbled_init(void);

#endif /* RGBLED_H */
