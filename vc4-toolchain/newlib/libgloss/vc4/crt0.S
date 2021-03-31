	.text
	.global _start
_start:
	mov r28, #_fstack
	sub sp, r28, #4096

	bl __vc4_init

	mov r0, #_fbss
	mov r2, #_ebss
	sub r2, r0
	mov r1, #0
	bl memset

	; Set up to call global destructors
	mov r0, _fini
	bl atexit

	; Call global constructors
	bl _init

	; In case main tries to look at argc, argv (some GCC tests do).
	mov r0, #0
	mov r1, #0
	mov r2, #0

	bl main
	bl exit
	rts
 
