/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file task_config.h
 * @brief FreeRTOS task stacks, priorities, periods, and stack-usage logging helper.
 */

#ifndef __TASK_CONFIG_H__
#define __TASK_CONFIG_H__

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/**
 * @name Task stack sizes (FreeRTOS words; 1 word = 4 bytes on ESP32)
 * @{
 */
#define STACK_ACQUISITION_WORDS     4096    /**< Acquisition task stack [words]. */
#define STACK_AUDIO_WORDS           4096    /**< Audio / I2S task stack [words]. */
#define STACK_SYSTEM_WORDS          4096    /**< System supervision task stack [words]. */
#define STACK_COMM_WORDS            4096    /**< MQTT / network task stack [words]. */
/** @} */

/**
 * @name Task priorities (higher value = higher priority)
 * @{
 */
#define ACQUISITION_TASK_PRIORITY   2
#define AUDIO_TASK_PRIORITY         2
#define SYSTEM_TASK_PRIORITY        4
#define COMM_TASK_PRIORITY          3
/** @} */

/**
 * @name Sensor polling intervals (milliseconds)
 * @{
 */
#define AS312_INTERVAL_MS      100
#define MICS5524_INTERVAL_MS   1000
#define SGP41_INTERVAL_MS      1000
#define SHT41_INTERVAL_MS      2000
#define BMP280_INTERVAL_MS     2000
#define AS7341_INTERVAL_MS     3000
#define SCD40_INTERVAL_MS      2500 // poll faster than the 5 s cycle to catch the data-ready window
#define PMS7003_INTERVAL_MS    5000
/** @} */

/**
 * @name Application task loop periods (milliseconds)
 * @{
 */
#define ACQUISITION_TASK_INTERVAL_MS 100
#define AUDIO_TASK_INTERVAL_MS 500
#define SYSTEM_TASK_INTERVAL_MS 2000
#define COMM_TASK_INTERVAL_MS 5000
/** @} */

/** @brief Log tag string used for verbose sensor debug lines. */
#define TAG_DEBUG "[DEBUG]"

/**
 * @brief Log stack high-water mark for the calling task (every 10 invocations).
 *
 * @param[in,out] counter Call counter; incremented each time; log when @c (*counter % 10) == 0.
 * @param[in] TAG ESP-IDF log tag string.
 * @param[in] task_stack_size Total stack size of this task in words (for “used” display).
 */
void logTaskStackUsage(uint32_t *counter, const char *TAG, UBaseType_t task_stack_size);

#endif // __TASK_CONFIG_H__