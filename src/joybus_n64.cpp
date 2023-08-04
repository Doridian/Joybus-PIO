#include <Arduino.h>
#include <string.h>

#include "joybus_n64.hpp"
#include "joybus_pio.hpp"
#include "joybus_checksum.hpp"

int joybus_n64_read_memory(JoybusPIOInstance instance, uint address,
                           uint8_t response[]) {
  uint32_t address_checksummed = joybus_checksummed_address(address);
  uint8_t payload[3] = {0x02, (uint8_t)(address_checksummed >> 8),
                        (uint8_t)(address_checksummed & 0xFF)};
  uint8_t raw_response[N64_BLOCK_SIZE + 1];
  int result = joybus_pio_transmit_receive(instance, payload, 3, raw_response,
                                           N64_BLOCK_SIZE + 1);
  if (result != N64_BLOCK_SIZE + 1) {
    if (result < 0) {
      return result;
    }
    return -20;
  }
  if (joybus_data_checksum(response, N64_BLOCK_SIZE) !=
      response[N64_BLOCK_SIZE]) {
    return -30;
  }
  memcpy(response, raw_response, N64_BLOCK_SIZE);
  return N64_BLOCK_SIZE;
}

int joybus_n64_write_memory(JoybusPIOInstance instance, uint address,
                            uint8_t data[]) {
  uint32_t address_checksummed = joybus_checksummed_address(address);
  uint8_t payload[N64_BLOCK_SIZE + 3] = {0x03,
                                         (uint8_t)(address_checksummed >> 8),
                                         (uint8_t)(address_checksummed & 0xFF)};
  memcpy(payload + 3, data, N64_BLOCK_SIZE);
  uint8_t checksum = joybus_data_checksum(data, N64_BLOCK_SIZE);
  uint8_t response;
  int result = joybus_pio_transmit_receive(instance, payload,
                                           N64_BLOCK_SIZE + 3, &response, 1);
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
  uint8_t payload[] = {0x01};
  state.buttons = UINT16_FIX_ENDIAN(state.buttons);
  state.valid = joybus_pio_transmit_receive(instance, payload, 1,
                                            (uint8_t *)&state, 4) == 4;
  return state;
}
