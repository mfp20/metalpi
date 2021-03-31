	.file		"crti.S"

	.section	".init"
	.p2align 1
	.global	_init
	.type	_init, @function
_init:
	stm lr, (--sp)
	; E.g. default_ctor_section_asm_out_constructor will align to the
	; pointer size, giving us a stray bkpt instruction here if we're not
	; careful.  Avoid that.
	nop
		
	.section	".fini"
	.p2align 1
	.global	_fini
	.type	_fini, @function
_fini:
	stm lr, (--sp)
	; As above.
	nop
