

	.text

	stm	r6, (--sp)
	ldm	r6, (sp++)
	stm	r6, lr, (--sp)
	ldm	r6, pc, (sp++)
	stm	r0, (--sp)
	stm	r16, (--sp)
	stm	gp, (--sp)
	ldm	r0, (sp++)
	ldm	r16, (sp++)
	ldm	gp, (sp++)
	stm	r0, lr, (--sp)
	stm	r16, lr, (--sp)
	stm	gp, lr, (--sp)
	ldm	r0, pc, (sp++)
	ldm	r16, pc, (sp++)
	ldm	gp, pc, (sp++)
	
	stm	r6-r5, (--sp)
	stm	r6-r6, (--sp)
	stm	r6-r7, (--sp)
	stm	r6-r0, (--sp)
	stm	r6-pc, (--sp)
	stm	r6-r5, lr, (--sp)
	stm	r6-r6, lr, (--sp)
	stm	r6-r7, lr, (--sp)
	stm	r6-r0, lr, (--sp)
	stm	r6-pc, lr, (--sp)
	
	ldm	r6-r5, (sp++)
	ldm	r6-r6, (sp++)
	ldm	r6-r7, (sp++)
	ldm	r6-r0, (sp++)
	ldm	r6-pc, (sp++)
	ldm	r6-r5, pc, (sp++)
	ldm	r6-r6, pc, (sp++)
	ldm	r6-r7, pc, (sp++)
	ldm	r6-r0, pc, (sp++)
	ldm	r6-pc, pc, (sp++)

	stm	r0-r5, (--sp)
	stm	r0-r6, (--sp)
	stm	r0-r7, (--sp)
	stm	r0-r0, (--sp)
	stm	r0-pc, (--sp)
	stm	r0-r5, lr, (--sp)
	stm	r0-r6, lr, (--sp)
	stm	r0-r7, lr, (--sp)
	stm	r0-r0, lr, (--sp)
	stm	r0-pc, lr, (--sp)
	
	ldm	r0-r5, (sp++)
	ldm	r0-r6, (sp++)
	ldm	r0-r7, (sp++)
	ldm	r0-r0, (sp++)
	ldm	r0-pc, (sp++)
	ldm	r0-r5, pc, (sp++)
	ldm	r0-r6, pc, (sp++)
	ldm	r0-r7, pc, (sp++)
	ldm	r0-r0, pc, (sp++)
	ldm	r0-pc, pc, (sp++)

	stm	r16-r5, (--sp)
	stm	r16-r6, (--sp)
	stm	r16-r7, (--sp)
	stm	r16-r0, (--sp)
	stm	r16-pc, (--sp)
	stm	r16-r5, lr, (--sp)
	stm	r16-r6, lr, (--sp)
	stm	r16-r7, lr, (--sp)
	stm	r16-r0, lr, (--sp)
	stm	r16-pc, lr, (--sp)
	
	ldm	r16-r5, (sp++)
	ldm	r16-r6, (sp++)
	ldm	r16-r7, (sp++)
	ldm	r16-r0, (sp++)
	ldm	r16-pc, (sp++)
	ldm	r16-r5, pc, (sp++)
	ldm	r16-r6, pc, (sp++)
	ldm	r16-r7, pc, (sp++)
	ldm	r16-r0, pc, (sp++)
	ldm	r16-pc, pc, (sp++)

	stm	gp-r5, (--sp)
	stm	gp-r6, (--sp)
	stm	gp-r7, (--sp)
	stm	gp-r0, (--sp)
	stm	gp-pc, (--sp)
	stm	gp-r5, lr, (--sp)
	stm	gp-r6, lr, (--sp)
	stm	gp-r7, lr, (--sp)
	stm	gp-r0, lr, (--sp)
	stm	gp-pc, lr, (--sp)
	
	ldm	gp-r5, (sp++)
	ldm	gp-r6, (sp++)
	ldm	gp-r7, (sp++)
	ldm	gp-r0, (sp++)
	ldm	gp-pc, (sp++)
	ldm	gp-r5, pc, (sp++)
	ldm	gp-r6, pc, (sp++)
	ldm	gp-r7, pc, (sp++)
	ldm	gp-r0, pc, (sp++)
	ldm	gp-pc, pc, (sp++)

	# These are really "st r1,(r25)--", "ld r1,(r25)++" (and are two
        # words each!).  Actually we're not providing aliases for these any
        # more.
	st	r1, (--sp)
	st	r3, (--sp)
	ld	r2, (sp++)
	ld	r7, (sp++)
