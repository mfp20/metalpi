/* Disassembler interface for targets using CGEN. -*- C -*-
   CGEN: Cpu tools GENerator

   THIS FILE IS MACHINE GENERATED WITH CGEN.
   - the resultant file is machine generated, cgen-dis.in isn't

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2005, 2007,
   2008, 2010  Free Software Foundation, Inc.

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
   along with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

/* ??? Eventually more and more of this stuff can go to cpu-independent files.
   Keep that in mind.  */

#include "sysdep.h"
#include <stdio.h>
#include "ansidecl.h"
#include "dis-asm.h"
#include "bfd.h"
#include "symcat.h"
#include "libiberty.h"
#include "vc4-desc.h"
#include "vc4-opc.h"
#include "opintl.h"

/* Default text to print if an instruction isn't recognized.  */
#define UNKNOWN_INSN_MSG _("*unknown*")

static void print_normal
  (CGEN_CPU_DESC, void *, long, unsigned int, bfd_vma, int);
static void print_address
  (CGEN_CPU_DESC, void *, bfd_vma, unsigned int, bfd_vma, int) ATTRIBUTE_UNUSED;
static void print_keyword
  (CGEN_CPU_DESC, void *, CGEN_KEYWORD *, long, unsigned int) ATTRIBUTE_UNUSED;
static void print_insn_normal
  (CGEN_CPU_DESC, void *, const CGEN_INSN *, CGEN_FIELDS *, bfd_vma, int);
static int print_insn
  (CGEN_CPU_DESC, bfd_vma,  disassemble_info *, bfd_byte *, unsigned);
static int default_print_insn
  (CGEN_CPU_DESC, bfd_vma, disassemble_info *) ATTRIBUTE_UNUSED;
static int read_insn
  (CGEN_CPU_DESC, bfd_vma, disassemble_info *, bfd_byte *, int, CGEN_EXTRACT_INFO *,
   unsigned long *);

/* -- disassembler routines inserted here.  */

/* -- dis.c */

#define CGEN_PRINT_INSN vc4_print_insn

typedef enum
{
  VC4_STANDARD_INSN,
  VC4_SWITCH_CASES
} vc4_disassemble_state;

typedef struct
{
  vc4_disassemble_state disassemble_state;
  int table_entry_size;
  bfd_vma base_address;
} vc4_private_data;

static int
vc4_print_insn (CGEN_CPU_DESC cd, bfd_vma pc, disassemble_info *info)
{
  bfd_byte buf[CGEN_MAX_INSN_SIZE];
  const int maxlen = 6;
  int buflen;
  int status;
  int insn;
  vc4_private_data *vc4_data = info->private_data;
  bfd_boolean symbol_here = (*info->symbol_at_address_func) (pc, info);

  if (info->private_data == NULL)
    vc4_data = info->private_data = calloc (sizeof (vc4_private_data), 1);

  info->bytes_per_line = buflen = maxlen;

  do
    {
      memset (buf, 0, 10);
      status = (*info->read_memory_func) (pc, buf, buflen, info);
      buflen -= 2;
    }
  while (status != 0 && buflen > 0);

  if (status != 0)
    {
      (*info->memory_error_func) (status, pc, info);
      return -1;
    }

  if (symbol_here
      && vc4_data->disassemble_state == VC4_SWITCH_CASES
      && pc > vc4_data->base_address)
    vc4_data->disassemble_state = VC4_STANDARD_INSN;

  switch (vc4_data->disassemble_state)
    {
    case VC4_STANDARD_INSN:
      {
        insn = buf[0] | (buf[1] << 8);

        if ((insn & 0xffc0) == 0x0080)
          {
            /* If there's a label right after this switch statement, start
               disassembling as switch cases instead of instructions.  This
               won't work for disassembling files without symbols!  */
            if ((*info->symbol_at_address_func) (pc + 2, info))
              vc4_data->disassemble_state = VC4_SWITCH_CASES;

            vc4_data->table_entry_size = (insn & 0x20) ? 2 : 1;
            vc4_data->base_address = pc + 2;
          }
      }
      break;

    case VC4_SWITCH_CASES:
      {
        bfd_vma target;
        bfd_signed_vma offset;

        switch (vc4_data->table_entry_size)
          {
          case 1:
            offset = buf[0];
            if (offset & 0x80)
              offset -= 0x100;
            break;
          case 2:
            offset = buf[0] | (buf[1] << 8);
            if (offset & 0x8000)
              offset -= 0x10000;
            break;
          default:
            abort ();
          }

        target = vc4_data->base_address + 2 * offset;

        info->fprintf_func (info->stream, ".case ");
        (*info->print_address_func) (target, info);

        return vc4_data->table_entry_size;
      }
      break;
    }

  return print_insn (cd, pc, info, buf, 6);
}

#define PRINT_SHLN(N)                                           \
  static void                                                   \
  print_shl##N (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,              \
	        void * dis_info,                                \
	        long value ATTRIBUTE_UNUSED,                    \
	        unsigned int attrs ATTRIBUTE_UNUSED,            \
	        bfd_vma pc ATTRIBUTE_UNUSED,                    \
	        int length ATTRIBUTE_UNUSED)                    \
  {                                                             \
    disassemble_info *info = (disassemble_info *) dis_info;     \
    (*info->fprintf_func) (info->stream, "<<" #N);              \
  }

PRINT_SHLN(1)
PRINT_SHLN(2)
PRINT_SHLN(3)
PRINT_SHLN(4)
PRINT_SHLN(5)
PRINT_SHLN(6)
PRINT_SHLN(7)
PRINT_SHLN(8)

/* Vector insn printing.  */

static void
print_vector_reg (void *dis_info, unsigned long value, vc4_operand whichop)
{
  vc4_vec_dir vec_dir;
  unsigned x, y;
  bfd_boolean do_inc, column_offset;
  unsigned scalar_reg;
  disassemble_info *info = (disassemble_info *) dis_info;
  const char *typename;
  bfd_boolean areg_p = whichop == OP_A;

  switch ((value >> 6) & 15)
    {
    case 0: case 2: case 4: case 6:
      vec_dir = H;
      x = ((value >> 7) & 3) << 4;
      if (areg_p)
        x |= (value >> 16) & 15;
      y = value & 63;
      break;
    case 8: case 10:
      vec_dir = HX;
      x = ((value >> 7) & 1) << 5;
      if (areg_p)
        x |= (value >> 16) & 15;
      y = value & 63;
      break;
    case 12:
      vec_dir = HY;
      x = 0;
      if (areg_p)
        x |= (value >> 16) & 15;
      y = value & 63;
      break;

    case 1: case 3: case 5: case 7:
      vec_dir = V;
      if (!areg_p)
        {
          x = ((value >> 7) & 3) << 4;
          x |= value & 15;
          y = value & 0x30;
        }
      else
        {
          x = ((value >> 7) & 3) << 4;
          x |= (value >> 16) & 15;
          y = value & 0x3f;
        }
      break;
    case 9: case 11:
      vec_dir = VX;
      if (!areg_p)
        {
          x = ((value >> 7) & 1) << 5;
          x |= value & 15;
          y = value & 0x30;
        }
      else
        {
          x = ((value >> 7) & 1) << 5;
          x |= (value >> 16) & 15;
          y = value & 0x3f;
        }
      break;
    case 13:
      vec_dir = VY;
      if (!areg_p)
        {
          x = value & 15;
          y = value & 0x30;
        }
      else
        {
          x = (value >> 16) & 15;
          y = value & 0x3f;
        }
      break;

    case 14:
    case 15:
      vec_dir = DASH;
      x = y = 0;
      break;
    }

  column_offset = (value >> 10) & 1;
  do_inc = (value >> 11) & 1;
  scalar_reg = (value >> 12) & 15;

  switch (vec_dir)
    {
    case H: typename = "H"; break;
    case HX: typename = "HX"; break;
    case HY: typename = "HY"; break;
    case V: typename = "V"; break;
    case VX: typename = "VX"; break;
    case VY: typename = "VY"; break;
    case DASH:
      if (whichop == OP_D || whichop == OP_A)
        (*info->fprintf_func) (info->stream, "-");
      else if (whichop == OP_B)
        {
          /* A scalar reg rN+s9 immediate.  */
          long offset;
          (*info->fprintf_func) (info->stream, "r%u", scalar_reg);
          offset = value & 127;
          offset |= ((value >> 10) & 3) << 7;
          if (offset & 0x100)
            offset |= ~0xffl;
          if (offset != 0)
            (*info->fprintf_func) (info->stream, "%s%d", offset > 0 ? "+" : "",
                                   offset);
        }
      return;
    }

  (*info->fprintf_func) (info->stream, "%s(%u", typename, y);
  if ((vec_dir == H || vec_dir == HX || vec_dir == HY)
      && do_inc)
    (*info->fprintf_func) (info->stream, "++");

  (*info->fprintf_func) (info->stream, ",%u", x);
  if ((vec_dir == V || vec_dir == VX || vec_dir == VY)
      && do_inc)
    (*info->fprintf_func) (info->stream, "++");

  (*info->fprintf_func) (info->stream, ")");

  if (scalar_reg != 15)
    (*info->fprintf_func) (info->stream, "+r%u", scalar_reg);

  if (column_offset)
    (*info->fprintf_func) (info->stream, "*");
}

static void
print_vec80aludreg (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	            void * dis_info,
	            unsigned long value,
	            unsigned int attrs ATTRIBUTE_UNUSED,
	            bfd_vma pc ATTRIBUTE_UNUSED,
	            int length ATTRIBUTE_UNUSED)
{
  print_vector_reg (dis_info, value, OP_D);
}

static void
print_vec80aluareg (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	            void * dis_info,
	            unsigned long value,
	            unsigned int attrs ATTRIBUTE_UNUSED,
	            bfd_vma pc ATTRIBUTE_UNUSED,
	            int length ATTRIBUTE_UNUSED)
{
  print_vector_reg (dis_info, value, OP_A);
}

static void
print_vec80alubreg (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	            void * dis_info,
	            unsigned long value,
	            unsigned int attrs ATTRIBUTE_UNUSED,
	            bfd_vma pc ATTRIBUTE_UNUSED,
	            int length ATTRIBUTE_UNUSED)
{
  print_vector_reg (dis_info, value, OP_B);
}

static void
print_vec80mods (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	         void * dis_info,
	         unsigned long value,
	         unsigned int attrs ATTRIBUTE_UNUSED,
	         bfd_vma pc ATTRIBUTE_UNUSED,
	         int length ATTRIBUTE_UNUSED)
{
  disassemble_info *info = (disassemble_info *) dis_info;
  unsigned long repeat = value & 7;
  bfd_boolean setf = (value >> 3) & 1;
  unsigned long predication = (value >> 4) & 7;
  unsigned long acc_sru = (value >> 7) & 127;
  const char *rep_str = "", *pred_str = "";

  switch (repeat)
    {
    case 1: rep_str = " REP2"; break;
    case 2: rep_str = " REP4"; break;
    case 3: rep_str = " REP8"; break;
    case 4: rep_str = " REP16"; break;
    case 5: rep_str = " REP32"; break;
    case 6: rep_str = " REP64"; break;
    case 7: rep_str = " REP r0"; break;
    }

  (*info->fprintf_func) (info->stream, "%s", rep_str);

  if (setf)
    (*info->fprintf_func) (info->stream, " SETF");

  switch (predication)
    {
    case 1: pred_str = " NONE"; break;
    case 2: pred_str = " IFZ"; break;
    case 3: pred_str = " IFNZ"; break;
    case 4: pred_str = " IFN"; break;
    case 5: pred_str = " IFNN"; break;
    case 6: pred_str = " IFC"; break;
    case 7: pred_str = " IFNC"; break;
    }

  (*info->fprintf_func) (info->stream, "%s", pred_str);

  if (acc_sru & SRU_BIT)
    {
      const char *func_str = "";
      switch ((acc_sru >> 3) & 7)
        {
        case 0: func_str = "SUMU"; break;
        case 1: func_str = "SUMS"; break;
        case 2: func_str = "max2"; break;
        case 3: func_str = "IMIN"; break;
        case 4: func_str = "max4"; break;
        case 5: func_str = "IMAX"; break;
        case 6: func_str = "max6"; break;
        case 7: func_str = "MAX"; break;
        }
      (info->fprintf_func) (info->stream, " %s r%u", func_str, acc_sru & 7);
    }
  else if (acc_sru & ENA_BIT)
    {
      struct
        {
          unsigned bitmask;
          const char *friendly_name;
        } known[] =
        {
          { WBA_BIT,                                 "UACC" },
          { WBA_BIT | SUB_BIT,                       "UDEC" },
          { SIGN_BIT | WBA_BIT,                      "SACC" },
          { SIGN_BIT | WBA_BIT | SUB_BIT,            "SDEC" },
          { HIGH_BIT | WBA_BIT,                      "UACCH" },
          { HIGH_BIT | WBA_BIT | SUB_BIT,            "UDECH" },
          { HIGH_BIT | SIGN_BIT | WBA_BIT,           "SACCH" },
          { HIGH_BIT | SIGN_BIT | WBA_BIT | SUB_BIT, "SDECH" },
          { 0,                                       "UADD" },
          { SUB_BIT,                                 "USUB" },
          { SIGN_BIT,                                "SADD" },
          { SIGN_BIT | SUB_BIT,                      "SSUB" },
          { HIGH_BIT,                                "UADDH" },
          { HIGH_BIT | SUB_BIT,                      "USUBH" },
          { HIGH_BIT | SIGN_BIT,                     "SADDH" },
          { HIGH_BIT | SIGN_BIT | SUB_BIT,           "SSUBH" }
        };
      unsigned i;

      if (acc_sru & CLRA_BIT)
        (*info->fprintf_func) (info->stream, " CLRA");

      for (i = 0; i < 16; i++)
        if ((acc_sru & ~(ENA_BIT | CLRA_BIT)) == known[i].bitmask)
          {
            (info->fprintf_func) (info->stream, " %s",
                                  known[i].friendly_name);
            break;
          }

      /* All cases should be covered above.  */
      if (i == 16)
        abort ();
    }
}

static void
print_vec48mod_setf (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	             void * dis_info,
	             unsigned long value,
	             unsigned int attrs ATTRIBUTE_UNUSED,
	             bfd_vma pc ATTRIBUTE_UNUSED,
	             int length ATTRIBUTE_UNUSED)
{
  disassemble_info *info = (disassemble_info *) dis_info;
  if (value)
    (info->fprintf_func) (info->stream, " SETF");
}

static void
print_vec48imm_mods (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	             void * dis_info,
	             unsigned long value,
	             unsigned int attrs ATTRIBUTE_UNUSED,
	             bfd_vma pc ATTRIBUTE_UNUSED,
	             int length ATTRIBUTE_UNUSED)
{
  print_vec80mods (cd, dis_info, value << 3, attrs, pc, length);
}

static void
print_scalar_reg (disassemble_info *info, int regno)
{
  (info->fprintf_func) (info->stream, "r%u", regno);
}

static void
print_ld_st_addr (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	          void * dis_info,
	          unsigned long value,
	          unsigned int attrs ATTRIBUTE_UNUSED,
	          bfd_vma pc ATTRIBUTE_UNUSED,
	          int length ATTRIBUTE_UNUSED)
{
  disassemble_info *info = (disassemble_info *) dis_info;
  unsigned rs = (value >> 22) & 15;
  unsigned rad = (value >> 16) & 63;
  long offset = value & 0xfffful;

  print_scalar_reg (info, rs);

  if (offset & 0x8000)
    offset |= ~0xffffl;

  /* Printing the '+' here for negative numbers too is kind of deliberate --
     that's how VC4 assembly sources seem to write it, and that's the assembly
     syntax we accept too.  */
  if (offset != 0)
    (info->fprintf_func) (info->stream, "+%ld", offset);

  if ((rad & 3) != 0)
    (*info->fprintf_func) (info->stream, "[ra/d low bits %d?]", rad & 3);

  rad >>= 2;

  if ((rad & 15) < 15)
    {
      (*info->fprintf_func) (info->stream, "+=");
      print_scalar_reg (info, rad);
    }
}

static void
print_vec80ldaddr (CGEN_CPU_DESC cd, void * dis_info, unsigned long value,
	           unsigned int attrs, bfd_vma pc, int length)
{
  print_ld_st_addr (cd, dis_info, value, attrs, pc, length);
}

static void
print_vec80staddr (CGEN_CPU_DESC cd, void * dis_info, unsigned long value,
	           unsigned int attrs, bfd_vma pc, int length)
{
  print_ld_st_addr (cd, dis_info, value, attrs, pc, length);
}

static void
print_extraldstbits (CGEN_CPU_DESC cd, void * dis_info, unsigned long value,
	            unsigned int attrs, bfd_vma pc, int length)
{
  disassemble_info *info = (disassemble_info *) dis_info;
  if (value != 0)
    (*info->fprintf_func) (info->stream, "\t; A/D extra: %lx (r%d%s%s)", value,
      value >> 2, (value & 2) ? "++" : "", (value & 1) ? "*" : "");
}

static void
print_plus_sreg (CGEN_CPU_DESC cd, void * dis_info, unsigned long value,
                 unsigned int attrs, bfd_vma pc, int length, unsigned sreg)
{
  disassemble_info *info = (disassemble_info *) dis_info;
  if (value != 0)
    (*info->fprintf_func) (info->stream, "+r%u", sreg);
}

#define PRINT_SREG(X)                                                   \
  static void print_plus_sr##X (CGEN_CPU_DESC cd, void * dis_info,      \
                                unsigned long value, unsigned int attrs,\
                                bfd_vma pc, int length)                 \
  {                                                                     \
    print_plus_sreg (cd, dis_info, value, attrs, pc, length, (X));      \
  }

PRINT_SREG(0)
PRINT_SREG(1)
PRINT_SREG(2)
PRINT_SREG(3)
PRINT_SREG(4)
PRINT_SREG(5)
PRINT_SREG(6)
PRINT_SREG(7)

static void
print_vec48aludreg_h (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	              void * dis_info,
	              unsigned long value,
	              unsigned int attrs ATTRIBUTE_UNUSED,
	              bfd_vma pc ATTRIBUTE_UNUSED,
	              int length ATTRIBUTE_UNUSED)
{
  print_vector_reg (dis_info, (value & ~0x40) | 0xf000, OP_D);
}

static void
print_vec48aludreg_v (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	              void * dis_info,
	              unsigned long value,
	              unsigned int attrs ATTRIBUTE_UNUSED,
	              bfd_vma pc ATTRIBUTE_UNUSED,
	              int length ATTRIBUTE_UNUSED)
{
  print_vector_reg (dis_info, value | 0xf040, OP_D);
}

static void
print_vec48aluareg_h (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	              void * dis_info,
	              unsigned long value,
	              unsigned int attrs ATTRIBUTE_UNUSED,
	              bfd_vma pc ATTRIBUTE_UNUSED,
	              int length ATTRIBUTE_UNUSED)
{
  print_vector_reg (dis_info, (value & ~0x40) | 0xf000, OP_A);
}

static void
print_vec48aluareg_v (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	              void * dis_info,
	              unsigned long value,
	              unsigned int attrs ATTRIBUTE_UNUSED,
	              bfd_vma pc ATTRIBUTE_UNUSED,
	              int length ATTRIBUTE_UNUSED)
{
  print_vector_reg (dis_info, value | 0xf040, OP_A);
}

static void
print_vec48alubreg_h (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	              void * dis_info,
	              unsigned long value,
	              unsigned int attrs ATTRIBUTE_UNUSED,
	              bfd_vma pc ATTRIBUTE_UNUSED,
	              int length ATTRIBUTE_UNUSED)
{
  print_vector_reg (dis_info, (value & ~0x40) | 0xf000, OP_B);
}

static void
print_vec48alubreg_v (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	              void * dis_info,
	              unsigned long value,
	              unsigned int attrs ATTRIBUTE_UNUSED,
	              bfd_vma pc ATTRIBUTE_UNUSED,
	              int length ATTRIBUTE_UNUSED)
{
  print_vector_reg (dis_info, value | 0xf040, OP_B);
}

static void
print_vec48sclr (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	         void * dis_info,
	         unsigned long value,
	         unsigned int attrs ATTRIBUTE_UNUSED,
	         bfd_vma pc ATTRIBUTE_UNUSED,
	         int length ATTRIBUTE_UNUSED)
{
  disassemble_info *info = (disassemble_info *) dis_info;
  print_scalar_reg (dis_info, value);
  /* 6 bits seems too many for a simple scalar reg field?  */
  if (value & 0x10)
    (info->fprintf_func) (info->stream, "?bit4?");
  if (value & 0x20)
    (info->fprintf_func) (info->stream, "?bit5?");
}

static void
print_dummy (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	         void * dis_info ATTRIBUTE_UNUSED,
	         unsigned long value ATTRIBUTE_UNUSED,
	         unsigned int attrs ATTRIBUTE_UNUSED,
	         bfd_vma pc ATTRIBUTE_UNUSED,
	         int length ATTRIBUTE_UNUSED)
{
}

/* -- */

void vc4_cgen_print_operand
  (CGEN_CPU_DESC, int, PTR, CGEN_FIELDS *, void const *, bfd_vma, int);

/* Main entry point for printing operands.
   XINFO is a `void *' and not a `disassemble_info *' to not put a requirement
   of dis-asm.h on cgen.h.

   This function is basically just a big switch statement.  Earlier versions
   used tables to look up the function to use, but
   - if the table contains both assembler and disassembler functions then
     the disassembler contains much of the assembler and vice-versa,
   - there's a lot of inlining possibilities as things grow,
   - using a switch statement avoids the function call overhead.

   This function could be moved into `print_insn_normal', but keeping it
   separate makes clear the interface between `print_insn_normal' and each of
   the handlers.  */

void
vc4_cgen_print_operand (CGEN_CPU_DESC cd,
			   int opindex,
			   void * xinfo,
			   CGEN_FIELDS *fields,
			   void const *attrs ATTRIBUTE_UNUSED,
			   bfd_vma pc,
			   int length)
{
  disassemble_info *info = (disassemble_info *) xinfo;

  switch (opindex)
    {
    case VC4_OPERAND_ADDCMPBAREG :
      print_keyword (cd, info, & vc4_cgen_opval_h_fastreg, fields->f_op7_4, 0);
      break;
    case VC4_OPERAND_ADDCMPBIMM :
      print_normal (cd, info, fields->f_op7_4s, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case VC4_OPERAND_ADDSPOFFSET :
      print_normal (cd, info, fields->f_addspoffset, 0, pc, length);
      break;
    case VC4_OPERAND_ALU16DREG :
      print_keyword (cd, info, & vc4_cgen_opval_h_fastreg, fields->f_op3_0, 0);
      break;
    case VC4_OPERAND_ALU16IMM :
      print_normal (cd, info, fields->f_op8_4, 0, pc, length);
      break;
    case VC4_OPERAND_ALU16IMM_SHL3 :
      print_normal (cd, info, fields->f_op8_4_shl3, 0, pc, length);
      break;
    case VC4_OPERAND_ALU16SREG :
      print_keyword (cd, info, & vc4_cgen_opval_h_fastreg, fields->f_op7_4, 0);
      break;
    case VC4_OPERAND_ALU32AREG :
      print_keyword (cd, info, & vc4_cgen_opval_h_reg, fields->f_op31_27, 0);
      break;
    case VC4_OPERAND_ALU32BREG :
      print_keyword (cd, info, & vc4_cgen_opval_h_reg, fields->f_op20_16, 0);
      break;
    case VC4_OPERAND_ALU32COND :
      print_keyword (cd, info, & vc4_cgen_opval_h_dotcond, fields->f_op26_23, 0);
      break;
    case VC4_OPERAND_ALU32DREG :
      print_keyword (cd, info, & vc4_cgen_opval_h_reg, fields->f_op4_0, 0);
      break;
    case VC4_OPERAND_ALU32MISSINGAREG :
      print_dummy (cd, info, fields->f_op31_27, 0, pc, length);
      break;
    case VC4_OPERAND_ALU48IDREG :
      print_keyword (cd, info, & vc4_cgen_opval_h_reg, fields->f_op4_0, 0);
      break;
    case VC4_OPERAND_ALU48IMMU :
      print_normal (cd, info, fields->f_op47_16, 0, pc, length);
      break;
    case VC4_OPERAND_ALU48ISREG :
      print_keyword (cd, info, & vc4_cgen_opval_h_reg, fields->f_op9_5, 0);
      break;
    case VC4_OPERAND_ALU48PCREL :
      print_address (cd, info, fields->f_pcrel32_48, 0|(1<<CGEN_OPERAND_PCREL_ADDR), pc, length);
      break;
    case VC4_OPERAND_APLUS_SR0 :
      print_plus_sr0 (cd, info, fields->f_op18, 0, pc, length);
      break;
    case VC4_OPERAND_APLUS_SR1 :
      print_plus_sr1 (cd, info, fields->f_op18, 0, pc, length);
      break;
    case VC4_OPERAND_APLUS_SR2 :
      print_plus_sr2 (cd, info, fields->f_op18, 0, pc, length);
      break;
    case VC4_OPERAND_APLUS_SR3 :
      print_plus_sr3 (cd, info, fields->f_op18, 0, pc, length);
      break;
    case VC4_OPERAND_APLUS_SR4 :
      print_plus_sr4 (cd, info, fields->f_op18, 0, pc, length);
      break;
    case VC4_OPERAND_APLUS_SR5 :
      print_plus_sr5 (cd, info, fields->f_op18, 0, pc, length);
      break;
    case VC4_OPERAND_APLUS_SR6 :
      print_plus_sr6 (cd, info, fields->f_op18, 0, pc, length);
      break;
    case VC4_OPERAND_APLUS_SR7 :
      print_plus_sr7 (cd, info, fields->f_op18, 0, pc, length);
      break;
    case VC4_OPERAND_BCC32IMM :
      print_normal (cd, info, fields->f_op29_24, 0, pc, length);
      break;
    case VC4_OPERAND_BCC32SREG :
      print_keyword (cd, info, & vc4_cgen_opval_h_fastreg, fields->f_op29_26, 0);
      break;
    case VC4_OPERAND_BPLUS_SR0 :
      print_plus_sr0 (cd, info, fields->f_op38, 0, pc, length);
      break;
    case VC4_OPERAND_BPLUS_SR1 :
      print_plus_sr1 (cd, info, fields->f_op38, 0, pc, length);
      break;
    case VC4_OPERAND_BPLUS_SR2 :
      print_plus_sr2 (cd, info, fields->f_op38, 0, pc, length);
      break;
    case VC4_OPERAND_BPLUS_SR3 :
      print_plus_sr3 (cd, info, fields->f_op38, 0, pc, length);
      break;
    case VC4_OPERAND_BPLUS_SR4 :
      print_plus_sr4 (cd, info, fields->f_op38, 0, pc, length);
      break;
    case VC4_OPERAND_BPLUS_SR5 :
      print_plus_sr5 (cd, info, fields->f_op38, 0, pc, length);
      break;
    case VC4_OPERAND_BPLUS_SR6 :
      print_plus_sr6 (cd, info, fields->f_op38, 0, pc, length);
      break;
    case VC4_OPERAND_BPLUS_SR7 :
      print_plus_sr7 (cd, info, fields->f_op38, 0, pc, length);
      break;
    case VC4_OPERAND_CONDCODE :
      print_keyword (cd, info, & vc4_cgen_opval_h_cond, fields->f_op10_7, 0);
      break;
    case VC4_OPERAND_CONDCODEBCC32 :
      print_keyword (cd, info, & vc4_cgen_opval_h_cond, fields->f_op11_8, 0);
      break;
    case VC4_OPERAND_DISP5 :
      print_normal (cd, info, fields->f_op20_16, 0, pc, length);
      break;
    case VC4_OPERAND_DPLUS_SR0 :
      print_plus_sr0 (cd, info, fields->f_op43, 0, pc, length);
      break;
    case VC4_OPERAND_DPLUS_SR1 :
      print_plus_sr1 (cd, info, fields->f_op43, 0, pc, length);
      break;
    case VC4_OPERAND_DPLUS_SR2 :
      print_plus_sr2 (cd, info, fields->f_op43, 0, pc, length);
      break;
    case VC4_OPERAND_DPLUS_SR3 :
      print_plus_sr3 (cd, info, fields->f_op43, 0, pc, length);
      break;
    case VC4_OPERAND_DPLUS_SR4 :
      print_plus_sr4 (cd, info, fields->f_op43, 0, pc, length);
      break;
    case VC4_OPERAND_DPLUS_SR5 :
      print_plus_sr5 (cd, info, fields->f_op43, 0, pc, length);
      break;
    case VC4_OPERAND_DPLUS_SR6 :
      print_plus_sr6 (cd, info, fields->f_op43, 0, pc, length);
      break;
    case VC4_OPERAND_DPLUS_SR7 :
      print_plus_sr7 (cd, info, fields->f_op43, 0, pc, length);
      break;
    case VC4_OPERAND_DUMMYABITS :
      print_extraldstbits (cd, info, fields->f_dummyabits, 0|(1<<CGEN_OPERAND_VIRTUAL), pc, length);
      break;
    case VC4_OPERAND_DUMMYDBITS :
      print_extraldstbits (cd, info, fields->f_op27_22, 0, pc, length);
      break;
    case VC4_OPERAND_FLOATIMM6 :
      print_normal (cd, info, fields->f_op21_16, 0, pc, length);
      break;
    case VC4_OPERAND_IMM6 :
      print_normal (cd, info, fields->f_op21_16s, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case VC4_OPERAND_IMM6_SHL1 :
      print_normal (cd, info, fields->f_op21_16s_shl1, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case VC4_OPERAND_IMM6_SHL2 :
      print_normal (cd, info, fields->f_op21_16s_shl2, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case VC4_OPERAND_IMM6_SHL3 :
      print_normal (cd, info, fields->f_op21_16s_shl3, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case VC4_OPERAND_IMM6_SHL4 :
      print_normal (cd, info, fields->f_op21_16s_shl4, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case VC4_OPERAND_IMM6_SHL5 :
      print_normal (cd, info, fields->f_op21_16s_shl5, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case VC4_OPERAND_IMM6_SHL6 :
      print_normal (cd, info, fields->f_op21_16s_shl6, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case VC4_OPERAND_IMM6_SHL7 :
      print_normal (cd, info, fields->f_op21_16s_shl7, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case VC4_OPERAND_IMM6_SHL8 :
      print_normal (cd, info, fields->f_op21_16s_shl8, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case VC4_OPERAND_LDSTOFF :
      print_normal (cd, info, fields->f_ldstoff, 0, pc, length);
      break;
    case VC4_OPERAND_MEM48OFFSET27 :
      print_normal (cd, info, fields->f_offset27_48, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case VC4_OPERAND_MEM48PCREL27 :
      print_address (cd, info, fields->f_pcrel27_48, 0|(1<<CGEN_OPERAND_PCREL_ADDR), pc, length);
      break;
    case VC4_OPERAND_MEM48SREG :
      print_keyword (cd, info, & vc4_cgen_opval_h_reg, fields->f_op47_43, 0);
      break;
    case VC4_OPERAND_OFF16BASEREG :
      print_keyword (cd, info, & vc4_cgen_opval_h_basereg, fields->f_op9_8, 0);
      break;
    case VC4_OPERAND_OFFSET12 :
      print_normal (cd, info, fields->f_offset12, 0|(1<<CGEN_OPERAND_SIGNED)|(1<<CGEN_OPERAND_VIRTUAL), pc, length);
      break;
    case VC4_OPERAND_OFFSET16 :
      print_normal (cd, info, fields->f_op31_16s, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case VC4_OPERAND_OFFSET16_SHL1 :
      print_normal (cd, info, fields->f_op31_16s_shl1, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case VC4_OPERAND_OFFSET16_SHL2 :
      print_normal (cd, info, fields->f_op31_16s_shl2, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case VC4_OPERAND_OFFSET16_SHL3 :
      print_normal (cd, info, fields->f_op31_16s_shl3, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case VC4_OPERAND_OFFSET16_SHL4 :
      print_normal (cd, info, fields->f_op31_16s_shl4, 0|(1<<CGEN_OPERAND_SIGNED), pc, length);
      break;
    case VC4_OPERAND_OFFSET23BITS :
      print_address (cd, info, fields->f_offset23bits, 0|(1<<CGEN_OPERAND_RELAX)|(1<<CGEN_OPERAND_PCREL_ADDR)|(1<<CGEN_OPERAND_VIRTUAL), pc, length);
      break;
    case VC4_OPERAND_OFFSET27BITS :
      print_address (cd, info, fields->f_offset27bits, 0|(1<<CGEN_OPERAND_PCREL_ADDR)|(1<<CGEN_OPERAND_VIRTUAL), pc, length);
      break;
    case VC4_OPERAND_OPERAND10_0 :
      print_normal (cd, info, fields->f_op10_0, 0, pc, length);
      break;
    case VC4_OPERAND_OPERAND47_16 :
      print_normal (cd, info, fields->f_op47_16, 0, pc, length);
      break;
    case VC4_OPERAND_OPERAND79_48 :
      print_normal (cd, info, fields->f_op79_48, 0, pc, length);
      break;
    case VC4_OPERAND_PCREL10BITS :
      print_address (cd, info, fields->f_pcrel10, 0|(1<<CGEN_OPERAND_PCREL_ADDR), pc, length);
      break;
    case VC4_OPERAND_PCREL16 :
      print_address (cd, info, fields->f_pcrel16, 0|(1<<CGEN_OPERAND_RELAX)|(1<<CGEN_OPERAND_PCREL_ADDR), pc, length);
      break;
    case VC4_OPERAND_PCREL8BITS :
      print_address (cd, info, fields->f_pcrel8, 0|(1<<CGEN_OPERAND_PCREL_ADDR), pc, length);
      break;
    case VC4_OPERAND_PCRELCC :
      print_address (cd, info, fields->f_pcrelcc, 0|(1<<CGEN_OPERAND_RELAX)|(1<<CGEN_OPERAND_PCREL_ADDR), pc, length);
      break;
    case VC4_OPERAND_PPENDREG0 :
      print_keyword (cd, info, & vc4_cgen_opval_h_reg, fields->f_op4_0_base_0, 0);
      break;
    case VC4_OPERAND_PPENDREG16 :
      print_keyword (cd, info, & vc4_cgen_opval_h_reg, fields->f_op4_0_base_16, 0);
      break;
    case VC4_OPERAND_PPENDREG24 :
      print_keyword (cd, info, & vc4_cgen_opval_h_reg, fields->f_op4_0_base_24, 0);
      break;
    case VC4_OPERAND_PPENDREG6 :
      print_keyword (cd, info, & vc4_cgen_opval_h_reg, fields->f_op4_0_base_6, 0);
      break;
    case VC4_OPERAND_PPSTARTREG :
      print_keyword (cd, info, & vc4_cgen_opval_h_ppreg, fields->f_op6_5, 0);
      break;
    case VC4_OPERAND_PREGDST :
      print_keyword (cd, info, & vc4_cgen_opval_h_preg, fields->f_op4_0, 0);
      break;
    case VC4_OPERAND_PREGSRC :
      print_keyword (cd, info, & vc4_cgen_opval_h_preg, fields->f_op20_16, 0);
      break;
    case VC4_OPERAND_SETF_MOD :
      print_vec48mod_setf (cd, info, fields->f_op38, 0, pc, length);
      break;
    case VC4_OPERAND_SHL1 :
      print_shl1 (cd, info, 0, 0, pc, length);
      break;
    case VC4_OPERAND_SHL2 :
      print_shl2 (cd, info, 0, 0, pc, length);
      break;
    case VC4_OPERAND_SHL3 :
      print_shl3 (cd, info, 0, 0, pc, length);
      break;
    case VC4_OPERAND_SHL4 :
      print_shl4 (cd, info, 0, 0, pc, length);
      break;
    case VC4_OPERAND_SHL5 :
      print_shl5 (cd, info, 0, 0, pc, length);
      break;
    case VC4_OPERAND_SHL6 :
      print_shl6 (cd, info, 0, 0, pc, length);
      break;
    case VC4_OPERAND_SHL7 :
      print_shl7 (cd, info, 0, 0, pc, length);
      break;
    case VC4_OPERAND_SHL8 :
      print_shl8 (cd, info, 0, 0, pc, length);
      break;
    case VC4_OPERAND_SPOFFSET :
      print_normal (cd, info, fields->f_spoffset, 0, pc, length);
      break;
    case VC4_OPERAND_SWI_IMM :
      print_normal (cd, info, fields->f_op5_0, 0, pc, length);
      break;
    case VC4_OPERAND_V48AREG_H :
      print_vec48aluareg_h (cd, info, fields->f_vec48areg, 0|(1<<CGEN_OPERAND_VIRTUAL), pc, length);
      break;
    case VC4_OPERAND_V48AREG_V :
      print_vec48aluareg_v (cd, info, fields->f_vec48areg, 0|(1<<CGEN_OPERAND_VIRTUAL), pc, length);
      break;
    case VC4_OPERAND_V48BREG_H :
      print_vec48alubreg_h (cd, info, fields->f_vec48breg, 0|(1<<CGEN_OPERAND_VIRTUAL), pc, length);
      break;
    case VC4_OPERAND_V48BREG_V :
      print_vec48alubreg_v (cd, info, fields->f_vec48breg, 0|(1<<CGEN_OPERAND_VIRTUAL), pc, length);
      break;
    case VC4_OPERAND_V48DREG_H :
      print_vec48aludreg_h (cd, info, fields->f_vec48dreg, 0|(1<<CGEN_OPERAND_VIRTUAL), pc, length);
      break;
    case VC4_OPERAND_V48DREG_V :
      print_vec48aludreg_v (cd, info, fields->f_vec48dreg, 0|(1<<CGEN_OPERAND_VIRTUAL), pc, length);
      break;
    case VC4_OPERAND_V48IMM :
      print_normal (cd, info, fields->f_op37_32, 0, pc, length);
      break;
    case VC4_OPERAND_V48IMM_MODS :
      print_vec48imm_mods (cd, info, fields->f_op41_38, 0, pc, length);
      break;
    case VC4_OPERAND_V48SCLR :
      print_vec48sclr (cd, info, fields->f_op37_32, 0, pc, length);
      break;
    case VC4_OPERAND_V80A32REG :
      print_vec80aluareg (cd, info, fields->f_vec80areg, 0|(1<<CGEN_OPERAND_VIRTUAL), pc, length);
      break;
    case VC4_OPERAND_V80B32REG :
      print_vec80alubreg (cd, info, fields->f_vec80breg, 0|(1<<CGEN_OPERAND_VIRTUAL), pc, length);
      break;
    case VC4_OPERAND_V80D32REG :
      print_vec80aludreg (cd, info, fields->f_vec80dreg, 0|(1<<CGEN_OPERAND_VIRTUAL), pc, length);
      break;
    case VC4_OPERAND_V80IMM :
      print_normal (cd, info, fields->f_vec80imm, 0|(1<<CGEN_OPERAND_SIGNED)|(1<<CGEN_OPERAND_VIRTUAL), pc, length);
      break;
    case VC4_OPERAND_V80MODS :
      print_vec80mods (cd, info, fields->f_vec80mods, 0|(1<<CGEN_OPERAND_VIRTUAL), pc, length);
      break;
    case VC4_OPERAND_V80MODS_MEM :
      print_vec80mods (cd, info, fields->f_vec80mods_mem, 0|(1<<CGEN_OPERAND_VIRTUAL), pc, length);
      break;
    case VC4_OPERAND_VEC_LDADDR :
      print_vec80ldaddr (cd, info, fields->f_vec80ldaddr, 0|(1<<CGEN_OPERAND_VIRTUAL), pc, length);
      break;
    case VC4_OPERAND_VEC_STADDR :
      print_vec80staddr (cd, info, fields->f_vec80staddr, 0|(1<<CGEN_OPERAND_VIRTUAL), pc, length);
      break;
    case VC4_OPERAND_VMEMWIDTH :
      print_keyword (cd, info, & vc4_cgen_opval_h_eltsize, fields->f_op4_3, 0);
      break;

    default :
      /* xgettext:c-format */
      fprintf (stderr, _("Unrecognized field %d while printing insn.\n"),
	       opindex);
    abort ();
  }
}

cgen_print_fn * const vc4_cgen_print_handlers[] = 
{
  print_insn_normal,
};


void
vc4_cgen_init_dis (CGEN_CPU_DESC cd)
{
  vc4_cgen_init_opcode_table (cd);
  vc4_cgen_init_ibld_table (cd);
  cd->print_handlers = & vc4_cgen_print_handlers[0];
  cd->print_operand = vc4_cgen_print_operand;
}


/* Default print handler.  */

static void
print_normal (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	      void *dis_info,
	      long value,
	      unsigned int attrs,
	      bfd_vma pc ATTRIBUTE_UNUSED,
	      int length ATTRIBUTE_UNUSED)
{
  disassemble_info *info = (disassemble_info *) dis_info;

  /* Print the operand as directed by the attributes.  */
  if (CGEN_BOOL_ATTR (attrs, CGEN_OPERAND_SEM_ONLY))
    ; /* nothing to do */
  else if (CGEN_BOOL_ATTR (attrs, CGEN_OPERAND_SIGNED))
    (*info->fprintf_func) (info->stream, "%ld", value);
  else
    (*info->fprintf_func) (info->stream, "0x%lx", value);
}

/* Default address handler.  */

static void
print_address (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	       void *dis_info,
	       bfd_vma value,
	       unsigned int attrs,
	       bfd_vma pc ATTRIBUTE_UNUSED,
	       int length ATTRIBUTE_UNUSED)
{
  disassemble_info *info = (disassemble_info *) dis_info;

  /* Print the operand as directed by the attributes.  */
  if (CGEN_BOOL_ATTR (attrs, CGEN_OPERAND_SEM_ONLY))
    ; /* Nothing to do.  */
  else if (CGEN_BOOL_ATTR (attrs, CGEN_OPERAND_PCREL_ADDR))
    (*info->print_address_func) (value, info);
  else if (CGEN_BOOL_ATTR (attrs, CGEN_OPERAND_ABS_ADDR))
    (*info->print_address_func) (value, info);
  else if (CGEN_BOOL_ATTR (attrs, CGEN_OPERAND_SIGNED))
    (*info->fprintf_func) (info->stream, "%ld", (long) value);
  else
    (*info->fprintf_func) (info->stream, "0x%lx", (long) value);
}

/* Keyword print handler.  */

static void
print_keyword (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	       void *dis_info,
	       CGEN_KEYWORD *keyword_table,
	       long value,
	       unsigned int attrs ATTRIBUTE_UNUSED)
{
  disassemble_info *info = (disassemble_info *) dis_info;
  const CGEN_KEYWORD_ENTRY *ke;

  ke = cgen_keyword_lookup_value (keyword_table, value);
  if (ke != NULL)
    (*info->fprintf_func) (info->stream, "%s", ke->name);
  else
    (*info->fprintf_func) (info->stream, "???");
}

/* Default insn printer.

   DIS_INFO is defined as `void *' so the disassembler needn't know anything
   about disassemble_info.  */

static void
print_insn_normal (CGEN_CPU_DESC cd,
		   void *dis_info,
		   const CGEN_INSN *insn,
		   CGEN_FIELDS *fields,
		   bfd_vma pc,
		   int length)
{
  const CGEN_SYNTAX *syntax = CGEN_INSN_SYNTAX (insn);
  disassemble_info *info = (disassemble_info *) dis_info;
  const CGEN_SYNTAX_CHAR_TYPE *syn;

  CGEN_INIT_PRINT (cd);

  for (syn = CGEN_SYNTAX_STRING (syntax); *syn; ++syn)
    {
      if (CGEN_SYNTAX_MNEMONIC_P (*syn))
	{
	  (*info->fprintf_func) (info->stream, "%s", CGEN_INSN_MNEMONIC (insn));
	  continue;
	}
      if (CGEN_SYNTAX_CHAR_P (*syn))
	{
	  (*info->fprintf_func) (info->stream, "%c", CGEN_SYNTAX_CHAR (*syn));
	  continue;
	}

      /* We have an operand.  */
      vc4_cgen_print_operand (cd, CGEN_SYNTAX_FIELD (*syn), info,
				 fields, CGEN_INSN_ATTRS (insn), pc, length);
    }
}

/* Subroutine of print_insn. Reads an insn into the given buffers and updates
   the extract info.
   Returns 0 if all is well, non-zero otherwise.  */

static int
read_insn (CGEN_CPU_DESC cd ATTRIBUTE_UNUSED,
	   bfd_vma pc,
	   disassemble_info *info,
	   bfd_byte *buf,
	   int buflen,
	   CGEN_EXTRACT_INFO *ex_info,
	   unsigned long *insn_value)
{
  int status = (*info->read_memory_func) (pc, buf, buflen, info);

  if (status != 0)
    {
      (*info->memory_error_func) (status, pc, info);
      return -1;
    }

  ex_info->dis_info = info;
  ex_info->valid = (1 << buflen) - 1;
  ex_info->insn_bytes = buf;

  *insn_value = bfd_get_bits (buf, buflen * 8, info->endian == BFD_ENDIAN_BIG);
  return 0;
}

/* Utility to print an insn.
   BUF is the base part of the insn, target byte order, BUFLEN bytes long.
   The result is the size of the insn in bytes or zero for an unknown insn
   or -1 if an error occurs fetching data (memory_error_func will have
   been called).  */

static int
print_insn (CGEN_CPU_DESC cd,
	    bfd_vma pc,
	    disassemble_info *info,
	    bfd_byte *buf,
	    unsigned int buflen)
{
  CGEN_INSN_INT insn_value;
  const CGEN_INSN_LIST *insn_list;
  CGEN_EXTRACT_INFO ex_info;
  int basesize;

  /* Extract base part of instruction, just in case CGEN_DIS_* uses it. */
  basesize = cd->base_insn_bitsize < buflen * 8 ?
                                     cd->base_insn_bitsize : buflen * 8;
  insn_value = cgen_get_insn_value (cd, buf, basesize);


  /* Fill in ex_info fields like read_insn would.  Don't actually call
     read_insn, since the incoming buffer is already read (and possibly
     modified a la m32r).  */
  ex_info.valid = (1 << buflen) - 1;
  ex_info.dis_info = info;
  ex_info.insn_bytes = buf;

  /* The instructions are stored in hash lists.
     Pick the first one and keep trying until we find the right one.  */

  insn_list = CGEN_DIS_LOOKUP_INSN (cd, (char *) buf, insn_value);
  while (insn_list != NULL)
    {
      const CGEN_INSN *insn = insn_list->insn;
      CGEN_FIELDS fields;
      int length;
      unsigned long insn_value_cropped;

#ifdef CGEN_VALIDATE_INSN_SUPPORTED 
      /* Not needed as insn shouldn't be in hash lists if not supported.  */
      /* Supported by this cpu?  */
      if (! vc4_cgen_insn_supported (cd, insn))
        {
          insn_list = CGEN_DIS_NEXT_INSN (insn_list);
	  continue;
        }
#endif

      /* Basic bit mask must be correct.  */
      /* ??? May wish to allow target to defer this check until the extract
	 handler.  */

      /* Base size may exceed this instruction's size.  Extract the
         relevant part from the buffer. */
      if ((unsigned) (CGEN_INSN_BITSIZE (insn) / 8) < buflen &&
	  (unsigned) (CGEN_INSN_BITSIZE (insn) / 8) <= sizeof (unsigned long))
	insn_value_cropped = bfd_get_bits (buf, CGEN_INSN_BITSIZE (insn), 
					   info->endian == BFD_ENDIAN_BIG);
      else
	insn_value_cropped = insn_value;

      if ((insn_value_cropped & CGEN_INSN_BASE_MASK (insn))
	  == CGEN_INSN_BASE_VALUE (insn))
	{
#ifdef CGEN_MAX_EXTRA_OPCODE_OPERANDS
	  unsigned int i, extra_field;
          unsigned int limit_bits = (CGEN_MAX_EXTRA_OPCODE_OPERANDS + 1)
                                    * cd->base_insn_bitsize;

          if (limit_bits > (unsigned int) CGEN_INSN_BITSIZE (insn))
            limit_bits = (unsigned int) CGEN_INSN_BITSIZE (insn);

	  /* Reject insns with opcode bits (constant ifields) beyond the base
	     insn that do not match the current insn in the list.  */
	  for (i = cd->base_insn_bitsize, extra_field = 0;
	       i < limit_bits;
	       i += cd->base_insn_bitsize, extra_field++)
	    {
	      bfd_byte extrabuf[CGEN_MAX_INSN_SIZE];
	      unsigned long bits;
	      int status, buflen;

	      buflen = cd->base_insn_bitsize / 8;
	      status = (*info->read_memory_func) (pc + i / 8, extrabuf, buflen,
						  info);

	      bits = bfd_get_bits (extrabuf, cd->base_insn_bitsize,
				   info->endian == BFD_ENDIAN_BIG);

	      if ((bits & CGEN_INSN_IFIELD_MASK (insn, extra_field))
		  != CGEN_INSN_IFIELD_VALUE (insn, extra_field))
		goto next_insn;
	    }
#endif /* ! CGEN_MAX_EXTRA_OPCODE_OPERANDS */

	  /* Printing is handled in two passes.  The first pass parses the
	     machine insn and extracts the fields.  The second pass prints
	     them.  */

	  /* Make sure the entire insn is loaded into insn_value, if it
	     can fit.  */
	  if (((unsigned) CGEN_INSN_BITSIZE (insn) > cd->base_insn_bitsize) &&
	      (unsigned) (CGEN_INSN_BITSIZE (insn) / 8) <= sizeof (unsigned long))
	    {
	      unsigned long full_insn_value;
	      int rc = read_insn (cd, pc, info, buf,
				  CGEN_INSN_BITSIZE (insn) / 8,
				  & ex_info, & full_insn_value);
	      if (rc != 0)
		return rc;
	      length = CGEN_EXTRACT_FN (cd, insn)
		(cd, insn, &ex_info, full_insn_value, &fields, pc);
	    }
	  else
	    length = CGEN_EXTRACT_FN (cd, insn)
	      (cd, insn, &ex_info, insn_value_cropped, &fields, pc);

	  /* Length < 0 -> error.  */
	  if (length < 0)
	    return length;
	  if (length > 0)
	    {
	      CGEN_PRINT_FN (cd, insn) (cd, info, insn, &fields, pc, length);
	      /* Length is in bits, result is in bytes.  */
	      return length / 8;
	    }
	}

    next_insn:
      insn_list = CGEN_DIS_NEXT_INSN (insn_list);
    }

  return 0;
}

/* Default value for CGEN_PRINT_INSN.
   The result is the size of the insn in bytes or zero for an unknown insn
   or -1 if an error occured fetching bytes.  */

#ifndef CGEN_PRINT_INSN
#define CGEN_PRINT_INSN default_print_insn
#endif

static int
default_print_insn (CGEN_CPU_DESC cd, bfd_vma pc, disassemble_info *info)
{
  bfd_byte buf[CGEN_MAX_INSN_SIZE];
  int buflen;
  int status;

  /* Attempt to read the base part of the insn.  */
  buflen = cd->base_insn_bitsize / 8;
  status = (*info->read_memory_func) (pc, buf, buflen, info);

  /* Try again with the minimum part, if min < base.  */
  if (status != 0 && (cd->min_insn_bitsize < cd->base_insn_bitsize))
    {
      buflen = cd->min_insn_bitsize / 8;
      status = (*info->read_memory_func) (pc, buf, buflen, info);
    }

  if (status != 0)
    {
      (*info->memory_error_func) (status, pc, info);
      return -1;
    }

  return print_insn (cd, pc, info, buf, buflen);
}

/* Main entry point.
   Print one instruction from PC on INFO->STREAM.
   Return the size of the instruction (in bytes).  */

typedef struct cpu_desc_list
{
  struct cpu_desc_list *next;
  CGEN_BITSET *isa;
  int mach;
  int endian;
  CGEN_CPU_DESC cd;
} cpu_desc_list;

int
print_insn_vc4 (bfd_vma pc, disassemble_info *info)
{
  static cpu_desc_list *cd_list = 0;
  cpu_desc_list *cl = 0;
  static CGEN_CPU_DESC cd = 0;
  static CGEN_BITSET *prev_isa;
  static int prev_mach;
  static int prev_endian;
  int length;
  CGEN_BITSET *isa;
  int mach;
  int endian = (info->endian == BFD_ENDIAN_BIG
		? CGEN_ENDIAN_BIG
		: CGEN_ENDIAN_LITTLE);
  enum bfd_architecture arch;

  /* ??? gdb will set mach but leave the architecture as "unknown" */
#ifndef CGEN_BFD_ARCH
#define CGEN_BFD_ARCH bfd_arch_vc4
#endif
  arch = info->arch;
  if (arch == bfd_arch_unknown)
    arch = CGEN_BFD_ARCH;
   
  /* There's no standard way to compute the machine or isa number
     so we leave it to the target.  */
#ifdef CGEN_COMPUTE_MACH
  mach = CGEN_COMPUTE_MACH (info);
#else
  mach = info->mach;
#endif

#ifdef CGEN_COMPUTE_ISA
  {
    static CGEN_BITSET *permanent_isa;

    if (!permanent_isa)
      permanent_isa = cgen_bitset_create (MAX_ISAS);
    isa = permanent_isa;
    cgen_bitset_clear (isa);
    cgen_bitset_add (isa, CGEN_COMPUTE_ISA (info));
  }
#else
  isa = info->insn_sets;
#endif

  /* If we've switched cpu's, try to find a handle we've used before */
  if (cd
      && (cgen_bitset_compare (isa, prev_isa) != 0
	  || mach != prev_mach
	  || endian != prev_endian))
    {
      cd = 0;
      for (cl = cd_list; cl; cl = cl->next)
	{
	  if (cgen_bitset_compare (cl->isa, isa) == 0 &&
	      cl->mach == mach &&
	      cl->endian == endian)
	    {
	      cd = cl->cd;
 	      prev_isa = cd->isas;
	      break;
	    }
	}
    } 

  /* If we haven't initialized yet, initialize the opcode table.  */
  if (! cd)
    {
      const bfd_arch_info_type *arch_type = bfd_lookup_arch (arch, mach);
      const char *mach_name;

      if (!arch_type)
	abort ();
      mach_name = arch_type->printable_name;

      prev_isa = cgen_bitset_copy (isa);
      prev_mach = mach;
      prev_endian = endian;
      cd = vc4_cgen_cpu_open (CGEN_CPU_OPEN_ISAS, prev_isa,
				 CGEN_CPU_OPEN_BFDMACH, mach_name,
				 CGEN_CPU_OPEN_ENDIAN, prev_endian,
				 CGEN_CPU_OPEN_END);
      if (!cd)
	abort ();

      /* Save this away for future reference.  */
      cl = xmalloc (sizeof (struct cpu_desc_list));
      cl->cd = cd;
      cl->isa = prev_isa;
      cl->mach = mach;
      cl->endian = endian;
      cl->next = cd_list;
      cd_list = cl;

      vc4_cgen_init_dis (cd);
    }

  /* We try to have as much common code as possible.
     But at this point some targets need to take over.  */
  /* ??? Some targets may need a hook elsewhere.  Try to avoid this,
     but if not possible try to move this hook elsewhere rather than
     have two hooks.  */
  length = CGEN_PRINT_INSN (cd, pc, info);
  if (length > 0)
    return length;
  if (length < 0)
    return -1;

  (*info->fprintf_func) (info->stream, UNKNOWN_INSN_MSG);
  return cd->default_insn_bitsize / 8;
}
