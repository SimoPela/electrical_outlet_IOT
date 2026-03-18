#ifndef MQTT_APP_H
#define MQTT_APP_H

#include "mqtt_client.h"

extern esp_mqtt_client_handle_t g_mqtt_client;

void mqtt_app_start(void);

#endif // MQTT_APP_H