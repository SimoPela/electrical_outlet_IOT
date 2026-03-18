/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "mqtt_publish.h"
#include "mqtt_publish_internal.h"
#include "mqtt_topic.h"
#include "mqtt_payload.h"

int mqtt_publish_all_periodic(esp_mqtt_client_handle_t client,
    const char *device_id,
    const device_state_t *state)
{
int rc = 0;

rc |= mqtt_publish_state(client, device_id, state);
rc |= mqtt_publish_system(client, device_id, state);
rc |= mqtt_publish_environment(client, device_id, state);
rc |= mqtt_publish_audio(client, device_id, state);
rc |= mqtt_publish_faults(client, device_id, state);
rc |= mqtt_publish_validity(client, device_id, state);
rc |= mqtt_publish_last_update(client, device_id, state);

return rc;
}