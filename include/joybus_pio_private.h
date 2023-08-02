// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// ---------- //
// joybus_pio //
// ---------- //

#define joybus_pio_wrap_target 0
#define joybus_pio_wrap 28

static const uint16_t joybus_pio_program_instructions[] = {
            //     .wrap_target
    0x80a0, //  0: pull   block                      
    0xa0c3, //  1: mov    isr, null                  
    0x60c2, //  2: out    isr, 2                     
    0x4063, //  3: in     null, 3                    
    0xa026, //  4: mov    x, isr                     
    0xa0c3, //  5: mov    isr, null                  
    0x60c6, //  6: out    isr, 6                     
    0x4063, //  7: in     null, 3                    
    0xa046, //  8: mov    y, isr                     
    0x0030, //  9: jmp    !x, 16                     
    0x004c, // 10: jmp    x--, 12                    
    0xea80, // 11: set    pindirs, 0             [10]
    0xef81, // 12: set    pindirs, 1             [15]
    0x7f81, // 13: out    pindirs, 1             [31]
    0xe380, // 14: set    pindirs, 0             [3] 
    0x004b, // 15: jmp    x--, 11                    
    0x0060, // 16: jmp    !y, 0                      
    0xe980, // 17: set    pindirs, 0             [9] 
    0xef81, // 18: set    pindirs, 1             [15]
    0xef80, // 19: set    pindirs, 0             [15]
    0xa0c3, // 20: mov    isr, null                  
    0x0096, // 21: jmp    y--, 22                    
    0x3f20, // 22: wait   0 pin, 0               [31]
    0x4001, // 23: in     pins, 1                    
    0x20a0, // 24: wait   1 pin, 0                   
    0x0096, // 25: jmp    y--, 22                    
    0x4068, // 26: in     null, 8                    
    0x4068, // 27: in     null, 8                    
    0x4068, // 28: in     null, 8                    
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program joybus_pio_program = {
    .instructions = joybus_pio_program_instructions,
    .length = 29,
    .origin = -1,
};

static inline pio_sm_config joybus_pio_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + joybus_pio_wrap_target, offset + joybus_pio_wrap);
    return c;
}
#endif
