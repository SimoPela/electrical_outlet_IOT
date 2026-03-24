/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "system_task.h"
#include "task_config.h"
#include "device_state.h"

#include "health.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "[SYSTEM]";

void system_task(void *pvParameters)
{
    (void)pvParameters;

    // Counter used to log stack usage periodically
    uint32_t counter = 0;
    // Counter used to print that the task is alive periodically
    uint32_t alive_counter = 0;

    ESP_LOGI(TAG, "System task started");

    // Reference tick for vTaskDelayUntil()
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        logTaskAlive(TAG, &alive_counter, 2);

        // Local snapshot of the shared device state
        device_state_t state_copy = {0};

        // Local working state recomputed at each iteration
        system_local_state_t local_state = {0};

        // Get current RTOS tick count
        TickType_t now = xTaskGetTickCount();

        // Read a consistent snapshot of the shared device state
        if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
        {
            state_copy = g_device_state;
            xSemaphoreGive(g_device_state_mutex);
        }

        // Health checks (mutate local_state via pointers)
        ESP_LOGD(TAG, "Starting health checks");
        as312HealthCheck(&now, TAG, &state_copy, &local_state);
        mics5524HealthCheck(&now, TAG, &state_copy, &local_state);
        sht41HealthCheck(&now, TAG, &state_copy, &local_state);
        sgp41HealthCheck(&now, TAG, &state_copy, &local_state);
        bmp280HealthCheck(&now, TAG, &state_copy, &local_state);
        scd40HealthCheck(&now, TAG, &state_copy, &local_state);
        pms7003HealthCheck(&now, TAG, &state_copy, &local_state);
        as7341HealthCheck(&now, TAG, &state_copy, &local_state);
        inmp441HealthCheck(&now, TAG, &state_copy, &local_state);
        ESP_LOGD(TAG, "Health checks completed");

        ESP_LOGD(TAG, "Trying to restore sensors");
        health_try_restore_sensors(TAG, &local_state);
        ESP_LOGD(TAG, "Sensors restored");

        // alarm logic
        local_state.as312_alarm = false;
        local_state.mics5524_alarm = false;

        // Global alarm flag
        local_state.alarm_active = local_state.as312_alarm || local_state.mics5524_alarm;

        // System is considered OK when not in degraded mode
        local_state.system_ok = !local_state.degraded_mode;

        // Write back only high-level system flags
        if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
        {
            g_device_state.degraded_mode = local_state.degraded_mode;
            g_device_state.as312_alarm = local_state.as312_alarm;
            g_device_state.mics5524_alarm = local_state.mics5524_alarm;
            g_device_state.alarm_active = local_state.alarm_active;
            g_device_state.system_ok = local_state.system_ok;
            xSemaphoreGive(g_device_state_mutex);
        }

        ESP_LOGD(TAG_DEBUG,
                 "system_ok=%d degraded=%d alarm_active=%d motion_alarm=%d gas_alarm=%d motion=%d valid=%d fault=%d",
                 local_state.system_ok,
                 local_state.degraded_mode,
                 local_state.alarm_active,
                 local_state.as312_alarm,
                 local_state.mics5524_alarm,
                 state_copy.motion_detected,
                 state_copy.as312_valid,
                 state_copy.as312_fault);

        // Log the stack usage periodically.
        // Once the stack size is validated, this can be removed.
        logTaskStackUsage(&counter, 10, TAG, STACK_SYSTEM_WORDS);

        // Run the task periodically every 1 second
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SYSTEM_TASK_INTERVAL_MS));
    }
}