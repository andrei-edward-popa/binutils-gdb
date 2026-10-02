/* STM8 ELF support for BFD.
   Copyright (C) 1999-2026 Free Software Foundation, Inc.

   Written by Åke Rehnman 2017-02-21,
   ake.rehnman (at) gmail dot com

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
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifndef _ELF_STM8_H
#define _ELF_STM8_H

#include "elf/reloc-macros.h"

/* Relocation types.  */
START_RELOC_NUMBERS (elf_stm8_reloc_type)
  RELOC_NUMBER (R_STM8_NONE,    0)
  RELOC_NUMBER (R_STM8_8,       1)
  RELOC_NUMBER (R_STM8_16,      2)
  RELOC_NUMBER (R_STM8_24,      3)
  RELOC_NUMBER (R_STM8_32,      4)
  RELOC_NUMBER (R_STM8_8_PCREL, 5)
  RELOC_NUMBER (R_STM8_HI8,     6)
  RELOC_NUMBER (R_STM8_LO8,     7)
  RELOC_NUMBER (R_STM8_HH8,     8)
END_RELOC_NUMBERS (R_STM8_max)

#endif /* _ELF_STM8_H */
