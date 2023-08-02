#include <Arduino.h>

#include "n64pio.hpp"

N64PIOInstance n64pio;
void setup() {

}

void setup1() {
  Serial.begin(115200);
  Serial.println("HI");
  
  n64pio = n64pio_program_init(pio0, 0, 16);
}

uint8_t address_xor_table[] = {
  0x01, 0x1A, 0x0D, 0x1C, 0x0E, 0x07, 0x19, 0x16, 0x0B, 0x1F, 0x15
};

uint32_t make_address(int pos) {
  uint32_t rpos = pos << 5; // 32?
  uint8_t csum = 0x00;
  for (int i = 15; i >= 5; i--) {
    if (rpos & (0b1 << i)) {
      csum ^= address_xor_table[15-i];
    }
  }
  rpos |= csum;
  return rpos;
}

#define PAK_SIZE (32*1024)
#define BLOCK_SIZE 32
#define BLOCK_COUNT (PAK_SIZE/BLOCK_SIZE)

void loop1() {
  byte payload[64] = {};
  byte res[64] = {};
  const uint sm = 0;

  delay(1000);
  Serial.print("Initializing...");
  payload[0] = 0x00;
  int res_size = n64pio_transmit_receive(n64pio, payload, res, 1, 3);
  if (res_size <= 0) {
    Serial.println(res_size);
    return;
  }
  Serial.print("Receiving: ");
  Serial.print(res[0], HEX);
  Serial.print(" ");
  Serial.print(res[1], HEX);
  Serial.print(" ");
  Serial.print(res[2], HEX);
  Serial.print(" ");
  Serial.println(" Done!");

  if (res[0] != 0x05) {
    return;
  }

  if (true) {
    return;
  }

  for (int a = 0; a < 5; a++) {
    uint32_t addr = make_address(a);

    /*
    delay(1);

    Serial.print("Writing... | ");
    Serial.print(a, HEX);
    Serial.print(" | ");
    payload[0] = 0x03;
    payload[1] = addr >> 8;
    payload[2] = addr & 0xFF;
    for (int i = 0; i < BLOCK_SIZE+1; i++) {
      
      payload[i + 3] = (i % 2 == 0) ? 0x55 : 0xAA;
      //payload[i + 3] = 0x00;
      Serial.print(".");
    }
    payload[BLOCK_SIZE+1+3] = 0x5F;
    //payload[BLOCK_SIZE+1+3] = 0x00;
    res_size = tx_data(payload, res, BLOCK_SIZE+1+3, 1);
    if (!res_size) {
      Serial.println(res_size);
      return;
    }
    Serial.print("!");
    Serial.println("| Done!");
    */

    delay(1);

    Serial.print("Reading... | ");
    Serial.print(a, HEX);
    Serial.print(" | ");
    payload[0] = 0x02;
    payload[1] = addr >> 8;
    payload[2] = addr & 0xFF;
    res_size = n64pio_transmit_receive(n64pio, payload, res, 3, BLOCK_SIZE+1);
    if (!res_size) {
      Serial.println(res_size);
      return;
    }
    for (int i = 0; i < BLOCK_SIZE+1; i++) {
      Serial.print(res[i], HEX);
      Serial.print(" ");
    }
    Serial.println("| Done!");
  }
}

void loop() { }

/*
// 0 = low 3us, high 1us
// 1 = low 1us, high 3us

// When line pulled low, wait 2us, sample, wait 2us, total 4us
// IDENT | 0x00 |  0 | 3
// INPUT | 0x01 |  0 | 4
// RDMEM | 0x02 |  2 | 32
// WRMEM | 0x03 | 34 | 1/2?
*/
