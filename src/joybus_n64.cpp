#include "joybus_n64.hpp"
#include "joybus_pio.hpp"
#include <string.h>
#include <Arduino.h>

static uint8_t address_xor_table[] = {
  0x01, 0x1A, 0x0D, 0x1C, 0x0E, 0x07, 0x19, 0x16, 0x0B, 0x1F, 0x15
};

static uint32_t make_address_checksummed(uint address) {
  uint32_t rpos = address << 5; // 32?
  for (int i = 15; i >= 5; i--) {
    if (rpos & (0b1 << i)) {
      rpos ^= address_xor_table[15-i];
    }
  }
  return rpos;
}

static uint8_t make_data_checksum(uint8_t data[], int len) {
  uint32_t generator = 0x185;
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

int joybus_n64_read_memory(JoybusPIOInstance instance, uint address, uint8_t response[]) {
  uint32_t address_checksummed = make_address_checksummed(address);
  uint8_t payload[3] = { 0x02, (uint8_t)(address_checksummed >> 8), (uint8_t)(address_checksummed & 0xFF) };
  int result = joybus_pio_transmit_receive(instance, payload, 3, response, N64_BLOCK_SIZE+1);
  if (result != N64_BLOCK_SIZE+1) {
    if (result < 0) {
      return result;
    }
    Serial.println(result, HEX);
    return -20;
  }
  if (make_data_checksum(response, N64_BLOCK_SIZE) != response[N64_BLOCK_SIZE]) {
    return -30;
  }
  return N64_BLOCK_SIZE;
}

int joybus_n64_write_memory(JoybusPIOInstance instance, uint address, uint8_t data[]) {
  uint32_t address_checksummed = make_address_checksummed(address);
  uint8_t payload[N64_BLOCK_SIZE+3] = { 0x03, (uint8_t)(address_checksummed >> 8), (uint8_t)(address_checksummed & 0xFF) };
  memcpy(payload + 3, data, N64_BLOCK_SIZE);
  uint8_t checksum = make_data_checksum(data, N64_BLOCK_SIZE);
  uint8_t response;
  int result = joybus_pio_transmit_receive(instance, payload, N64_BLOCK_SIZE+3, &response, 1);
  if (result != 1) {
    if (result < 0) {
      return result;
    }
    return -20;
  }

  if (response != checksum) {
    return -30;
  }
  return result;
}

N64ControllerState joybus_n64_read_controller(JoybusPIOInstance instance) {
  N64ControllerState state;
  uint8_t payload[] = { 0x01 };
  state.buttons = UINT16_FIX_ENDIAN(state.buttons);
  state.valid = joybus_pio_transmit_receive(instance, payload, 1, (uint8_t*)&state, 4) == 4;
  return state;
}
