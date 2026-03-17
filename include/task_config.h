/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#ifndef __TASK_CONFIG_H__
#define __TASK_CONFIG_H__

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/*
Stack sizes (in WORDS, not bytes)
1 word = 4 bytes on ESP32
*/

#define STACK_ACQUISITION_WORDS     4096    // 16 KB
#define STACK_AUDIO_WORDS           4096    
#define STACK_SYSTEM_WORDS          4096    
#define STACK_COMM_WORDS            4096    

// Task priorities
#define ACQUISITION_TASK_PRIORITY   2
#define AUDIO_TASK_PRIORITY         2
#define SYSTEM_TASK_PRIORITY        4
#define COMM_TASK_PRIORITY          3

//Sensor polling intervals
#define AS312_INTERVAL_MS      100
#define MICS5524_INTERVAL_MS   1000
#define SGP41_INTERVAL_MS      1000
#define SHT41_INTERVAL_MS      2000
#define BMP280_INTERVAL_MS     2000
#define AS7341_INTERVAL_MS     3000
#define SCD40_INTERVAL_MS      5000
#define PMS7003_INTERVAL_MS    5000

/**
 * @brief Log the stack usage of the current task periodically.
 *
 * This function prints the used and remaining stack space every 10 calls,
 * based on the provided counter value.
 *
 * @param counter Pointer to the call counter used to decide when to print.
 * @param TAG Logging tag used by ESP_LOGI.
 * @param task_stack_size Total stack size of the task in words.
 */
void logTaskStackUsage(uint32_t *counter, const char *TAG, UBaseType_t task_stack_size);

#endif // __TASK_CONFIG_H__