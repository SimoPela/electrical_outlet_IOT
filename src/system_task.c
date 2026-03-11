#include "system_task.h"
#include "task_config.h"
#include "device_state.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SYSTEM";

void system_task(void *pvParameters)
{
    (void)pvParameters;
    // Counter used to log the stack usage periodically (counter = 10, tick = 1000 ms -> log every 10 seconds)
    uint32_t counter = 0;
    // Counter used to print that the task is alive every 2000 ms
    uint32_t alive_counter = 0;

    ESP_LOGI(TAG, "System task started");
    // get the time of the last wakeup
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        // Print that the task is alive every 2000 ms
        alive_counter++;
        if (alive_counter >= 2)
        {
            ESP_LOGI(TAG, "System task alive");
            alive_counter = 0;
        }

        // Copy the layout of the device state structure
        device_state_t state_copy = {0};

        // Define some system flags
        bool degraded_mode = false;
        bool alarm_active = false;
        bool system_ok = false;

        // Get the current time
        TickType_t now = xTaskGetTickCount();

        // Read snapshot of device state
        if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
        {
            state_copy = g_device_state;
            xSemaphoreGive(g_device_state_mutex);
        }
        
        // Now there is only motion sensor, in future here all the sensor will be checked
        // Sensor reported invalid data
        if (!state_copy.motion_valid)
        {
            degraded_mode = true;
        }

        // Sensor reported hardware fault
        if (state_copy.motion_fault)
        {
            degraded_mode = true;
        }

        // Sensor stopped updating
        if ((now - state_copy.motion_last_update) > pdMS_TO_TICKS(MOTION_TIMEOUT_MS))
        {
            degraded_mode = true;
        }

        // Motion sensor does not generate alarms in this example
        alarm_active = false;

        // System health
        system_ok = !degraded_mode;

        // Write back only system-level flags
        if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
        {
            g_device_state.degraded_mode = degraded_mode;
            g_device_state.alarm_active = alarm_active;
            g_device_state.system_ok = system_ok;
            xSemaphoreGive(g_device_state_mutex);
        }

        ESP_LOGI(TAG, "system_ok=%d degraded=%d alarm=%d motion=%d valid=%d fault=%d",
                 system_ok,
                 degraded_mode,
                 alarm_active,
                 state_copy.motion_detected,
                 state_copy.motion_valid,
                 state_copy.motion_fault);


        // Log the stack usage periodically. Once the stack size is tuned, this can be removed.
        logTaskStackUsage(&counter, TAG, STACK_SYSTEM_WORDS);

        // wait for 1 second
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
    }
}