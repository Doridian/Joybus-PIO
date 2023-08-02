#include <Arduino.h>

#include "n64pio.hpp"
#include "n64pio_private.h"

#define PAYLOAD_PACKET_MAX 3
#define OFFSET_NOT_LOADED 0xFFFFFFFF
#define PIO_INDEX(instance) ((instance.pio == pio0) ? 0 : 1)

uint n64pio_offsets[2] = {OFFSET_NOT_LOADED, OFFSET_NOT_LOADED};

static int n64pio_add_program(N64PIOInstance instance) {
  if (instance.offset == OFFSET_NOT_LOADED) {
    instance.offset = n64pio_offsets[PIO_INDEX(instance)];
    if (instance.offset == OFFSET_NOT_LOADED) {
      instance.offset = pio_add_program(instance.pio, &n64pio_program);
      n64pio_offsets[PIO_INDEX(instance)] = instance.offset;
    }
  }
  return pio_add_program(instance.pio, &n64pio_program);
}

N64PIOInstance n64pio_program_init(PIO pio, uint sm, uint pin) {
  N64PIOInstance instance;
  instance.pio = pio;
  instance.sm = sm;
  instance.pin = pin;
  instance.offset = OFFSET_NOT_LOADED;

  pio_sm_set_enabled(instance.pio, instance.sm, false);

  gpio_set_dir(instance.pin, GPIO_IN);
  gpio_disable_pulls(instance.pin);
  gpio_set_oeover(instance.pin, GPIO_OVERRIDE_HIGH);
  gpio_set_outover(instance.pin, GPIO_OVERRIDE_LOW);

  instance.config = n64pio_program_get_default_config(instance.offset);
  sm_config_set_in_pins(&instance.config, instance.pin);
  sm_config_set_out_pins(&instance.config, instance.pin, 1);
  sm_config_set_set_pins(&instance.config, instance.pin, 1);

  sm_config_set_out_shift(&instance.config, false, false, 32);
  sm_config_set_in_shift(&instance.config, false, true, 32);

  float frac = (clock_get_hz(clk_sys) / 1000000) / 16;
  sm_config_set_clkdiv(&instance.config, frac);

  pio_gpio_init(instance.pio, instance.pin);
  pio_sm_set_consecutive_pindirs(instance.pio, instance.sm, instance.pin, 1, false);

  // Load our configuration, and jump to the start of the program
  pio_sm_init(instance.pio, instance.sm, instance.offset, &instance.config);

  pio_sm_set_enabled(instance.pio, instance.sm, true);

  return instance;
}

void n64pio_reset(N64PIOInstance instance) {
  pio_sm_set_enabled(instance.pio, instance.sm, false);
  pio_sm_init(instance.pio, instance.sm, instance.offset, &instance.config);
  pio_sm_set_consecutive_pindirs(instance.pio, instance.sm, instance.pin, 1, false);
  pio_sm_set_enabled(instance.pio, instance.sm, true);
}

static void tx_data(N64PIOInstance instance, byte* payload, uint8_t payload_len, uint8_t response_len) {
  while (pio_sm_is_tx_fifo_full(instance.pio, instance.sm)) {
    tight_loop_contents();
  }

  uint32_t data = ((payload_len >= 3) ? (payload[2] << 0) : 0) |
                  ((payload_len >= 2) ? (payload[1] << 8) : 0) |
                  ((payload_len >= 1) ? (payload[0] << 16) : 0) |
                  (payload_len << (6+24)) | response_len << 24;
  data ^= 0x00FFFFFF; // Invert payload
  instance.pio->txf[instance.sm] = data;
}

int transmit_receive(N64PIOInstance instance, byte payload[], byte response[], uint payload_len, uint response_len) {
  byte* payload_cur = payload;
  while (payload_len > PAYLOAD_PACKET_MAX) {
    tx_data(instance, payload_cur, PAYLOAD_PACKET_MAX, 0);
    payload_cur += PAYLOAD_PACKET_MAX;
    payload_len -= PAYLOAD_PACKET_MAX;
  }

  byte* response_cur = response;
  tx_data(instance, payload_cur, payload_len, response_len);
  while (response_len > 0) {
    io_ro_32 *rxfifo_shift = (io_ro_32*)&instance.pio->rxf[0];

    unsigned long start = millis();
    while (pio_sm_is_rx_fifo_empty(instance.pio, 0)) {
      if ((millis() - start) > 10) {
        n64pio_reset(instance);
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