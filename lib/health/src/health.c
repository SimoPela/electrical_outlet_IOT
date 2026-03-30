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

esp_err_t as312HealthRestore(const char *TAG)
{
    ESP_LOGD(TAG, "Restoring AS312");
    ESP_RETURN_ON_ERROR(as312_restore(), TAG, "AS312 restore failed");
    return ESP_OK;
}

void mics5524HealthRestore(const char *TAG)
{
    ESP_LOGD(TAG, "Restoring MiCS-5524");
    //TODO: implement this
}

void sht41HealthRestore(const char *TAG)
{
    ESP_LOGD(TAG, "Restoring SHT41");
    //TODO: implement this
}

void sgp41HealthRestore(const char *TAG)
{
    ESP_LOGD(TAG, "Restoring SGP41");
    //TODO: implement this
}

void bmp280HealthRestore(const char *TAG)
{
    ESP_LOGD(TAG, "Restoring BMP280");
    //TODO: implement this
}

void scd40HealthRestore(const char *TAG)
{
    ESP_LOGD(TAG, "Restoring SCD40");
    //TODO: implement this
}

void pms7003HealthRestore(const char *TAG)
{
    ESP_LOGD(TAG, "Restoring PMS7003");
    //TODO: implement this
}

void as7341HealthRestore(const char *TAG)
{
    ESP_LOGD(TAG, "Restoring AS7341");
    //TODO: implement this
}

void inmp441HealthRestore(const char *TAG)
{
    ESP_LOGD(TAG, "Restoring INMP441");
    //TODO: implement this
}





void sensorHealthCheck(const char *TAG, TickType_t *now,
                       const device_state_t *state_copy, health_local_state_t *local_state)
{
    ESP_LOGD(TAG, "Starting health checks");
    as312HealthCheck(now, TAG, state_copy, local_state);
    mics5524HealthCheck(now, TAG, state_copy, local_state);
    sht41HealthCheck(now, TAG, state_copy, local_state);
    sgp41HealthCheck(now, TAG, state_copy, local_state);
    bmp280HealthCheck(now, TAG, state_copy, local_state);
    scd40HealthCheck(now, TAG, state_copy, local_state);
    pms7003HealthCheck(now, TAG, state_copy, local_state);
    as7341HealthCheck(now, TAG, state_copy, local_state);
    inmp441HealthCheck(now, TAG, state_copy, local_state);
    ESP_LOGD(TAG, "Health checks completed");
}

void sensorHealthRestore(const char *TAG, health_local_state_t *local_state)
{
    ESP_LOGD(TAG, "Starting sensor restore");
    if (local_state->degraded_as312) {  
        as312HealthRestore(TAG);
    }
    if (local_state->degraded_mics5524) {
        mics5524HealthRestore(TAG);
    }
    if (local_state->degraded_sht41) {
        sht41HealthRestore(TAG);
    }
    if (local_state->degraded_sgp41) {
        sgp41HealthRestore(TAG);
    }
    if (local_state->degraded_bmp280) {
        bmp280HealthRestore(TAG);
    }
    if (local_state->degraded_scd40) {
        scd40HealthRestore(TAG);
    }
    if (local_state->degraded_pms7003) {
        pms7003HealthRestore(TAG);
    }
    if (local_state->degraded_as7341) {
        as7341HealthRestore(TAG);
    }
    if (local_state->degraded_inmp441) {
        inmp441HealthRestore(TAG);
    }
    ESP_LOGD(TAG, "Sensor restore completed");
}