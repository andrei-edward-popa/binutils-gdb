#source: pr11304a.s
#source: pr11304b.s
#ld: -e 0 --section-start .zzz=0x800000
#readelf: -S --wide
# Address 0x800000 overlaps with .heap section on tic6x-*-elf.
#notarget: tic6x-*-elf
# STM8 uses explicit program headers in its default linker script, so an
# allocated orphan section created with --section-start isn't assigned to
# a load segment automatically.
#xfail: stm8-*-*

#failif
#...
  \[[ 0-9]+\] \.zzz[ \t]+PROGBITS[ \t0-9a-f]+AX?.*
#...
  \[[ 0-9]+\] \.zzz[ \t]+PROGBITS[ \t0-9a-f]+AX?.*
#...
