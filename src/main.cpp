#include <Arduino.h>

#include "joybus_pio.hpp"
#include "joybus_n64.hpp"

JoybusPIOInstance joybus_pio;
void setup() {

}

void setup1() {
  Serial.begin(115200);
  Serial.println("HI");
  
  joybus_pio = joybus_pio_program_init(pio0, 0, 16);
}

void loop1() {
  delay(1000);
  Serial.print("Initializing...");
  JoybusControllerInfo info = joybus_init(joybus_pio, true);
  Serial.print(" Type: ");
  Serial.print(info.type, HEX);
  Serial.print(" Aux: ");
  Serial.print(info.aux, HEX);
  Serial.println(" Done!");

  if (info.type != 0x0500) {
    return;
  }

  uint8_t payload[64] = {};
  uint8_t res[64] = {};
  int res_size = 0;
  for (int a = 0; a < 5; a++) {
    delay(1);

    Serial.print("Writing... | ");
    Serial.print(a, HEX);
    Serial.print(" | ");
    for (int i = 0; i < N64_BLOCK_SIZE; i++) {
      payload[i] = (i % 2 == 0) ? 0x55 : 0xAA;
      Serial.print(".");
    }
    payload[0] = a & 0xFF;
    payload[1] = (a >> 8) & 0xFF;
    /*res_size = joybus_n64_write_memory(joybus_pio, a, payload);
    if (!res_size) {
      Serial.println(res_size);
      return;
    }*/
    Serial.println(" | Done!");

    delay(1);

    Serial.print("Reading... | ");
    Serial.print(a, HEX);
    Serial.print(" | ");
    res_size = joybus_n64_read_memory(joybus_pio, a, res);
    if (!res_size) {
      Serial.println(res_size);
      return;
    }
    for (int i = 0; i < N64_BLOCK_SIZE; i++) {
      Serial.print(res[i], HEX);
      Serial.print(" ");
    }
    Serial.println("| Done!");
  }
}

void loop() { }
