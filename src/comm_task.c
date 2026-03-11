#include <comm_task.h>
#include <task_config.h>
#include <device_state.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "COMM";

void comm_task(void *pvParameters)
{
    (void)pvParameters;
    // Counter used to log the stack usage periodically (counter = 10, tick = 1000 ms -> log every 10 seconds)
    uint32_t counter = 0;

    ESP_LOGI(TAG, "Comm task started");
    // get the time of the last wakeup
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        ESP_LOGI(TAG, "Comm task alive");

        // Log the stack usage periodically. Once the stack size is tuned, this can be removed.
        logTaskStackUsage(&counter, TAG, STACK_COMM_WORDS);

        // wait for 1 second
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
    }
}