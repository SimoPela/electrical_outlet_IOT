#ifndef MQTT_PUBLISH_H
#define MQTT_PUBLISH_H

#include <stddef.h>
#include <stdbool.h>

/* ---- TELEMETRY ---- */
int mqtt_publish_environment(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

int mqtt_publish_audio(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

int mqtt_publish_state(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

/* ---- STATUS ---- */
int mqtt_publish_system(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

int mqtt_publish_faults(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

int mqtt_publish_validity(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

int mqtt_publish_last_update(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

int mqtt_publish_availability(esp_mqtt_client_handle_t client, const char *device_id, bool online);

/* ---- EVENTS ---- */
int mqtt_publish_alarm(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

/* ---- GROUP ---- */
int mqtt_publish_all_periodic(esp_mqtt_client_handle_t client, const char *device_id, const device_state_t *state);

#endif // MQTT_PUBLISH_H