#include "as312.h"
#include "esp32_pinout.h"
#include "driver/gpio.h"

bool as312_read_motion(void)
{
    return gpio_get_level(PIN_PIR_OUT) != 0;
}