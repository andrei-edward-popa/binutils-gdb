#objdump: -s -j .text
#name: STM8 register names remain valid symbols

.*: +file format elf32-stm8

Contents of section \.text:
 0000 00010203 04050607 08 .*
