from pioemu import emulate, State

program = [
    0x80a0, #  0: pull   block                      
    0xa0c3, #  1: mov    isr, null                  
    0x60c2, #  2: out    isr, 2                     
    0x4063, #  3: in     null, 3                    
    0xa026, #  4: mov    x, isr                     
    0xa0c3, #  5: mov    isr, null                  
    0x60c6, #  6: out    isr, 6                     
    0x4063, #  7: in     null, 3                    
    0xa046, #  8: mov    y, isr                     
    0x0009, #  9: jmp    9     
]

initial = State()
#initial.transmit_fifo.append(0x3F000000)
initial.transmit_fifo.append(0xC0000000)
generator = emulate(program, initial_state=initial, stop_when=lambda _, state: state.program_counter >= 9, shift_isr_right=False, shift_osr_right=False)

for before, after in generator:
  print(after)
