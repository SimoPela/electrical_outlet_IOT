/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file task_config.c
 * @brief Implementation of the task stack/heap logging helpers declared in task_config.h.
 */

#include "task_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


/** @copydoc logTaskStackHeapUsage */
void logTaskStackHeapUsage(uint32_t *counter,
    uint32_t ceiling,
    const char *TAG,
    UBaseType_t task_stack_size)
{
    if (++(*counter) % ceiling == 0)
    {
        /* STACK (FreeRTOS — in words) */
        UBaseType_t stack_remaining = uxTaskGetStackHighWaterMark(NULL);
        UBaseType_t stack_used = task_stack_size - stack_remaining;

        /* HEAP (ESP-IDF — in bytes) */
        size_t heap_free     = heap_caps_get_free_size(MALLOC_CAP_8BIT);
        size_t heap_total    = heap_caps_get_total_size(MALLOC_CAP_8BIT);
        size_t heap_min_free = heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
        size_t heap_used     = heap_total - heap_free;

        ESP_LOGI(TAG,
        "Stack used: %u words | remaining: %u words || Heap used: %u B | free: %u B | min free: %u B",
        (unsigned int)stack_used,
        (unsigned int)stack_remaining,
        (unsigned int)heap_used,
        (unsigned int)heap_free,
        (unsigned int)heap_min_free);
    }
}

/** @copydoc logTaskAlive */
void logTaskAlive(const char *TAG, uint32_t *alive_counter, uint32_t ceiling)
{
    if (++(*alive_counter) % ceiling == 0)
    {
        ESP_LOGD(TAG, "Task alive");
    }
}
