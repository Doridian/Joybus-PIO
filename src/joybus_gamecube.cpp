#include <string.h>
#include <Arduino.h>

#include "joybus_gamecube.hpp"
#include "joybus_pio.hpp"

static inline GCControllerState joybus_gc_read_state(JoybusPIOInstance instance, uint8_t payload[], int payload_len, int response_overhead) {
  GCControllerState state;
  int res = joybus_pio_transmit_receive(instance, payload, payload_len, (uint8_t*)&state, 9 + response_overhead);
  if (res != 8 + response_overhead) {
    state.buttons = 0x0;
  }
  state.buttons = UINT16_FIX_ENDIAN(state.buttons);
  return state;
}

GCControllerState joybus_gc_short_poll(JoybusPIOInstance instance, uint8_t flags) {
  uint8_t payload[] = { 0x40, 0x03, flags };
  return joybus_gc_read_state(instance, payload, 3, 0);
}

GCControllerState joybus_gc_probe_origin(JoybusPIOInstance instance) {
  uint8_t payload[] = { 0x41 };
  return joybus_gc_read_state(instance, payload, 1, 2);
}

GCControllerState joybus_gc_recalibrate(JoybusPIOInstance instance) {
  uint8_t payload[] = { 0x42, 0x00, 0x00 };
  return joybus_gc_read_state(instance, payload, 3, 2);
}
