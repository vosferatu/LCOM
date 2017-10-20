#include <minix/syslib.h>
#include <minix/drivers.h>
#include "keyboard.h"
#include "timer.h"
#include "i8042.h"

//tickdelay(micros_to_ticks(DELAY_US);

int hook_id_1 = KEYBOARD_IRQ;

int keyboard_subscribe_int(void ) {

	if ((sys_irqsetpolicy(KEYBOARD_IRQ, (IRQ_REENABLE | IRQ_EXCLUSIVE), &hook_id_1) == OK) && (sys_irqenable(&hook_id_1) == OK))
		return BIT(KEYBOARD_IRQ); //return activated bit by kernel on hook_id. which is value we set
	else
		{
			printf("\nCouldn't subscribe keyboard interrupts.\n");
			return -1;  //return negative on failure
		}
}

int keyboard_unsubscribe_int() {

	if ((sys_irqdisable (&hook_id_1) == OK) && (sys_irqrmpolicy(&hook_id_1) == OK))
		return 0;
	else
		{
			printf("\nUnable to unsubscribe interrupts.\n");
			return 1;
		}
}

unsigned long keyboard_c_handler(){
	unsigned long scan;

	if (kbd_read(&scan) != OK)
		return 1;
	if (scan == TWO_BYTE_SCAN){
		scan |= (TWO_BYTE_SCAN << 8);
	}

	/*if (scan & BREAK_CODE){
		printf("\tBreakcode: 0x%X.\n", scan);
		if (scan == ESC_BREAK)
			return -1;
	}
	else printf("\tMakecode: 0x%X.\n", scan);*/

	return scan;
}

int kbd_write(unsigned long command){
	unsigned long stat;
	unsigned int i = 0;

	while (i < N_TRY){
		if (sys_inb(STAT_REG, &stat) != OK)
			return -1;
		if ((stat & IN_OK) == 0){
			if (sys_outb(KBD_CMD, command) != OK)
				return -1;
			else return 0;
		}
		tickdelay(micros_to_ticks(DELAY_MS));
		i++;
	}

	return -1;
}

int kbd_read(unsigned long *data){
	unsigned long stat;
	unsigned int i = 0;

	while (i < N_TRY){
		if (sys_inb(STAT_REG, &stat) != OK)
			return -1;
		if ((stat & IN_OK)== 0){
			if (sys_inb(OUT_BUF, data) != OK)
				return -1;
			if ((stat & STAT_ERR) == 0)
				return 0;
			else return -1;
		}
		tickdelay(micros_to_ticks(DELAY_MS));
		i++;
	}

}


void kb_set_led(unsigned short led){

	unsigned short capslk = 0, scrlk = 0, numlk = 0, cmd = 0;

		switch (led) {
		case (0): {
			if (scrlk == 0)
				scrlk = BIT(0);
			else
				scrlk = 0;
			break;
		}

		case (1): {
			if (numlk == 0)
				numlk = BIT(1);
			else
				numlk = 0;
			break;
		}

		case (2): {
			if (capslk == 0)
				capslk = BIT(2);
			else
				capslk = 0;
			break;
		}


		case (3): {
			scrlk = 0;
			numlk = 0;            // initial state (the 3 leds off)
			capslk = 0;
			break;
		}

		default: {
			printf("Arguments not valid.");// if any other argument is given, the function aborts
			return;
		}
	}

		cmd = scrlk | numlk | capslk;

		unsigned long msg;

		do {
				sys_outb(KBD_ARG, LED_CMD);
				tickdelay(micros_to_ticks(DELAY_MS));
				sys_inb(OUT_BUF, &msg);

		} while (msg != KBD_ACK);

		// only when keyboard ack the led_cmd is that when the cmd is sent


		do {

		sys_outb(OUT_BUF, cmd);
		tickdelay(micros_to_ticks(DELAY_MS));
		sys_inb(OUT_BUF, &msg);

		if (msg == KBD_ACK)
			break;

		if (msg == KBD_ERROR) {
			kb_set_led(led);  // retry
			break;
		}

		} while (msg == KBD_RESEND);

}

