#include "i8254.h"
#include "timer.h"
#include <minix/syslib.h>
#include <minix/drivers.h>
#include "keyboard.h"


int hook_id = TIMER0_IRQ;

static unsigned long ih_counter = 0;


void timed_scan_handler(unsigned short *time) {

	ih_counter++;
	if(ih_counter % 60 == 0){
		(*time)++;
	}

}

void timer_leds_handler(unsigned short *led) {

	unsigned int count = 0;

	while (count != 1) {

	ih_counter++;
	if(ih_counter % 60 == 0){
		count++;
	}

	kb_set_led(*led);
}


}


int timer_set_square(unsigned long timer, unsigned long freq) { //1st week

	int div;

	unsigned char control, config;

	if (freq <= 0)
	{
		printf("\nFrequency must be a positive integer.\n");
		return 1;
	}

	div = TIMER_FREQ/freq;

	timer_get_conf(timer, &config);

	config &= BITMASK_4LSB;

	switch(timer){
		case 0:
			control = config | BIT(4) | BIT(5) | TIMER_SEL0;
			break;
		case 1:
			control = config | BIT(4) | BIT(5) | TIMER_SEL1;
			break;
		case 2:
			control = config | BIT(4) | BIT(5) | TIMER_SEL2;
			break;
		default:
			{
				printf("\nTimer must range from 0 to 2.\n");
				return 1;
			}
			break;
	}

	if (sys_outb(TIMER_CTRL, (unsigned long) control) != OK)
		{
			printf("\nCouldn't read from Timer.\n");
			return 1;
		}

	timer = TIMER_0 + timer; //timer variable becomes TIMER_X port

	if (sys_outb(timer, div) != OK) //push lsb
		{
			printf("\nCouldn't write LSB to Timer.");
			return 1;
		}

	if (sys_outb(timer, div >> 8) != OK)  //push msb
		{
			printf("\nCouldn't write MSB to Timer.");
			return 1;
		}

	return 0;
}

int timer_subscribe_int(void ) { //2nd week

	if ((sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id) == OK) && (sys_irqenable(&hook_id) == OK))
		return BIT(TIMER0_IRQ); //return activated bit by kernel on hook_id. which is value we set
	else
		{
			printf("\nCouldn't subscribe interrupts.\n");
			return -1;  //return negative on failure
		}
}

int timer_unsubscribe_int() { //2nd week

	if ((sys_irqdisable (&hook_id) == OK) && (sys_irqrmpolicy(&hook_id) == OK))
		return 0;
	else
		{
			printf("\nUnable to unsubscribe interrupts.\n");
			return 1;
		}
}

void timer_int_handler() {  //2nd week

	ih_counter = ih_counter +1;

}

int timer_get_conf(unsigned long timer, unsigned char *st) { //1st week
	
	unsigned long config;

	char read_back;

	if ((timer > 2) || (timer < 0))
			{
				printf("\nTimer must range from 0 to 2.\n");
				return 1;
			}


	read_back = 0 | TIMER_RB_SEL(timer)| TIMER_RB_COUNT_ | BIT(6) | BIT(7);

	if(sys_outb(TIMER_CTRL, read_back) != OK)
		{
			printf("\nCouldn't write to Timer.\n");
			return 1;
		}

	if(sys_inb(TIMER_0+timer, &config ) != OK)
		{
			printf("\nCouldn't read from Timer.\n");
			return 1;
		}

	*st = (unsigned char) config;

	return 0;
}

int timer_display_conf(unsigned char conf) { //1st week
	printf ("\n");
	printf ("	-------------------------\n");
	printf ("	|  TIMER CONFIGURATION  | \n");
	printf ("	-------------------------\n\n");

	printf("	Status byte: 0x%X\n\n"
				"	OUTPUT = %X\n"
				"	Null Counter = %X\n",
				conf, (conf >> 7), (conf >> 6) & BITMASK_001);

	printf("	Type of access:");

			switch (conf & BITMASK_LSB_MSB) {
				case TIMER_LSB:
					printf(" LSB\n");
					break;
				case TIMER_MSB:
					printf(" MSB\n");
					break;
				default:
					printf(" LSB followed by MSB\n");
					break;
				}

			printf("	Operating mode:");

			switch (conf & BITMASK_MODE) {
				case TIMER_SQR_WAVE:
					printf(" --Mode 3-- square wave generator\n");
					break;
				case TIMER_RATE_GEN:
					printf(" --Mode 2-- rate generator\n");
					break;
				case BITMASK_MODE_0:
					printf(" --Mode 0-- interrupt on terminal count \n");
					break;
				case BITMASK_MODE_1:
					printf(" --Mode 1-- hardware retriggerable one-shot\n");
					break;
				case BITMASK_MODE_4:
					printf(" --Mode 4-- software triggered strobe\n");
					break;
				case BITMASK_MODE_5:
					printf(" --Mode 5-- hardware triggered strobe (retriggerable)\n");
					break;

				default:
					printf("Mode doesn't exist.\n");
					break;
						}


			printf("	Counting mode: ");

			switch (conf & BITMASK_001) {
				case TIMER_BCD:
					printf("Count in BCD\n");
					break;
				case TIMER_BIN:
					printf("Count in binary\n");
					break;
			}

			printf ("\n\n");

	return 0;

}

int timer_test_square(unsigned long freq) { //1st week
	//frequency signal test is in set square function
	if (timer_set_square(0, freq) != OK)
	{
		printf("\nCouldn't set square wave generator.\n");
		return 1;
	}

	return 0;
}

int timer_test_int(unsigned long time) { //2nd week
	
	//test

	int ipc_status, r, irq_set = timer_subscribe_int();
	unsigned long seconds = 0;
	message msg;

	if (irq_set == -1)
		return 1; //message printed in subscribe_int

	if(time < 0)
	{
		printf("\nTime must be positive integer.\n");
		return 1;
	}

	timer_set_square(0,60);

	if (time > 0)
		printf("\t\t START\n");

	while( seconds < time )
	{

		if( (r = driver_receive(ANY, &msg, &ipc_status)) != OK )
		{
			printf("driver_receive failed with: %d", r);
			continue;
		}

		if (is_ipc_notify(ipc_status))
			{ /* received notification */
				switch (_ENDPOINT_P(msg.m_source))
				{
				case HARDWARE: /* hardware interrupt notification */
					if (msg.NOTIFY_ARG & irq_set) //msg.NOTIFY_ARG works like hook_id received from kernel. So irq_set must have that value
					{ // subscribed interrupt with subscribe_int function
						timer_int_handler();

						if (ih_counter == 60)
						{
							seconds = seconds +1;
							printf("\t %lu seconds passed out of %lu.\n", seconds, time);
							ih_counter = 0;
						}

					}
					break;
				default:
					break; /* no other notifications expected: do nothing */
				}
			} else
				{ /* received a standard message, not a notification */
				/* no standard messages expected: do nothing */
				timer_unsubscribe_int();
				}
	}

	timer_unsubscribe_int();

	if (time == 0)
		printf("\tTime argument was 0. 0 seconds passed instantly.\n");
	else printf("\t Timer has reached its value.\n");

	return 0;
}

int timer_test_config(unsigned long timer) { //1st week
	
	char TIMER_CONF;

	if (timer_get_conf(timer, &TIMER_CONF) != OK)
	{
		printf("\ntimer_get_conf not successfully executed.\n");
		return 1;
	}

	if (timer_display_conf(TIMER_CONF) != OK)
	{
		printf("\ntimer_display_conf not successfully executed.\n");
		return 1;
	}

	return 0;

}
