/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file device_state.h
 * @brief Global device snapshot shared across FreeRTOS tasks (mutex-protected).
 */

#ifndef __DEVICE_STATE_H__
#define __DEVICE_STATE_H__

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/** @brief Number of spectral channels from the AS7341 (F1–F8). */
#define AS7341_CHANNELS 8

/** @brief AS7341 basic counts per channel. */
typedef struct
{
    float channels[AS7341_CHANNELS]; /**< Channel values @c [0..AS7341_CHANNELS-1]. */
} as7341_data_t;

/**
 * @brief Single source of truth for measurements, validity, faults, and system flags.
 *
 * Updated by @c acquisition_task and @c audio_task; read by @c system_task and @c comm_task.
 * All access must go through @c g_device_state_mutex unless documented otherwise.
 */
typedef struct
{
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

    float mics5524_gas_level_raw;
    float mics5524_gas_ppm;

    bool as312_motion_detected;

    // -----------------------------
    // Audio state
    // -----------------------------
    float noise_db;

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
    TickType_t inmp441_last_update;

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
    bool inmp441_valid;

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
    bool inmp441_fault;

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
    bool mics5524_alarm;
    bool as312_alarm;
    const char *co2_alarm_level;

} device_state_t;

/** @brief Global live device state. */
extern device_state_t g_device_state;

/** @brief Mutex guarding @c g_device_state (short critical sections). */
extern SemaphoreHandle_t g_device_state_mutex;

/**
 * @brief Mutex serializing sensor driver I/O and @c *_restore() (acquisition, audio INMP441, health).
 *
 * Prevents @c health_try_restore_sensors from tearing down I2C / I2S while other tasks use the same drivers.
 */
extern SemaphoreHandle_t g_sensor_driver_mutex;

/**
 * @brief Zero-initialize @c g_device_state and create @c g_device_state_mutex.
 *
 * Call once before any task uses the shared state.
 */
void device_state_init(void);

#endif // __DEVICE_STATE_H__