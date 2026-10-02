/* Target-dependent code for STM8, for GDB.

   Copyright (C) 1996-2026 Free Software Foundation, Inc.
   Copyright (c) 2026 Andrei-Edward Popa

   Written by Åke Rehnman (at) gmail dot com.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include "arch-utils.h"
#include "dwarf2/frame.h"
#include "extract-store-integer.h"
#include "frame.h"
#include "frame-unwind.h"
#include "dis-asm.h"
#include "gdbarch.h"
#include "gdbtypes.h"
#include "infcall.h"
#include "objfiles.h"
#include "progspace.h"
#include "regcache.h"
#include "target-descriptions.h"
#include "target.h"
#include "minsyms.h"
#include "gdbcore.h"
#include "elf-bfd.h"
#include "elf/stm8.h"
#include "opcode/stm8.h"
#include "gdbsupport/function-view.h"
#include "gdbsupport/selftest.h"

/* Raw registers.  Keep these numbers in sync with OpenOCD's STM8
   register packet.  */

enum stm8_regnum
{
  STM8_PC_REGNUM,
  STM8_A_REGNUM,
  STM8_X_REGNUM,
  STM8_Y_REGNUM,
  STM8_SP_REGNUM,
  STM8_CC_REGNUM,

  /* Byte aliases used by GCC's DWARF locations.  */
  STM8_XH_REGNUM,
  STM8_XL_REGNUM,
  STM8_YH_REGNUM,
  STM8_YL_REGNUM,
  STM8_FP_REGNUM
};

static constexpr int STM8_NUM_REGS = 6;
static constexpr int STM8_NUM_PSEUDO_REGS = 5;

static const char *const stm8_register_names[STM8_NUM_REGS]
  = { "pc", "a", "x", "y", "sp", "cc" };

static const int stm8_register_bits[STM8_NUM_REGS] = { 32, 8, 16, 16, 16, 8 };

struct stm8_gdbarch_tdep : gdbarch_tdep_base
{
  bool has_soft_fp = false;
  bool large_model = false;
};

static const char *
stm8_register_name (struct gdbarch *gdbarch, int regnum)
{
  if (regnum >= 0 && regnum < STM8_NUM_REGS)
    return stm8_register_names[regnum];

  switch (regnum)
    {
    case STM8_XH_REGNUM:
      return "xh";
    case STM8_XL_REGNUM:
      return "xl";
    case STM8_YH_REGNUM:
      return "yh";
    case STM8_YL_REGNUM:
      return "yl";
    case STM8_FP_REGNUM:
      return "fp";
    }

  return nullptr;
}

static struct type *
stm8_register_type (struct gdbarch *gdbarch, int regnum)
{
  switch (regnum)
    {
    case STM8_PC_REGNUM:
      /* OpenOCD transports the 24-bit PC in a 32-bit container.  */
      return builtin_type (gdbarch)->builtin_uint32;

    case STM8_X_REGNUM:
    case STM8_Y_REGNUM:
    case STM8_SP_REGNUM:
    case STM8_FP_REGNUM:
      return builtin_type (gdbarch)->builtin_uint16;

    default:
      return builtin_type (gdbarch)->builtin_uint8;
    }
}

struct stm8_pseudo_register_part
{
  int raw_regnum;
  int byte;
};

static stm8_pseudo_register_part
stm8_pseudo_register_part_for_regnum (int regnum)
{
  switch (regnum)
    {
    case STM8_XH_REGNUM:
      return { STM8_X_REGNUM, 0 };
    case STM8_XL_REGNUM:
      return { STM8_X_REGNUM, 1 };
    case STM8_YH_REGNUM:
      return { STM8_Y_REGNUM, 0 };
    case STM8_YL_REGNUM:
      return { STM8_Y_REGNUM, 1 };
    default:
      internal_error (_("invalid STM8 pseudo-register number %d"), regnum);
    }
}

static enum register_status
stm8_pseudo_register_read (struct gdbarch *gdbarch,
			   readable_regcache *regcache, int regnum,
			   gdb_byte *buf)
{
  if (regnum == STM8_FP_REGNUM)
    {
      bound_minimal_symbol fp = lookup_minimal_symbol (current_program_space,
						       "__stm8_frame_pointer");
      if (!fp.minsym || target_read_memory (fp.value_address (), buf, 2) != 0)
	return REG_UNAVAILABLE;
      return REG_VALID;
    }
  stm8_pseudo_register_part part
    = stm8_pseudo_register_part_for_regnum (regnum);
  gdb_byte raw[2];
  enum register_status status;

  status = regcache->raw_read (part.raw_regnum, raw);
  if (status == REG_VALID)
    buf[0] = raw[part.byte];
  return status;
}

static void
stm8_pseudo_register_write (struct gdbarch *gdbarch, struct regcache *regcache,
			    int regnum, const gdb_byte *buf)
{
  if (regnum == STM8_FP_REGNUM)
    {
      bound_minimal_symbol fp = lookup_minimal_symbol (current_program_space,
						       "__stm8_frame_pointer");
      if (!fp.minsym)
	error (_("STM8 software frame pointer is unavailable"));
      write_memory (fp.value_address (), buf, 2);
      return;
    }
  stm8_pseudo_register_part part
    = stm8_pseudo_register_part_for_regnum (regnum);
  gdb_byte raw[2];
  enum register_status status;

  status = regcache->raw_read (part.raw_regnum, raw);
  if (status == REG_VALID)
    {
      raw[part.byte] = buf[0];
      regcache->raw_write (part.raw_regnum, raw);
    }
}

/* A is a single byte and cannot be the low part of a multi-register
   data pointer.  DW_OP_breg expressions nevertheless read registers as
   pointer values; zero-extend A instead of consuming a byte of X.  */

static bool
stm8_convert_register_p (gdbarch *, int regnum, type *type)
{
  return regnum == STM8_A_REGNUM && type->code () == TYPE_CODE_PTR
    && type->length () == 2;
}

static bool
stm8_register_to_value (const frame_info_ptr &frame, int regnum, type *type,
			gdb_byte *buf, bool *optimized, bool *unavailable)
{
  gdb_byte byte;
  if (!get_frame_register_bytes (frame, regnum, 0,
				 gdb::make_array_view (&byte, 1),
				 optimized, unavailable))
    return false;
  store_unsigned_integer (buf, type->length (),
			  gdbarch_byte_order (get_frame_arch (frame)), byte);
  return true;
}

static void
stm8_value_to_register (const frame_info_ptr &frame, int regnum, type *type,
			const gdb_byte *buf)
{
  ULONGEST value = extract_unsigned_integer
    (buf, type->length (), gdbarch_byte_order (get_frame_arch (frame)));
  if (value > 0xff)
    error (_("value does not fit in the STM8 accumulator"));
  gdb_byte byte = value;
  put_frame_register (frame, regnum, gdb::make_array_view (&byte, 1));
}

/* GCC's byte registers retain their RTL numbers.  Columns 9 and 10
   describe complete index registers; column 11 is the return PC.  */

static constexpr int stm8_dwarf_regmap[]
  = { STM8_A_REGNUM,  STM8_YH_REGNUM, STM8_YL_REGNUM, STM8_XH_REGNUM,
      STM8_XL_REGNUM, STM8_SP_REGNUM, STM8_CC_REGNUM, -1,
      STM8_FP_REGNUM, STM8_X_REGNUM, STM8_Y_REGNUM, STM8_PC_REGNUM };

static int
stm8_dwarf_regnum (int reg)
{
  if ((unsigned int) reg < ARRAY_SIZE (stm8_dwarf_regmap))
    return stm8_dwarf_regmap[reg];
  return -1;
}

static int
stm8_dwarf2_reg_to_regnum (struct gdbarch *gdbarch, int reg)
{
  if (reg == 8
      && gdbarch_num_pseudo_regs (gdbarch) < STM8_NUM_PSEUDO_REGS)
    return -1;
  return stm8_dwarf_regnum (reg);
}

enum stm8_return_value_layout
{
  STM8_RETURN_VALUE_A,
  STM8_RETURN_VALUE_X,
  STM8_RETURN_VALUE_YL_X,
  STM8_RETURN_VALUE_Y_X,
  STM8_RETURN_VALUE_MEMORY
};

static enum stm8_return_value_layout
stm8_return_value_layout_for_size (ULONGEST size)
{
  switch (size)
    {
    case 1:
      return STM8_RETURN_VALUE_A;
    case 2:
      return STM8_RETURN_VALUE_X;
    case 3:
      return STM8_RETURN_VALUE_YL_X;
    case 4:
      return STM8_RETURN_VALUE_Y_X;
    default:
      return STM8_RETURN_VALUE_MEMORY;
    }
}

static void
stm8_store_return_value (struct type *type, struct regcache *regcache,
			 const gdb_byte *valbuf)
{
  switch (stm8_return_value_layout_for_size (type->length ()))
    {
    case STM8_RETURN_VALUE_A:
      regcache->raw_write (STM8_A_REGNUM, valbuf);
      break;

    case STM8_RETURN_VALUE_X:
      regcache->raw_write (STM8_X_REGNUM, valbuf);
      break;

    case STM8_RETURN_VALUE_YL_X:
      {
	gdb_byte y[2] = {};

	/* A 24-bit result is returned in YL:X.  Since STM8 is
	   big-endian, VALBUF[0] is the most significant byte.  */
	regcache->raw_read (STM8_Y_REGNUM, y);
	y[1] = valbuf[0];
	regcache->raw_write (STM8_Y_REGNUM, y);
	regcache->raw_write (STM8_X_REGNUM, valbuf + 1);
      }
      break;

    case STM8_RETURN_VALUE_Y_X:
      regcache->raw_write (STM8_Y_REGNUM, valbuf);
      regcache->raw_write (STM8_X_REGNUM, valbuf + 2);
      break;

    case STM8_RETURN_VALUE_MEMORY:
      error (_("unsupported STM8 return value size %s"),
	     pulongest (type->length ()));
    }
}

static void
stm8_extract_return_value (struct type *type, struct regcache *regcache,
			   gdb_byte *valbuf)
{
  switch (stm8_return_value_layout_for_size (type->length ()))
    {
    case STM8_RETURN_VALUE_A:
      regcache->raw_read (STM8_A_REGNUM, valbuf);
      break;

    case STM8_RETURN_VALUE_X:
      regcache->raw_read (STM8_X_REGNUM, valbuf);
      break;

    case STM8_RETURN_VALUE_YL_X:
      {
	gdb_byte y[2];

	regcache->raw_read (STM8_Y_REGNUM, y);
	valbuf[0] = y[1];
	regcache->raw_read (STM8_X_REGNUM, valbuf + 1);
      }
      break;

    case STM8_RETURN_VALUE_Y_X:
      regcache->raw_read (STM8_Y_REGNUM, valbuf);
      regcache->raw_read (STM8_X_REGNUM, valbuf + 2);
      break;

    case STM8_RETURN_VALUE_MEMORY:
      error (_("unsupported STM8 return value size %s"),
	     pulongest (type->length ()));
    }
}

/* Implement the return_value gdbarch method.  */

static enum return_value_convention
stm8_return_value (struct gdbarch *gdbarch, struct value *function,
		   struct type *valtype, struct regcache *regcache,
		   gdb_byte *readbuf, const gdb_byte *writebuf)
{
  if (valtype->code () == TYPE_CODE_STRUCT
      || valtype->code () == TYPE_CODE_UNION
      || valtype->code () == TYPE_CODE_ARRAY
      || stm8_return_value_layout_for_size (valtype->length ())
	   == STM8_RETURN_VALUE_MEMORY)
    return RETURN_VALUE_STRUCT_CONVENTION;

  if (readbuf != nullptr)
    stm8_extract_return_value (valtype, regcache, readbuf);
  if (writebuf != nullptr)
    stm8_store_return_value (valtype, regcache, writebuf);

  return RETURN_VALUE_REGISTER_CONVENTION;
}

enum stm8_return_kind
{
  STM8_RETURN_RET,
  STM8_RETURN_RETF,
  STM8_RETURN_IRET
};

static constexpr int STM8_REG_UNSAVED = 0x7fffffff;

struct stm8_prologue
{
  CORE_ADDR prologue_end;
  int frame_size;
  int fp_frame_depth;
  int fp_saved_offset;
  int saved_entry_offset[STM8_NUM_REGS];
};

struct stm8_frame_cache
{
  struct stm8_prologue prologue;
  CORE_ADDR entry_sp;
  CORE_ADDR caller_sp;
  enum stm8_return_kind return_kind;
};

static void
stm8_init_prologue (struct stm8_prologue *p)
{
  p->prologue_end = 0;
  p->frame_size = 0;
  p->fp_frame_depth = -1;
  p->fp_saved_offset = STM8_REG_UNSAVED;
  for (int regnum = 0; regnum < STM8_NUM_REGS; ++regnum)
    p->saved_entry_offset[regnum] = STM8_REG_UNSAVED;
}

/* Follow stack changes in the straight-line body after the prologue.
   A leading PUSH may save a scratch register for just a few instructions;
   treating that save as permanent displaces the caller's return address
   after the matching POP.  Stop at control-flow changes, since a linear
   scan cannot determine which branch the inferior took.  */

static void
stm8_analyze_stack_changes (
  CORE_ADDR pc, CORE_ADDR stop_pc, struct stm8_prologue *p,
  gdb::function_view<int (CORE_ADDR, gdb_byte *, int)> read_code)
{
  int depth = p->frame_size;
  int stable_depth = depth;
  while (pc < stop_pc)
    {
      gdb_byte buf[5];
      if (read_code (pc, buf, 1) != 0)
	break;
      unsigned int insn = buf[0];
      if (buf[0] == 0x72 || buf[0] == 0x90
	  || buf[0] == 0x91 || buf[0] == 0x92)
	{
	  if (read_code (pc + 1, buf + 1, 1) != 0)
	    break;
	  insn = (insn << 8) | buf[1];
	}
      const struct stm8_opcodes_s *opcode = stm8_opcodes;
      while (opcode->name != nullptr && opcode->bin_opcode != insn)
	++opcode;
      if (opcode->name == nullptr || opcode->name[0] == 'j'
	  || startswith (opcode->name, "btj")
	  || startswith (opcode->name, "ret")
	  || strcmp (opcode->name, "iret") == 0
	  || strcmp (opcode->name, "trap") == 0
	  || startswith (opcode->name, "call"))
	{
	  /* Callees can pop arguments.  Do not carry an unmatched body
	     save across a call or assume a branch's fall-through path.  */
	  depth = stable_depth;
	  break;
	}
      int length = stm8_compute_insn_size (opcode);
      if (pc + length > stop_pc || read_code (pc, buf, length) != 0)
	break;

      if (startswith (opcode->name, "push"))
	depth += strcmp (opcode->name, "pushw") == 0 ? 2 : 1;
      else if (startswith (opcode->name, "pop"))
	{
	  int size = strcmp (opcode->name, "popw") == 0 ? 2 : 1;
	  if (depth < size)
	    break;
	  for (int regnum = 0; regnum < STM8_NUM_REGS; ++regnum)
	    if (p->saved_entry_offset[regnum] >= 1 - depth
		&& p->saved_entry_offset[regnum] <= size - depth)
	      p->saved_entry_offset[regnum] = STM8_REG_UNSAVED;
	  depth -= size;
	}
      else if (opcode->constraints[0] == ST8_REG_SP)
	{
	  if (insn == 0x52)
	    depth += buf[1];
	  else if (insn == 0x5b && depth >= buf[1])
	    depth -= buf[1];
	  else
	    break;
	}
      stable_depth = std::min (stable_depth, depth);
      pc += length;
    }
  p->frame_size = depth;
}

/* Analyze the stack-changing part of an STM8 function prologue.

   SP points at the next free stack byte.  The offsets recorded here are
   relative to SP at function entry, after CALL/CALLF has pushed the return
   address.  */

static void
stm8_analyze_prologue (CORE_ADDR start_pc, CORE_ADDR stop_pc,
		       struct stm8_prologue *p)
{
  CORE_ADDR pc = start_pc;
  int depth = 0;
  bool y_is_old_fp = false;
  int y_sp_depth = -1;
  bound_minimal_symbol fp = lookup_minimal_symbol (current_program_space,
						   "__stm8_frame_pointer");

  stm8_init_prologue (p);
  p->prologue_end = start_pc;

  while (pc < stop_pc)
    {
      gdb_byte buf[4];
      unsigned int insn;
      int length;

      if (target_read_code (pc, buf, sizeof (buf)) != 0)
	break;

      insn = buf[0];
      if (buf[0] == 0x90)
	insn = (insn << 8) | buf[1];

      switch (insn)
	{
	case 0x90ce: /* LDW Y,__stm8_frame_pointer.  */
	  if (!fp.minsym || ((buf[2] << 8) | buf[3]) != fp.value_address ())
	    goto end_prologue;
	  y_is_old_fp = true;
	  length = 4;
	  break;
	case 0x3b: /* PUSH longmem.  */
	  ++depth;
	  length = 3;
	  break;

	case 0x52: /* SUB SP,#imm8.  */
	  depth += buf[1];
	  length = 2;
	  break;

	case 0x88: /* PUSH A.  */
	  p->saved_entry_offset[STM8_A_REGNUM] = -depth;
	  ++depth;
	  length = 1;
	  break;

	case 0x89: /* PUSHW X.  */
	  p->saved_entry_offset[STM8_X_REGNUM] = -(depth + 1);
	  depth += 2;
	  length = 1;
	  break;

	case 0x8a: /* PUSH CC.  */
	  p->saved_entry_offset[STM8_CC_REGNUM] = -depth;
	  ++depth;
	  length = 1;
	  break;

	case 0x9089: /* PUSHW Y.  */
	  if (y_is_old_fp)
	    {
	      p->fp_saved_offset = -(depth + 1);
	      y_is_old_fp = false;
	    }
	  p->saved_entry_offset[STM8_Y_REGNUM] = -(depth + 1);
	  depth += 2;
	  length = 2;
	  break;

	case 0x9096: /* LDW Y,SP.  */
	  y_sp_depth = depth;
	  length = 2;
	  break;

	case 0x90cf: /* LDW longmem,Y.  */
	  if (!fp.minsym || ((buf[2] << 8) | buf[3]) != fp.value_address ()
	      || y_sp_depth < 0)
	    goto end_prologue;
	  p->fp_frame_depth = y_sp_depth;
	  length = 4;
	  break;

	default:
	end_prologue:
	  p->frame_size = depth;
	  p->prologue_end = pc;
	  stm8_analyze_stack_changes (pc, stop_pc, p, target_read_code);
	  return;
	}

      pc += length;
      p->prologue_end = pc;
    }

  p->frame_size = depth;
}

/* Locate the final opcode at an instruction boundary.  An address operand
   can end in the same byte as RETF or IRET.  */

static int
stm8_last_opcode (const gdb_byte *code, size_t size)
{
  size_t offset = 0;
  while (offset < size)
    {
      unsigned int insn = code[offset];
      if (insn == 0x72 || insn == 0x90 || insn == 0x91 || insn == 0x92)
	{
	  if (offset + 1 == size)
	    return -1;
	  insn = (insn << 8) | code[offset + 1];
	}
      const stm8_opcodes_s *opcode = stm8_opcodes;
      while (opcode->name != nullptr && opcode->bin_opcode != insn)
	++opcode;
      if (opcode->name == nullptr)
	return -1;
      unsigned int length = stm8_compute_insn_size (opcode);
      if (length > size - offset)
	return -1;
      offset += length;
      if (offset == size)
	return insn;
    }
  return -1;
}

static enum stm8_return_kind
stm8_return_kind_for_pc (CORE_ADDR pc)
{
  gdb_byte opcode;

  /* If execution is stopped on a return instruction, this is the most
     reliable indication of the frame format.  */
  if (target_read_code (pc, &opcode, 1) == 0)
    {
      if (opcode == 0x87)
	return STM8_RETURN_RETF;
      if (opcode == 0x80)
	return STM8_RETURN_IRET;
      if (opcode == 0x81)
	return STM8_RETURN_RET;
    }

  const char *name = nullptr;
  CORE_ADDR func_start = 0;
  CORE_ADDR func_end = 0;

  if (find_pc_partial_function (pc, &name, &func_start, &func_end)
      && func_end > func_start
      && target_read_code (func_end - 1, &opcode, 1) == 0
      && (opcode == 0x87 || opcode == 0x80))
    {
      std::vector<gdb_byte> code (func_end - func_start);
      if (target_read_code (func_start, code.data (), code.size ()) == 0)
	{
	  int last = stm8_last_opcode (code.data (), code.size ());
	  if (last == 0x87)
	    return STM8_RETURN_RETF;
	  if (last == 0x80)
	    return STM8_RETURN_IRET;
	}
    }

  /* Large-model tail calls need not end in RETF.  The ELF ABI marker is
     authoritative when no explicit return/interrupt instruction applies.  */
  bfd *abfd = current_program_space->exec_bfd ();
  if (abfd != nullptr && bfd_get_flavour (abfd) == bfd_target_elf_flavour
      && (elf_elfheader (abfd)->e_flags & EF_STM8_MODEL_LARGE))
    return STM8_RETURN_RETF;

  return STM8_RETURN_RET;
}

static int
stm8_return_address_size (enum stm8_return_kind kind)
{
  switch (kind)
    {
    case STM8_RETURN_RET:
      return 2;
    case STM8_RETURN_RETF:
      return 3;
    case STM8_RETURN_IRET:
      return 9;
    }

  gdb_assert_not_reached ("invalid STM8 return kind");
}

/* Return the register for an explicit argument, or -1 for the stack.
   Aggregates and variadic arguments are always passed on the stack.
   Only the first two explicit arguments can use A and X.  */

static int
stm8_argument_regnum (int index, int first_regnum, ULONGEST size,
		      bool aggregate, bool variadic)
{
  if (aggregate || variadic)
    return -1;
  if (size == 1 && (index == 0
		    || (index == 1 && first_regnum == STM8_X_REGNUM)))
    return STM8_A_REGNUM;
  if (size == 2 && (index == 0
		    || (index == 1 && first_regnum == STM8_A_REGNUM)))
    return STM8_X_REGNUM;
  return -1;
}

static CORE_ADDR
stm8_frame_align (gdbarch *, CORE_ADDR sp)
{
  return sp;
}

/* RAM is executable on STM8.  Reserve the return breakpoint on the stack
   rather than modifying flash at the program entry point for every call.
   Keep the slot above SP because pushes use post-decrement addressing.  */

static CORE_ADDR
stm8_push_dummy_code (gdbarch *, CORE_ADDR sp, CORE_ADDR function,
		      value **, int, type *, CORE_ADDR *real_pc,
		      CORE_ADDR *bp_addr, regcache *)
{
  if (stm8_return_kind_for_pc (function) == STM8_RETURN_IRET)
    error (_("Cannot call an STM8 interrupt handler from GDB"));
  if (sp < 2 || sp > 0xffff)
    error (_("STM8 inferior call exceeds the near stack address space"));
  *bp_addr = --sp;
  *real_pc = function;
  return sp - 1;
}

/* Near nonvariadic functions with a small result, or with a floating
   result and first argument, discard their stack arguments on return.  */

static bool
stm8_callee_pops_args (type *func_type, int return_size)
{
  type *result_type = check_typedef (func_type->target_type ());
  return return_size == 2 && !func_type->has_varargs ()
    && (result_type->code () == TYPE_CODE_VOID || result_type->length () <= 2
	|| (result_type->code () == TYPE_CODE_FLT
	    && func_type->num_fields () > 0
	    && check_typedef (func_type->field (0).type ())->code ()
	       == TYPE_CODE_FLT));
}

/* While stopped in an inferior call, the unwound SP precedes callee
   cleanup.  Match the SP at the return breakpoint used by push_dummy_call.
   At that breakpoint itself there is no callee frame to account for.  */

static frame_id
stm8_dummy_id (gdbarch *, const frame_info_ptr &frame)
{
  CORE_ADDR sp = get_frame_sp (frame);
  frame_info_ptr callee = get_next_frame (frame);
  if (callee != nullptr)
    {
      symbol *func = get_frame_function (callee);
      if (func != nullptr)
	{
	  type *func_type = check_typedef (func->type ());
	  int return_size = stm8_return_address_size
	    (stm8_return_kind_for_pc (get_frame_pc (callee)));
	  if (stm8_callee_pops_args (func_type, return_size))
	    {
	      type *result_type = check_typedef (func_type->target_type ());
	      if (result_type->code () == TYPE_CODE_STRUCT
		  || result_type->code () == TYPE_CODE_UNION
		  || result_type->code () == TYPE_CODE_ARRAY)
		sp += 2;
	      int first_regnum = -1;
	      for (int i = 0; i < func_type->num_fields (); ++i)
		{
		  type *arg_type = check_typedef (func_type->field (i).type ());
		  bool aggregate = arg_type->code () == TYPE_CODE_STRUCT
		    || arg_type->code () == TYPE_CODE_UNION
		    || arg_type->code () == TYPE_CODE_ARRAY;
		  int regnum = stm8_argument_regnum
		    (i, first_regnum, arg_type->length (), aggregate, false);
		  if (i == 0)
		    first_regnum = regnum;
		  if (regnum < 0)
		    sp += arg_type->length ();
		}
	    }
	}
    }
  return frame_id_build (sp, get_frame_pc (frame));
}

/* Implement the push_dummy_call gdbarch method.  The hidden result
   pointer precedes stack arguments without consuming an explicit
   argument position.  The stack is byte aligned and grows downward
   with post-decrement pushes.  */

static CORE_ADDR
stm8_push_dummy_call (gdbarch *gdbarch, value *function, regcache *regcache,
		      CORE_ADDR bp_addr, int nargs, value **args,
		      CORE_ADDR sp, function_call_return_method return_method,
		      CORE_ADDR struct_addr)
{
  type *func_type = check_typedef (function->type ());
  while (func_type->code () == TYPE_CODE_PTR)
    func_type = check_typedef (func_type->target_type ());
  gdb_assert (func_type->code () == TYPE_CODE_FUNC
	      || func_type->code () == TYPE_CODE_METHOD);

  CORE_ADDR func_addr = find_function_addr (function, nullptr);
  stm8_return_kind return_kind = stm8_return_kind_for_pc (func_addr);
  int return_size = stm8_return_address_size (return_kind);

  bool variadic = func_type->has_varargs ();
  int first_regnum = -1;
  std::vector<gdb_byte> stack_args;
  if (return_method == return_method_struct)
    {
      stack_args.resize (2);
      store_unsigned_integer (stack_args.data (), 2,
			      gdbarch_byte_order (gdbarch), struct_addr);
      if (sp == struct_addr)
	--sp;
    }

  for (int i = 0; i < nargs; ++i)
    {
      value *arg = args[i];
      /* infcall promotes narrow integers to int even for prototyped
	 functions.  The STM8 ABI passes the declared byte width.  */
      if (func_type->is_prototyped () && i < func_type->num_fields ())
	arg = value_cast (func_type->field (i).type (), arg);
      type *arg_type = check_typedef (arg->type ());
      auto contents = arg->contents_all ();
      bool aggregate = arg_type->code () == TYPE_CODE_STRUCT
		       || arg_type->code () == TYPE_CODE_UNION
		       || arg_type->code () == TYPE_CODE_ARRAY;
      int regnum = stm8_argument_regnum (i, first_regnum, arg_type->length (),
					aggregate, variadic);
      if (i == 0)
	first_regnum = regnum;
      if (regnum >= 0)
	regcache->raw_write (regnum, contents.data ());
      else
	stack_args.insert (stack_args.end (),
			   contents.begin (), contents.end ());
    }

  if (sp > 0xffff || stack_args.size () + return_size >= sp)
    error (_("STM8 inferior call exceeds the near stack address space"));
  if (bp_addr >= (CORE_ADDR (1) << (8 * return_size)))
    error (_("STM8 inferior call return address is out of range"));

  sp -= stack_args.size ();
  CORE_ADDR caller_sp = sp;
  if (!stack_args.empty ())
    write_memory (sp + 1, stack_args.data (), stack_args.size ());

  if (stm8_callee_pops_args (func_type, return_size))
    caller_sp += stack_args.size ();

  gdb_byte return_pc[3];
  store_unsigned_integer (return_pc, return_size,
			  gdbarch_byte_order (gdbarch), bp_addr);
  sp -= return_size;
  write_memory (sp + 1, return_pc, return_size);
  regcache_cooked_write_unsigned (regcache, STM8_SP_REGNUM, sp);
  return caller_sp;
}

static CORE_ADDR
stm8_skip_prologue (struct gdbarch *gdbarch, CORE_ADDR pc)
{
  CORE_ADDR func_start;
  CORE_ADDR func_end;
  CORE_ADDR sal_end;
  struct stm8_prologue p;

  if (!find_pc_partial_function (pc, nullptr, &func_start, &func_end))
    return pc;

  stm8_analyze_prologue (func_start, func_end, &p);
  sal_end = skip_prologue_using_sal (gdbarch, func_start);

  if (sal_end > p.prologue_end)
    p.prologue_end = sal_end;

  return p.prologue_end > pc ? p.prologue_end : pc;
}

static struct stm8_frame_cache *
stm8_get_frame_cache (const frame_info_ptr &this_frame, void **this_cache)
{
  if (*this_cache != nullptr)
    return static_cast<stm8_frame_cache *> (*this_cache);

  auto *cache = frame_obstack_zalloc<stm8_frame_cache> ();
  *this_cache = cache;
  stm8_init_prologue (&cache->prologue);

  CORE_ADDR func_start = get_frame_func (this_frame);
  CORE_ADDR current_pc = get_frame_pc (this_frame);
  CORE_ADDR current_sp = get_frame_sp (this_frame);

  if (func_start != 0)
    stm8_analyze_prologue (func_start, current_pc, &cache->prologue);

  cache->return_kind = stm8_return_kind_for_pc (current_pc);

  /* At the return instruction the epilogue has already restored SP.  */
  gdb_byte opcode;
  if (target_read_code (current_pc, &opcode, 1) == 0
      && (opcode == 0x80 || opcode == 0x81 || opcode == 0x87))
    stm8_init_prologue (&cache->prologue);

  cache->entry_sp = current_sp + cache->prologue.frame_size;
  if (cache->prologue.fp_frame_depth >= 0)
    cache->entry_sp = get_frame_register_unsigned (this_frame, STM8_FP_REGNUM)
		      + cache->prologue.fp_frame_depth;
  cache->caller_sp = cache->entry_sp
		     + stm8_return_address_size (cache->return_kind);

  return cache;
}

static void
stm8_frame_this_id (const frame_info_ptr &this_frame, void **this_cache,
		    struct frame_id *this_id)
{
  struct stm8_frame_cache *cache = stm8_get_frame_cache (this_frame,
							 this_cache);

  if (cache->caller_sp == 0)
    return;

  *this_id = frame_id_build (cache->caller_sp, get_frame_func (this_frame));
}

static bool
stm8_read_stack_value (struct gdbarch *gdbarch, CORE_ADDR address, int size,
		       ULONGEST *value)
{
  gdb_byte buf[3];

  gdb_assert (size >= 1 && size <= (int) sizeof (buf));
  if (target_read_memory (address, buf, size) != 0)
    return false;

  *value = extract_unsigned_integer (buf, size, gdbarch_byte_order (gdbarch));
  return true;
}

static struct value *
stm8_frame_prev_register (const frame_info_ptr &this_frame, void **this_cache,
			  int regnum)
{
  struct stm8_frame_cache *cache = stm8_get_frame_cache (this_frame,
							 this_cache);
  struct gdbarch *gdbarch = get_frame_arch (this_frame);

  if (regnum == STM8_SP_REGNUM)
    return frame_unwind_got_constant (this_frame, regnum, cache->caller_sp);

  if (regnum == STM8_PC_REGNUM)
    {
      CORE_ADDR address;
      int size;
      ULONGEST pc;

      if (cache->return_kind == STM8_RETURN_IRET)
	{
	  address = cache->entry_sp + 7;
	  size = 3;
	}
      else
	{
	  address = cache->entry_sp + 1;
	  size = cache->return_kind == STM8_RETURN_RETF ? 3 : 2;
	}

      if (!stm8_read_stack_value (gdbarch, address, size, &pc))
	return frame_unwind_got_optimized (this_frame, regnum);

      /* CALL and CALLR do not stack PCE.  They can only return within
	 the same 64-KiB section, so recover PCE from this function.  */
      if (cache->return_kind == STM8_RETURN_RET)
	pc |= get_frame_pc (this_frame) & 0xff0000;

      return frame_unwind_got_constant (this_frame, regnum, pc);
    }

  if (regnum == STM8_FP_REGNUM)
    {
      if (cache->prologue.fp_saved_offset != STM8_REG_UNSAVED)
	return frame_unwind_got_memory (this_frame, regnum,
					cache->entry_sp
					  + cache->prologue.fp_saved_offset);
      return frame_unwind_got_register (this_frame, regnum, regnum);
    }

  if (cache->return_kind == STM8_RETURN_IRET)
    {
      switch (regnum)
	{
	case STM8_CC_REGNUM:
	  return frame_unwind_got_memory (this_frame, regnum,
					  cache->entry_sp + 1);
	case STM8_A_REGNUM:
	  return frame_unwind_got_memory (this_frame, regnum,
					  cache->entry_sp + 2);
	case STM8_X_REGNUM:
	  return frame_unwind_got_memory (this_frame, regnum,
					  cache->entry_sp + 3);
	case STM8_Y_REGNUM:
	  return frame_unwind_got_memory (this_frame, regnum,
					  cache->entry_sp + 5);
	}
    }

  if (regnum >= 0 && regnum < STM8_NUM_REGS
      && cache->prologue.saved_entry_offset[regnum] != STM8_REG_UNSAVED)
    return frame_unwind_got_memory (
      this_frame, regnum,
      cache->entry_sp + cache->prologue.saved_entry_offset[regnum]);

  return frame_unwind_got_register (this_frame, regnum, regnum);
}

static const struct frame_unwind_legacy
  stm8_frame_unwind ("stm8 prologue", NORMAL_FRAME, FRAME_UNWIND_ARCH,
		     default_frame_unwind_stop_reason, stm8_frame_this_id,
		     stm8_frame_prev_register, nullptr, default_frame_sniffer);

/* The software FP is callee preserved.  Byte pseudo-registers also
   need an explicit default rule; GDB's generic unspecified-register
   fallback only handles raw registers.  Explicit CFI overrides these
   defaults while a register is saved on the stack.  */

static void
stm8_dwarf2_frame_init_reg (gdbarch *, int regnum,
			    dwarf2_frame_state_reg *reg,
			    const frame_info_ptr &)
{
  if (regnum == STM8_PC_REGNUM)
    reg->how = DWARF2_FRAME_REG_RA;
  else if (regnum == STM8_SP_REGNUM)
    reg->how = DWARF2_FRAME_REG_CFA;
  else if (regnum >= STM8_NUM_REGS)
    reg->how = DWARF2_FRAME_REG_SAME_VALUE;
}

constexpr gdb_byte stm8_break_insn[] = { 0x8b };
using stm8_breakpoint = BP_MANIPULATION (stm8_break_insn);

/* Validate a target-supplied description and map the six raw registers
   by name.  This deliberately does not depend on the remote register
   order.  */

static bool
stm8_validate_tdesc (const struct target_desc *tdesc,
		     tdesc_arch_data_up *tdesc_data)
{
  const struct tdesc_feature *feature;
  tdesc_arch_data_up data;

  if (tdesc == nullptr || !tdesc_has_registers (tdesc))
    return false;

  feature = tdesc_find_feature (tdesc, "org.gnu.gdb.stm8.core");
  if (feature == nullptr)
    return false;

  data = tdesc_data_alloc ();
  for (int regnum = 0; regnum < STM8_NUM_REGS; ++regnum)
    {
      if (!tdesc_numbered_register (feature, data.get (), regnum,
				    stm8_register_names[regnum])
	  || tdesc_register_bitsize (feature, stm8_register_names[regnum])
	       != stm8_register_bits[regnum])
	return false;
    }

  *tdesc_data = std::move (data);
  return true;
}

static struct gdbarch *
stm8_gdbarch_init (struct gdbarch_info info, struct gdbarch_list *arches)
{
  const struct target_desc *tdesc = info.target_desc;
  tdesc_arch_data_up tdesc_data;

  if (tdesc != nullptr && tdesc_has_registers (tdesc)
      && !stm8_validate_tdesc (tdesc, &tdesc_data))
    return nullptr;

  /* A software register exists only in executables which define its
     backing object.  Bare architectures retain the
     four ordinary byte aliases without an unreadable FP register.  */
  bool has_soft_fp = false;
  bfd *abfd = info.abfd;
  if (abfd == nullptr && current_program_space != nullptr)
    abfd = current_program_space->exec_bfd ();
  bool large_model = (abfd != nullptr
		      && bfd_get_flavour (abfd) == bfd_target_elf_flavour
		      && (elf_elfheader (abfd)->e_flags & EF_STM8_MODEL_LARGE));
  if (abfd != nullptr)
    {
      long bytes = bfd_get_symtab_upper_bound (abfd);
      if (bytes > 0)
	{
	  std::vector<asymbol *> symbols (bytes / sizeof (asymbol *));
	  long count = bfd_canonicalize_symtab (abfd, symbols.data ());
	  for (long i = 0; i < count; ++i)
	    if (strcmp (bfd_asymbol_name (symbols[i]), "__stm8_frame_pointer")
		== 0)
	      {
		has_soft_fp = true;
		break;
	      }
	}
    }
  for (arches = gdbarch_list_lookup_by_info (arches, &info); arches != nullptr;
       arches = gdbarch_list_lookup_by_info (arches->next, &info))
    if (gdbarch_tdep<stm8_gdbarch_tdep> (arches->gdbarch)->has_soft_fp
	== has_soft_fp
	&& gdbarch_tdep<stm8_gdbarch_tdep> (arches->gdbarch)->large_model
	   == large_model)
      return arches->gdbarch;

  gdbarch *gdbarch = gdbarch_alloc (&info,
				    gdbarch_tdep_up (new stm8_gdbarch_tdep));
  gdbarch_tdep<stm8_gdbarch_tdep> (gdbarch)->has_soft_fp = has_soft_fp;
  gdbarch_tdep<stm8_gdbarch_tdep> (gdbarch)->large_model = large_model;

  set_gdbarch_num_regs (gdbarch, STM8_NUM_REGS);
  set_gdbarch_num_pseudo_regs (gdbarch, has_soft_fp
					  ? STM8_NUM_PSEUDO_REGS
					  : STM8_NUM_PSEUDO_REGS - 1);
  set_gdbarch_register_name (gdbarch, stm8_register_name);
  set_gdbarch_register_type (gdbarch, stm8_register_type);
  set_gdbarch_pseudo_register_read (gdbarch, stm8_pseudo_register_read);
  set_gdbarch_convert_register_p (gdbarch, stm8_convert_register_p);
  set_gdbarch_register_to_value (gdbarch, stm8_register_to_value);
  set_gdbarch_value_to_register (gdbarch, stm8_value_to_register);
  set_gdbarch_deprecated_pseudo_register_write (gdbarch,
						stm8_pseudo_register_write);
  set_tdesc_pseudo_register_name (gdbarch, stm8_register_name);
  set_tdesc_pseudo_register_type (gdbarch, stm8_register_type);

  set_gdbarch_pc_regnum (gdbarch, STM8_PC_REGNUM);
  set_gdbarch_sp_regnum (gdbarch, STM8_SP_REGNUM);
  set_gdbarch_ps_regnum (gdbarch, STM8_CC_REGNUM);
  set_gdbarch_dwarf2_reg_to_regnum (gdbarch, stm8_dwarf2_reg_to_regnum);

  /* STM8 C implementations use 16-bit data pointers and a 16-bit int,
     while BFD/GDB keep code addresses in a 32-bit CORE_ADDR container.  */
  set_gdbarch_char_signed (gdbarch, false);
  set_gdbarch_short_bit (gdbarch, 16);
  set_gdbarch_int_bit (gdbarch, 16);
  set_gdbarch_long_bit (gdbarch, 32);
  set_gdbarch_long_long_bit (gdbarch, 64);
  set_gdbarch_float_bit (gdbarch, 32);
  set_gdbarch_double_bit (gdbarch, 32);
  set_gdbarch_long_double_bit (gdbarch, 32);
  set_gdbarch_float_format (gdbarch, floatformats_ieee_single);
  set_gdbarch_double_format (gdbarch, floatformats_ieee_single);
  set_gdbarch_long_double_format (gdbarch, floatformats_ieee_single);
  set_gdbarch_ptr_bit (gdbarch, 16);
  set_gdbarch_code_ptr_bit (gdbarch, large_model ? 24 : 16);
  set_gdbarch_addr_bit (gdbarch, 32);
  set_gdbarch_dwarf2_addr_size (gdbarch, 4);

  set_gdbarch_inner_than (gdbarch, core_addr_lessthan);
  set_gdbarch_breakpoint_kind_from_pc (gdbarch, stm8_breakpoint::kind_from_pc);
  set_gdbarch_sw_breakpoint_from_kind (gdbarch, stm8_breakpoint::bp_from_kind);
  set_gdbarch_print_insn (gdbarch, print_insn_stm8);
  set_gdbarch_return_value (gdbarch, stm8_return_value);
  set_gdbarch_push_dummy_call (gdbarch, stm8_push_dummy_call);
  set_gdbarch_call_dummy_location (gdbarch, ON_STACK);
  set_gdbarch_push_dummy_code (gdbarch, stm8_push_dummy_code);
  set_gdbarch_dummy_id (gdbarch, stm8_dummy_id);
  set_gdbarch_frame_align (gdbarch, stm8_frame_align);

  set_gdbarch_skip_prologue (gdbarch, stm8_skip_prologue);
  dwarf2_frame_set_init_reg (gdbarch, stm8_dwarf2_frame_init_reg);
  dwarf2_append_unwinders (gdbarch);
  frame_unwind_append_unwinder (gdbarch, &stm8_frame_unwind);

  if (tdesc_data != nullptr)
    {
      tdesc_use_registers (gdbarch, tdesc, std::move (tdesc_data), nullptr);

      /* A target description using code_ptr for PC would otherwise inherit
	 ptr_bit (16).  The remote PC is a 32-bit transport container.  */
      set_gdbarch_register_type (gdbarch, stm8_register_type);
    }

  return gdbarch;
}

#if GDB_SELF_TEST

namespace selftests
{

static void
stm8_last_opcode_test ()
{
  const gdb_byte jump_80[] = { 0xac, 0, 0x81, 0x80 };
  const gdb_byte jump_87[] = { 0xac, 0, 0x81, 0x87 };
  const gdb_byte interrupt[] = { 0x90, 0x89, 0x80 };
  const gdb_byte far_return[] = { 0xae, 0x12, 0x34, 0x87 };
  SELF_CHECK (stm8_last_opcode (jump_80, sizeof (jump_80)) == 0xac);
  SELF_CHECK (stm8_last_opcode (jump_87, sizeof (jump_87)) == 0xac);
  SELF_CHECK (stm8_last_opcode (interrupt, sizeof (interrupt)) == 0x80);
  SELF_CHECK (stm8_last_opcode (far_return, sizeof (far_return)) == 0x87);
  SELF_CHECK (stm8_last_opcode (far_return, 2) == -1);
}

static void
stm8_argument_register_test ()
{
  SELF_CHECK (stm8_argument_regnum (0, -1, 1, false, false)
	      == STM8_A_REGNUM);
  SELF_CHECK (stm8_argument_regnum (0, -1, 2, false, false)
	      == STM8_X_REGNUM);
  SELF_CHECK (stm8_argument_regnum (1, STM8_A_REGNUM, 2, false, false)
	      == STM8_X_REGNUM);
  SELF_CHECK (stm8_argument_regnum (1, STM8_X_REGNUM, 1, false, false)
	      == STM8_A_REGNUM);
  SELF_CHECK (stm8_argument_regnum (1, STM8_A_REGNUM, 1, false, false) == -1);
  SELF_CHECK (stm8_argument_regnum (1, STM8_X_REGNUM, 2, false, false) == -1);
  for (int size : {1, 2, 3, 4, 8})
    {
      SELF_CHECK (stm8_argument_regnum (2, STM8_A_REGNUM, size, false, false)
		  == -1);
      SELF_CHECK (stm8_argument_regnum (0, -1, size, true, false) == -1);
      SELF_CHECK (stm8_argument_regnum (0, -1, size, false, true) == -1);
    }
}

static void
stm8_pseudo_register_mapping_test ()
{
  stm8_pseudo_register_part part;

  part = stm8_pseudo_register_part_for_regnum (STM8_XH_REGNUM);
  SELF_CHECK (part.raw_regnum == STM8_X_REGNUM && part.byte == 0);
  part = stm8_pseudo_register_part_for_regnum (STM8_XL_REGNUM);
  SELF_CHECK (part.raw_regnum == STM8_X_REGNUM && part.byte == 1);
  part = stm8_pseudo_register_part_for_regnum (STM8_YH_REGNUM);
  SELF_CHECK (part.raw_regnum == STM8_Y_REGNUM && part.byte == 0);
  part = stm8_pseudo_register_part_for_regnum (STM8_YL_REGNUM);
  SELF_CHECK (part.raw_regnum == STM8_Y_REGNUM && part.byte == 1);
}

static void
stm8_dwarf_register_mapping_test ()
{
  const int expected[] = { STM8_A_REGNUM, STM8_YH_REGNUM, STM8_YL_REGNUM,
			   STM8_XH_REGNUM, STM8_XL_REGNUM, STM8_SP_REGNUM,
			   STM8_CC_REGNUM, -1, STM8_FP_REGNUM,
			   STM8_X_REGNUM, STM8_Y_REGNUM, STM8_PC_REGNUM };

  for (unsigned int i = 0; i < ARRAY_SIZE (expected); ++i)
    SELF_CHECK (stm8_dwarf_regnum (i) == expected[i]);

  SELF_CHECK (stm8_dwarf_regnum (-1) == -1);
  SELF_CHECK (stm8_dwarf_regnum (ARRAY_SIZE (expected)) == -1);
}

static void
stm8_return_value_layout_test ()
{
  SELF_CHECK (stm8_return_value_layout_for_size (1) == STM8_RETURN_VALUE_A);
  SELF_CHECK (stm8_return_value_layout_for_size (2) == STM8_RETURN_VALUE_X);
  SELF_CHECK (stm8_return_value_layout_for_size (3) == STM8_RETURN_VALUE_YL_X);
  SELF_CHECK (stm8_return_value_layout_for_size (4) == STM8_RETURN_VALUE_Y_X);
  SELF_CHECK (stm8_return_value_layout_for_size (5)
	      == STM8_RETURN_VALUE_MEMORY);
}

static void
stm8_temporary_stack_save_test ()
{
  /* LD A,#23 ends prologue recognition, but POP A restores the SP.  */
  const gdb_byte insns[] = { 0xa6, 23, 0x84, 0x90, 0x89,
			    0x4b, 42, 0x84, 0x90, 0x85, 0x81 };
  auto reader = [&] (CORE_ADDR addr, gdb_byte *buf, int len)
    {
      if (addr + len > sizeof (insns))
	return -1;
      memcpy (buf, insns + addr, len);
      return 0;
    };
  struct stm8_prologue p;
  for (CORE_ADDR stop : { 2, 3, 5, 7, 8, 10, 11 })
    {
      stm8_init_prologue (&p);
      p.frame_size = 1;
      p.saved_entry_offset[STM8_A_REGNUM] = 0;
      stm8_analyze_stack_changes (0, stop, &p, reader);
      int expected = stop == 2 ? 1 : stop == 5 || stop == 8 ? 2
				 : stop == 7 ? 3 : 0;
      SELF_CHECK (p.frame_size == expected);
      SELF_CHECK (p.saved_entry_offset[STM8_A_REGNUM]
		  == (stop == 2 ? 0 : STM8_REG_UNSAVED));
    }

  /* Do not interpret bytes on the other side of a branch as executed.  */
  const gdb_byte branch[] = { 0x20, 0, 0x84 };
  auto branch_reader = [&] (CORE_ADDR addr, gdb_byte *buf, int len)
    {
      if (addr + len > sizeof (branch))
	return -1;
      memcpy (buf, branch + addr, len);
      return 0;
    };
  stm8_init_prologue (&p);
  p.frame_size = 1;
  stm8_analyze_stack_changes (0, sizeof (branch), &p, branch_reader);
  SELF_CHECK (p.frame_size == 1);
}

static void
stm8_return_frame_size_test ()
{
  SELF_CHECK (stm8_return_address_size (STM8_RETURN_RET) == 2);
  SELF_CHECK (stm8_return_address_size (STM8_RETURN_RETF) == 3);
  SELF_CHECK (stm8_return_address_size (STM8_RETURN_IRET) == 9);
}

static void
stm8_register_name_test ()
{
  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_PC_REGNUM), "pc")
	      == 0);
  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_A_REGNUM), "a") == 0);
  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_X_REGNUM), "x") == 0);
  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_Y_REGNUM), "y") == 0);
  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_SP_REGNUM), "sp")
	      == 0);
  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_CC_REGNUM), "cc")
	      == 0);

  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_XH_REGNUM), "xh")
	      == 0);
  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_XL_REGNUM), "xl")
	      == 0);
  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_YH_REGNUM), "yh")
	      == 0);
  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_YL_REGNUM), "yl")
	      == 0);
}

} /* namespace selftests.  */

#endif /* GDB_SELF_TEST.  */

INIT_GDB_FILE (stm8_tdep)
{
  gdbarch_register (bfd_arch_stm8, stm8_gdbarch_init);

#if GDB_SELF_TEST
  selftests::register_test ("stm8-last-opcode",
			    selftests::stm8_last_opcode_test);
  selftests::register_test ("stm8-argument-registers",
			    selftests::stm8_argument_register_test);
  selftests::register_test ("stm8-pseudo-register-mapping",
			    selftests::stm8_pseudo_register_mapping_test);
  selftests::register_test ("stm8-dwarf-register-mapping",
			    selftests::stm8_dwarf_register_mapping_test);
  selftests::register_test ("stm8-return-value-layout",
			    selftests::stm8_return_value_layout_test);
  selftests::register_test ("stm8-temporary-stack-save",
			    selftests::stm8_temporary_stack_save_test);
  selftests::register_test ("stm8-return-frame-size",
			    selftests::stm8_return_frame_size_test);
  selftests::register_test ("stm8-register-names",
			    selftests::stm8_register_name_test);
#endif
}
