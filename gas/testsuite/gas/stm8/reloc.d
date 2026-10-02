#objdump: -r
#name: STM8 relocations

.*: +file format elf32-stm8

RELOCATION RECORDS FOR \[\.text\]:
OFFSET +TYPE +VALUE
0*1 +R_STM8_8 +ext8.*
0*3 +R_STM8_LO8 +ext.*
0*5 +R_STM8_HI8 +ext.*
0*7 +R_STM8_HH8 +ext.*
0*9 +R_STM8_16 +ext16.*
0*c +R_STM8_24 +ext24.*
0*10 +R_STM8_8_PCREL +extbranch.*
0*11 +R_STM8_32 +ext32.*
0*15 +R_STM8_LO8 +ext.*
0*16 +R_STM8_HI8 +ext.*
0*17 +R_STM8_HH8 +ext.*
