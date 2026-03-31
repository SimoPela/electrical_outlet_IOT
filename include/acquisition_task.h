/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file acquisition_task.h
 * @brief FreeRTOS acquisition task: local sensor snapshot type and task declaration.
 */

#ifndef __ACQUISITION_TASK_H__
#define __ACQUISITION_TASK_H__

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "device_state.h"

/**
 * @brief Task-local buffer for sensor reads before commit to @c g_device_state.
 *
 * Mirrors measurement, timestamp, validity, and fault fields needed for the acquisition loop.
 */
typedef struct
{
    bool  as312_motion_detected;
    float mics5524_gas_level_raw;
    float mics5524_gas_ppm;

    float voc_index;
    float nox_index;

    float temperature_c;
    float humidity_percent;
    
    float bmp280_pressure_hpa;
    float bmp280_temperature_c;

    float co2_ppm;
    float temperature_scd40;
    float humidity_scd40;

    float pm1_0_ug_m3;
    float pm2_5_ug_m3;
    float pm10_ug_m3;

    as7341_data_t light;

    // -----------------------------
    // Last update timestamps
    // -----------------------------
    TickType_t as312_last_update;
    TickType_t mics5524_last_update;
    TickType_t sgp41_last_update;
    TickType_t sht41_last_update;
    TickType_t bmp280_last_update;
    TickType_t scd40_last_update;
    TickType_t pms7003_last_update;
    TickType_t as7341_last_update;

    // -----------------------------
    // Validity flags
    // -----------------------------
    bool as312_valid;
    bool mics5524_valid;
    bool sgp41_valid;
    bool sht41_valid;
    bool bmp280_valid;
    bool scd40_valid;
    bool pms7003_valid;
    bool as7341_valid;

    // -----------------------------
    // Fault flags
    // -----------------------------
    bool as312_fault;
    bool mics5524_fault;
    bool sgp41_fault;
    bool sht41_fault;
    bool bmp280_fault;
    bool scd40_fault;
    bool pms7003_fault;
    bool as7341_fault;

} acquisition_local_state_t;

/**
 * @brief FreeRTOS task entry: polls sensors on a tick schedule and copies results to @c g_device_state.
 * @param pvParameters Unused (NULL).
 */
void acquisition_task(void *pvParameters);

#endif // __ACQUISITION_TASK_H__