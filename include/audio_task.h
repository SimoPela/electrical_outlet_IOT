/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#ifndef AUDIO_TASK_H
#define AUDIO_TASK_H

typedef struct{
    // -----------------------------
    // Audio state
    // -----------------------------
    float noise_db;
} audio_local_state_t;

void audio_task(void *pvParameters);

#endif