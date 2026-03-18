/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "mqtt_publish_internal.h"
#include "mqtt_topic.h"
#include "mqtt_payload.h"
#include "esp_log.h"
#include <stdio.h>

#include "app_config.h"

static const char *TAG = "MQTT_PUBLISH";

int mqtt_publish_raw(esp_mqtt_client_handle_t client, const char *topic, const char *payload, int qos, int retain)
{
    if (!client || !topic || !payload)
    {
        ESP_LOGE(TAG, "Invalid publish args");
        return -1;
    }

    int msg_id = esp_mqtt_client_publish(client, topic, payload, 0, qos, retain);

    if (msg_id < 0)
    {
        ESP_LOGE(TAG, "Publish failed: %s", topic);
        return -1;
    }

    ESP_LOGI(TAG, "Published [%s]: %s", topic, payload);
    return msg_id;
}

int mqtt_publish_with_builder(esp_mqtt_client_handle_t client,
                              const char *device_id,
                              int (*topic_builder)(char *, size_t, const char *),
                              int (*payload_builder)(char *, size_t, const device_state_t *),
                              const device_state_t *state,
                              int qos,
                              int retain)
{
    char topic[APP_MQTT_TOPIC_MAX_LEN];
    char payload[APP_MQTT_PAYLOAD_MAX_LEN];

    if (!client || !device_id || !topic_builder || !payload_builder || !state)
    {
        ESP_LOGE(TAG, "Invalid publish args");
        return -1;
    }

    if (topic_builder(topic, sizeof(topic), device_id) < 0)
    {
        ESP_LOGE(TAG, "Topic builder failed");
        return -1;
    }

    if (payload_builder(payload, sizeof(payload), state) < 0)
    {
        ESP_LOGE(TAG, "Payload builder failed");
        return -1;
    }

    return mqtt_publish_raw(client, topic, payload, qos, retain);
}