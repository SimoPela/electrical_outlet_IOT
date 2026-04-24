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
#define AS7341_INTERVAL_MS     1000
#define SCD40_INTERVAL_MS      2500 // poll faster than the 5 s cycle to catch the data-ready window
#define PMS7003_INTERVAL_MS    5000
/** @} */

/**
 * @name mqtt polling intervals (milliseconds)
 * @{
 */
 #define MQTT_ENVIRONMENT_INTERVAL_MS 1000
 #define MQTT_SYSTEM_INTERVAL_MS 10000
 /** @} */

/**
 * @name Application task loop periods (milliseconds)
 * @{
 */
#define ACQUISITION_TASK_INTERVAL_MS 1000
#define AUDIO_TASK_INTERVAL_MS 500
#define SYSTEM_TASK_INTERVAL_MS 100
#define COMM_TASK_INTERVAL_MS 100
/** @} */

/**
 * @name Stack/heap log ceilings (one log every LOG_STACK_PERIOD_MS)
 *
 * ceiling = LOG_STACK_PERIOD_MS / TASK_INTERVAL_MS
 * @{
 */
#define LOG_STACK_PERIOD_MS             10000UL
#define LOG_CEILING_ACQUISITION  (LOG_STACK_PERIOD_MS / ACQUISITION_TASK_INTERVAL_MS)  /**< 10  */
#define LOG_CEILING_AUDIO        (LOG_STACK_PERIOD_MS / AUDIO_TASK_INTERVAL_MS)         /**< 20  */
#define LOG_CEILING_SYSTEM       (LOG_STACK_PERIOD_MS / SYSTEM_TASK_INTERVAL_MS)        /**< 100 */
#define LOG_CEILING_COMM         (LOG_STACK_PERIOD_MS / COMM_TASK_INTERVAL_MS)          /**< 100 */
/** @} */

/** @brief Log tag string used for verbose sensor debug lines. */
#define TAG_DEBUG "[DEBUG]"

/**
 * @brief Log stack and heap usage for the calling task every @p ceiling invocations.
 *
 * @param[in,out] counter Incremented each call; logs when @c (++*counter % ceiling) == 0 (avoid @p ceiling == 0).
 * @param[in] ceiling Period between log lines (e.g. 10 or 50 depending on task loop rate).
 * @param[in] TAG Reserved for call-site consistency; current implementation logs with @c TAG_DEBUG.
 * @param[in] task_stack_size Total stack size of this task in words (for "used" display).
 */
void logTaskStackHeapUsage(uint32_t *counter, uint32_t ceiling, const char *TAG, UBaseType_t task_stack_size);

/**
 * @brief Periodic “task alive” heartbeat at @c ESP_LOGI level using the given @p TAG.
 *
 * @param[in] TAG ESP-IDF log tag for the message (visible at default @c INFO level).
 * @param[in,out] alive_counter Incremented each call; logs when @c (++*alive_counter % ceiling) == 0.
 * @param[in] ceiling Number of calls between two “Task alive” lines.
 *
 * @note @p ceiling must not be 0.
 * @note First log occurs when the counter reaches @p ceiling, not on the first call (after increment).
 */
void logTaskAlive(const char *TAG, uint32_t *alive_counter, uint32_t ceiling);


#endif // __TASK_CONFIG_H__