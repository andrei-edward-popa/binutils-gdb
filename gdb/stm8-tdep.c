/* Target-dependent code for STM8, for GDB.

   Copyright (C) 1996-2026 Free Software Foundation, Inc.

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
#include "objfiles.h"
#include "progspace.h"
#include "regcache.h"
#include "target-descriptions.h"
#include "target.h"
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

  /* SDCC exposes the byte halves of X and Y in DWARF.  */
  STM8_XH_REGNUM,
  STM8_XL_REGNUM,
  STM8_YH_REGNUM,
  STM8_YL_REGNUM
};

static constexpr int STM8_NUM_REGS = 6;
static constexpr int STM8_NUM_PSEUDO_REGS = 4;

static const char *const stm8_register_names[STM8_NUM_REGS]
  = { "pc", "a", "x", "y", "sp", "cc" };

static const int stm8_register_bits[STM8_NUM_REGS]
  = { 32, 8, 16, 16, 16, 8 };

enum stm8_producer
{
  STM8_PRODUCER_GCC,
  STM8_PRODUCER_SDCC
};

struct stm8_gdbarch_tdep : gdbarch_tdep_base
{
};

/* Return the compiler which produced the currently loaded debug
   information.  SDCC uses byte pseudo-registers in its DWARF register
   numbering, while GCC uses the word X/Y registers.  */

static enum stm8_producer
stm8_get_producer ()
{
  if (current_program_space != nullptr)
    for (auto &objfile : current_program_space->objfiles ())
      for (auto &cust : objfile.compunits ())
	if (cust.producer () != nullptr
	    && startswith (cust.producer (), "SDCC"))
	  return STM8_PRODUCER_SDCC;

  return STM8_PRODUCER_GCC;
}

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
stm8_pseudo_register_write (struct gdbarch *gdbarch,
			    struct regcache *regcache, int regnum,
			    const gdb_byte *buf)
{
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

/* DWARF register numberings used by the two STM8 compiler ports.

   SDCC describes the byte halves of X and Y separately as well as the
   complete word registers.  The GCC STM8 port uses only the complete
   registers.  */

static constexpr int stm8_dwarf_regmap_sdcc[] =
{
  STM8_A_REGNUM,
  STM8_XL_REGNUM,
  STM8_XH_REGNUM,
  STM8_YL_REGNUM,
  STM8_YH_REGNUM,
  STM8_CC_REGNUM,
  STM8_X_REGNUM,
  STM8_Y_REGNUM,
  STM8_SP_REGNUM,
  STM8_PC_REGNUM
};

static constexpr int stm8_dwarf_regmap_gcc[] =
{
  STM8_A_REGNUM,
  STM8_X_REGNUM,
  STM8_Y_REGNUM,
  STM8_SP_REGNUM
};

static int
stm8_dwarf2_reg_to_regnum_for_producer (enum stm8_producer producer,
				       int reg)
{
  if (reg < 0)
    return -1;

  if (producer == STM8_PRODUCER_SDCC)
    {
      if ((unsigned int) reg < ARRAY_SIZE (stm8_dwarf_regmap_sdcc))
	return stm8_dwarf_regmap_sdcc[reg];
    }
  else if ((unsigned int) reg < ARRAY_SIZE (stm8_dwarf_regmap_gcc))
    return stm8_dwarf_regmap_gcc[reg];

  return -1;
}

static int
stm8_dwarf2_reg_to_regnum (struct gdbarch *gdbarch, int reg)
{
  return stm8_dwarf2_reg_to_regnum_for_producer (stm8_get_producer (), reg);
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
  for (int regnum = 0; regnum < STM8_NUM_REGS; ++regnum)
    p->saved_entry_offset[regnum] = STM8_REG_UNSAVED;
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
	case 0x3b:              /* PUSH longmem.  */
	  ++depth;
	  length = 3;
	  break;

	case 0x52:              /* SUB SP,#imm8.  */
	  depth += buf[1];
	  length = 2;
	  break;

	case 0x88:              /* PUSH A.  */
	  p->saved_entry_offset[STM8_A_REGNUM] = -depth;
	  ++depth;
	  length = 1;
	  break;

	case 0x89:              /* PUSHW X.  */
	  p->saved_entry_offset[STM8_X_REGNUM] = -(depth + 1);
	  depth += 2;
	  length = 1;
	  break;

	case 0x8a:              /* PUSH CC.  */
	  p->saved_entry_offset[STM8_CC_REGNUM] = -depth;
	  ++depth;
	  length = 1;
	  break;

	case 0x9089:            /* PUSHW Y.  */
	  p->saved_entry_offset[STM8_Y_REGNUM] = -(depth + 1);
	  depth += 2;
	  length = 2;
	  break;

	case 0x9096:            /* LDW Y,SP.  */
	  length = 2;
	  break;

	case 0x90cf:            /* LDW longmem,Y.  */
	  length = 4;
	  break;

	default:
	  p->frame_size = depth;
	  p->prologue_end = pc;
	  return;
	}

      pc += length;
      p->prologue_end = pc;
    }

  p->frame_size = depth;
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
      && target_read_code (func_end - 1, &opcode, 1) == 0)
    {
      if (opcode == 0x87)
	return STM8_RETURN_RETF;
      if (opcode == 0x80)
	return STM8_RETURN_IRET;
    }

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
  cache->caller_sp
    = cache->entry_sp + stm8_return_address_size (cache->return_kind);

  return cache;
}

static void
stm8_frame_this_id (const frame_info_ptr &this_frame, void **this_cache,
		    struct frame_id *this_id)
{
  struct stm8_frame_cache *cache = stm8_get_frame_cache (this_frame, this_cache);

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

  *value = extract_unsigned_integer (buf, size,
				     gdbarch_byte_order (gdbarch));
  return true;
}

static struct value *
stm8_frame_prev_register (const frame_info_ptr &this_frame,
			  void **this_cache, int regnum)
{
  struct stm8_frame_cache *cache = stm8_get_frame_cache (this_frame, this_cache);
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
    return frame_unwind_got_memory
      (this_frame, regnum,
       cache->entry_sp + cache->prologue.saved_entry_offset[regnum]);

  return frame_unwind_got_register (this_frame, regnum, regnum);
}

static const struct frame_unwind_legacy stm8_frame_unwind (
  "stm8 prologue",
  NORMAL_FRAME,
  FRAME_UNWIND_ARCH,
  default_frame_unwind_stop_reason,
  stm8_frame_this_id,
  stm8_frame_prev_register,
  nullptr,
  default_frame_sniffer
);

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

  arches = gdbarch_list_lookup_by_info (arches, &info);
  if (arches != nullptr)
    return arches->gdbarch;

  gdbarch *gdbarch
    = gdbarch_alloc (&info, gdbarch_tdep_up (new stm8_gdbarch_tdep));

  set_gdbarch_num_regs (gdbarch, STM8_NUM_REGS);
  set_gdbarch_num_pseudo_regs (gdbarch, STM8_NUM_PSEUDO_REGS);
  set_gdbarch_register_name (gdbarch, stm8_register_name);
  set_gdbarch_register_type (gdbarch, stm8_register_type);
  set_gdbarch_pseudo_register_read (gdbarch, stm8_pseudo_register_read);
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
  set_gdbarch_ptr_bit (gdbarch, 16);
  set_gdbarch_addr_bit (gdbarch, 32);
  set_gdbarch_dwarf2_addr_size (gdbarch, 4);

  set_gdbarch_inner_than (gdbarch, core_addr_lessthan);
  set_gdbarch_breakpoint_kind_from_pc (gdbarch,
				       stm8_breakpoint::kind_from_pc);
  set_gdbarch_sw_breakpoint_from_kind (gdbarch,
				       stm8_breakpoint::bp_from_kind);
  set_gdbarch_print_insn (gdbarch, print_insn_stm8);
  set_gdbarch_return_value (gdbarch, stm8_return_value);

  set_gdbarch_skip_prologue (gdbarch, stm8_skip_prologue);
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

namespace selftests {

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
  const int sdcc_expected[] =
    {
      STM8_A_REGNUM, STM8_XL_REGNUM, STM8_XH_REGNUM, STM8_YL_REGNUM,
      STM8_YH_REGNUM, STM8_CC_REGNUM, STM8_X_REGNUM, STM8_Y_REGNUM,
      STM8_SP_REGNUM, STM8_PC_REGNUM
    };
  const int gcc_expected[] =
    {
      STM8_A_REGNUM, STM8_X_REGNUM, STM8_Y_REGNUM, STM8_SP_REGNUM
    };

  for (unsigned int i = 0; i < ARRAY_SIZE (sdcc_expected); ++i)
    SELF_CHECK (stm8_dwarf2_reg_to_regnum_for_producer
		(STM8_PRODUCER_SDCC, i) == sdcc_expected[i]);

  for (unsigned int i = 0; i < ARRAY_SIZE (gcc_expected); ++i)
    SELF_CHECK (stm8_dwarf2_reg_to_regnum_for_producer
		(STM8_PRODUCER_GCC, i) == gcc_expected[i]);

  SELF_CHECK (stm8_dwarf2_reg_to_regnum_for_producer
	      (STM8_PRODUCER_SDCC, -1) == -1);
  SELF_CHECK (stm8_dwarf2_reg_to_regnum_for_producer
	      (STM8_PRODUCER_SDCC, ARRAY_SIZE (sdcc_expected)) == -1);
  SELF_CHECK (stm8_dwarf2_reg_to_regnum_for_producer
	      (STM8_PRODUCER_GCC, ARRAY_SIZE (gcc_expected)) == -1);
}

static void
stm8_return_value_layout_test ()
{
  SELF_CHECK (stm8_return_value_layout_for_size (1) == STM8_RETURN_VALUE_A);
  SELF_CHECK (stm8_return_value_layout_for_size (2) == STM8_RETURN_VALUE_X);
  SELF_CHECK (stm8_return_value_layout_for_size (3)
	      == STM8_RETURN_VALUE_YL_X);
  SELF_CHECK (stm8_return_value_layout_for_size (4) == STM8_RETURN_VALUE_Y_X);
  SELF_CHECK (stm8_return_value_layout_for_size (5)
	      == STM8_RETURN_VALUE_MEMORY);
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
  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_PC_REGNUM),
		      "pc") == 0);
  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_A_REGNUM),
		      "a") == 0);
  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_X_REGNUM),
		      "x") == 0);
  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_Y_REGNUM),
		      "y") == 0);
  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_SP_REGNUM),
		      "sp") == 0);
  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_CC_REGNUM),
		      "cc") == 0);

  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_XH_REGNUM),
		      "xh") == 0);
  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_XL_REGNUM),
		      "xl") == 0);
  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_YH_REGNUM),
		      "yh") == 0);
  SELF_CHECK (strcmp (stm8_register_name (nullptr, STM8_YL_REGNUM),
		      "yl") == 0);
}

} /* namespace selftests */

#endif /* GDB_SELF_TEST */

INIT_GDB_FILE (stm8_tdep)
{
  gdbarch_register (bfd_arch_stm8, stm8_gdbarch_init);

#if GDB_SELF_TEST
  selftests::register_test ("stm8-pseudo-register-mapping",
			    selftests::stm8_pseudo_register_mapping_test);
  selftests::register_test ("stm8-dwarf-register-mapping",
			    selftests::stm8_dwarf_register_mapping_test);
  selftests::register_test ("stm8-return-value-layout",
			    selftests::stm8_return_value_layout_test);
  selftests::register_test ("stm8-return-frame-size",
			    selftests::stm8_return_frame_size_test);
  selftests::register_test ("stm8-register-names",
			    selftests::stm8_register_name_test);
#endif
}
