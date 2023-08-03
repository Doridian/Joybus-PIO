#include "joybus_generic.hpp"

JoybusControllerInfo joybus_handshake(JoybusPIOInstance instance, bool reset) {
  JoybusControllerInfo info;
  info.type = 0x0000;
  uint8_t payload[] = {reset ? (uint8_t)0xFF : (uint8_t)0x00};
  joybus_pio_transmit_receive(instance, payload, 1, (uint8_t *)&info, 3);
  info.type = UINT16_FIX_ENDIAN(info.type);
  return info;
}
