/* Opcode definitions for the STM8 processor.

   Copyright (C) 2007-2026 Free Software Foundation, Inc.

   Written by Åke Rehnman <ake.rehnman@gmail.com>.
   Adapted by Sophie Friedrich.

   This file is part of the GNU opcodes library.

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

#ifndef OPCODE_STM8_H
#define OPCODE_STM8_H

typedef enum
{
  ST8_END = 0,
  ST8_BIT_0,
  ST8_BIT_1,
  ST8_BIT_2,
  ST8_BIT_3,
  ST8_BIT_4,
  ST8_BIT_5,
  ST8_BIT_6,
  ST8_BIT_7,
  ST8_PCREL,
  ST8_REG_CC,
  ST8_REG_A,
  ST8_REG_X,
  ST8_REG_Y,
  ST8_REG_SP,
  ST8_REG_XL,
  ST8_REG_XH,
  ST8_REG_YL,
  ST8_REG_YH,
  ST8_BYTE,             /* 8-bit immediate.  */
  ST8_WORD,             /* 16-bit immediate.  */
  ST8_SHORTMEM,         /* 8-bit direct address.  */
  ST8_LONGMEM,          /* 16-bit direct address.  */
  ST8_EXTMEM,           /* 24-bit direct address.  */
  ST8_INDX,
  ST8_INDY,
  ST8_SHORTOFF_X,
  ST8_LONGOFF_X,
  ST8_EXTOFF_X,
  ST8_SHORTOFF_Y,
  ST8_LONGOFF_Y,
  ST8_EXTOFF_Y,
  ST8_SHORTOFF_SP,
  ST8_SHORTPTRW,
  ST8_LONGPTRW,
  ST8_SHORTPTRW_X,
  ST8_LONGPTRW_X,
  ST8_SHORTPTRW_Y,
  ST8_LONGPTRW_Y,
  ST8_LONGPTRE,
  ST8_LONGPTRE_X,
  ST8_LONGPTRE_Y
} stm8_addr_mode_t;

struct stm8_opcodes_s
{
  const char *name;
  stm8_addr_mode_t constraints[5];
  unsigned int bin_opcode;
};

extern const struct stm8_opcodes_s stm8_opcodes[];

extern int stm8_compute_insn_size (const struct stm8_opcodes_s *opcode);
extern int stm8_num_opcode_operands (const struct stm8_opcodes_s *opcode);
extern unsigned int stm8_opcode_size (unsigned int opcode);

#endif /* OPCODE_STM8_H */
