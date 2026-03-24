/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file system_task.h
 * @brief FreeRTOS system supervision task: health flags, alarms, sensor staleness thresholds.
 */

#ifndef SYSTEM_TASK_H
#define SYSTEM_TASK_H

#include <stdbool.h>

/**
 * @name Sensor and link supervision timeouts (milliseconds)
 * @{
 */
#define MOTION_TIMEOUT_MS 15000
#define GAS_TIMEOUT_MS 15000
#define SGP41_TIMEOUT_MS 15000
#define SHT41_TIMEOUT_MS 15000
#define BMP280_TIMEOUT_MS 15000
#define SCD40_TIMEOUT_MS 15000
#define PMS7003_TIMEOUT_MS 15000
#define AS7341_TIMEOUT_MS 15000
#define AUDIO_TIMEOUT_MS 15000
#define WIFI_TIMEOUT_MS 15000
#define MQTT_TIMEOUT_MS 15000
#define ALARM_TIMEOUT_MS 15000
#define DEGRADED_MODE_TIMEOUT_MS 15000
#define SYSTEM_OK_TIMEOUT_MS 15000
/** @} */

/** @brief Working copy of high-level system flags computed by @ref system_task. */
typedef struct
{
    bool degraded_mode;  /**< Set when supervision detects invalid/stale/faulty inputs. */
    bool motion_alarm;   /**< Motion-related alarm (policy-specific). */
    bool gas_alarm;      /**< Gas-related alarm (policy-specific). */
    bool alarm_active;   /**< Logical OR of active alarm lines. */
    bool system_ok;      /**< True when the device is not in degraded mode. */
} system_local_state_t;

/**
 * @brief FreeRTOS task entry: reads @c g_device_state, evaluates supervision rules, writes flags back.
 * @param pvParameters Unused (NULL).
 */
void system_task(void *pvParameters);

#endif