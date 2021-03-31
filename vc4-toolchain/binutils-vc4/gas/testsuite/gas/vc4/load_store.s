	.text

	; Word load/stores, immediate offset.

	; Low registers, no offset.
	ld r0,(r1)
	st r0,(r1)
	; Low registers, 5-bit offset.
	ld r0,(r1+4)
	st r0,(r1+4)
	; Any reg, 5-bit offset, conditional.
	ld.gt r0,(r1+4)
	st.ge r0,(r1+4)
	; Any reg, 12-bit offset.
	ld r0,(r1+64)
	st r0,(r1+64)
	; High reg, no offset.
	ld r16,(r0)
	st r16,(r0)
	; High reg, 16-bit offset (limited base regs).
	ld r16,(r0+4096)
	st r16,(r0+4096)
	; SP, 5-bit shifted offset.
	ld r4,(sp+124)
	st r4,(sp+124)
	; Out of range of SP special insn.
	ld r4,(sp+256)
	st r4,(sp+256)
	; 27-bit offset.
	ld r4,(r5+0x800000)
	st r4,(r5+0x800000)

	; Halfword loads/stores, immediate offset.
	ldh r0,(r1)
	sth r0,(r1)
	; No short offset form: this is a 32-bit insn.
	ldh r0,(r1+8)
	sth r0,(r1+8)
	; 16-bit offset.
	ldh r0,(gp+4000)
	sth r0,(gp+4000)
	; 27-bit offset.
	ldh r0,(gp+100000)
	sth r0,(gp+100000)

	; Byte load/stores, immediate offset.

	; Low regs, no offset.
	ldb r0,(r1)
	stb r0,(r1)
	; 12-bit offset (no short form).
	ldb r0,(r1+55)
	stb r0,(r1+55)
	; High regs, no offset.
	ldb r18,(r20)
	stb r18,(r20)
	; 16-bit offset.
	ldb r3,(sp+30000)
	stb r3,(sp+30000)
	; Conditional, 5-bit offset.
	ldb.eq r2,(r5+12)
	stb.ne r3,(r6+31)
	; 27-bit offset.
	ldb r16,(r16+213213)
	stb r15,(r14+213213)

	; Signed byte/halfword loads, immediate offset.

	; No offset, low regs.
	ldsb r0,(r1)
	ldsh r0,(r1)
	; 12-bit offset.
	ldsb r0,(r1+5)
	ldsh r0,(r1+6)
	; No offset, high regs.
	ldsb r16,(r18)
	ldsh r17,(r19)
	; 16-bit offset, limited base regs.
	ldsb r18,(r0+30000+16)
	ldsh r18,(r0+30000+16)
	; Conditional, 5-bit offset.
	ldsb.lo r18,(r12+26)
	ldsh.hs r18,(r12+26)
	; 27-bit offset.
	ldsb r12,(r13+200000)
	ldsh r12,(r13+200000)

	; Pre/post inc/dec.
	ld r0,(r1++)
	ldb r0,(r1++)
	ldh r0,(--r1)
	ldsb r0,(--r1)
	st r0,(r1++)
	sth r0,(r1++)
	stb r0,(--r1)
	ldsh r0,(--r1)

	; Same, conditional.
	ld.gt r0,(r1++)
	ldb.ge r0,(r1++)
	ldh.eq r0,(--r1)
	ldsb.ne r0,(--r1)
	st.lt r0,(r1++)
	sth.le r0,(r1++)
	stb.hi r0,(--r1)
	ldsh.ls r0,(--r1)

	; Register indexed.
	ld r0,(r1+r2<<2)
	ldh r1,(r2+r3 shl 1)
	ldb r2,(r3+r4)
	ldsh r2,(r3+r4*2)
	st r0,(r1+r2*4)
	sth r1,(r2+r3<<1)
	stb r2,(r3+r4)
	ldsb r3,(r4+r5)

	; Same, conditional.
	ld.eq r0,(r1+r2<<2)
	ldh.ne r1,(r2+r3 shl 1)
	ldb.gt r2,(r3+r4)
	ldsh.ge r2,(r3+r4*2)
	st.lt r0,(r1+r2*4)
	sth.le r1,(r2+r3<<1)
	stb.hs r2,(r3+r4)
	ldsb.lo r3,(r4+r5)
