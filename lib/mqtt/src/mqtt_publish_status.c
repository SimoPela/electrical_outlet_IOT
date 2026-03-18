#include "mqtt_publish.h"
#include "mqtt_publish_internal.h"
#include "mqtt_topic.h"
#include "mqtt_payload.h"

int mqtt_publish_system(esp_mqtt_client_handle_t client,
                        const char *device_id,
                        const device_state_t *state)
{
    return mqtt_publish_with_builder(client, device_id,
                                     mqtt_topic_system,
                                     mqtt_payload_build_system,
                                     state, 1, 1);
}

int mqtt_publish_faults(esp_mqtt_client_handle_t client,
                        const char *device_id,
                        const device_state_t *state)
{
    return mqtt_publish_with_builder(client, device_id,
                                     mqtt_topic_faults,
                                     mqtt_payload_build_faults,
                                     state, 1, 1);
}

int mqtt_publish_validity(esp_mqtt_client_handle_t client,
                          const char *device_id,
                          const device_state_t *state)
{
    return mqtt_publish_with_builder(client, device_id,
                                     mqtt_topic_validity,
                                     mqtt_payload_build_validity,
                                     state, 1, 1);
}

int mqtt_publish_last_update(esp_mqtt_client_handle_t client,
                             const char *device_id,
                             const device_state_t *state)
{
    return mqtt_publish_with_builder(client, device_id,
                                     mqtt_topic_last_update,
                                     mqtt_payload_build_last_update,
                                     state, 0, 0);
}

int mqtt_publish_availability(esp_mqtt_client_handle_t client,
                              const char *device_id,
                              bool online)
{
    char topic[128];
    char payload[32];

    if (mqtt_topic_availability(topic, sizeof(topic), device_id) < 0)
        return -1;

    if (mqtt_payload_build_availability(payload, sizeof(payload), online) < 0)
        return -1;

    return mqtt_publish_raw(client, topic, payload, 1, 1);
}

