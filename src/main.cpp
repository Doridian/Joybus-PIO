#include <Arduino.h>
#include "n64out.h"

#define PIN_CTR 16

#define PAYLOAD_PACKET_MAX 3

uint offset;
void setup() {

}

void setup1() {
  Serial.begin(115200);
  Serial.println("HI");
  
  offset = pio_add_program(pio0, &n64out_program);
  n64out_program_init(pio0, 0, offset, PIN_CTR);
}

static void _tx_data(byte* payload, uint8_t payload_len, uint8_t response_len) {
  while (pio_sm_is_tx_fifo_full(pio0, 0)) {
    tight_loop_contents();
  }

  uint8_t data[4] = { ~payload[2], ~payload[1], ~payload[0], (payload_len << 6) | response_len };
  pio0->txf[0] = *(uint32_t*)data;
}

static int tx_data(byte payload[], byte response[], int payload_len, int response_len) {
  byte* payload_cur = payload;
  while (payload_len > PAYLOAD_PACKET_MAX) {
    _tx_data(payload_cur, PAYLOAD_PACKET_MAX, 0);
    payload_cur += PAYLOAD_PACKET_MAX;
    payload_len -= PAYLOAD_PACKET_MAX;
  }

  byte* response_cur = response;
  _tx_data(payload_cur, payload_len, response_len);
  while (response_len > 0) {
    io_ro_32 *rxfifo_shift = (io_ro_32*)&pio0->rxf[0];

    unsigned long start = millis();
    while (pio_sm_is_rx_fifo_empty(pio0, 0)) {
      if ((millis() - start) > 10) {
        n64out_program_init(pio0, 0, offset, PIN_CTR);
        return -1;
      }
    }

    Serial.println(*(uint32_t*)rxfifo_shift, HEX);

    *response_cur = *rxfifo_shift;
    response_cur += 4;
    response_len -= 4;
  }

  return response_cur - response;
}

uint8_t address_xor_table[] = {
  0x01, 0x1A, 0x0D, 0x1C, 0x0E, 0x07, 0x19, 0x16, 0x0B, 0x1F, 0x15
};

uint32_t make_address(int pos) {
  uint32_t rpos = pos << 5; // 32?
  uint8_t csum = 0x00;
  for (int i = 15; i >= 5; i--) {
    if (rpos & (0b1 << i)) {
      csum ^= address_xor_table[15-i];
    }
  }
  rpos |= csum;
  return rpos;
}

#define PAK_SIZE (32*1024)
#define BLOCK_SIZE 32
#define BLOCK_COUNT (PAK_SIZE/BLOCK_SIZE)

void loop1() {
  byte payload[64] = {};
  byte res[64] = {};

  delay(1000);
  Serial.print("Initializing...");
  payload[0] = 0xFF;
  int res_size = tx_data(payload, res, 1, 3);
  if (res_size <= 0) {
    Serial.println(res_size);
    return;
  }
  Serial.print("Receiving: ");
  Serial.print(res[0], HEX);
  Serial.print(" ");
  Serial.print(res[1], HEX);
  Serial.print(" ");
  Serial.print(res[2], HEX);
  Serial.print(" ");
  Serial.println(" Done!");

  if (res[0] != 0x05) {
    return;
  }

  for (int a = 0; a < 5; a++) {
    uint32_t addr = make_address(a);

    /*delay(1);

    Serial.print("Writing... | ");
    Serial.print(a, HEX);
    Serial.print(" | ");
    tx_byte(0x03);
    tx_byte(addr >> 8);
    tx_byte(addr & 0xFF);
    for (int i = 0; i < BLOCK_SIZE+1; i++) {
      tx_byte((i % 2 == 0) ? 0x55 : 0xAA);
      Serial.print(".");
    }
    tx_byte(95);
    Serial.print("!");
    Serial.println("| Done!");*/

    delay(1);

    /*
    Serial.print("Reading... | ");
    Serial.print(a, HEX);
    Serial.print(" | ");
    tx_byte(0x02);
    tx_byte(addr >> 8);
    tx_byte(addr & 0xFF);
    uint32_t intstatus = save_and_disable_interrupts();
    tx_stopbit();
    int res_size = rx_bytes(BLOCK_SIZE+1, res);
    restore_interrupts(intstatus);
    if (!res_size) {
      return;
    }
    for (int i = 0; i < BLOCK_SIZE+1; i++) {
      Serial.print(res[i], HEX);
      Serial.print(" ");
    }
    Serial.println("| Done!");
    */
  }
}

void loop() { }

/*
// 0 = low 3us, high 1us
// 1 = low 1us, high 3us

// When line pulled low, wait 2us, sample, wait 2us, total 4us
// IDENT | 0x00 |  0 | 3
// INPUT | 0x01 |  0 | 4
// RDMEM | 0x02 |  2 | 32
// WRMEM | 0x03 | 34 | 1/2?

static inline void set_byte_to_bits(byte bits[], byte data, int pos) {
  bits[pos*8 + 0] = (data & 0b1) != 0;
  bits[pos*8 + 1] = (data & 0b10) != 0;
  bits[pos*8 + 2] = (data & 0b100) != 0;
  bits[pos*8 + 3] = (data & 0b1000) != 0;
  bits[pos*8 + 4] = (data & 0b10000) != 0;
  bits[pos*8 + 5] = (data & 0b100000) != 0;
  bits[pos*8 + 6] = (data & 0b1000000) != 0;
  bits[pos*8 + 7] = (data & 0b10000000) != 0;
}

static inline byte get_byte_for_bits(byte bits[], int pos) {
  return (
    bits[pos*8 + 0] << 0 |
    bits[pos*8 + 1] << 1 |
    bits[pos*8 + 2] << 2 |
    bits[pos*8 + 3] << 3 |
    bits[pos*8 + 4] << 4 |
    bits[pos*8 + 5] << 5 |
    bits[pos*8 + 6] << 6 |
    bits[pos*8 + 7] << 7 |
    bits[pos*8 + 8] << 8
  );
}

int send_command(const byte cmd, const byte data[], const int data_len, byte result[], const int result_len) {
  byte cmd_bits_buf[64 * 8] = { 0 }; // Limit to 64 bytes, no N64 command is bigger
  byte result_bits_buf[64 * 8] = { 0 }; // Limit to 64 bytes, no N64 result is bigger

  set_byte_to_bits(cmd_bits_buf, cmd, 0);
  for (int i = 0; i < data_len; i++) {
    set_byte_to_bits(cmd_bits_buf, data[i], i+1);
  }

  byte* result_bits_ptr = result_bits_buf;
  int result_bits_left = result_len*8;

  byte* cmd_buf_ptr = cmd_bits_buf;
  register int cmd_bits_left = (data_len+1) * 8;

  uint32_t intstatus = save_and_disable_interrupts();
  DRIVE_LOW(); SLEEP_1US(); DRIVE_HIGH(); SLEEP_1US();
  DRIVE_LOW(); SLEEP_1US(); DRIVE_HIGH(); SLEEP_1US();
  while (cmd_bits_left--) {
    DRIVE_LOW(); SLEEP_1US(); DRIVE_HIGH(); SLEEP_1US();
    //cmd_buf_ptr++;
  }
  restore_interrupts(intstatus);

  // Every command "drives" high, so relinquishes the bus for input mode, now we just need to clock the data in...
  while (result_bits_left--) {
    while(gpio_get(PIN_CTR)); // Wait for controller to reply
    const unsigned long start = micros();
    while(!gpio_get(PIN_CTR));
    const unsigned long duration = micros() - start;
    if (duration > 2) {
      *result_bits_ptr = 1;
    } else {
      *result_bits_ptr = 0;
    }
    result_bits_ptr++;
  }

  for(int i = 0; i < result_len; i++) {
    result[i] = get_byte_for_bits(result_bits_buf, i);
  }

  return 0;
}


void loop1() {
  delay(1000);
  byte resbuf[64];

  send_command(0x00, NULL, 0, resbuf, 0);

  for (int i = 0; i < 3; i++) {
    Serial.print(resbuf[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}
*/