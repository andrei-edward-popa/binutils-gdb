	.text
	.globl	_start
_start:
	ld	a,#ext8
	ld	a,#lo8(ext)
	ld	a,#hi8(ext)
	ld	a,#hh8(ext)
	jp	ext16
	callf	ext24
	jra	extbranch
	.long	ext32
