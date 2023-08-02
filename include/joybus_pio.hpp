#pragma once

#include <stdint.h>

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

typedef struct JoybusPIOInstance {
    PIO pio;
    uint sm;
    uint offset;
    uint pin;
    pio_sm_config config;
} JoybusPIOInstance;

JoybusPIOInstance joybus_pio_program_init(PIO pio, uint sm, uint pin);
int joybus_pio_transmit_receive(JoybusPIOInstance instance, uint8_t payload[], int payload_len, uint8_t response[], int response_len);
void joybus_pio_reset(JoybusPIOInstance instance);
