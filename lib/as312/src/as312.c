/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */


#include "as312.h"
#include "esp32_pinout.h"
#include "gpio_init.h"
#include "driver/gpio.h"

bool as312_read_motion(void)
{
    return gpio_get_level(PIN_PIR_OUT) != 0;
}

esp_err_t as312_restore(void)
{
    return gpio_init_all();
}