#include "joybus_n64.hpp"
#include "joybus_pio.hpp"
#include <string.h>

static uint8_t address_xor_table[] = {
  0x01, 0x1A, 0x0D, 0x1C, 0x0E, 0x07, 0x19, 0x16, 0x0B, 0x1F, 0x15
};

static uint32_t make_address_checksummed(uint address) {
  uint32_t rpos = address << 5; // 32?
  uint8_t csum = 0x00;
  for (int i = 15; i >= 5; i--) {
    if (rpos & (0b1 << i)) {
      csum ^= address_xor_table[15-i];
    }
  }
  rpos |= csum;
  return rpos;
}

static uint8_t make_data_checksum(uint8_t data[]) {
  uint32_t generator = 0x185;
  uint32_t crc = 0x00;

  for (int i = 0; i < N64_BLOCK_SIZE; i++) {
    crc ^= data[i]; // << (n - 8) with n = 8

    for (int b = 0; b < 7; b++) {
      if (crc & 0b10000000) {
        crc ^= generator;
      }
      crc <<= 1;
    }
  }

  return crc & 0xFF;
}

int joybus_n64_read_memory(JoybusPIOInstance instance, uint address, uint8_t response[]) {
  uint32_t address_checksummed = make_address_checksummed(address);
  uint8_t payload[3] = { 0x02, (uint8_t)(address_checksummed >> 8), (uint8_t)(address_checksummed & 0xFF) };
  int len = joybus_pio_transmit_receive(instance, payload, response, 3, N64_BLOCK_SIZE+1);
  if (len != N64_BLOCK_SIZE+1) {
    if (len < 0) {
      return len;
    }
    return -2;
  }
  if (make_data_checksum(response) != response[N64_BLOCK_SIZE+1]) {
    return -3;
  }
  return N64_BLOCK_SIZE;
}

int joybus_n64_write_memory(JoybusPIOInstance instance, uint address, uint8_t data[]) {
  uint32_t address_checksummed = make_address_checksummed(address);
  uint8_t payload[(N64_BLOCK_SIZE+1)+3] = { 0x03, (uint8_t)(address_checksummed >> 8), (uint8_t)(address_checksummed & 0xFF) };
  memcpy(payload + 3, data, N64_BLOCK_SIZE);
  payload[(N64_BLOCK_SIZE+1)+2] = make_data_checksum(data);
  uint8_t response;
  return joybus_pio_transmit_receive(instance, payload, &response, (N64_BLOCK_SIZE+1)+3, 1);
}
