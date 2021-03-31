	.text
	addscale r1,r2<<1
	addscale r1,r2<<2
	addscale r1,r2 lsl #1
	addscale r1,r2 lsl 1
	addscale r1,r2 shl #1
	addscale r1,r2 shl 1
	addscale r1,r2 * 2

	addscale r0,r1,r8 lsl 3
	addscale r0,r1,r8*8
	addscale r0,r1,r8 shl #3

	addscale r0,r1,r8 lsl #5
	addscale r0,r1,r8*32
	addscale r0,r1,r8<<5

	addscale r0,r1,r8 lsl #8
	addscale r0,r1,r8*256
	addscale r0,r1,r8<<8

	subscale r0,r8,r12<<1
	subscale r0,r8,r12 lsl 2
	subscale r0,r8,r12 shl 3
	subscale r0,r8,r12 * 16
	subscale r0,r8,r12<<5
	subscale r0,r8,r12 lsl 6
	subscale r0,r8,r12 shl 7
	subscale r0,r8,r12 * 256
