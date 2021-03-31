	.text
case3:
	nop
	rts
case4:
	nop
	rts
top:
	nop
	nop
	switch r4
$L0:
	.case case1 - $L0
	.case case2 - $L0
	.case case0 - $L0
	.case case3 - $L0
	.case case4 - $L0
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
