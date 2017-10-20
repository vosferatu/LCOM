#ifndef __I8042_H_
#define __I8042_H_

//bitwise operations

#define BIT(n) (0x01<<(n))

#define IN_OK 	BIT(1)
#define OUT_OK 	BIT(0)

//i8042 ports I/O access

#define STAT_REG 	0x64   	//read kbc state

//use sysoutb e sysinb
//input

#define KBD_ARG 	0x60  	//arguments to keyboard command

#define KBD_CMD 	0x60  	//commands to keyboard

#define KBC_CMD 	0x64  	//commands to kbc

#define KBC_ARG 	0x60	//OR arguments of kbc command

//output

#define OUT_BUF 	0x60  //scancodes, return values, protocol messages


//bitmasks

#define ESC_BREAK 0x81

#define KBD_LED_CMD 0xED

#define READ_CMD_BYTE 0x20

#define TWO_BYTE_SCAN 0xE0

#define KEYBOARD_IRQ 1

#define BREAK_CODE BIT(7)

#define STAT_ERR (BIT(7) | BIT(6)) //Parity + timeout

//other

#define DELAY_MS    20000

#define N_TRY 		5



//responses

#define KBD_ACK			0xFA	//acknowledge command
#define KBD_RESEND		0xFE	//RESEND latest byte
#define KBD_ERROR		0xFC	//error state




/**@}*/

#endif /* _LCOM_I8254_H */
