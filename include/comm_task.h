/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file comm_task.h
 * @brief FreeRTOS communication task: MQTT telemetry and events.
 */

#ifndef COMM_TASK_H
#define COMM_TASK_H

/**
 * @brief FreeRTOS task entry: snapshots @c g_device_state and publishes MQTT messages.
 * @param pvParameters Unused (NULL).
 */
void comm_task(void *pvParameters);

#endif