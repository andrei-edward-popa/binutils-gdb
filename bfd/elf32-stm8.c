/* STM8-specific support for 32-bit ELF.
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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "libiberty.h"
#include "elf-bfd.h"
#include "elf/stm8.h"

static reloc_howto_type elf32_stm8_howto_table[] =
{
  HOWTO (R_STM8_NONE, 0, 0, 0, false, 0, complain_overflow_dont,
	 bfd_elf_generic_reloc, "R_STM8_NONE", false, 0, 0, false),
  HOWTO (R_STM8_8, 0, 1, 8, false, 0, complain_overflow_unsigned,
	 bfd_elf_generic_reloc, "R_STM8_8", false, 0, 0xff, false),
  HOWTO (R_STM8_16, 0, 2, 16, false, 0, complain_overflow_unsigned,
	 bfd_elf_generic_reloc, "R_STM8_16", false, 0, 0xffff, false),
  HOWTO (R_STM8_24, 0, 3, 24, false, 0, complain_overflow_unsigned,
	 bfd_elf_generic_reloc, "R_STM8_24", false, 0, 0x00ffffff, false),
  HOWTO (R_STM8_32, 0, 4, 32, false, 0, complain_overflow_unsigned,
	 bfd_elf_generic_reloc, "R_STM8_32", false, 0, 0xffffffff, false),
  HOWTO (R_STM8_8_PCREL, 0, 1, 8, true, 0, complain_overflow_signed,
	 bfd_elf_generic_reloc, "R_STM8_8_PCREL", false, 0, 0xff, true),
  HOWTO (R_STM8_HI8, 8, 1, 8, false, 0, complain_overflow_dont,
	 bfd_elf_generic_reloc, "R_STM8_HI8", false, 0, 0xff, false),
  HOWTO (R_STM8_LO8, 0, 1, 8, false, 0, complain_overflow_dont,
	 bfd_elf_generic_reloc, "R_STM8_LO8", false, 0, 0xff, false),
  HOWTO (R_STM8_HH8, 16, 1, 8, false, 0, complain_overflow_dont,
	 bfd_elf_generic_reloc, "R_STM8_HH8", false, 0, 0xff, false),
};

struct elf32_stm8_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned int elf_reloc_val;
};

static const struct elf32_stm8_reloc_map elf32_stm8_reloc_map[] =
{
  { BFD_RELOC_NONE, R_STM8_NONE },
  { BFD_RELOC_8, R_STM8_8 },
  { BFD_RELOC_16, R_STM8_16 },
  { BFD_RELOC_24, R_STM8_24 },
  { BFD_RELOC_32, R_STM8_32 },
  { BFD_RELOC_8_PCREL, R_STM8_8_PCREL },
  { BFD_RELOC_STM8_HI8, R_STM8_HI8 },
  { BFD_RELOC_STM8_LO8, R_STM8_LO8 },
  { BFD_RELOC_STM8_HH8, R_STM8_HH8 },
};

static reloc_howto_type *
elf32_stm8_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			      bfd_reloc_code_real_type code)
{
  for (size_t i = 0; i < ARRAY_SIZE (elf32_stm8_reloc_map); ++i)
    if (elf32_stm8_reloc_map[i].bfd_reloc_val == code)
      return &elf32_stm8_howto_table[elf32_stm8_reloc_map[i].elf_reloc_val];

  return NULL;
}

static reloc_howto_type *
elf32_stm8_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED, const char *r_name)
{
  for (size_t i = 0; i < ARRAY_SIZE (elf32_stm8_howto_table); ++i)
    if (elf32_stm8_howto_table[i].name != NULL
	&& strcasecmp (elf32_stm8_howto_table[i].name, r_name) == 0)
      return &elf32_stm8_howto_table[i];

  return NULL;
}

static bool
elf32_stm8_info_to_howto_rela (bfd *abfd, arelent *cache_ptr,
				Elf_Internal_Rela *dst)
{
  unsigned int r_type = ELF32_R_TYPE (dst->r_info);

  if (r_type >= (unsigned int) R_STM8_max)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  cache_ptr->howto = &elf32_stm8_howto_table[r_type];
  return true;
}

static int
elf32_stm8_relocate_section (struct bfd_link_info *info,
			     bfd *input_bfd,
			     asection *input_section,
			     bfd_byte *contents,
			     Elf_Internal_Rela *relocs,
			     Elf_Internal_Sym *local_syms,
			     asection **local_sections)
{
  Elf_Internal_Shdr *symtab_hdr = &elf_symtab_hdr (input_bfd);
  struct elf_link_hash_entry **sym_hashes = elf_sym_hashes (input_bfd);
  Elf_Internal_Rela *relend = relocs + input_section->reloc_count;

  for (Elf_Internal_Rela *rel = relocs; rel < relend; ++rel)
    {
      unsigned int r_type = ELF32_R_TYPE (rel->r_info);
      unsigned long r_symndx = ELF32_R_SYM (rel->r_info);
      reloc_howto_type *howto;
      struct elf_link_hash_entry *h = NULL;
      Elf_Internal_Sym *sym = NULL;
      asection *sec = NULL;
      bfd_vma relocation;
      bfd_reloc_status_type r;
      const char *name;

      if (r_type >= (unsigned int) R_STM8_max)
	{
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			      input_bfd, r_type);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      howto = &elf32_stm8_howto_table[r_type];

      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  relocation = _bfd_elf_rela_local_sym (info->output_bfd,
						sym, &sec, rel);

	  name = bfd_elf_string_from_elf_section
	    (input_bfd, symtab_hdr->sh_link, sym->st_name);
	  name = name == NULL ? bfd_section_name (sec) : name;
	}
      else
	{
	  bool unresolved_reloc, warned, ignored;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned, ignored);

	  name = h->root.root.string;
	}

      if (sec != NULL && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, R_STM8_NONE,
					 howto, 0, contents);

      if (bfd_link_relocatable (info))
	continue;

      r = _bfd_final_link_relocate (howto, input_bfd, input_section,
				    contents, rel->r_offset,
				    relocation, rel->r_addend);

      if (r != bfd_reloc_ok)
	{
	  const char *msg = NULL;

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      (*info->callbacks->reloc_overflow)
		(info, h ? &h->root : NULL, name, howto->name,
		 (bfd_vma) 0, input_bfd, input_section, rel->r_offset);
	      break;

	    case bfd_reloc_undefined:
	      (*info->callbacks->undefined_symbol)
		(info, name, input_bfd, input_section, rel->r_offset, true);
	      break;

	    case bfd_reloc_outofrange:
	      msg = _("internal error: out of range error");
	      break;

	    case bfd_reloc_notsupported:
	      msg = _("internal error: unsupported relocation error");
	      break;

	    case bfd_reloc_dangerous:
	      msg = _("internal error: dangerous relocation");
	      break;

	    default:
	      msg = _("internal error: unknown error");
	      break;
	    }

	  if (msg != NULL)
	    (*info->callbacks->warning) (info, msg, name, input_bfd,
					 input_section, rel->r_offset);
	}
    }

  return true;
}

#define ELF_ARCH			bfd_arch_stm8
#define ELF_MACHINE_CODE		EM_STM8
#define ELF_MAXPAGESIZE			1

#define TARGET_BIG_SYM			stm8_elf32_vec
#define TARGET_BIG_NAME			"elf32-stm8"

#define elf_info_to_howto		elf32_stm8_info_to_howto_rela
#define elf_info_to_howto_rel		NULL
#define elf_backend_relocate_section	elf32_stm8_relocate_section
#define elf_backend_can_gc_sections	1
#define elf_backend_rela_normal		1

#define bfd_elf32_bfd_reloc_type_lookup	elf32_stm8_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup	elf32_stm8_reloc_name_lookup

#include "elf32-target.h"
