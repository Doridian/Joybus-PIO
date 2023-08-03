#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "joybus_pio.hpp"

typedef struct __attribute__((__packed__)) JoybusControllerInfo {
  uint16_t type;
  uint8_t aux;
} JoybusControllerInfo;

JoybusControllerInfo joybus_handshake(JoybusPIOInstance instance, bool reset);
