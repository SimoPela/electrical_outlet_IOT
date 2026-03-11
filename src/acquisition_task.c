#include "acquisition_task.h"
#include "task_config.h"
#include "device_state.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ACQUISITION";

void acquisition_task(void *pvParameters)
{
    (void)pvParameters;

    // Counter used to log the stack usage periodically
    uint32_t counter = 0;

    // Counter used to print that the task is alive every 2000 ms
    uint32_t alive_counter = 0;

    ESP_LOGI(TAG, "Acquisition task started");

    // Get the reference wake time for vTaskDelayUntil()
    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t now = xLastWakeTime;

    // Local persistent state: keeps the last valid values
    acquisition_local_state_t local_state = {
        .motion_detected = false,
        .gas_level_raw = 0.0f,
        .voc_index = 0.0f,
        .nox_index = 0.0f,
        .temperature_c = 0.0f,
        .humidity_percent = 0.0f,
        .pressure_hpa = 0.0f,
        .co2_ppm = 0.0f,
        .temperature_scd40 = 0.0f,
        .humidity_scd40 = 0.0f,
        .pm1_0_ug_m3 = 0.0f,
        .pm2_5_ug_m3 = 0.0f,
        .pm10_ug_m3 = 0.0f,
        .light = {{0}}
    };

    // Force first read
    TickType_t last_as312    = now - pdMS_TO_TICKS(AS312_INTERVAL_MS);
    TickType_t last_mics5524 = now - pdMS_TO_TICKS(MICS5524_INTERVAL_MS);
    TickType_t last_sgp41    = now - pdMS_TO_TICKS(SGP41_INTERVAL_MS);
    TickType_t last_sht41    = now - pdMS_TO_TICKS(SHT41_INTERVAL_MS);
    TickType_t last_bmp280   = now - pdMS_TO_TICKS(BMP280_INTERVAL_MS);
    TickType_t last_as7341   = now - pdMS_TO_TICKS(AS7341_INTERVAL_MS);
    TickType_t last_scd40    = now - pdMS_TO_TICKS(SCD40_INTERVAL_MS);
    TickType_t last_pms7003  = now - pdMS_TO_TICKS(PMS7003_INTERVAL_MS);

    for (;;)
    {
        // Print that the task is alive every 2000 ms
        alive_counter++;
        if (alive_counter >= 20)
        {
            ESP_LOGI(TAG, "Acquisition task alive");
            alive_counter = 0;
        }

        // Update current time
        now = xTaskGetTickCount();

        // AS312 - motion
        if ((now - last_as312) >= pdMS_TO_TICKS(AS312_INTERVAL_MS))
        {
            last_as312 = now;

            // TODO: replace with real sensor read
            local_state.motion_detected = true;
        }

        // MiCS-5524 - gas
        if ((now - last_mics5524) >= pdMS_TO_TICKS(MICS5524_INTERVAL_MS))
        {
            last_mics5524 = now;

            // TODO: replace with real ADC read
            local_state.gas_level_raw = 1293.0f;
        }

        // SGP41 - VOC / NOx
        if ((now - last_sgp41) >= pdMS_TO_TICKS(SGP41_INTERVAL_MS))
        {
            last_sgp41 = now;

            // TODO: replace with real sensor read
            local_state.voc_index = 1223.0f;
            local_state.nox_index = 1253.0f;
        }

        // SHT41 - temperature and humidity
        if ((now - last_sht41) >= pdMS_TO_TICKS(SHT41_INTERVAL_MS))
        {
            last_sht41 = now;

            // TODO: replace with real sensor read
            local_state.temperature_c = 22.5f;
            local_state.humidity_percent = 45.0f;
        }

        // BMP280 - pressure
        if ((now - last_bmp280) >= pdMS_TO_TICKS(BMP280_INTERVAL_MS))
        {
            last_bmp280 = now;

            // TODO: replace with real sensor read
            local_state.pressure_hpa = 1013.25f;
        }

        // AS7341 - light spectrum
        if ((now - last_as7341) >= pdMS_TO_TICKS(AS7341_INTERVAL_MS))
        {
            last_as7341 = now;

            // TODO: replace with real sensor read
            for (int i = 0; i < AS7341_CHANNELS; i++)
            {
                local_state.light.channels[i] = 1293.0f;
            }
        }

        // SCD40 - CO2
        if ((now - last_scd40) >= pdMS_TO_TICKS(SCD40_INTERVAL_MS))
        {
            last_scd40 = now;

            // TODO: replace with real sensor read
            local_state.co2_ppm = 400.0f;
            local_state.temperature_scd40 = 22.5f;
            local_state.humidity_scd40 = 45.0f;
        }

        // PMS7003 - particulate matter
        if ((now - last_pms7003) >= pdMS_TO_TICKS(PMS7003_INTERVAL_MS))
        {
            last_pms7003 = now;

            // TODO: replace with real sensor read
            local_state.pm1_0_ug_m3 = 1293.0f;
            local_state.pm2_5_ug_m3 = 1293.0f;
            local_state.pm10_ug_m3 = 1293.0f;
        }

        // Copy the complete local state into the shared device state
        if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
        {
            g_device_state.motion_detected   = local_state.motion_detected;

            g_device_state.gas_level_raw     = local_state.gas_level_raw;

            g_device_state.voc_index         = local_state.voc_index;
            g_device_state.nox_index         = local_state.nox_index;

            g_device_state.temperature_c     = local_state.temperature_c;
            g_device_state.humidity_percent  = local_state.humidity_percent;

            g_device_state.pressure_hpa      = local_state.pressure_hpa;

            g_device_state.co2_ppm           = local_state.co2_ppm;
            g_device_state.temperature_scd40 = local_state.temperature_scd40;
            g_device_state.humidity_scd40    = local_state.humidity_scd40;

            g_device_state.pm1_0_ug_m3       = local_state.pm1_0_ug_m3;
            g_device_state.pm2_5_ug_m3       = local_state.pm2_5_ug_m3;
            g_device_state.pm10_ug_m3        = local_state.pm10_ug_m3;

            for (int i = 0; i < AS7341_CHANNELS; i++)
            {
                g_device_state.light.channels[i] = local_state.light.channels[i];
            }

            xSemaphoreGive(g_device_state_mutex);
        }

        // Log the stack usage periodically
        logTaskStackUsage(&counter, TAG, STACK_ACQUISITION_WORDS);

        // Wait for 100 ms
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    }
}
