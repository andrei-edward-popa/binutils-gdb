#name: STM8 final link consumes relocations
#source: relocs.s
#ld: --defsym ext8=0x12 --defsym ext=0x812345 --defsym ext16=0x1234 --defsym ext24=0x812345 --defsym extbranch=0x8012 --defsym ext32=0x12345678
#readelf: -r

There are no relocations in this file\.
#pass
