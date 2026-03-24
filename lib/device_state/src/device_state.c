/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "device_state.h"

#include "esp_log.h"

device_state_t g_device_state = {0};
SemaphoreHandle_t g_device_state_mutex = NULL;
SemaphoreHandle_t g_sensor_driver_mutex = NULL;

static const char *TAG = "device_state";

void device_state_init(void)
{
    // -----------------------------
    // Sensor measurements
    // -----------------------------
    g_device_state.co2_ppm = 0.0f;
    g_device_state.temperature_scd40 = 0.0f;
    g_device_state.humidity_scd40 = 0.0f;

    g_device_state.temperature_c = 0.0f;
    g_device_state.humidity_percent = 0.0f;
    
    g_device_state.bmp280_pressure_hpa = 0.0f;
    g_device_state.bmp280_temperature_c = 0.0f;

    g_device_state.voc_index = 0.0f;
    g_device_state.nox_index = 0.0f;

    g_device_state.pm1_0_ug_m3 = 0.0f;
    g_device_state.pm2_5_ug_m3 = 0.0f;
    g_device_state.pm10_ug_m3 = 0.0f;

    g_device_state.noise_db = 0.0f;

    g_device_state.gas_level_raw = 0.0f;
    g_device_state.gas_ppm = 0.0f;

    for (int i = 0; i < AS7341_CHANNELS; i++)
    {
        g_device_state.light.channels[i] = 0.0f;
    }

    g_device_state.motion_detected = false;

    // -----------------------------
    // Last update timestamps
    // -----------------------------
    g_device_state.as312_last_update = 0;
    g_device_state.mics5524_last_update = 0;
    g_device_state.sgp41_last_update = 0;
    g_device_state.sht41_last_update = 0;
    g_device_state.bmp280_last_update = 0;
    g_device_state.scd40_last_update = 0;
    g_device_state.pms7003_last_update = 0;
    g_device_state.as7341_last_update = 0;
    g_device_state.inmp441_last_update = 0;

    // -----------------------------
    // Validity flags
    // -----------------------------
    g_device_state.as312_valid = false;
    g_device_state.mics5524_valid = false;
    g_device_state.sgp41_valid = false;
    g_device_state.sht41_valid = false;
    g_device_state.bmp280_valid = false;
    g_device_state.scd40_valid = false;
    g_device_state.pms7003_valid = false;
    g_device_state.as7341_valid = false;
    g_device_state.inmp441_valid = false;

    // -----------------------------
    // Fault flags
    // -----------------------------
    g_device_state.as312_fault = false;
    g_device_state.mics5524_fault = false;
    g_device_state.sgp41_fault = false;
    g_device_state.sht41_fault = false;
    g_device_state.bmp280_fault = false;
    g_device_state.scd40_fault = false;
    g_device_state.pms7003_fault = false;
    g_device_state.as7341_fault = false;
    g_device_state.inmp441_fault = false;

    // -----------------------------
    // System state
    // -----------------------------
    g_device_state.wifi_connected = false;
    g_device_state.mqtt_connected = false;
    g_device_state.degraded_mode = false;
    g_device_state.system_ok = false;

    // -----------------------------
    // Alarm state
    // -----------------------------
    g_device_state.alarm_active = false;
    g_device_state.mics5524_alarm = false;
    g_device_state.as312_alarm = false;

    // -----------------------------
    // Mutex
    // -----------------------------
    g_device_state_mutex = xSemaphoreCreateMutex();
    g_sensor_driver_mutex = xSemaphoreCreateMutex();
    if (g_sensor_driver_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create g_sensor_driver_mutex");
    }
}