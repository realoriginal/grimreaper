//
// GRIMREAPER
//
// Austin Hudson
//
// suspicious.actor
//

// Section for our code
.section .text$A, "xr"

// Exported symbol name
.globl _Start

// Imported symbol name
.extern _Entry@0

_Start:
	// Setup the stack
	pushl	%ebp
	movl	%esp, %ebp

	// Execute the entrypoint
	call	_Entry@0

	// Cleanup the stack
	movl	%ebp, %esp
	popl	%ebp

	// Return
	ret
