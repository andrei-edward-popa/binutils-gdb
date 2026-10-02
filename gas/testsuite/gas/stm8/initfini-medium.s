# Copyright (c) 2026 Andrei-Edward Popa

.stm8_model "medium"
.section .preinit_array, "aw", @preinit_array
.word entry
.section .init_array, "aw", @init_array
.word entry
.section .fini_array, "aw", @fini_array
.word entry
.text
entry:
nop
