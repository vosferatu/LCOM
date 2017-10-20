#include <minix/syslib.h>
#include <minix/drivers.h>
#include <machine/int86.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "vbe.h"
#include "lmlib.h"
#include "read_xpm.h"
#include "i8254.h"
#include "i8042.h"

/* Constants for VBE 0x105 mode */

/* The physical address may vary from VM to VM.
 * At one time it was 0xD0000000
 *  #define VRAM_PHYS_ADDR    0xD0000000 
 * Currently on lab B107 is 0xF0000000
 * Better run my version of lab5 as follows:
 *     service run `pwd`/lab5 -args "mode 0x105"
 */
#define VRAM_PHYS_ADDR	0xE0000000		//JoaoMENDES PC settings
#define H_RES           1024
#define V_RES		  	768
#define BITS_PER_PIXEL	  8

/* Private global variables */

static char *video_mem;		/* Process address to which VRAM is mapped */

static unsigned h_res;		/* Horizontal screen resolution in pixels */
static unsigned v_res;		/* Vertical screen resolution in pixels */
static unsigned bits_per_pixel; /* Number of VRAM bits per pixel */

int vg_exit() {
  struct reg86u reg86;

  reg86.u.b.intno = 0x10; /* BIOS video services */

  reg86.u.b.ah = 0x00;    /* Set Video Mode function */
  reg86.u.b.al = 0x03;    /* 80x25 text mode*/

  if( sys_int86(&reg86) != OK ) {
      printf("\tvg_exit(): sys_int86() failed \n");
      return 1;
  } else{
	  printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
      return 0;
  }
}

void *vg_init(unsigned short mode){

	/* To be completed */

	vbe_mode_info_t vmi_p;
	struct reg86u reg86;
	struct mem_range mr;
	unsigned long size_VRAM = 0;

	reg86.u.b.intno = VIDEO_CARD;
	reg86.u.w.ax = VBE_SET_MODE;
	reg86.u.w.bx = (LINEAR_MODE | mode);

	if (sys_int86(&reg86) != OK) {
		return NULL;
	}

	if (lm_init() == NULL) {
		return NULL;
	}


	switch (reg86.u.w.ax) {
		case VBE_CALL_FAILED:
			return NULL;
			break;
		case VBE_UNSUPPORTED:
			return NULL;
			break;
		case VBE_INVALID_MODE:
			return NULL;
			break;
		default:
			break;
	}


	if (vbe_get_mode_info(mode, &vmi_p) != OK) {
		return NULL;
	}

	//global variables provided
	h_res = vmi_p.XResolution;
	v_res = vmi_p.YResolution;
	bits_per_pixel = vmi_p.BitsPerPixel;

	size_VRAM = h_res*v_res*(bits_per_pixel/8);

	mr.mr_base = (phys_bytes)(vmi_p.PhysBasePtr);
	mr.mr_limit = mr.mr_base + size_VRAM;

	if (sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr) != OK){
		panic("video_txt: sys_privctl (ADD_MEM) failed: %d\n");
		//return 1;
	}

	video_mem = vm_map_phys(SELF, (void *) mr.mr_base, size_VRAM);

	if(video_mem == MAP_FAILED){
		return NULL;
	}

	return video_mem;
}

void set_pixel(unsigned short x, unsigned short y, unsigned long color) {

	char *pixel = video_mem + x*bits_per_pixel/8 + h_res*y*bits_per_pixel/8;
	*pixel = (char) color;

	return;
}

int vg_square(unsigned short x, unsigned short y, unsigned short size, unsigned long color) {

	if ((x < 0) || (x >= h_res) || (y < 0) || (y >= v_res) || (color < 0) || (color > 256)) {
		return 1;
	}

	int height, width;
	unsigned int bytes_pixel;
	char *pixel;

	pixel = video_mem;

	if ((bits_per_pixel/8) <= 0)
		bytes_pixel = 1;
	else
		bytes_pixel = bits_per_pixel/8;


	for (height = 0; height < size; height++){

		pixel += (y + height) * bytes_pixel * h_res + x * bytes_pixel;

		for (width = 0; width < size; width++){
			*pixel = (char) color;
			pixel += bytes_pixel;
		}

		pixel = video_mem;
	}

	return 0;
}

int vg_line(unsigned short xi, unsigned short yi,  unsigned short xf, unsigned short yf, unsigned long color){

	if ((xi < 0) || (xi >= h_res) || (yi < 0) || (yi >= v_res) || (color < 0) || (color > 256)) {
		return 1;
	}

	if ((xf < 0) || (xf >= h_res) || (yf < 0) || (yf >= v_res) || (color < 0) || (color > 256)) {
		return 1;
	}

	unsigned int i;

	if(xf == xi){
		for(i = yi; i <= yf; i++){
			set_pixel(xi, i, color);
		}

		return 0;
	}

	if (yf == yi){
		for (i = xi; i < xf; i++){
			set_pixel(i, yi, color);
		}

		return 0;
	}

	unsigned int diff;
	float slope = ((double) (yf - yi)) / (xf - xi);//slope of the straight line between the 2 points

	if (slope < 0)
		diff = 1;
	else diff = 0;

	if ((slope > -1) && (slope < 1)) {
		for (i = xi; i <= xf; i++) {
			set_pixel(i, diff * yi + (int) (i * slope), color);//"calculates" the next point
		}
	} else {
		for (i = yi; i <= yf; i++) {
			set_pixel(diff * xi + (int) (i / slope), i, color);//"calculates" the next point
		}
	}

	return 0;
}

int draw_map(unsigned short x, unsigned short y, unsigned short width, unsigned short height, char *map) {

	if ( ((y + height) >= v_res) || ((x + width) >= h_res) || ((x < 0) || (y < 0)) ){
		return 1;
	}

	unsigned int i, j, k = 0;
	for (i = 0; (i < height) && ((y + i) < v_res) ; i++) {
		for (j = 0; (j < width) && ((x + j) < h_res); j++) {
			set_pixel(x + j, y + i, map[k]);//k gets the color
			k++;
		}
	}

	if((j >= width) && (i >= height))
		return 0;

	/*unsigned short new_y = y;//this piece of code "transports" the part of the image that passes the limits
								//to the opposite side, displaying both parts at once
	if((new_y + i) >= v_res){
		new_y = 0;
	}

	for (i = 0; i < height ; i++) {
		for (j = 0; j < width; j++) {
			set_pixel(x+j, new_y + i, map[k]);//k gets the color
			k++;
		}
	}*/

	return 0;
}

int vg_xpm(unsigned short xi, unsigned short yi, char *xpm[]){

	int width, height;
	char *img;

	if ((img = read_xpm(xpm, &width, &height)) == NULL)
		return 1;

	if(draw_map(xi, yi, (unsigned short) width, (unsigned short) height, img) != 0){
		free(img);
		return 1;
	}

	free(img); //malloc in read_xpm
	return 0;
}

void clear_screen(){
	unsigned int i, j;
	for(i = 0; i < v_res; i++){
		for(j = 0; j < h_res; j++){
			set_pixel(j, i, 0);
		}
	}
}

int vg_xpm_move(unsigned short xi, unsigned short yi, char *xpm[],
		unsigned short hor, short delta, unsigned short time){
	unsigned short time_total = 0;

	int irq_set_timer = timer_subscribe_int(), irq_set_keyboard = keyboard_subscribe_int(), width, height;
	int r, ipc_status;
	message msg;
	float speed = 0, position = 0;

	long B_CODE = 0x00;

	if ((irq_set_timer == -1) || (irq_set_keyboard == -1))
		return 1; //message printed in subscribe_int

	char *img;

	if ((img = read_xpm(xpm, &width, &height)) == NULL)
		return 1;

	speed = (float) (delta) / (time * 60);

	while ((B_CODE != ESC_BREAK) && (time_total != time)) {
		if (r = driver_receive(ANY, &msg, &ipc_status)) {
			printf("driver_receive failed with: 0x%x", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.NOTIFY_ARG & irq_set_timer) { /* subscribed interrupt */

					timed_scan_handler(&time_total);

					if (hor == 0) {
						clear_screen();
						if ((xi + width + (short) position) >= (short) h_res)
							draw_map((h_res - width - 1), yi, width, height, img);

						if ((xi + (short) position) < 0)
							draw_map(0, yi, width, height, img);

						draw_map((xi + (short) position), yi, width, height, img);

					} else {

						if ((yi + height + (short) position) >= (short) v_res)
							draw_map(xi, (v_res - height - 1), width, height, img);

						if ((yi + (short) position) < 0)
							draw_map(xi, 0, width, height, img);

						draw_map(xi, (yi + (short) position), width, height, img);
					}

					//clear_screen();
					position = position + speed;
				}

				if (msg.NOTIFY_ARG & irq_set_keyboard) {
					B_CODE = keyboard_c_handler();
				}

				break;
			default:
				break; /* no other notifications expected: do nothing */
			}
		} else {/*received a standard message, not a notification*/
			/*no standard messages expected, do nothing*/
		}

	}

	free(img);

	if(vg_exit() != 0){
		return 1;
	}

	if (time_total >= time){
		printf("\t%d seconds of movement have passed.\n", time);
	}

	if (B_CODE == ESC_BREAK){
		printf("\n\tThe ESC key was Released.\n");
	}


	if ((keyboard_unsubscribe_int() != OK) || (timer_unsubscribe_int() != OK))
		return 1;

	return 0;
}

