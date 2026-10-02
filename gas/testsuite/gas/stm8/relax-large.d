#as: --model=large
#source: relax.s
#objdump: -dr
#name: STM8 large branch relaxation

.*:[ \t]+file format elf32-stm8

Disassembly of section \.text:


00000000[ \t]+<start>:
[ \t]+0:[ \t]+20[ \t]+7f[ \t]+jra[ \t]+\$\+127.*
[ \t]+2:[ \t]+ac[ \t]+00[ \t]+00[ \t]+00[ \t]+jpf[ \t]+\$0x000000.*
[ \t]+3:[ \t]+R_STM8_24[ \t]+\.text\+0x84
[ \t]+6:[ \t]+27[ \t]+80[ \t]+jreq[ \t]+\$\-128.*
[ \t]+8:[ \t]+26[ \t]+04[ \t]+jrne[ \t]+\$\+4.*
[ \t]+a:[ \t]+ac[ \t]+00[ \t]+00[ \t]+00[ \t]+jpf[ \t]+\$0x000000.*
[ \t]+b:[ \t]+R_STM8_24[ \t]+\.text\-0x77
[ \t]+e:[ \t]+90[ \t]+29[ \t]+7f[ \t]+jrh[ \t]+\$\+127.*
[ \t]+11:[ \t]+90[ \t]+28[ \t]+04[ \t]+jrnh[ \t]+\$\+4.*
[ \t]+14:[ \t]+ac[ \t]+00[ \t]+00[ \t]+00[ \t]+jpf[ \t]+\$0x000000.*
[ \t]+15:[ \t]+R_STM8_24[ \t]+\.text\+0x94
[ \t]+18:[ \t]+ac[ \t]+00[ \t]+00[ \t]+00[ \t]+jpf[ \t]+\$0x000000.*
[ \t]+19:[ \t]+R_STM8_24[ \t]+external
[ \t]+1c:[ \t]+26[ \t]+04[ \t]+jrne[ \t]+\$\+4.*
[ \t]+1e:[ \t]+ac[ \t]+00[ \t]+00[ \t]+00[ \t]+jpf[ \t]+\$0x000000.*
[ \t]+1f:[ \t]+R_STM8_24[ \t]+weak_target
[ \t]+22:[ \t]+26[ \t]+04[ \t]+jrne[ \t]+\$\+4.*
[ \t]+24:[ \t]+ac[ \t]+00[ \t]+00[ \t]+00[ \t]+jpf[ \t]+\$0x000000.*
[ \t]+25:[ \t]+R_STM8_24[ \t]+\.other
[ \t]+28:[ \t]+ad[ \t]+00[ \t]+callr[ \t]+\$\+0.*
[ \t]+29:[ \t]+R_STM8_8_PCREL[ \t]+external\-0x1

0000002a[ \t]+<weak_target>:
[ \t]+2a:[ \t]+9d[ \t]+nop[ \t]*

Disassembly[ \t]+of[ \t]+section[ \t]+\.other:

00000000[ \t]+<other_target>:
[ \t]+0:[ \t]+9d[ \t]+nop[ \t]*
