#pragma once

#include <stdint.h>

#include "joybus_pio.hpp"

int joybus_gba_write(JoybusPIOInstance instance, uint8_t data[]);
int joybus_gba_read(JoybusPIOInstance instance, uint8_t data[]);
int joybus_gba_transact(JoybusPIOInstance instance, uint8_t send[], uint8_t recv[]);
int joybus_gba_poll(JoybusPIOInstance instance, uint8_t send[], uint8_t recv[], int retries);
