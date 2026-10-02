/* Disassemble STM8 instructions.
   Copyright (C) 1999-2026 Free Software Foundation, Inc.

   Contributed by Ake Rehnman.
   Adapted by Sophie 'Tyalie' Friedrich.

   This file is part of libopcodes.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"

#include "dis-asm.h"

#include "opcode/stm8.h"

/* Special opcode prefixes that extend the following instruction.  */
#define PDY 0x90
#define PIX 0x92
#define PIY 0x91
#define PWSP 0x72

typedef struct
{
  char comment[40];
  bool is_symbol;
  bfd_vma symbol_addr;
} disas_op_element_t;

static int stm8_dis (bfd_vma addr, disassemble_info *info,
		     unsigned int op, int *cmd_len);
static int stm8_operands (disas_op_element_t *element,
			  bfd_vma addr_w_offset, disassemble_info *info,
			  uint32_t *print_offset, unsigned char buf[],
			  stm8_addr_mode_t arg);

static int
fetch_data (unsigned char *buf, bfd_vma addr, disassemble_info *info, int n)
{
  int status;

  status = info->read_memory_func (addr, buf, n, info);

  if (status == 0)
    return 0;

  info->memory_error_func (status, addr, info);

  return -1;
}

static int
stm8_operands (disas_op_element_t *element, bfd_vma addr_w_offset,
	       disassemble_info *info, uint32_t *print_offset,
	       unsigned char buf[], stm8_addr_mode_t arg)
{
  int64_t val;
  fprintf_styled_ftype print = info->fprintf_styled_func;

  switch (arg)
    {
    case ST8_REG_A:
      print (info->stream, dis_style_register, "A");
      *print_offset += 1;
      break;
    case ST8_REG_X:
      print (info->stream, dis_style_register, "X");
      *print_offset += 1;
      break;
    case ST8_REG_Y:
      print (info->stream, dis_style_register, "Y");
      *print_offset += 1;
      break;
    case ST8_REG_SP:
      print (info->stream, dis_style_register, "SP");
      *print_offset += 2;
      break;
    case ST8_REG_CC:
      print (info->stream, dis_style_register, "CC");
      *print_offset += 2;
      break;
    case ST8_REG_XL:
      print (info->stream, dis_style_register, "XL");
      *print_offset += 2;
      break;
    case ST8_REG_XH:
      print (info->stream, dis_style_register, "XH");
      *print_offset += 2;
      break;
    case ST8_REG_YL:
      print (info->stream, dis_style_register, "YL");
      *print_offset += 2;
      break;
    case ST8_REG_YH:
      print (info->stream, dis_style_register, "YH");
      *print_offset += 2;
      break;
    case ST8_BIT_0:
    case ST8_BIT_1:
    case ST8_BIT_2:
    case ST8_BIT_3:
    case ST8_BIT_4:
    case ST8_BIT_5:
    case ST8_BIT_6:
    case ST8_BIT_7:
      print (info->stream, dis_style_sub_mnemonic, "#%d", arg - ST8_BIT_0);
      *print_offset += 2;
      break;
    case ST8_BYTE:
      val = buf[0];
      print (info->stream, dis_style_immediate, "#0x%2.2x", buf[0]);
      *print_offset += 5;
      return 1;
    case ST8_WORD:
      val = (buf[0] << 8) + buf[1];
      print (info->stream, dis_style_immediate, "#0x%4.4x", (uint32_t) val);
      *print_offset += 7;
      return 2;
    case ST8_PCREL:
      print (info->stream, dis_style_address_offset, "$%-+4d", (int8_t) buf[0]);
      *print_offset += 5;

      element->is_symbol = true;
      element->symbol_addr = (addr_w_offset - 1) + (int8_t) buf[0];
      return 1;
    case ST8_SHORTMEM:
      print (info->stream, dis_style_address, "$0x%2.2x", buf[0]);
      *print_offset += 5;
      return 1;
    case ST8_INDX:
      print (info->stream, dis_style_text, "(");
      print (info->stream, dis_style_register, "X");
      print (info->stream, dis_style_text, ")");
      *print_offset += 3;
      break;
    case ST8_SHORTOFF_X:
      print (info->stream, dis_style_text, "(");
      print (info->stream, dis_style_address, "$0x%2.2x", buf[0]);
      print (info->stream, dis_style_text, ",");
      print (info->stream, dis_style_register, "X");
      print (info->stream, dis_style_text, ")");
      *print_offset += 10;
      return 1;
    case ST8_INDY:
      print (info->stream, dis_style_text, "(");
      print (info->stream, dis_style_register, "Y");
      print (info->stream, dis_style_text, ")");
      *print_offset += 3;
      break;
    case ST8_SHORTOFF_Y:
      print (info->stream, dis_style_text, "(");
      print (info->stream, dis_style_address, "$0x%2.2x", buf[0]);
      print (info->stream, dis_style_text, ",");
      print (info->stream, dis_style_register, "Y");
      print (info->stream, dis_style_text, ")");
      *print_offset += 10;
      return 1;
    case ST8_SHORTOFF_SP:
      print (info->stream, dis_style_text, "(");
      print (info->stream, dis_style_address, "$0x%2.2x", buf[0]);
      print (info->stream, dis_style_text, ",");
      print (info->stream, dis_style_register, "SP");
      print (info->stream, dis_style_text, ")");
      *print_offset += 10;
      return 1;
    case ST8_SHORTPTRW:
      print (info->stream, dis_style_text, "[");
      print (info->stream, dis_style_address, "$0x%2.2x.w", buf[0]);
      print (info->stream, dis_style_text, "]");
      *print_offset += 10;
      return 1;
    case ST8_SHORTPTRW_X:
      print (info->stream, dis_style_text, "([");
      print (info->stream, dis_style_address, "$0x%2.2x", buf[0]);
      print (info->stream, dis_style_text, "],");
      print (info->stream, dis_style_register, "X");
      print (info->stream, dis_style_text, ")");
      *print_offset += 12;
      return 1;
    case ST8_SHORTPTRW_Y:
      print (info->stream, dis_style_text, "[");
      print (info->stream, dis_style_address, "$0x%2.2x", buf[0]);
      print (info->stream, dis_style_text, ",");
      print (info->stream, dis_style_register, "Y");
      print (info->stream, dis_style_text, "]");
      *print_offset += 12;
      return 1;
    case ST8_LONGMEM:
      val = (buf[0] << 8) + buf[1];
      print (info->stream, dis_style_address, "$0x%4.4x", (uint32_t) val);
      *print_offset += 7;

      element->is_symbol = true;
      element->symbol_addr = val;
      return 2;
    case ST8_LONGOFF_X:
      val = (buf[0] << 8) + buf[1];
      print (info->stream, dis_style_text, "(");
      print (info->stream, dis_style_address, "$0x%4.4x", (uint32_t) val);
      print (info->stream, dis_style_text, ",");
      print (info->stream, dis_style_register, "X");
      print (info->stream, dis_style_text, ")");
      *print_offset += 13;
      return 2;
    case ST8_LONGOFF_Y:
      val = (buf[0] << 8) + buf[1];
      print (info->stream, dis_style_text, "(");
      print (info->stream, dis_style_address, "$0x%4.4x", (uint32_t) val);
      print (info->stream, dis_style_text, ",");
      print (info->stream, dis_style_register, "Y");
      print (info->stream, dis_style_text, ")");
      *print_offset += 13;
      return 2;
    case ST8_LONGPTRW:
      val = (buf[0] << 8) + buf[1];
      print (info->stream, dis_style_text, "[");
      print (info->stream, dis_style_address, "$0x%4.4x.w", (uint32_t) val);
      print (info->stream, dis_style_text, "]");
      *print_offset += 11;
      return 2;
    case ST8_LONGPTRW_X:
      val = (buf[0] << 8) + buf[1];
      print (info->stream, dis_style_text, "([");
      print (info->stream, dis_style_address, "$0x%4.4x.w", (uint32_t) val);
      print (info->stream, dis_style_text, "],");
      print (info->stream, dis_style_register, "X");
      print (info->stream, dis_style_text, ")");
      *print_offset += 15;
      return 2;
    case ST8_LONGPTRW_Y:
      val = (buf[0] << 8) + buf[1];
      print (info->stream, dis_style_text, "([");
      print (info->stream, dis_style_address, "$0x%4.4x.w", (uint32_t) val);
      print (info->stream, dis_style_text, "],");
      print (info->stream, dis_style_register, "Y");
      print (info->stream, dis_style_text, ")");
      *print_offset += 15;
      return 2;
    case ST8_LONGPTRE:
      val = (buf[0] << 8) + buf[1];
      print (info->stream, dis_style_text, "[");
      print (info->stream, dis_style_address, "$0x%4.4x.e", (uint32_t) val);
      print (info->stream, dis_style_text, "]");
      *print_offset += 11;
      return 2;
    case ST8_LONGPTRE_X:
      val = (buf[0] << 8) + buf[1];
      print (info->stream, dis_style_text, "([");
      print (info->stream, dis_style_address, "$0x%4.4x.e", (uint32_t) val);
      print (info->stream, dis_style_text, "],");
      print (info->stream, dis_style_register, "X");
      print (info->stream, dis_style_text, ")");
      *print_offset += 15;
      return 2;
    case ST8_LONGPTRE_Y:
      val = (buf[0] << 8) + buf[1];
      print (info->stream, dis_style_text, "([");
      print (info->stream, dis_style_address, "$0x%4.4x.e", (uint32_t) val);
      print (info->stream, dis_style_text, "],");
      print (info->stream, dis_style_register, "Y");
      print (info->stream, dis_style_text, ")");
      *print_offset += 15;
      return 2;
    case ST8_EXTMEM:
      val = (buf[0] << 16) + (buf[1] << 8) + buf[2];
      print (info->stream, dis_style_address, "$0x%6.6x", (uint32_t) val);
      *print_offset += 9;

      element->is_symbol = true;
      element->symbol_addr = val;
      return 3;
    case ST8_EXTOFF_X:
      val = (buf[0] << 16) + (buf[1] << 8) + buf[2];
      print (info->stream, dis_style_text, "(");
      print (info->stream, dis_style_address, "$0x%6.6x", (uint32_t) val);
      print (info->stream, dis_style_text, ",");
      print (info->stream, dis_style_register, "X");
      print (info->stream, dis_style_text, ")");
      *print_offset += 13;
      return 3;
    case ST8_EXTOFF_Y:
      val = (buf[0] << 16) + (buf[1] << 8) + buf[2];
      print (info->stream, dis_style_text, "(");
      print (info->stream, dis_style_address, "$0x%6.6x", (uint32_t) val);
      print (info->stream, dis_style_text, ",");
      print (info->stream, dis_style_register, "Y");
      print (info->stream, dis_style_text, ")");
      *print_offset += 13;
      return 3;
    case ST8_END:
      break;
    }
  return 0;
}

static int
stm8_dis (bfd_vma addr, disassemble_info *info, unsigned int op, int *cmd_len)
{
  const struct stm8_opcodes_s *opcode;
  bfd_byte buffer[sizeof (opcode->constraints)
		  / sizeof (opcode->constraints[0]) * 3];

  disas_op_element_t
      elements[sizeof (opcode->constraints) / sizeof (opcode->constraints[0])]
      = { 0 };

  int operand_len;
  uint32_t offset = 0;

  /* Find the opcode descriptor for this binary representation.  */
  for (opcode = stm8_opcodes; opcode->name; opcode++)
    {
      if (opcode->bin_opcode == op)
	break;
    }

  if (opcode->name)
    {

      /* Decode and print the operands.  */
      operand_len = stm8_compute_insn_size (opcode) - stm8_opcode_size (op);
      *cmd_len += operand_len;

      if (operand_len != 0 && fetch_data (buffer, addr, info, operand_len))
	return -1;

      bool reverse = false;

      if ((op == 0x35) || (op == 0x45) || (op == 0x55))
	reverse = true;

      info->fprintf_styled_func (info->stream, dis_style_mnemonic, "%s\t",
				 opcode->name);

      bfd_byte *m_buf = buffer;

      for (int curr_operand = 0;
	   curr_operand < stm8_num_opcode_operands (opcode); curr_operand++)
	{
	  disas_op_element_t *element
	      = &elements[reverse ? (stm8_num_opcode_operands (opcode)
				     - curr_operand - 1)
				  : curr_operand];

	  if (curr_operand > 0)
	    {
	      info->fprintf_styled_func (info->stream, dis_style_text, ",");
	      offset += 1;
	    }

	  m_buf += stm8_operands (element, addr + *cmd_len, info, &offset,
				  m_buf, opcode->constraints[curr_operand]);
	}
    }
  else
    {
      info->fprintf_styled_func (info->stream, dis_style_assembler_directive,
				 ".word\t");
      info->fprintf_styled_func (info->stream, dis_style_text, "0x%04x", op);
      sprintf (elements[0].comment, "????");
      offset += 6;
    }

  info->fprintf_func (info->stream, "%.*s", (40 - offset - 1) / 8,
		      "\t\t\t\t\t\t\t\t");
  bool is_first = true;

  for (disas_op_element_t *element = elements;
       element != &elements[sizeof (elements) / sizeof (elements[0])];
       element++)
    {
      if ((element->comment[0] || element->is_symbol) && is_first)
	info->fprintf_styled_func (info->stream, dis_style_comment_start, ";");

      if (element->comment[0])
	info->fprintf_func (info->stream, " %s", element->comment);

      if (element->is_symbol)
	{
	  info->fprintf_func (info->stream, " ");
	  info->print_address_func (element->symbol_addr, info);
	}
    }

  return 0;
}

int
print_insn_stm8 (bfd_vma addr, disassemble_info *info)
{
  uint16_t op;
  bfd_byte buffer[2];

  int cmd_len = 0;

  if (fetch_data (buffer, addr, info, 1) != 0)
    return -1;

  op = buffer[0];
  cmd_len++;

  /* Handle STM8 opcode prefixes.  */
  if ((buffer[0] == PDY) || (buffer[0] == PIX) || (buffer[0] == PIY)
      || (buffer[0] == PWSP))
    {
      if (fetch_data (buffer, addr, info, 2))
	return -1;
      cmd_len++;
      op = bfd_getb16 (buffer);
    }

  if (stm8_dis (addr + cmd_len, info, op, &cmd_len) < 0)
    return -1;

  return cmd_len;
}
