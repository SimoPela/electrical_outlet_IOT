/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "system_task.h"
#include "task_config.h"
#include "device_state.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "[SYSTEM]";

void system_task(void *pvParameters)
{
    (void)pvParameters;

    // Counter used to log stack usage periodically
    // (counter = 10, task period = 1000 ms -> log every 10 seconds)
    uint32_t counter = 0;

    // Counter used to print that the task is alive periodically
    // (counter = 2, task period = 1000 ms -> log every 2 seconds)
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

        // --------------------------------------------------
        // Sensor supervision logic
        // For now only the motion sensor is checked.
        // In the future, all sensors will be handled here.
        // --------------------------------------------------

        // Sensor reported invalid data
        if (!state_copy.motion_valid)
        {
            local_state.degraded_mode = true;
        }

        // Sensor reported hardware fault
        if (state_copy.motion_fault)
        {
            local_state.degraded_mode = true;
        }

        // Sensor stopped updating within the expected timeout
        if ((now - state_copy.motion_last_update) > pdMS_TO_TICKS(MOTION_TIMEOUT_MS))
        {
            local_state.degraded_mode = true;
        }

        // --------------------------------------------------
        // Alarm logic
        // For now, motion does not generate an alarm.
        // This can be extended later with real alarm policies.
        // --------------------------------------------------
        local_state.motion_alarm = false;
        local_state.gas_alarm = false;

        // Global alarm flag
        local_state.alarm_active = local_state.motion_alarm || local_state.gas_alarm;

        // System is considered OK when not in degraded mode
        local_state.system_ok = !local_state.degraded_mode;

        // Write back only high-level system flags
        if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
        {
            g_device_state.degraded_mode = local_state.degraded_mode;
            g_device_state.motion_alarm = local_state.motion_alarm;
            g_device_state.gas_alarm = local_state.gas_alarm;
            g_device_state.alarm_active = local_state.alarm_active;
            g_device_state.system_ok = local_state.system_ok;
            xSemaphoreGive(g_device_state_mutex);
        }

        ESP_LOGD(TAG_DEBUG,
                 "system_ok=%d degraded=%d alarm_active=%d motion_alarm=%d gas_alarm=%d motion=%d valid=%d fault=%d",
                 local_state.system_ok,
                 local_state.degraded_mode,
                 local_state.alarm_active,
                 local_state.motion_alarm,
                 local_state.gas_alarm,
                 state_copy.motion_detected,
                 state_copy.motion_valid,
                 state_copy.motion_fault);

        // Log the stack usage periodically.
        // Once the stack size is validated, this can be removed.
        logTaskStackUsage(&counter, 10, TAG, STACK_SYSTEM_WORDS);

        // Run the task periodically every 1 second
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SYSTEM_TASK_INTERVAL_MS));
    }
}