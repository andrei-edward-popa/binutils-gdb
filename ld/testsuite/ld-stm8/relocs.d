#name: STM8 final link relocations
#source: relocs.s
#ld: --defsym ext8=0x12 --defsym ext=0x812345 --defsym ext16=0x1234 --defsym ext24=0x812345 --defsym extbranch=0x8012 --defsym ext32=0x12345678
#objdump: -s -j .text

.*: +file format elf32-stm8

Contents of section \.text:
 8000 a612a645 a623a681 cc12348d 81234520 .*
 8010 01123456 78 .*
#pass
