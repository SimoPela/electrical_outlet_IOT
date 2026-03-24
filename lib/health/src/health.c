#include "health.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void as312HealthCheck(TickType_t *now, const char *TAG,
                      const device_state_t *state_copy, health_local_state_t *local_state)
{
    if (!state_copy->as312_valid) {
        ESP_LOGD(TAG, "AS312: sensor is not valid");
        local_state->degraded_mode = true;
    }
    if (state_copy->as312_fault) {
        ESP_LOGD(TAG, "AS312: sensor is in fault");
        local_state->degraded_mode = true;
    }
    if ((*now - state_copy->as312_last_update) > pdMS_TO_TICKS(AS312_TIMEOUT_MS)) {
        ESP_LOGD(TAG, "AS312: sensor stopped updating within the expected timeout");
        local_state->degraded_mode = true;
    }
}

void mics5524HealthCheck(TickType_t *now, const char *TAG,
                         const device_state_t *state_copy, health_local_state_t *local_state)
{
    if (!state_copy->mics5524_valid) {
        ESP_LOGD(TAG, "MiCS-5524: sensor is not valid");
        local_state->degraded_mode = true;
    }
    if (state_copy->mics5524_fault) {
        ESP_LOGD(TAG, "MiCS-5524: sensor is in fault");
        local_state->degraded_mode = true;
    }
    if ((*now - state_copy->mics5524_last_update) > pdMS_TO_TICKS(MICS5524_TIMEOUT_MS)) {
        ESP_LOGD(TAG, "MiCS-5524: sensor stopped updating within the expected timeout");
        local_state->degraded_mode = true;
    }
}

void sht41HealthCheck(TickType_t *now, const char *TAG,
                      const device_state_t *state_copy, health_local_state_t *local_state)
{
    if (!state_copy->sht41_valid) {
        ESP_LOGD(TAG, "SHT41: sensor is not valid");
        local_state->degraded_mode = true;
    }
    if (state_copy->sht41_fault) {
        ESP_LOGD(TAG, "SHT41: sensor is in fault");
        local_state->degraded_mode = true;
    }
    if ((*now - state_copy->sht41_last_update) > pdMS_TO_TICKS(SHT41_TIMEOUT_MS)) {
        ESP_LOGD(TAG, "SHT41: sensor stopped updating within the expected timeout");
        local_state->degraded_mode = true;
    }
}

void sgp41HealthCheck(TickType_t *now, const char *TAG,
                      const device_state_t *state_copy, health_local_state_t *local_state)
{
    if (!state_copy->sgp41_valid) {
        ESP_LOGD(TAG, "SGP41: sensor is not valid");
        local_state->degraded_mode = true;
    }
    if (state_copy->sgp41_fault) {
        ESP_LOGD(TAG, "SGP41: sensor is in fault");
        local_state->degraded_mode = true;
    }
    if ((*now - state_copy->sgp41_last_update) > pdMS_TO_TICKS(SGP41_TIMEOUT_MS)) {
        ESP_LOGD(TAG, "SGP41: sensor stopped updating within the expected timeout");
        local_state->degraded_mode = true;
    }
}

void bmp280HealthCheck(TickType_t *now, const char *TAG,
                       const device_state_t *state_copy, health_local_state_t *local_state)
{
    if (!state_copy->bmp280_valid) {
        ESP_LOGD(TAG, "BMP280: sensor is not valid");
        local_state->degraded_mode = true;
    }
    if (state_copy->bmp280_fault) {
        ESP_LOGD(TAG, "BMP280: sensor is in fault");
        local_state->degraded_mode = true;
    }
    if ((*now - state_copy->bmp280_last_update) > pdMS_TO_TICKS(BMP280_TIMEOUT_MS)) {
        ESP_LOGD(TAG, "BMP280: sensor stopped updating within the expected timeout");
        local_state->degraded_mode = true;
    }
}

void scd40HealthCheck(TickType_t *now, const char *TAG,
                      const device_state_t *state_copy, health_local_state_t *local_state)
{
    if (!state_copy->scd40_valid) {
        ESP_LOGD(TAG, "SCD40: sensor is not valid");
        local_state->degraded_mode = true;
    }
    if (state_copy->scd40_fault) {
        ESP_LOGD(TAG, "SCD40: sensor is in fault");
        local_state->degraded_mode = true;
    }
    if ((*now - state_copy->scd40_last_update) > pdMS_TO_TICKS(SCD40_TIMEOUT_MS)) {
        ESP_LOGD(TAG, "SCD40: sensor stopped updating within the expected timeout");
        local_state->degraded_mode = true;
    }
}

void pms7003HealthCheck(TickType_t *now, const char *TAG,
                        const device_state_t *state_copy, health_local_state_t *local_state)
{
    if (!state_copy->pms7003_valid) {
        ESP_LOGD(TAG, "PMS7003: sensor is not valid");
        local_state->degraded_mode = true;
    }
    if (state_copy->pms7003_fault) {
        ESP_LOGD(TAG, "PMS7003: sensor is in fault");
        local_state->degraded_mode = true;
    }
    if ((*now - state_copy->pms7003_last_update) > pdMS_TO_TICKS(PMS7003_TIMEOUT_MS)) {
        ESP_LOGD(TAG, "PMS7003: sensor stopped updating within the expected timeout");
        local_state->degraded_mode = true;
    }
}

void as7341HealthCheck(TickType_t *now, const char *TAG,
                       const device_state_t *state_copy, health_local_state_t *local_state)
{
    if (!state_copy->as7341_valid) {
        ESP_LOGD(TAG, "AS7341: sensor is not valid");
        local_state->degraded_mode = true;
    }
    if (state_copy->as7341_fault) {
        ESP_LOGD(TAG, "AS7341: sensor is in fault");
        local_state->degraded_mode = true;
    }
    if ((*now - state_copy->as7341_last_update) > pdMS_TO_TICKS(AS7341_TIMEOUT_MS)) {
        ESP_LOGD(TAG, "AS7341: sensor stopped updating within the expected timeout");
        local_state->degraded_mode = true;
    }
}

void inmp441HealthCheck(TickType_t *now, const char *TAG,
                        const device_state_t *state_copy, health_local_state_t *local_state)
{
    if (!state_copy->inmp441_valid) {
        ESP_LOGD(TAG, "INMP441: sensor is not valid");
        local_state->degraded_mode = true;
    }
    if (state_copy->inmp441_fault) {
        ESP_LOGD(TAG, "INMP441: sensor is in fault");
        local_state->degraded_mode = true;
    }
    if ((*now - state_copy->inmp441_last_update) > pdMS_TO_TICKS(INMP441_TIMEOUT_MS)) {
        ESP_LOGD(TAG, "INMP441: sensor stopped updating within the expected timeout");
        local_state->degraded_mode = true;
    }
}
