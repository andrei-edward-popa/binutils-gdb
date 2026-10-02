# Copyright (c) 2026 Andrei-Edward Popa
# SPDX-License-Identifier: GPL-3.0-or-later

	.text
	.global start
start:
	jra .+129
	jra .+130
	jreq .-126
	jreq .-127
	jrh .+130
	jrh .+131
	jra external
	jreq weak_target
	jreq other_target
	callr external
	.weak weak_target
weak_target:
	nop
	.section .other,"ax"
other_target:
	nop
