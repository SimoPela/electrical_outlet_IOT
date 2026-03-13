#include "comm_task.h"
#include "task_config.h"
#include "device_state.h"
#include "mqtt_payload.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "COMM";

void comm_task(void *pvParameters)
{
    (void)pvParameters;

    uint32_t counter = 0;
    uint32_t alive_counter = 0;

    ESP_LOGI(TAG, "Communication task started");

    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        // Print that the task is alive every 5 seconds
        alive_counter++;
        if (alive_counter >= 5)
        {
            ESP_LOGI(TAG, "Communication task alive");
            alive_counter = 0;
        }

        // Copy the layout of the device state structure
        device_state_t state_copy = {0};

        // Read a consistent snapshot
        if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
        {
            state_copy = g_device_state;
            xSemaphoreGive(g_device_state_mutex);
        }

        // Example communication logic
        if (state_copy.mqtt_connected)
        {
            char payload[256] = {0};

            // Build the payload for the state topic (EXAMPLE)
            if (mqtt_payload_build_state(payload, sizeof(payload), &state_copy) >= 0)
            {
                ESP_LOGI(TAG, "Publishing state payload: %s", payload);

                // TODO: replace with real MQTT publish
                // esp_mqtt_client_publish(client, "devices/<device_id>/state", payload, 0, 1, 0);
            }
            else
            {
                ESP_LOGE(TAG, "Failed to build MQTT payload");
            }
        }
        else
        {
            ESP_LOGW(TAG, "MQTT not connected, publish skipped");
        }
        
        // Log the stack usage
        logTaskStackUsage(&counter, TAG, STACK_COMM_WORDS);

        // Publish period: 5 seconds
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5000));
    }
}