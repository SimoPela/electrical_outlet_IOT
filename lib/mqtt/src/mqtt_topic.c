/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file mqtt_topic.c
 * @brief MQTT topic string builders under @c devices/<device_id>/...
 */

#include "mqtt_topic.h"
#include <stdio.h>
#include "esp_log.h"

static const char *TAG = "MQTT_TOPIC";

/** Root segment for all device topics. */
#define MQTT_BASE "devices"

/** @copydoc mqtt_topic_build */
int mqtt_topic_build(char *buffer, size_t size, const char *device_id, const char *suffix)
{
    if (!buffer || !device_id || !suffix || size == 0)
    {
        ESP_LOGE(TAG, "MQTT topic build failed: buffer=%p device_id=%p suffix=%p size=%zu", buffer, device_id, suffix, size);
        return -1;
    }

    int written = snprintf(buffer, size, MQTT_BASE "/%s/%s", device_id, suffix);

    if (written < 0 || (size_t)written >= size)
    {
        ESP_LOGE(TAG, "MQTT topic build failed: written = %d, size = %zu", written, size);
        return -1;
    }
    return written;
}

/** @copydoc mqtt_topic_environment */
int mqtt_topic_environment(char *buffer, size_t size, const char *device_id)
{
    return mqtt_topic_build(buffer, size, device_id, "telemetry/environment");
}

/** @copydoc mqtt_topic_system */
int mqtt_topic_system(char *buffer, size_t size, const char *device_id)
{
    return mqtt_topic_build(buffer, size, device_id, "status/system");
}

/** @copydoc mqtt_topic_alarm */
int mqtt_topic_alarm(char *buffer, size_t size, const char *device_id)
{
    return mqtt_topic_build(buffer, size, device_id, "event/alarm");
}
