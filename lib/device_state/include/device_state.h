/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#ifndef __DEVICE_STATE_H__
#define __DEVICE_STATE_H__

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Number of spectral channels measured by the AS7341
#define AS7341_CHANNELS 8

// AS7341 spectral data structure
typedef struct
{
    float channels[AS7341_CHANNELS];
} as7341_data_t;

typedef struct
{
    // -----------------------------
    // Sensor measurements
    // -----------------------------
    float co2_ppm;
    float temperature_scd40;
    float humidity_scd40;

    float temperature_c;
    float humidity_percent;

    float bmp280_pressure_hpa;
    float bmp280_temperature_c;

    float voc_index;
    float nox_index;

    float pm1_0_ug_m3;
    float pm2_5_ug_m3;
    float pm10_ug_m3;

    as7341_data_t light;

    float gas_level_raw;
    float gas_ppm;

    bool motion_detected;

    // -----------------------------
    // Audio state
    // -----------------------------
    float noise_db;

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
    TickType_t audio_last_update;

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
    bool audio_valid;

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
    bool audio_fault;

    // -----------------------------
    // System state
    // -----------------------------
    bool wifi_connected;
    bool mqtt_connected;
    bool degraded_mode;
    bool system_ok;

    // -----------------------------
    // Alarm state
    // -----------------------------
    bool alarm_active;
    bool gas_alarm;
    bool motion_alarm;

} device_state_t;

extern device_state_t g_device_state;
extern SemaphoreHandle_t g_device_state_mutex;

void device_state_init(void);

#endif // __DEVICE_STATE_H__