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
#define REG_RECV 0x02

#define GBA_DELAY 70

uint8_t joybus_gba_get_siostat(JoybusPIOInstance instance) {
    JoybusControllerInfo info = joybus_handshake(instance, false);
    return info.aux;
}

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

int joybus_gba_wait_and_write(JoybusPIOInstance instance, uint8_t data[]) {
    JoybusControllerInfo info;
    info.aux = 0;
    while ((info.aux & REG_RECV) != 0) {
      info = joybus_handshake(instance, false);
      if (info.type != 0x0004) {
        return -200;
      }
      delayMicroseconds(GBA_DELAY);
    }

    return joybus_gba_write(instance, data);
}

int joybus_gba_wait_and_read(JoybusPIOInstance instance, uint8_t data[]) {
    JoybusControllerInfo info;
    info.aux = 0;
    while ((info.aux & REG_SEND) == 0) {
      info = joybus_handshake(instance, false);
      if (info.type != 0x0004) {
        return -201;
      }
      delayMicroseconds(GBA_DELAY);
    }

    return joybus_gba_read(instance, data);
}

// References for the below code (the boot sequence itself as well as the key/CRC/encryption functions)
// https://github.com/FIX94/gc-gba-link-cable-demo/blob/master/source/main.c
// https://github.com/Sage-of-Mirrors/libgbacom/tree/master/libgbacom

static uint32_t calculate_gc_key(uint32_t size) {
  size = (size - 0x200) >> 3;
  int res1 = (size & 0x3F80) << 1;
  res1 |= (size & 0x4000) << 2;
  res1 |= (size & 0x7F);
  res1 |= 0x380000;
  int res2 = res1 >> 8;
  res2 += res1 >> 16;
  res2 += res1;
  res2 <<= 24;
  res2 |= res1;
  res2 |= 0x80808080;

  if ((res2 & 0x200) == 0) {
    res2 ^= 0x6177614B;
  } else {
    res2 ^= 0x6F646573;
  }
  return res2;
}

static uint32_t gba_crc(uint32_t crc, uint32_t value) {
  for (int i = 0; i < 0x20; i++) {
    if ((crc ^ value) & 1) {
      crc >>= 1;
      crc ^= 0xa1c1;
    } else {
      crc >>= 1;
    }
    value >>= 1;
  }
  return crc;
}

static void gba_encrypt(uint8_t data[], uint8_t enc_bytes[], uint32_t i, uint32_t& session_key, uint32_t& fcrc) {
  uint32_t plaintext = *(uint32_t*)data;

  fcrc = gba_crc(fcrc, plaintext);
  session_key = (session_key * 0x6177614B) + 1;

  uint32_t encrypted = plaintext ^ session_key;
  encrypted ^= ((~(i + (0x20 << 20))) + 1);
  encrypted ^= 0x20796220;

  *(uint32_t*)enc_bytes = encrypted;
}

int joybus_gba_boot(JoybusPIOInstance instance, uint8_t rom[], int rom_len) {
    JoybusControllerInfo info;
    delay(10);
    info = joybus_handshake(instance, true);
    if (info.type != 0x04) {
        return -98;
    }
    delay(10);
    info = joybus_handshake(instance, false);
    if (info.type != 0x04) {
        return -99;
    }
    if ((info.aux & REG_PSF0) == 0) {
      return -100;
    }

    delayMicroseconds(GBA_DELAY);


    int len;
    uint32_t session_key;
    len = joybus_gba_wait_and_read(instance, (uint8_t*)&session_key);
    if (len < 0) {
      return -101;
    }

    delayMicroseconds(GBA_DELAY);

    session_key ^= 0x6F646573;

    uint32_t our_key = calculate_gc_key(rom_len);
    len = joybus_gba_wait_and_write(instance, (uint8_t*)&our_key);
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

    len = joybus_gba_wait_and_write(instance, (uint8_t*)&fcrc);
    if (len < 0) {
      return -105;
    }

    delayMicroseconds(GBA_DELAY);

    // Read back useless CRC reply
    uint8_t res[4];
    len = joybus_gba_wait_and_read(instance, res);
    if (len < 0) {
      return -106;
    }

    delayMicroseconds(GBA_DELAY);

    return 0;
}

int joybus_gba_default_handshake(JoybusPIOInstance instance, uint8_t rom[], int rom_len) {
    uint8_t res[4];
    // Read game code
    int len = joybus_gba_wait_and_read(instance, res);
    if (len < 0) {
      return -108;
    }

    delayMicroseconds(GBA_DELAY);
    // Send received gamecode back
    len = joybus_gba_wait_and_write(instance, rom + 0xAC);
    if (len < 0) {
      return -109;
    }

    // Ensure we have a match
    if (memcmp(res, rom + 0xAC, 4) != 0) {
      return -110;
    }

    return 0;
}