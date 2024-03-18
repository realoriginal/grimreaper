//
// GRIMREAPER
//
// Austin Hudson
//
// suspicious.actor
//

// Section for our code
.section .text$C, "xr"

// Exported symbol name
.globl _GetIp

_GetIp:
	// execute the next instruction
	call	__next_instruction

	__next_instruction:
	// pop the return address off the stack
	popl	%eax

	// subtract the difference between the two labels
	subl	$(__next_instruction - _GetIp), %eax

	// return the pointer
	ret

_Leave:
	.string "ENDOFCODE"
