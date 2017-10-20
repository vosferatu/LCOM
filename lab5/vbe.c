#include <minix/syslib.h>
#include <minix/drivers.h>
#include <machine/int86.h>

#include "vbe.h"
#include "lmlib.h"

#define LINEAR_MODEL_BIT 14

#define PB2BASE(x) (((x) >> 4) & 0x0F000)
#define PB2OFF(x) ((x) & 0x0FFFF)

int vbe_get_mode_info(unsigned short mode, vbe_mode_info_t *vmi_p) {
  
  /* To be completed */

	struct reg86u reg86;
	mmap_t map;

	if (lm_alloc(sizeof(vbe_mode_info_t), &map) == NULL) {
		return 1;
	}

	reg86.u.b.intno = VIDEO_CARD;

	reg86.u.w.ax = VBE_MODE_INFO;
	reg86.u.w.cx = mode;

	reg86.u.w.es = PB2BASE(map.phys); //ES:DI
	reg86.u.w.di = PB2OFF(map.phys);

	if (sys_int86(&reg86) != OK) {
		return 1;
	}

	switch (reg86.u.w.ax) {
		case VBE_CALL_FAILED:
			return 1;
			break;
		case VBE_UNSUPPORTED:
			return 1;
			break;
		case VBE_INVALID_MODE:
			return 1;
			break;
		default:
			break;
	}

	*vmi_p = *((vbe_mode_info_t *) map.virtual);

	lm_free(&map);

	return 0;

}

int vbe_get_controller_info(){

	vbe_info_t* controller;
	mmap_t map;
	struct reg86u reg86;

	if (lm_alloc(sizeof(vbe_info_t), &map) == NULL) {
		return 1;
	}

	strcpy(controller->VbeSignature, "VBE2");

	reg86.u.b.intno = VIDEO_CARD;
	reg86.u.w.ax = VBE_CTRL_INFO;
	reg86.u.w.es = PB2BASE(map.phys);
	reg86.u.w.di = PB2OFF(map.phys);

	if (sys_int86(&reg86)!=OK)
	{
			return 1;
	}

	switch (reg86.u.w.ax) {
		case VBE_CALL_FAILED:
			return 1;
			break;
		case VBE_UNSUPPORTED:
			return 1;
			break;
		case VBE_INVALID_MODE:
			return 1;
			break;
	}

	//*controller = *((vbe_info_t*) map.virtual);

	printf("\nThe controller capabilities:\n");

	if(controller->Capabilities[0] & BIT(0))
		printf("\tDAC width is switchable to 8 bits per primary color.\n");

	else
		printf("\tDAC is fixed width, with 6 bits per primary color.\n");


	if(controller->Capabilities[0] & BIT(1))
		printf("\tController isn't VGA compatible.\n");

	else
		printf("\tController is VGA compatible.\n");


	if(controller->Capabilities[0] & BIT(2))
		printf("\tWhen programming large blocks of information to the RAMDAC, use the blank bit in Function 09h.\n");

	else
		printf("\tNormal RAMDAC operation.\n");


	printf("\n\tList of mode numbers supported:\n");
	int i = 1;
	uint16_t* farptr = (void *)((controller->VideoModePtr & 0xFFFF0000) >> 12);
	farptr += PB2OFF(controller->VideoModePtr);
	farptr += (uint32_t) map.virtual & 0xF0000000;

	while (*farptr != VIDEO_MODE_END)
	{
		printf("%d - 0x%04X\t", i, (unsigned short)*farptr);
		farptr++;
		if (!(i % 5))
			printf("\n");
		i++;
	}

	printf("\n\nSize of VRAM memory: %d blocks of 64kb\n", controller->TotalMemory);

	return 0;
}
