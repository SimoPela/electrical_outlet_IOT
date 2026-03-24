/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#ifndef MICS5524_H
#define MICS5524_H

#include "esp_err.h"

/**
 * @brief Initializes the MICS5524 module (ADC calibration).
 *        Call once before any reading.
 */
esp_err_t mics5524_init(void);

/**
 * @brief Reads the average voltage on the sensor's ADC pin.
 * @return Voltage in Volts [0.0 .. 3.3], or -1.0f in case of error.
 */
float mics5524_read_voltage(void);

/**
 * @brief Estimates CO concentration in ppm from the Rs/R0 curve.
 * @return Estimated ppm, or -1.0f in case of error.
 * @note  R0 must be calibrated in clean air in the .c source file.
 */
float mics5524_read_ppm(void);

#endif /* MICS5524_H */