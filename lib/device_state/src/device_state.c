#include "device_state.h"

device_state_t g_device_state = {0};
SemaphoreHandle_t g_device_state_mutex = NULL;

void device_state_init(void)
{
    g_device_state.co2_ppm = 0.0f;
    g_device_state.temperature_scd40 = 0.0f;
    g_device_state.humidity_scd40 = 0.0f;

    g_device_state.temperature_c = 0.0f;
    g_device_state.humidity_percent = 0.0f;

    g_device_state.pressure_hpa = 0.0f;

    g_device_state.voc_index = 0.0f;
    g_device_state.nox_index = 0.0f;

    g_device_state.pm1_0_ug_m3 = 0.0f;
    g_device_state.pm2_5_ug_m3 = 0.0f;
    g_device_state.pm10_ug_m3 = 0.0f;

    g_device_state.noise_db = 0.0f;

    g_device_state.gas_level_raw = 0.0f;

    for (int i = 0; i < AS7341_CHANNELS; i++)
    {
        g_device_state.light.channels[i] = 0.0f;
    }

    g_device_state.motion_detected = false;

    g_device_state.wifi_connected = false;
    g_device_state.mqtt_connected = false;

    g_device_state.alarm_active = false;
    g_device_state.degraded_mode = false;

    g_device_state_mutex = xSemaphoreCreateMutex();
}