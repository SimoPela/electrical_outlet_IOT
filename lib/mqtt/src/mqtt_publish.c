/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file mqtt_publish.c
 * @brief High-level MQTT publish helpers: environment telemetry, system status, and alarm events.
 */

#include "mqtt_publish.h"
#include "mqtt_topic.h"
#include "mqtt_payload.h"
#include "app_config.h"
#include "esp_log.h"

static const char *TAG = "MQTT_PUBLISH";

/** @copydoc mqtt_publish_raw */
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

    ESP_LOGD(TAG, "Published [%s]: %s", topic, payload);
    return msg_id;
}

/** @copydoc mqtt_publish_with_builder */
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

/** @copydoc mqtt_publish_system */
int mqtt_publish_system(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state)
{
    char topic[APP_MQTT_TOPIC_MAX_LEN];
    char payload[APP_MQTT_PAYLOAD_MAX_LEN];

    if (mqtt_topic_system(topic, sizeof(topic), device_id) < 0)
    {
        return -1;
    }

    if (mqtt_payload_build_system(payload, sizeof(payload), state, state->mqtt_connected) < 0)
    {
        return -1;
    }

    return mqtt_publish_raw(client, topic, payload, APP_QOS_1, APP_RETAIN_1);
}

/** @copydoc mqtt_publish_environment */
int mqtt_publish_environment(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state)
{
    return mqtt_publish_with_builder(client, device_id,
                                     mqtt_topic_environment,
                                     mqtt_payload_build_environment,
                                     state, APP_QOS_0, APP_RETAIN_0);
}

/** @copydoc mqtt_publish_alarm */
int mqtt_publish_alarm(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state)
{
    return mqtt_publish_with_builder(client, device_id,
                    mqtt_topic_alarm,
                    mqtt_payload_build_alarm,
                    state, APP_QOS_1, APP_RETAIN_0);
}

/**
 * @brief Publish system status, environment telemetry, and alarm state in one call.
 *
 * Convenience wrapper used for periodic batch publishing.  Each publish is
 * attempted independently; a single failure does not block the others.
 *
 * @param client    Active MQTT client handle.
 * @param device_id Logical device identifier used in topic construction.
 * @param state     Read-only snapshot of the current device state.
 * @return 0 if all three publishes succeeded, non-zero if any failed.
 */
int mqtt_publish_all_periodic(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state)
{
    int rc = 0;

    rc |= mqtt_publish_system(client, device_id, state);
    rc |= mqtt_publish_environment(client, device_id, state);
    rc |= mqtt_publish_alarm(client, device_id, state);

    return rc;
}
