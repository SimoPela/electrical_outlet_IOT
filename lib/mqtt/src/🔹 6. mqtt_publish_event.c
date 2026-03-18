#include "mqtt_publish.h"
#include "mqtt_publish_internal.h"
#include "mqtt_topic.h"
#include "mqtt_payload.h"

int mqtt_publish_alarm(esp_mqtt_client_handle_t client,
                       const char *device_id,
                       const device_state_t *state)
{
    return mqtt_publish_with_builder(client, device_id,
                                     mqtt_topic_alarm,
                                     mqtt_payload_build_alarm,
                                     state, 1, 0);
}