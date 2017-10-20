#include "vbe.h"
#include "test5.h"
#include "keyboard.h"


void *test_init(unsigned short mode, unsigned short delay) {
	
	/* To be completed */
	
	void *video_card;
	vbe_mode_info_t vmi_p;

	if ((video_card = vg_init(mode)) == NULL)
		return NULL;

	if (vbe_get_mode_info(mode, &vmi_p) != 0){
		printf("\tvbe_get_mode_info() unsuccessfully executed.\n");
		return NULL;
	}

	timer_test_int(delay);

	if (vg_exit() != 0){
		printf("\tvg_exit() unsuccessfully executed.\n");
		return NULL;
	}

	printf("\n\t\t::::::VBE Mode Info::::::\n\n");
	printf("\tX resolution: %d\n\tY resolution: %d\n\tBits per pixel: %d\n\tPhysical address of the Video RAM: Ox%X\n",
				vmi_p.XResolution, vmi_p.YResolution, vmi_p.BitsPerPixel, vmi_p.PhysBasePtr);

	return video_card;

}


int test_square(unsigned short x, unsigned short y, unsigned short size, unsigned long color) {
	
	/* To be completed */
	
	if (vg_init(VBE_STD_MODE) == NULL){
		return 1;

	}

	if (vg_square(x, y, size, color) != 0) {
		vg_exit();
		printf("\tvg_square() unsuccessfully executed.\n");
		return 1;
	}

	if(kbd_scan() != 0){
		vg_exit();
		printf("\tkbd_scan() unsuccessfully executed.\n");
		return 1;
	}

	if (vg_exit() != 0){
		return 1;
	}

	printf("\n\tThe ESC key was Released.\n");

	return 0;

}

int test_line(unsigned short xi, unsigned short yi, 
		           unsigned short xf, unsigned short yf, unsigned long color) {
	
	/* To be completed */
	
	if (vg_init(VBE_STD_MODE) == NULL)
		return 1;

	unsigned short store = xf;

	if(xi > xf){
		xf = xi;
		xi = store;
	}

	if(yi > yf){
		store = yf;
		yf = yi;
		yi = store;
	}

	if (vg_line(xi, yi, xf, yf, color) != 0) {
		vg_exit();
		printf("\tvg_line() unsuccessfully executed.\n");
		return 1;
	}

	if(kbd_scan() != 0){
		vg_exit();
		printf("\tkbd_scan() unsuccessfully executed.\n");
		return 1;
	}

	if (vg_exit() != 0){
		return 1;
	}

	if((yi == yf) && (xi == xf)){
		printf("\tInput only produced point (%d,%d).\n", xi, yi);
	}

	printf("\n\tThe ESC key was Released.\n");

	return 0;

}

int test_xpm(unsigned short xi, unsigned short yi, char *xpm[]) {
	
	/* To be completed */
	
	if (vg_init(VBE_STD_MODE) == NULL){
		return 1;

	}

	if(vg_xpm(xi, yi, xpm) != 0) {
		vg_exit();
		printf("\tvg_xpm() unsuccessfully executed.\n");
		printf("\tvg_xpm() YOUR values are out of bounds.\n");
		return 1;
	}

	if(kbd_scan() != 0){
		vg_exit();
		printf("\tkbd_scan() unsuccessfully executed.\n");
		return 1;
	}

	if(vg_exit() != 0){
		return 1;
	}

	printf("\n\tThe ESC key was Released.\n");

	return 0;

}	

int test_move(unsigned short xi, unsigned short yi, char *xpm[], 
				unsigned short hor, short delta, unsigned short time) {
	
	/* To be completed */
	
	if(time <= 0){
		printf("\tTIME can't be NULL or NEGATIVE.\n");
		return 1;
	}

	if (vg_init(VBE_STD_MODE) == NULL){
		return 1;

	}

	if(vg_xpm_move(xi, yi, xpm, hor, delta, time) != 0) {
		printf("\tvg_xpm_move() unsuccessfully executed.\n");
		return 1;
	}

	return 0;
}					

int test_controller() {
	
	/* To be completed */

	if(lm_init() == NULL)
		return 1;

	if(vbe_get_controller_info() != 0)
		return 1;
	
	return 0;

}					
	
