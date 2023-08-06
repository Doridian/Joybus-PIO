#include <Arduino.h>

#include "joybus_gba.hpp"
#include "joybus_pio.hpp"

#define JOYBUS_READ_GBA 0x14
#define JOYBUS_WRITE_GBA 0x15

int joybus_gba_write(JoybusPIOInstance instance, uint8_t data[]) {
    uint8_t payload[] = { JOYBUS_WRITE_GBA, data[0], data[1], data[2], data[3] };
    uint8_t siostat_rx;
    int len = joybus_pio_transmit_receive(instance, payload, 5, &siostat_rx, 1);
    if (len != 1) {
        return -10;
    }
        return 4;
}

int joybus_gba_read(JoybusPIOInstance instance, uint8_t data[]) {
    uint8_t payload[] = { JOYBUS_READ_GBA };
    return joybus_pio_transmit_receive(instance, payload , 1, data, 5);
}

int joybus_gba_transact(JoybusPIOInstance instance, uint8_t send[], uint8_t recv[]) {
    int len = joybus_gba_write(instance, send);
    if (len < 0) {
        return len;
    }
    while (true) {
        delay(1);
        len = joybus_gba_read(instance, recv);
        if (len < 0) {
            return len;
        }
        if (recv[4] & 0b00000010) {
            return len;
        }
        Serial.print("-");
    }
}

int joybus_gba_poll(JoybusPIOInstance instance, uint8_t send[], uint8_t recv[], int retries) {
    uint8_t buf[5];
    while (retries--) {
        int last = joybus_gba_transact(instance, send, buf);
        if (last != 5) {
            delay(1);
            continue;
        }
        if ((buf[0] != recv[0]) || (buf[1] != recv[1]) || (buf[1] != recv[2]) || (buf[3] != recv[3])) {
            Serial.print(buf[0], HEX);
            Serial.print(buf[1], HEX);
            Serial.print(buf[2], HEX);
            Serial.print(buf[3], HEX);
            Serial.print(buf[4], HEX);
            Serial.print(" ");
            delay(1);
            continue;
        }
        return 4;
    }

    return -10;
}