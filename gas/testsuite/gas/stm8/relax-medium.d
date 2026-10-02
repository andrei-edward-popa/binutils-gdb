#as: --model=medium
#source: relax.s
#objdump: -dr
#name: STM8 medium branch relaxation

.*:[ \t]+file format elf32-stm8

Disassembly of section \.text:


00000000[ \t]+<start>:
[ \t]+0:[ \t]+20[ \t]+7f[ \t]+jra[ \t]+\$\+127.*
[ \t]+2:[ \t]+cc[ \t]+00[ \t]+00[ \t]+jp[ \t]+\$0x0000.*
[ \t]+3:[ \t]+R_STM8_16[ \t]+\.text\+0x84
[ \t]+5:[ \t]+27[ \t]+80[ \t]+jreq[ \t]+\$\-128.*
[ \t]+7:[ \t]+26[ \t]+03[ \t]+jrne[ \t]+\$\+3.*
[ \t]+9:[ \t]+cc[ \t]+00[ \t]+00[ \t]+jp[ \t]+\$0x0000.*
[ \t]+a:[ \t]+R_STM8_16[ \t]+\.text\-0x78
[ \t]+c:[ \t]+90[ \t]+29[ \t]+7f[ \t]+jrh[ \t]+\$\+127.*
[ \t]+f:[ \t]+90[ \t]+28[ \t]+03[ \t]+jrnh[ \t]+\$\+3.*
[ \t]+12:[ \t]+cc[ \t]+00[ \t]+00[ \t]+jp[ \t]+\$0x0000.*
[ \t]+13:[ \t]+R_STM8_16[ \t]+\.text\+0x92
[ \t]+15:[ \t]+cc[ \t]+00[ \t]+00[ \t]+jp[ \t]+\$0x0000.*
[ \t]+16:[ \t]+R_STM8_16[ \t]+external
[ \t]+18:[ \t]+26[ \t]+03[ \t]+jrne[ \t]+\$\+3.*
[ \t]+1a:[ \t]+cc[ \t]+00[ \t]+00[ \t]+jp[ \t]+\$0x0000.*
[ \t]+1b:[ \t]+R_STM8_16[ \t]+weak_target
[ \t]+1d:[ \t]+26[ \t]+03[ \t]+jrne[ \t]+\$\+3.*
[ \t]+1f:[ \t]+cc[ \t]+00[ \t]+00[ \t]+jp[ \t]+\$0x0000.*
[ \t]+20:[ \t]+R_STM8_16[ \t]+\.other
[ \t]+22:[ \t]+ad[ \t]+00[ \t]+callr[ \t]+\$\+0.*
[ \t]+23:[ \t]+R_STM8_8_PCREL[ \t]+external\-0x1

00000024[ \t]+<weak_target>:
[ \t]+24:[ \t]+9d[ \t]+nop[ \t]*

Disassembly[ \t]+of[ \t]+section[ \t]+\.other:

00000000[ \t]+<other_target>:
[ \t]+0:[ \t]+9d[ \t]+nop[ \t]*
