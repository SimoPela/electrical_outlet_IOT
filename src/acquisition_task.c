#include "acquisition_task.h"
#include "task_config.h"
#include "device_state.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ACQUISITION";

// example sensor values structure
typedef struct
{
    float temp;
    float hum;
    float co2;
    float pressure;
    float voc;
    float nox;
    float pm1;
    float pm2_5;
    float pm10;
    float gas_level_raw;
    as7341_data_t light;
    bool motion_detected;
} sensor_values_example_t;

void acquisition_task(void *pvParameters)
{
    (void)pvParameters;
    // Counter used to log the stack usage periodically (counter = 10, tick = 1000 ms -> log every 10 seconds)
    uint32_t counter = 0;
    
    ESP_LOGI(TAG, "Acquisition task started");
    // get the time of the last wakeup
    TickType_t xLastWakeTime = xTaskGetTickCount();
    // get the current time
    TickType_t now = xLastWakeTime;

    // Example sensor values (placeholder for real sensors)
    sensor_values_example_t sensor_values = {
        .temp = 22.5f,
        .hum = 45.0f,
        .co2 = 400.0f,
        .pressure = 1013.25f,
        .voc = 0.0f,
        .nox = 0.0f,
        .pm1 = 0.0f,
        .pm2_5 = 0.0f,
        .pm10 = 0.0f,
        .gas_level_raw = 0.0f,
        .light = {{0}},
        .motion_detected = false
    };

    // Force first read
    TickType_t last_as312    = now - pdMS_TO_TICKS(AS312_INTERVAL_MS);

    for (;;) {
        // print each 100ms !
        ESP_LOGI(TAG, "Acquisition task alive");

        // update the current time
        now = xTaskGetTickCount();

        // AS312 - motion
        if ((now - last_as312) >= pdMS_TO_TICKS(AS312_INTERVAL_MS))
        {
            last_as312 = now;

            // Update the shared device state
            if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
            {
                g_device_state.motion_detected = sensor_values.motion_detected;
                
                xSemaphoreGive(g_device_state_mutex);
            } 
        }

        // Log the stack usage periodically. Once the stack size is tuned, this can be removed.
        logTaskStackUsage(&counter, TAG, STACK_ACQUISITION_WORDS);

        // wait for 1 second
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    }
}