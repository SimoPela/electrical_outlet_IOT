#include "alarm.h"
#include "esp_log.h"

static void as312AlarmLogic(const char *TAG, const device_state_t *state_copy, alarm_local_state_t *local_state)
{
    if (state_copy->as312_motion_detected) {
        ESP_LOGW(TAG, "AS312 alarm detected");
        local_state->as312_alarm = true;
    }
    else {
        ESP_LOGD(TAG, "AS312 alarm not detected");
        local_state->as312_alarm = false;
    }
}

static void mics5524AlarmLogic(const char *TAG, const device_state_t *state_copy, alarm_local_state_t *local_state)
{
    if (state_copy->mics5524_gas_ppm > MICS5524_ALARM_THRESHOLD) {
        ESP_LOGW(TAG, "MICS5524 alarm detected");
        local_state->mics5524_alarm = true;
    }
    else {
        ESP_LOGD(TAG, "MICS5524 alarm not detected");
        local_state->mics5524_alarm = false;
    }
}

static void scd40AlarmLogic(const char *TAG, const device_state_t *state_copy, alarm_local_state_t *local_state)
{
    if (state_copy->co2_ppm < 400) {
        local_state->co2_alarm_level = CO2_LEVEL_OPTIMAL;
    }
    else if (state_copy->co2_ppm < 800 && state_copy->co2_ppm > 400) {
        local_state->co2_alarm_level = CO2_LEVEL_GOOD;
    }
    else if (state_copy->co2_ppm < 1200 && state_copy->co2_ppm > 800) {
        local_state->co2_alarm_level = CO2_LEVEL_MODERATE;
    }
    else if (state_copy->co2_ppm < 1800 && state_copy->co2_ppm > 1200) {
        local_state->co2_alarm_level = CO2_LEVEL_POOR;
    }
    else if (state_copy->co2_ppm < 5000 && state_copy->co2_ppm > 1800) {
        local_state->co2_alarm_level = CO2_LEVEL_VERY_POOR;
    }
    else {
        local_state->co2_alarm_level = CO2_LEVEL_CRITICAL;
    }
}

void systemAlarmLogic(const char *TAG, const device_state_t *state_copy, alarm_local_state_t *local_state)
{
    //ESP_LOGD(TAG, "Starting system alarm logic");
    as312AlarmLogic(TAG, state_copy, local_state);
    mics5524AlarmLogic(TAG, state_copy, local_state);
    scd40AlarmLogic(TAG, state_copy, local_state);
    //ESP_LOGD(TAG, "System alarm logic completed");
}
