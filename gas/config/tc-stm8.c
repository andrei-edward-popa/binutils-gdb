/* tc-stm8.c -- Assembler for the STM8.
   Copyright (C) 2007-2026 Free Software Foundation, Inc.

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

#include "as.h"
#include "dwarf2dbg.h"
#include "safe-ctype.h"
#include "opcode/stm8.h"

typedef enum
{
  OP_ILLEGAL = 0,
  OP_IMM,
  OP_SHORTMEM,
  OP_MEM,
  OP_INDX,
  OP_INDY,
  OP_SOFF_X,
  OP_OFF_X,
  OP_SOFF_Y,
  OP_OFF_Y,
  OP_SOFF_SP,
  OP_SPTRW,
  OP_LPTRW,
  OP_LPTRE,
  OP_SPTRW_X,
  OP_LPTRW_X,
  OP_SPTRW_Y,
  OP_LPTRW_Y,
  OP_LPTRE_X,
  OP_LPTRE_Y,
  OP_REGISTER,
  OP_HI8,
  OP_LO8,
  OP_HH8
} stm8_operand_t;

typedef struct
{
  const char *name;
  stm8_operand_t op;
  bfd_reloc_code_real_type reloc;
} exp_mod_data_t;

static htab_t stm8_hash;

const char comment_chars[] = ";";
const char line_comment_chars[] = "#";
const char line_separator_chars[] = "{";


/* The target-specific pseudo-ops which we support.  */
const pseudo_typeS md_pseudo_table[] =
{
  { NULL, NULL, 0 }
};

const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant.
   As in 0f12.456
   or	 0d1.2345e12  */
const char FLT_CHARS[] = "rRsSfFdDxXpP";

/* STM8 PC-relative displacements are relative to the instruction following
   the displacement field.  */
long
md_pcrel_from (fixS *fixP)
{
  return fixP->fx_size + fixP->fx_where + fixP->fx_frag->fr_address;
}

const char *
md_atof (int type, char *litP, int *sizeP)
{
  return ieee_md_atof (type, litP, sizeP, true);
}

void
md_show_usage (FILE *stream ATTRIBUTE_UNUSED)
{
}

const char md_shortopts[] = "";

const struct option md_longopts[] =
{
  { NULL, no_argument, NULL, 0 }
};

const size_t md_longopts_size = sizeof (md_longopts);

int
md_parse_option (int c ATTRIBUTE_UNUSED, const char *arg ATTRIBUTE_UNUSED)
{
  return 0;
}

struct stm8_register
{
  const char *name;
  stm8_addr_mode_t reg;
};

static const struct stm8_register stm8_registers[] =
{
  { "A", ST8_REG_A }, { "X", ST8_REG_X }, { "Y", ST8_REG_Y },
  { "SP", ST8_REG_SP }, { "CC", ST8_REG_CC },
  { "XL", ST8_REG_XL }, { "XH", ST8_REG_XH },
  { "YL", ST8_REG_YL }, { "YH", ST8_REG_YH },
  { "a", ST8_REG_A }, { "x", ST8_REG_X }, { "y", ST8_REG_Y },
  { "sp", ST8_REG_SP }, { "cc", ST8_REG_CC },
  { "xl", ST8_REG_XL }, { "xh", ST8_REG_XH },
  { "yl", ST8_REG_YL }, { "yh", ST8_REG_YH }
};

void
md_begin (void)
{
  const struct stm8_opcodes_s *opcode;

  stm8_hash = str_htab_create ();

  /* Store the first encoding for each mnemonic.  Encodings for the same
     mnemonic are contiguous in stm8_opcodes and are examined by
     md_assemble.  */
  for (opcode = stm8_opcodes; opcode->name != NULL; opcode++)
    if (str_hash_find (stm8_hash, opcode->name) == NULL)
      str_hash_insert (stm8_hash, opcode->name, opcode, 0);
}

static const exp_mod_data_t exp_mod_data[] =
{
  { "lo8", OP_LO8, BFD_RELOC_STM8_LO8 },
  { "hi8", OP_HI8, BFD_RELOC_STM8_HI8 },
  { "hh8", OP_HH8, BFD_RELOC_STM8_HH8 }
};

static inline char *
skip_space (char *s)
{
  while (*s == ' ' || *s == '\t')
    ++s;
  return s;
}

/* Extract one word from FROM and copy it to TO.  */

static char *
extract_word (char *from, char *to, int limit)
{
  char *op_end;
  int size = 0;

  /* Drop leading whitespace.  */
  from = skip_space (from);
  *to = 0;

  /* Find the op code end.  */
  for (op_end = from; *op_end != 0 && is_part_of_name (*op_end);)
    {
      to[size++] = *op_end++;
      if (size + 1 >= limit)
	break;
    }

  to[size] = 0;
  return op_end;
}

void
md_operand (expressionS *exp ATTRIBUTE_UNUSED)
{
}

/* Attempt to simplify or eliminate a fixup. To indicate that a fixup
   has been eliminated, set fix->fx_done. If fix->fx_addsy is non-NULL,
   we will have to generate a reloc entry.  */
void
md_apply_fix (fixS *fixP, valueT *valP, segT segment ATTRIBUTE_UNUSED)
{
  valueT value = *valP;
  offsetT signed_value = (offsetT) value;
  char *buf = fixP->fx_where + fixP->fx_frag->fr_literal;

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_8:
      if (fixP->fx_addsy != NULL)
	{
	  fixP->fx_no_overflow = 1;
	  fixP->fx_done = 0;
	}
      else
	md_number_to_chars (buf, value, 1);
      break;

    case BFD_RELOC_16:
      if (fixP->fx_addsy != NULL)
	{
	  fixP->fx_no_overflow = 1;
	  fixP->fx_done = 0;
	}
      else
	md_number_to_chars (buf, value, 2);
      break;

    case BFD_RELOC_24:
      if (fixP->fx_addsy != NULL)
	{
	  fixP->fx_no_overflow = 1;
	  fixP->fx_done = 0;
	}
      else
	md_number_to_chars (buf, value, 3);
      break;

    case BFD_RELOC_32:
      if (fixP->fx_addsy != NULL)
	{
	  fixP->fx_no_overflow = 1;
	  fixP->fx_done = 0;
	}
      else
	md_number_to_chars (buf, value, 4);
      break;

    case BFD_RELOC_8_PCREL:
      if (fixP->fx_addsy != NULL)
	{
	  fixP->fx_no_overflow = 1;
	  fixP->fx_done = 0;
	}
      else
	{
	  if (signed_value > 127 || signed_value < -128)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("relative jump out of range"));
	  md_number_to_chars (buf, value, 1);
	  fixP->fx_no_overflow = 1;
	  fixP->fx_done = 1;
	}
      break;

    case BFD_RELOC_STM8_LO8:
      fixP->fx_no_overflow = 1;
      if (fixP->fx_addsy == NULL)
	md_number_to_chars (buf, value & 0xff, 1);
      break;

    case BFD_RELOC_STM8_HI8:
      fixP->fx_no_overflow = 1;
      if (fixP->fx_addsy == NULL)
	md_number_to_chars (buf, (value >> 8) & 0xff, 1);
      break;

    case BFD_RELOC_STM8_HH8:
      fixP->fx_no_overflow = 1;
      if (fixP->fx_addsy == NULL)
	md_number_to_chars (buf, (value >> 16) & 0xff, 1);
      break;

    default:
      as_fatal (_("internal error: unsupported STM8 relocation %s"),
		bfd_get_reloc_code_name (fixP->fx_r_type));
    }

  if (fixP->fx_addsy == NULL && !fixP->fx_pcrel)
    fixP->fx_done = 1;
}

/* Generate a machine-dependent relocation from a fixup.  */

arelent *
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED, fixS *fixP)
{
  arelent *reloc;

  if (fixP->fx_addsy == NULL)
    {
      as_bad_where (fixP->fx_file, fixP->fx_line,
		    _("cannot create relocation without a symbol"));
      return NULL;
    }

  reloc = notes_alloc (sizeof (*reloc));
  reloc->sym_ptr_ptr = notes_alloc (sizeof (*reloc->sym_ptr_ptr));
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixP->fx_addsy);
  reloc->address = fixP->fx_frag->fr_address + fixP->fx_where;
  reloc->addend = fixP->fx_offset;

  /* BFD defines P as the address of the relocated byte.  STM8 relative
     branches use the address following the one-byte displacement.  */
  if (fixP->fx_r_type == BFD_RELOC_8_PCREL)
    reloc->addend--;

  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixP->fx_r_type);
  if (reloc->howto == NULL)
    {
      as_bad_where (fixP->fx_file, fixP->fx_line,
		    _("cannot represent %s relocation in object file"),
		    bfd_get_reloc_code_name (fixP->fx_r_type));
      return NULL;
    }

  return reloc;
}

valueT
md_section_align (segT seg, valueT size)
{
  unsigned int align = bfd_section_alignment (seg);
  valueT mask = ((valueT)1 << align) - 1;

  return (size + mask) & ~mask;
}

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return NULL;
}


/* Put number into target byte order.  */

void
md_number_to_chars (char *ptr, valueT use, int nbytes)
{
  number_to_chars_bigendian (ptr, use, nbytes);
}

int
md_estimate_size_before_relax (fragS *fragP ATTRIBUTE_UNUSED,
			       segT segment_type ATTRIBUTE_UNUSED)
{
  as_fatal (_("unexpected STM8 relaxation request"));
  return 0;
}

void
md_convert_frag (bfd *abfd ATTRIBUTE_UNUSED, segT sec ATTRIBUTE_UNUSED,
		 fragS *fragP ATTRIBUTE_UNUSED)
{
  as_fatal (_("unexpected STM8 relaxation conversion"));
}

static char *
match_parentheses (char *str)
{
  char *p;
  int cnt = 0;

  p = str;
  while (*p != 0)
    {
      if (*p == '(')
	cnt++;
      if (*p == ')')
	{
	  if (--cnt == 0)
	    return p;
	}
      p++;
    }
  return NULL;
}

static int
split_words (char *str, char **chunks)
{
  int i;
  char *p;

  p = str;
  for (i = 0; i < 3; i++)
    {
      chunks[i] = str;
      if (*str == 0)
	break;
      while (*p != 0)
	{
	  if (*p == '(')
	    {
	      p = match_parentheses (str);
	      if (p == 0)
		return 0;
	    }
	  if (*p == ',')
	    {
	      *p = 0;
	      p++;
	      break;
	    }
	  p++;
	}
      str = p;
    }
  return i;
}

static int
read_arg_ptr (char *str, expressionS *exps)
{
  char *s;
  char *p = NULL;
  char c = 0;

  if ((str[0] == '[') && (strstr (str, "]")))
    {
      s = str;
      s++;
      input_line_pointer = s;

      /* Temporarily hide the pointer-size suffix from expression().  */
      if ((p = strstr (s, ".s]")))
	{
	  c = *p;
	  *p = 0;
	}
      else if ((p = strstr (s, ".w]")))
	{
	  c = *p;
	  *p = 0;
	}
      else if ((p = strstr (s, ".e]")))
	{
	  c = *p;
	  *p = 0;
	}

      expression (exps);

      /* Restore the suffix delimiter.	*/
      if (p)
	*p = c;

      /* Return the default pointer length.  */
      if (*input_line_pointer == ']')
	{
	  input_line_pointer += 1;
	  return 2;
	}
      if ((*input_line_pointer == '.') && (*(input_line_pointer + 1) == 's'))
	{
	  input_line_pointer += 2;
	  return 1;
	}
      else if ((*input_line_pointer == '.')
	       && (*(input_line_pointer + 1) == 'w'))
	{
	  input_line_pointer += 2;
	  return 2;
	}
      else if ((*input_line_pointer == '.')
	       && (*(input_line_pointer + 1) == 'e'))
	{
	  input_line_pointer += 2;
	  return 3;
	}
      else
	{
	  as_bad (_("expected `]`, `.s`, `.w` or `.e`, found `%c`"),
		  *input_line_pointer);
	  return -1;
	}
    }
  return 0;
}

static int
read_arg_idx (char *str, expressionS *exps)
{
  char *s;
  char *p = NULL;
  char c = 0;

  s = str;
  input_line_pointer = s;

  /* Temporarily hide the short-offset suffix from expression().  */
  if ((p = strstr (s, ".s")))
    {
      c = *p;
      *p = 0;
    }

  expression (exps);

  /* Restore the suffix delimiter.  */
  if (p)
    *p = c;

  /* Return the default offset length.	*/
  if (*input_line_pointer == ',')
    {
      input_line_pointer += 1;
      return 2;
    }
  else if ((*input_line_pointer == '.') && (*(input_line_pointer + 1) == 's'))
    {
      input_line_pointer += 2;
      return 1;
    }
  else
    {
      as_bad (_("expected `,` or `.s`, found `%c`"), *input_line_pointer);
      return -1;
    }

  return 0;
}

static char *
toupperstr (char *str)
{
  int i;
  for (i = 0; str[i]; i++)
    {
      str[i] = TOUPPER (str[i]);
    }
  return str;
}

static char *
strend (char *str, const char *suffix)
{
  char *p = strrchr (str, suffix[0]);

  if (p != NULL && strcmp (p, suffix) == 0)
    return p;

  return NULL;
}

/* Parse an STM8 expression modifier such as lo8(...), hi8(...) or hh8(...).
   Return 1 if STR was a modifier expression, 0 otherwise.  */
static int
read_exp_modifier (char *str, expressionS *exps)
{
  size_t i;

  for (i = 0; i < ARRAY_SIZE (exp_mod_data); i++)
    {
      const exp_mod_data_t *const pexp = &exp_mod_data[i];
      size_t len = strlen (pexp->name);

      if (strncasecmp (str, pexp->name, len) != 0)
	continue;

      str += len;
      while (ISSPACE (*str))
	str++;

      if (*str != '(')
	return 0;

      input_line_pointer = ++str;
      expression (exps);

      if (*input_line_pointer != ')')
	{
	  as_bad (_("`)' required"));
	  return 0;
	}

      input_line_pointer++;

      if (*input_line_pointer != '\0')
	{
	  as_bad (_("garbage in operand '%s'"), input_line_pointer);
	  return 0;
	}

      exps->X_md = pexp->op;
      return 1;
    }

  return 0;
}

/* Parse STM8 expression modifiers used by data directives such as
   .byte lo8(symbol).  */
bfd_reloc_code_real_type
stm8_parse_cons_expression (expressionS *exp, int nbytes)
{
  char *saved_input;
  size_t i;

  saved_input = input_line_pointer;

  if (nbytes == 1)
    for (i = 0; i < ARRAY_SIZE (exp_mod_data); i++)
      {
	const exp_mod_data_t *pexp = &exp_mod_data[i];
	size_t len = strlen (pexp->name);

	if (strncasecmp (input_line_pointer, pexp->name, len) != 0)
	  continue;

	input_line_pointer += len;
	while (ISSPACE (*input_line_pointer))
	  input_line_pointer++;

	if (*input_line_pointer != '(')
	  {
	    input_line_pointer = saved_input;
	    break;
	  }

	input_line_pointer++;
	while (ISSPACE (*input_line_pointer))
	  input_line_pointer++;

	expression (exp);

	if (*input_line_pointer != ')')
	  {
	    as_bad (_("`)' required"));
	    return BFD_RELOC_NONE;
	  }

	input_line_pointer++;
	return pexp->reloc;
      }

  input_line_pointer = saved_input;
  expression (exp);
  return BFD_RELOC_NONE;
}

static int
read_register (char *str, expressionS *exp)
{
  char *start;
  char *end;
  size_t len;
  size_t i;

  start = skip_space (str);
  end = start + strlen (start);

  while (end > start && ISSPACE (end[-1]))
    end--;

  len = end - start;

  for (i = 0; i < ARRAY_SIZE (stm8_registers); i++)
    {
      const char *name = stm8_registers[i].name;

      if (strlen (name) == len
	  && strncasecmp (start, name, len) == 0)
	{
	  exp->X_op = O_register;
	  exp->X_add_number = stm8_registers[i].reg;
	  exp->X_md = OP_REGISTER;
	  return 1;
	}
    }

  return 0;
}

/* In: argument
   Out: value
   Modifies: type */
static int
read_arg (char *str, expressionS *exps)
{
  char strx[256];
  char *p;
  int ret;

  /* Decode the operand syntax far enough to select an opcode entry.  */
  if (str == NULL)
    return 0;

  /* Immediate.	 */
  if (str[0] == '#')
    {
      str++;

      if (read_exp_modifier (str, exps))
	return 1;

      input_line_pointer = str;
      expression (exps);
      exps->X_md = OP_IMM;
      return 1;
    }

  strncpy (strx, str, sizeof (strx) - 1);
  strx[sizeof (strx) - 1] = '\0';
  toupperstr (strx);

  /* Decode a pointer operand.	*/
  if (str[0] == '[')
    {
      ret = read_arg_ptr (str, exps);
      if (ret > 0)
	{
	  if (ret == 1)
	    {
	      exps->X_md = OP_SPTRW;
	      return 1;
	    }
	  if (ret == 2)
	    {
	      exps->X_md = OP_LPTRW;
	      return 1;
	    }
	  if (ret == 3)
	    {
	      exps->X_md = OP_LPTRE;
	      return 1;
	    }
	}
      else
	return 0;
    }
  /* Decode indexed operands.  */
  /* Index X.  */
  else if ((str[0] == '(') && (strstr (strx, "(X)")))
    {
      exps->X_md = OP_INDX;
      return 1;
    }
  /* Index Y.  */
  else if ((str[0] == '(') && (strstr (strx, "(Y)")))
    {
      exps->X_md = OP_INDY;
      return 1;
    }
  /* Offset,X.	*/
  else if ((str[0] == '(') && (strstr (strx, ",X)")))
    {
      str++;
      if (str[0] == '[')
	{
	  ret = read_arg_ptr (str, exps);
	  if (ret == 1)
	    {
	      exps->X_md = OP_SPTRW_X;
	      return 1;
	    }
	  if (ret == 2)
	    {
	      exps->X_md = OP_LPTRW_X;
	      return 1;
	    }
	  if (ret == 3)
	    {
	      exps->X_md = OP_LPTRE_X;
	      return 1;
	    }
	}

      else
	{
	  ret = read_arg_idx (str, exps);
	  if (ret == 1)
	    {
	      exps->X_md = OP_SOFF_X;
	      return 1;
	    }
	  if (ret == 2)
	    {
	      exps->X_md = OP_OFF_X;
	      return 1;
	    }
	}
      return 0;
    }
  /* Offset,Y.	*/
  else if ((str[0] == '(') && (strstr (strx, ",Y)")))
    {
      str++;
      if (str[0] == '[')
	{
	  ret = read_arg_ptr (str, exps);
	  if (ret == 1)
	    {
	      exps->X_md = OP_SPTRW_Y;
	      return 1;
	    }
	  if (ret == 2)
	    {
	      exps->X_md = OP_LPTRW_Y;
	      return 1;
	    }
	  if (ret == 3)
	    {
	      exps->X_md = OP_LPTRE_Y;
	      return 1;
	    }
	}
      else
	{
	  ret = read_arg_idx (str, exps);
	  if (ret == 1)
	    {
	      exps->X_md = OP_SOFF_Y;
	      return 1;
	    }
	  if (ret == 2)
	    {
	      exps->X_md = OP_OFF_Y;
	      return 1;
	    }
	}
      return 0;
    }
  /* Offset,SP.	 */
  else if ((str[0] == '(') && (strstr (strx, ",SP)")))
    {
      str++;
      ret = read_arg_idx (str, exps);
      if (ret > 0)
	{
	  exps->X_md = OP_SOFF_SP;
	  return 1;
	}
      return 0;
    }
  else if (read_exp_modifier (str, exps))
    return 1;

  if ((p = strend (str, ".s")))
    {
      *p = 0;
      input_line_pointer = str;
      expression (exps);
      exps->X_md = OP_SHORTMEM;
      return 1;
    }

  if (read_register (str, exps))
    return 1;

  input_line_pointer = str;
  expression (exps);

  if (exps->X_op != O_illegal)
    {
      exps->X_md = OP_MEM;
      return 1;
    }

  /* Can't parse an expression, notifying caller about that. */
  return 0;
}

static int
read_args (char *str, expressionS exps[])
{
  char *chunks[3];
  int count = split_words (str, chunks);
  int i;

  for (i = 0; i < count; i++)
    if (!read_arg (chunks[i], &exps[i]))
      {
	as_bad (_("invalid operand: %s"), chunks[i]);
	return -1;
      }

  return count;
}

static void
stm8_bfd_out (const struct stm8_opcodes_s *op, expressionS exp[], int count,
	      char *frag)
{
  int i;
  int arg = 0;
  int dir = 1;

  /* if count is negative the arguments are reversed */
  if (count < 0)
    {
      count = -count;
      arg = count - 1;
      dir = -1;
    }

  for (i = 0; i < count; i++, arg += dir)
    {
      int where = frag - frag_now->fr_literal;

      if (exp[arg].X_op != O_illegal)
	{
	  switch (op->constraints[arg])
	    {
	    case ST8_REG_CC:
	    case ST8_REG_A:
	    case ST8_REG_X:
	    case ST8_REG_Y:
	    case ST8_REG_SP:
	    case ST8_REG_XL:
	    case ST8_REG_XH:
	    case ST8_REG_YL:
	    case ST8_REG_YH:
	    case ST8_INDX:
	    case ST8_INDY:
	      break;
	    case ST8_EXTMEM:
	    case ST8_EXTOFF_X:
	    case ST8_EXTOFF_Y:
	      fix_new_exp (frag_now, where, 3, &exp[arg], false, BFD_RELOC_24);
	      bfd_put_bits (0, frag, 24, true);
	      frag += 3;
	      break;
	    case ST8_LONGPTRW_Y:
	    case ST8_LONGPTRW_X:
	    case ST8_LONGPTRW:
	    case ST8_LONGPTRE_Y:
	    case ST8_LONGPTRE_X:
	    case ST8_LONGPTRE:
	    case ST8_LONGOFF_Y:
	    case ST8_LONGOFF_X:
	    case ST8_WORD:
	    case ST8_LONGMEM:
	      fix_new_exp (frag_now, where, 2, &exp[arg], false, BFD_RELOC_16);
	      bfd_put_bits (0, frag, 16, true);
	      frag += 2;
	      break;
	    case ST8_SHORTPTRW_Y:
	    case ST8_SHORTPTRW_X:
	    case ST8_SHORTPTRW:
	    case ST8_SHORTOFF_Y:
	    case ST8_SHORTOFF_X:
	    case ST8_SHORTOFF_SP:
	    case ST8_BYTE:
	    case ST8_SHORTMEM:
	      if (exp[arg].X_md == OP_LO8)
		fix_new_exp (frag_now, where, 1, &exp[arg], false,
			     BFD_RELOC_STM8_LO8);
	      else if (exp[arg].X_md == OP_HI8)
		fix_new_exp (frag_now, where, 1, &exp[arg], false,
			     BFD_RELOC_STM8_HI8);
	      else if (exp[arg].X_md == OP_HH8)
		fix_new_exp (frag_now, where, 1, &exp[arg], false,
			     BFD_RELOC_STM8_HH8);
	      else
		fix_new_exp (frag_now, where, 1, &exp[arg], false,
			     BFD_RELOC_8);
	      bfd_put_bits (0, frag, 8, true);
	      frag += 1;
	      break;
	    case ST8_PCREL:
	      fix_new_exp (frag_now, where, 1, &exp[arg], true,
			   BFD_RELOC_8_PCREL);
	      bfd_put_bits (0, frag, 8, true);
	      frag += 1;
	      break;
	    case ST8_BIT_0:
	    case ST8_BIT_1:
	    case ST8_BIT_2:
	    case ST8_BIT_3:
	    case ST8_BIT_4:
	    case ST8_BIT_5:
	    case ST8_BIT_6:
	    case ST8_BIT_7:
	      /* The selected opcode already contains the bit number.  */
	      break;
	    case ST8_END:
	      as_fatal (_("internal error: illegal STM8 operand constraint"));
	      break;
	    }
	}
    }
}

static int
cmpspec (const stm8_addr_mode_t addr_mode[], expressionS exps[], int count)
{
  int i, ret = 0;
  unsigned int value;
  stm8_operand_t operand;

  for (i = 0; i < count; i++)
    {
      operand = exps[i].X_md;
      if (operand == OP_ILLEGAL)
	{
	  ret++;
	  continue;
	}
      if (exps[i].X_op == O_constant)
	value = exps[i].X_add_number;
      else
	value = -1;

      switch (operand)
	{
	case OP_REGISTER:
	  if (addr_mode[i] == (stm8_addr_mode_t)exps[i].X_add_number)
	    continue;
	  break;
	case OP_IMM:
	  if (addr_mode[i] == ST8_BYTE || addr_mode[i] == ST8_WORD)
	    continue;
	  if (exps[i].X_op == O_constant
	      && value <= 7
	      && addr_mode[i] == (stm8_addr_mode_t) (ST8_BIT_0 + value))
	    continue;
	  break;
	case OP_INDX:
	  if (addr_mode[i] == ST8_INDX)
	    continue;
	  break;
	case OP_INDY:
	  if (addr_mode[i] == ST8_INDY)
	    continue;
	  break;
	case OP_SOFF_X:
	  if (addr_mode[i] == ST8_SHORTOFF_X)
	    continue;
	  break;
	case OP_OFF_X:
	  if (addr_mode[i] == ST8_SHORTOFF_X)
	    if (value < 0x100)
	      continue;
	  if (addr_mode[i] == ST8_LONGOFF_X)
	    continue;
	  if (addr_mode[i] == ST8_EXTOFF_X)
	    continue;
	  break;
	case OP_SOFF_Y:
	  if (addr_mode[i] == ST8_SHORTOFF_Y)
	    continue;
	  break;
	case OP_OFF_Y:
	  if (addr_mode[i] == ST8_SHORTOFF_Y)
	    if (value < 0x100)
	      continue;
	  if (addr_mode[i] == ST8_LONGOFF_Y)
	    continue;
	  if (addr_mode[i] == ST8_EXTOFF_Y)
	    continue;
	  break;
	case OP_SOFF_SP:
	  if (addr_mode[i] == ST8_SHORTOFF_SP)
	    continue;
	  break;
	case OP_SPTRW:
	  if (addr_mode[i] == ST8_SHORTPTRW)
	    continue;
	  break;
	case OP_LPTRW:
	  if (addr_mode[i] == ST8_LONGPTRW)
	    continue;
	  break;
	case OP_SPTRW_X:
	  if (addr_mode[i] == ST8_SHORTPTRW_X)
	    continue;
	  break;
	case OP_LPTRW_X:
	  if (addr_mode[i] == ST8_LONGPTRW_X)
	    continue;
	  break;
	case OP_SPTRW_Y:
	  if (addr_mode[i] == ST8_SHORTPTRW_Y)
	    continue;
	  break;
	case OP_LPTRW_Y:
	  if (addr_mode[i] == ST8_LONGPTRW_Y)
	    continue;
	  break;
	case OP_LPTRE:
	  if (addr_mode[i] == ST8_LONGPTRE)
	    continue;
	  break;
	case OP_LPTRE_X:
	  if (addr_mode[i] == ST8_LONGPTRE_X)
	    continue;
	  break;
	case OP_LPTRE_Y:
	  if (addr_mode[i] == ST8_LONGPTRE_Y)
	    continue;
	  break;
	case OP_MEM:
	  if (addr_mode[i] == ST8_PCREL)
	    continue;
	  if (addr_mode[i] == ST8_EXTMEM)
	    continue;
	  if (addr_mode[i] == ST8_LONGMEM)
	    continue;
	  break;
	case OP_SHORTMEM:
	  if (addr_mode[i] == ST8_SHORTMEM)
	    continue;
	  break;
	case OP_LO8:
	case OP_HI8:
	case OP_HH8:
	  if (addr_mode[i] == ST8_BYTE)
	    continue;
	  break;
	case OP_ILLEGAL:
	  break;
	}

      /* Not a match.  */
      ret++;
    }
  return ret;
}

/* This is the guts of the machine-dependent assembler.	 STR points to a
   machine dependent instruction.  This function is supposed to emit
   the frags/bytes it assembles to.  */

void
md_assemble (char *str)
{
  char op[16];
  char *saved_input_line_pointer = input_line_pointer;
  char *line = xstrdup (str);
  char *args;
  expressionS exps[3];
  const struct stm8_opcodes_s *opcode;
  int count;
  int i;

  args = skip_space (extract_word (line, op, sizeof (op)));
  memset (exps, 0, sizeof (exps));

  opcode = str_hash_find (stm8_hash, op);
  if (opcode == NULL)
    {
      as_bad (_("unknown opcode `%s'"), op);
      goto out;
    }

  count = read_args (args, exps);
  if (count < 0)
    goto out;

  for (i = 0; opcode[i].name != NULL && strcmp (op, opcode[i].name) == 0; i++)
    if (stm8_num_opcode_operands (&opcode[i]) == count
	&& cmpspec (opcode[i].constraints, exps, count) == 0)
      {
	int insn_size = stm8_compute_insn_size (&opcode[i]);
	unsigned int opcode_size = stm8_opcode_size (opcode[i].bin_opcode);
	char *frag = frag_more (insn_size);
	int output_count = count;

	bfd_put_bits (opcode[i].bin_opcode, frag, opcode_size * 8, true);
	frag += opcode_size;

	/* MOV encodes its two operands in reverse order.  */
	if (opcode[i].bin_opcode == 0x35
	    || opcode[i].bin_opcode == 0x45
	    || opcode[i].bin_opcode == 0x55)
	  output_count = -output_count;

	stm8_bfd_out (&opcode[i], exps, output_count, frag);
	dwarf2_emit_insn (insn_size);
	goto out;
      }

  as_bad (_("invalid instruction: %s"), str);

out:
  input_line_pointer = saved_input_line_pointer;
  free (line);
}

/* If you define this macro, it should return the position from which
   the PC relative adjustment for a PC relative fixup should be made.
   On many processors, the base of a PC relative instruction is the
   next instruction, so this macro would return the length of an
   instruction, plus the address of the PC relative fixup.  The latter
   can be calculated as fixp->fx_where +
   fixp->fx_frag->fr_address.  */

long
md_pcrel_from_section (fixS *fixp, segT sec)
{
  if (fixp->fx_addsy != NULL
      && (!S_IS_DEFINED (fixp->fx_addsy)
	  || (S_GET_SEGMENT (fixp->fx_addsy) != sec)))
    return 0;

  return fixp->fx_size + fixp->fx_where + fixp->fx_frag->fr_address;
}
