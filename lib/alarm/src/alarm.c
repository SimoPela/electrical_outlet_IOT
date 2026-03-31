#include "alarm.h"
#include "esp_log.h"

void systemAlarmLogic(const char *TAG, const device_state_t *state_copy, alarm_local_state_t *local_state)
{
    ESP_LOGD(TAG, "Starting system alarm logic");
    if (state_copy->mics5524_gas_ppm > MICS5524_ALARM_THRESHOLD) {
        ESP_LOGW(TAG, "MICS5524 alarm detected");
        local_state->mics5524_alarm = true;
    }
    else {
        ESP_LOGD(TAG, "MICS5524 alarm not detected");
        local_state->mics5524_alarm = false;
    }
    if (state_copy->as312_motion_detected) {
        ESP_LOGW(TAG, "AS312 alarm detected");
        local_state->as312_alarm = true;
    }
    else {
        ESP_LOGD(TAG, "AS312 alarm not detected");
        local_state->as312_alarm = false;
    }
    ESP_LOGD(TAG, "System alarm logic completed");
}
