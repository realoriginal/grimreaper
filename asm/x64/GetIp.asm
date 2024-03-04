;;
;; GRIMREAPER
;;
;; Austin Hudson
;;
;; suspicious.actor
;;
[BITS 64]

;;
;; Labels
;;
GLOBAL	GetIp

[SECTION .text$C]

;;
;; Purpose:
;;
;; Returns the address of itself to the callee
;;
GetIp:
	;; call the next instruction
	call	_next_instruction

	_next_instruction:
	;; pop the return address from the stack
	pop	rax

	;; subtract the difference from _next_instruction to GetIp
	sub	rax, 5

	;; return the address to the callee
	ret

;;
;; Purpose:
;;
;; End of code marker for the extraction script
;;
Leave:
	db 'ENDOFCODE'
