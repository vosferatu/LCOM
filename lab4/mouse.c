#include <minix/syslib.h>
#include <minix/drivers.h>
#include "mouse.h"
#include "i8042_mouse.h"

int mouse_hook = MOUSE_IRQ;

int mouse_subscribe_int() {
	unsigned long status;

	if ((sys_irqsetpolicy(MOUSE_IRQ, (IRQ_REENABLE | IRQ_EXCLUSIVE),
			&mouse_hook) == OK) && (sys_irqenable(&mouse_hook) == OK)) {
		do {
			mouse_write(KBC_CMD, ENABLE_MOUSE);
			mouse_write(KBC_CMD, WRITE_TO_MOUSE);
			mouse_write(OUT_BUF, ENABLE_PACKETS);
			mouse_read(&status);
		} while (status != ACK);

		return BIT(MOUSE_IRQ); //return activated bit by kernel on hook_id. which is value we set
	} else {
		printf("\nCouldn't subscribe mouse interrupts.\n");
		return -1;  //return negative on failure
	}
}

int mouse_unsubscribe_int() {
	if ((sys_irqdisable(&mouse_hook) == OK)
			&& (sys_irqrmpolicy(&mouse_hook) == OK))
		return 0;
	else {
		printf("\nUnable to unsubscribe mouse interrupts.\n");
		return 1;
	}
}

int mouse_read(unsigned long *data) {
	unsigned long stat;
	unsigned int i = 0;

	while (i < N_TRY) {
		if (sys_inb(STAT_REG, &stat) != OK)
			return -1;
		if ((stat & IN_OK) == 0) {
			if (sys_inb(OUT_BUF, data) != OK)
				return -1;
			if ((stat & STAT_ERR) == 0)
				return 0;
			else
				return -1;
		}
		tickdelay(micros_to_ticks(DELAY_MS));
		i++;
	}

}

int mouse_write(char where_to, unsigned long command) {
	unsigned long stat;
	unsigned int i = 0;

	while (i < N_TRY) {
		if (sys_inb(STAT_REG, &stat) != OK)
			return -1;
		if ((stat & IN_OK) == 0) {
			if (sys_outb(where_to, command) != OK)
				return -1;
			else
				return 0;
		}
		tickdelay(micros_to_ticks(DELAY_MS));
		i++;
	}

	return -1;
}

void mouse_interface(unsigned long command) {
	unsigned long status;

	do {
		mouse_write(STAT_REG, WRITE_TO_MOUSE);
		mouse_write(OUT_BUF, command);
		mouse_read(&status);
	} while (status != ACK);

	return;
}

void mouse_int_handler(unsigned int count, char packets[]) {
	unsigned long onebyte;
	unsigned int i = -1;

	while (i != OK) {
		i = mouse_read(&onebyte);
	}

	packets[count] = (char) onebyte;

	return;
}

void gesture_machine(char packets[], int *distance) {
	int slope = -1;
	char zero = 0;

	if (packets[0] & BIT(1)) { //right button -> drawing state

		if (packets[0] & BIT(4)) {
			if (packets[0] & BIT(5))
				slope = 1;
			else
				slope = -1;
		} else {
			if (packets[0] & BIT(5))
				slope = -1;
			else
				slope = 1;
		}

		if (slope < 0) {
			(*distance) = 0;
			return;
		}

		//even if y = 0, the movement is still continuous because it's moving in x
		//but slope = y/x = 0/x = 0, so the slope isn't positive, but we can still sum because it's 0
		if ((packets[0] & BIT(5)) && (packets[1] != 0))		//changes variable in test function
			(*distance) -= (-packets[2]); //(*distance) = -(packets[2]); //ask teacher
		else
			(*distance) += packets[2]; //(*distance) = packets[2];
	} else {
		(*distance) = 0; //if not in the conditions, reset our counter
	}

}

int gesture_state_machine(char packets[], short *distance,
		event_t event) {
	static int done = 0;
	static state_t gest_state = INIT;

	switch (gest_state) {
	case INIT:
		if (event == RDOWN) {
			gest_state = DRAW;
		}

		if (event == RUP) {
			done = 0;
		}

		if ((event == MOVE) && (packets[1] != 0)) {
			if (packets[0] & BIT(5))
				done -= (-packets[2]);
			else
				done += packets[2];

			if ( ((done >= (*distance)) && ((*distance) > 0)) || ((done <= (*distance)) && ((*distance) < 0)))
				gest_state = COMP;

			gest_state = DRAW;
		}
		break;
	case DRAW:
		if (event == RDOWN) {
		}

		if (event == RUP) {
			done = 0;
			gest_state = INIT;
		}

		if ((event == MOVE) && (packets[1] != 0)) {

			if ((*distance) < 0){

				if (packets[0] & BIT(5))
					done -= (-packets[2]);
				}
			else {
				if (!(packets[0] & BIT(5)))
					done -= (-packets[2]);
			}


			if ( ((done >= (*distance)) && ((*distance) > 0)) || ((done <= (*distance)) && ((*distance) < 0)) )
				gest_state = COMP;
		}
		break;
	default:
		break;
	}

	if (done > 0 && (*distance) < 0)
		done = 0;

	if (done < 0 && (*distance) > 0)
		done = 0;

	printf("\ndistance: %d\n", done);

	return gest_state;
}
