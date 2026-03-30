/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file rgbled.h
 * @brief RGB LED helper.
 */

 #ifndef RGBLED_H
 #define RGBLED_H
 
 #include <stdbool.h>
 #include "esp_err.h"
 
 /**
  * @brief Initialize the PIR GPIO configured in board pinout.
  * @return ESP_OK on success, or an ESP-IDF error code.
  */
 esp_err_t rgbled_init(void);
 
 #endif /* RGBLED_H */
 