#ifndef LED_RGB_H
#define LED_RGB_H

#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"


typedef struct {
    int red_gpio;
    int green_gpio;
    int blue_gpio;
    int active_high;   // 1 = common cathode (HIGH accende), 0 = common anode (LOW accende)
} led_rgb_cfg_t;

void led_rgb_task(void *pv); // pv -> led_rgb_cfg_t*

#endif // LED_RGB_H