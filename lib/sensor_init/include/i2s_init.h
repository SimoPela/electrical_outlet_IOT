/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#ifndef I2S_INIT_H
#define I2S_INIT_H

#include "esp_err.h"
#include "driver/i2s_std.h"

esp_err_t i2s_init_all(void);
i2s_chan_handle_t i2s_get_rx_channel(void);

#endif /* I2S_INIT_H */