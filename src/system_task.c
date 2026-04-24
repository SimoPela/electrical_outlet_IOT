/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file system_task.c
 * @brief FreeRTOS system supervision task implementation.
 *
 * At each @c SYSTEM_TASK_INTERVAL_MS tick:
 *  1. Snapshots @c g_device_state.
 *  2. Runs per-sensor health checks and populates @c health_local_state_t.
 *  3. Runs alarm logic and populates @c alarm_local_state_t.
 *  4. Writes the derived flags back to @c g_device_state.
 */

#include "system_task.h"
#include "task_config.h"
#include "device_state.h"

#include "health.h"
#include "alarm.h"

#include "rgbled.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "[SYSTEM]";

/** @copydoc system_task */
void system_task(void *pvParameters)
{
    (void)pvParameters;

    uint32_t counter = 0;
    uint32_t alive_counter = 0;

    ESP_LOGI(TAG, "System task started");

    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        logTaskAlive(TAG, &alive_counter, 2);

        device_state_t state_copy = {0};
        system_local_state_t health_local_state = {0};
        alarm_local_state_t alarm_local_state = {0};

        TickType_t now = xTaskGetTickCount();

        // Read a consistent snapshot of the shared device state
        if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
        {
            state_copy = g_device_state;
            xSemaphoreGive(g_device_state_mutex);
        }

        // Health checks of all sensors (skipped before ALL_SENSORS_TIMEOUT_MS to avoid premature fault flags)
        sensorHealthCheck(TAG, &now, &state_copy, &health_local_state);

        // System health check
        systemHealthCheck(TAG, &health_local_state);

        // System alarm logic
        systemAlarmLogic(TAG, &state_copy, &alarm_local_state);

        // Write back only high-level system flags
        if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
        {
            g_device_state.degraded_mode = health_local_state.degraded_mode;
            g_device_state.as312_alarm = alarm_local_state.as312_alarm;
            g_device_state.mics5524_alarm = alarm_local_state.mics5524_alarm;
            g_device_state.system_ok = health_local_state.system_ok;
            g_device_state.co2_alarm_level = alarm_local_state.co2_alarm_level;
            xSemaphoreGive(g_device_state_mutex);
        }

        ESP_LOGD(TAG_DEBUG,
                 "system_ok=%d degraded=%d as312_alarm=%d mics5524_alarm=%d co2_alarm_level=%s",
                 health_local_state.system_ok,
                 health_local_state.degraded_mode,
                 alarm_local_state.as312_alarm,
                 alarm_local_state.mics5524_alarm,
                 alarm_local_state.co2_alarm_level);

        logTaskStackHeapUsage(&counter, LOG_CEILING_SYSTEM, TAG, STACK_SYSTEM_WORDS);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SYSTEM_TASK_INTERVAL_MS));
    }
}
