#include <string.h>
#include <Arduino.h>

#include "joybus_gamecube.hpp"
#include "joybus_pio.hpp"

GCControllerState joybus_gc_short_poll(JoybusPIOInstance instance, uint8_t flags) {
  GCControllerState state;
  uint8_t payload[] = { 0x40, 0x03, flags };
  int res = joybus_pio_transmit_receive(instance, payload, 3, (uint8_t*)&state, 8);
  if (res != 8) {
    Serial.println("WRONG LEN");
    Serial.println(res);
    state.buttons = 0x0;
  }
  state.buttons = UINT16_FIX_ENDIAN(state.buttons);
  return state;
}

GCControllerState joybus_gc_probe_origin(JoybusPIOInstance instance) {
  GCControllerState state;
  uint8_t payload[] = { 0x41 };
  int res = joybus_pio_transmit_receive(instance, payload, 1, (uint8_t*)&state, 10);
  if (res != 10) {
    Serial.println("WRONG LEN");
    Serial.println(res);
    state.buttons = 0x0;
  }
  state.buttons = UINT16_FIX_ENDIAN(state.buttons);
  return state;
}

GCControllerState joybus_gc_recalibrate(JoybusPIOInstance instance) {
  GCControllerState state;
  uint8_t payload[] = { 0x42, 0x00, 0x00 };
  int res = joybus_pio_transmit_receive(instance, payload, 3, (uint8_t*)&state, 10);
  if (res != 10) {
    Serial.println("WRONG LEN");
    Serial.println(res);
    state.buttons = 0x0;
  }
  state.buttons = UINT16_FIX_ENDIAN(state.buttons);
  return state;
}
