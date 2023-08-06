#include <Arduino.h>

#include "joybus_gba.hpp"
#include "joybus_pio.hpp"
#include "joybus_generic.hpp"

#define JOYBUS_READ_GBA 0x14
#define JOYBUS_WRITE_GBA 0x15

#define REG_VALID_MASK 0xC5
#define REG_PSF1 0x20
#define REG_PSF0 0x10
#define REG_SEND 0x08

#define GBA_DELAY 100

int joybus_gba_write(JoybusPIOInstance instance, uint8_t data[]) {
    uint8_t payload[] = { JOYBUS_WRITE_GBA, data[0], data[1], data[2], data[3] };
    uint8_t siostat_rx;
    int len = joybus_pio_transmit_receive(instance, payload, 5, &siostat_rx, 1);
    if (len != 1 || (siostat_rx & REG_VALID_MASK)) {
        return -10;
    }
    return 4;
}

int joybus_gba_read(JoybusPIOInstance instance, uint8_t data[]) {
    uint8_t payload[] = { JOYBUS_READ_GBA };
    uint8_t buf[5];
    int len = joybus_pio_transmit_receive(instance, payload , 1, buf, 5);
    if (len != 5 || (buf[4] & REG_VALID_MASK)) {
        return -20;
    }
    memcpy(data, buf, 4);
    return 4;
}

int joybus_gba_transact(JoybusPIOInstance instance, uint8_t send[], uint8_t recv[]) {
    int len = joybus_gba_write(instance, send);
    if (len < 0) {
        return len;
    }
    return joybus_gba_read(instance, recv);
}

// References for the below code (the boot sequence itself as well as the key/CRC/encryption functions)
// https://github.com/FIX94/gc-gba-link-cable-demo/blob/master/source/main.c
// https://github.com/Sage-of-Mirrors/libgbacom/tree/master/libgbacom

static uint32_t calculate_gc_key(uint32_t size) {
  unsigned int ret = 0;
  size = (size - 0x200) >> 3;
  int res1 = (size & 0x3F80) << 1;
  res1 |= (size & 0x4000) << 2;
  res1 |= (size & 0x7F);
  res1 |= 0x380000;
  int res2 = res1;
  res1 = res2 >> 0x10;
  int res3 = res2 >> 8;
  res3 += res1;
  res3 += res2;
  res3 <<= 24;
  res3 |= res2;
  res3 |= 0x80808080;

  if ((res3 & 0x200) == 0) {
    ret |= (((res3) & 0xFF) ^ 0x4B) << 24;
    ret |= (((res3 >> 8) & 0xFF) ^ 0x61) << 16;
    ret |= (((res3 >> 16) & 0xFF) ^ 0x77) << 8;
    ret |= (((res3 >> 24) & 0xFF) ^ 0x61);
  } else {
    ret |= (((res3) & 0xFF) ^ 0x73) << 24;
    ret |= (((res3 >> 8) & 0xFF) ^ 0x65) << 16;
    ret |= (((res3 >> 16) & 0xFF) ^ 0x64) << 8;
    ret |= (((res3 >> 24) & 0xFF) ^ 0x6F);
  }
  return ret;
}

static uint32_t gba_crc(uint32_t crc, uint32_t value) {
  int i;
  for (i = 0; i < 0x20; i++)
  {
    if ((crc ^ value) & 1)
    {
      crc >>= 1;
      crc ^= 0xa1c1;
    }
    else
      crc >>= 1;
    value >>= 1;
  }
  return crc;
}

static void gba_encrypt(uint8_t* data, uint8_t* enc_bytes, uint32_t i, uint32_t& session_key, uint32_t& fcrc) {
  uint32_t plaintext = ((int)((data[0]) << 24) | (int)((data[1]) << 16) | (int)((data[2]) << 8) | (int)((data[3])));
  plaintext = __builtin_bswap32(plaintext);

  fcrc = gba_crc(fcrc, plaintext);
  session_key = (session_key * 0x6177614B) + 1;

  uint32_t encrypted = plaintext ^ session_key;
  encrypted ^= ((~(i + (0x20 << 20))) + 1);
  encrypted ^= 0x20796220;

  encrypted = __builtin_bswap32(encrypted);
  
  enc_bytes[0] = (encrypted & 0xFF000000) >> 24;
  enc_bytes[1] = (encrypted & 0x00FF0000) >> 16;
  enc_bytes[2] = (encrypted & 0x0000FF00) >> 8;
  enc_bytes[3] = (encrypted & 0x000000FF);
}

int joybus_gba_boot(JoybusPIOInstance instance, uint8_t rom[], int rom_len) {
    JoybusControllerInfo info;
    delay(10);
    info = joybus_handshake(instance, true);
    delay(10);
    info = joybus_handshake(instance, false);
    if ((info.aux & REG_PSF0) == 0) {
      return -100;
    }

    int len;

    delayMicroseconds(GBA_DELAY);
    uint32_t session_key;
    len = joybus_gba_read(instance, (uint8_t*)&session_key);
    if (len < 0) {
      return -101;
    }

    session_key = __builtin_bswap32(session_key);
    session_key ^= 0x7365646F;
    session_key = __builtin_bswap32(session_key);

    uint32_t our_key = calculate_gc_key(rom_len);
    our_key = __builtin_bswap32(our_key);
    delayMicroseconds(GBA_DELAY);
    len = joybus_gba_write(instance, (uint8_t*)&our_key);
    if (len < 0) {
      return -102;
    }

    // Send the ROM header to the GBA
    for (int i = 0; i < 0xC0; i += 4) {
      delayMicroseconds(GBA_DELAY);
      len = joybus_gba_write(instance, rom + i);
      if (len < 0) {
        return -103;
      }
    }

    uint32_t fcrc = 0x15A0;
    uint32_t i;

    for (i = 0xC0; i < rom_len; i += 4) {
      delayMicroseconds(GBA_DELAY);
      uint8_t encrypted_bytes[4];
      gba_encrypt(rom + i, encrypted_bytes, i, session_key, fcrc);
      len = joybus_gba_write(instance, encrypted_bytes);
      if (len < 0) {
        return -104;
      }
    }

    delayMicroseconds(GBA_DELAY);

    fcrc |= (rom_len << 16);
    session_key = (session_key * 0x6177614B) + 1;
    fcrc ^= session_key;
    fcrc ^= ((~(i + (0x20 << 20))) + 1);
    fcrc ^= 0x20796220;

    len = joybus_gba_write(instance, (uint8_t*)&fcrc);
    if (len < 0) {
      return -105;
    }

    delayMicroseconds(GBA_DELAY);

    uint8_t res[4];
    len = joybus_gba_read(instance, res);
    if (len < 0) {
      return -106;
    }

    // Wait for handshake ready
    info.aux = 0;
    while ((info.aux & REG_SEND) == 0) {
      delayMicroseconds(GBA_DELAY);
      info = joybus_handshake(instance, false);
    }

    delayMicroseconds(GBA_DELAY);
    // Read game code
    len = joybus_gba_read(instance, res);
    if (len < 0) {
      return -107;
    }

    delayMicroseconds(GBA_DELAY);
    // Send received gamecode back
    len = joybus_gba_write(instance, rom + 0xAC);
    if (len < 0) {
      return -108;
    }

    // Ensure we have a match
    if (memcmp(res, rom + 0xAC, 4) != 0) {
      return -109;
    }

    return 0;
}