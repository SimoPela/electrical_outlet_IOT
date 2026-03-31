/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
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
        system_local_state_t health_local_state = {0};

        // Local working state for alarm logic
        alarm_local_state_t alarm_local_state = {0};

        // Get current RTOS tick count
        TickType_t now = xTaskGetTickCount();

        // Read a consistent snapshot of the shared device state
        if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
        {
            state_copy = g_device_state;
            xSemaphoreGive(g_device_state_mutex);
        }

        // Health checks of all sensors (after ALL_SENSORS_TIMEOUT_MS seconds to avoid premature checks)
        sensorHealthCheck(TAG, &now, &state_copy, &health_local_state);

        // Sensor Restore if needed (after ALL_SENSORS_TIMEOUT_MS seconds to avoid premature restores)
        sensorHealthRestore(TAG,&health_local_state, &now); // TODO: implement this

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
            xSemaphoreGive(g_device_state_mutex);
        }

        ESP_LOGD(TAG_DEBUG,
                 "system_ok=%d degraded=%d as312_alarm=%d mics5524_alarm=%d",
                 health_local_state.system_ok,
                 health_local_state.degraded_mode,
                 alarm_local_state.as312_alarm,
                 alarm_local_state.mics5524_alarm);

        // Log the stack usage periodically.
        // Once the stack size is validated, this can be removed.
        logTaskStackUsage(&counter, 10, TAG, STACK_SYSTEM_WORDS);

        // Run the task periodically every 1 second
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SYSTEM_TASK_INTERVAL_MS));
    }
}