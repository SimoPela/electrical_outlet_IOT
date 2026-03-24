/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "task_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void logTaskStackUsage(uint32_t *counter, uint32_t ceiling, const char *TAG, UBaseType_t task_stack_size)
{
    if (++(*counter) % ceiling == 0)
    {
        UBaseType_t stack_remaining = uxTaskGetStackHighWaterMark(NULL);
        UBaseType_t stack_used = task_stack_size - stack_remaining;

        ESP_LOGD(TAG_DEBUG, "Stack used: %u words | remaining: %u words",
                 stack_used, stack_remaining);
    }
}


void logTaskAlive(const char *TAG, uint32_t *alive_counter, uint32_t ceiling)
{
    if (++(*alive_counter) % ceiling ==0)
    {
        ESP_LOGI(TAG, "Task alive");
    }
}
