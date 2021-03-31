	.text
top:
	nop
	nop
	switch r4
.c:
;	.byte (case0 - $c)/2
;	.byte (case1 - $c)/2
;	.byte (case2 - $c)/2
;	.byte (case3 - $c)/2
;	.byte (case4 - $c)/2

	.case case1 - .c
	.case case2 - .c
	.case case3 - .c
	.case case4 - .c
	.endswitch
case0:
	nop
	rts
case1:
	nop
	rts
case2:
	nop
	rts
case3:
	nop
	rts
case4:
	nop
	rts
