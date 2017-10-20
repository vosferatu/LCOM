#include <minix/drivers.h>
#include "test3.h"
#include <limits.h>
#include <string.h>
#include <errno.h>


static int proc_args(int argc, char **argv);
static unsigned short parse_ushort(char *str, int base);
static void print_usage(char **argv);

int main(int argc, char **argv)
{
	/* DO NOT FORGET TO initialize service */

	sef_startup();

	if (argc == 1) {			/* Prints usage of the program if no arguments are passed */
		print_usage(argv);
		return 0;
	}
	else return proc_args(argc, argv);
}

static void print_usage(char **argv)
{
	printf("Usage: one of the following:\n"
			"\t service run %s -args \"scan <decimal no.- if 0, C; else, assembly>\"\n"
			"\t service run %s -args \"leds <decimal no. - leds 0, 1 or 2>\"\n"
			"\t service run %s -args \"timedscan <decimal no. - time (in seconds)>\"\n",
			argv[0], argv[0], argv[0]);
}

static int proc_args(int argc, char **argv) {

	unsigned short ass, time;

	if (strncmp(argv[1], "scan", strlen("scan")) == 0) {
		if (argc != 3) {
			printf("keyboard: wrong no. of arguments for kbd_test_scan()\n");
			return 1;
		}

		ass = parse_ushort(argv[2], 10);  /* Parses string to unsigned short */

		if (ass == USHRT_MAX)
			return 1;

		printf("keyboard::kbd_test_scan(%hu)\n", ass);
		return kbd_test_scan(ass);
	}

	else if (strncmp(argv[1], "leds", strlen("leds")) == 0) {

		if (argc < 3) {
			printf("keyboard: wrong no of arguments for kbd_test_leds()\n");
			return 1;
		}

		unsigned short noleds = argc-2;
		unsigned short leds[noleds];

		unsigned int i;
		for (i = 0; i < noleds; i++){
			leds[i] = parse_ushort(argv[i+2],10); /* Parses string to unsigned short */

			if (leds[i] == USHRT_MAX)
			return 1;
		}


		printf("keyboard::kbd_test_leds(%d)", noleds);

		/*unsigned short j;
		for (j = 0; i < noleds; j++){
			printf("%d, ", leds[j]);

		}
			printf(")");*/

		return kbd_test_leds(noleds, leds);
	}

	else if (strncmp(argv[1], "timedscan", strlen("timedscan")) == 0) {

			if (argc != 3) {
				printf("keyboard: wrong no. of arguments for kbd_test_timed_scan()\n");
				return 1;
			}

			time = parse_ushort(argv[2], 10);			/* Parses string to unsigned short */

			if (time == USHRT_MAX)
				return 1;
			printf("keyboard::kbd_test_timed_scan(%d)\n", time);

			return kbd_test_timed_scan(time);
		}

	else {
		printf("keyboard: %s - no valid function!\n", argv[1]);
		return 1;
	}

}

static unsigned short parse_ushort(char *str, int base)
{
	char *endptr;
	unsigned short val;

	// Convert string to unsigned short

	val = (unsigned short) strtoul(str, &endptr, base);

	/* Check for conversion errors */
	if ((errno == ERANGE && val == USHRT_MAX) || (errno != 0 && val == 0)) {
		perror("strtoul");
		return USHRT_MAX;
	}

	if (endptr == str) {
		printf("keyboard: parse_ushort: no digits were found in %s\n", str);
		return USHRT_MAX;
	}

	/* Successful conversion */
	return val;
}

static unsigned long parse_ulong(char *str, int base)
{
	char *endptr;
	unsigned long val;

	/* Convert string to unsigned long */
	val = strtoul(str, &endptr, base);

	/* Check for conversion errors */
	if ((errno == ERANGE && val == ULONG_MAX) || (errno != 0 && val == 0)) {
		perror("strtoul");
		return ULONG_MAX;
	}

	if (endptr == str) {
		printf("timer: parse_ulong: no digits were found in %s\n", str);
		return ULONG_MAX;
	}

	/* Successful conversion */
	return val;
}



