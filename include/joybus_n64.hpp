#pragma once

#include <stdint.h>

#include "joybus_pio.hpp"

#define N64_PAK_SIZE (32*1024)
#define N64_BLOCK_SIZE 32
#define N64_BLOCK_COUNT (PAK_SIZE/BLOCK_SIZE)

typedef struct N64ControllerState {
    uint16_t buttons;
    int8_t joystick_x;
    int8_t joystick_y;
} N64ControllerState;

int joybus_n64_read_memory(JoybusPIOInstance instance, uint address, uint8_t response[]);
int joybus_n64_write_memory(JoybusPIOInstance instance, uint address, uint8_t data[]);
N64ControllerState joybus_n64_read_controller(JoybusPIOInstance instance);
