	.text
top:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	switch r4
$c:
	.case case4 - $c
	.case case3 - $c
	.case case2 - $c
	.case case1 - $c
	.case case0 - $c
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
