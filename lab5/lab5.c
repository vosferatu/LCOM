#include <minix/syslib.h>
#include <minix/drivers.h>
#include "test5.h"
#include "pixmap.h"

#define H_RES   1024  //direct-color mode
#define V_RES	768

static int proc_args(int argc, char *argv[]);

static long parse_long(char *str, int base);

static unsigned long parse_ulong(char *str, int base);

static void print_usage(char *argv[]);



int main(int argc, char **argv) {

	/* DO NOT FORGET TO initialize service */

	sef_startup();

	if ( argc == 1 ) {

		print_usage(argv);		/* Prints usage of the program if no arguments are passed */
		return 0;

	} else return proc_args(argc, argv);

}

static void print_usage(char *argv[]) {
	printf("Usage: one of the following:\n"
		"\t service run %s -args \"test_init <unsigned short mode> <unsigned short delay>\" \n"
		"\t service run %s -args \"test_square <unsigned short x> <unsigned short y> <unsigned short size> <unsigned long color>\" \n"
		"\t service run %s -args \"test_line <unsigned short xi> <unsigned short yi> <unsigned short xf> <unsigned short yf> <unsigned long color> \" \n"
		"\t service run %s -args \"test_xpm <unsigned short xi> <unsigned short yi> <char array xpm>\" \n",
		"\t service run %s -args \"test_move <unsigned short xi> <unsigned short yi> <char array xpm> <unsigned short hor> <short delta> <unsigned short time>\" \n",
		"\t service run %s -args \"test_controller\" \n",
			argv[0], argv[0], argv[0], argv[0], argv[0], argv[0]);
}


static int proc_args(int argc, char *argv[]) {

	unsigned short mode, delay, x, y, size, xf, yf, hor, time;
	short delta;
	unsigned long color;




	if (strncmp(argv[1], "test_init", strlen("test_init")) == 0) {
		if (argc != 4) {
			printf("VBE: wrong no of arguments for test_init() \n");
			return 1;
		}

		if ((mode = parse_ulong(argv[2], 16)) == ULONG_MAX)
			return 1;

		if ((delay = parse_ulong(argv[3], 10)) == ULONG_MAX)
			return 1;

		printf("VBE:: test_init(0x%x, %d)\n", mode, delay);

		if(test_init(mode, delay) == NULL)
			return 1;

		return 0;
	}

	else if (strncmp(argv[1], "test_square", strlen("test_square")) == 0) {
		if (argc != 6) {
			printf("VBE: wrong no of arguments for test_square() \n");
			return 1;
		}

		if ((x = parse_ulong(argv[2], 10)) > H_RES)
			return 1;

		if ((y = parse_ulong(argv[3], 10)) > V_RES)
			return 1;

		if((size = parse_ulong(argv[4], 10)) == ULONG_MAX)
			return 1;

		if (x + size > H_RES || y + size > V_RES)
			return 1;

		if ((color = parse_ulong(argv[5], 16)) == ULONG_MAX)
			return 1;

		printf("VBE::test_square(%d, %d, %d, %d)\n", x, y, size, color);

		return test_square(x, y, size, color);
	}

	else if (strncmp(argv[1], "test_line", strlen("test_line")) == 0) {
		if (argc != 7) {
			printf("VBE: wrong no of arguments for test_line() \n");
			return 1;
		}

		if ((x = parse_ulong(argv[2], 10)) > H_RES)
			return 1;

		if ((y = parse_ulong(argv[3], 10)) > V_RES)
			return 1;

		if ((xf = parse_ulong(argv[4], 10)) > H_RES)
			return 1;

		if ((yf = parse_ulong(argv[5], 10)) > V_RES)
			return 1;

		if ((color = parse_ulong(argv[6], 10)) == ULONG_MAX)
			return 1;

		printf("VBE::test_line(%d, %d, %d, %d, %d)\n", x, y, xf, yf, color);

		return test_line(x, y, xf, yf, color);
	}

	else if (strncmp(argv[1], "test_xpm", strlen("test_xpm")) == 0) {
		if (argc != 4) {
			printf("VBE: wrong no of arguments for test_xpm() \n");
			return 1;
		}

		if ((x = parse_ulong(argv[2], 10)) > H_RES)
			return 1;

		if ((y = parse_ulong(argv[3], 10)) > V_RES)
			return 1;

		//TODO choose sprite
		extern char *penguin[];

		printf("VBE::test_xpm(%d,%d)\n", x, y);

		return test_xpm(x, y, penguin);

	}
	else if (strncmp(argv[1], "test_move", strlen("test_move")) == 0) {
		if (argc != 7) {
			printf("VBE: wrong no of arguments for test_move() \n");
			return 1;
		}

		if ((x = parse_ulong(argv[2], 10)) > H_RES)
			return 1;

		if ((y = parse_ulong(argv[3], 10)) > V_RES)
			return 1;

		if ((hor = parse_ulong(argv[4], 10)) == ULONG_MAX)
			return 1;

		if ((delta = parse_long(argv[5], 10)) == LONG_MAX)
			return 1;

		if ((time = parse_ulong(argv[6], 10)) == ULONG_MAX)
			return 1;

		//TODO choose sprite
		extern char *penguin[];

		printf("VBE::test_move(%d, %d, penguin, %d, %d, %d)\n", x, y, hor, delta,time);

		return test_move(x, y, penguin, hor,delta,time);

	}

	else if (strncmp(argv[1], "test_controller", strlen("test_controller")) == 0) {
		if (argc != 2) {
			printf("VBE: wrong no of arguments for test_controller() \n");
			return 1;
		}

		printf("VBE::test_controller()\n");

		return test_controller();

	}

	else {
		printf("VBE: non valid function \"%s\" to test\n", argv[1]);
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

