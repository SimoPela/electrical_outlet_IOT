/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file audio_task.h
 * @brief FreeRTOS audio task: INMP441 I2S capture and SPL estimate.
 */

#ifndef AUDIO_TASK_H
#define AUDIO_TASK_H

/** @brief Last computed ambient noise level from the microphone path. */
typedef struct {
    float noise_db; /**< Estimated sound pressure level [dB], datasheet formula + trim. */
} audio_local_state_t;

/**
 * @brief FreeRTOS task entry: reads INMP441 via @c inmp441_w_read and updates @c g_device_state.
 * @param pvParameters Unused (NULL).
 */
void audio_task(void *pvParameters);

#endif