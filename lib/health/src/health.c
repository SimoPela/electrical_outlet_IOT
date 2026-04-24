/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file health.c
 * @brief Per-sensor health check implementations.
 *
 * Each @c *HealthCheck function tests the matching sensor's validity flag,
 * fault flag, and timestamp age.  A positive result sets @c degraded_mode
 * on @c health_local_state_t.  Checks are skipped before the sensor has
 * published its first timestamp (@c *_last_update == 0).
 *
 * The aggregate @ref sensorHealthCheck is skipped entirely before the system
 * has been running for @c ALL_SENSORS_TIMEOUT_MS to avoid false degradation
 * during cold-start stabilization.
 */

#include "health.h"

#include "as312.h"
#include "as7341_w.h"
#include "bmp.h"
#include "inmp441_w.h"
#include "mics5524.h"
#include "pms7003_w.h"
#include "scd40.h"
#include "sgp41.h"
#include "sht41.h"
#include "uart_init.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"

#include <stdbool.h>

#ifndef HEALTH_RESTORE_BURST_INTERVAL_MS
#define HEALTH_RESTORE_BURST_INTERVAL_MS 10000
#endif

/** @copydoc as312HealthCheck */
void as312HealthCheck(TickType_t *now, const char *TAG,
                      const device_state_t *state_copy, health_local_state_t *local_state)
{
    if (state_copy->as312_last_update == 0) {
        return;
    }
    if (!state_copy->as312_valid) {
        ESP_LOGD(TAG, "AS312: sensor is not valid");
        local_state->degraded_mode = true;
        local_state->degraded_as312 = true;
    }
    if (state_copy->as312_fault) {
        ESP_LOGD(TAG, "AS312: sensor is in fault");
        local_state->degraded_mode = true;
        local_state->degraded_as312 = true;
    }
    if ((*now - state_copy->as312_last_update) > pdMS_TO_TICKS(AS312_TIMEOUT_MS)) {
        ESP_LOGD(TAG, "AS312: sensor stopped updating within the expected timeout");
        local_state->degraded_mode = true;
        local_state->degraded_as312 = true;
    }
}

/** @copydoc mics5524HealthCheck */
void mics5524HealthCheck(TickType_t *now, const char *TAG,
                         const device_state_t *state_copy, health_local_state_t *local_state)
{
    if (!state_copy->mics5524_valid) {
        ESP_LOGD(TAG, "MiCS-5524: sensor is not valid");
        local_state->degraded_mode = true;
        local_state->degraded_mics5524 = true;
    }
    if (state_copy->mics5524_fault) {
        ESP_LOGD(TAG, "MiCS-5524: sensor is in fault");
        local_state->degraded_mode = true;
        local_state->degraded_mics5524 = true;
    }
    if ((*now - state_copy->mics5524_last_update) > pdMS_TO_TICKS(MICS5524_TIMEOUT_MS)) {
        ESP_LOGD(TAG, "MiCS-5524: sensor stopped updating within the expected timeout");
        local_state->degraded_mode = true;
        local_state->degraded_mics5524 = true;
    }
}

/** @copydoc sht41HealthCheck */
void sht41HealthCheck(TickType_t *now, const char *TAG,
                      const device_state_t *state_copy, health_local_state_t *local_state)
{
    if (state_copy->sht41_last_update == 0) {
        return;
    }
    if (!state_copy->sht41_valid) {
        ESP_LOGD(TAG, "SHT41: sensor is not valid");
        local_state->degraded_mode = true;
        local_state->degraded_sht41 = true;
    }
    if (state_copy->sht41_fault) {
        ESP_LOGD(TAG, "SHT41: sensor is in fault");
        local_state->degraded_mode = true;
        local_state->degraded_sht41 = true;
    }
    if ((*now - state_copy->sht41_last_update) > pdMS_TO_TICKS(SHT41_TIMEOUT_MS)) {
        ESP_LOGD(TAG, "SHT41: sensor stopped updating within the expected timeout");
        local_state->degraded_mode = true;
        local_state->degraded_sht41 = true;
    }
}

/** @copydoc sgp41HealthCheck */
void sgp41HealthCheck(TickType_t *now, const char *TAG,
                      const device_state_t *state_copy, health_local_state_t *local_state)
{
    if (state_copy->sgp41_last_update == 0) {
        return;
    }
    if (!state_copy->sgp41_valid) {
        ESP_LOGD(TAG, "SGP41: sensor is not valid");
        local_state->degraded_mode = true;
        local_state->degraded_sgp41 = true;
    }
    if (state_copy->sgp41_fault) {
        ESP_LOGD(TAG, "SGP41: sensor is in fault");
        local_state->degraded_mode = true;
        local_state->degraded_sgp41 = true;
    }
    if ((*now - state_copy->sgp41_last_update) > pdMS_TO_TICKS(SGP41_TIMEOUT_MS)) {
        ESP_LOGD(TAG, "SGP41: sensor stopped updating within the expected timeout");
        local_state->degraded_mode = true;
        local_state->degraded_sgp41 = true;
    }
}

/** @copydoc bmp280HealthCheck */
void bmp280HealthCheck(TickType_t *now, const char *TAG,
                       const device_state_t *state_copy, health_local_state_t *local_state)
{
    if (state_copy->bmp280_last_update == 0) {
        return;
    }
    if (!state_copy->bmp280_valid) {
        ESP_LOGD(TAG, "BMP280: sensor is not valid");
        local_state->degraded_mode = true;
        local_state->degraded_bmp280 = true;
    }
    if (state_copy->bmp280_fault) {
        ESP_LOGD(TAG, "BMP280: sensor is in fault");
        local_state->degraded_mode = true;
        local_state->degraded_bmp280 = true;
    }
    if ((*now - state_copy->bmp280_last_update) > pdMS_TO_TICKS(BMP280_TIMEOUT_MS)) {
        ESP_LOGD(TAG, "BMP280: sensor stopped updating within the expected timeout");
        local_state->degraded_mode = true;
        local_state->degraded_bmp280 = true;
    }
}

/** @copydoc scd40HealthCheck */
void scd40HealthCheck(TickType_t *now, const char *TAG,
                      const device_state_t *state_copy, health_local_state_t *local_state)
{
    if (state_copy->scd40_last_update == 0) {
        return;
    }
    if (!state_copy->scd40_valid) {
        ESP_LOGD(TAG, "SCD40: sensor is not valid");
        local_state->degraded_mode = true;
        local_state->degraded_scd40 = true;
    }
    if (state_copy->scd40_fault) {
        ESP_LOGD(TAG, "SCD40: sensor is in fault");
        local_state->degraded_mode = true;
        local_state->degraded_scd40 = true;
    }
    if ((*now - state_copy->scd40_last_update) > pdMS_TO_TICKS(SCD40_TIMEOUT_MS)) {
        ESP_LOGD(TAG, "SCD40: sensor stopped updating within the expected timeout");
        local_state->degraded_mode = true;
        local_state->degraded_scd40 = true;
    }
}

/** @copydoc pms7003HealthCheck */
void pms7003HealthCheck(TickType_t *now, const char *TAG,
                        const device_state_t *state_copy, health_local_state_t *local_state)
{
    if (!state_copy->pms7003_valid) {
        ESP_LOGD(TAG, "PMS7003: sensor is not valid");
        local_state->degraded_mode = true;
        local_state->degraded_pms7003 = true;
    }
    if (state_copy->pms7003_fault) {
        ESP_LOGD(TAG, "PMS7003: sensor is in fault");
        local_state->degraded_mode = true;
        local_state->degraded_pms7003 = true;
    }
    if ((*now - state_copy->pms7003_last_update) > pdMS_TO_TICKS(PMS7003_TIMEOUT_MS)) {
        ESP_LOGD(TAG, "PMS7003: sensor stopped updating within the expected timeout");
        local_state->degraded_mode = true;
        local_state->degraded_pms7003 = true;
    }
}

/** @copydoc as7341HealthCheck */
void as7341HealthCheck(TickType_t *now, const char *TAG,
                       const device_state_t *state_copy, health_local_state_t *local_state)
{
    if (state_copy->as7341_last_update == 0) {
        return;
    }
    if (!state_copy->as7341_valid) {
        ESP_LOGD(TAG, "AS7341: sensor is not valid");
        local_state->degraded_mode = true;
        local_state->degraded_as7341 = true;
    }
    if (state_copy->as7341_fault) {
        ESP_LOGD(TAG, "AS7341: sensor is in fault");
        local_state->degraded_mode = true;
        local_state->degraded_as7341 = true;
    }
    if ((*now - state_copy->as7341_last_update) > pdMS_TO_TICKS(AS7341_TIMEOUT_MS)) {
        ESP_LOGD(TAG, "AS7341: sensor stopped updating within the expected timeout");
        local_state->degraded_mode = true;
        local_state->degraded_as7341 = true;
    }
}

/** @copydoc inmp441HealthCheck */
void inmp441HealthCheck(TickType_t *now, const char *TAG,
                        const device_state_t *state_copy, health_local_state_t *local_state)
{
    if (state_copy->inmp441_last_update == 0) {
        return;
    }
    if (!state_copy->inmp441_valid) {
        ESP_LOGD(TAG, "INMP441: sensor is not valid");
        local_state->degraded_mode = true;
        local_state->degraded_inmp441 = true;
    }
    if (state_copy->inmp441_fault) {
        ESP_LOGD(TAG, "INMP441: sensor is in fault");
        local_state->degraded_mode = true;
        local_state->degraded_inmp441 = true;
    }
    if ((*now - state_copy->inmp441_last_update) > pdMS_TO_TICKS(INMP441_TIMEOUT_MS)) {
        ESP_LOGD(TAG, "INMP441: sensor stopped updating within the expected timeout");
        local_state->degraded_mode = true;
        local_state->degraded_inmp441 = true;
    }
}

/** @copydoc sensorHealthCheck */
void sensorHealthCheck(const char *TAG, TickType_t *now,
                       const device_state_t *state_copy, health_local_state_t *local_state)
{
    if(*now < pdMS_TO_TICKS(ALL_SENSORS_TIMEOUT_MS)) {
        return;
    }
    as312HealthCheck(now, TAG, state_copy, local_state);
    mics5524HealthCheck(now, TAG, state_copy, local_state);
    sht41HealthCheck(now, TAG, state_copy, local_state);
    sgp41HealthCheck(now, TAG, state_copy, local_state);
    bmp280HealthCheck(now, TAG, state_copy, local_state);
    scd40HealthCheck(now, TAG, state_copy, local_state);
    pms7003HealthCheck(now, TAG, state_copy, local_state);
    as7341HealthCheck(now, TAG, state_copy, local_state);
    inmp441HealthCheck(now, TAG, state_copy, local_state);
}

/** @copydoc systemHealthCheck */
void systemHealthCheck(const char *TAG, health_local_state_t *local_state)
{
    (void)TAG;
    local_state->system_ok = !local_state->degraded_mode;
}
