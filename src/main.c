// standard headers
#include <stdint.h>
#include <stdbool.h>

// freertos headers
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// esp-idf headers
#include "esp_log.h"

// task config headers
#include "task_config.h"

// task headers
#include "acquisition_task.h"
#include "audio_task.h"
#include "system_task.h"
#include "comm_task.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "System booting...");
    ESP_LOGI(TAG, "Creating tasks");

    // Create tasks and check the return value of xTaskCreate.
    // If there is not enough RAM, the function will fail and return a value different from pdPASS.
    
    // Acquisition task
    if (xTaskCreate(acquisition_task,
        "acquisition_task",
        STACK_ACQUISITION_WORDS,
        NULL,
        ACQUISITION_TASK_PRIORITY,
        NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create acquisition_task");
        abort(); // abort the program if the task creation fails
    }
    else
    {
        ESP_LOGI(TAG, "Acquisition task created");
    }

    // Audio task
    if(xTaskCreate(audio_task, 
       "audio_task",
       STACK_AUDIO_WORDS,
       NULL,
       AUDIO_TASK_PRIORITY,
       NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create audio_task");
        abort();
    }
    else
    {
        ESP_LOGI(TAG, "Audio task created");
    }

    // Comm task
    if(xTaskCreate(comm_task, 
       "comm_task",
       STACK_COMM_WORDS,
       NULL,
       COMM_TASK_PRIORITY,
       NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create comm_task");
        abort();
    }
    else
    {
        ESP_LOGI(TAG, "Comm task created");
    }

    // System task
    if(xTaskCreate(system_task, 
       "system_task",
       STACK_SYSTEM_WORDS,
       NULL,
       SYSTEM_TASK_PRIORITY,
       NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create system_task");
        abort();
    }
    else
    {
        ESP_LOGI(TAG, "System task created");
    }

    ESP_LOGI(TAG, "All tasks started");
}