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
 *
 * Supervision applies only after at least one measurement timestamp was published for that
 * sensor (@c *_last_update != 0), so cold start does not trigger restores or false degradation.
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
    bool degraded_as312;  /**< True if AS312 is invalid, faulted, or stale. */
    bool degraded_mics5524;  /**< True if MiCS-5524 is invalid, faulted, or stale. */
    bool degraded_sht41;  /**< True if SHT41 is invalid, faulted, or stale. */
    bool degraded_sgp41;  /**< True if SGP41 is invalid, faulted, or stale. */
    bool degraded_bmp280;  /**< True if BMP280 is invalid, faulted, or stale. */
    bool degraded_scd40;  /**< True if SCD40 is invalid, faulted, or stale. */
    bool degraded_pms7003;  /**< True if PMS7003 is invalid, faulted, or stale. */
    bool degraded_as7341;  /**< True if AS7341 is invalid, faulted, or stale. */
    bool degraded_inmp441;  /**< True if INMP441 is invalid, faulted, or stale. */
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

/**
 * @brief Attempt hardware recovery for each sensor whose @c degraded_* flag is set.
 *
 * Uses @c xTaskGetTickCount() and enforces a minimum interval between restore bursts
 * to avoid hammering the bus while acquisition tasks may still be using drivers.
 *
 * On failure only, logs @c "sensor restore %%d: …" with a numeric id:
 * 1 AS312, 2 MiCS-5524, 3 SHT41, 4 SGP41, 5 BMP280, 6 SCD40, 7 UART (PMS path),
 * 8 PMS7003, 9 AS7341, 10 INMP441 / I2S.
 *
 * @param log_tag ESP-IDF log tag (e.g. task tag).
 * @param[in] local_state Same struct as for the @c *HealthCheck functions; only read here.
 */
void health_try_restore_sensors(const char *log_tag, const health_local_state_t *local_state);

#endif /* HEALTH_H */
