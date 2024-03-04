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
GLOBAL	_Start

;;
;; Imports
;;
EXTERN	_Entry

[SECTION .text$A]

;;
;; Purpose:
;;
;; Sets up the stack of the thread before executing the
;; entrypoint.
;;
_Start:
	;; setup the stack
	push	ebp
	mov	ebp, esp

	;; execute the entrypoint
	call	_Entry

	;; cleanup the stack
	mov	esp, ebp
	pop	ebp

	;; return
	ret
