#include <Arduino.h>

#include "n64pio.hpp"
#include "n64pio_utils.hpp"

N64PIOInstance n64pio;
void setup() {

}

void setup1() {
  Serial.begin(115200);
  Serial.println("HI");
  
  n64pio = n64pio_program_init(pio0, 0, 16);
}

void loop1() {
  uint8_t payload[64] = {};
  uint8_t res[64] = {};
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

  for (int a = 0; a < 5; a++) {
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
    res_size = n64pio_read_memory(n64pio, a, res);
    if (!res_size) {
      Serial.println(res_size);
      return;
    }
    for (int i = 0; i < N64_BLOCK_SIZE+1; i++) {
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
