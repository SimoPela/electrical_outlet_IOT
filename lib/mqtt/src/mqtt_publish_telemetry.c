#include "mqtt_publish.h"
#include "mqtt_publish_internal.h"
#include "mqtt_topic.h"
#include "mqtt_payload.h"

int mqtt_publish_environment(esp_mqtt_client_handle_t client,
                             const char *device_id,
                             const device_state_t *state)
{
    return mqtt_publish_with_builder(client, device_id,
                                     mqtt_topic_environment,
                                     mqtt_payload_build_environment,
                                     state, 0, 0);
}

int mqtt_publish_audio(esp_mqtt_client_handle_t client,
                       const char *device_id,
                       const device_state_t *state)
{
    return mqtt_publish_with_builder(client, device_id,
                                     mqtt_topic_audio,
                                     mqtt_payload_build_audio,
                                     state, 0, 0);
}

int mqtt_publish_state(esp_mqtt_client_handle_t client,
                       const char *device_id,
                       const device_state_t *state)
{
    return mqtt_publish_with_builder(client, device_id,
                                     mqtt_topic_state,
                                     mqtt_payload_build_state,
                                     state, 1, 0);
}