	.text

	bkpt
	nop
	sleep
	user
	ei
	di
	cbclr
	cbadd1
	cbadd2
	cbadd3
	rti

	swi	r0
	swi	r13
	swi	pc
	
	rts

	b	r0
	b	r19
	b	pc

	bl	r0
	bl	r18
	bl	pc

	tbb	r2
	tbh	r3

	version	r5

