/* tc-stm8.h -- Assembler definitions for the STM8.
   Copyright (C) 1999-2026 Free Software Foundation, Inc.

   Written by Ake Rehnman 2017-02-21,
   ake.rehnman (at) gmail dot com

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#define TC_STM8

#define TARGET_FORMAT "elf32-stm8"
#define TARGET_ARCH bfd_arch_stm8
#define TARGET_MACH bfd_mach_stm8
#define TARGET_BYTES_BIG_ENDIAN 1

#define ONLY_STANDARD_ESCAPES
#define WORKING_DOT_WORD
#define DIFF_EXPR_OK
#define NUMBERS_WITH_SUFFIX 1

#define TC_PARSE_CONS_EXPRESSION(EXP, NBYTES) \
  stm8_parse_cons_expression ((EXP), (NBYTES))

extern bfd_reloc_code_real_type
stm8_parse_cons_expression (expressionS *, int);

#define MD_PCREL_FROM_SECTION(FIX, SEC) md_pcrel_from_section (FIX, SEC)
extern long md_pcrel_from_section (struct fix *, segT);

#define md_register_arithmetic 0
