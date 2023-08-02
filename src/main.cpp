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

bool inited = false;

void loop1() {
  if (!inited) {
    delay(5000);
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
    inited = true;
  }

  delay(100);
  Serial.print("Querying... ");
  N64ControllerState state = joybus_n64_read_controller(joybus_pio);
  Serial.print("Buttons: ");
  Serial.print(state.buttons, BIN);
  Serial.print(" Joystick X: ");
  Serial.print(state.joystick_x, DEC);
  Serial.print(" Joystick Y: ");
  Serial.print(state.joystick_y, DEC);
  Serial.println(" Done!");

  uint8_t payload[64] = {};
  uint8_t res[64] = {};
  int last_result = 0;
  for (int addr = 0; addr < 5; addr++) {
    delay(1);

    Serial.print("Writing... | ");
    Serial.print(addr, HEX);
    Serial.print(" | ");
    for (int i = 0; i < N64_BLOCK_SIZE; i++) {
      payload[i] = (i % 2 == 0) ? 0x55 : 0xAA;
      Serial.print(".");
    }
    payload[0] = addr & 0xFF;
    payload[1] = (addr >> 8) & 0xFF;
    last_result = joybus_n64_write_memory(joybus_pio, addr, payload);
    if (last_result <= 0) {
      Serial.println(last_result);
      return;
    }
    Serial.println(" | Done!");

    delay(1);

    Serial.print("Reading... | ");
    Serial.print(addr, HEX);
    Serial.print(" | ");
    last_result = joybus_n64_read_memory(joybus_pio, addr, res);
    if (last_result <= 0) {
      Serial.println(last_result);
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
