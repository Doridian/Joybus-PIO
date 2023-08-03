#pragma once

#include <stdint.h>

#include "joybus_pio.hpp"

typedef struct __attribute__((packed)) GCControllerState {
    uint16_t buttons;
    uint8_t joystick_x;
    uint8_t joystick_y;
    uint8_t cstick_x;
    uint8_t cstick_y;
    uint8_t analog_l;
    uint8_t analog_r;
    uint8_t reserved[2];
} GCControllerState;

#define GC_POLL_FLAG_RUMBLE 0b00000001
#define GC_POLL_FLAG_RUMBLE_BEFORE 0b00000010

GCControllerState joybus_gc_probe_origin(JoybusPIOInstance instance);
GCControllerState joybus_gc_recalibrate(JoybusPIOInstance instance);
GCControllerState joybus_gc_short_poll(JoybusPIOInstance instance, uint8_t flags);
