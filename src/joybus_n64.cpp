#include <Arduino.h>
#include <string.h>

#include "joybus_n64.hpp"
#include "joybus_pio.hpp"
#include "joybus_generic.hpp"

int joybus_n64_read_memory(JoybusPIOInstance instance, uint address,
                           uint8_t response[]) {
  return joybus_read_memory(instance, 0x02, address, response);
}

int joybus_n64_write_memory(JoybusPIOInstance instance, uint address,
                            uint8_t data[]) {
  return joybus_write_memory(instance, 0x03, address, data);
}

N64ControllerState joybus_n64_read_controller(JoybusPIOInstance instance) {
  N64ControllerState state;
  uint8_t payload[] = {0x01};
  state.buttons = UINT16_FIX_ENDIAN(state.buttons);
  state.valid = joybus_pio_transmit_receive(instance, payload, 1,
                                            (uint8_t *)&state, 4) == 4;
  return state;
}
