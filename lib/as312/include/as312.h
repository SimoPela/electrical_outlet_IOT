/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file as312.h
 * @brief AS312 PIR motion sensor digital input read helper.
 */

#ifndef AS312_H
#define AS312_H

#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief Sample the PIR GPIO configured in board pinout.
 * @return true if motion is asserted by the sensor, false otherwise.
 */
bool as312_read_motion(void);

/**
 * @brief Re-apply PIR GPIO configuration via @ref gpio_init_all.
 * @return ESP_OK on success, or an ESP-IDF error code.
 */
esp_err_t as312_restore(void);

#endif /* AS312_H */
