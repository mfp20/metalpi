/* tc-vc4.h -- Header file for tc-vc4.c.
   Copyright 2012 Free Software Foundation, Inc.
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

#include <inttypes.h>

#define TC_VC4

#define LISTING_HEADER "Vc4 GAS "

/* The target BFD architecture.  */
#define TARGET_ARCH bfd_arch_vc4

extern unsigned long vc4_machine;
#define TARGET_MACH (vc4_machine)

#define TARGET_FORMAT		"elf32-vc4"
#define TARGET_BYTES_BIG_ENDIAN	0

extern const char vc4_comment_chars [];
#define tc_comment_chars vc4_comment_chars

/* Permit temporary numeric labels.  */
#define LOCAL_LABELS_FB	1

#define DIFF_EXPR_OK	1	/* .-foo gets turned into PC relative relocs */

/* We don't need to handle .word strangely.  */
#define WORKING_DOT_WORD

/* Values passed to md_apply_fix don't include the symbol value.  */
#define MD_APPLY_SYM_VALUE(FIX) 0

#define ALUOP_16BIT        1
#define ALUOP_32BIT        2
#define ALUOP_48BIT        3
#define BCC_16BIT          4
#define BCC_32BIT          5
#define LEA_32BIT          6
#define LEA_48BIT          7
#define ADD_32BIT          8
#define ADD_48BIT          9
#define CONDBINOP_32BIT   10
#define DECONDBINOP_32BIT 11
#define DECONDBINOP_48BIT 12
#define SWITCH_TBB        13
#define SWITCH_TBH        14
#define CASE_TBB          15
#define CASE_TBH          16

/*struct vc4_param;
struct vc4_asm;

enum op_type
  {
    ot_unknown,
    ot_cpuid,
    ot_reg,
    ot_reg_range,
    ot_reg_addr,
    ot_reg_addr_pi,
    ot_reg_addr_pd,
    ot_reg_addr_offset,
    ot_2reg_addr_begin,
    ot_2reg_addr_end,
    ot_num,
    ot_reg_shl,
    ot_reg_lsr
  };

struct op_info
{
  enum op_type type;
  expressionS exp;
  int reg, num2;
};

struct vc4_frag_option
{
  const struct vc4_asm *as;
  const struct vc4_param *param;
  uint16_t ins[5];
  bfd_reloc_code_real_type bfd_fixup;
};

struct vc4_frag_type
{
  size_t num;
  size_t cur;
  struct op_info op_inf;
  struct vc4_frag_option d[3];
};*/

#include "write.h"

/*void vc4_init_frag(fragS *f);
void vc4_init_fix(fixS *f);*/

/*#define TC_FRAG_TYPE		struct vc4_frag_type
#define TC_FRAG_INIT(fragp)	vc4_init_frag(fragp)
#define TC_FIX_TYPE             int
#define TC_INIT_FIX_DATA(fixp)  vc4_init_fix(fixp)*/

/*#define md_apply_fix md_apply_fix*/

/*extern bfd_boolean vc4_fix_adjustable (struct fix *);
#define tc_fix_adjustable(FIX) vc4_fix_adjustable (FIX)*/

#define md_cgen_record_fixup_exp vc4_cgen_record_fixup_exp

#define tc_gen_reloc vc4_tc_gen_reloc
#define md_apply_fix vc4_apply_fix

/* Call md_pcrel_from_section(), not md_pcrel_from().  */
extern long md_pcrel_from_section (struct fix *, segT);
#define MD_PCREL_FROM_SECTION(FIX, SEC) md_pcrel_from_section (FIX, SEC)

/* GAS will call this function for any expression that can not be
   recognized.  When the function is called, `input_line_pointer'
   will point to the start of the expression.  */
#define md_operand(x)


#define md_relax_frag(SEG, FRAGP, STRETCH) \
  vc4_relax_frag ((SEG), (FRAGP), (STRETCH))
extern long vc4_relax_frag (asection *, struct frag *, long);

/*#define TC_CONS_FIX_NEW(FRAG, WHERE, NBYTES, EXP) \
  vc4_cons_fix_new (FRAG, WHERE, NBYTES, EXP)
extern void vc4_cons_fix_new (fragS *, int, int, expressionS *);*/

#define TC_CGEN_PARSE_FIX_EXP(opinfo, exp) \
  vc4_cgen_parse_fix_exp(opinfo, exp)
extern int vc4_cgen_parse_fix_exp (int, expressionS *);

extern const struct relax_type md_relax_table[];
#define TC_GENERIC_RELAX_TABLE md_relax_table

extern void vc4_prepare_relax_scan (fragS *, offsetT, offsetT *);
#define md_prepare_relax_scan(fragP, address, aim, this_state, this_type) \
  vc4_prepare_relax_scan (fragP, address, &aim)

#define TC_CGEN_MAX_RELAX(insn,len) 10

#define LISTING_WORD_SIZE 2

#define TARGET_USE_CFIPOP 1
#define DWARF2_LINE_MIN_INSN_LENGTH 2
#define DWARF2_DEFAULT_RETURN_COLUMN 26
#define DWARF2_CIE_DATA_ALIGNMENT (-4)
