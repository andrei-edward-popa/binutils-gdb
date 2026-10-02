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
#include "dis-asm.h"
#include "gdbarch.h"
#include "gdbtypes.h"
#include "objfiles.h"
#include "progspace.h"
#include "regcache.h"
#include "target-descriptions.h"

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

  if (tdesc_data != nullptr)
    {
      tdesc_use_registers (gdbarch, tdesc, std::move (tdesc_data), nullptr);

      /* A target description using code_ptr for PC would otherwise inherit
	 ptr_bit (16).  The remote PC is a 32-bit transport container.  */
      set_gdbarch_register_type (gdbarch, stm8_register_type);
    }

  return gdbarch;
}

INIT_GDB_FILE (stm8_tdep)
{
  gdbarch_register (bfd_arch_stm8, stm8_gdbarch_init);
}
