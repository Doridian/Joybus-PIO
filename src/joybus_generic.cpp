#include "joybus_generic.hpp"
#include "joybus_checksum.hpp"

#include <string.h>

JoybusControllerInfo joybus_handshake(JoybusPIOInstance instance, bool reset) {
  JoybusControllerInfo info;
  info.type = 0x0000;
  uint8_t payload[] = {reset ? (uint8_t)0xFF : (uint8_t)0x00};
  joybus_pio_transmit_receive(instance, payload, 1, (uint8_t *)&info, 3);
  info.type = UINT16_FIX_ENDIAN(info.type);
  return info;
}

int joybus_read_memory(JoybusPIOInstance instance, uint8_t command, uint address,
                           uint8_t response[]) {
  uint32_t address_checksummed = joybus_checksummed_address(address);
  uint8_t payload[3] = {command, (uint8_t)(address_checksummed >> 8),
                        (uint8_t)(address_checksummed & 0xFF)};
  uint8_t raw_response[JOYBUS_BLOCK_SIZE + 1];
  int result = joybus_pio_transmit_receive(instance, payload, 3, raw_response,
                                           JOYBUS_BLOCK_SIZE + 1);
  if (result != JOYBUS_BLOCK_SIZE + 1) {
    if (result < 0) {
      return result;
    }
    return -20;
  }
  if (joybus_data_checksum(response, JOYBUS_BLOCK_SIZE) !=
      response[JOYBUS_BLOCK_SIZE]) {
    return -30;
  }
  memcpy(response, raw_response, JOYBUS_BLOCK_SIZE);
  return JOYBUS_BLOCK_SIZE;
}

int joybus_write_memory(JoybusPIOInstance instance, uint8_t command, uint address,
                            uint8_t data[]) {
  uint32_t address_checksummed = joybus_checksummed_address(address);
  uint8_t payload[JOYBUS_BLOCK_SIZE + 3] = {command,
                                         (uint8_t)(address_checksummed >> 8),
                                         (uint8_t)(address_checksummed & 0xFF)};
  memcpy(payload + 3, data, JOYBUS_BLOCK_SIZE);
  uint8_t checksum = joybus_data_checksum(data, JOYBUS_BLOCK_SIZE);
  uint8_t response;
  int result = joybus_pio_transmit_receive(instance, payload,
                                           JOYBUS_BLOCK_SIZE + 3, &response, 1);
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