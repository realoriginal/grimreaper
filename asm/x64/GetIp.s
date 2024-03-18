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
.globl GetIp

GetIp:
	// execute the next instruction
	call	_next_instruction

	_next_instruction:
	// pop the return address off the stack
	popq	%rax

	// subtract the difference between the two labels
	subq	$(_next_instruction - GetIp), %rax

	// return the pointer
	ret

Leave:
	// Marker
	.string "ENDOFCODE"
