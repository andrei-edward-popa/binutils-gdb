	.section .vectors,"ax",@progbits
	.byte	0x82, 0x00, 0x80, 0x04

	.text
	.globl	_start
_start:
	nop
	ret

	.section .rodata,"a",@progbits
	.byte	0xaa, 0xbb

	.data
	.globl	data_object
data_object:
	.byte	0x12, 0x34

	.bss
	.globl	bss_object
bss_object:
	.space	3

	.section .noinit,"aw",@nobits
	.globl	noinit_object
noinit_object:
	.space	2
