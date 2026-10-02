#objdump: -dr --insn-width=8
#name: STM8 prefixed branches and pointer operands

.*: +file format elf32-stm8


Disassembly of section \.text:

0+ <back>:
[ \t]+0:[ \t]+26 fe[ \t]+jrne[ \t]+\$-2[ \t]+; 0 <back>
[ \t]+2:[ \t]+90 28 fb[ \t]+jrnh[ \t]+\$-5[ \t]+; 0 <back>
[ \t]+5:[ \t]+72 0e 12 34 f6[ \t]+btjt[ \t]+\$0x1234,#7,\$-10[ \t]+; 1234 <back\+0x1234> 0 <back>
[ \t]+a:[ \t]+72 01 12 34 f1[ \t]+btjf[ \t]+\$0x1234,#0,\$-15[ \t]+; 1234 <back\+0x1234> 0 <back>
[ \t]+f:[ \t]+91 d6 12[ \t]+ld[ \t]+A,\(\[\$0x12.s\],Y\)[ \t]*
[ \t]+12:[ \t]+35 56 12 34[ \t]+mov[ \t]+\$0x1234,#0x56[ \t]+; 1234 <back\+0x1234>
[ \t]+16:[ \t]+45 34 12[ \t]+mov[ \t]+\$0x12.s,\$0x34.s[ \t]*
[ \t]+19:[ \t]+55 56 78 12 34[ \t]+mov[ \t]+\$0x1234,\$0x5678[ \t]+; 1234 <back\+0x1234> 5678 <back\+0x5678>
