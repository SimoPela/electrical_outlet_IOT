/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#ifndef NETWORK_APP_H
#define NETWORK_APP_H

#include <stdbool.h>
#include <stdint.h>

#define APP_WIFI_CONNECTED_BIT BIT0
#define APP_WIFI_FAIL_BIT      BIT1

bool network_app_init(void);
bool network_app_wait_until_connected(uint32_t timeout_ms);

#endif