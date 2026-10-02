# Copyright (c) 2026 Andrei-Edward Popa

.stm8_model "large"
.section .preinit_array, "aw", @preinit_array
.3byte entry
.section .init_array, "aw", @init_array
.3byte entry
.section .fini_array, "aw", @fini_array
.3byte entry
.text
entry:
nop
