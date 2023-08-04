#include "joybus_checksum.hpp"

static const uint8_t address_xor_table[] = {0x01, 0x1A, 0x0D, 0x1C, 0x0E, 0x07,
                                      0x19, 0x16, 0x0B, 0x1F, 0x15};

uint32_t joybus_checksummed_address(unsigned int address) {
  uint32_t rpos = address << 5; // 32?
  for (int i = 15; i >= 5; i--) {
    if (rpos & (0b1 << i)) {
      rpos ^= address_xor_table[15 - i];
    }
  }
  return rpos;
}

static const uint32_t generator = 0x185;
uint8_t joybus_data_checksum(uint8_t data[], int len) {
  uint32_t crc = 0x00;

  for (int i = 0; i < len; i++) {
    crc ^= data[i]; // << (n - 8) with n = 8

    for (int b = 0; b < 8; b++) {
      crc <<= 1;
      if (crc & (0b1 << 8)) {
        crc ^= generator;
      }
    }
  }

  return crc & 0xFF;
}
