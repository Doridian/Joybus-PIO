#pragma once

#include <stdint.h>

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

typedef struct N64PIOInstance {
    PIO pio;
    uint sm;
    uint offset;
    uint pin;
    pio_sm_config config;
} N64PIOInstance;

N64PIOInstance n64pio_program_init(PIO pio, uint sm, uint pin);
int n64pio_transmit_receive(N64PIOInstance instance, uint8_t payload[], uint8_t response[], int payload_len, int response_len);
void n64pio_reset(N64PIOInstance instance);
