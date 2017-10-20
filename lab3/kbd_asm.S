#include "i8042.h"

.global _keyboard_asm_handler

.data

.text

_keyboard_asm_handler:

			xor		%eax, %eax										// EAX = 0
/*			xor 	%ecx, %ecx
			mov 	$N_TRY, %ecx

top:
    		cmp ecx, $0      				// test at top of loop
    		je end       					// loop exit when while condition false


   			in $STAT_REG, %bl
   			test %bl, $IN_OK
   			jz  ticket
   			jmp end

ticket:  	test %bl, $STAT_ERR
   			jz bottom
    		dec ecx
    		jmp top
bottom:*/

			in		$OUT_BUF, %al									// read output buffer
			ret
/*
end: 		xor %eax, %eax
			mov $-1, %al
			ret
*/
