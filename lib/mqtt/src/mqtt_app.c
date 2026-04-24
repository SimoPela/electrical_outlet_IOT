/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

/**
 * @file mqtt_app.c
 * @brief MQTT client lifecycle implementation: global handle, event handler, and startup.
 */

#include "mqtt_app.h"
#include "app_config.h"

#include "device_state.h"

#include "esp_event.h"
#include "esp_log.h"
#include "mqtt_client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include <inttypes.h>

static const char *TAG = "MQTT_APP";

/** @brief Global MQTT client handle (NULL before @ref mqtt_app_start succeeds). */
esp_mqtt_client_handle_t g_mqtt_client = NULL;

/**
 * @brief Write @c mqtt_connected in @c g_device_state under the device-state mutex.
 *
 * @param[in] connected New MQTT connection state.
 */
static void mqtt_set_connected_flag(bool connected)
{
    if (xSemaphoreTake(g_device_state_mutex, portMAX_DELAY) == pdTRUE)
    {
        g_device_state.mqtt_connected = connected;
        xSemaphoreGive(g_device_state_mutex);
    }
    else
    {
        ESP_LOGW(TAG, "Failed to lock device state mutex while updating mqtt_connected");
    }
}

/**
 * @brief ESP-IDF MQTT event handler registered in @ref mqtt_app_start.
 *
 * Updates @c mqtt_connected on connect/disconnect/error events and logs
 * subscriptions, publications, and incoming data at DEBUG level.
 *
 * @param handler_args Unused.
 * @param base         Event base (unused; always @c MQTT_EVENTS).
 * @param event_id     @c esp_mqtt_event_id_t cast to int32_t.
 * @param event_data   Pointer to @c esp_mqtt_event_t.
 */
static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    (void)handler_args;
    (void)base;

    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id)
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGD(TAG, "MQTT connected");
            mqtt_set_connected_flag(true);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT disconnected");
            mqtt_set_connected_flag(false);
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGD(TAG, "MQTT subscribed, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGD(TAG, "MQTT unsubscribed, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGD(TAG, "MQTT published, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGD(TAG, "MQTT data received");
            ESP_LOGD(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
            ESP_LOGD(TAG, "DATA=%.*s", event->data_len, event->data);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT event error");
            mqtt_set_connected_flag(false);
            break;

        default:
            ESP_LOGD(TAG, "MQTT other event id=%" PRId32, event_id);
            break;
    }
}

/** @copydoc mqtt_app_start */
void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = APP_MQTT_BROKER_URI,
    };

    if (APP_MQTT_USERNAME[0] != '\0')
    {
        mqtt_cfg.credentials.username = APP_MQTT_USERNAME;
    }

    if (APP_MQTT_PASSWORD[0] != '\0')
    {
        mqtt_cfg.credentials.authentication.password = APP_MQTT_PASSWORD;
    }

    g_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

    if (g_mqtt_client == NULL)
    {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        mqtt_set_connected_flag(false);
        return;
    }

    esp_err_t err = esp_mqtt_client_register_event(g_mqtt_client,
                                                   ESP_EVENT_ANY_ID,
                                                   mqtt_event_handler,
                                                   NULL);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register MQTT event handler");
        mqtt_set_connected_flag(false);
        return;
    }

    err = esp_mqtt_client_start(g_mqtt_client);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start MQTT client");
        mqtt_set_connected_flag(false);
        return;
    }

    ESP_LOGI(TAG, "MQTT client started");
}
