/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file health.h
 * @brief Per-sensor supervision: validity, fault, and stale-data timeouts into @c health_local_state_t.
 *
 * Used by @c system_task with a snapshot of @c g_device_state. Each @c *HealthCheck may set
 * @c degraded_mode on the output struct; it does not clear flags set by other checks.
 */

#ifndef HEALTH_H
#define HEALTH_H

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "device_state.h"

/**
 * @name Sensor supervision timeouts (milliseconds)
 * @{
 */
#define AS312_TIMEOUT_MS      15000
#define MICS5524_TIMEOUT_MS   15000
#define SGP41_TIMEOUT_MS      15000
#define SHT41_TIMEOUT_MS      15000
#define BMP280_TIMEOUT_MS     15000
#define SCD40_TIMEOUT_MS      15000
#define PMS7003_TIMEOUT_MS    15000
#define AS7341_TIMEOUT_MS     15000
#define INMP441_TIMEOUT_MS    15000
/** @} */

/**
 * @brief Working set of system-level flags derived from sensor health (written by @c system_task).
 */
typedef struct
{
    bool degraded_mode;   /**< True if any supervised sensor is invalid, faulted, or stale. */
    bool as312_alarm;     /**< AS312 / motion alarm line (policy). */
    bool mics5524_alarm;  /**< Gas / MiCS alarm line (policy). */
    bool alarm_active;    /**< True if any alarm line is active. */
    bool system_ok;       /**< Typically @c !degraded_mode after policy merge. */
} health_local_state_t;

/**
 * @brief AS312: invalid, fault, or no update within @ref AS312_TIMEOUT_MS.
 * @param[in,out] now Current tick; unchanged (passed for uniform API).
 * @param[in] TAG ESP-IDF log tag for debug messages.
 * @param[in] state_copy Read-only snapshot of device state.
 * @param[in,out] local_state @c degraded_mode may be set to true.
 */
void as312HealthCheck(TickType_t *now, const char *TAG,
                      const device_state_t *state_copy, health_local_state_t *local_state);

/**
 * @brief MiCS-5524: invalid, fault, or stale ( @ref MICS5524_TIMEOUT_MS ).
 * @param[in,out] now Current tick (unused).
 * @param[in] TAG Log tag.
 * @param[in] state_copy Device snapshot.
 * @param[in,out] local_state Output flags.
 */
void mics5524HealthCheck(TickType_t *now, const char *TAG,
                         const device_state_t *state_copy, health_local_state_t *local_state);

/**
 * @brief SHT41: invalid, fault, or stale ( @ref SHT41_TIMEOUT_MS ).
 * @param[in,out] now Current tick (unused).
 * @param[in] TAG Log tag.
 * @param[in] state_copy Device snapshot.
 * @param[in,out] local_state @c degraded_mode may be set.
 */
void sht41HealthCheck(TickType_t *now, const char *TAG,
                      const device_state_t *state_copy, health_local_state_t *local_state);

/**
 * @brief SGP41: invalid, fault, or stale ( @ref SGP41_TIMEOUT_MS ).
 * @param[in,out] now Current tick (unused).
 * @param[in] TAG Log tag.
 * @param[in] state_copy Device snapshot.
 * @param[in,out] local_state @c degraded_mode may be set.
 */
void sgp41HealthCheck(TickType_t *now, const char *TAG,
                      const device_state_t *state_copy, health_local_state_t *local_state);

/**
 * @brief BMP280: invalid, fault, or stale ( @ref BMP280_TIMEOUT_MS ).
 * @param[in,out] now Current tick (unused).
 * @param[in] TAG Log tag.
 * @param[in] state_copy Device snapshot.
 * @param[in,out] local_state @c degraded_mode may be set.
 */
void bmp280HealthCheck(TickType_t *now, const char *TAG,
                       const device_state_t *state_copy, health_local_state_t *local_state);

/**
 * @brief SCD40: invalid, fault, or stale ( @ref SCD40_TIMEOUT_MS ).
 * @param[in,out] now Current tick (unused).
 * @param[in] TAG Log tag.
 * @param[in] state_copy Device snapshot.
 * @param[in,out] local_state @c degraded_mode may be set.
 */
void scd40HealthCheck(TickType_t *now, const char *TAG,
                      const device_state_t *state_copy, health_local_state_t *local_state);

/**
 * @brief PMS7003: invalid, fault, or stale ( @ref PMS7003_TIMEOUT_MS ).
 * @param[in,out] now Current tick (unused).
 * @param[in] TAG Log tag.
 * @param[in] state_copy Device snapshot.
 * @param[in,out] local_state @c degraded_mode may be set.
 */
void pms7003HealthCheck(TickType_t *now, const char *TAG,
                        const device_state_t *state_copy, health_local_state_t *local_state);

/**
 * @brief AS7341: invalid, fault, or stale ( @ref AS7341_TIMEOUT_MS ).
 * @param[in,out] now Current tick (unused).
 * @param[in] TAG Log tag.
 * @param[in] state_copy Device snapshot.
 * @param[in,out] local_state @c degraded_mode may be set.
 */
void as7341HealthCheck(TickType_t *now, const char *TAG,
                       const device_state_t *state_copy, health_local_state_t *local_state);

/**
 * @brief INMP441 / audio path: invalid, fault, or stale ( @ref INMP441_TIMEOUT_MS ).
 * @param[in,out] now Current tick (unused).
 * @param[in] TAG Log tag.
 * @param[in] state_copy Device snapshot (@c inmp441_* fields from @c audio_task).
 * @param[in,out] local_state @c degraded_mode may be set.
 *
 * @note Independent of MiCS-5524; uses @c inmp441_valid, @c inmp441_fault, @c inmp441_last_update.
 */
void inmp441HealthCheck(TickType_t *now, const char *TAG,
                        const device_state_t *state_copy, health_local_state_t *local_state);

#endif /* HEALTH_H */
