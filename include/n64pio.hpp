#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

void n64pio_program_init(PIO pio, uint sm, uint offset, uint pin);
int n64pio_add_program(PIO pio);
int transmit_receive(PIO pio, uint sm, byte payload[], byte response[], uint payload_len, uint response_len);
