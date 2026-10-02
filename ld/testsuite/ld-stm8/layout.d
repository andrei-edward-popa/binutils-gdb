#name: STM8 default linker script layout
#source: layout.s
#ld:
#objdump: -h

.*: +file format elf32-stm8

Sections:
Idx Name +Size +VMA +LMA +File off +Algn
 +0 \.vectors +00000004 +00008000 +00008000 +.* +2\*\*0
 +CONTENTS, ALLOC, LOAD, READONLY, CODE
 +1 \.text +00000002 +00008004 +00008004 +.* +2\*\*0
 +CONTENTS, ALLOC, LOAD, READONLY, CODE
 +2 \.rodata +00000002 +00008006 +00008006 +.* +2\*\*0
 +CONTENTS, ALLOC, LOAD, READONLY, DATA
 +3 \.data +00000002 +00000000 +00008008 +.* +2\*\*0
 +CONTENTS, ALLOC, LOAD, DATA
 +4 \.bss +00000003 +00000002 +00000002 +.* +2\*\*0
 +ALLOC
 +5 \.noinit +00000002 +00000005 +00000005 +.* +2\*\*0
 +ALLOC
#pass
