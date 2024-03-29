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



#ifdef USE_PCH
#include "be_com_pch.h"
#endif /* USE_PCH */
#pragma hdrstop
#include "defs.h"
#include "config.h"
#include "config_opt.h"
#include "stab.h"
#include "wn.h"
#include "opt_points_to.h"
#include "opt_sym.h"
#include "opt_util.h"

#if defined(_GCC_NO_PRAGMAWEAK) || defined(__CYGWIN__)
  AUX_ID WN_aux(const WN*) { return (AUX_ID)0; }
#endif

#if defined(__linux__) || defined(__CYGWIN__)
  extern AUX_ID (*WN_aux_p) (const WN*);
#define WN_aux (*WN_aux_p)

// Solaris CC workaround
// Solaris CC #pragma weak can be either followed by one mangled name in
// string form, or followed by two mangled names such that B will replace
// A in case A is not defined.

#elif !defined(__GNUC__) && defined(_SOLARIS_SOLARIS)
#pragma weak "__1cGWN_aux6FpknCWN__I_"

#else  // in case of GNU g++, have to define these two, not sure why there are two
#pragma weak WN_aux__GPC2WN
#pragma weak WN_aux__FPC2WN
#endif // __linux__


// ************************************************************************
//
//  Define POINTS_TO operations
//
// ************************************************************************


// This table is used to implement two functions
//  Same_base() and Different_base()
//
// Notice that Same_Base() is not the negation of Different_base().
// 
// Same_base() returns TRUE if the base are the same and returns
// FALSE if doesn't know.
//
// Different_base() returns TRUE if the base are different and returns
// FALSE if doesn't know.
//
//
// Given two POINTS_TO and their base_kind, the base action table determines
// what kind of comparisons should be used on the Base() field.
// 
//   NO_INFO:     no conclusion can be drawn.
//   SAME_BASE:   the base are the same if p1.Base() == p2.Base()
//   COMP_BASE:   the base are the same if p1.Base() == p2.Base()
//                the base are different if p1.Base() != p2.Base()
//   DIFF:        the base are always different.
//
//
enum BASE_ACTION {
  NO_INFO   = 0,          // base cannot be compared
  SAME_BASE = 0x1,        // base are same <== p1.Base() == p2.Base(), base might be the same even if p1.Base() != p2.Base()
  COMP_BASE = 0x2,        // base can be compared for equality and inequality
  DIFF      = 0x3         // base always diff
};

static const BASE_ACTION
base_action_tbl[MAX_BASE_KIND][MAX_BASE_KIND] = {
  // INVALID,FIXED,       DYNAMIC,   UNKNOWN
  //---------------------------------------------------------
  {  NO_INFO, NO_INFO,	  NO_INFO,   NO_INFO}, // INVALID
  {  NO_INFO, COMP_BASE,  NO_INFO,   NO_INFO}, // FIXED
  {  NO_INFO, NO_INFO,	  SAME_BASE, NO_INFO}, // DYNAMIC
  {  NO_INFO, NO_INFO,	  NO_INFO,   NO_INFO}, // UNKNOWN
};
  

//  TRUE:  base are the same
//  FALSE: don't know
//
BOOL POINTS_TO::Same_base(const POINTS_TO *pt) const
{
  BASE_ACTION a = base_action_tbl[this->Base_kind()][pt->Base_kind()];
  if ((a == COMP_BASE || a == SAME_BASE) &&
      this->Base() == pt->Base())
    return TRUE;
  return FALSE;
}


//  TRUE:  base are different
//  FALSE: don't know
//
BOOL POINTS_TO::Different_base(const POINTS_TO *pt) const
{
  BASE_ACTION a = base_action_tbl[this->Base_kind()][pt->Base_kind()];
  if (a == DIFF) {
    return TRUE;
  } else if (a == COMP_BASE && this->Base() != pt->Base()) {
    return TRUE;
  }
  return FALSE;
}


//  TRUE:  the access might overlap.
//  FALSE: the access doesn't overlap.
//  
//   The base must be the same, otherwise it is meaningless to call this function.
//
BOOL POINTS_TO::Overlap(const POINTS_TO *p) const
{
  // can only compare fixed value.  Maybe it should be enhanced to deal with
  // symbolic constants in the future.
  //
  if (this->Ofst_kind() != OFST_IS_FIXED || 
         p->Ofst_kind() != OFST_IS_FIXED)
    return TRUE;

  if (this->Base_is_fixed() && ST_sclass(this->Base()) == SCLASS_REG) {
    Is_True(ST_sclass(p->Base()) == SCLASS_REG,  ("calling overlap() with different bases."));
    return this->Byte_Ofst() == p->Byte_Ofst();
  }

  if (this->Bit_Size() == 0 && p->Bit_Size() == 0) {
    if (this->Byte_Ofst() <= p->Byte_Ofst()) {
      if (this->Byte_Ofst() + this->Byte_Size() > p->Byte_Ofst())
	return TRUE;
    } else {
      if (p->Byte_Ofst() + p->Byte_Size() > this->Byte_Ofst())
	return TRUE;
    }
  } else {
    // one of them is a bit field
    INT64 ofst1 = this->Byte_Ofst() * 8 + this->Bit_Ofst();
    INT64 ofst2 = p->Byte_Ofst() * 8 + p->Bit_Ofst();
    UINT64 size1 =
      this->Bit_Size () ? this->Bit_Size () : this->Byte_Size() * 8;
    UINT64 size2 = p->Bit_Size () ? p->Bit_Size() : p->Byte_Size() * 8;
    
    if (ofst1 <= ofst2) {
      if (ofst1 + size1 > ofst2)
	return TRUE;
    } else {
      if (ofst2 + size2 > ofst1)
	return TRUE;
    }
  }
  return FALSE;
}

void POINTS_TO::Meet_info_from_alias_class(const POINTS_TO *pt)
{
  if (Alias_class() == OPTIMISTIC_AC_ID) {
    Set_alias_class(pt->Alias_class());
  }
  else if ((pt->Alias_class() != OPTIMISTIC_AC_ID) &&
	   (pt->Alias_class() != Alias_class())) {
    Set_alias_class(PESSIMISTIC_AC_ID);
  }
  if (Ip_alias_class() == OPTIMISTIC_AC_ID) {
    Set_ip_alias_class(pt->Ip_alias_class());
  }
  else if ((pt->Ip_alias_class() != OPTIMISTIC_AC_ID) &&
	   (pt->Ip_alias_class() != Ip_alias_class())) {
    Set_ip_alias_class(PESSIMISTIC_AC_ID);
  }
    
  if (!pt->Not_alloca_mem())   Reset_not_alloca_mem();
}

//  Combine *this and *pt in a conservative manner.
//  Put results in *this.
// 
void POINTS_TO::Meet(const POINTS_TO *pt, ST *definition)
{
  // Skip things being processed.  This can only happen if the use-def
  // chain contains a cycle (i.e., the pointer is an induction
  // variable). In that case, we give up on the offset and size
  // (because they are the variant part of the pointer expression).
  // We will try to determine if the base is defined outside of the
  // loop.
  if (pt->Expr_kind() == EXPR_IS_BEING_PROCESSED) {
    Set_ofst_kind(OFST_IS_UNKNOWN);
    return;
  }

  // EXPR_IS_ANY is a placeholder used to accumulate the alias
  // information from all the operands of a PHI node.
  if (this->Expr_kind() == EXPR_IS_ANY) {
    this->Copy_non_sticky_info(pt);
    return;
  }

  // Is_True(pt->Expr_kind() != EXPR_IS_ANY, ("Should not see EXPR_IS_ANY."));
  if (pt->Expr_kind() == EXPR_IS_ANY)
    return;

  // Check based_sym; based sym is sticky and required to be
  // consistent if either of the sticky attributes (restricted,
  // unique_pt) is set.
  if (pt->Based_sym() != Based_sym()) {
    FmtAssert(pt->Based_sym() == NULL ||
	      (!Unique_pt() && !Restricted()),
	      ("POINTS_TO::Meet: Inconsistent Based_sym()"));
    if (!Unique_pt() && !Restricted()) {
      Set_based_sym(NULL);
    }
  }
    
  // If expression is different, give up!!!
  if ((pt->Expr_kind() != Expr_kind()) &&
      (!Unique_pt() && !Restricted())) {
    Set_expr_kind(EXPR_IS_UNKNOWN);
    Set_base_kind(BASE_IS_UNKNOWN);
    Set_ofst_kind(OFST_IS_UNKNOWN);
    Set_alias_class(PESSIMISTIC_AC_ID);
    Set_ip_alias_class(PESSIMISTIC_AC_ID);
    Reset_attr();
    return;
  }

  // If both are address expression, try to combine the offset and size field.
  //
  if (pt->Expr_kind() == EXPR_IS_ADDR) {
    
    // See if they points to the same variable (base)
    if (!Same_base(pt)) {
      // bases are not the same (may be different).
      Set_base(definition);             // set base to this phi statement.
      Set_ofst_kind(OFST_IS_UNKNOWN);   // offset is useless if base is not the same.
      Set_base_kind(BASE_IS_UNKNOWN);
    } else {
      // base are the same, combine their offsets
      // both are pointer or both are ranges.
      if (this->Ofst_kind() == OFST_IS_FIXED && 
	  pt->Ofst_kind() == OFST_IS_FIXED &&
	  this->Is_pointer() == pt->Is_pointer()) {
	mINT64 byte_lower = MIN (this->Byte_Ofst(), pt->Byte_Ofst());
	mINT64 byte_upper = MAX (this->Byte_Ofst() + this->Byte_Size(),
				 pt->Byte_Ofst() + pt->Byte_Size()); 
	if (this->Bit_Size() != 0 && pt->Bit_Size() != 0) {
	  // both are bit fields
	  if (byte_upper - byte_lower > 8) {
	    // range to large to hold both bit fields
	    Set_bit_ofst_size (0, 0);
	  } else {
	    INT64 bits_lower = MIN (this->Byte_Ofst() * 8 + this->Bit_Ofst(),
				    pt->Byte_Ofst() * 8 + pt->Bit_Ofst());
	    INT64 bits_upper =
	      MAX (this->Byte_Ofst() * 8 + this->Bit_Ofst() + this->Bit_Size(),
		   pt->Byte_Ofst() * 8 + pt->Bit_Ofst() + pt->Bit_Size());
	    Set_bit_ofst_size (bits_lower - byte_lower * 8,
			       bits_upper - bits_lower);
	  }
	}
	Set_byte_ofst(byte_lower);
	Set_byte_size(byte_upper - byte_lower);
      } else
	Set_ofst_kind(OFST_IS_UNKNOWN);
    }
  }

  Meet_info_from_alias_class(pt);

  // update declaration fields
  if (!pt->Not_addr_saved())   Reset_not_addr_saved();
  if (!pt->Not_addr_passed())  Reset_not_addr_passed();
  if (!pt->Local())            Reset_local();
  if (!pt->Global())           Reset_global();
  if (!pt->Named())            Reset_named();
  if (!pt->Const())            Reset_const();
  if (!pt->F_param())          Reset_F_param();
  if (!pt->No_alias())         Reset_no_alias();
  if (!pt->Is_pointer())       Reset_is_pointer();
  if (!pt->Safe_to_speculate()) Reset_safe_to_speculate();
  if (!pt->Not_f90_target())   Reset_not_f90_target();

  // update weak symbol
  if (pt->Weak())              Set_weak();
  if (pt->Weak_base())         Set_weak_base();
  
  if (pt->Ty() != Ty())        Set_ty((TY_IDX) 0);

  if (pt->Known_f90_pointer() && Known_not_f90_pointer()) {
    DevWarn("Alias analysis: f90 pointer meets non-f90 pointer");
    Reset_known_not_f90_pointer();
  }
  if (pt->Known_not_f90_pointer() && Known_f90_pointer()) {
    DevWarn("Alias analysis: f90 pointer meets non-f90 pointer");
    Reset_known_f90_pointer();
  }

  CHECK_POINTS_TO(this);
}


// Input parameter: st and st_ofst
// output parameter: base and base_ofst (that is equivalent to st)
//
void
Expand_ST_into_base_and_ofst(ST *st, INT64 st_ofst, ST **base, INT64 *ofst)
{
  // cannot follow the base_st of a PREG!
  // or text
  // or sclass formal (because the incomplete data layout)
  // or mergable blocks (e.g., .lit4)
  // or preemptible symbols
  if (ST_sclass(st) == SCLASS_REG || 
      ST_sclass(st) == SCLASS_TEXT ||
      (ST_class(st) == CLASS_BLOCK && STB_merge(st)) ||
      ((Gen_PIC_Shared || Gen_PIC_Call_Shared) && ST_is_preemptible(st)) )
  {
    *ofst = st_ofst;
    *base = st;
    return;
  }

  INT64   tmpofst = 0;
  ST     *tmpbase = st;

  while (ST_base(tmpbase) != tmpbase) {

    // pv 345133:  not split up when base of symbol is mergable block
    if (ST_class(tmpbase) == CLASS_BLOCK && STB_merge(tmpbase)) {
      *ofst = st_ofst;
      *base = st;
      return;
    }

    // pv 458112, 380316 and 466728: 
    // not split up when base of symbol is CLASS_BLOCK and SCLASS_FORMAL,
    // i.e., Formal_Arg_StkSeg
    if (ST_sclass(tmpbase) == SCLASS_FORMAL &&
	ST_class(ST_base(tmpbase)) == CLASS_BLOCK)
      break;

    // several places in wopt assumed that ST_sclass(st) == ST_sclass(ST_base(st))
    // and it is not always true.  
    if (ST_sclass(tmpbase) == SCLASS_FORMAL &&
      	ST_sclass(tmpbase) != ST_sclass(ST_base(tmpbase)))
      break;	

    // pv 559974
    // At -O1 with regions it is possible to have some STs lowered and
    // others not, if it is already lowered, don't modify.
    if (ST_sclass(ST_base(tmpbase)) == SCLASS_UNKNOWN)
      break;

    // pv 571261:
    // do not expand a func symbol into .text+offset
    if (ST_sclass(tmpbase) == SCLASS_TEXT)
      break;

    tmpofst += ST_ofst(tmpbase);
    tmpbase = ST_base(tmpbase); 
  }

  // pv 345133:  not split up when base of symbol is mergable block
  // may not reach the test inside the while-loop because
  // the ST with CLASS_BLOCK's base is itself!
  if (ST_class(tmpbase) == CLASS_BLOCK && STB_merge(tmpbase)) {
    *ofst = st_ofst;
    *base = st;
    return;
  }

  *ofst = tmpofst + st_ofst;
  *base = tmpbase;
}


//  Build the POINTS_TO for a scalar
//  The POINTS_TO contains fully lowered base and ofst.
//
//    Warning:  this routine is not complete.  Strong symbols are not recorded in
//    the symbol table, so a two pass method need to be used to find all strong
//    symbols.
//    
void
POINTS_TO::Analyze_ST(ST *st, INT64 byte_ofst, INT64 byte_size,
		      UINT8 bit_ofst, UINT8 bit_size, TY_IDX ty,
		      BOOL has_equiv)
{
  Init();
  Set_ty(ty);

  Set_expr_kind(EXPR_IS_ADDR);
  Set_ofst_kind(OFST_IS_FIXED);
  Set_base_kind(BASE_IS_FIXED);
  Set_named();   // this variable has a name

  // Fix 363108: simple scalar and structure can be speculated
  if (ty != (TY_IDX)0) {
    switch (TY_kind (ty)) {
    case KIND_SCALAR:
    case KIND_POINTER:
    case KIND_STRUCT:
      Set_safe_to_speculate();
      break;
    default:
      break;
    }
  }
  
  // get to its base
  ST  *base;
  INT64 ofst;
  Expand_ST_into_base_and_ofst(st, byte_ofst, &base, &ofst);
  Set_base(base);
  Set_byte_ofst(ofst);
  Set_byte_size(byte_size);
  Set_bit_ofst_size(bit_ofst, bit_size);

  if (ST_class(st) == CLASS_CONST || 
      ST_class(st) == CLASS_FUNC ||
      (ST_class(st) == CLASS_VAR && ST_is_const_var (st))) {

    Set_const();
    // doesn't really need to set no alias, but harmless to do so.
    Set_no_alias();  
    Set_not_addr_saved();
    Set_not_addr_passed();

  } else {    // all the ST macro are broken for CLASS_CONST!

    BOOL not_addr_used_locally = FALSE;

    if (ST_class(st) == CLASS_VAR) {
      if (!ST_is_f90_target(st)) {
	Set_not_f90_target();
      }
    }
    else {
      Set_not_f90_target();
    }

    if (Not_f90_target()) {
      // This item is accessed directly (LDID/STID), not through an f90
      // pointer.
      Set_known_not_f90_pointer();
    }

    ST *sclass_st = st;

    switch (ST_sclass(sclass_st)) {
    case SCLASS_UNKNOWN:
      Is_True(FALSE, ("Analyze_ST::Storage class for ST %s is unknown.", ST_name(st)));
      break;
    case SCLASS_AUTO:
    case SCLASS_FORMAL_REF:   
      if (!ST_addr_saved(st))
	Set_not_addr_saved();
      if (!BE_ST_addr_passed(st))
	  Set_not_addr_passed();
      if (!BE_ST_addr_used_locally(st))
	not_addr_used_locally = TRUE;

      if (CURRENT_SYMTAB != ST_IDX_level (ST_st_idx (st))) // nested_procedure
	Set_global();
      else
	Set_local();
      // if (TY_is_restrict(ST_type(st)))
      // Set_ST_pt_to_unique_mem(st);

      // Fix 704324: optional argument cannot be speculated
      // Fix 707375: argument cannot be speculated in the presence of
      //             altentry, since some entries may not define it
      if (ST_is_optional_argument(st) ||
	  (ST_sclass(sclass_st) == SCLASS_FORMAL_REF &&
	   PU_has_altentry(Get_Current_PU())))
	Reset_safe_to_speculate();
      break;
    case SCLASS_FORMAL:
      Set_formal();

//   Fix 615048: after CG lowering, the variable is written into, so
//   we cannot set the const flag. 
//
//      if (Is_FORTRAN() && !ST_is_value_parm(st)) 
//	Set_const();   // make sure the addr is not modified by calls.

      if (!ST_addr_saved(st))
	Set_not_addr_saved();
      if (!BE_ST_addr_passed(st))
	Set_not_addr_passed();
      if (!BE_ST_addr_used_locally(st))
	not_addr_used_locally = TRUE;
      if (CURRENT_SYMTAB != ST_IDX_level (ST_st_idx (st)))
	Set_global();
      else
	Set_local();  
      // if (TY_is_restrict(ST_type(st)))
      // Set_ST_pt_to_unique_mem(st);
      break;
    case SCLASS_FSTATIC:  // FILE static
      Set_not_auto();
      Set_global();
      if (!ST_addr_saved(st))
	Set_not_addr_saved();
      if (!BE_ST_addr_passed(st))
	Set_not_addr_passed();
      if (!BE_ST_addr_used_locally(st))
	not_addr_used_locally = TRUE;
      // if (TY_is_restrict(ST_type(st)))
      //  Set_ST_pt_to_unique_mem(st);
      break;
    case SCLASS_PSTATIC:  // PU static
      if (ST_is_private_local(st))
	Set_local();
      else
	Set_global();
      if (!ST_addr_saved(st))
	Set_not_addr_saved();
      if (!BE_ST_addr_passed(st))
	Set_not_addr_passed();
      if (!BE_ST_addr_used_locally(st))
	not_addr_used_locally = TRUE;
      // if (TY_is_restrict(ST_type(st)))
      // Set_ST_pt_to_unique_mem(st);
      break;
    case SCLASS_UGLOBAL:
    case SCLASS_DGLOBAL:  // public globals
    case SCLASS_COMMON:
      Set_not_auto();
      // fall thru
    case SCLASS_EXTERN:
      Set_global();
      if (OPT_IPA_addr_analysis &&
	  PU_ipa_addr_analysis(Get_Current_PU())) {
	if (!ST_addr_saved(st))
	  Set_not_addr_saved();
	if (!BE_ST_addr_passed (st))
	  Set_not_addr_passed();
	if (!BE_ST_addr_used_locally(st))
	  not_addr_used_locally = TRUE;
      }

      if (ST_is_weak_symbol (st)) { 
	Set_weak();
	// because the weak sym base may be addr taken/passed!
	Reset_not_addr_saved();
	Reset_not_addr_passed();  
	Reset_safe_to_speculate();
      }
      // if (TY_is_restrict(ST_type(st)))
      //  Set_ST_pt_to_unique_mem(st);
      break;
    case SCLASS_REG:
      // neither address taken nor global
      // set up for calls that may modify dedicated registers
      Set_no_alias();
      Set_not_addr_saved();
      Set_not_addr_passed();
      Set_safe_to_speculate();
      Set_local();
      if (byte_ofst <= Last_Dedicated_Preg_Offset)
	Set_dedicated();
      break;
    case SCLASS_TEXT:
      // taking the address of a function
      break;
    default:
      FmtAssert(FALSE, ("Handle ST class %d.", ST_sclass(st)));
    }

    //  Partial fix 363108.  With IPA, a global variable that is not
    //  address saved/passed/used does not need to have virtual
    //  variable in the chi list.

    if (/* !Global() && */
	Not_addr_saved() &&
	Not_addr_passed() &&
	not_addr_used_locally &&
	!has_equiv)
      Set_no_alias();
  }
}


//  Generate the POINTS_TO for an indirect reference based on ST
//  This makes sense only if it is restricted, unique or it is a
//  Fortran parameter.
//
void POINTS_TO::Analyze_ST_as_base(ST *st, INT64 ofst, TY_IDX ty)
{
  Init();
  Set_expr_kind(EXPR_IS_UNKNOWN);
  Set_base_kind(BASE_IS_UNKNOWN);
  Set_ofst_kind(OFST_IS_UNKNOWN);

  if (ty != (TY_IDX)NULL && TY_kind(ty) == KIND_POINTER) 
    Set_expr_kind(EXPR_IS_ADDR); 
  else
    return;

  if (ST_class(st) != CLASS_VAR) return;

  if (ofst == 0) {
    if (TY_is_restrict(ST_type(st))) {
      Set_restricted();
      Set_based_sym(st);
      Set_expr_kind(EXPR_IS_ADDR);
      Set_base_kind(BASE_IS_UNKNOWN);
      Set_ofst_kind(OFST_IS_UNKNOWN);
    } else if (ST_pt_to_unique_mem(st)) {
      Set_unique_pt();
      Set_based_sym(st);
      Set_expr_kind(EXPR_IS_ADDR);
      Set_base_kind(BASE_IS_UNKNOWN);
      Set_ofst_kind(OFST_IS_UNKNOWN);
    } else if (Alias_Pointer_Cray && st != NULL && !ST_is_temp_var(st)) {
      Set_unique_pt();
      Set_based_sym(st);
      Set_expr_kind(EXPR_IS_ADDR);
      Set_base_kind(BASE_IS_UNKNOWN);
      Set_ofst_kind(OFST_IS_UNKNOWN);
    }
    if (Alias_Pointer_Parms && Is_FORTRAN() &&
	ST_sclass(st) == SCLASS_FORMAL && !ST_is_value_parm(st)) {
      Set_F_param();
      Set_based_sym(st);
      Set_expr_kind(EXPR_IS_ADDR);
      Set_base_kind(BASE_IS_UNKNOWN);
      Set_global();
      Set_named();   // For the Ragnarok option
    }
    Set_ofst_kind(OFST_IS_FIXED);
    Set_byte_ofst(0);
    Set_byte_size(0); 

  } else {

    // ofst != 0.  Special case for LNO.
    // When the pt_to_unique_mem is set for an array A
    // *A[i] is not aliased with anything else,
    // but *A[i] and *A[j] are alised.
    //
    if (ST_pt_to_unique_mem(st)) {
      Set_unique_pt();
      Set_based_sym(st);
      Set_expr_kind(EXPR_IS_ADDR);
      Set_base_kind(BASE_IS_UNKNOWN);
      Set_ofst_kind(OFST_IS_UNKNOWN);
    }
    Set_ofst_kind(OFST_IS_FIXED);
    Set_byte_ofst(0);
    Set_byte_size(0); 
  }
}

// lower the ST in the POINTS_TO in the base symbol
// Use the WN node to determine the access range.
// If WN is NULL, use the Mongoose symbol table ST_size() instead.
//
void POINTS_TO::Lower_to_base(WN *wn)
{
  if (Expr_kind() != EXPR_IS_ADDR) {
    // erase useful fields.
    Set_base_kind(BASE_IS_UNKNOWN);
    Set_ofst_kind(OFST_IS_UNKNOWN);
    Reset_safe_to_speculate();
    return;
  }

  if (Base_is_fixed()) {
    ST     *base = Base();  // LDA contains the non-lowered ST 
    mINT64 ofst = 0;
    mINT64 size;
    if (((ST_class(base) != CLASS_VAR) &&
	 (ST_class(base) != CLASS_CONST) &&
	 (ST_class(base) != CLASS_PREG)) ||
	(ST_type(base) == (TY_IDX) NULL)) {
      size = 0;
    }
    else {
      size = TY_size(ST_type(base));   // LDA contains the non-lowered ST
    }

    if (ST_sclass(base) == SCLASS_REG || ST_sclass(base) == SCLASS_TEXT) {
      Set_base_kind(BASE_IS_UNKNOWN);
      Reset_safe_to_speculate();
      return;
    }

    Expand_ST_into_base_and_ofst (base, 0, &base, &ofst);

    // if the pt already has an offset, shift it.
    mINT64 object_size = 0;
    if (Ofst_kind() == OFST_IS_FIXED && wn != NULL && 
	(object_size = WN_object_size(wn)) > 0) {
      Set_base(base);
      Shift_ofst(ofst);
      Is_True(Is_pointer(), ("Pt is not a pointer."));
      Set_byte_size( Byte_Size() + object_size);
    } else {
      // if no more info, the access range is from 0 to ST_size()
      Set_bit_ofst_size(0, 0);
      if (size > 0) {  // if ST_size is known.
	Set_ofst_kind(OFST_IS_FIXED);
	Set_base(base);
	Set_byte_ofst(ofst);
	Set_byte_size(size);
      } else {
	Set_base(base);
	Set_ofst_kind(OFST_IS_UNKNOWN);
      }
    }

    // Fix 368671:  if there is out bound array reference
    // do not analyze the ofst.
    if (Ofst_kind() == OFST_IS_FIXED) {
      if (Byte_Ofst() < ofst || Byte_Size() + Byte_Ofst() > size + ofst) {
	Reset_safe_to_speculate();
	if (!Alias_Common_Scalar) 
	  Set_ofst_kind(OFST_IS_UNKNOWN); // Destroy ofst info if access out of bound
	else
	  Set_byte_size(size);		// Set size to ST_size if access out of bound
	Set_bit_ofst_size(0, 0);
      }
      
    }

  } else if (Restricted() || Unique_pt()) { // has based_sym

    Reset_safe_to_speculate();
    ST *st = Based_sym();
    UINT64 size = st ? TY_size (ST_type (st)) : 0;
    if (size > 0) {
      Set_ofst_kind(OFST_IS_FIXED);
      Set_byte_ofst(0);
      Set_byte_size(size);
    } else
      Set_ofst_kind(OFST_IS_UNKNOWN);
    Set_bit_ofst_size(0, 0);
  } else {

    Reset_safe_to_speculate();
    if (Ofst_kind() == OFST_IS_FIXED && Is_pointer() && wn != NULL) {
      Set_byte_size(Byte_Size() + WN_object_size(wn));
    } else
      Set_ofst_kind(OFST_IS_UNKNOWN);
  }

  // already converted a pointer range into memory range
  Reset_is_pointer();

  Is_True(!(Ofst_kind() == OFST_IS_FIXED && Byte_Size() == 0),
        ("Ofst is fixed and size == 0."));
}


//  Determine the base symbol and offset from an LDA expression.
//  The POINTS_TO contains non-lowered st and st_ofst!
//
void POINTS_TO::Analyze_Lda_Base(WN *wn_lda, const OPT_STAB &opt_stab)
{
  Is_True(WN_operator(wn_lda) == OPR_LDA, ("WN operator is not LDA."));
  AUX_ID aux = WN_aux(wn_lda);

  // Copy the POINTS_TO from opt_stab directly, but changes it to 
  // a pointer and reduces the size to 0.
  //
  // Copy(opt_stab.Aux_stab_entry(aux)->Points_to());
  // cannot copy directly because the points_to might have already
  // been lowered!

  ST *st = opt_stab.Aux_stab_entry(aux)->St();
  INT64 st_ofst =  opt_stab.Aux_stab_entry(aux)->St_ofst();
  Copy_non_sticky_info(opt_stab.Aux_stab_entry(aux)->Points_to());
  Set_base(st);
  Set_byte_ofst(st_ofst);
  Set_byte_size( 0 );
  Set_is_pointer();
}


/*ARGSUSED*/
void POINTS_TO::Analyze_Lda_Base(WN *wn_lda, const STAB_ADAPTER &stab)
{
  Is_True(WN_operator(wn_lda) == OPR_LDA, ("WN operator is not LDA."));
  Analyze_ST(WN_st(wn_lda), WN_offset(wn_lda), 0, 0, 0, (TY_IDX)NULL, TRUE);
  Set_is_pointer();
  Set_byte_size(0);
  Set_bit_ofst_size(0,0);
}


void POINTS_TO::Analyze_WN_expr(WN *wn)
{
  Analyze_WN_expr(wn, STAB_ADAPTER());
}


ST *Is_nested_call(const WN *wn)
{
  return Is_nested_call(wn, STAB_ADAPTER());
}


// *******************************************************
//
//    Points To Lists
//
// *******************************************************

// prepend the points_to item to the list
void
POINTS_TO_LIST::Prepend( POINTS_TO *pt, MEM_POOL *pool )
{
  POINTS_TO_NODE *ptn = CXX_NEW( POINTS_TO_NODE(pt), pool );
  Prepend( ptn );
}



// Print the content of a POINTS_TO structure
//
void POINTS_TO::Print(FILE *fp) const
{
  BOOL pr_ofst = FALSE;
  BOOL pr_size = FALSE;

  switch (Expr_kind()) {
  case EXPR_IS_INVALID:
    fprintf(fp, "expr invalid\n");
    return;
  case EXPR_IS_INT:
    fprintf(fp, "expr is integer\n");
    return;
  case EXPR_IS_ANY:
    fprintf(fp, "expr is any, ");
    break;
  case EXPR_IS_UNKNOWN:
    fprintf(fp, "expr is unknown, ");
    break;
  case EXPR_IS_BEING_PROCESSED:
    fprintf(fp, "expr is being processed, ");
    break;
  }
  
  switch (Base_kind()) {
  case BASE_IS_INVALID:
    fprintf(fp, "base invalid, ");
    break;
  case BASE_IS_FIXED:
    fprintf(fp, "fixed %s, ", ST_class(Base()) != CLASS_CONST ?
	    ST_name(Base()) : "constant");
    pr_ofst = TRUE;
    break;
  case BASE_IS_DYNAMIC:
    fprintf(fp, "dynamic, ");
    pr_ofst = TRUE;
    break;
  case BASE_IS_UNKNOWN:
    fprintf(fp, "base unknown, ");
    break;
  }
  if (pr_ofst) {
    switch (Ofst_kind()) {
    case OFST_IS_INVALID:
      fprintf(fp, "ofst invalid, ");
      break;
    case OFST_IS_FIXED:
      fprintf(fp, "byte ofst is %lld, ", Byte_Ofst());
      if (Bit_Size() != 0)
	fprintf(fp, "bit ofst is %d, ", Bit_Ofst());
      pr_size = TRUE;
      break;
    case OFST_IS_UNKNOWN:
      fprintf(fp, "ofst unknown, ");
      break;
    }
  }
  if (pr_size) {
    fprintf(fp, "byte size is %lld, ", Byte_Size());
    if (Bit_Size() != 0)
      fprintf(fp, "bit size is %d, ", Bit_Size());
  }
  fprintf(fp, "per-PU class %d, ", Alias_class());
  fprintf(fp, "global class %d, ", Ip_alias_class());
  
  // print attributes
  fprintf(fp, "attr=");
  char *pr_separator = "";
  if (Not_addr_saved()) {
    fprintf(fp, "%snot_addr_saved", pr_separator);
    pr_separator = "|";
  }
  if (Not_addr_passed()) {
    fprintf(fp, "%snot_addr_passed", pr_separator);
    pr_separator = "|";
  }
  if (Local()) {
    fprintf(fp, "%slocal", pr_separator);
    pr_separator = "|";
  }
  if (Global()) {
    fprintf(fp, "%sglobal", pr_separator);
    pr_separator = "|";
  }
  if (Named()) {
    fprintf(fp, "%snamed", pr_separator);
    pr_separator = "|";
  }
  if (Const()) {
    fprintf(fp, "%sconst", pr_separator);
    pr_separator = "|";
  }
  if (Restricted()) {
    fprintf(fp, "%srestricted", pr_separator);
    pr_separator = "|";
  }
  if (Unique_pt()) {
    fprintf(fp, "%sunique_pt", pr_separator);
    pr_separator = "|";
  }
  if (F_param()) {
    fprintf(fp, "%sf_param", pr_separator);
    pr_separator = "|";
  }
  if (Dedicated()) {
    fprintf(fp, "%sdedicated", pr_separator);
    pr_separator = "|";
  }
  if (No_alias()) {
    fprintf(fp, "%sno_alias", pr_separator);
    pr_separator = "|";
  }
  if (Weak()) {
    fprintf(fp, "%sweak", pr_separator);
    pr_separator = "|";
  }
  if (Weak_base()) {
    fprintf(fp, "%sweak_base", pr_separator);
    pr_separator = "|";
  }
  if (Is_pointer()) {
    fprintf(fp, "%sis_pointer", pr_separator);
    pr_separator = "|";
  }
  if (Safe_to_speculate()) {
    fprintf(fp, "%ssafe_to_speculate", pr_separator);
    pr_separator = "|";
  }
  if (Not_auto()) {
    fprintf(fp, "%snot_auto", pr_separator);
    pr_separator = "|";
  }
  if (Known_f90_pointer()) {
    fprintf(fp, "%sf90_pointer", pr_separator);
    pr_separator = "|";
  }
  if (Known_not_f90_pointer()) {
    fprintf(fp, "%snot_f90_pointer", pr_separator);
    pr_separator = "|";
  }
  if (Not_f90_target()) {
    fprintf(fp, "%snot_f90_target", pr_separator);
    pr_separator = "|";
  }
  if (Default_vsym()) {
    fprintf(fp, "%sdef_vsym", pr_separator);
    pr_separator = "|";
  }

#ifdef _LP64
#define UNDEFINED_PTR    (void *)0xa5a5a5a5a5a5a5a5LL
#else
#define UNDEFINED_PTR    (void *)0xa5a5a5a5
#endif /* _LP64 */

  if (Based_sym())
    fprintf(fp, ", based_sym=%s(%d)\n", (Based_sym() == UNDEFINED_PTR) ?
	    "*UNDEFINED*" : ST_name(Based_sym()), Based_sym_depth());
  else
    fprintf(fp, ", based_sym=null\n");
}


#ifdef Is_True_On
//  Validate alias information
void CHECK_POINTS_TO(POINTS_TO *pt)
{
  if ( pt->Expr_kind() != EXPR_IS_ANY
    && pt->Expr_kind() != EXPR_IS_BEING_PROCESSED)
  {
    FmtAssert ( pt->Expr_kind() != EXPR_IS_INVALID,
		("CHECK_POINTS_TO:  pt is EXPR_IS_INVALID.") );
    if ( pt->Expr_kind() == EXPR_IS_ADDR ) {
      FmtAssert ( pt->Base_kind() != BASE_IS_INVALID,
		  ("CHECK_POINTS_TO:  pt is BASE_IS_INVALID.") );
    }
  }

  // verify that all except sticky attr are reset if expr is not an
  // address
  if (pt->Expr_kind() != EXPR_IS_ADDR) {
    if (pt->Not_addr_saved() ||
	pt->Not_addr_passed() ||
	pt->Local() ||
	pt->Global() ||
	pt->Named() ||
	pt->Const() ||
	// pt->Restricted() ||  //  <-- sticky -- don't check
	// pt->Unique_pt() ||   //  <-- sticky -- don't check
	pt->F_param() ||
	pt->No_alias())
      FmtAssert ( FALSE,
		  ("CHECK_POINTS_TO:  pt has invalid attributes.") );
  }
}
#endif

