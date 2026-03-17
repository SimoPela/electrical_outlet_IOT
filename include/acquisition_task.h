/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#ifndef __ACQUISITION_TASK_H__
#define __ACQUISITION_TASK_H__

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "device_state.h"

typedef struct
{
    // -----------------------------
    // Sensor measurements
    // -----------------------------
    bool motion_detected;
    float gas_level_raw;

    float voc_index;
    float nox_index;

    float temperature_c;
    float humidity_percent;
    float pressure_hpa;

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
    TickType_t motion_last_update;
    TickType_t gas_last_update;
    TickType_t sgp41_last_update;
    TickType_t sht41_last_update;
    TickType_t bmp280_last_update;
    TickType_t scd40_last_update;
    TickType_t pms7003_last_update;
    TickType_t as7341_last_update;

    // -----------------------------
    // Validity flags
    // -----------------------------
    bool motion_valid;
    bool gas_valid;
    bool sgp41_valid;
    bool sht41_valid;
    bool bmp280_valid;
    bool scd40_valid;
    bool pms7003_valid;
    bool as7341_valid;

    // -----------------------------
    // Fault flags
    // -----------------------------
    bool motion_fault;
    bool gas_fault;
    bool sgp41_fault;
    bool sht41_fault;
    bool bmp280_fault;
    bool scd40_fault;
    bool pms7003_fault;
    bool as7341_fault;

} acquisition_local_state_t;

void acquisition_task(void *pvParameters);

#endif // __ACQUISITION_TASK_H__