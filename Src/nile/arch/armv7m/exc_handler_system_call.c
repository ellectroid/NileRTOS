#include "nile/compiler.h"
#include "nile/kernel/syscall_dispatcher.h"

void NILE_NAKED SVC_Handler() {
	 __asm__ volatile (
	        "TST    LR, #4            \n"
	        "BEQ    syscall_skip      \n"
	        "MRS    R3, PSP           \n"

	        "LDR    R0, [R3]          \n"
	        "LDR    R1, [R3, #4]      \n"
	        "LDR    R2, [R3, #8]      \n"

	        "PUSH   {R3, LR}          \n"
	        "BL     syscall_handler_dispatch \n"
	        "POP    {R3, LR}          \n"
		    "STR    R0, [R3]          \n" //write return value

	        "syscall_skip:            \n"
	        "BX     LR                \n"
	    );
}
