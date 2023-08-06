#include <Arduino.h>

#include "joybus_pio.hpp"
#include "joybus_generic.hpp"
#include "joybus_gamecube.hpp"
#include "joybus_n64.hpp"
#include "joybus_gba.hpp"
#include "data_testrom.hpp"

JoybusPIOInstance joybus_pio;
void setup() {}

void setup1() {
  Serial.begin(115200);
  Serial.println("HI");

  joybus_pio = joybus_pio_program_init(pio0, 0, 16);
}

bool inited = false;
bool initedType = false;
JoybusControllerInfo info;

void loop1() {
  if (!inited) {
    delay(100);
    Serial.print("Initializing...");
    info = joybus_handshake(joybus_pio, true);
    Serial.print(" Type: ");
    Serial.print(info.type, HEX);
    Serial.print(" Aux: ");
    Serial.print(info.aux, HEX);
    Serial.println(" Done!");

    if (info.type == 0) {
      return;
    }

    inited = true;
    delay(10);
  }

  switch (info.type) {
  case 0x0004: { // GBA
    Serial.print("GBA booting... ");
    int res = joybus_gba_boot(joybus_pio, (uint8_t*)ROM_gba, ROM_gba_len);
    if (res < 0) {
      Serial.print("ERROR ");
      Serial.println(res);
      return;
    }
    Serial.println("Done!");
    delay(30000);
    break;
  }
  case 0x0500: { // N64
    Serial.print("Querying N64... ");
    N64ControllerState state = joybus_n64_read_controller(joybus_pio);
    if (!state.valid) {
      Serial.println("ERROR");
      return;
    }
    Serial.print("Buttons: ");
    Serial.print(state.buttons, BIN);
    Serial.print(" Joystick X: ");
    Serial.print(state.joystick_x, DEC);
    Serial.print(" Joystick Y: ");
    Serial.print(state.joystick_y, DEC);
    Serial.println(" Done!");
    break;
  }
  case 0x0900: { // GameCube
    GCControllerState origin;
    if (!initedType) {
      Serial.print("Recalibrating GC... ");
      origin = joybus_gc_recalibrate(joybus_pio);
    } else {
      Serial.print("Probing GC origin... ");
      origin = joybus_gc_probe_origin(joybus_pio);
    }
    if (!origin.valid) {
      Serial.println("ERROR");
      return;
    }
    Serial.print(" Joystick X: ");
    Serial.print(origin.joystick_x, DEC);
    Serial.print(" Joystick Y: ");
    Serial.print(origin.joystick_y, DEC);
    Serial.print(" C X: ");
    Serial.print(origin.cstick_x, DEC);
    Serial.print(" C Y: ");
    Serial.print(origin.cstick_y, DEC);
    Serial.println(" Done!");

    Serial.print("Querying GC Position... ");
    GCControllerState raw_state = joybus_gc_short_poll(joybus_pio, 0);
    if (!raw_state.valid) {
      Serial.println("ERROR");
      return;
    }
    NormalizedGCControllerState state = gc_normalize(raw_state, origin);
    Serial.print("Buttons: ");
    Serial.print(state.buttons, BIN);
    Serial.print(" Joystick X: ");
    Serial.print(state.joystick_x, DEC);
    Serial.print(" Joystick Y: ");
    Serial.print(state.joystick_y, DEC);
    Serial.print(" CStick X: ");
    Serial.print(state.cstick_x, DEC);
    Serial.print(" CStick Y: ");
    Serial.print(state.cstick_y, DEC);
    Serial.print(" L: ");
    Serial.print(state.analog_l, DEC);
    Serial.print(" R: ");
    Serial.print(state.analog_r, DEC);
    Serial.println(" Done!");
    break;
  }
  default: {
    inited = false;
    initedType = false;
    return;
  }
  }

  initedType = true;

  if (true) {
    return;
  }

  uint8_t payload[64] = {};
  uint8_t res[64] = {};
  int last_result = 0;
  for (int addr = 0; addr < 5; addr++) {
    delay(1);

    Serial.print("Writing... | ");
    Serial.print(addr, HEX);
    Serial.print(" | ");
    for (int i = 0; i < JOYBUS_BLOCK_SIZE; i++) {
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
    for (int i = 0; i < JOYBUS_BLOCK_SIZE; i++) {
      Serial.print(res[i], HEX);
      Serial.print(" ");
    }
    Serial.println("| Done!");
  }
}

void loop() {}
