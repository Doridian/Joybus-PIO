#include <Arduino.h>

#include "n64pio.hpp"
#include "n64pio_private.h"

#define PAYLOAD_PACKET_MAX 3

int n64pio_add_program(PIO pio) {
  return pio_add_program(pio, &n64pio_program);
}

void n64pio_program_init(PIO pio, uint sm, uint offset, uint pin) {
    pio_sm_set_enabled(pio, sm, false);
    gpio_set_dir(pin, GPIO_IN);
    gpio_disable_pulls(pin);
    gpio_set_oeover(pin, GPIO_OVERRIDE_HIGH);
    gpio_set_outover(pin, GPIO_OVERRIDE_LOW);
    pio_sm_config c = n64pio_program_get_default_config(offset);
    sm_config_set_in_pins(&c, pin);
    sm_config_set_out_pins(&c, pin, 1);
    sm_config_set_set_pins(&c, pin, 1);
    sm_config_set_out_shift(&c, false, false, 32);
    sm_config_set_in_shift(&c, false, true, 32);
    float frac = (clock_get_hz(clk_sys) / 1000000) / 16;
    sm_config_set_clkdiv(&c, frac);
    pio_gpio_init(pio, pin);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, false);
    // Load our configuration, and jump to the start of the program
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

static void _tx_data(PIO pio, uint sm, byte* payload, uint8_t payload_len, uint8_t response_len) {
  while (pio_sm_is_tx_fifo_full(pio, sm)) {
    tight_loop_contents();
  }

  uint32_t data = ((payload_len >= 3) ? (payload[2] << 0) : 0) |
                  ((payload_len >= 2) ? (payload[1] << 8) : 0) |
                  ((payload_len >= 1) ? (payload[0] << 16) : 0) |
                  (payload_len << (6+24)) | response_len << 24;
  data ^= 0x00FFFFFF; // Invert payload
  pio->txf[sm] = data;
}

int transmit_receive(PIO pio, uint sm, byte payload[], byte response[], uint payload_len, uint response_len) {
  byte* payload_cur = payload;
  while (payload_len > PAYLOAD_PACKET_MAX) {
    _tx_data(pio, sm, payload_cur, PAYLOAD_PACKET_MAX, 0);
    payload_cur += PAYLOAD_PACKET_MAX;
    payload_len -= PAYLOAD_PACKET_MAX;
  }

  byte* response_cur = response;
  _tx_data(pio, sm, payload_cur, payload_len, response_len);
  while (response_len > 0) {
    io_ro_32 *rxfifo_shift = (io_ro_32*)&pio->rxf[0];

    unsigned long start = millis();
    while (pio_sm_is_rx_fifo_empty(pio, 0)) {
      if ((millis() - start) > 10) {
        // n64pio_program_init(pio, 0, offset, PIN_CONTROLLER);
        return -1;
      }
    }

    uint32_t rxfifo_data = *rxfifo_shift;

    int i = (response_len > 3) ? 32 : (response_len << 3);
    while ((i -= 8) >= 0) {
      *(response_cur++) = (rxfifo_data >> i) & 0xFF;
    }
    response_len -= 4;
  }

  return (byte*)response_cur - response;
}