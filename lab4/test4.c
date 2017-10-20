#include <minix/syslib.h>
#include <minix/drivers.h>
#include "test4.h"
#include "mouse.h"
#include "i8042_mouse.h"
#include "timer.h"


int test_packet(unsigned short cnt){
	int irq_set_mouse = mouse_subscribe_int(), r, ipc_status;
	message msg;
	unsigned char packet[3];

	if (cnt > USHRT_MAX){
		printf("\n\tParameter cnt exceeded 65535.\n");
		return -1;
	}

	if (cnt < 0){
		printf("\n\tParameter cnt lower than 0.\n");
		return -1;
	}

	mouse_interface(SET_STREAM);

	if (irq_set_mouse == -1)
			return 1; //message printed in subscribe_int

	unsigned int i;
	while (i < cnt){
		unsigned int counter = 0;

		while (counter < 3) {
			if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {/* Get a request message. */
				printf("driver_receive failed with: %d", r);
				continue;
			}
			if (is_ipc_notify(ipc_status)) { /* received notification */
				switch (_ENDPOINT_P(msg.m_source)) {
					case HARDWARE: /* hardware interrupt notification */
						if (msg.NOTIFY_ARG & irq_set_mouse) { /* subscribed interrupt */
							mouse_int_handler(counter, packet);
						if((packet[0] != ACK) && (packet[0] & BIT(3))){
							counter++;
							if (counter == 3) {
								printf("\nB1: 0x%2X B2: 0x%2X B3: 0x%2X  LB: %d MB: %d RB: %d  XOV: %d YOV: %d  ",
										packet[0], packet[1], packet[2], (packet[0] & BIT(0)), (packet[0] & BIT(2)) >> 2,
										(packet[0] & BIT(1)) >> 1, (packet[0] & BIT(6)) >> 6, (packet[0] & BIT(7)) >> 7);

								if(packet[0] & BIT(4))
									printf("X: -%3d  ", 256 - packet[1]);
								else
									printf("X:  %3d  ", packet[1]);

								if(packet[0] & BIT(5))
									printf("Y: -%3d", 256 - packet[2]);
								else
									printf("Y:  %3d", packet[2]);
							}
						}
							break;
					default:
						break; /* no other notifications expected: do nothing */
						} else { /* received a standard message, not a notification */
							/* no standard messages expected: do nothing */
						}
				}
			}
		}
		if (i == cnt-1)
			printf ("\n");
		i++;
	}

	mouse_unsubscribe_int();

	return 0;
}	
	
int test_async(unsigned short idle_time) {
	int irq_set_mouse = mouse_subscribe_int(), irq_set_timer = timer_subscribe_int(), r, ipc_status;
	unsigned short time = 0;
	unsigned int counter = 0;
	message msg;
	unsigned char packet[3];

	if (irq_set_mouse == -1)
				return 1; //message printed in subscribe_int

	if (irq_set_timer == -1)
		return 1; //message printed in subscribe_int

	if (idle_time > USHRT_MAX){
		printf("\n\tParameter idle_time exceeded 65535.\n");
		return -1;
	}

	if (idle_time < 0){
		printf("\n\tParameter idle_time lower than 0.\n");
		return -1;
	}

	mouse_interface(SET_STREAM);

	//timer_set_square(0, 60);//to make sure timer 0 configuration is correct

	while (time < idle_time) {
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {/* Get a request message. */
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { /* received notification */
			switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE: /* hardware interrupt notification */
					if (msg.NOTIFY_ARG & irq_set_mouse) { /* subscribed interrupt */
						time = 0;
						mouse_int_handler(counter, packet);
						if ((packet[0] != ACK) && (packet[0] & BIT(3))) {
							counter++;
							if (counter == 3) {
								counter = 0;
								printf("\nB1: 0x%2X B2: 0x%2X B3: 0x%2X  LB: %d MB: %d RB: %d  XOV: %d  YOV: %d  ",
										packet[0], packet[1], packet[2], (packet[0] & BIT(0)), (packet[0] & BIT(2)) >> 2,
										(packet[0] & BIT(1)) >> 1, (packet[0] & BIT(6)) >> 6, (packet[0] & BIT(7)) >> 7);
								if(packet[0] & BIT(4))
									printf("X: -%3d  ", 256 - packet[1]);
								else
									printf("X:  %3d  ", packet[1]);

								if(packet[0] & BIT(5))
									printf("Y: -%3d", 256 - packet[2]);
								else
									printf("Y:  %3d", packet[2]);
							}
						}
					}

					if (msg.NOTIFY_ARG & irq_set_timer) { /*received timer interrupt*/
						timed_scan_handler(&time);
					}
					break;
				default:
					break; /* no other notifications expected: do nothing */
			}
		} else { /* received a standard message, not a notification */
				/* no standard messages expected: do nothing */
		}
	}

	if (time >= idle_time)
		printf("\n\t %lu seconds have passed without mouse interaction.\n",(unsigned long) time);

	mouse_unsubscribe_int();
	timer_unsubscribe_int();

	return 0;
}
	
int test_config(void) {
	int irq_set_mouse = mouse_subscribe_int();
	unsigned char packet[3];

	mouse_interface(DISABLE_STREAM);
	mouse_interface(MOUSE_STATUS);

	unsigned int i;
	for(i = 0; i < 3; i++){
		mouse_int_handler(i, packet);
	}

	printf("\n--------------MOUSE STATUS-------------------------\n");

	if(packet[0] & BIT(6))
		printf("\n\tMODE: Remote(polled).");
	else
		printf("\n\tMODE: Stream.");

	if(packet[0] & BIT(5))
		printf("\n\tDATA REPORTING: Enabled.");
	else
		printf("\n\tDATA REPORTING: Disabled.");

	if(packet[0] & BIT(4))
		printf("\n\tSCALING: 2:1 (non-linear).");
	else
		printf("\n\tSCALING: 1:1 (linear).");
	//bit 3 of packet 1 is always a zero
	// not 2-1-0 for having LB-MB-RB
	if(packet[0] & BIT(0))
		printf("\n\tLEFT MOUSE BUTTON: Pressed.");
	else printf("\n\tLEFT MOUSE BUTTON: Released.");

	if(packet[0] & BIT(2))
		printf("\n\tMIDDLE MOUSE BUTTON: Pressed.");
	else printf("\n\tMIDDLE MOUSE BUTTON: Released.");

	if(packet[0] & BIT(1))
		printf("\n\tRIGHT MOUSE BUTTON: Pressed.");
	else printf("\n\tRIGHT MOUSE BUTTON: Released.");

	printf("\n\tRESOLUTION: %d counts per millimeter.", packet[1]);
	printf("\n\tSAMPLE RATE: %d samples per second.\n\n", packet[2]);

	mouse_unsubscribe_int();

	return 0;
}	
	
/*int test_gesture(short length) {
	int irq_set_mouse = mouse_subscribe_int(), r, ipc_status, distance = 0;
	unsigned int counter = 0;
	message msg;
	unsigned char packet[3];

	if (irq_set_mouse == -1)
		return 1; //message printed in subscribe_int

	if (length == 0){
		printf("\n\tParameter length was 0.\n");
		return -1;
	}

	if (length > SHRT_MAX){
		printf("\n\tParameter length exceeded 32767.\n");
		return -1;
	}

	if (length < SHRT_MIN){
		printf("\n\tParameter length lower than −32767.\n");
		return -1;
	}

	mouse_interface(SET_STREAM);

	while ( ((distance < length) && (length > 0)) || ((distance > length) && (length < 0)) ) {
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {// Get a request message.
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { // received notification
			switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE: // hardware interrupt notification
					if (msg.NOTIFY_ARG & irq_set_mouse) { // subscribed interrupt
						mouse_int_handler(counter, packet);
						if ((packet[0] != ACK) && (packet[0] & BIT(3))) {
							counter++;
							if (counter == 3) {
								counter = 0;
								printf("\nB1: 0x%2X B2: 0x%2X B3: 0x%2X  LB: %d MB: %d RB: %d  XOV: %d  YOV: %d  ",
										packet[0], packet[1], packet[2], (packet[0] & BIT(0)), (packet[0] & BIT(2)) >> 2,
										(packet[0] & BIT(1)) >> 1, (packet[0] & BIT(6)) >> 6, (packet[0] & BIT(7)) >> 7);
								if(packet[0] & BIT(4))
									printf("X: -%3d  ", 256 - packet[1]);
								else
									printf("X:  %3d  ", packet[1]);

								if(packet[0] & BIT(5))
									printf("Y: -%3d", 256 - packet[2]);
								else
									printf("Y:  %3d", packet[2]);

								gesture_machine(packet, &distance);//updates variable distance
								printf("\ndistance: %d\n", distance);
							}

						}
					}
					break;
				default:
					break; // no other notifications expected: do nothing
				}
			} else { //received a standard message, not a notification
					// no standard messages expected: do nothing
			}
	}

	printf("\n\t A positive slope line has been drawn with at least %d length in y,\n\t while the Right Mouse Button was being pressed.\n",(int) length);

	mouse_unsubscribe_int();

	return 0;
}*/

int test_gesture(short length) {
	int irq_set_mouse = mouse_subscribe_int(), r, ipc_status;
	unsigned int counter = 0;
	message msg;
	unsigned char packet[3];
	int slope = -1;

	static event_t gest_event = RDOWN;

	state_t state = INIT;

	if (irq_set_mouse == -1)
		return 1; //message printed in subscribe_int

	if (length == 0){
		printf("\n\tParameter length was 0.\n");
		return -1;
	}

	if (length > SHRT_MAX){
		printf("\n\tParameter length exceeded 32767.\n");
		return -1;
	}

	if (length < SHRT_MIN){
		printf("\n\tParameter length lower than −32767.\n");
		return -1;
	}

	mouse_interface(SET_STREAM);

	while ( state != COMP ) {
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {// Get a request message.
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { // received notification
			switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE: // hardware interrupt notification
					if (msg.NOTIFY_ARG & irq_set_mouse) { // subscribed interrupt
						mouse_int_handler(counter, packet);
						if ((packet[0] != ACK) && (packet[0] & BIT(3))) {
							counter++;
							if (counter == 3) {
								counter = 0;
								printf("\nB1: 0x%2X B2: 0x%2X B3: 0x%2X  LB: %d MB: %d RB: %d  XOV: %d  YOV: %d  ",
										packet[0], packet[1], packet[2], (packet[0] & BIT(0)), (packet[0] & BIT(2)) >> 2,
										(packet[0] & BIT(1)) >> 1, (packet[0] & BIT(6)) >> 6, (packet[0] & BIT(7)) >> 7);
								if(packet[0] & BIT(4))
									printf("X: -%3d  ", 256 - packet[1]);
								else
									printf("X:  %3d  ", packet[1]);

								if(packet[0] & BIT(5))
									printf("Y: -%3d", 256 - packet[2]);
								else
									printf("Y:  %3d", packet[2]);


								if (packet[0] & BIT(1)){ //right button -> drawing state

									gest_event = RDOWN;

									if(packet[0] & BIT(4)){
										if (packet[0] & BIT(5))
											slope = 1;
										else slope = -1;
										}
									else{
										if (packet[0] & BIT(5))
											slope = -1;
										else slope = 1;
									}

									if (slope > 0){
										gest_event = MOVE;
									}
								}
								else {
									gest_event = RUP;
								}

								state = gesture_state_machine(packet, &length, gest_event);
							}

						}
					}
					break;
				default:
					break; // no other notifications expected: do nothing
				}
			} else { //received a standard message, not a notification
					// no standard messages expected: do nothing
			}
	}

	printf("\n\t A positive slope line has been drawn with at least %d length in y,\n\t while the Right Mouse Button was being pressed.\n",(int) length);

	mouse_unsubscribe_int();

	return 0;
}
