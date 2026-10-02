# Copyright (C) 2026 Free Software Foundation, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.

cat <<EOF2
/* Copyright (C) 2026 Free Software Foundation, Inc.

   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */

OUTPUT_FORMAT("${OUTPUT_FORMAT}")
OUTPUT_ARCH(${ARCH})
EOF2

test -n "${RELOCATING}" && cat <<EOF2
ENTRY(${ENTRY})

${LIB_SEARCH_DIRS}

/* These regions provide generic STM8 defaults.  Device-specific scripts
   should provide the actual RAM and flash sizes.  */
MEMORY
{
  data (rw) : ORIGIN = ${RAM_START}, LENGTH = ${RAM_SIZE}
  text (rx) : ORIGIN = ${ROM_START}, LENGTH = ${ROM_SIZE}
}

PHDRS
{
  text PT_LOAD FLAGS(5);
  data PT_LOAD FLAGS(6);
}
EOF2

cat <<EOF2
SECTIONS
{
  .vectors ${RELOCATING-0} :
  {
    ${RELOCATING+PROVIDE (__vectors_start = .);}
    KEEP (*(.vectors))
    KEEP (*(.vectors.*))
    ${RELOCATING+PROVIDE (__vectors_end = .);}
  } ${RELOCATING+> text :text}

  .text ${RELOCATING-0} :
  {
    ${RELOCATING+PROVIDE (__text_start = .);}
    KEEP (*(SORT_NONE(.init)))
    *(.text)
    ${RELOCATING+*(.text.*)}
    ${RELOCATING+*(.gnu.linkonce.t.*)}
    KEEP (*(SORT_NONE(.fini)))
    ${RELOCATING+PROVIDE (__text_end = .);}
  } ${RELOCATING+> text :text}

  .rodata ${RELOCATING-0} :
  {
    *(.rodata)
    ${RELOCATING+*(.rodata.*)}
    ${RELOCATING+*(.gnu.linkonce.r.*)}
    ${RELOCATING+
    PROVIDE (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE (__preinit_array_end = .);

    PROVIDE (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    PROVIDE (__init_array_end = .);

    PROVIDE (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array))
    PROVIDE (__fini_array_end = .);

    KEEP (*crtbegin*.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend*.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))

    KEEP (*crtbegin*.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend*.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))

    KEEP (*(.eh_frame))
    *(.gcc_except_table .gcc_except_table.*)}
  } ${RELOCATING+> text :text}

  .data ${RELOCATING-0} :
  {
    ${RELOCATING+PROVIDE (__data_start = .);}
    *(.data)
    ${RELOCATING+*(.data.*)}
    ${RELOCATING+*(.gnu.linkonce.d.*)}
    ${RELOCATING+PROVIDE (__data_end = .);}
  } ${RELOCATING+> data AT> text :data}

  ${RELOCATING+PROVIDE (__data_load_start = LOADADDR(.data));}
  ${RELOCATING+PROVIDE (__data_load_end = LOADADDR(.data) + SIZEOF(.data));}

  .bss ${RELOCATING-0} ${RELOCATING+(NOLOAD)} :
  {
    ${RELOCATING+PROVIDE (__bss_start = .);}
    *(.bss)
    ${RELOCATING+*(.bss.*)}
    ${RELOCATING+*(.gnu.linkonce.b.*)}
    ${RELOCATING+*(COMMON)}
    ${RELOCATING+PROVIDE (__bss_end = .);}
  } ${RELOCATING+> data :NONE}

  .noinit ${RELOCATING-0} ${RELOCATING+(NOLOAD)} :
  {
    ${RELOCATING+PROVIDE (__noinit_start = .);}
    *(.noinit)
    ${RELOCATING+*(.noinit.*)}
    ${RELOCATING+PROVIDE (__noinit_end = .);}
  } ${RELOCATING+> data :NONE}

  ${RELOCATING+_end = .;}
  ${RELOCATING+PROVIDE (end = .);}
EOF2

source_sh $srcdir/scripttempl/misc-sections.sc
source_sh $srcdir/scripttempl/DWARF.sc

cat <<EOF2
  ${OTHER_SECTIONS}
}
EOF2
