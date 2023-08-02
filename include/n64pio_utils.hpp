#pragma once

#include <stdint.h>

#include "n64pio.hpp"

#define N64_PAK_SIZE (32*1024)
#define N64_BLOCK_SIZE 32
#define N64_BLOCK_COUNT (PAK_SIZE/BLOCK_SIZE)

int n64pio_read_memory(N64PIOInstance instance, uint address, uint8_t response[]);
int n64pio_write_memory(N64PIOInstance instance, uint address, uint8_t data[]);
