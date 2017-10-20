#include <minix/syslib.h>
#include <minix/drivers.h>
#include "test4.h"

static void print_usage(char *argv[]);
static int proc_args(int argc, char *argv[]);
static unsigned long parse_ulong(char *str, int base);
static long parse_long(char *str, int base);



int main(int argc, char **argv) {

	sef_startup();

	/* DO NOT FORGET TO initialize service */

	if ( argc == 1 ) {
		print_usage(argv);						/* Prints usage of the program if no arguments are passed */
		return 0;
	} else return proc_args(argc, argv);

	return 0;
}



static void print_usage(char *argv[]) {
	printf("Usage: one of the following:\n"
			"\t service run %s -args \"packet <cnt>\" \n"
			"\t service run %s -args \"async <idle_time>\" \n"
			"\t service run %s -args \"config\" \n"
			"\t service run %s -args \"gesture <length>\" \n" ,
			argv[0], argv[0], argv[0], argv[0]);
}



static int proc_args(int argc, char *argv[]) {

	unsigned long cnt, idle_time;
	long length;

	if (strncmp(argv[1], "packet", strlen("packet")) == 0) {
		if(argc != 3) {
			printf("mouse: wrong no. of arguments for test_packet() \n");
			return 1;
		}
		if( (cnt = parse_ulong(argv[2], 10)) == ULONG_MAX )
			return 1;
		printf("mouse:: test_packet(%lu)\n", cnt);
		return test_packet(cnt);
	} else if (strncmp(argv[1], "async", strlen("async")) == 0) {
		if(argc != 3) {
			printf("mouse: wrong no. of arguments for test_async() \n");
			return 1;
		}
		if( (idle_time = parse_ulong(argv[2], 10)) == ULONG_MAX )
			return 1;
		printf("mouse:: test_async(%lu)\n", idle_time);
		return test_async(idle_time);
	} else if (strncmp(argv[1], "config", strlen("config")) == 0) {
		if(argc != 2) {
			printf("mouse: wrong no. of arguments for test_config() \n");
			return 1;
		}
		printf("mouse:: test_config()\n");
		return test_config();
	} else if (strncmp(argv[1], "gesture", strlen("gesture")) == 0) {
		if(argc != 3) {
			printf("mouse: wrong no of arguments for test_gesture() \n");
			return 1;
		}
		if( (length = parse_long(argv[2], 10)) == LONG_MAX )
			return 1;
		printf("mouse:: test_gesture(%ld)\n", length);
		return test_gesture(length);
	} else {
		printf("mouse: %s - no valid function!\n", argv[1]);
		return 1;
	}
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



static long parse_long(char *str, int base) {
	char *endptr;
	unsigned long val;

	/* Convert string to long */
	val = strtol(str, &endptr, base);

	/* Check for conversion errors */
	if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
			|| (errno != 0 && val == 0)) {
		perror("strtol");
		return LONG_MAX;
	}

	if (endptr == str) {
		printf("mouse: parse_long: no digits were found in %s \n", str);
		return LONG_MAX;
	}

	/* Successful conversion */
	return val;
}
