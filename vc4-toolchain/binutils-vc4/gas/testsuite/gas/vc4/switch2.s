	.text
top:
	nop
	switch r7
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

	.rept 256
	nop
	.endr

case3:
	nop
	rts
case4:
	nop
	rts
