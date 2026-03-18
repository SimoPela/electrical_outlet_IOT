/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#ifndef MQTT_PUBLISH_INTERNAL_H
#define MQTT_PUBLISH_INTERNAL_H

#include "mqtt_client.h"
#include "device_state.h"

int mqtt_publish_raw(esp_mqtt_client_handle_t client, const char *topic, const char *payload, int qos, int retain);

int mqtt_publish_with_builder(esp_mqtt_client_handle_t client,
                              const char *device_id,
                              int (*topic_builder)(char *, size_t, const char *),
                              int (*payload_builder)(char *, size_t, const device_state_t *),
                              const device_state_t *state,
                              int qos,
                              int retain);

#endif // MQTT_PUBLISH_INTERNAL_H