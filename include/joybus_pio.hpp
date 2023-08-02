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

typedef struct __attribute__((__packed__))  JoybusControllerInfo {
    uint16_t type;
    uint8_t aux;
} JoybusControllerInfo;

#define UINT16_FIX_ENDIAN(v) (((v >> 8) & 0x00FF) | ((v << 8) & 0xFF00))

JoybusPIOInstance joybus_pio_program_init(PIO pio, uint sm, uint pin);
int joybus_pio_transmit_receive(JoybusPIOInstance instance, uint8_t payload[], int payload_len, uint8_t response[], int response_len);
void joybus_pio_reset(JoybusPIOInstance instance);

JoybusControllerInfo joybus_init(JoybusPIOInstance instance, bool reset);
