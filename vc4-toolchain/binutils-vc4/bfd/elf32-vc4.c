/* VC4-specific support for 32-bit ELF.
   Copyright 2012
   Free Software Foundation, Inc.
   Contributed by Mark Marshall, markmarshall14@gmail.com

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
#include "elf-bfd.h"
#include "elf/vc4.h"
#include "libiberty.h"
#include "safe-ctype.h"
#include <assert.h>

#define DEBUGn(X, ...)
#define DEBUG(X, ...)

static bfd_reloc_status_type
vc4_elf_reloc (bfd *abfd ATTRIBUTE_UNUSED,
	       arelent *reloc_entry,
	       asymbol *symbol ATTRIBUTE_UNUSED,
	       void *data ATTRIBUTE_UNUSED,
	       asection *input_section,
	       bfd *output_bfd,
	       char **error_message ATTRIBUTE_UNUSED);

static reloc_howto_type vc4_elf_howto_table[] =
{
  /* This reloc does nothing.  */
  HOWTO (R_VC4_NONE,	/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VC4_NONE",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A PC relative 7 bit relocation.  */
  HOWTO (R_VC4_PCREL7_MUL2,	/* type */
	 1,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 7,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VC4_PCREL7",	/* name */
	 FALSE,			/* partial_inplace */
	 0x007f,		/* src_mask */
	 0x007f,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* A PC relative 8 bit relocation.  */
  HOWTO (R_VC4_PCREL8_MUL2,      /* type */
	 1,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 TRUE,			/* pc_relative */
	 16,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VC4_PCREL8",        /* name */
	 FALSE,			/* partial_inplace */
	 0x00ff0000,		/* src_mask */
	 0x00ff0000,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* A PC relative 10 bit relocation.  */
  HOWTO (R_VC4_PCREL10_MUL2,   /* type */
	 1,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 10,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VC4_PCREL10",      /* name */
	 FALSE,			/* partial_inplace */
	 0x03ff0000,		/* src_mask */
	 0x03ff0000,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* A PC relative 16 bit relocation.  */
  HOWTO (R_VC4_PCREL16,         /* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 TRUE,			/* pc_relative */
	 16,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 vc4_elf_reloc,		/* special_function */
	 "R_VC4_PCREL16",       /* name */
	 FALSE,			/* partial_inplace */
	 0x0000,		/* src_mask */
	 0xffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* A PC relative 23 bit relocation.  */
  HOWTO (R_VC4_PCREL23_MUL2,    /* type */
	 1,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 23,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VC4_PCREL23_MUL2",  /* name */
	 FALSE,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0x007fffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* A PC relative 27 bit relocation.  */
  HOWTO (R_VC4_PCREL27,        /* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 27,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VC4_PCREL27",      /* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x07ffffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* A PC relative 27 bit relocation.  */
  HOWTO (R_VC4_PCREL27_MUL2,   /* type */
	 1,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 27,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VC4_PCREL27_MUL2",  /* name */
	 FALSE,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0x07ffffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* A PC relative 32 bit relocation.  */
  HOWTO (R_VC4_PCREL32,        /* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_VC4_PCREL32",	/* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 TRUE),			/* pcrel_offset */

  /* A absolute 5 bit relocation.  */
  HOWTO (R_VC4_IMM5_MUL4,       /* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 5,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 vc4_elf_reloc,		/* special_function */
	 "R_VC4_IMM5_MUL4",     /* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A absolute 5 bit relocation.  */
  HOWTO (R_VC4_IMM5_1,          /* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 5,			/* bitsize */
	 FALSE,			/* pc_relative */
	 4,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VC4_IMM5_1",        /* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x0000001f,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A absolute 5 bit relocation.  */
  HOWTO (R_VC4_IMM5_2,         /* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 5,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 vc4_elf_reloc,		/* special_function */
	 "R_VC4_IMM5_2",        /* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A absolute 6 bit relocation.  */
  HOWTO (R_VC4_IMM6,           /* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 6,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 vc4_elf_reloc,		/* special_function */
	 "R_VC4_IMM6",         /* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A absolute 6 bit relocation.  */
  HOWTO (R_VC4_IMM6_MUL4,      /* type */
	 2,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 6,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 vc4_elf_reloc,		/* special_function */
	 "R_VC4_IMM6_MUL4",     /* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A absolute 11 bit relocation.  */
  HOWTO (R_VC4_IMM11,          /* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 11,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 vc4_elf_reloc,		/* special_function */
	 "R_VC4_IMM11",        /* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A absolute 12 bit relocation.  */
  HOWTO (R_VC4_IMM12,          /* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 12,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 vc4_elf_reloc,		/* special_function */
	 "R_VC4_IMM12",         /* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A absolute 16 bit relocation.  */
  HOWTO (R_VC4_IMM16,           /* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VC4_IMM16",         /* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A absolute 23 bit relocation.  */
  HOWTO (R_VC4_IMM23,           /* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 23,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 vc4_elf_reloc,		/* special_function */
	 "R_VC4_IMM23",         /* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A absolute 27 bit relocation.  */
  HOWTO (R_VC4_IMM27,           /* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 27,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 vc4_elf_reloc,		/* special_function */
	 "R_VC4_IMM27",         /* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A absolute 32 bit relocation.  */
  HOWTO (R_VC4_IMM32,           /* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 16,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VC4_IMM32",         /* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A absolute 32 bit relocation.  */
  HOWTO (R_VC4_IMM32_2,         /* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 vc4_elf_reloc,		/* special_function */
	 "R_VC4_IMM32_2",       /* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A absolute 8 bit relocation.  */
  HOWTO (R_VC4_8,               /* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VC4_8",             /* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x000000ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A absolute 16 bit relocation.  */
  HOWTO (R_VC4_16,              /* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VC4_16",            /* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A absolute 32 bit relocation.  */
  HOWTO (R_VC4_32,              /* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VC4_32",            /* name */
	 FALSE,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

};

struct vc4_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned char elf_reloc_val;
};

static const struct vc4_reloc_map vc4_reloc_map[] =
{
  { BFD_RELOC_NONE, R_VC4_NONE },
  { BFD_RELOC_VC4_REL7_MUL2, R_VC4_PCREL7_MUL2 },
  { BFD_RELOC_VC4_REL8_MUL2, R_VC4_PCREL8_MUL2 },
  { BFD_RELOC_VC4_REL10_MUL2, R_VC4_PCREL10_MUL2 },
  { BFD_RELOC_VC4_REL16, R_VC4_PCREL16 },
  { BFD_RELOC_VC4_REL23_MUL2, R_VC4_PCREL23_MUL2 },
  { BFD_RELOC_VC4_REL27, R_VC4_PCREL27 },
  { BFD_RELOC_VC4_REL27_MUL2, R_VC4_PCREL27_MUL2 },
  { BFD_RELOC_VC4_REL32, R_VC4_PCREL32 },
  { BFD_RELOC_VC4_IMM5_MUL4, R_VC4_IMM5_MUL4 },
  { BFD_RELOC_VC4_IMM5_1, R_VC4_IMM5_1 },
  { BFD_RELOC_VC4_IMM5_2, R_VC4_IMM5_2 },
  { BFD_RELOC_VC4_IMM6, R_VC4_IMM6 },
  { BFD_RELOC_VC4_IMM6_MUL4, R_VC4_IMM6_MUL4 },
  { BFD_RELOC_VC4_IMM11, R_VC4_IMM11 },
  { BFD_RELOC_VC4_IMM12, R_VC4_IMM12 },
  { BFD_RELOC_VC4_IMM16, R_VC4_IMM16 },
  { BFD_RELOC_VC4_IMM23, R_VC4_IMM23 },
  { BFD_RELOC_VC4_IMM27, R_VC4_IMM27 },
  { BFD_RELOC_VC4_IMM32, R_VC4_IMM32 },
  { BFD_RELOC_VC4_IMM32_2, R_VC4_IMM32_2 },
  { BFD_RELOC_8, R_VC4_8 },
  { BFD_RELOC_16, R_VC4_16 },
  { BFD_RELOC_32, R_VC4_32 }
};

static reloc_howto_type *
vc4_reloc_type_lookup (bfd * abfd ATTRIBUTE_UNUSED,
		       bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (vc4_reloc_map); i++)
    if (vc4_reloc_map[i].bfd_reloc_val == code)
      return &vc4_elf_howto_table[vc4_reloc_map[i].elf_reloc_val];

  return NULL;
}

static reloc_howto_type *
vc4_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
		       const char *r_name)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (vc4_elf_howto_table); i++)
    if (vc4_elf_howto_table[i].name != NULL
	&& strcasecmp (vc4_elf_howto_table[i].name, r_name) == 0)
      return &vc4_elf_howto_table[i];

  return NULL;
}

/* Set the howto pointer for an Vc4 ELF reloc.  */

static void
vc4_info_to_howto_rela (bfd * abfd ATTRIBUTE_UNUSED,
			arelent * cache_ptr,
			Elf_Internal_Rela * dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  BFD_ASSERT (r_type < (unsigned int) R_VC4_max);
  cache_ptr->howto = &vc4_elf_howto_table[r_type];
}

static bfd_reloc_status_type
vc4_final_link_relocate (reloc_howto_type *howto,
			 bfd *input_bfd ATTRIBUTE_UNUSED,
			 asection *input_section,
			 bfd_byte *contents,
			 Elf_Internal_Rela *rel,
			 bfd_vma relocation)
{
  bfd_vma pc;
  long s;
  unsigned long u;
  unsigned char *byte;
  bfd_reloc_status_type r = bfd_reloc_ok;

  pc = input_section->output_section->vma
       + input_section->output_offset
       + rel->r_offset;

  s = relocation + rel->r_addend;

  byte = (unsigned char *)contents + rel->r_offset;

  if (howto->pc_relative)
    s -= pc;

  u = (unsigned long) s;

  switch (howto->type)
    {
    case R_VC4_PCREL27:
      if (s < -0x4000000 || s >= 0x4000000)
	r = bfd_reloc_overflow;
      byte[2] = u & 0xff;
      byte[3] = (u >> 8) & 0xff;
      byte[4] = (u >> 16) & 0xff;
      byte[5] = (byte[5] & 0xf8) | ((u >> 24) & 0x7);
      break;

    case R_VC4_PCREL27_MUL2:
      if (s < -0x8000000 || s >= 0x8000000 || (s & 1) != 0)
        r = bfd_reloc_overflow;
      byte[2] = (u >> 1) & 0xff;
      byte[3] = (u >> 9) & 0xff;
      byte[0] = (byte[0] & 0x80) | ((u >> 17) & 0x7f);
      byte[1] = (byte[1] & 0xf0) | ((u >> 24) & 0xf);
      break;

    case R_VC4_PCREL23_MUL2:
      if (s < -0x800000 || s >= 0x800000 || (s & 1) != 0)
        r = bfd_reloc_overflow;
      byte[2] = (u >> 1) & 0xff;
      byte[3] = (u >> 9) & 0xff;
      byte[0] = (u >> 17) & 0x7f;
      break;

    case R_VC4_IMM5_1:
      if (u >= 0x20)
        r = bfd_reloc_overflow;
      byte[0] = (byte[0] & 0x0f) | ((u & 0x0f) << 4);
      byte[1] = (byte[1] & 0xfe) | ((u >> 4) & 1);
      break;

    case R_VC4_IMM32:
    case R_VC4_PCREL32:
      byte[2] = u & 0xff;
      byte[3] = (u >> 8) & 0xff;
      byte[4] = (u >> 16) & 0xff;
      byte[5] = (u >> 24) & 0xff;
      break;

    case R_VC4_32:
      byte[3] = (u >> 24) & 0xff;
      byte[2] = (u >> 16) & 0xff;
      /* Fallthrough.  */

    case R_VC4_16:
      byte[1] = (u >> 8) & 0xff;
      /* Fallthrough.  */

    case R_VC4_8:
      byte[0] = u & 0xff;
      break;

    default:
      abort ();
    }

  return r;
}

/* Relocate a VC4 ELF section.
   There is some attempt to make this function usable for many architectures,
   both USE_REL and USE_RELA ['twould be nice if such a critter existed],
   if only to serve as a learning tool.

   The RELOCATE_SECTION function is called by the new ELF backend linker
   to handle the relocations for a section.

   The relocs are always passed as Rela structures; if the section
   actually uses Rel structures, the r_addend field will always be
   zero.

   This function is responsible for adjusting the section contents as
   necessary, and (if using Rela relocs and generating a relocatable
   output file) adjusting the reloc addend as necessary.

   This function does not have to worry about setting the reloc
   address or the reloc symbol index.

   LOCAL_SYMS is a pointer to the swapped in local symbols.

   LOCAL_SECTIONS is an array giving the section in the input file
   corresponding to the st_shndx field of each local symbol.

   The global hash table entry for the global symbols can be found
   via elf_sym_hashes (input_bfd).

   When generating relocatable output, this function must handle
   STB_LOCAL/STT_SECTION symbols specially.  The output symbol is
   going to be the section symbol corresponding to the output
   section, which means that the addend must be adjusted
   accordingly.  */

/* This is based on mep_elf_relocate_section, but RX, M32C, Microblaze,
   IQ2000, MT and RL78 also appear to be derived from the same origin.  */

static bfd_boolean
vc4_elf_relocate_section (bfd *output_bfd,
			  struct bfd_link_info *info,
			  bfd *input_bfd,
			  asection *input_section,
			  bfd_byte *contents,
			  Elf_Internal_Rela *relocs,
			  Elf_Internal_Sym *local_syms,
			  asection **local_sections)
{
  Elf_Internal_Shdr *           symtab_hdr;
  struct elf_link_hash_entry ** sym_hashes;
  Elf_Internal_Rela *           rel;
  Elf_Internal_Rela *           relend;

  symtab_hdr = & elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend     = relocs + input_section->reloc_count;

  for (rel = relocs; rel < relend; rel ++)
    {
      reloc_howto_type *           howto;
      unsigned long                r_symndx;
      Elf_Internal_Sym *           sym;
      asection *                   sec;
      struct elf_link_hash_entry * h;
      bfd_vma                      relocation;
      bfd_reloc_status_type        r;
      const char *                 name = NULL;
      /*int                          r_type;

      r_type = ELF32_R_TYPE (rel->r_info);*/
      r_symndx = ELF32_R_SYM (rel->r_info);
      howto  = vc4_elf_howto_table + ELF32_R_TYPE (rel->r_info);
      h      = NULL;
      sym    = NULL;
      sec    = NULL;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections [r_symndx];
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);

	  name = bfd_elf_string_from_elf_section
	    (input_bfd, symtab_hdr->sh_link, sym->st_name);
	  name = (name == NULL) ? bfd_section_name (input_bfd, sec) : name;
	}
      else
	{
	  bfd_boolean warned, unresolved_reloc;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned);

	  name = h->root.root.string;
	}

      if (sec != NULL && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      if (info->relocatable)
	continue;

      r = vc4_final_link_relocate (howto, input_bfd, input_section,
				   contents, rel, relocation);

      if (r != bfd_reloc_ok)
	{
	  const char * msg = (const char *) NULL;

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      r = info->callbacks->reloc_overflow
		(info, (h ? &h->root : NULL), name, howto->name, (bfd_vma) 0,
		 input_bfd, input_section, rel->r_offset);
	      break;

	    case bfd_reloc_undefined:
	      r = info->callbacks->undefined_symbol
		(info, name, input_bfd, input_section, rel->r_offset, TRUE);
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

	  if (msg)
	    r = info->callbacks->warning
	      (info, msg, name, input_bfd, input_section, rel->r_offset);

	  if (! r)
	    return FALSE;
	}
    }

  return TRUE;
}

static bfd_reloc_status_type
vc4_elf_reloc (bfd *abfd ATTRIBUTE_UNUSED,
	       arelent *reloc_entry ATTRIBUTE_UNUSED,
	       asymbol *symbol ATTRIBUTE_UNUSED,
	       void *data ATTRIBUTE_UNUSED,
	       asection *input_section ATTRIBUTE_UNUSED,
	       bfd *output_bfd ATTRIBUTE_UNUSED,
	       char **error_message ATTRIBUTE_UNUSED)
{
  fprintf (stderr, "vc4_elf_reloc special function\n");
  return bfd_reloc_notsupported;
}

/* Return the section that should be marked against GC for a given
   relocation.  */

static asection *
vc4_elf_gc_mark_hook (asection *sec,
		      struct bfd_link_info *info,
		      Elf_Internal_Rela *rel,
		      struct elf_link_hash_entry *h,
		      Elf_Internal_Sym *sym)
{
  if (h != NULL)
    switch (ELF32_R_TYPE (rel->r_info))
      {
      case R_VC4_GNU_VTINHERIT:
      case R_VC4_GNU_VTENTRY:
	return NULL;
      }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* Look through the relocs for a section during the first phase.
   Since we don't do .gots or .plts, we just need to consider the
   virtual table relocs for gc.  */

static bfd_boolean
vc4_elf_check_relocs (bfd *abfd,
		      struct bfd_link_info *info,
		      asection *sec,
		      const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;

  if (info->relocatable)
    return TRUE;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;

      r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx < symtab_hdr->sh_info)
	h = NULL;
      else
	{
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}

      switch (ELF32_R_TYPE (rel->r_info))
	{
	  /* This relocation describes the C++ object vtable hierarchy.
	     Reconstruct it for later use during GC.  */
	case R_VC4_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return FALSE;
	  break;

	  /* This relocation describes which C++ vtable entries are actually
	     used.  Record for later use during GC.  */
	case R_VC4_GNU_VTENTRY:
	  BFD_ASSERT (h != NULL);
	  if (h != NULL
	      && !bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return FALSE;
	  break;
	}
    }

  return TRUE;
}

/* Set the right machine number.  */

static bfd_boolean
vc4_elf_object_p (bfd *abfd)
{
  bfd_default_set_arch_mach (abfd, bfd_arch_vc4, bfd_mach_vc4);
  return TRUE;
}

/* Store the machine number in the flags field.  */

static void
vc4_elf_final_write_processing (bfd *abfd,
				bfd_boolean linker ATTRIBUTE_UNUSED)
{
  elf_elfheader (abfd)->e_flags &= ~0xf;
  elf_elfheader (abfd)->e_flags |= 0;
}


#define ELF_ARCH			bfd_arch_vc4
#define ELF_MACHINE_CODE		EM_VIDEOCORE3
#define ELF_MACHINE_ALT1		EM_VIDEOCORE
#define ELF_MAXPAGESIZE			0x1000

#define TARGET_LITTLE_SYM		bfd_elf32_vc4_vec
#define TARGET_LITTLE_NAME		"elf32-vc4"

#define elf_info_to_howto_rel		NULL
#define elf_info_to_howto		vc4_info_to_howto_rela
#define elf_backend_relocate_section	vc4_elf_relocate_section
#define elf_backend_gc_mark_hook	vc4_elf_gc_mark_hook
#define elf_backend_check_relocs	vc4_elf_check_relocs

#define elf_backend_can_gc_sections	1
#define elf_backend_rela_normal		1

#define bfd_elf32_bfd_reloc_type_lookup vc4_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup vc4_reloc_name_lookup

#define elf_backend_object_p                vc4_elf_object_p
#define elf_backend_final_write_processing  vc4_elf_final_write_processing

#include "elf32-target.h"
