#pragma once

#include <Arduino.h>

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
int transmit_receive(N64PIOInstance instance, byte payload[], byte response[], uint payload_len, uint response_len);
