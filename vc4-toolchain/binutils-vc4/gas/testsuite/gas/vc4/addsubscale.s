	.text

	; 16-bit variants.
	add.s r0,#31
	add.s r0,#31<<3

	add r0,#31
	add r0,#31<<3

	; 32-bit (conditional) variants.
	add.eq.m r0,r1,#17
	add.eq.m r0,r1,#17<<1
	add.eq.m r0,r1,#17<<2
	add.eq.m r0,r1,#17<<3
	add.eq.m r0,r1,#17<<4
	add.eq.m r0,r1,#17<<5
	add.eq.m r0,r1,#17<<6
	add.eq.m r0,r1,#17<<7
	add.eq.m r0,r1,#17<<8

	sub.eq.m r0,r1,#17
	sub.eq.m r0,r1,#17<<1
	sub.eq.m r0,r1,#17<<2
	sub.eq.m r0,r1,#17<<3
	sub.eq.m r0,r1,#17<<4
	sub.eq.m r0,r1,#17<<5
	sub.eq.m r0,r1,#17<<6
	sub.eq.m r0,r1,#17<<7
	sub.eq.m r0,r1,#17<<8

	add.eq r0,r1,#17
	add.eq r0,r1,#17<<1
	add.eq r0,r1,#17<<2
	add.eq r0,r1,#17<<3
	add.eq r0,r1,#17<<4
	add.eq r0,r1,#17<<5
	add.eq r0,r1,#17<<6
	add.eq r0,r1,#17<<7
	add.eq r0,r1,#17<<8

	sub.eq r0,r1,#17
	sub.eq r0,r1,#17<<1
	sub.eq r0,r1,#17<<2
	sub.eq r0,r1,#17<<3
	sub.eq r0,r1,#17<<4
	sub.eq r0,r1,#17<<5
	sub.eq r0,r1,#17<<6
	sub.eq r0,r1,#17<<7
	sub.eq r0,r1,#17<<8
