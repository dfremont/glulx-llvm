; Include file imported by all programs compiled with the Glulx LLVM backend.

; Default stack size; we use the I7 default of 64k.
	!stack 65536

; Program entry point; does nothing except call the user's main() function.
	!main 0
	callf main_, 0
	quit

; Macros to replace/translate GAS glasm doesn't know about and which
; we haven't hacked LLVM to remove.
	!macro ".file"
	!endm
	!macro ".p2align"
	!align \1
	!endm
