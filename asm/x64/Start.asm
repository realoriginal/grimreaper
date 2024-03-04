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
GLOBAL	Start

;;
;; Imports
;;
EXTERN	Entry

[SECTION .text$A]

;;
;; Purpose:
;;
;; Sets up the stack of the thread before executing the
;; entrypoint.
;;
Start:
	;; setup the stack
	push	rsi
	mov	rsi, rsp
	and	rsp, 0FFFFFFFFFFFFFFF0h

	;; execute the entrypoint
	sub	rsp, 020h
	call	Entry

	;; cleanup the stack
	mov	rsp, rsi
	pop	rsi

	;; return
	ret
