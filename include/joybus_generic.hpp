#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "joybus_pio.hpp"

#define JOYBUS_BLOCK_SIZE 32

typedef struct __attribute__((__packed__)) JoybusControllerInfo {
  uint16_t type;
  uint8_t aux;
} JoybusControllerInfo;

JoybusControllerInfo joybus_handshake(JoybusPIOInstance instance, bool reset);

int joybus_read_memory(JoybusPIOInstance instance, uint8_t command, uint address,
                       uint8_t response[]);
int joybus_write_memory(JoybusPIOInstance instance, uint8_t command, uint address,
                        uint8_t data[]);
