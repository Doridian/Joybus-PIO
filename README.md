# Joybus-PIO

## Description

Implementation of Nintendo's Joybus protocol based on an RP2040's PIO.

The entire PIO program only needs a single state machine, so one PIO unit can run 4 controllers at once.

It can also boot GBA consoles with games that support being booted via joyboot.

## Credits

Without the below documentation/code, this would have taken much longer to figure out.

### Protocol

- https://n64brew.dev/wiki/Joybus_Protocol

- https://simplecontrollers.com/blogs/resources/gamecube-protocol

### GBA joyboot

- https://github.com/FIX94/gc-gba-link-cable-demo/blob/master/source/main.c

- https://github.com/Sage-of-Mirrors/libgbacom/tree/master/libgbacom/src/VBA
