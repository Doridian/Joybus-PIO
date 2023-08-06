#pragma once

#include <stdint.h>

#include "joybus_pio.hpp"

int joybus_gba_unsafe_write(JoybusPIOInstance instance, uint8_t data[]);
int joybus_gba_unsafe_read(JoybusPIOInstance instance, uint8_t data[]);
int joybus_gba_read(JoybusPIOInstance instance, uint8_t data[]);
int joybus_gba_write(JoybusPIOInstance instance, uint8_t data[]);

int joybus_gba_boot(JoybusPIOInstance instance, uint8_t rom[], int rom_len);
int joybus_gba_default_handshake(JoybusPIOInstance instance, uint8_t rom[], int rom_len);
