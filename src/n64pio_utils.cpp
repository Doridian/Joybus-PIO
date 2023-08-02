#include <n64pio_utils.hpp>
#include <n64pio.hpp>
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

int n64pio_read_memory(N64PIOInstance instance, uint address, uint8_t response[]) {
  uint32_t address_checksummed = make_address_checksummed(address);
  uint8_t payload[3] = { 0x02, (uint8_t)(address_checksummed >> 8), (uint8_t)(address_checksummed & 0xFF) };
  return n64pio_transmit_receive(instance, payload, response, 3, N64_BLOCK_SIZE+1);
}

int n64pio_write_memory(N64PIOInstance instance, uint address, uint8_t data[]) {
  uint32_t address_checksummed = make_address_checksummed(address);
  uint8_t payload[(N64_BLOCK_SIZE+1)+3] = { 0x03, (uint8_t)(address_checksummed >> 8), (uint8_t)(address_checksummed & 0xFF) };
  memcpy(payload + 3, data, N64_BLOCK_SIZE);
  payload[(N64_BLOCK_SIZE+1)+2] = 0x00; // TODO: Checksum!

  uint8_t response;
  return n64pio_transmit_receive(instance, payload, &response, (N64_BLOCK_SIZE+1)+3, 1);
}