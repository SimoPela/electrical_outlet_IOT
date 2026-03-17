/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#ifndef SYSTEM_TASK_H
#define SYSTEM_TASK_H

#include <stdbool.h>

// Example timeout 
#define MOTION_TIMEOUT_MS 15000
#define GAS_TIMEOUT_MS 15000
#define SGP41_TIMEOUT_MS 15000
#define SHT41_TIMEOUT_MS 15000
#define BMP280_TIMEOUT_MS 15000
#define SCD40_TIMEOUT_MS 15000
#define PMS7003_TIMEOUT_MS 15000
#define AS7341_TIMEOUT_MS 15000
#define AUDIO_TIMEOUT_MS 15000
#define WIFI_TIMEOUT_MS 15000
#define MQTT_TIMEOUT_MS 15000
#define ALARM_TIMEOUT_MS 15000
#define DEGRADED_MODE_TIMEOUT_MS 15000
#define SYSTEM_OK_TIMEOUT_MS 15000

typedef struct
{
    // -----------------------------
    // System flags
    // -----------------------------
    bool degraded_mode;
    bool motion_alarm;
    bool gas_alarm;
    bool alarm_active;
    bool system_ok;
} system_local_state_t;

void system_task(void *pvParameters);

#endif