/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if 
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston MA 02111-1307, USA.

  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/


/* CGEXP routines for loads and stores */
#include <elf.h>
#include <vector.h>
#include "defs.h"
#include "em_elf.h"
#include "erglob.h"
#include "ercg.h"
#include "tracing.h"
#include "config.h"
#include "config_targ2.h"
#include "config_debug.h"
#include "xstats.h"
#include "topcode.h"
#include "tn.h"
#include "cg_flags.h"
#include "targ_isa_lits.h"
#include "op.h"
#include "stblock.h"
#include "data_layout.h"
#include "strtab.h"
#include "symtab.h"
#include "cg.h"
#include "cgexp.h"
#include "cgexp_internals.h"

void
Expand_Lda (TN *dest, TN *src, OPS *ops)
{
  FmtAssert(FALSE, ("NYI: Expand_Lda"));
}

static ISA_ENUM_CLASS_VALUE 
Get_ldhint (const char *s, ISA_ENUM_CLASS_VALUE default_val)
{
  ISA_ENUM_CLASS_VALUE ldhint = default_val;
  if (s) {
    if (strcmp(s, "nt1") == 0) {
      ldhint = ECV_ldhint_nt1;
    } else if (strcmp(s, "nta") == 0) {
      ldhint = ECV_ldhint_nta;
    } else if (strcmp(s, "none") == 0) {
      ldhint = ECV_ldhint;
    } else {
      fprintf(stderr, "bad ldhint specification: '%s';"
		      "must be 'nt1', 'nta' or 'none'\n",
		      s);
    }
  }
  return ldhint;
}

static ISA_ENUM_CLASS_VALUE
Pick_Load_Hint (VARIANT variant)
{
  UINT32 pf_flags = V_pf_flags(variant);
  static ISA_ENUM_CLASS_VALUE ldhint_L1;
  static ISA_ENUM_CLASS_VALUE ldhint_L2;
  static BOOL hints_inited = FALSE;
  if (!hints_inited) {
    ldhint_L1 = Get_ldhint(CGEXP_ldhint_L1, ECV_ldhint);
    ldhint_L2 = Get_ldhint(CGEXP_ldhint_L2, ECV_ldhint);
    hints_inited = TRUE;
  }

  ISA_ENUM_CLASS_VALUE ldhint;
  if (PF_GET_STRIDE_1L(pf_flags)) {
    ldhint = ldhint_L1;
  } else if (PF_GET_STRIDE_2L(pf_flags)) {
    ldhint = ldhint_L2;
  } else {
    ldhint = ECV_ldhint;
  }

  return ldhint;
}

static ISA_ENUM_CLASS_VALUE
Pick_Load_Type (VARIANT variant, TYPE_ID mtype)
{
  if (MTYPE_is_float(mtype)) {
    return ECV_fldtype;
  } else if (V_volatile(variant)) {
    return ECV_ldtype_acq;
  } else {
    return ECV_ldtype;
  }
}

static TOP
Pick_Load_Instruction (TYPE_ID rtype, TYPE_ID desc)
{
  switch (desc) {
  case MTYPE_I1: case MTYPE_U1:	return TOP_ld1;
  case MTYPE_I2: case MTYPE_U2:	return TOP_ld2;
  case MTYPE_I4: case MTYPE_U4:	return TOP_ld4;
  case MTYPE_I8: case MTYPE_U8:	return TOP_ld8;
  case MTYPE_F4: 		return TOP_ldfs;
  case MTYPE_F8: 		return TOP_ldfd;
  case MTYPE_F10: 		return TOP_ldfe;
  case MTYPE_F16: 		return TOP_ldf_fill;

  case MTYPE_V:
    if (rtype != MTYPE_V) {
      // use rtype to pick load (e.g. if lda)
      return Pick_Load_Instruction(rtype,rtype);
    }
    /*FALLTHROUGH*/

  default:  
    FmtAssert(FALSE, ("NYI: Pick_Load_Instruction mtype"));
    /*NOTREACHED*/
  }
}

void
Expand_Load (OPCODE opcode, TN *result, TN *base, TN *ofst, 
	     VARIANT variant, OPS *ops)
{
  TYPE_ID mtype = OPCODE_desc(opcode);
  TOP top = Pick_Load_Instruction (OPCODE_rtype(opcode), mtype);
  ISA_ENUM_CLASS_VALUE ldhint = Pick_Load_Hint (variant);
  Is_True (TN_is_constant(ofst), ("Illegal load offset TN"));
  // Don't generate an Add if the offset is 0.
  if (!TN_has_value(ofst) || (TN_value(ofst) != 0)) {
    // expand load o(b) into add t=o,b; load t
    TN *tmp = Build_TN_Like (base);
    Expand_Add (tmp, ofst, base, Pointer_Mtype, ops);
    base = tmp;
  }
  if (top == TOP_ldf_fill) {
    Build_OP (top, result, True_TN, Gen_Enum_TN(ldhint), base, ops);
  } else {
    ISA_ENUM_CLASS_VALUE ldtype = Pick_Load_Type (variant, mtype);
    Build_OP (top, result, True_TN, 
	      Gen_Enum_TN(ldtype), Gen_Enum_TN(ldhint), 
	      base, ops);
  }
}

static ISA_ENUM_CLASS_VALUE
Get_sthint (const char *s, ISA_ENUM_CLASS_VALUE default_val)
{
  ISA_ENUM_CLASS_VALUE sthint = default_val;
  if (s) {
    if (strcmp(s, "nta") == 0) {
      sthint = ECV_sthint_nta;
    } else if (strcmp(s, "none") == 0) {
      sthint = ECV_sthint;
    } else {
      fprintf(stderr, "bad sthint specification: '%s';"
		      "must be 'nta' or 'none'\n",
		      s);
    }
  }
  return sthint;
}

static ISA_ENUM_CLASS_VALUE
Pick_Store_Hint (VARIANT variant)
{
  UINT32 pf_flags = V_pf_flags(variant);
  static ISA_ENUM_CLASS_VALUE sthint_L1;
  static ISA_ENUM_CLASS_VALUE sthint_L2;
  static BOOL hints_inited = FALSE;
  if (!hints_inited) {
    sthint_L1 = Get_sthint(CGEXP_sthint_L1, ECV_sthint);
    sthint_L2 = Get_sthint(CGEXP_sthint_L2, ECV_sthint);
    hints_inited = TRUE;
  }

  ISA_ENUM_CLASS_VALUE sthint;
  if (PF_GET_STRIDE_1L(pf_flags)) {
    sthint = sthint_L1;
  } else if (PF_GET_STRIDE_2L(pf_flags)) {
    sthint = sthint_L2;
  } else {
    sthint = ECV_sthint;
  }

  return sthint;
}

static ISA_ENUM_CLASS_VALUE
Pick_Store_Type (VARIANT variant)
{
  return V_volatile(variant) ? ECV_sttype_rel : ECV_sttype;
}

static TOP
Pick_Store_Instruction (TYPE_ID mtype)
{
  switch (mtype) {
  case MTYPE_I1: case MTYPE_U1:	return TOP_st1;
  case MTYPE_I2: case MTYPE_U2:	return TOP_st2;
  case MTYPE_I4: case MTYPE_U4:	return TOP_st4;
  case MTYPE_I8: case MTYPE_U8:	return TOP_st8;
  case MTYPE_F4: 		return TOP_stfs;
  case MTYPE_F8: 		return TOP_stfd;
  case MTYPE_F10: 		return TOP_stfe;
  case MTYPE_F16: 		return TOP_stf_spill;
  default:
    FmtAssert(FALSE, ("NYI: Pick_Store_Instruction mtype"));
    /*NOTREACHED*/
  }
}

void
Expand_Store (TYPE_ID mtype, TN *src, TN *base, TN *ofst, 
	      VARIANT variant, OPS *ops)
{
  TOP top = Pick_Store_Instruction (mtype);
  ISA_ENUM_CLASS_VALUE sthint = Pick_Store_Hint (variant);
  Is_True (TN_is_constant(ofst), ("Illegal load offset TN"));
  // Don't generate an Add if the offset is 0.
  if (!TN_has_value(ofst) || (TN_value(ofst) != 0)) {
    // expand store o(b) into add t=o,b; store t
    TN *tmp = Build_TN_Like (base);
    Expand_Add (tmp, ofst, base, Pointer_Mtype, ops);
    base = tmp;
  }
  if (MTYPE_is_float(mtype)) {
    Build_OP (top, True_TN, Gen_Enum_TN(sthint), base, src, ops);
  } else {
    ISA_ENUM_CLASS_VALUE sttype = Pick_Store_Type (variant);
    Build_OP (top, True_TN, Gen_Enum_TN(sttype), Gen_Enum_TN(sthint),
	      base, src, ops);
  }
}

/* ====================================================================
 *
 * INT32 get_variant_alignment(TYPE_ID rtype, VARIANT variant)
 *
 * Given a variant compute a valid alignment
 * return gcd of m,n;
 * Used for alignment reasons;
 *
 * ==================================================================== */
static INT32 get_variant_alignment(TYPE_ID rtype, VARIANT variant)
{
  INT32 r;
  INT32 n= V_alignment(variant);
  INT32 m= MTYPE_alignment(rtype);

  while(r = m % n)
  {
    m=  n;
    n=  r;
  }
  return n;
}

static TYPE_ID 
Composed_Align_Type(TYPE_ID mtype, VARIANT variant, INT32 *alignment, INT32 *partials)
{
  *alignment =	get_variant_alignment(mtype, variant);
  *partials =	MTYPE_alignment(mtype) / *alignment;
  return Mtype_AlignmentClass( *alignment, MTYPE_CLASS_UNSIGNED_INTEGER);
}

static OPCODE 
OPCODE_make_signed_op(OPERATOR op, TYPE_ID rtype, TYPE_ID desc, BOOL is_signed)
{
  if (MTYPE_is_signed(rtype) != is_signed)
	rtype = MTYPE_complement(rtype);
  if (MTYPE_is_signed(desc) != is_signed)
	desc =	MTYPE_complement(desc);

  return OPCODE_make_op(op, rtype, desc);
}

/* ====================================================================
 *
 * Adjust_Addr_TNs
 *
 * We have a memory reference operation, with a base and displacement,
 * where the displacement is literal.  We want to create another memop
 * with the displacement modified by a small amount.
 *
 * WARNING:  If an add operation is required, it will be expanded here.
 *
 * ====================================================================
 */

static void
Adjust_Addr_TNs (
  TOP	opcode,		/* The new memory operation */
  TN	**base_tn,	/* The base address -- may be modified */
  TN	**disp_tn,	/* The displacement -- may be modified */
  INT16	disp,		/* A displacement to add */
  OPS *ops)
{

  if ( Potential_Immediate_TN_Expr (opcode, *disp_tn, disp) )
  {
    if ( TN_has_value(*disp_tn) ) {
      *disp_tn = Gen_Literal_TN ( TN_value(*disp_tn) + disp, 4 );
    } else {
      *disp_tn = Gen_Symbol_TN ( TN_var(*disp_tn),
				 TN_offset(*disp_tn) + disp, 0);
    }
  } else {
    TN *tmp = Build_TN_Of_Mtype (Pointer_Mtype);
    // because disp may be symbolic reloc on base,
    // want to still add it with base and create new base and disp.
    Expand_Add (tmp, *disp_tn, *base_tn, Pointer_Mtype, ops);
    *base_tn = tmp;
    *disp_tn = Gen_Literal_TN (disp, 4);
  }
}

static void
Expand_Composed_Load ( OPCODE op, TN *result, TN *base, TN *disp, VARIANT variant, OPS *ops)
{
  TYPE_ID rtype= OPCODE_rtype(op);
  TYPE_ID desc = OPCODE_desc(op);

  if (MTYPE_is_float(rtype))
  {
    TN     *load;

    switch (rtype) {
    case MTYPE_F4:
      load = Build_TN_Of_Mtype(MTYPE_I4);
      Expand_Composed_Load ( OPC_I4I4LDID, load, base, disp, variant, ops);
      // setf_s moves bits into fp reg.
      Build_OP ( TOP_setf_s, result, True_TN, load, ops );
      break;
    case MTYPE_F8:
      load = Build_TN_Of_Mtype(MTYPE_I8);
      Expand_Composed_Load ( OPC_I8I8LDID, load, base, disp, variant, ops);
      Build_OP ( TOP_setf_d, result, True_TN, load, ops );
      break;
    default:
      FmtAssert(FALSE, ("Expand_Composed_Load doesn't handle %s",
			MTYPE_name(rtype)));
      /*NOTREACHED*/
    }
    Reset_TN_is_fpu_int(result);
    return;
  }

  TOP		top;
  INT32		alignment, nLoads, i;
  OPCODE	new_opcode;
  TYPE_ID	new_desc;
  TN		*tmpV[8];

  new_desc =	Composed_Align_Type(desc, variant, &alignment, &nLoads);
  new_opcode =	OPCODE_make_signed_op(OPR_LDID, rtype, new_desc, FALSE);
  top = Pick_Load_Instruction (rtype, new_desc);

  Is_True(nLoads > 1, ("Expand_Composed_Load with nLoads == %d", nLoads));

 /* Generate the component loads, storing the result in a vector
  * of TNs. The vector is filled in such a way that the LSB is in
  * tmpV[0] so that later code can ignore the endianess of the target.
  */
  INT endian_xor = (Target_Byte_Sex == BIG_ENDIAN) ? (nLoads-1) : 0;
  for (i=0; i < nLoads; i++)
  {
    INT idx = i ^ endian_xor;
    tmpV[idx] = Build_TN_Of_Mtype(rtype);
    Expand_Load ( new_opcode, tmpV[idx], base, disp, variant, ops);
    if (i < nLoads-1) Adjust_Addr_TNs ( top, &base, &disp, alignment, ops);
  }

  /* Now combine the components into the desired value. The only
   * complication is that the form of the 'dep' instruction that we
   * need, supports a maximum length of 16 bits. Fortunately that
   * leaves just creating a 64-bit integer from two 32-bit pieces --
   * the mix4.r instruction handles that case.
   */
  INT nLoadBits = alignment * 8;
  if (nLoadBits <= 16) {
    TN *tmp0 = tmpV[0]; 
    for (i=1; i < (nLoads-1); i++)
    {
      TN *tmp= Build_TN_Of_Mtype(rtype);
      Build_OP(TOP_dep, tmp, True_TN, tmpV[i], tmp0,
	       Gen_Literal_TN(i*nLoadBits, 4), Gen_Literal_TN(nLoadBits, 4), ops);
      tmp0 = tmp;
    }
    Build_OP(TOP_dep, result, True_TN, tmpV[i], tmp0,
	     Gen_Literal_TN(i*nLoadBits, 4), Gen_Literal_TN(nLoadBits, 4), ops);
  } else {
    FmtAssert(nLoadBits == 32 && nLoads == 2,
	      ("Expand_Composed_Load: unexpected composition"));
    Build_OP(TOP_mix4_r, result, True_TN, tmpV[1], tmpV[0], ops);
  }
}

void
Expand_Misaligned_Load ( OPCODE op, TN *result, TN *base, TN *disp, VARIANT variant, OPS *ops)
{
  Expand_Composed_Load ( op, result, base, disp, variant, ops);
}


static void
Expand_Composed_Store (TYPE_ID mtype, TN *obj, TN *base, TN *disp, VARIANT variant, OPS *ops)
{
  if (MTYPE_is_float(mtype))
  {
    TN     *tmp;

    switch (mtype) {
    case MTYPE_F4:
      tmp = Build_TN_Of_Mtype(MTYPE_I4);
      Build_OP ( TOP_getf_s, tmp, True_TN, obj, ops );
      Expand_Composed_Store (MTYPE_I4, tmp, base, disp, variant, ops);
      break;
    case MTYPE_F8:
      tmp = Build_TN_Of_Mtype(MTYPE_I8);
      Build_OP ( TOP_getf_d, tmp, True_TN, obj, ops );
      Expand_Composed_Store (MTYPE_I8, tmp, base, disp, variant, ops);
      break;
    default:
      FmtAssert(FALSE, ("Expand_Composed_Store doesn't handle %s",
			MTYPE_name(mtype)));
      /*NOTREACHED*/
    }
    return;
  }

  TOP		top;
  INT32		alignment, nStores;
  TYPE_ID	new_desc;

  new_desc =	Composed_Align_Type(mtype, variant, &alignment, &nStores);
  top = Pick_Store_Instruction (new_desc);

  if (Target_Byte_Sex == BIG_ENDIAN)
    Adjust_Addr_TNs ( top, &base, &disp, MTYPE_alignment(mtype)-alignment, ops);
  Expand_Store ( new_desc, obj, base, disp, variant, ops); 

  while(--nStores >0)
  {
	TN *tmp = Build_TN_Of_Mtype(mtype);
	Expand_Shift( tmp, obj, Gen_Literal_TN(alignment*8, 4), mtype, shift_lright, ops);
	obj = tmp;

        if (Target_Byte_Sex == BIG_ENDIAN)
	  Adjust_Addr_TNs ( top, &base, &disp, -alignment, ops);
	else Adjust_Addr_TNs ( top, &base, &disp, alignment, ops);

	Expand_Store ( new_desc, obj, base, disp, variant, ops); 
  }
}

void
Expand_Misaligned_Store (TYPE_ID mtype, TN *obj_tn, TN *base_tn, TN *disp_tn, VARIANT variant, OPS *ops)
{
  Expand_Composed_Store (mtype, obj_tn, base_tn, disp_tn, variant, ops);
}

static void
Exp_Ldst (
  OPCODE opcode,
  TN *tn,
  ST *sym,
  INT64 ofst,
  BOOL indirect_call,
  BOOL is_store,
  BOOL is_load,
  OPS *ops,
  VARIANT variant)
{
  ST *base_sym;
  INT64 base_ofst;
  TN *base_tn;
  TN *ofst_tn;
  BOOL is_lda = (!is_load && !is_store);
  OPS newops;
  OP *op;
  OPS_Init(&newops);

  if (Trace_Exp2) {
        fprintf(TFile, "exp_ldst %s: ", OPCODE_name(opcode));
        if (tn) Print_TN(tn,FALSE);
	if (is_store) fprintf(TFile, " -> ");
	else fprintf(TFile, " <- ");
        fprintf(TFile, "%lld (%s)\n", ofst, ST_name(sym));
  }

  Allocate_Object(sym);         /* make sure sym is allocated */

  Base_Symbol_And_Offset_For_Addressing (sym, ofst, &base_sym, &base_ofst);

  if (ST_on_stack(sym)) {
	// formals on small stack are not assigned to sp/fp yet
	// cause base is not finished, but they will be assigned to SP.
	// So only use FP if already based on FP.
	base_tn = (base_sym == FP_Sym) ? FP_TN : SP_TN;
	if (sym == base_sym) {
		// can have direct reference to SP or FP,
		// e.g. if actual stored to stack.
		ofst_tn = Gen_Literal_TN (base_ofst, Pointer_Size);
	}
	else {
		/* Because we'd like to see symbol name in .s file, 
		 * still reference the symbol rather than the sp/fp base.  
		 * Do put in the offset from the symbol.  
		 * We put the symbol in the TN and then
		 * let cgemit replace symbol with the final offset.
		 * We generate a SW reg, <sym>, <SP> rather than SW reg,<sym>
		 * because cgemit and others expect a separate tn for the
		 * offset and base. 
		 */
        	ofst_tn = Gen_Symbol_TN (sym, ofst, 0);
	}
  }
  else if ((ST_class(base_sym) == CLASS_BLOCK || ST_class(base_sym)==CLASS_VAR)
	 && ST_gprel(base_sym)) 
  {
	// gp-relative reference
	PU_References_GP = TRUE;
	if (ISA_LC_Value_In_Class(base_ofst, LC_i22)) {
		base_tn = GP_TN;
    		ofst_tn = Gen_Symbol_TN (sym, ofst, TN_RELOC_IA_GPREL22);
    	} 
    	else {
		FmtAssert(FALSE, ("gp-relative offset doesn't fit in 22 bits"));
	}
  }
  else if (Guaranteed_Small_GOT) {
	// integer tmps cause are address tns
	TN *tmp1 = Build_TN_Of_Mtype (Pointer_Mtype);
	TN *tmp2 = Build_TN_Of_Mtype (Pointer_Mtype);
	base_tn = GP_TN;
	// first get address of LT entry
	if (ST_class(sym) == CLASS_FUNC && ! Get_Trace (TP_CGEXP, 256) ) {
		Expand_Add (tmp1, 
			Gen_Symbol_TN (base_sym, 0, TN_RELOC_IA_LTOFF_FPTR),
			base_tn, Pointer_Mtype, &newops);
	}
	else {
		Expand_Add (tmp1, 
			Gen_Symbol_TN (base_sym, 0, TN_RELOC_IA_LTOFF22),
			base_tn, Pointer_Mtype, &newops);
	}
	// then get address of var
	if (is_lda && base_ofst == 0) {
		// want to stop at address
		// (either that or add with 0)
		tmp2 = tn;
		is_lda = FALSE;	// so nothing done
	}
	// load is of address, not of result type
	Expand_Load (OPCODE_make_signed_op(OPR_LDID, 
			Pointer_Mtype, Pointer_Mtype, FALSE),
		tmp2, tmp1, Gen_Literal_TN (0, 4), variant, &newops);
	// got address should not alias
      	Set_OP_no_alias(OPS_last(&newops));
	base_tn = tmp2;
	// add offset to address
	ofst_tn = Gen_Literal_TN(base_ofst, 4);
  }
  else {
 	FmtAssert(FALSE, ("NYI: Exp_Ldst"));
  }

  if (is_store) {
	if (V_align_all(variant) == 0)
		Expand_Store (OPCODE_desc(opcode), tn, base_tn, ofst_tn, 
			variant, &newops);
  	else 
		Expand_Misaligned_Store (OPCODE_desc(opcode), tn, 
			base_tn, ofst_tn, variant, &newops);
  }
  else if (is_load) {
	if (V_align_all(variant) == 0)
		Expand_Load (opcode, tn, base_tn, ofst_tn, variant, &newops);
  	else 
		Expand_Misaligned_Load (opcode, tn, 
			base_tn, ofst_tn, variant, &newops);
  }
  else if (is_lda) {
	Expand_Add (tn, ofst_tn, base_tn, OPCODE_rtype(opcode), &newops);
  }

  FOR_ALL_OPS_OPs (&newops, op) {
  	if (is_load && ST_is_constant(sym) && OP_load(op)) {
  		// If we expanded a load of a constant, 
		// nothing else can alias with the loads 
		// we have generated.
      		Set_OP_no_alias(op);
	}
	if (Trace_Exp2) {
        	fprintf(TFile, "exp_ldst into "); Print_OP (op);
    	}
  }
  /* Add the new OPs to the end of the list passed in */
  OPS_Append_Ops(ops, &newops);
}

void Exp_Lda ( 
  TYPE_ID mtype, 
  TN *tgt_tn, 
  ST *sym, 
  INT64 ofst, 
  OPERATOR call_opr,
  OPS *ops)
{
  OPCODE opcode = OPCODE_make_op(OPR_LDA, mtype, MTYPE_V);
  Exp_Ldst (opcode, tgt_tn, sym, ofst, 
	(call_opr == OPR_ICALL),
	FALSE, FALSE, ops, V_NONE);
}

void
Exp_Load (
  TYPE_ID rtype, 
  TYPE_ID desc, 
  TN *tgt_tn, 
  ST *sym, 
  INT64 ofst, 
  OPS *ops, 
  VARIANT variant)
{
  OPCODE opcode = OPCODE_make_op (OPR_LDID, rtype, desc);
  Exp_Ldst (opcode, tgt_tn, sym, ofst, FALSE, FALSE, TRUE, ops, variant);
}

void
Exp_Store (
  TYPE_ID mtype, 
  TN *src_tn, 
  ST *sym, 
  INT64 ofst, 
  OPS *ops, 
  VARIANT variant)
{
  OPCODE opcode = OPCODE_make_op(OPR_STID, MTYPE_V, mtype);
  Exp_Ldst (opcode, src_tn, sym, ofst, FALSE, TRUE, FALSE, ops, variant);
}

static ISA_ENUM_CLASS_VALUE 
Get_lfhint (const char *s, ISA_ENUM_CLASS_VALUE default_val)
{
  ISA_ENUM_CLASS_VALUE lfhint = default_val;
  if (s) {
    if (strcmp(s, "nt1") == 0) {
      lfhint = ECV_lfhint_nt1;
    } else if (strcmp(s, "nt2") == 0) {
      lfhint = ECV_lfhint_nt2;
    } else if (strcmp(s, "nta") == 0) {
      lfhint = ECV_lfhint_nta;
    } else if (strcmp(s, "none") == 0) {
      lfhint = ECV_lfhint;
    } else {
      fprintf(stderr, "bad lfhint specification: '%s';"
		      "must be 'nt1', 'nt2', 'nta' or 'none'\n",
		      s);
    }
  }
  return lfhint;
}

static ISA_ENUM_CLASS_VALUE
Pick_Prefetch_Hint (VARIANT variant)
{
  UINT32 pf_flags = V_pf_flags(variant);
  static ISA_ENUM_CLASS_VALUE lfhint_L1;
  static ISA_ENUM_CLASS_VALUE lfhint_L2;
  static BOOL inited = FALSE;
  if (!inited) {
    lfhint_L1 = Get_lfhint(CGEXP_lfhint_L1, ECV_lfhint_nt2);
    lfhint_L2 = Get_lfhint(CGEXP_lfhint_L2, ECV_lfhint_nta);
    inited = TRUE;
  }

  /* Set the locality hint.
   *
   * NOTE: 
   *   There's some confusion about cache level numbers.
   *
   *   From LNO's point of view L0 caches (16KB each for instructions and 
   *   integer data) don't exist, L1 is the unified 96KB, and L2 is 2-4MB. 
   *
   *   According to Jim, in the hints for lfetch level 1 means L0 above,
   *   and level 2 means L1 above. Therefore, in Exp_Preftech
   */
  ISA_ENUM_CLASS_VALUE lfhint;
  if (PF_GET_STRIDE_1L(pf_flags)) {
    lfhint = lfhint_L1;
  } else if (PF_GET_STRIDE_2L(pf_flags)) {
    lfhint = lfhint_L2;
  } else {
    lfhint = ECV_lfhint;
  }

  return lfhint;
}

void Exp_Prefetch (TOP opc, TN* src1, TN* src2, VARIANT variant, OPS* ops)
{
  ISA_ENUM_CLASS_VALUE lfhint;
  UINT32 pf_flags = V_pf_flags(variant);
  FmtAssert(opc == TOP_UNDEFINED,
            ("Prefetch opcode should be selected in Exp_Prefetch"));
  opc =   (PF_GET_WRITE(pf_flags) || CG_exclusive_prefetch)
	? TOP_lfetch_excl : TOP_lfetch;
  lfhint = Pick_Prefetch_Hint(variant);

  // Don't generate an add if the offset is 0.
  if (!TN_is_zero(src2)) {
    TN* tmp = Build_TN_Like(src1);
    Expand_Add (tmp, src1, src2, Pointer_Mtype, ops);
    src1 = tmp;
  }

  Build_OP(opc, True_TN, Gen_Enum_TN(lfhint), src1, ops);
}

/* ======================================================================
 * Exp_Extract_Bits
 * ======================================================================*/
void
Exp_Extract_Bits (TYPE_ID rtype, TYPE_ID desc, UINT bit_offset, UINT bit_size,
		  TN *tgt_tn, TN *src_tn, OPS *ops)
{
  TOP extr_op = MTYPE_signed(rtype) ? TOP_extr : TOP_extr_u;
  UINT pos =   Target_Byte_Sex == BIG_ENDIAN
	     ? MTYPE_bit_size(desc)-bit_offset-bit_size : bit_offset;
  Build_OP(extr_op, tgt_tn, True_TN, src_tn, 
	   Gen_Literal_TN(pos, 4), Gen_Literal_TN(bit_size, 4), ops);
}

/* ======================================================================
 * Exp_Deposit_Bits - deposit src2_tn into a field of src1_tn returning
 * the result in tgt_tn.
 * ======================================================================*/
void
Exp_Deposit_Bits (TYPE_ID rtype, TYPE_ID desc, UINT bit_offset, UINT bit_size,
		  TN *tgt_tn, TN *src1_tn, TN *src2_tn, OPS *ops)
{
  FmtAssert(bit_size != 0, ("size of bit field cannot be 0"));

  UINT targ_bit_offset = bit_offset;
  if (Target_Byte_Sex == BIG_ENDIAN) {
    targ_bit_offset = MTYPE_bit_size(desc) - bit_offset - bit_size;
  }

  if (bit_size <= 16) {
    Build_OP(TOP_dep, tgt_tn, True_TN, src2_tn, src1_tn,
	     Gen_Literal_TN(targ_bit_offset, 4), Gen_Literal_TN(bit_size, 4), ops);
    return;
  }

  // bit_size > 16 requires 3 instructions
  TN *tmp1_tn = Build_TN_Like (src1_tn);
  TN *tmp2_tn = Build_TN_Like (src1_tn);
  Build_OP(TOP_dep_i, tmp1_tn, True_TN, Gen_Literal_TN(0, 4), src1_tn,
	   Gen_Literal_TN(targ_bit_offset, 4), Gen_Literal_TN(bit_size, 4), ops);
  Build_OP(TOP_dep_z, tmp2_tn, True_TN, src2_tn,
	   Gen_Literal_TN(targ_bit_offset, 4), Gen_Literal_TN(bit_size, 4), ops);
  Build_OP(TOP_or, tgt_tn, True_TN, tmp1_tn, tmp2_tn, ops);
}

void 
Expand_Lda_Label (TN *dest, TN *lab, OPS *ops)
{
	TN *tmp1 = Build_TN_Of_Mtype (Pointer_Mtype);
	Set_TN_is_reloc_ia_ltoff22(lab);
	// first get address of LT entry
	Expand_Add (tmp1, lab, GP_TN, Pointer_Mtype, ops);
	// then get address of var
	Expand_Load (
		// load is of address, not of result type
		OPCODE_make_op(OPR_LDID, Pointer_Mtype, Pointer_Mtype),
		dest, tmp1, Gen_Literal_TN (0, 4), V_NONE, ops);
}
