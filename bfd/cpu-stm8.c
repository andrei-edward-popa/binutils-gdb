/* BFD support for the STM8 processor.
   Copyright (C) 2007-2026 Free Software Foundation, Inc.

   Written by Åke Rehnman (at) gmail dot com

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

const bfd_arch_info_type bfd_stm8_arch =
{
  8,				/* Bits in a word.  */
  32,				/* Bits in an address.  */
  8,				/* Bits in a byte.  */
  bfd_arch_stm8,		/* Architecture number.  */
  bfd_mach_stm8,		/* Machine number.  */
  "stm8",			/* Architecture name.  */
  "stm8",			/* Printable name.  */
  4,				/* Section alignment power.  */
  true,				/* Default machine.  */
  bfd_default_compatible,	/* Architecture comparison fn.  */
  bfd_default_scan,		/* String to architecture convert fn.  */
  bfd_arch_default_fill,	/* Default fill.  */
  NULL,				/* Next -- there are none.  */
  0				/* Maximum offset of a reloc from the start of an insn.  */
};
