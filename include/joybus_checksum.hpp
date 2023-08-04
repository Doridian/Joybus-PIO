#pragma once

#include <stdint.h>

uint32_t joybus_checksummed_address(unsigned int address);
uint8_t joybus_data_checksum(uint8_t data[], int len);
