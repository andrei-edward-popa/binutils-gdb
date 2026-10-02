	.text
	.globl	_start
_start:
	nop
	ret
	trap
	ld	a,#0x12
	ld	a,0x34.s
	ld	a,0x1234
	ld	a,(x)
	ld	a,(0x34.s,x)
	ld	a,(0x1234,x)
	ld	a,(y)
	ld	a,(0x34.s,y)
	ld	a,(0x1234,y)
	ld	a,(0x34.s,sp)
	ldw	x,#0x1234
	ldw	y,#0x5678
	bset	0x1234,#3
	bres	0x1234,#7
	call	0x1234
	callf	0x812345
	jp	0x4321
	jpf	0x823456
	jra	1f
	nop
1:
	nop
2:
	nop
	jra	2b
