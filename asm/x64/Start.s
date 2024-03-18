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
.globl Start

// Imported symbol name
.extern Entry

Start:
	// Setup the stack
	pushq	%rsi
	movq	%rsp, %rsi
	andq	$-16, %rsp
	
	// Execute the entrypoint
	subq	$32, %rsp
	call	Entry

	// Cleanup the stack
	movq	%rsi, %rsp
	popq	%rsi

	// Return
	ret
