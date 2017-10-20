#include <minix/syslib.h>
#include <minix/drivers.h>
#include "test3.h"
#include "keyboard.h"
#include "timer.h"
#include "i8042.h"

int kbd_test_scan(unsigned short ass) {
	int ipc_status, r, irq_set = keyboard_subscribe_int(), asm_res = 0;
	long MB_CODE = 0x00;
	message msg;

	if (irq_set == -1)
		return 1; //message printed in subscribe_int

	while (MB_CODE != BREAK_CODE) { /* You may want to use a different condition */
			if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {/* Get a request message. */
				printf("driver_receive failed with: %d", r);
				continue;
			}
			if (is_ipc_notify(ipc_status)) { /* received notification */
				switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE: /* hardware interrupt notification */
					if (msg.NOTIFY_ARG & irq_set) { /* subscribed interrupt */
						if(ass == 0){

												//MB_CODE = keyboard_c_handler();
												printf("\nassembly\n");
											}
											else {
												sys_enable_iop(SELF);
												MB_CODE = keyboard_asm_handler();
											}
											if (MB_CODE & BREAK_CODE)
												printf("Breakcode: 0x%x\n", MB_CODE);
											else
												printf("Makecode: 0x%x\n", MB_CODE);
					}
					break;
				default:
					break; /* no other notifications expected: do nothing */
				}
			} else { /* received a standard message, not a notification */
				/* no standard messages expected: do nothing */
			}
			/*if (MB_CODE == -1)
				break;*/
	}

	if (keyboard_unsubscribe_int() != OK)
			return 1;
	printf("ESC key was released.\n");
	return 0;
}


int kbd_test_leds(unsigned short n, unsigned short *leds) {

	unsigned long irq_set, cmd;
	unsigned int it= 0;

	message msg;

	unsigned short i;
	for (i = 0; i < n; i++)
		if ( (leds[i] > 2) || (leds[i] < 0)){
			printf("\n\t Led must be 0 (Scroll Lock), 1 (Numeric Lock) or 2 (Caps Lock).\n");
			return 1;
		}

	kb_set_led(3); // off all leds

	irq_set = timer_subscribe_int();

	if (irq_set < 0)
		return 1;
	else if  (irq_set >= 0)
		irq_set = BIT(irq_set);

	int r, ipc_status;

	while (it < n) {
			if (r = driver_receive(ANY, &msg, &ipc_status)) {
				printf("driver_receive failed with: %d", r);
				continue;
			}

			if (is_ipc_notify(ipc_status)) {
				switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE:
					if (msg.NOTIFY_ARG & irq_set) { /* subscribed interrupt */

							timer_leds_handler(&leds[it]);
							it++;

						}

					break;
				default:
					break; /* no other notifications expected: do nothing */
				}
			} else {/*received a standard message, not a notification*/
				/*no standard messages expected, do nothing*/
			}
		}

	timer_unsubscribe_int();

	return 0;
}




//---------------------------------------lab 3.2---------------------------------------------------------------------------------
int kbd_test_timed_scan(unsigned short n) {
		unsigned short time = 0;

		int irq_set_timer = timer_subscribe_int(), irq_set_keyboard = keyboard_subscribe_int();
		int r, ipc_status;
		message msg;

		long MB_CODE = 0x00;

		if ((irq_set_timer == -1) || (irq_set_keyboard == -1))
			return 1; //message printed in subscribe_int


		while (MB_CODE != ESC_BREAK && time != n) {
			if (r = driver_receive(ANY, &msg, &ipc_status)) {
				printf("driver_receive failed with: 0x%x", r);
				continue;
			}
			if (is_ipc_notify(ipc_status)) {
				switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE:
					if (msg.NOTIFY_ARG & irq_set_timer) { /* subscribed interrupt */
						timed_scan_handler(&time);
					}

					if (msg.NOTIFY_ARG & irq_set_keyboard) {
						time = 0; //reset timer after key action
						MB_CODE = keyboard_c_handler();
						if (MB_CODE & BREAK_CODE)
							printf("Breakcode: 0x%x\n", MB_CODE);
						else
							printf("Makecode: 0x%x\n", MB_CODE);
					}

					break;
				default:
					break; /* no other notifications expected: do nothing */
				}
			} else {/*received a standard message, not a notification*/
				/*no standard messages expected, do nothing*/
			}

	}
		if (time == n)
			printf("\t %lu seconds have passed with no keys pressed/released.\n",(unsigned long) time);

		if (MB_CODE == ESC_BREAK)
			printf("ESC key was released.\n");

		if ((keyboard_unsubscribe_int() != OK) || (timer_unsubscribe_int() != OK))
				return 1;

		return 0;
}
