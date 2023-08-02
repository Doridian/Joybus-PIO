#pragma once

#include <stdint.h>

#include "joybus_pio.hpp"

#define N64_PAK_SIZE (32*1024)
#define N64_BLOCK_SIZE 32
#define N64_BLOCK_COUNT (PAK_SIZE/BLOCK_SIZE)

int joybus_n64_read_memory(JoybusPIOInstance instance, uint address, uint8_t response[]);
int joybus_n64_write_memory(JoybusPIOInstance instance, uint address, uint8_t data[]);
