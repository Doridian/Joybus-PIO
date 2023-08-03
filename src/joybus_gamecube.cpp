#include <Arduino.h>
#include <string.h>

#include "joybus_gamecube.hpp"
#include "joybus_pio.hpp"

static inline GCControllerState joybus_gc_read_state(JoybusPIOInstance instance,
                                                     uint8_t payload[],
                                                     int payload_len,
                                                     int response_overhead) {
  GCControllerState state;
  state.valid = joybus_pio_transmit_receive(
                    instance, payload, payload_len, (uint8_t *)&state,
                    8 + response_overhead) == 8 + response_overhead;
  state.buttons = UINT16_FIX_ENDIAN(state.buttons);
  return state;
}

GCControllerState joybus_gc_short_poll(JoybusPIOInstance instance,
                                       uint8_t flags) {
  uint8_t payload[] = {0x40, 0x03, flags};
  return joybus_gc_read_state(instance, payload, 3, 0);
}

GCControllerState joybus_gc_probe_origin(JoybusPIOInstance instance) {
  uint8_t payload[] = {0x41};
  return joybus_gc_read_state(instance, payload, 1, 2);
}

GCControllerState joybus_gc_recalibrate(JoybusPIOInstance instance) {
  uint8_t payload[] = {0x42, 0x00, 0x00};
  return joybus_gc_read_state(instance, payload, 3, 2);
}

#define GC_NORMALIZE_ELEMENT(NAME)                                             \
  output.NAME = ((int)state.NAME) - ((int)origin.NAME)

NormalizedGCControllerState gc_normalize(GCControllerState state,
                                         GCControllerState origin) {
  NormalizedGCControllerState output;
  output.buttons = state.buttons;
  GC_NORMALIZE_ELEMENT(joystick_x);
  GC_NORMALIZE_ELEMENT(joystick_y);
  GC_NORMALIZE_ELEMENT(cstick_x);
  GC_NORMALIZE_ELEMENT(cstick_y);
  GC_NORMALIZE_ELEMENT(analog_l);
  GC_NORMALIZE_ELEMENT(analog_r);
  return output;
}
