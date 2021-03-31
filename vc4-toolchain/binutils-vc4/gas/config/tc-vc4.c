/* tc-vc4.c -- Assembler for the Vc4 family.
   Copyright 2012
   Free Software Foundation.
   Contributed by Mark Marshall, markmarshall14@gmail.com

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to
   the Free Software Foundation, 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#include "as.h"
#include "subsegs.h"
#include "symcat.h"
#include "safe-ctype.h"
#include "opcodes/vc4-desc.h"
#include "opcodes/vc4-opc.h"
#include "cgen.h"
#include <inttypes.h>
#include <limits.h>
#include <assert.h>
#include <stdbool.h>
#ifdef OBJ_ELF
#include "elf/vc4.h"
#endif

typedef struct
{
  const CGEN_INSN *insn;
  CGEN_FIELDS fields;
#if CGEN_INT_INSN_P
  CGEN_INSN_INT buffer[CGEN_MAX_INSN_SIZE / sizeof (CGEN_INSN_INT)];
#define INSN_VALUE(buf) (*(buf))
#else
  unsigned char buffer [CGEN_MAX_INSN_SIZE];
#define INSN_VALUE(buf) (buf)
#endif
} vc4_insn;

const char vc4_comment_chars[] = ";";
const char line_comment_chars[]   = "#";
const char line_separator_chars[] = "@";
const char EXP_CHARS[]            = "eE";
const char FLT_CHARS[]            = "dD";


#define VC4_SHORTOPTS "m:"
const char * md_shortopts = VC4_SHORTOPTS;

struct option md_longopts[] =
{
  {NULL, no_argument, NULL, 0}
};
size_t md_longopts_size = sizeof (md_longopts);

unsigned long vc4_machine = 0; /* default */

int
md_parse_option (int c ATTRIBUTE_UNUSED, char * arg ATTRIBUTE_UNUSED)
{
  return 0;
}

void
md_show_usage (FILE * stream ATTRIBUTE_UNUSED)
{
}

static void
vc4_handle_case (int arg ATTRIBUTE_UNUSED)
{
  expressionS exp;
  char *p;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  if (is_it_end_of_statement ())
    {
      demand_empty_rest_of_line ();
      return;
    }

  expression (&exp);

  symbolS *sym = exp.X_add_symbol;
  offsetT off = exp.X_add_number;

  if (exp.X_op != O_constant && exp.X_op != O_symbol)
    {
      sym = make_expr_symbol (&exp);
      off = 0;
    }

  p = frag_more (1);
  frag_var (rs_machine_dependent, 1, 0, CASE_TBB, sym, off, p);

  demand_empty_rest_of_line ();
}

static void
vc4_handle_endswitch (int arg ATTRIBUTE_UNUSED)
{
  s_align_bytes (2);
}

/* The target specific pseudo-ops which we support.  */
const pseudo_typeS md_pseudo_table[] =
{
  { "case", vc4_handle_case, 0 },
  { "endswitch", vc4_handle_endswitch, 0 },
  { 0, 0, 0 }
};

void
md_begin (void)
{
  /* Initialize the `cgen' interface.  */

  /* Set the machine number and endian.  */
  gas_cgen_cpu_desc = vc4_cgen_cpu_open (CGEN_CPU_OPEN_MACHS, 0,
					 CGEN_CPU_OPEN_ENDIAN,
					 CGEN_ENDIAN_LITTLE,
					 CGEN_CPU_OPEN_END);
  vc4_cgen_init_asm (gas_cgen_cpu_desc);

  /* This is a callback from cgen to gas to parse operands.  */
  cgen_set_parse_operand_fn (gas_cgen_cpu_desc, gas_cgen_parse_operand);
}

void
md_assemble (char *str)
{
  vc4_insn insn;
  char *errmsg;

  /* Initialize GAS's cgen interface for a new instruction.  */
  gas_cgen_init_parse ();

  insn.insn = vc4_cgen_assemble_insn
    (gas_cgen_cpu_desc, str, & insn.fields, insn.buffer, & errmsg);

  if (!insn.insn)
    {
      as_bad ("%s", errmsg);
      return;
    }

  if (CGEN_INSN_ATTR_VALUE (insn.insn, CGEN_INSN_SWITCH))
    {
      char *f = frag_more (2);
      /* Switch/TBB/TBH insns won't get the right type of frag/fixup in
	 gas_cgen_finish_insn because they don't have explicit PC-relative
	 operands: do that ourselves here instead.  */
      frag_var (rs_machine_dependent, 2, 0, SWITCH_TBB, 0, 0, f);
      memcpy (f, insn.buffer, 2);
    }
  else
    /* Doesn't really matter what we pass for RELAX_P here.  */
    gas_cgen_finish_insn (insn.insn, insn.buffer,
			  CGEN_FIELDS_BITSIZE (& insn.fields), 1, NULL);
}

/* Return the bfd reloc type for OPERAND of INSN at fixup FIXP.
   Returns BFD_RELOC_NONE if no reloc type can be found.
   *FIXP may be modified if desired.  */

bfd_reloc_code_real_type
md_cgen_lookup_reloc (const CGEN_INSN *insn ATTRIBUTE_UNUSED,
		      const CGEN_OPERAND *operand,
		      fixS *fixP ATTRIBUTE_UNUSED)
{
  switch (operand->type)
    {
    case VC4_OPERAND_PCRELCC:
      return BFD_RELOC_VC4_REL7_MUL2;
    case VC4_OPERAND_PCREL10BITS:
      return BFD_RELOC_VC4_REL10_MUL2;
    case VC4_OPERAND_PCREL8BITS:
      return BFD_RELOC_VC4_REL8_MUL2;
    case VC4_OPERAND_OFFSET23BITS:
      return BFD_RELOC_VC4_REL23_MUL2;
    case VC4_OPERAND_OFFSET27BITS:
      return BFD_RELOC_VC4_REL27_MUL2;
    case VC4_OPERAND_MEM48PCREL27:
      return BFD_RELOC_VC4_REL27;
    case VC4_OPERAND_ALU48PCREL:
      return BFD_RELOC_VC4_REL32;
    case VC4_OPERAND_ALU16IMM:
      return BFD_RELOC_VC4_IMM5_1;
    case VC4_OPERAND_ALU48IMMU:
      return BFD_RELOC_VC4_IMM32;
    default:
      break;
    }

  fprintf (stderr, "Unknown operand type %d\n", (int) operand->type);

  return BFD_RELOC_NONE;
}

/* Write a value out to the object file, using the appropriate endianness.  */

void
md_number_to_chars (char * buf, valueT val, int n)
{
  number_to_chars_littleendian (buf, val, n);
}

char *
md_atof (int type, char * litP, int *  sizeP)
{
  return ieee_md_atof (type, litP, sizeP, TRUE);
}

valueT
md_section_align (segT segment, valueT size)
{
  int align = bfd_get_section_alignment (stdoutput, segment);
  return ((size + (1 << align) - 1) & (-1 << align));
}

symbolS *
md_undefined_symbol (char * name ATTRIBUTE_UNUSED)
{
  return NULL;
}

/* The subtypes available for relaxable instruction fields.
   The fields are:
   1) most positive reach of this state,
   2) most negative reach of this state,
   3) how many bytes this mode will add to the size of the current frag
   4) which index into the table to try if we can't fit into this one.  */

const relax_typeS md_relax_table[] =
{
  /* The first entry must be unused because an `rlx_more' value of zero ends
     each list.  */
  { 1, 1, 0, 0 },

  /* 16-bit, 32-bit and 48-bit immediate (unconditional) ALU ops.  */
  {         31,           0, 0, 2 }, /* 5-bit unsigned immediate.  */
  {     0x7fff,     -0x8000, 2, 3 }, /* 16-bit signed immediate.  */
  { 0x7fffffff, -0x80000000, 4, 0 }, /* 32-bit immediate.  */
  
  /* 16-bit and 32-bit conditional branch.  */
  {    126 - 2,    -128 - 2, 0, 5 }, /* 7-bit offset, left-shifted by 1.  */
  {   0x7ffffe,   -0x800000, 2, 0 }, /* 23-bit offset, left-shifted by 1.  */
  
  /* 32-bit and 48-bit LEA instructions.  GAS counts from the *end* of the
     instruction, hence -4.  */
  { 0x7fff - 4, -0x8000 - 4, 0, 7 }, /* 16-bit offset.  */
  { 0x7fffffff, -0x80000000, 2, 0 }, /* 32-bit offset.  */
  
  /* 32-bit and 48-bit add rd, rs, #imm instructions.  */
  {         31,         -32, 0, 9 }, /* 6-bit signed immediate.  */
  { 0x7fffffff, -0x80000000, 2, 0 }, /* 32-bit immediate.  */
  
  /* 32-bit conditional operation (two operands, with "always" condition).  */
  {         31,         -32, 0,  0 }, /* 6-bit signed immediate (cond'n).  */
  {         31,         -32, 0, 12 }, /* 6-bit signed immediate (always).  */
  { 0x7fffffff, -0x80000000, 2,  0 }, /* 32-bit immediate.  */

  /* "Switch" (tbb and tbh) insns.  Dummy entries.  */
  {          0,           0, 0, 14 }, /* tbb insn.  */
  {          0,           0, 0,  0 }, /* tbh insn.  */

  /* Case table entries for above.  */
  {       0xff,      -0x100, 0, 16 }, /* 8-bit case index.  */
  {     0xffff,    -0x10000, 1,  0 }  /* 16-bit case index.  */
};

static bool
binary_opcode_p (int opcode)
{
  switch (opcode)
    {
    case 0: /* MOV.  */
    case 1: /* CMN.  */
    case 8: /* NOT.  */
    case 10: /* CMP.  */
    case 12: /* BTST.  */
    case 25: /* NEG.  */
    case 27: /* MSB.  */
    case 29: /* BITREV.  */
    case 31: /* ABS.  */
      return true;

    default:
      ;
    }
  return false;
}

int
md_estimate_size_before_relax (fragS *fragP, segT segment)
{
  unsigned int firstword;
  unsigned char *opcode = (unsigned char *) fragP->fr_opcode;

#ifdef DEBUG
  fprintf (stderr, "md_estimate_size_before_relax: subtype=%d\n",
	   fragP->fr_subtype);
#endif

  /* We already know the subtype for these, and CASE_TB* has no opcode.  */
  if (fragP->fr_subtype == SWITCH_TBB
      || fragP->fr_subtype == SWITCH_TBH
      || fragP->fr_subtype == CASE_TBB
      || fragP->fr_subtype == CASE_TBH)
    return (fragP->fr_subtype == CASE_TBH) ? 1 : 0;

  firstword = (opcode[1] << 8) | opcode[0];

  /* A short conditional branch instruction.  Change to the correct subtype
     (an index into md_relax_table) before relaxing, because CGEN apparently
     just defaults to 1.  */
  if ((firstword & 0xf800) == 0x1800)
    fragP->fr_subtype = BCC_16BIT;

  /* A tbb/switch.b instruction.  */
  else if ((firstword & 0xffe0) == 0x0080)
    {
      fragP->fr_subtype = SWITCH_TBB;
      /* Actually we shouldn't reach here.  */
      abort ();
    }

  /* FIXME: Cases below here aren't used now.  */

  /* A 32-bit ALU operation with a 16-bit signed immediate operand.  */
  else if ((firstword & 0xfc00) == 0xb000)
    fragP->fr_subtype = ALUOP_32BIT;

  /* A condition-always 32-bit ALU two-operand operation with a 6-bit signed
     immediate operand.  */
  else if ((firstword & 0xfc00) == 0xc000)
    {
      unsigned int secondword = (opcode[3] << 8) | opcode[2];
      if (((secondword >> 7) & 0xf) == 0xe
          && binary_opcode_p ((firstword >> 5) & 0x1f))
        fragP->fr_subtype = DECONDBINOP_32BIT;
      else
        fragP->fr_subtype = CONDBINOP_32BIT;
    }

  /* A 32-bit LEA instruction.  */
  if ((firstword & 0xffe0) == 0xbfe0)
    fragP->fr_subtype = LEA_32BIT;

  else if ((firstword & 0xffe0) == 0xc040)
    fragP->fr_subtype = ADD_32BIT;

  /* Undefined symbols can't be relaxed by the assembler, and should use the
     biggest insn type available (until they can be relaxed by the linker).  */
  if (S_GET_SEGMENT (fragP->fr_symbol) != segment
      || S_IS_EXTERNAL (fragP->fr_symbol)
      || S_IS_WEAK (fragP->fr_symbol))
    {
      const CGEN_INSN *insn;
      int i;
      bool found = false;

      switch (fragP->fr_subtype)
	{
	case ALUOP_16BIT:
	case ALUOP_32BIT:
	case ALUOP_48BIT:
	  fragP->fr_subtype = ALUOP_48BIT;
	  break;
	case BCC_16BIT:
	case BCC_32BIT:
	  fragP->fr_subtype = BCC_32BIT;
	  break;
	case LEA_32BIT:
	case LEA_48BIT:
	  fragP->fr_subtype = LEA_48BIT;
	  break;
        case ADD_32BIT:
          fragP->fr_subtype = ADD_48BIT;
          break;
        case CONDBINOP_32BIT:
          /* There's no wider insn for conditionals other than 'always'.  */
          break;
        case DECONDBINOP_32BIT:
          fragP->fr_subtype = DECONDBINOP_48BIT;
          break;
	default:
	  abort ();
	}

      for (i = 0, insn = fragP->fr_cgen.insn; ; i++, insn++)
        {
          if (strcmp (CGEN_INSN_MNEMONIC (insn),
		      CGEN_INSN_MNEMONIC (fragP->fr_cgen.insn)) == 0
	      && CGEN_INSN_ATTR_VALUE (insn, CGEN_INSN_RELAXED))
	    {
	      found = true;
	      break;
	    }
	}

      if (!found)
        abort ();

      fragP->fr_cgen.insn = insn;
    }

  return md_relax_table[fragP->fr_subtype].rlx_length;
}

long
md_pcrel_from_section (fixS *fixP, segT sec)
{
  if (fixP->fx_addsy != (symbolS *) NULL
      && (! S_IS_DEFINED (fixP->fx_addsy)
	  || S_GET_SEGMENT (fixP->fx_addsy) != sec
          || S_IS_EXTERNAL (fixP->fx_addsy)
          || S_IS_WEAK (fixP->fx_addsy)))
    {
      if (S_GET_SEGMENT (fixP->fx_addsy) != sec
	  && S_IS_DEFINED (fixP->fx_addsy)
	  && ! S_IS_EXTERNAL (fixP->fx_addsy)
	  && ! S_IS_WEAK (fixP->fx_addsy))
	return fixP->fx_offset;

      /* The symbol is undefined (or not defined in this section).  */
      return 0;
    }

  return fixP->fx_frag->fr_address + fixP->fx_where /*+ fixP->fx_size*/;
}

arelent *
vc4_tc_gen_reloc (asection *section ATTRIBUTE_UNUSED, fixS *fixp)
{
  arelent *rel;
  bfd_reloc_code_real_type r_type;

  if (fixp->fx_addsy && fixp->fx_subsy)
    {
      if ((S_GET_SEGMENT (fixp->fx_addsy) != S_GET_SEGMENT (fixp->fx_subsy))
	  || S_GET_SEGMENT (fixp->fx_addsy) == undefined_section)
	{
	  as_bad_where (fixp->fx_file, fixp->fx_line,
			_("Difference of symbols in different sections is not supported"));
	  return NULL;
	}
    }

  rel = xmalloc (sizeof (arelent));
  rel->sym_ptr_ptr = xmalloc (sizeof (asymbol *));
  *rel->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  rel->address = fixp->fx_frag->fr_address + fixp->fx_where;
  rel->addend = fixp->fx_offset;

  r_type = fixp->fx_r_type;

#ifdef DEBUG
  fprintf (stderr, "vc4_tc_gen_reloc: %s\n", bfd_get_reloc_code_name (r_type));
#endif

  rel->howto = bfd_reloc_type_lookup (stdoutput, r_type);

  if (rel->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("Cannot represent relocation type %s"),
		    bfd_get_reloc_code_name (r_type));
      return NULL;
    }

  return rel;
}

int
vc4_cgen_parse_fix_exp (int opinfo, expressionS *exp ATTRIBUTE_UNUSED)
{
  return opinfo;
}

fixS *
vc4_cgen_record_fixup_exp (fragS *frag,
			   int where,
			   const CGEN_INSN *insn,
			   int length,
			   const CGEN_OPERAND *operand,
			   int opinfo,
			   expressionS *exp)
{
  fixS *fixP = gas_cgen_record_fixup_exp (frag, where, insn, length, operand,
					  opinfo, exp);
#ifdef DEBUG
  fprintf (stderr, "vc4_cgen_record_fixup_exp\n");
#endif
  return fixP;
}

static struct frag *last_switch = NULL;

long
vc4_relax_frag (asection *sec, struct frag *frag, long stretch)
{
  long growth;

#ifdef DEBUG
  fprintf (stderr, "vc4_relax_frag (subtype=%d) ", (int) frag->fr_subtype);
#endif

  growth = relax_frag (sec, frag, stretch);

#ifdef DEBUG
  fprintf (stderr, "Grow by %d\n", growth);
#endif

  if (frag->fr_subtype == SWITCH_TBB || frag->fr_subtype == SWITCH_TBH)
    last_switch = frag;
  else if (frag->fr_subtype == CASE_TBB || frag->fr_subtype == CASE_TBH)
    {
      if (!last_switch)
	{
	  as_bad (_(".case without preceding switch instruction"));
	  return 0;
	}

      if (growth > 0)
        frag->fr_subtype = CASE_TBH;
      else if (frag->fr_subtype == CASE_TBB
	  && last_switch->fr_subtype == SWITCH_TBH)
	{
	  frag->fr_subtype = CASE_TBH;
	  growth++;
	}

      if (frag->fr_subtype == CASE_TBH)
        last_switch->fr_subtype = SWITCH_TBH;
    }
  else
    last_switch = NULL;

  return growth;
}

void
vc4_prepare_relax_scan (fragS *fragP, offsetT address, offsetT *aim)
{
  if (fragP->fr_subtype == CASE_TBB || fragP->fr_subtype == CASE_TBH)
    (*aim) += address;
}

void
vc4_apply_fix (fixS *fixP, valueT *valP, segT seg)
{
#ifdef DEBUG
  fprintf (stderr, "vc4_apply_fix\n");
#endif
  gas_cgen_md_apply_fix (fixP, valP, seg);
}

static int
target_address_for (fragS *frag)
{
  int rv = frag->fr_offset;
  symbolS *sym = frag->fr_symbol;

  if (sym)
    rv += S_GET_VALUE (sym);

  return rv;
}

void
md_convert_frag (bfd *headers ATTRIBUTE_UNUSED, segT seg, fragS *fragP)
{
  unsigned char *opcode = (unsigned char *) fragP->fr_opcode;
  unsigned int firstword;
  int where = fragP->fr_opcode - fragP->fr_literal;
  int extension;
  int operand = -1;
  int addend;

#ifdef DEBUG
  fprintf (stderr, "md_convert_frag, subtype=%d, fr_fix=%d, fr_var=%d, "
	   "opcode=%p\n", fragP->fr_subtype, fragP->fr_fix, fragP->fr_var,
	   opcode);
#endif

  firstword = (opcode[1] << 8) | opcode[0];

  addend = target_address_for (fragP) - (fragP->fr_address + where);

  switch (fragP->fr_subtype)
    {
    case ALUOP_16BIT:
      extension = 0;
      break;
    case ALUOP_32BIT:
      {
        unsigned int op5, dstreg;

        /* 16-bit scalar immediate insns.  */
        if ((firstword & 0xe000) == 0x6000)
          {
            op5 = ((firstword >> 9) & 0xf) << 1;
            dstreg = firstword & 0xf;
          }
        else
          abort ();

        /* Rebuild as a 32-bit immediate insn.  */
        firstword = 0xb000 | (op5 << 5) | dstreg;
        opcode[0] = firstword & 0xff;
        opcode[1] = (firstword >> 8) & 0xff;
        operand = VC4_OPERAND_OFFSET16;
        extension = 2;
      }
      break;
    case ALUOP_48BIT:
      {
	unsigned int op5, dstreg;

	/* 16-bit scalar immediate insns.  */
	if ((firstword & 0xe000) == 0x6000)
	  {
	    op5 = ((firstword >> 9) & 0xf) << 1;
	    dstreg = firstword & 0xf;
	  }
	/* 32-bit scalar immediate insns.  */
	else if ((firstword & 0xfc00) == 0xb000)
	  {
	    op5 = (firstword >> 5) & 0x1f;
	    dstreg = firstword & 0x1f;
	  }
	else
	  abort ();

	/* Rebuild as a 48-bit immediate insn.  */
	firstword = 0xe800 | (op5 << 5) | dstreg;
	opcode[0] = firstword & 0xff;
	opcode[1] = (firstword >> 8) & 0xff;
        opcode[2] = opcode[3] = 0;
	operand = VC4_OPERAND_ALU48IMMU;
	/* !!! It's not clear why this doesn't depend on the starting size
	   of the insn?  */
	extension = 4;
      }
      break;
    case BCC_16BIT:
      extension = 0;
      break;
    case BCC_32BIT:
      {
	unsigned int condcode;
	/* 16-bit conditional branches.  */
	gas_assert ((firstword & 0xf800) == 0x1800);
	condcode = (firstword >> 7) & 0xf;
	/* Rebuild as 32-bit conditional branch insn.  */
	firstword = 0x9000 | (condcode << 8);
	opcode[0] = firstword & 0xff;
	opcode[1] = (firstword >> 8) & 0xff;
	extension = 2;
	operand = VC4_OPERAND_OFFSET23BITS;
      }
      break;
    case LEA_32BIT:
      extension = 0;
      break;
    case LEA_48BIT:
      {
	unsigned int dstreg;
	/* 32-bit LEA instruction.  */
	gas_assert ((firstword & 0xffe0) == 0xbfe0);
	dstreg = firstword & 0x1f;
	/* Rebuild as 48-bit LEA instruction.  */
	firstword = 0xe500 | dstreg;
	opcode[0] = firstword & 0xff;
	opcode[1] = (firstword >> 8) & 0xff;
        opcode[2] = opcode[3] = 0;
	extension = 2;
	operand = VC4_OPERAND_ALU48PCREL;
      }
      break;
    case ADD_32BIT:
      extension = 0;
      break;
    case ADD_48BIT:
      {
        unsigned int dstreg, srcreg;
        unsigned int secondword;

        secondword = (opcode[3] << 8) | opcode[2];

        /* 32-bit ADD rd, rs, #imm instruction.  */
        if ((firstword & 0xffe0) == 0xc040)
          {
            gas_assert (((secondword >> 7) & 0xf) == 0xe);
            dstreg = firstword & 0x1f;
            srcreg = (secondword >> 1) & 0x1f;
          }
        else
          abort ();

        /* Rebuild as 48-bit ADD instruction.  */
        firstword = 0xec00 | (srcreg << 5) | dstreg;
        opcode[0] = firstword & 0xff;
        opcode[1] = (firstword >> 8) & 0xff;
        opcode[2] = opcode[3] = 0;
        extension = 2;
        operand = VC4_OPERAND_ALU48IMMU;
      }
      break;
    case CONDBINOP_32BIT:
      as_bad ("bad use of conditional insn");
      return;
    case DECONDBINOP_32BIT:
      extension = 0;
      break;
    case DECONDBINOP_48BIT:
      {
        unsigned int op5, dstreg, srcreg;
        unsigned int secondword = (opcode[3] << 8) | opcode[2];

        /* 32-bit OP<cond> rd, #imm instruction, where cond==always.  */
        if ((firstword & 0xfc00) == 0xc000
            && ((secondword >> 7) & 0xf) == 0xe
            && binary_opcode_p ((firstword >> 5) & 0x1f))
          {
            op5 = (firstword >> 5) & 0x1f;
            dstreg = firstword & 0x1f;
            srcreg = (secondword >> 11) & 0x1f;
          }
        else
          abort ();

        gas_assert (srcreg == dstreg || srcreg == 0 || dstreg == 0);

        /* Rebuild as 48-bit instruction.  */
        firstword = 0xe800 | (op5 << 5) | srcreg | dstreg;
        opcode[0] = firstword & 0xff;
        opcode[1] = (firstword >> 8) & 0xff;
        opcode[2] = opcode[3] = 0;
        extension = 2;
        operand = VC4_OPERAND_ALU48IMMU;
      }
      break;
    case SWITCH_TBB:
      extension = 0;
      break;
    case CASE_TBB:
      addend = target_address_for (fragP);
      extension = 0;
      break;
    case SWITCH_TBH:
      extension = 0;
      /* Rewrite as TBH.  */
      firstword &= 0x001f;
      firstword |= 0x00a0;
      opcode[0] = firstword & 0xff;
      opcode[1] = (firstword >> 8) & 0xff;
      break;
    case CASE_TBH:
      addend = target_address_for (fragP);
      extension = 1;
      break;
    default:
      abort ();
    }

  if (fragP->fr_subtype != SWITCH_TBB
      && fragP->fr_subtype != SWITCH_TBH
      && fragP->fr_subtype != CASE_TBB
      && fragP->fr_subtype != CASE_TBH
      && (S_GET_SEGMENT (fragP->fr_symbol) != seg
	  || S_IS_EXTERNAL (fragP->fr_symbol)
	  || S_IS_WEAK (fragP->fr_symbol)))
    {
      gas_assert (fragP->fr_cgen.insn != 0);

      gas_cgen_record_fixup (fragP, where, fragP->fr_cgen.insn,
			     (fragP->fr_fix - where) * 8,
			     cgen_operand_lookup_by_num (gas_cgen_cpu_desc,
							 operand),
			     fragP->fr_cgen.opinfo, fragP->fr_symbol,
			     fragP->fr_offset);
    }
  else
    /* A relaxable insn, but all bets are off when the relaxation machinery is
       active.  We have to put the operand bytes into the frag ourselves!  */
    switch (fragP->fr_subtype)
      {
      case ALUOP_16BIT: /* 5-bit unsigned immediate (in 16-bit instruction).  */
	opcode[0] = (opcode[0] & 0x0f) | ((addend & 0xf) << 4);
	opcode[1] = (opcode[1] & 0xfe) | ((addend >> 4) & 1);
        break;
      case ALUOP_48BIT: /* 32-bit immediate (in 48-bit instruction).  */
      case LEA_48BIT: /* 32-bit PC-relative offset (in 48-bit LEA insn).  */
      case ADD_48BIT: /* 32-bit immediate (in 48-bit add instruction).  */
	opcode[2] = addend & 0xff;
	opcode[3] = (addend >> 8) & 0xff;
	opcode[4] = (addend >> 16) & 0xff;
	opcode[5] = (addend >> 24) & 0xff;
        break;
      case BCC_16BIT: /* 7-bit PC-relative offset (in 16-bit words).  */
	opcode[0] = (opcode[0] & 0x80) | ((addend >> 1) & 0x7f);
	break;
      case BCC_32BIT: /* 23-bit PC-relative offset (in 16-bit words).  */
	opcode[2] = (addend >> 1) & 0xff;
	opcode[3] = (addend >> 9) & 0xff;
	opcode[0] = (addend >> 17) & 0x7f;
        break;
      case ALUOP_32BIT: /* 16-bit signed immediate (in 32-bit instruction).  */
      case LEA_32BIT: /* 16-bit PC-relative offset (in 32-bit LEA insn).  */
	opcode[2] = addend & 0xff;
	opcode[3] = (addend >> 8) & 0xff;
        break;
      case ADD_32BIT: /* 6-bit signed immediate (in 32-bit instruction).  */
        opcode[2] = (opcode[2] & 0xc0) | (addend & 0x3f);
        break;
      case SWITCH_TBB:
      case SWITCH_TBH:
        break;
      case CASE_TBH:
        opcode[1] = (addend >> 9) & 0xff;
	/* Fallthrough.  */
      case CASE_TBB:
        opcode[0] = (addend >> 1) & 0xff;
	break;
      default:
	abort ();
      }

  fragP->fr_fix += extension;
}

