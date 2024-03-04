;;
;; GRIMREAPER
;;
;; Austin Hudson
;;
;; suspicious.actor
;;
[BITS 32]

;;
;; Labels
;;
GLOBAL	_GetIp

[SECTION .text$C]

;;
;; Purpose:
;;
;; Returns the address of itself to the callee
;;
_GetIp:
	;; call the next instruction
	call	__next_instruction

	__next_instruction:
	;; pop the return address from the stack
	pop	eax

	;; subtract the difference from _next_instruction to GetIp
	sub	eax, $__next_instruction - $_GetIp

	;; return the address to the callee
	ret

;;
;; Purpose:
;;
;; End of code marker for the extraction script
;;
_Leave:
	db 'ENDOFCODE'
