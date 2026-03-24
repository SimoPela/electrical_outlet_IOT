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

#include <stdbool.h>

#ifndef HEALTH_RESTORE_BURST_INTERVAL_MS
#define HEALTH_RESTORE_BURST_INTERVAL_MS 10000
#endif

static TickType_t s_last_restore_burst_tick;

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

void health_try_restore_sensors(const char *log_tag, const health_local_state_t *local_state)
{
    if (local_state == NULL || g_sensor_driver_mutex == NULL) {
        return;
    }

    TickType_t now = xTaskGetTickCount();
    if (s_last_restore_burst_tick != 0 &&
        (now - s_last_restore_burst_tick) < pdMS_TO_TICKS(HEALTH_RESTORE_BURST_INTERVAL_MS)) {
        return;
    }

    if (xSemaphoreTake(g_sensor_driver_mutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
        ESP_LOGW(log_tag, "sensor restore skipped (bus busy)");
        return;
    }

    bool attempted = false;
    esp_err_t err;

#define HR_OK(code, call) \
    do { \
        err = (call); \
        if (err != ESP_OK) { \
            ESP_LOGW(log_tag, "sensor restore %d: %s", (code), esp_err_to_name(err)); \
        } \
        attempted = true; \
    } while (0)

    if (local_state->degraded_as312) {
        HR_OK(1, as312_restore());
    }
    if (local_state->degraded_mics5524) {
        HR_OK(2, mics5524_restore(true));
    }
    if (local_state->degraded_sht41) {
        HR_OK(3, sht41_restore());
    }
    if (local_state->degraded_sgp41) {
        HR_OK(4, sgp41_restore());
    }
    if (local_state->degraded_bmp280) {
        HR_OK(5, bmp_restore());
    }
    if (local_state->degraded_scd40) {
        HR_OK(6, scd40_restore());
    }
    if (local_state->degraded_pms7003) {
        err = uart_restore();
        if (err != ESP_OK) {
            ESP_LOGW(log_tag, "sensor restore %d: %s", 7, esp_err_to_name(err));
        }
        HR_OK(8, pms7003_w_restore());
    }
    if (local_state->degraded_as7341) {
        HR_OK(9, as7341_w_restore());
    }
    if (local_state->degraded_inmp441) {
        HR_OK(10, inmp441_w_restore());
    }
#undef HR_OK

    if (attempted) {
        s_last_restore_burst_tick = now;
    }

    xSemaphoreGive(g_sensor_driver_mutex);
}
