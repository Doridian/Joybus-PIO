#pragma once

#include <stdint.h>

#include "joybus_pio.hpp"

#define N64_PAK_SIZE (32 * 1024)
#define N64_BLOCK_SIZE 32
#define N64_BLOCK_COUNT (PAK_SIZE / BLOCK_SIZE)

typedef enum N64Button {
  N64B_C_RIGHT    = 0b0000000000000001,
  N64B_C_LEFT     = 0b0000000000000010,
  N64B_C_DOWN     = 0b0000000000000100,
  N64B_C_UP       = 0b0000000000001000,
  N64B_R_TRIGGER  = 0b0000000000010000,
  N64B_L_TRIGGER  = 0b0000000000100000,
  N64B_ZERO_1     = 0b0000000001000000,
  N64B_RESET      = 0b0000000010000000,
  N64B_DPAD_RIGHT = 0b0000000100000000,
  N64B_DPAD_LEFT  = 0b0000001000000000,
  N64B_DPAD_DOWN  = 0b0000010000000000,
  N64B_DPAD_UP    = 0b0000100000000000,
  N64B_START      = 0b0001000000000000,
  N64B_Z_TRIG     = 0b0010000000000000,
  N64B_B_BUTTON   = 0b0100000000000000,
  N64B_A_BUTTON   = 0b1000000000000000,
} N64Button;

typedef struct __attribute__((__packed__)) N64ControllerState {
  uint16_t buttons;
  int8_t joystick_x;
  int8_t joystick_y;
  uint8_t valid;
} N64ControllerState;

// response[] is expected to be N64_BLOCK_SIZE + 1 bytes long (for the checksum)
int joybus_n64_read_memory(JoybusPIOInstance instance, uint address,
                           uint8_t response[]);

// data[] is expected to be N64_BLOCK_SIZE bytes long
int joybus_n64_write_memory(JoybusPIOInstance instance, uint address,
                            uint8_t data[]);

N64ControllerState joybus_n64_read_controller(JoybusPIOInstance instance);
