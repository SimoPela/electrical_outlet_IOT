#ifndef __DEVICE_STATE_H__
#define __DEVICE_STATE_H__

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

typedef struct
{
    float co2_ppm;
    float temperature_c;
    float humidity_percent;
    float pressure_hpa;

    float voc_index;
    float nox_index;

    float pm1_0_ug_m3;
    float pm2_5_ug_m3;
    float pm10_ug_m3;

    float noise_db;

    float gas_level_raw;

    bool motion_detected;

    bool wifi_connected;
    bool mqtt_connected;
    bool alarm_active;
    bool degraded_mode;

} device_state_t;

extern device_state_t g_device_state;
extern SemaphoreHandle_t g_device_state_mutex;

void device_state_init(void);

#endif // __DEVICE_STATE_H__