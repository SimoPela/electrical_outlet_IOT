/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file alarm.h
 * @brief Alarm policy layer: per-sensor alarm logic and CO₂ level classification.
 *
 * All functions operate on a read-only @c device_state_t snapshot and write
 * results into @c alarm_local_state_t.  The aggregate entry point is
 * @ref systemAlarmLogic, called once per @c system_task iteration.
 */

#ifndef ALARM_H
#define ALARM_H

#include "health.h"

/** @brief Gas concentration threshold above which the MiCS-5524 alarm activates [ppm]. */
#define MICS5524_ALARM_THRESHOLD 5000

/**
 * @brief Working set of alarm flags derived from sensor readings (written by @c system_task).
 */
typedef struct
{
    bool as312_alarm;            /**< Motion alarm (AS312 PIR detected). */
    bool mics5524_alarm;         /**< Gas alarm (MiCS-5524 CO above threshold). */
    const char *co2_alarm_level; /**< CO₂ air-quality label; one of the @c CO2_LEVEL_* strings. */
} alarm_local_state_t;

/**
 * @name CO₂ air-quality labels (IDA / ASHRAE 62.1 inspired thresholds)
 * @{
 */
#define CO2_LEVEL_OPTIMAL    "optimal"    /**< < 400 ppm  */
#define CO2_LEVEL_GOOD       "good"       /**< 400–800 ppm */
#define CO2_LEVEL_MODERATE   "moderate"   /**< 800–1200 ppm */
#define CO2_LEVEL_POOR       "poor"       /**< 1200–1800 ppm */
#define CO2_LEVEL_VERY_POOR  "very poor"  /**< 1800–5000 ppm */
#define CO2_LEVEL_CRITICAL   "critical"   /**< > 5000 ppm */
/** @} */

/**
 * @brief Evaluate all alarm policies and populate @p local_state.
 *
 * Runs AS312, MiCS-5524 and SCD40 alarm sub-checks in order.
 *
 * @param[in]  TAG        ESP-IDF log tag for debug/warning messages.
 * @param[in]  state_copy Read-only snapshot of the current device state.
 * @param[out] local_state Alarm flags to be written back to @c g_device_state.
 */
void systemAlarmLogic(const char *TAG, const device_state_t *state_copy, alarm_local_state_t *local_state);

#endif /* ALARM_H */
