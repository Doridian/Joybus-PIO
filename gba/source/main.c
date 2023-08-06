
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <gba_sio.h>
#include <stdio.h>
#include <stdlib.h>

#define REG_JOYCNTRL (*(vu32*)0x4000140)
#define REG_JOYCTRL_RST 0b00000001

int joyctrl_rst_rdy = 0;

void ResetHalt() {
	if (REG_JOYCNTRL & REG_JOYCTRL_RST) {
		SystemCall(0x26);
		while (1) {
			Halt();
		}
		return;
	}
	Halt();
}

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
	//---------------------------------------------------------------------------------
	// the vblank interrupt must be enabled for VBlankIntrWait() to work
	// since the default dispatcher handles the bios flags no vblank handler
	// is required
	irqInit();
	irqEnable(IRQ_VBLANK);

	consoleDemoInit();

	iprintf("Handshaking...\n");
	Halt();

	REG_JOYTR = 0x30303030; // our "game code"
	do {
		ResetHalt();
	} while (REG_JSTAT & 0b00001000);

	iprintf("Receiving...\n");

	do {
		ResetHalt();
	} while (!(REG_JSTAT & 0b00000010));

	iprintf("Comparing...\n");

	if (REG_JOYRE != 0x30303030) {
		Halt();
		iprintf("Invalid handshake!\n");
		while (1) {
			ResetHalt();
		}
	}

	iprintf("Entering input loop!\n");

	while (1) {
		ResetHalt();
		REG_JOYTR = REG_JOYCNTRL;
	}

	return 0;
}
