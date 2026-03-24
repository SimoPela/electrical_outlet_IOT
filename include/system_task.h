/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file system_task.h
 * @brief FreeRTOS system supervision task: health flags, alarms, sensor staleness thresholds.
 *
 * Pulls in @ref health.h for @c health_local_state_t and per-sensor @c *HealthCheck helpers.
 */

#ifndef SYSTEM_TASK_H
#define SYSTEM_TASK_H

#include <stdbool.h>
#include "health.h"

/** @brief Alias for @c health_local_state_t used in @c system_task.c. */
typedef health_local_state_t system_local_state_t;

/**
 * @brief FreeRTOS task entry: reads @c g_device_state, evaluates supervision rules, writes flags back.
 * @param pvParameters Unused (NULL).
 */
void system_task(void *pvParameters);

#endif