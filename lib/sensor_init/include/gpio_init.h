/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file gpio_init.h
 * @brief Board GPIO setup (PMS7003 control lines, PIR, RGB LED, etc.).
 */

#ifndef GPIO_INIT_H
#define GPIO_INIT_H

#include "esp_err.h"

/**
 * @brief Configure GPIO directions and default levels per @c esp32_pinout.h.
 * @return ESP_OK on success, or an ESP-IDF error code.
 */
esp_err_t gpio_init_all(void);

#endif /* GPIO_INIT_H */
