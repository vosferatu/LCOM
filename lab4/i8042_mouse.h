#ifndef __I8042_MOUSE_H
#define __I8042_MOUSE_H

//bitwise operations

#define BIT(n) (0x01<<(n))

#define IN_OK 	BIT(1)
#define OUT_OK 	BIT(0)

//i8042 ports I/O access

#define STAT_REG 	0x64   	//read kbc state

//use sysoutb e sysinb
//input

#define KBC_CMD 	0x64  	//commands to kbc

#define KBC_ARG 	0x60	//OR arguments of kbc command

//output

#define OUT_BUF 	0x60  //scancodes, return values, protocol messages

//bitmasks

#define READ_CMD_BYTE 0x20

#define MOUSE_IRQ 12

#define STAT_ERR (BIT(7) | BIT(6)) //Parity + timeout

//MOUSE COMMANDS

#define ENABLE_MOUSE 0xA8

#define WRITE_TO_MOUSE 0xD4

#define MOUSE_STATUS 0xE9

#define DISABLE_STREAM 0xF5

#define SET_STREAM 0xEA

#define ENABLE_PACKETS 0xF4


//other

#define DELAY_MS    20000

#define N_TRY 		5

//responses

#define ACK			0xFA	//acknowledge command
#define NACK		0xFE	//RESEND latest byte
#define ERROR		0xFC	//error state


#endif /* __I_8042_MOUSE.H */
