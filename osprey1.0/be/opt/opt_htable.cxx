//-*-c++-*-
// ====================================================================
// ====================================================================
//
// Module: opt_htable.cxx
// $Revision: 1.1.1.1 $
// $Date: 2002-05-22 20:06:49 $
// $Author: dsystem $
// $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/opt/opt_htable.cxx,v $
//
// ====================================================================
//
// Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of version 2 of the GNU General Public License as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it would be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// Further, this software is distributed without any warranty that it
// is free of the rightful claim of any third person regarding
// infringement  or the like.  Any license provided herein, whether
// implied or otherwise, applies only to this software file.  Patent
// licenses, if any, provided herein do not apply to combinations of
// this program with other software, or any other product whatsoever.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
//
// Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
// Mountain View, CA 94043, or:
//
// http://www.sgi.com
//
// For further information regarding this notice, see:
//
// http://oss.sgi.com/projects/GenInfo/NoticeExplan
//
// ====================================================================
//
// Description: PLEASE fill me in.
//
// ====================================================================
// ====================================================================


#ifdef USE_PCH
#include "opt_pch.h"
#endif // USE_PCH
#pragma hdrstop


#include <math.h>			// for sqrt() used in htable stats

#include "defs.h"
#include "erglob.h"
#include "opcode.h"
#include "errors.h"
#include "mtypes.h"
#include "cxx_memory.h"
#include "wn_util.h"
#include "targ_const.h"			// for TCON-related stuff
#include "const.h"			// for symbol/TCON-related
#include "pf_cg.h"
#include "stblock.h"
#include "w2op.h"
#include "config_opt.h"         // for Delay_U64_Lowering

#include "opt_sys.h"            // BZERO definition
#include "opt_config.h"
#include "opt_wn.h"
#include "opt_util.h"
#include "opt_cfg.h"
#include "opt_sym.h"
#include "opt_htable.h"
#include "opt_ssa.h"
#include "opt_combine.h"
#include "opt_main.h"
#include "opt_mu_chi.h"
#include "opt_exc.h"
#include "opt_fold.h"
#include "config_targ.h"		// ISA info
#include "opt_prop.h"
#include "bb_node_set.h"
#include "opt_bb.h"
#include "opt_cvtl_rule.h"

#include <strings.h>   // bcopy

EXP_KIDS_ITER::EXP_KIDS_ITER(mUINT32 cnt, CODEREP **kp)
{
  kid_count = cnt;
  cur_idx = (mUINT32)NIL;
  kids = kp;
  FmtAssert(kids != NULL, ("EXPREP_KIDS_ITER(EXPREP): NULL operand list"));
}

// ====================================================================
// Check if a ivar is a volatile reference (or any part is volatile)
// ====================================================================

BOOL
CODEREP::Is_ivar_volatile( void ) const
{
  const OPERATOR  opr = Opr();
  const CODEREP  *ivar_vsym = Get_ivar_vsym();

  // Consider an Mload/Mstore/Iload/Istore associated with a volatile vsym
  // a volatile memory operation.
  //
  if (ivar_vsym != NULL && ivar_vsym->Is_var_volatile())
     return TRUE;

  switch ( opr ) {
  case OPR_MLOAD:
    {
      Is_True( TY_kind(Ilod_ty()) == KIND_POINTER,
	       ("Is_ivar_volatile: Ilod_ty() of MLOAD not pointer") );
      if (Ilod_TY_is_volatile(Ilod_ty()))
	return TRUE;
    }
    break;

  case OPR_ILOAD:
  case OPR_ILDBITS:
    {
      if ( TY_is_volatile(Ilod_ty()) )
	return TRUE;
	
      if ( TY_is_volatile(Ilod_base_ty()) )
	return TRUE;

      if ( TY_kind(Ilod_base_ty()) == KIND_POINTER ) 
	if (Ilod_TY_is_volatile(Ilod_base_ty()))
	  return TRUE;
    }
    break;

  case OPR_PARM:
    {
      if ( TY_is_volatile(Ilod_ty()) )
	return TRUE;
    }
    break;
  }

  return FALSE;
}

/* CVTL-RELATED start (correctness) */
static BOOL Sign_extended(MTYPE dtyp, MTYPE dsctyp)
{
  if (dsctyp == MTYPE_BS)
    return MTYPE_signed(dtyp);
  if (MTYPE_is_integral(dtyp) && MTYPE_is_integral(dsctyp) &&
      MTYPE_size_min(dsctyp) <= MTYPE_size_min(MTYPE_I4)) {
    if (MTYPE_size_min(dsctyp) == MTYPE_size_min(MTYPE_I4))
      return Is_hi_sign_extended(dtyp, dsctyp);
    else
      return Is_lo_sign_extended(dtyp, dsctyp);
  }
  return FALSE;
}

// ========================================================================
// Set the sign extension flag for LOAD/ILOAD
// ========================================================================
void
CODEREP::Set_sign_extension_flag(void)
{
  Is_True(inCODEKIND(Kind(), CK_VAR|CK_IVAR), 
	  ("CODEREP::Set_sign_extension_flag: illegal kind"));
  Assign_sign_extd(Sign_extended(Dtyp(), Dsctyp()));
}
/* CVTL-RELATED finish */
      
// NOTE: CK_OP CODEREPs are bigger than the others so don't copy an operator
//  node into another type of node.
void
CODEREP::Copy(const CODEREP &cr)
{
  Set_kind(cr.Kind());
  Set_dtyp_strictly(cr.Dtyp());
  Set_dsctyp(cr.Dsctyp());
  Set_usecnt(cr.Usecnt());
  Assign_flags(cr.flags);
  Assign_is_lcse(cr.Is_lcse());
  Assign_is_saved(cr.Is_saved());
  Assign_is_volatile(cr.Is_volatile());
  Assign_sign_extd(cr.Is_sign_extd());
  Set_emit_bb(cr.Emit_bb());
  
  _coderep_id = 0;
  Set_Bitpos(cr.Bitpos());

  if (kind == CK_VAR) {
    Set_aux_id( cr.Aux_id() );
    Set_version( cr.Version() );
    Set_lod_ty( cr.Lod_ty() );
    Set_defstmt( cr.Defstmt() );
    Set_offset( cr.Offset() );
    Set_defchi( cr.Defchi() );
    Set_isvar_flags( cr.Isvar_flags() );
    Set_field_id( cr.Field_id() );
  }
  else if (kind == CK_LDA) {
    Set_lda_aux_id( cr.Lda_aux_id() );
    Set_lda_base_st( cr.Lda_base_st() );
    Set_lda_ty( cr.Lda_ty() );
    Set_offset(cr.Offset());
  }
  else if (kind == CK_OP) {
    Set_opr(cr.Opr());
    Set_kid_count(cr.Kid_count());
    for (INT i = 0; i < Kid_count(); i++) {
      Set_opnd(i, cr.Get_opnd(i));
    }
    Reset_isop_flags();
    Set_temp_id(0);

    switch (Opr()) {
    case OPR_ARRAY:
      Set_elm_siz(cr.Elm_siz());
      break;
    case OPR_CVTL:
      Set_offset(cr.Offset());
      break;
    case OPR_EXTRACT_BITS:
    case OPR_COMPOSE_BITS:
      Set_op_bit_size(cr.Op_bit_size());
      Set_op_bit_offset(cr.Op_bit_offset());
    case OPR_INTRINSIC_CALL:
    case OPR_INTRINSIC_OP:
      Set_intrinsic(cr.Intrinsic());
      break;
    case OPR_TAS:
      Set_ty_index(cr.Ty_index());
      break;
    case OPR_ASM_INPUT:
      Set_asm_constraint(cr.Asm_constraint());
      break;
    }
  }
  else if (kind == CK_IVAR) {
    Set_opr(cr.Opr());
    Set_ivar_occ(cr.Ivar_occ());
    Set_ilod_base(cr.Ilod_base());
    Set_istr_base(cr.Istr_base());
    Set_ilod_ty(cr.Ilod_ty());
    Set_offset(cr.Offset());
    Set_i_field_id(cr.I_field_id());
    Set_ivar_defstmt(cr.Ivar_defstmt());
    Set_ivar_mu_node(cr.Ivar_mu_node());
    Set_mload_size(cr.Mload_size());
  }
  else if (kind == CK_CONST) {
    Set_const_val( cr.Const_val() );
  }
  else if (kind == CK_RCONST) {
    Set_const_id( cr.Const_id() );
  }
  else {
    Is_True( kind == CK_DELETED,
	     ("CODEREP::Copy: Invalid kind: %d", (INT)kind) );
  }
}

// Returns whether cr is among the top nth (starting from 0) elements of the 
// code stack at aux_id aux.
static BOOL Among_top_nth_coderep(CODEREP *cr, AUX_ID aux, INT32 n, 
				  OPT_STAB *sym)
{
  INT32 i;
  for (i = 0; i != n+1; i++) {
    if (cr == sym->Top_nth_coderep(aux, i))
      return TRUE;
  }
  return FALSE;
}

// Returns TRUE if all the opnds of the mu-list are defined by defstmt
//  If mu_vsym_depth is not zero, matching succeeds if defstmt's version is 
//  among the versions in the coderep stack from top up to that depth.
BOOL
CODEREP::Match_mu_and_def(STMTREP *defstmt, INT32 mu_vsym_depth, OPT_STAB *sym)
const
{
#ifdef Is_True_On
  if (! WOPT_Enable_Ivar_Common) return FALSE;
#endif
  MU_NODE *mnode = Ivar_mu_node();
  CODEREP *cr = mnode->OPND();
  if (cr && cr->Is_flag_set(CF_IS_ZERO_VERSION))
    return FALSE;
  if (mu_vsym_depth == 0) {
    if (cr != NULL && cr->Defstmt() != defstmt)
      return FALSE;
  }
  else {
    // find the chi node corresponding to this mu node
    CHI_NODE *cnode;
    CHI_LIST_ITER chi_iter;
    FOR_ALL_NODE(cnode, chi_iter, Init(defstmt->Chi_list()))
      if (cnode->Live() && cnode->Aux_id() == mnode->Aux_id())
	break;
    if (cnode == NULL || cnode->Aux_id() != mnode->Aux_id())
      return FALSE;
    // top of stack must be mnode->OPND()
    if (! Among_top_nth_coderep(cnode->RESULT(), mnode->Aux_id(), 
				mu_vsym_depth, sym))
      return FALSE;
  }
  return TRUE;
}


//  Returns TRUE the MU-lists are identical.
//  If mu_vsym_depth is not zero, matching succeeds if "this" is among the
//  versions in the coderep stack from top up to that depth.
//  **** ASSUMPTION: ****
//   the mu nodes are presented in the same order.
//  The assumption is guaranteed by Compute_FFA() which
//  inserts the MU and CHI lists.
//
inline BOOL
CODEREP::Match_mu_and_mu(MU_NODE *mu_node, INT32 mu_vsym_depth, OPT_STAB *sym)
const
{
#ifdef Is_True_On
  if (! WOPT_Enable_Ivar_Common) return FALSE;
#endif
  MU_NODE *mnode1 = Ivar_mu_node();
  MU_NODE *mnode2 = mu_node;
  if (mnode2 == NULL ||
      mnode1 == NULL ||
      (mnode1 && mnode1->OPND()->Is_flag_set(CF_IS_ZERO_VERSION)) ||
      (mnode2 && mnode2->OPND()->Is_flag_set(CF_IS_ZERO_VERSION)))
    return FALSE;
  if (mu_vsym_depth == 0) {
    if (mnode1->OPND() != mnode2->OPND())
      return FALSE;
  }
  else {
    // top of stack must be mnode2->OPND()
    if (! Among_top_nth_coderep(mnode1->OPND(), mnode2->Aux_id(), 
				mu_vsym_depth, sym))
      return FALSE;
  }
  return TRUE;
}
   
BOOL
CODEREP::Match(CODEREP* cr, INT32 mu_vsym_depth, OPT_STAB *sym)
{
  // compare this with cr, return TRUE if identical
  Is_True(cr->Kind() != CK_VAR, 
	  ("CODEREP::Match: should not need to call with a VAR node"));
  if (Kind() != cr->Kind()) return FALSE;
  if (Dtyp() != cr->Dtyp() && 
      //!inCODEKIND(Kind(),CK_CONST|CK_RCONST) && 
      !(Kind() == CK_IVAR && OPCODE_is_load(Op())))
    return FALSE;

  // in SSAPRE, this allows us to create a new node for the expr whose
  // CF_OWNED_BY_TEMP flag is set
  if (Is_flag_set(CF_OWNED_BY_TEMP))
    return FALSE;

  // UNDID THIS SINCE EPRE CANNOT HANDLE IT CORRECTLY!
  //
  // We turn off any redundancy elimination by means of online
  // value numbering when we test our new off-line value numbering
  // scheme.  However, due to side-effects we cannot do so for rconsts
  // and ivars (rdahl):
  //
  // if ((WOPT_Enable_Value_Numbering == VNFRE_BEFORE_AND_AFTER_EPRE ||
  // 	 WOPT_Enable_Value_Numbering == 
  //                            VNFRE_SINGLE_PASS_BEFORE_AND_AFTER_EPRE) &&
  // 	 !inCODEKIND(Kind(), CK_IVAR|CK_RCONST))
  //    return FALSE;

  switch (Kind()) {

  case CK_LDA:
    if (Lda_base_st() == cr->Lda_base_st() && Offset() == cr->Offset() &&
	Is_flag_set(CF_LDA_LABEL) == cr->Is_flag_set(CF_LDA_LABEL)) 
      return TRUE;
    else
      return FALSE;

#if 0	// CK_VAR nodes never need to be matched
  case CK_VAR:
    if (Aux_id() == cr->Aux_id() &&  Version() == cr->Version() &&
	! Is_var_volatile())
      return TRUE;
    else
      return FALSE;
#endif

  case CK_OP:
#if 0
    if (OPCODE_commutative_op(Op()) == Op()) {
      // commutative ops must have only 2 kids
      // TODO: handle opcodes that are commutative with different opcode
      //       e.g. a <= b becoming b >= a
      return Get_opnd(0) == cr->Get_opnd(0) && Get_opnd(1) == cr->Get_opnd(1)
        || Get_opnd(1) == cr->Get_opnd(0) && Get_opnd(0) == cr->Get_opnd(1);
    }
    else
#endif
    if (Op() == cr->Op() && Kid_count() == cr->Kid_count()) {
      for (INT i = 0; i < Kid_count(); i++)
	if (Get_opnd(i) != cr->Get_opnd(i))
	  return FALSE;
      switch (Opr()) {
      case OPR_CVTL:
	if (Offset() != cr->Offset())
	  return FALSE;
	break;
      case OPR_EXTRACT_BITS:
      case OPR_COMPOSE_BITS:
	if (Op_bit_offset() != cr->Op_bit_offset() ||
	    Op_bit_size() != cr->Op_bit_size())
	  return FALSE;
	break;
      case OPR_ARRAY:
	if (Kid_count() != cr->Kid_count() || Elm_siz() != cr->Elm_siz())
	  return FALSE;
	break;

      case OPR_CALL:
      case OPR_ICALL:
        return FALSE;

      case OPR_INTRINSIC_CALL:
	// never match intrinsic calls (yet)  (some problem with
	// mu lists being null)
	return FALSE;

      case OPR_INTRINSIC_OP:
	if (Intrinsic() != cr->Intrinsic())
	  return FALSE;
	break;
      }
      return TRUE;
    } else
      return FALSE;

  case CK_IVAR:
    {
      const OPCODE ivar_opc = Op();
      const OPERATOR ivar_opr = Opr();
      const OPCODE cr_opc = cr->Op();
      const OPERATOR cr_opr = cr->Opr();

      // e.g. do not match MLOAD and ILOAD.
      if (cr_opr != ivar_opr) return FALSE;

      if ( Get_mtype_class(OPCODE_rtype(ivar_opc)) != 
	   Get_mtype_class(OPCODE_rtype(cr_opc)) )
	return FALSE;
      if (ivar_opr == OPR_PREFETCH)   // no CSE for PREFETCH op
	return FALSE;
      else if ( ivar_opr == OPR_PARM && cr_opr == OPR_PARM)
      {
	// types must match
	if ( ivar_opc != cr_opc )
	  return FALSE;
        // flag must match
        if (Offset() != cr->Offset())
          return FALSE;

	// the actual parameter is stored in the base
	if ( Ilod_base() != cr->Ilod_base() )
	  return FALSE;

        // the TY_IDX has to be different
        if (Ilod_ty() != cr->Ilod_ty())
          return FALSE;

	// must have matching mu lists as well
        if (cr->Ivar_mu_node() && Ivar_mu_node()) 
          return Match_mu_and_mu(cr->Ivar_mu_node(), 0, NULL);
        else if (cr->Ivar_mu_node() || Ivar_mu_node())
          return FALSE;
        else
          return TRUE;
      }

      if (cr->Is_ivar_volatile())
	return FALSE;

      if (ivar_opr == OPR_MLOAD) {
	if (cr->Mload_size() != Mload_size())
	  return FALSE;
      }
      else { // for ILOADs check that they have identical alignments
	TY_IDX ivar_addr_ty = Ilod_base_ty();
	TY_IDX cr_addr_ty = cr->Ilod_base_ty();
	if (ivar_addr_ty != cr_addr_ty) {
	  if (! ivar_addr_ty || ! cr_addr_ty)
	    return FALSE;
	  if (TY_kind(ivar_addr_ty) != KIND_POINTER)
	    return FALSE;
	  if (TY_kind(cr_addr_ty) != KIND_POINTER)
	    return FALSE;
	  if (TY_align_exp(TY_pointed(ivar_addr_ty)) !=
	      TY_align_exp(TY_pointed(cr_addr_ty)))
	    return FALSE;
	}
        Is_True(ivar_opr == cr_opr, 
		("CODEREP::Match: ILOAD operator is inconsistent with bitfield"));
	if (ivar_opr == OPR_ILDBITS) {
	  if (I_bit_offset() != cr->I_bit_offset() ||
	      I_bit_size() != cr->I_bit_size())
	    return FALSE;
	}
      }

      if (Ilod_base() == NULL) {  // *this is a definition
	if (cr->Ilod_base() == Istr_base() &&
	  cr->Match_mu_and_def( Ivar_defstmt(), mu_vsym_depth, sym)) {
	  if (Offset() != cr->Offset())
	    return FALSE;
	  if (Dsctyp() == MTYPE_BS)
	    if (I_field_id() != cr->I_field_id())
	      return FALSE;
	  Set_ilod_base(Istr_base());
	  // SIDE EFFECT: Set the mu of the existing coderep to match
	  // the LHS of the chi node.
	  if (Ivar_mu_node() == NULL) {
	    CODEREP *vsym = Get_ivar_vsym();
	    if (vsym == NULL) {
	      MU_NODE *mnode = CXX_NEW(MU_NODE, sym->Occ_pool());
	      mnode->Set_aux_id( cr->Ivar_mu_node()->Aux_id());
	      mnode->Set_OPND( cr->Ivar_mu_node()->OPND());
	      Set_ivar_mu_node(mnode);
	      Ivar_occ()->Set_aux_id( mnode->Aux_id());
	    } else {
	      // Is this the right MEM_POOL to use?
	      MU_NODE *mnode = CXX_NEW(MU_NODE, sym->Occ_pool());
	      mnode->Set_aux_id(vsym->Aux_id());
	      mnode->Set_OPND(vsym);
	      Set_ivar_mu_node(mnode);
	    } 
	  }
	  if (Ivar_occ()->Lno_dep_vertex_load() == 0)
	    Ivar_occ()->Set_lno_dep_vertex_load(cr->Ivar_occ()->Lno_dep_vertex_load());  // preserve LNO info
	  return TRUE; 
	} 
	else return FALSE;
      } 

      if (cr->Ilod_base() == NULL) {   // *cr is a definition
	if (Ilod_base() == cr->Istr_base() &&
	    Match_mu_and_def( cr->Ivar_defstmt(), 0, NULL)) {
	  if (Offset() != cr->Offset())
	     return FALSE;
	  if (Dsctyp() == MTYPE_BS)
	    if (I_field_id() != cr->I_field_id())
	      return FALSE;
	  Set_istr_base (Ilod_base());
	  Ivar_occ()->Set_lno_dep_vertex_store(cr->Ivar_occ()->Lno_dep_vertex_store()); // preserve LNO info
	  return TRUE;
	} else
	  return FALSE;
      }

      // both *this and *cr are uses.
      if (Ilod_base() == cr->Ilod_base() && 
	  Match_mu_and_mu(cr->Ivar_mu_node(), mu_vsym_depth, sym)){
	if (Offset() != cr->Offset()) 
	  return FALSE;
        if (Dsctyp() == MTYPE_BS)
	  if (I_field_id() != cr->I_field_id())
	    return FALSE;
	if (Ivar_occ()->Lno_dep_vertex_load() == 0)
	  Ivar_occ()->Set_lno_dep_vertex_load(cr->Ivar_occ()->Lno_dep_vertex_load());  // preserve LNO info
	return TRUE;
      }
      else
	return FALSE;
    }

  case CK_CONST:
    if (Const_val() == cr->Const_val()) 
      return TRUE;
    else return FALSE;

  case CK_RCONST:
    if (Const_id() == cr->Const_id()) {
      Set_dtyp(cr->Dtyp());
      Set_dsctyp(cr->Dsctyp());
      return TRUE;
    }
    else return FALSE;
  }
  Is_True(FALSE,("CODEREP::Match, should never reach this point"));
  return FALSE;
}


BOOL
CODEREP::Match_constval(const CODEREP *cr) 
{
  Is_True(Kind() == CK_CONST && cr->Kind() == CK_CONST,
	  ("CR not CONST kind."));
  // ignore the Dtyp()
  return Const_val() == cr->Const_val();
}

BOOL
CODEREP::Contains( const CODEREP *cr ) const
{
  // returns TRUE if this tree contains the node 'cr'
  if (this == cr) return TRUE;
  INT i;
  switch (kind) {
  case CK_OP:
    for (i = 0; i < Kid_count(); i++)
      if (Get_opnd(i)->Contains(cr)) return TRUE;
    return FALSE;
  case CK_IVAR:
    Is_True(Ilod_base() != NULL,
	    ("CODEREP::Contains: CK_IVAR must be rvalue"));
    if (Ilod_base()->Contains(cr)) return TRUE;
    if (Opr() == OPR_MLOAD) {
      CODEREP *num_byte = Mload_size();
      if (num_byte->Contains(cr)) return TRUE;
    }
    return FALSE;
  case CK_LDA:
  case CK_VAR:
  case CK_CONST:
  case CK_RCONST:
    return FALSE;
  }
  return FALSE;
}

BOOL
CODEREP::Contains_image( const CODEREP *cr ) const
{
  // returns TRUE if this tree contains the same image(bitpos) as node 'cr'
  if (Bitpos() == cr->Bitpos()) return TRUE;
  INT i;
  switch (kind) {
  case CK_OP:
    for (i = 0; i < Kid_count(); i++)
      if (Get_opnd(i)->Contains_image(cr)) return TRUE;
    return FALSE;
  case CK_IVAR:
    if (Ilod_base()->Contains_image(cr)) return TRUE;
    if (Opr() == OPR_MLOAD) {
      CODEREP *num_byte = Mload_size();
      if (num_byte->Contains_image(cr)) return TRUE;
    }
    return FALSE;
  case CK_LDA:
  case CK_VAR:
  case CK_CONST:
  case CK_RCONST:
    return FALSE;
  }
  return FALSE;
}

void
CODEREP::Print(INT indent, FILE *fp) const
{
  INT i;
  if (Kind() == CK_OP) {
    for (i = 0; i < Kid_count(); i++)
      Get_opnd(i)->Print(indent+1, fp);
  } else if (Kind() == CK_IVAR) {
    if (Ilod_base() != NULL)
      Ilod_base()->Print(indent+1, fp);
    else
      Istr_base()->Print(indent+1, fp);
  }
  Print_node(indent, fp);
  fprintf(fp, " b=%s",Print_bit());
  if (Kind() == CK_IVAR) {
    fprintf(fp, " mu<");
    if (Ivar_mu_node() && Ivar_mu_node()->OPND())  // Check if the MU has been deleted.
      fprintf(fp, "%d/cr%d", Ivar_mu_node()->OPND()->Aux_id(), 
	      Ivar_mu_node()->OPND()->Coderep_id());
    fprintf(fp, ">\n"); 
  } else
    fprintf(fp, "\n");
}

void
CODEREP::Print_node(INT32 indent, FILE *fp) const
{
  INT i;
  switch (Kind()) {
  case CK_OP:
    fprintf(fp, ">");	// mark line visually as htable dump
    for (i = 0; i < indent; i++) fprintf(fp, " ");
    char buf[32];
    sprintf(buf, "%s", OPCODE_name(Op()));
    fprintf(fp, "%s", buf+4);
    switch (Opr()) {
    case OPR_CVTL:
      fprintf(fp, " %d", Offset());
      break;
    case OPR_EXTRACT_BITS:
    case OPR_COMPOSE_BITS:
      fprintf(fp," o:%d s:%d",Op_bit_offset(),Op_bit_size());
      break;
    case OPR_ARRAY:
      fprintf(fp, " %d %lld", Num_dim(), Elm_siz());
      break;
    case OPR_INTRINSIC_CALL:
    case OPR_INTRINSIC_OP:
      fprintf(fp, " %d", Intrinsic());
      break;
    case OPR_ASM_INPUT:
      fprintf(fp, " opnd:%d", Asm_opnd_num());
      break;
    }
    break;
  case CK_IVAR:
    if (Opr() == OPR_MLOAD) {
      CODEREP *num_byte = Mload_size();
      num_byte->Print(indent+1, fp);
    }
    if (Opr() == OPR_ILOADX)
      Index()->Print(indent+1, fp);
    fprintf(fp, ">");	// mark line visually as htable dump
    for (i = 0; i < indent; i++) fprintf(fp, " ");
    if (Opr() == OPR_ILOADX ||
        Opr() == OPR_MLOAD ||
        Opr() == OPR_PARM)
      sprintf(buf, "%s ty=%x ", OPCODE_name(Op()), Ilod_ty());
    else
      sprintf(buf,"    %s%s%s %d ty=%x ",
              MTYPE_name(Dtyp()),MTYPE_name(Dsctyp()),
	      Opr() == OPR_ILOAD ? "ILOAD" : "ILDBITS",
              Offset(), Ilod_ty());
    fprintf(fp, "%s", buf+4);
    break;
  case CK_LDA:
    fprintf(fp, ">");	// mark line visually as htable dump
    for (i = 0; i < indent; i++) fprintf(fp, " ");
    fprintf(fp, "LDA %s sym%d %d", MTYPE_name(Dtyp()),Lda_aux_id(),Offset());
    break;
  case CK_VAR:
    fprintf(fp, ">");	// mark line visually as htable dump
    for (i = 0; i < indent; i++) fprintf(fp, " ");
    if (! Bit_field_valid())
      fprintf(fp, "LDID");
    else fprintf(fp, "LDBITS");
    fprintf(fp, " %s %s sym%dv%d %d ty=%x ", MTYPE_name(Dsctyp()),
	    MTYPE_name(Dtyp()), Aux_id(), Version(), Offset(), Lod_ty());
    break;
  case CK_CONST:
    fprintf(fp, ">");	// mark line visually as htable dump
    for (i = 0; i < indent; i++) fprintf(fp, " ");
    fprintf(fp, "LDC %s %lld", MTYPE_name(Dtyp()), Const_val());
    break;
  case CK_RCONST:
    fprintf(fp, ">");	// mark line visually as htable dump
    for (i = 0; i < indent; i++) fprintf(fp, " ");
    fprintf(fp, "LDRC %s 0x%p", MTYPE_name(Dtyp()), Const_id());
    break;
  case CK_DELETED:
  default:
    Is_True(FALSE,("CODEREP::Print_node: illegal kind: %s",Print_kind()));
    break;
  }
  fprintf(fp, " <u=%d cr%d>", Usecnt(), Coderep_id());
  switch (kind) {
  case CK_VAR:
    if (Is_var_nodef()) 
      fprintf(fp, " (no-def)");
    if (Is_var_volatile()) 
      fprintf(fp, " (vol)");
    break;
  case CK_IVAR:
#if 0 //TODO: put in after ilod is recognized CSE
    if (Op_defstmt() == NULL) fprintf(fp, " (no-def)");
#endif
    if (Is_ivar_volatile()) 
      fprintf(fp, " (vol)");
    break;
  case CK_OP:
    fprintf(fp, " isop_flags:0x%x", u2.isop.isop_flags);
    break;
  }

  fprintf( fp, " flags:0x%x", flags&0x1ff );
}

// Print a CODEREP to a string
// the string is padded with spaces to be N chars long
// if name_format is TRUE, print the opcode name
// if name_format is FALSE, print the bit positions of the children
#define Nchars 20
char *
CODEREP::Print_str(BOOL name_format) const
{
  static char buf[100];

  buf[0] = '\0';

  switch (Kind()) {
  case CK_OP:
    if (name_format)
      sprintf(buf,"%s",OPCODE_name(Op())+4);
    else {
      sprintf(buf,"<");
      for (INT i=0; i<Kid_count(); i++) {
	strcat(buf,Opnd(i)->Print_bit());
	if (i+1 < Kid_count())
	  strcat(buf,", ");
      }
      strcat(buf,">");
    }
    break;
  case CK_IVAR:
    if (name_format) {
      if (Opr() == OPR_ILOADX ||
	  Opr() == OPR_MLOAD ||
	  Opr() == OPR_PARM) {
	sprintf(buf,"%s %d ty=%x ", OPCODE_name(Op()),
                Offset(), Ilod_ty());
      }
      else
	sprintf(buf,"%s%s%s %d ty=%x ",
		MTYPE_name(Dtyp()),MTYPE_name(Dsctyp()),
		Opr() == OPR_ILOAD ? "ILOAD" : "ILDBITS",
		Offset(), Ilod_ty());
    }
    else {
      if (Opr() == OPR_MLOAD)
	sprintf(buf,"<%s>",Mload_size()->Print_bit());
    }
    break;
  case CK_LDA:
    if (name_format)
      sprintf(buf,"LDA sym%d %d",Lda_aux_id(),Offset());
    break;
  case CK_VAR:
    if (name_format) {
      if (! Bit_field_valid())
        sprintf(buf,"LDID ");
      else sprintf(buf,"LDBITS ");
      sprintf(buf,"sym%dv%d %d ty=%x ",Aux_id(),Version(),Offset(), Lod_ty());
    }
    break;
  case CK_CONST:
    if (name_format)
      sprintf(buf,"LDC %lld",Const_val());
    break;
  case CK_RCONST:
    if (name_format)
      sprintf(buf,"LDRC 0x%p",Const_id());
    break;
  default:
    Warn_todo("CODEREP::Print: CODEKIND is not implemented yet");
    break;
  }
  if (name_format && strlen(buf) < Nchars) {  	// pad with spaces to Nchars
    INT i;
    for (i=strlen(buf); i<Nchars; i++)
      buf[i] = ' ';
    buf[i] = '\0';
  }
  return buf;
}

// print the kind field to a string
char *
CODEREP::Print_kind(void) const
{
  static char str[20];
  switch (Kind()) {
  case CK_LDA:	        strcpy(str,"CK_LDA");		break;
  case CK_CONST:	strcpy(str,"CK_CONST");		break;
  case CK_RCONST:	strcpy(str,"CK_RCONST"); 	break;
  case CK_VAR:	        strcpy(str,"CK_VAR");		break;
  case CK_IVAR:  	strcpy(str,"CK_IVAR");		break;
  case CK_OP:		strcpy(str,"CK_OP");		break;
  case CK_DELETED:	strcpy(str,"CK_DELETED");	break;
  default:		strcpy(str,"UNKNOWN");		break;
  }
  return str;
}

// given the opcode of the operator and some info about which kid we
// are dealing with, come up with the type for the kid 
MTYPE
Operand_type( OPCODE op, INT which_kid, INT num_kids )
{
  switch (OPCODE_operator(op)) {
  case OPR_CALL:
  case OPR_INTRINSIC_CALL:
  case OPR_INTRINSIC_OP:
  case OPR_TAS:
    return MTYPE_UNKNOWN;

  case OPR_CEIL:
  case OPR_CVT:
  case OPR_FLOOR:
  case OPR_RND:
  case OPR_TRUNC:
  case OPR_LT:
  case OPR_LE:
  case OPR_EQ:
  case OPR_NE:
  case OPR_GE:
  case OPR_GT:
    return OPCODE_desc(op);

  case OPR_ICALL:
    if ( which_kid == num_kids-1 )
      // last kid is the address of function being called
      return Pointer_type;
    else
      return MTYPE_UNKNOWN;

  case OPR_ILDBITS:
  case OPR_ILOAD:		// only 1 kid
  case OPR_ILOADX:		// 2 kids, but both pointer types
  case OPR_MLOAD:		// 2 kids, but both pointer types
    return Pointer_type;

  case OPR_ISTORE:
  case OPR_ISTBITS:
  case OPR_ISTOREX:
    if ( which_kid == 0 )
      return OPCODE_desc(op);	// value being stored
    else
      return Pointer_type;	// address(es) being stored to

  case OPR_ARRAY:
    if ( which_kid == 0 )
      return Pointer_type;	// base of the array
    else
      return MTYPE_UNKNOWN;

  case OPR_MSTORE:
    if ( which_kid == 0 )
      return MTYPE_UNKNOWN;	// chunk being stored
    else
      return Pointer_type;	// address being stored to

  case OPR_COMPLEX:
    {
      MTYPE rtype = OPCODE_rtype(op);
      if ( rtype == MTYPE_C4 )
	return MTYPE_F4;
      else if ( rtype == MTYPE_C8 )
	return MTYPE_F8;
      else if ( rtype == MTYPE_CQ )
	return MTYPE_FQ;
      else {
	FmtAssert( FALSE, 
		   ("CODEREP::Operand_type: unknown type %d", rtype) );
	return MTYPE_UNKNOWN;
      }
    }
      
  case OPR_REALPART:
  case OPR_IMAGPART:
    {
      MTYPE rtype = OPCODE_rtype(op);
      if ( rtype == MTYPE_F4 )
	return MTYPE_C4;
      else if ( rtype == MTYPE_F8 )
	return MTYPE_C8;
      else if ( rtype == MTYPE_FQ )
	return MTYPE_CQ;
      else {
	FmtAssert( FALSE, 
		   ("CODEREP::Operand_type: unknown type %d", rtype) );
	return MTYPE_UNKNOWN;
      }
    }

  case OPR_SELECT:
    if ( which_kid == 0 )
      return MTYPE_UNKNOWN;
    else
      return OPCODE_rtype(op);

  case OPR_SHL:
  case OPR_ASHR:
  case OPR_LSHR:
    if ( which_kid == 1 )
      return MTYPE_UNKNOWN;
    else
      return OPCODE_rtype(op);

  case OPR_CVTL:
  default:
    return OPCODE_rtype(op);
  }
}

/* CVTL-RELATED start (correctness) */
// Insert correct type conversion
CODEREP*
CODEREP::Fixup_type(MTYPE opr_type, CODEMAP *htable)
{
  INT           cvt_kind;
  OPCODE        opc;
  CODEREP      *cr = Alloc_stack_cr(0);

  FmtAssert(this != NULL,
	    ("CODEREP::Fixup_type: called with null CR "));

  // check if there is missing convert from this node to its parent
  cvt_kind = Need_type_conversion(Dtyp(), opr_type, &opc);

  switch (cvt_kind) {
  case NOT_AT_ALL:
    return this;
  case NEED_CVT:
    IncUsecnt();
    cr->Init_expr(opc, this);
    return htable->Rehash(cr);
  case NEED_CVTL:
    IncUsecnt();
    cr->Init_expr(opc, this);
    cr->Set_offset(opr_type);
    return htable->Rehash(cr);
  }

  return this;
}
/* CVTL-RELATED finish */

void
CODEREP::DecUsecnt_rec(void)
{
  //FmtAssert(usecnt != 0, 
  //   ("CODEREP::DecUsecnt_rec: trying to decrement usecnt when value is 0"));
  // convert to Warn_todo tempoary ...
  if (this == NULL) {
    Warn_todo(
      "CODEREP::DecUsecnt_rec: trying to decrement usecnt when this is null.");
    return;
  }

  if (usecnt == 0) 
    Warn_todo(
      "CODEREP::DecUsecnt_rec: trying to decrement usecnt when value is 0.");
  DecUsecnt();
  switch (kind) {
  case CK_LDA: 
  case CK_CONST: 
  case CK_RCONST:
  case CK_VAR:
    return;
  case CK_IVAR:
    if (Usecnt()) return;
    // usecnt becomes 0, so need just do the same for all kids
    Ilod_base()->DecUsecnt_rec();
    if (Opr() == OPR_MLOAD) {
      CODEREP *num_byte = Mload_size();
      num_byte->DecUsecnt_rec();
    }
    return;
  case CK_OP:
    if (Usecnt()) return;
    // usecnt becomes 0, so need just do the same for all kids
    for (INT32 i = 0; i < Kid_count(); i++) 
      Opnd(i)->DecUsecnt_rec();
    return;
  }
}

// Same as DecUsecnt_rec, except that the given node's usecnt is not
// decremented.  Only its kids' usecnt are decremented.
// NOTE:  This function may only be useful when eliminating some
// statement and we need to deal with its lvalue (Lhs).
void
CODEREP::DecKidsUsecnt_rec( void )
{
  // increment just this node's usecnt
  IncUsecnt();
  // then decrement it and all of its kids' usecnt
  DecUsecnt_rec();
}

void
CODEREP::IncUsecnt_rec(void)
{
  IncUsecnt();
  switch (kind) {
  case CK_LDA: 
  case CK_CONST: 
  case CK_RCONST:
  case CK_VAR:
    return;
  case CK_IVAR:
    if (Usecnt() <= 1) return;
    // it is a CSE, so need to decrease usecnt of all kids
    Ilod_base()->DecUsecnt_rec();
    if (Opr() == OPR_MLOAD)
      Mload_size()->DecUsecnt_rec(); // the kid for the size
    return;
  case CK_OP:
    if (Usecnt() <= 1) return;
    // it is a CSE, so need to decrease usecnt of all kids
    for (INT32 i = 0; i < Kid_count(); i++) 
      Opnd(i)->DecUsecnt_rec();
    return;
  }
}


STMTREP*
CODEREP::Create_cpstmt(CODEREP *a, MEM_POOL*p)
{
  // create copy from 'this' to 'a'. Return the created stmtrep.
  STMTREP *cpstmt = CXX_NEW(STMTREP,p);
  IncUsecnt();

#if 1
  // The dsctyp is wrong.
  Is_True(a->Dsctyp() != MTYPE_UNKNOWN || Dsctyp() != MTYPE_UNKNOWN
	  || Kind() == CK_CONST || Kind() == CK_RCONST || Kind() == CK_LDA,
          ("CODEREP::Create_cpstmt: both assigner and assignee has no type.  This problem is likely due to the SSA form is incorrectly setup in the earlier phase."));
  if (a->Dsctyp() == MTYPE_UNKNOWN) {
    if (Kind() == CK_CONST) {
      a->Set_dtyp_strictly(Dtyp());
      a->Set_dsctyp(Dtyp());
      a->Set_lod_ty(MTYPE_To_TY(Dtyp()));
    }
    else {
      a->Set_dtyp(Dtyp());
      a->Set_dsctyp(Dtyp()); // in case 'this' is an expression
      a->Set_lod_ty(MTYPE_To_TY(Dtyp()));
    }
    a->Set_sign_extension_flag();
  }
  else if (Kind() == CK_VAR && Dsctyp() == MTYPE_UNKNOWN) {
    Copy_type(a);
    Set_sign_extension_flag();
  }

  cpstmt->Init(a, this, OPCODE_make_op(a->Bit_field_valid() ? 
				OPR_STBITS : OPR_STID, MTYPE_V, a->Dsctyp()));

#else

  // for testing
  Warn_todo("Create_cpstmt has the wrong dsctype.");
  cpstmt->Init(a, this, OPCODE_make_op(a->Bit_field_valid() ?
				OPR_STBITS : OPR_STID, MTYPE_V, MTYPE_U8));

#endif

  // NOTE:  we can set the mu/chi lists to null here because this store
  // is guaranteed not to have side-effects.  If there other stores
  // that could side-effect the location we're storing to, we would
  // not have gotten into the situation where we can be inserting this
  // store.
  cpstmt->Set_chi_list(NULL);
//  cpstmt->Set_rhs_type(Dtyp());
  if (a->Kind() == CK_VAR) 
    a->Set_defstmt(cpstmt);
  else if (a->Kind() == CK_IVAR) 
    a->Set_ivar_defstmt(cpstmt);
  return cpstmt;
}

STMTREP*
CODEREP::Create_istr_stmt(CODEREP *a, MEM_POOL*p)
{
  // create copy from 'this' to 'a'. Return the created stmtrep.
  STMTREP *cpstmt = CXX_NEW(STMTREP,p);
  IncUsecnt();

  // The dsctyp is wrong.
  Is_True(a->Dsctyp() != MTYPE_UNKNOWN || Dsctyp() != MTYPE_UNKNOWN,
          ("CODEREP::Create_istr_stmt: both assigner and assignee has no type"));
  if (a->Dsctyp() == MTYPE_UNKNOWN) {
    a->Set_dtyp(Dtyp());
    a->Set_dsctyp(Dsctyp());
    a->Set_lod_ty(Lod_ty());
    a->Set_sign_extension_flag();
  }
  else if (Dsctyp() == MTYPE_UNKNOWN) {
    Set_dtyp(a->Dtyp());
    Set_dsctyp(a->Dsctyp());
    Set_lod_ty(a->Ilod_ty());
    Set_sign_extension_flag();
  }

  if (a->Istr_base() == NULL)
    a->Set_istr_base(a->Ilod_base());

  cpstmt->Init(a, this, OPCODE_make_op(a->Opr() == OPR_ILOAD ? OPR_ISTORE : OPR_ISTBITS, MTYPE_V, a->Dsctyp()));

//  cpstmt->Set_rhs_type(Dtyp());
  a->Set_ivar_defstmt(cpstmt);
  return cpstmt;
}

// optimization has invalidate the has_zver flag; recompute this flag from
// scratch
void
STMTREP::Recompute_has_zver(void)
{
  CHI_LIST_ITER chi_iter;
  CHI_NODE *chi;
  Reset_has_zver();
  FOR_ALL_NODE(chi, chi_iter, Init(Chi_list())) {
    if (! chi->Live())
      continue;
    CODEREP *res = chi->RESULT();
    if (res->Kind() == CK_VAR && res->Is_flag_set(CF_IS_ZERO_VERSION)) {
      Set_has_zver();
      return;
    }
  }
}

BB_NODE*
CODEREP::Defbb(void) const
{
  // the BB that defines this coderep
  Is_True(Kind() == CK_VAR || Kind() == CK_IVAR,
	  ("CODEREP::Defbb: handles CK_VAR and CK_IVAR only"));
  if (Is_flag_set(CF_DEF_BY_PHI))
    return Defphi()->Bb();
  if (Get_defstmt() == NULL) return NULL;
  return Get_defstmt()->Bb();
}


// Returns TRUE if the statment is defined at by the entry chi list.
//
BOOL
CODEREP::Def_at_entry(void) const
{
  STMTREP *defstmt = Get_defstmt();
  if (defstmt != NULL && defstmt->Opr() == OPR_OPT_CHI)
    return TRUE;
  return FALSE;
}


IDX_32 
CODEMAP::Hash_op_and_canon(CODEREP *cr, BOOL canonicalize)
{

  if (canonicalize &&
      (cr->Kid_count() == 2 && OPCODE_commutative_op(cr->Op()) == cr->Op() &&
       !OPCODE_is_compare(cr->Op()) &&
       inCODEKIND(cr->Get_opnd(0)->Kind(), CK_VAR | CK_LDA | CK_RCONST) &&
       inCODEKIND(cr->Get_opnd(1)->Kind(), CK_VAR | CK_LDA | CK_RCONST))) {
    if (CR_Compare_Trees(cr->Get_opnd(0), cr->Get_opnd(1)) == 1) {
      if (cr->Coderep_id() != 0)
        Warn_todo("CODEMAP::Hash_op_and_canon, hashed cr not in canonical form");
      CODEREP *tmp = cr->Get_opnd(0);
      cr->Set_opnd(0, cr->Get_opnd(1));
      cr->Set_opnd(1, tmp);
    }
  }

  INT val = cr->Op();

  for (INT i = 0; i < cr->Kid_count(); i++) {
    CODEREP *opnd = cr->Get_opnd(i);
    val += (opnd->Coderep_id() << 4);
  }
  return val % size;
}

IDX_32 
CODEMAP::Hash_ivar(CODEREP *cr, CODEREP *base)
{
  INT val;
  OPERATOR opr = cr->Opr();
  if (OPERATOR_is_load(opr))		// only use size for memory ops
    val = opr + MTYPE_size_min(cr->Dsctyp());
  else
    val = cr->Op();
  val += (base->Coderep_id() << 4);
  return val % size;
}

IDX_32
CODEMAP::Hash(CODEREP *cr)
{
  switch (cr->Kind()) {
  case CK_IVAR: 
    return Hash_ivar(cr, cr->Ilod_base() ? cr->Ilod_base() : cr->Istr_base());
  case CK_OP: 
    return Hash_op_and_canon(cr); 
  case CK_LDA: 
    return Hash_lda(cr->Lda_base_st(), (IDTYPE)cr->Offset());
  case CK_CONST: 
    return Hash_const(cr->Const_val());
  case CK_RCONST:
    return Hash_rconst(cr->Const_id(), (IDTYPE)0); 
  default:
    FmtAssert(FALSE, ("CODEMAP::Hash: called with bad node"));
  }
  return 0;	// to satisfy compiler
}

void
CODEREP_CONTAINER::Add_it(CODEREP *cr, const mUINT16 idx, CODEMAP *htable)
{
  // Replicate cr and append it to the tail
  SLIST::Prepend(cr);
  cr->Set_coderep_id(htable->Next_coderep_id());
  htable->Set_hash_bucket(idx, Head());
}

void
CODEREP_CONTAINER::Delete_it(CODEREP *cr, const mUINT16 idx, CODEMAP *htable)
{
  // remove cr from the bucket; must succeed (cr must be in list)
  if (Head() == cr) {
    SLIST::Remove_Headnode();
    htable->Set_hash_bucket(idx, Head());
  }
  else {
    CODEREP *prev;
    CODEREP_ITER cr_iter;
    FOR_ALL_NODE(prev, cr_iter, Init(this->Head())) {
      if (prev->Next() == cr) {
        cr->Remove(prev);
      }
    }
  }
}

BOOL
CODEREP_CONTAINER::Contains(CODEREP *cr)
{
  // test coderep_container contains cr
  CODEREP *tmp;
  CODEREP_ITER cr_iter;
  FOR_ALL_NODE(tmp, cr_iter, Init(this->Head())) {
    if (tmp->Match(cr))
      return TRUE;
  }
  return FALSE;
}

CODEREP*
CODEREP_CONTAINER::Find_cr(CODEREP *cr, INT32 mu_vsym_depth, OPT_STAB *sym)
{
  // return the coderep it exists in MAP, return NULL otherwise
  // it also establish the len field and the tail field.
  CODEREP *tmp;
  CODEREP_ITER cr_iter;
  INT32 counter = 0;
  FOR_ALL_NODE(tmp, cr_iter, Init(this)) {
    Set_Tail(tmp);
    if (tmp->Match(cr, mu_vsym_depth, sym)) 
      return tmp;
    counter++;
  }

  // note: this is just a warning to us for now, and should be removed
  // and we can then remove the counter variable all together
  if ( counter >= 1000 ) {
    Warn_todo( "CODEREP_CONTAINER::Find_cr has 1000 hash entries" );
  }

  len = cr_iter.Len();
  return NULL;
}

CODEREP_LIST*
CODEREP_LIST::Prepend(CODEREP *cr, MEM_POOL *pool)
{
  CODEREP_LIST_CONTAINER coderep_list_container(this);
  CODEREP_LIST *new_crlst = (CODEREP_LIST*)CXX_NEW( CODEREP_LIST(cr), pool );
  if ( new_crlst == NULL ) ErrMsg ( EC_No_Mem, "CODEREP_LIST::Prepend" );
  coderep_list_container.Prepend(new_crlst);
  return (CODEREP_LIST*)coderep_list_container.Head();
}

BOOL 
CODEREP_LIST::Contains(CODEREP *cr) 
{
  // check if cr is already in the list
  CODEREP *tmp;
  CODEREP_LIST_ITER cr_iter;
  FOR_ALL_ELEM(tmp, cr_iter, Init(this)) 
    if (tmp == cr)
      return TRUE;
  return FALSE;
}

BOOL 
CODEREP_LIST_CONTAINER::Contains(CODEREP *cr) 
{
  // check if cr is already in the container
  CODEREP *tmp;
  CODEREP_LIST_ITER cr_iter;
  FOR_ALL_ELEM(tmp, cr_iter, Init(this)) 
    if (tmp == cr)
      return TRUE;
  return FALSE;
}

void
CODEREP_LIST_CONTAINER::Prepend(CODEREP *cr, MEM_POOL *pool)
{
  CODEREP_LIST *new_crlst = (CODEREP_LIST*)CXX_NEW( CODEREP_LIST(cr), pool );
  if ( new_crlst == NULL )
    ErrMsg ( EC_No_Mem, "CODEREP_LIST_CONTAINER::Prepend" );
  Prepend(new_crlst);
}

CR_PAIR_LIST*
CR_PAIR_LIST::Prepend(CODEREP *nd1,  // insert in front, return head
		      CODEREP *nd2,
		      MEM_POOL *pool)
{
  CR_PAIR_LIST_CONTAINER cr_pair_list_container(this);
  CR_PAIR_LIST *new_crlst = (CR_PAIR_LIST*)CXX_NEW( CR_PAIR_LIST(nd1,nd2), pool );
  if ( new_crlst == NULL ) ErrMsg ( EC_No_Mem, "CR_PAIR_LIST::Prepend" );
  cr_pair_list_container.Prepend(new_crlst);
  return (CR_PAIR_LIST*)cr_pair_list_container.Head();
}

BOOL 
CR_PAIR_LIST::Contains(CODEREP *nd1,
		       CODEREP *nd2)
{
  if (nd2 > nd1) { // sort to make nd1 > nd2
    CODEREP *tmp = nd1; nd1 = nd2; nd2 = tmp;
  }
  // check if cr is already in the list
  CR_PAIR_LIST_ITER cr_iter;
  CR_PAIR_LIST     *tmp;
  FOR_ALL_NODE(tmp, cr_iter, Init(this))
    if (tmp->Nd1() == nd1 && tmp->Nd2())
      return TRUE;
  return FALSE;
}

BOOL 
CR_PAIR_LIST_CONTAINER::Contains(CODEREP *nd1,
				 CODEREP *nd2) 
{
  // check if cr is already in the container
  if (nd2 > nd1) { // sort to make nd1 > nd2
    CODEREP *tmp = nd1; nd1 = nd2; nd2 = tmp;
  }
  CR_PAIR_LIST_ITER cr_iter;
  CR_PAIR_LIST     *tmp;
  FOR_ALL_NODE(tmp, cr_iter, Init(this))
    if (tmp->Nd1() == nd1 && tmp->Nd2())
      return TRUE;
  return FALSE;
}

void
CR_PAIR_LIST_CONTAINER::Prepend(CODEREP *nd1,
				CODEREP *nd2,
				MEM_POOL *pool)
{
  CR_PAIR_LIST *new_crlst = (CR_PAIR_LIST*)CXX_NEW( CR_PAIR_LIST(nd1,nd2), pool );
  if ( new_crlst == NULL )
    ErrMsg ( EC_No_Mem, "CR_PAIR_LIST_CONTAINER::Prepend" );
  Prepend(new_crlst);
}

void
CODEMAP::Alloc_hash_vec(void)
{
  // called by the constructor
  hash_vec = CXX_NEW_ARRAY(CODEREP*, size+1, mem_pool);
  if (hash_vec == NULL) ErrMsg ( EC_No_Mem, "CODEREP::Alloc_hash_vec" );
  BZERO(hash_vec, sizeof(CODEREP*) * (size+1));
}

void
CODEMAP::Free_hash_vec(void)
{
  // called by the destructor
  // do nothing since we want to pop the Opt_default_pool at the end
  // of optimization phase.
}

CODEMAP::CODEMAP(mUINT32   hash_size,
                 CFG      *cfg,
                 OPT_STAB *asym,
                 SSA      *ssa, 
                 IDX_32    phi_hash_size,
		 OPT_PHASE phase,
                 MEM_POOL *pool):
  _phi_id_map(phi_hash_size, NULL, pool, 0)
{
  mem_pool = pool;
  sym = asym;
  _cfg = cfg;
  _ssa = ssa;
  size = hash_size;
  _phase = phase;
  Alloc_hash_vec();
  _prefetch_map  = WN_MAP_Create(pool);
  _coderep_id_cnt = 1;				// ID of 0 is unused
  _num_iloadfolds = _num_istorefolds = _num_inputprops = _num_mainprops = _num_shrinks = 0;
  _tracing = Get_Trace( TP_GLOBOPT, CR_DUMP_FLAG );
  _phi_hash_valid = FALSE;
  _pre_temp_id = 0;
}

CODEMAP::~CODEMAP(void)
{
  size = 0;
  Free_hash_vec();
  WN_MAP_Delete(_prefetch_map);
}

// search bucket, create CR if not found; increment usecnt
CODEREP *
CODEMAP::Find_or_append_CR(IDX_32 hash_idx, CODEREP *cr,
			   INT32 mu_vsym_depth)
{ CODEREP_CONTAINER cr_list;
  cr_list.Init_Head(Hash_bucket(hash_idx));
  CODEREP *retv = cr_list.Find_cr(cr, mu_vsym_depth, Sym());
  if (retv) {
    retv->IncUsecnt_rec();
    return retv;
  } else {
    retv = CXX_NEW_VARIANT(CODEREP(*cr),
			   cr->Extra_space_used(),
			   mem_pool);
    cr_list.Add_it(retv, hash_idx, this);
    retv->Set_usecnt(1);
    return retv;
  }
}

// Create CR and append to bucket; does not increment usecnt
CODEREP *
CODEMAP::Append_CR(IDX_32 hash_idx, CODEREP *cr)
{ CODEREP_CONTAINER cr_list;
  CODEREP *retv = CXX_NEW_VARIANT(CODEREP(*cr),
				  cr->Extra_space_used(),
				  mem_pool);
  cr_list.Init_Head(Hash_bucket(hash_idx));
  retv->Set_usecnt(0);
  retv->Reset_flag((CR_FLAG)(CF_DEF_BY_PHI | CF_DEF_BY_CHI));
  cr_list.Add_it(retv, hash_idx, this);
  return retv;
}

// search bucket, error if not found
CODEREP *
CODEMAP::Find_CR(IDX_32 hash_idx, CODEREP *cr, INT32 mu_vsym_depth, BOOL ok_to_fail)
{ CODEREP_CONTAINER cr_list;
  cr_list.Init_Head(Hash_bucket(hash_idx));
  CODEREP *retv = cr_list.Find_cr(cr, mu_vsym_depth, Sym());
  if (retv) {
    if (!ok_to_fail) retv->IncUsecnt_rec();
    return retv;
  }
  if (!ok_to_fail)
    FmtAssert(FALSE,("CODEREP::Find_CR, CR was not found"));
  return NULL; // to satisfy compiler
}

// remove this node from htable; node must not be CK_VAR
void	      
CODEMAP::Remove(CODEREP *cr)
{
  IDX_32 hash_idx = Hash(cr);
  CODEREP_CONTAINER cr_list;
  cr_list.Init_Head(Hash_bucket(hash_idx));
  cr_list.Delete_it(cr, hash_idx, this);
}

// ======================================================================
// For ILOAD
// ======================================================================
CODEREP *
CODEMAP::Hash_Ivar(CODEREP *cr, INT32 mu_vsym_depth)
{ 
  Is_True(cr->Kind() == CK_IVAR,("CODEMAP::Hash_Ivar, wrong kind"));
  Is_True(cr->Ilod_base() || cr->Istr_base(),("CODEMAP::Hash_Ivar, no base"));

  CODEREP *new_cr = Alloc_stack_cr(0);
  OPCODE   opc;
  INT      need_cvt;
  CODEREP *exist_cr;
  IDX_32   hash_idx;

  /* CVTL-RELATED start (correctness) */
  // change {I,U}4{I,U}8ILOD to {I,U}4{I,U}8CVT({I,U}8{I,U}8ILOD) (truncation)
  if (cr->Dsctyp() != MTYPE_BS &&
      MTYPE_is_integral(cr->Dtyp()) && MTYPE_is_integral(cr->Dsctyp()) &&
      MTYPE_size_min(cr->Dsctyp()) > MTYPE_size_min(cr->Dtyp())) {
    need_cvt = Need_type_conversion(cr->Dsctyp(), cr->Dtyp(), &opc);
    Is_True(need_cvt == NEED_CVT, ("CODEMAP::Hash_Ivar: type err"));

    cr->IncUsecnt();
    cr->Set_dtyp(cr->Dsctyp());
    hash_idx = Hash_ivar(cr, cr->Ilod_base()?cr->Ilod_base():cr->Istr_base());
    exist_cr = Find_CR(hash_idx,cr,mu_vsym_depth,TRUE);
    if (exist_cr == NULL) {
      exist_cr = Append_CR(hash_idx,cr);
    }
    else { // pick stronger alignment
      TY_IDX exist_addr_ty = exist_cr->Ilod_base_ty();
      TY_IDX cr_addr_ty = cr->Ilod_base_ty();
      if (cr->Opr() == OPR_ILOAD && exist_addr_ty && cr_addr_ty &&
	  TY_kind(exist_addr_ty) == KIND_POINTER &&
	  TY_kind(cr_addr_ty) == KIND_POINTER &&
	  TY_align_exp(TY_pointed(cr_addr_ty)) <
	      TY_align_exp(TY_pointed(exist_addr_ty)))
	exist_cr->Set_ilod_base_ty(cr_addr_ty);
    }
    new_cr->Init_expr(opc, exist_cr);
    return Hash_Op(new_cr);
  }

  // Set the sign_extd flag for ILOAD of integral variable
  cr->Set_sign_extension_flag();
  hash_idx = Hash_ivar(cr, cr->Ilod_base()?cr->Ilod_base():cr->Istr_base());
  exist_cr = Find_CR(hash_idx,cr,mu_vsym_depth,TRUE);
  if (exist_cr == NULL) {
    return Append_CR(hash_idx,cr);
  } else {
     // pick stronger alignment
     TY_IDX exist_addr_ty = exist_cr->Ilod_base_ty();
     TY_IDX cr_addr_ty = cr->Ilod_base_ty();
     if (cr->Opr() == OPR_ILOAD && exist_addr_ty && cr_addr_ty &&
	 TY_kind(exist_addr_ty) == KIND_POINTER &&
	 TY_kind(cr_addr_ty) == KIND_POINTER &&
	 TY_align_exp(TY_pointed(cr_addr_ty)) <
	     TY_align_exp(TY_pointed(exist_addr_ty)))
       exist_cr->Set_ilod_base_ty(cr_addr_ty);

#ifdef Is_True_On
    BOOL is_sign_extd = exist_cr->Is_sign_extd();
    exist_cr->Set_sign_extension_flag();
    Is_True(is_sign_extd==exist_cr->Is_sign_extd(),("CODEMAP::Hash_Ivar: existing cr's sign extension flag is not consistent"));
#endif

    if (exist_cr->Is_sign_extd()==cr->Is_sign_extd()) // no type conversion
      return exist_cr;

    need_cvt =
      Need_load_type_conversion(exist_cr->Is_sign_extd(), cr->Is_sign_extd(),
                                cr->Dtyp(), cr->Dsctyp(), &opc);
    Is_True(need_cvt != NOT_AT_ALL, ("CODEMAP::Hash_Ivar: type err"));

    cr->IncUsecnt();
    new_cr->Init_expr(opc, exist_cr);
    if (need_cvt == NEED_CVTL) {
      new_cr->Set_offset(MTYPE_size_min(cr->Dsctyp()));
    }
    return Hash_Op(new_cr);
  }
  /* CVTL-RELATED finish */
}

//----------------------------------------------------------------------------

CODEREP *
CODEMAP::Add_lda_node(CODEREP *lda, INT64 new_offset)
{
  CODEREP *cr = Alloc_stack_cr(0);
  cr->Copy(*lda);
  cr->Set_offset(new_offset);
  return Hash_Lda(cr);
}


CODEREP *
CODEMAP::Add_unary_node(OPCODE op, CODEREP *kid)
{
  CODEREP *cr = Alloc_stack_cr(0);
  cr->Init_op( op, 1 );
  cr->Set_opnd( 0, kid);
  if (cr->Opr() == OPR_EXTRACT_BITS) printf("This is a disaster\n");
  return Hash_Op(cr);
}


CODEREP*
CODEMAP::Add_bin_node(OPCODE op, CODEREP *kid0, CODEREP *kid1)
{
  CODEREP *cr = Alloc_stack_cr(1);
  cr->Init_op( op, 2 );
  cr->Set_opnd( 0, kid0);
  cr->Set_opnd( 1, kid1);
  return Hash_Op(cr);
}


CODEREP*
CODEMAP::Add_tertiary_node(OPCODE op, CODEREP *kid0, CODEREP *kid1, CODEREP *kid2)
{
  CODEREP *cr = Alloc_stack_cr(2);
  cr->Init_op( op, 3 );
  cr->Set_opnd( 0, kid0);
  cr->Set_opnd( 1, kid1);
  cr->Set_opnd( 2, kid2);
  return Hash_Op(cr);
}

CODEREP *
CODEMAP::Add_nary_node(OPCODE op, CODEREP **kids, mUINT32 n)
{
  CODEREP *cr = Alloc_stack_cr(n - 1);

  mUINT32       i;

  cr->Init_op( op, n );
  for (i = 0; i < n; i++) {
    cr->Set_opnd( i, kids[i] );
  }
  return Hash_Op(cr);
}

CODEREP*
CODEMAP::Add_unary_node_and_fold(OPCODE op, CODEREP *kid)
{
  FOLD         ftmp;
  CODEREP      *crtmp;
  CODEREP      *cr;

  cr = Add_unary_node(op, kid);
  if ((crtmp = ftmp.Fold_Expr(cr)) != NULL)
    cr = crtmp;
  return cr;
}

BOOL WOPT_Enable_Canon_Compare=TRUE;

CODEREP*
CODEMAP::Add_bin_node_and_fold(OPCODE op,
                               CODEREP *kid0,
                               CODEREP *kid1,
                               BB_NODE *bb)
{
  FOLD         ftmp;
  CODEREP      *crtmp;
  CODEREP      *cr;

  cr = Alloc_stack_cr(1);
  cr->Init_op( op, 2 );
  cr->Set_opnd( 0, kid0);
  cr->Set_opnd( 1, kid1);

  BOOL do_canonicalization = TRUE;
  BOOL modified;
  if ((crtmp = ftmp.Fold_Expr(cr)) != NULL) {
    if (crtmp->Kind() == CK_OP && OPCODE_is_compare(crtmp->Op()) &&
        WOPT_Enable_Canon_Compare && _phase == MAINOPT_PHASE && bb) {
      cr = Alloc_stack_cr(1);
      cr->Init_op( crtmp->Op(), 2 );
      cr->Set_opnd(0, crtmp->Get_opnd(0));
      cr->Set_opnd(1, crtmp->Get_opnd(1));

      do_canonicalization = !Canonicalize_compare(cr, bb, &modified);
      return Hash_Op(cr, do_canonicalization);
    }
    else
      return crtmp;
  }
  if (WOPT_Enable_Canon_Compare && _phase == MAINOPT_PHASE && bb)
    do_canonicalization = !Canonicalize_compare(cr, bb, &modified);

  return Hash_Op(cr, do_canonicalization);
}


CODEREP *
CODEMAP::Add_expr_and_fold(CODEREP *cr) 
{
  switch (cr->Kind()) {
  case CK_VAR:	
    return NULL;
  case CK_LDA:	 
  case CK_CONST: 
  case CK_RCONST:
  case CK_IVAR:  
    return Rehash(cr);
  case CK_OP:
    {
      FOLD ftmp;
      CODEREP *newcr = ftmp.Fold_Expr(cr);
      return (newcr != NULL) ? newcr : Rehash(cr);
    }
  default:
    Fail_FmtAssertion("CODEMAP::Add_expr_and_fold: unhandled CK_KIND");
    // To satisfy the compiler:
    return NULL;
  }
}


CODEREP*
CODEMAP::Add_const(MTYPE typ, INT64 val)
{
  CODEREP *cr = Alloc_stack_cr(0);
  cr->Init_const(typ, val);
  cr = Hash_Const(cr);
  if (Split_64_Bit_Int_Ops && MTYPE_size_min(cr->Dtyp()) < MTYPE_size_min(typ))
    cr = Add_unary_node(OPCODE_make_op(OPR_CVT, typ, cr->Dtyp()), cr);
  return cr;
}

// This function is exported to other classes
// It creates a CANON_CR object on stack and use it to call the private
// member function CODEMAP::Add_expr.  After the function return, it
// then pull out the content from the CANON_CR to create the returning
// CODEREP.
CODEREP*
CODEMAP::Add_expr(WN       *wn,
		  OPT_STAB *opt_stab,
		  STMTREP  *stmt,
		  BOOL     *proped,
		  COPYPROP *copyprop)
{
  CANON_CR ccr;
  *proped |= Add_expr(wn, opt_stab, stmt, &ccr, copyprop);
  CODEREP *cr = ccr.Convert2cr(wn, this, *proped);
  return cr;
}

BOOL
CODEMAP::Canon_add_sub(WN       *wn,
                      OPT_STAB *opt_stab,
                      STMTREP  *stmt,
                      CANON_CR *ccr,
                      CODEREP  *cr,
		      COPYPROP *copyprop)
{
  CANON_CR kid1;
  CODEREP *retv;
  const OPERATOR opr = WN_operator(wn);
  const OPCODE op = WN_opcode(wn);
  BOOL  propagated = FALSE;

  propagated |= Add_expr(WN_kid(wn, 0), opt_stab, stmt, ccr, copyprop);
  propagated |= Add_expr(WN_kid(wn, 1), opt_stab, stmt, &kid1, copyprop);
  if (opr == OPR_ADD)
    ccr->Set_scale(ccr->Scale() + kid1.Scale());
  else ccr->Set_scale(ccr->Scale() - kid1.Scale());
  if (kid1.Tree() == NULL) 
    return propagated;
  if (ccr->Tree() == NULL) {
    if (opr == OPR_ADD)
      ccr->Set_tree(kid1.Tree());
    else
      ccr->Set_tree(Add_unary_node(
           OPCODE_make_op(OPR_NEG, OPCODE_rtype(op), MTYPE_V), kid1.Tree()));
    return propagated;
  }
  if (kid1.Tree() == NULL) {
    return propagated;
  }
  // both ccr and kid1 has tree
  cr->Set_opnd(0, ccr->Tree());
  cr->Set_opnd(1, kid1.Tree());
  if (propagated) {
    FOLD ftmp;
    retv = ftmp.Fold_Expr(cr);
    if (retv == NULL) { // either not folded or Fold_Expr has not rehashed
      retv = Hash_Op(cr);
    }
  }
  else {
    retv = Hash_Op(cr);
  }
  ccr->Set_tree(retv);
  return propagated;
}

BOOL
CODEMAP::Canon_neg(WN       *wn,
                   OPT_STAB *opt_stab,
                   STMTREP  *stmt,
                   CANON_CR *ccr,
                   CODEREP  *cr,
		   COPYPROP *copyprop)
{
  CODEREP *retv;
  BOOL propagated = Add_expr(WN_kid(wn, 0), opt_stab, stmt, ccr, copyprop);
  ccr->Set_scale(- ccr->Scale());
  if (ccr->Tree() == NULL) 
    return propagated;
  cr->Set_opnd(0, ccr->Tree());
  retv = Hash_Op(cr);
  ccr->Set_tree(retv);
  return propagated;
}

BOOL
CODEMAP::Canon_mpy(WN       *wn,
                   OPT_STAB *opt_stab,
                   STMTREP  *stmt,
                   CANON_CR *ccr,
                   CODEREP  *cr,
		   COPYPROP *copyprop)
{
  CANON_CR kid1;
  CODEREP *retv;
  BOOL  propagated = FALSE;
  BOOL  propagated1 = FALSE;

  propagated = Add_expr(WN_kid(wn, 0), opt_stab, stmt, ccr, copyprop);
  propagated1 = Add_expr(WN_kid(wn, 1), opt_stab, stmt, &kid1, copyprop);
  if (ccr->Tree() == NULL && kid1.Tree() == NULL) {
    ccr->Set_scale(ccr->Scale() * kid1.Scale());
    return propagated+propagated1;
  }
  if (kid1.Tree() == NULL) {
    if (kid1.Scale() == 0) {	// mult by 0: fold to 0
      ccr->Tree()->DecUsecnt_rec();
      ccr->Set_tree(NULL);
      ccr->Set_scale(0);
      return propagated+propagated1;
      }
    // materialize the constant part of ccr into tree
    cr->Set_opnd(0, ccr->Tree());
    cr->Set_opnd(1, Add_const(WN_rtype(wn), kid1.Scale()));
    ccr->Set_scale(ccr->Scale() * kid1.Scale());
    }
  else if (ccr->Tree() == NULL) {
    if (ccr->Scale() == 0) {	// mult by 0; fold to 0
      kid1.Tree()->DecUsecnt_rec();
      ccr->Set_tree(NULL);
      ccr->Set_scale(0);
      return propagated+propagated1;
      }
    // materialize the constant part of ccr into tree
    cr->Set_opnd(0, Add_const(WN_rtype(wn), ccr->Scale()));
    cr->Set_opnd(1, kid1.Tree());
    ccr->Set_scale(ccr->Scale() * kid1.Scale());
  }
  else {
    // both ccr and kid1 has tree
    cr->Set_opnd(0, ccr->Convert2cr(wn, this, propagated));
    cr->Set_opnd(1, kid1.Convert2cr(wn, this, propagated1));
    ccr->Set_scale(0);
  }
  if (propagated) {
    FOLD ftmp;
    retv = ftmp.Fold_Expr(cr);
    if (retv == NULL) { // either not folded or Fold_Expr has not rehashed
      retv = Hash_Op(cr);
    }
  }
  else {
    retv = Hash_Op(cr);
  }
  ccr->Set_tree(retv);
  return propagated+propagated1;
}

/* CVTL-RELATED start (performance) */
BOOL
CODEMAP::Canon_cvt(WN       *wn,
                   OPT_STAB *opt_stab,
                   STMTREP  *stmt,
                   CANON_CR *ccr,
                   CODEREP  *cr,
		   COPYPROP *copyprop)
{
  const OPCODE op = WN_opcode(wn);
  WN   *kid = WN_kid(wn, 0);
  BOOL  propagated = Add_expr(kid, opt_stab, stmt, ccr, copyprop);
#ifdef TARG_MIPS
  // U8I4CVT and I8I4CVT are nops so return kid, MIPS III and above
  // since U8I4CVT is required to preserve the type of its type for
  // Fix_var_type at emitter time, we do not delete U8I4CVT #329096
  if (Is_Target_ISA_M3Plus() && (op == OPC_I8I4CVT || op == OPC_U8I4CVT))
    return propagated;
#endif
  if ((Get_mtype_class(OPCODE_rtype(op)) & 
       Get_mtype_class(OPCODE_desc(op))) != 0 &&
      MTYPE_size_min(OPCODE_rtype(op)) == MTYPE_size_min(OPCODE_desc(op))) 
    return propagated;

  if ( WOPT_Enable_Cvt_Folding && 
      (op == OPC_I8U4CVT || op == OPC_U8U4CVT) && 
      WN_operator(kid) == OPR_LSHR ) {
    WN *bits = WN_kid1(kid);
    if ( WN_operator(bits) == OPR_INTCONST ) {
      INT bits_cnt = WN_const_val(bits);
      INT result_size = Actual_data_size(kid);
      if ( ( result_size <= MTYPE_size_min(MTYPE_I4) ) && 
	  ( ( bits_cnt & 0x1F ) != 0 ) )
	{
	  return propagated;
	}
    }
  }
  
  CODEREP *retv;
  CODEREP *expr;
  expr = ccr->Convert2cr(WN_kid0(wn), this, propagated);
  cr->Set_opnd(0, expr);
  retv = Hash_Op(cr);
  ccr->Set_tree(retv);
  ccr->Set_scale(0);
  return propagated;
}
/* CVTL-RELATED finish */

// Create a new coderep node for this new variable version in the htable
CODEREP *
CODEMAP::Add_def(IDTYPE st, mINT16 version, STMTREP *stmt,
		 MTYPE dtyp, MTYPE dsctyp, mINT32 ofst, TY_IDX ty,
		 UINT field_id, BOOL is_store)
{
  CODEREP         *cr = Alloc_stack_cr(0);
  CODEREP         *retv;
  INT              need_cvt = NOT_AT_ALL;
  OPCODE           opc;

  if (is_store && dtyp!=dsctyp && dsctyp != MTYPE_BS) {
    dtyp = Mtype_TransferSign(dsctyp,dtyp);
    if (MTYPE_size_min(dtyp)<=MTYPE_size_min(dsctyp))
      dtyp = dsctyp;  // necessary canonicalization
  }
  
#ifdef Is_True_On
  // check dtyp and dsctyp consistency
  if (MTYPE_is_float(dtyp)) {
    FmtAssert(MTYPE_is_float(dsctyp) && MTYPE_size_min(dtyp) == MTYPE_size_min(dsctyp),
	      ("CODEMAP::Add_def: dtyp and dsctyp mismatch."));
  }
  if (MTYPE_is_complex(dtyp)) {
    FmtAssert(MTYPE_is_float(dsctyp) && MTYPE_size_min(dtyp) == MTYPE_size_min(dsctyp),
	      ("CODEMAP::Add_def: dtyp and dsctyp mismatch."));
  }
#endif

  const AUX_STAB_ENTRY* aux = sym->Aux_stab_entry(st);
  if (ty == 0 && aux->Is_dedicated_preg()) {
    ST *s = aux->St();
    ty = ST_type(s);
  }

  /* CVTL-RELATED start (correctness) */
  if (_phase != MAINOPT_PHASE || 
      ! Only_Unsigned_64_Bit_Ops || 
      Delay_U64_Lowering) {

    // Fix 777333.   Also add CVT for I8I4LDID.
    if ( dtyp == MTYPE_U8 && dsctyp == MTYPE_U4 ) {
      // make U8U4LDID into U8U4CVT(U4U4LDID)
      if ( ! is_store ) 
        need_cvt = Need_type_conversion(dsctyp, dtyp, &opc);
      dtyp = MTYPE_U4;
    }
    if ( dtyp == MTYPE_I8 && dsctyp == MTYPE_I4 ) {
      // make I8I4LDID into I8I4CVT(I4I4LDID)
      if ( ! is_store ) 
        need_cvt = Need_type_conversion(dsctyp, dtyp, &opc);
      dtyp = MTYPE_I4;
    }
  }

  cr->Init_var(dtyp, st, version, dsctyp, ofst, ty, field_id);
  if (aux->Bit_size() > 0) {
    if (aux->Field_id() != 0) {
      dsctyp = MTYPE_BS;		// we're in preopt with H WHIRL
      cr->Set_dsctyp(MTYPE_BS);
    }
    else if (dsctyp != MTYPE_BS) {
      cr->Set_bit_field_valid();
      cr->Set_bit_offset_size (aux->Bit_ofst(), aux->Bit_size());
    }
  }
  // change {I,U}4{I,U}8LDID to {I,U}4{I,U}8CVT({I,U}8{I,U}8LDID) (truncation)
  if (MTYPE_is_integral(dtyp) && MTYPE_is_integral(dsctyp) &&
      MTYPE_size_min(dsctyp) > MTYPE_size_min(dtyp)) {
    need_cvt = Need_type_conversion(cr->Dsctyp(), cr->Dtyp(), &opc);
    Is_True(need_cvt == NEED_CVT, ("CODEMAP::Add_def: type err"));
    cr->Set_dtyp(dsctyp);
  }
  cr->Set_sign_extension_flag();
  /* CVTL-RELATED finish */

  retv = CXX_NEW_VARIANT(CODEREP(*cr), cr->Extra_space_used(), mem_pool);
  retv->Set_coderep_id(Next_coderep_id());
  retv->Set_usecnt(0);
  retv->Reset_flag((CR_FLAG)(CF_DEF_BY_PHI | CF_DEF_BY_CHI));
  if (stmt) retv->Set_defstmt(stmt);

  // link up coderep nodes of the same variable
  AUX_STAB_ENTRY *aux_entry = sym->Aux_stab_entry(st);
  aux_entry->Set_cr_list(aux_entry->Cr_list()->Prepend(retv));

  /* CVTL-RELATED start (correctness) */
  if (need_cvt == NEED_CVT) {
    CODEREP *new_cr = Alloc_stack_cr(0);
    new_cr->Init_expr(opc, retv);
    retv = Hash_Op(new_cr);
  }
  /* CVTL-RELATED finish */

  return retv;
}

// Determine if two array subscript expressions only MAY yield the same value.
// If they MUST or MUST NOT yield the same value, return FALSE.
// Algorithm only matches expression patterns in canonicalized form.
static BOOL
Indices_only_may_overlap(CODEREP *x, CODEREP *y)
{
  if (x == y) return FALSE; 
  if (inCODEKIND(x->Kind(), CK_LDA | CK_RCONST)) return TRUE;
  if (inCODEKIND(y->Kind(), CK_LDA | CK_RCONST)) return TRUE;
  if (x->Kind() == CK_CONST && y->Kind() == CK_CONST) {
    // both are integer constants
    return FALSE;
  }
  if (x->Kind() == CK_CONST || y->Kind() == CK_CONST) return TRUE;
  if (x->Kind() != CK_OP && y->Kind() != CK_OP) return TRUE;
  // now, one or both is CK_OP; if not CK_OP, can only be CK_VAR/CK_IVAR
  if (x->Kind() == CK_OP && y->Kind() == CK_OP) {
    if (x->Opr() != OPR_ADD && x->Opr() != OPR_SUB) 
      return TRUE;
    if (y->Opr() != OPR_ADD && y->Opr() != OPR_SUB) 
      return TRUE;
    if (x->Opr() != x->Opr())
      return TRUE;
    CODEREP *x0 = x->Opnd(0);
    CODEREP *x1 = x->Opnd(1);
    CODEREP *y0 = y->Opnd(0);
    CODEREP *y1 = y->Opnd(1);
    if (x0 == y0 && x1->Kind() == CK_CONST && y1->Kind() == CK_CONST)
      return FALSE;
    return TRUE;
  }
  if (x->Kind() != CK_OP) { // switch x and y
    CODEREP *t = y;
    y = x;
    x = t;
  }
  if (x->Opnd(0) != y) return TRUE;
  if (x->Kid_count() > 1 && 
      x->Opnd(1)->Kind() == CK_CONST && x->Opnd(1)->Const_val() != 0) 
    return FALSE;
  return TRUE;
}

// determine if two ivar nodes, in which one is a store and one is a load,
// may be referencing the same location; lval is the store side; when not
// sure, return TRUE
static BOOL
Ivars_may_overlap(CODEREP *lval, CODEREP *rval)
{
  if (lval->Is_ivar_volatile()) return TRUE;
  if (lval->Istr_base() == rval->Ilod_base()) {
    INT32 lsize = MTYPE_size_min(lval->Dsctyp()) >> 3; // in bytes
    INT32 rsize = MTYPE_size_min(rval->Dsctyp()) >> 3; // in bytes
    INT32 loffset = lval->Offset();
    INT32 roffset = rval->Offset();
    return (loffset >= roffset) && (loffset - roffset < rsize) ||
	   (roffset >= loffset) && (roffset - loffset < lsize);
  }
  else if (lval->Istr_base()->Kind() == CK_OP && 
	   lval->Istr_base()->Opr() == OPR_ADD &&
	   lval->Istr_base()->Opnd(1)->Kind() == CK_CONST &&
	   lval->Istr_base()->Opnd(0) == rval->Ilod_base()) {
    INT64 lsize = MTYPE_size_min(lval->Dsctyp()) >> 3; // in bytes
    INT64 rsize = MTYPE_size_min(rval->Dsctyp()) >> 3; // in bytes
    INT64 loffset = lval->Offset() + lval->Istr_base()->Opnd(1)->Const_val();
    INT64 roffset = rval->Offset();
    return (loffset >= roffset) && (loffset - roffset < rsize) ||
	   (roffset >= loffset) && (roffset - loffset < lsize);
  }
  else if (rval->Ilod_base()->Kind() == CK_OP && 
	   rval->Ilod_base()->Opr() == OPR_ADD &&
	   rval->Ilod_base()->Opnd(1)->Kind() == CK_CONST &&
	   rval->Ilod_base()->Opnd(0) == lval->Istr_base()) {
    INT64 lsize = MTYPE_size_min(lval->Dsctyp()) >> 3; // in bytes
    INT64 rsize = MTYPE_size_min(rval->Dsctyp()) >> 3; // in bytes
    INT64 loffset = lval->Offset();
    INT64 roffset = rval->Offset() + rval->Ilod_base()->Opnd(1)->Const_val();
    return (loffset >= roffset) && (loffset - roffset < rsize) ||
	   (roffset >= loffset) && (roffset - loffset < lsize);
  }
  else if (lval->Offset() == rval->Offset()) { // analyze array nodes
    CODEREP *lbase = lval->Istr_base();
    CODEREP *rbase = rval->Ilod_base();
    if (lbase->Kind() != CK_OP || lbase->Opr() != OPR_ARRAY ||
        rbase->Kind() != CK_OP || rbase->Opr() != OPR_ARRAY) 
      return TRUE;
    INT32 kid_count = lbase->Kid_count();
    if (kid_count != rbase->Kid_count())
      return TRUE;
    if (lbase->Opnd(0) != rbase->Opnd(0))
      return TRUE;
    INT32 dim = kid_count >> 1;
    INT32 i;
    for (i = 1; i <= dim; i++)
      if (lbase->Opnd(i) != rbase->Opnd(i))
	return TRUE;
    // analyze the subscripts
    BOOL subscripts_same = TRUE;
    for (i = 1; i <= dim; i++) {
      if (Indices_only_may_overlap(lbase->Opnd(dim+i), rbase->Opnd(dim+i)))
	return TRUE;
      if (lbase->Opnd(dim+i) != rbase->Opnd(dim+i)) 
	subscripts_same = FALSE;
    }
    return subscripts_same;
  }
  else return TRUE;
}

CODEREP *
CODEMAP::Add_idef(OPCODE opc, OCC_TAB_ENTRY *occ, STMTREP *stmt,
		  MU_NODE *mnode, MTYPE dtyp, MTYPE dsctyp, TY_IDX lodty,
		  UINT field_id, mINT32 ofst, CODEREP *size, CODEREP *lbase,
		  CODEREP *sbase)
{
  CODEREP          *cr = Alloc_stack_cr(IVAR_EXTRA_NODE_CNT);
  CODEREP          *retv;
  OPERATOR         oper = OPCODE_operator(opc);

  cr->Init_ivar(opc, dtyp, occ, dsctyp, lodty, lbase, sbase,
		ofst, size, field_id);
  // all IVAR codereps have a mu_node
  cr->Set_ivar_mu_node(mnode);
  if (stmt) {
    cr->Set_ivar_defstmt(stmt);
    retv = Hash_New_ivar(cr);
  } 
  else {
    INT32 depth = 0;
    if (WOPT_Enable_VN_Full && 
	mnode != NULL &&
	OPERATOR_is_scalar_iload (cr->Opr()) &&
	! cr->Is_ivar_volatile()) {
      // find search distance in the use-def chain of the virtual variable in
      // terms of depth down its coderep stack 
      AUX_ID aux = mnode->OPND()->Aux_id();
      BOOL alias_with_cr = FALSE;
      CODEREP *vsym;
      if (Sym()->NULL_coderep(aux))
	alias_with_cr = TRUE;
      while (! alias_with_cr && Sym()->Elements_coderep(aux) > depth) {
	vsym = Sym()->Top_nth_coderep(aux, depth);
	STMTREP *dstmt = (vsym ? vsym->Defstmt() : NULL);
	alias_with_cr = (dstmt == NULL || 
			 vsym->Is_flag_set(CF_IS_ZERO_VERSION) ||
			 ! OPERATOR_is_scalar_istore (dstmt->Opr()) ||
			 Ivars_may_overlap(dstmt->Lhs(), cr));
	if (! alias_with_cr) depth++;
      }
      // if zero version, need to back down by 1 because zero version 
      // cannot be used in finding match
      if (depth > 0 && 
	  (vsym == NULL || vsym->Is_flag_set(CF_IS_ZERO_VERSION)))
	depth--;
    }
    OPERATOR oper = OPCODE_operator(opc);
    if (OPERATOR_is_scalar_iload (oper) ||
	oper == OPR_ILOADX ||
	oper == OPR_PARM) 
      retv = Hash_Ivar(cr, depth);
    else {
      // OPR_MLOAD OPR_PREFETCH
      retv = Hash_New_ivar(cr);
    }
  }

  return retv;
}

/* CVTL-RELATED start (correctness) */
// ====================================================================
// Return a CODEREP with right sign extension
// ====================================================================
CODEREP *
CODEREP::Var_type_conversion(CODEMAP *htable, MTYPE to_dtyp,
			     MTYPE to_dsctyp, TY_IDX to_ty, UINT field_id)
{
  Is_True(Kind() == CK_VAR, ("CODEREP::Var_type_conversion: not CK_VAR"));
  Is_True(to_dtyp != MTYPE_BS, ("CODEREP::Var_type_conversion: dtyp cannot be MTYPE_BS"));

  CODEREP * retval = this;
  if ( Is_flag_set(CF_MADEUP_TYPE) || Dsctyp() == MTYPE_UNKNOWN ) {
    // the type was made up by SSA, so change it
    Set_dtyp( to_dtyp );
    Set_dsctyp( to_dsctyp );
    Set_sign_extension_flag();
    Set_lod_ty( to_ty );
    Set_field_id (field_id);
    Reset_flag( CF_MADEUP_TYPE );
  } else {

    if (to_dsctyp != MTYPE_BS && Dsctyp() != MTYPE_BS) {
      if (MTYPE_size_min( Dsctyp() ) != MTYPE_size_min( to_dsctyp )) {
        Is_True(ST_class(htable->Opt_stab()->St(Aux_id())) == CLASS_PREG,
	        ("CODEREP::Var_type_conversion: load from different dsc sizes"));
        DevWarn("CODEREP::Var_type_conversion: (fixed) loads of different size share aux_id %d",Aux_id());
  
        if ( MTYPE_size_min(to_dsctyp) > MTYPE_size_min(Dsctyp()) ) {
	  Set_dsctyp( Mtype_TransferSign(Dsctyp(), to_dsctyp ) );
	  Set_sign_extension_flag();
        } else {
	  to_dsctyp = Mtype_TransferSign(to_dsctyp, Dsctyp());
        }
      }
    }
    else Is_True(to_dsctyp == Dsctyp(),
		 ("CODEREP::Var_type_conversion: MTYPE_BS does not match"));

    BOOL is_sign_extd = Is_sign_extd();
    BOOL to_sign_extd = Sign_extended( to_dtyp, to_dsctyp );

#ifdef Is_True_On
    Set_sign_extension_flag();
    Is_True(is_sign_extd == Is_sign_extd(),
	    ("CODEREP::Var_type_conversion: inconsistent sign_extd flag"));
#endif

    if ( is_sign_extd != to_sign_extd ) {
      OPCODE opc;
      INT need_cvt =
	Need_load_type_conversion( is_sign_extd, to_sign_extd, 
				  to_dtyp, to_dsctyp, &opc );
      Is_True(need_cvt != NOT_AT_ALL, 
	      ("CODEREP::Var_type_conversion: type err"));
      CODEREP *new_cr = Alloc_stack_cr(0);
      IncUsecnt();
      new_cr->Init_expr( opc, this );
      if (need_cvt == NEED_CVTL) {
	if (to_dsctyp != MTYPE_BS)
	  new_cr->Set_offset( MTYPE_size_min( to_dsctyp ) );
	else {
	  UINT cur_field_id = 0;
	  UINT64 field_offset = 0;
	  FLD_HANDLE fld = FLD_And_Offset_From_Field_Id(to_ty, field_id,
						cur_field_id, field_offset);
	  new_cr->Set_offset(FLD_bsize(fld));
        }
      }
      retval = htable->Hash_Op( new_cr );
    }
  }

  return retval;
}
/* CVTL-RELATED finish */

CODEREP *
CODEMAP::Cur_def(WN *wn, OPT_STAB *opt_stab)
{
  IDTYPE du     = WN_ver(wn);
  FmtAssert(du != 0,
	    ("CODEMAP::Cur_def: WN_st wasn't set correctly"));

  MTYPE  dtyp   = WN_rtype( wn );
  MTYPE  dsctyp = WN_desc( wn );
  TY_IDX ty     = WN_ty( wn );

  CODEREP *retv = opt_stab->Du_coderep( du );
  if ( retv == NULL ) {
    retv = Add_def(opt_stab->Du_aux_id( du ),
		   opt_stab->Du_version( du ),
		   NULL/*stmtrep*/,
		   dtyp,
		   dsctyp,
		   WN_offset(wn),
		   ty,
		   WN_field_id(wn),
		   FALSE);
    CODEREP *retv_var = ( retv->Kind() == CK_VAR ) ? retv : retv->Opnd(0);
    retv_var->Set_lod_ty( ty ) ;
    if ( opt_stab->Du_is_volatile( du ) ) { 
      // set the volatile attribute for this var, even if this
      // particular reference is not cast as volatile
      retv_var->Set_var_volatile();
    }
    else {
      // only cache non-volatile codereps
      opt_stab->Du_set_coderep( du, retv_var );
    }
  }
  else { 
    // cached value should not be a volatile
    Is_True( ! retv->Is_var_volatile() &&
             ! opt_stab->Du_is_volatile( du ),
      ("CODEMAP::Cur_def: Cached volatile reference [%3d]", retv->Aux_id()) );

    /* CVTL-RELATED start (correctness) */
    // change {I,U}4{I,U}8LDID to {I,U}4{I,U}8CVT({I,U}8{I,U}8LDID)
    // (truncation)
    if (dsctyp != MTYPE_BS &&
	MTYPE_is_integral(dtyp) && MTYPE_is_integral(dsctyp) &&
	MTYPE_size_min(dsctyp) > MTYPE_size_min(dtyp)) {
      Is_True(MTYPE_size_min(dtyp) == MTYPE_size_min(MTYPE_I4),
	      ("CODEMAP::Add_def: unexpected type"));
      retv = retv->Var_type_conversion(this, dsctyp, dsctyp, ty, WN_field_id(wn));

      CODEREP *new_cr = Alloc_stack_cr(0);
      OPCODE   opc; 
      INT      need_cvt = Need_type_conversion(dsctyp, dtyp, &opc);

      Is_True(need_cvt == NEED_CVT,("CODEMAP::Cur_def: type err"));

      retv->IncUsecnt();
      new_cr->Init_expr( opc, retv );
      retv = Hash_Op( new_cr );

    } else {
      retv = retv->Var_type_conversion(this, dtyp, dsctyp, ty, WN_field_id(wn));
      if (dsctyp == MTYPE_BS) {
	CODEREP *v = retv;
	if (v->Kind() == CK_OP) 
	  v = v->Opnd(0);  // skip the CVTL inserted earlier
	// these fields could be wrong because there was no real node to look
	// at earlier, so fix them here
	v->Set_lod_ty(ty);
	v->Set_field_id(WN_field_id(wn));
	v->Set_offset(WN_offset(wn));
      }
    }
    /* CVTL-RELATED finish */

  }

  return retv;
}

// ====================================================================
// Convert a TCON value into a coderep
// ====================================================================

CODEREP *
CODEMAP::Add_tcon(TCON_IDX tc)
{
  CODEREP *cr = Alloc_stack_cr(0);
  MTYPE mtype = TCON_ty(Tcon_Table[tc]);

  switch ( mtype ) {
    case MTYPE_B:
    case MTYPE_I1:
    case MTYPE_I2:
    case MTYPE_I4:
    case MTYPE_I8:
    case MTYPE_U1:
    case MTYPE_U2:
    case MTYPE_U4:
    case MTYPE_U8:
      cr->Init_const(mtype, Targ_To_Host(Tcon_Table[tc]));
      return Hash_Const(cr);

    case MTYPE_F4:
    case MTYPE_F8:
    case MTYPE_FQ:
    case MTYPE_C4:
    case MTYPE_C8:
    case MTYPE_CQ:
      {
        ST *new_sym = New_Const_Sym(tc, MTYPE_To_TY(mtype));
	cr->Init_rconst(mtype, new_sym);
      }
      return Hash_Rconst(cr);
    
    default:
      FmtAssert( FALSE,
	("CODEMAP::Add_tcon: unexpected tcon type: %s",
	 Mtype_Name(mtype)) );
      return NULL;
  }
}

// ====================================================================
// wn is an ILOAD node whose base is an LDA; if it can be folded to an
// LDID, do the folding and return the CK_VAR node; otherwise, return NULL
// ====================================================================
CODEREP *
CODEMAP::Iload_folded(WN *wn, 			// the iload node
		      CANON_CR *base_ccr)	// gives the LDA info
{
  const OPCODE op = WN_opcode(wn);
  const OPERATOR opr = OPCODE_operator(op);
  CODEREP *retv;
  MU_NODE *mnode = Sym()->Get_mem_mu_node(wn);
  Is_True(mnode != NULL && mnode->Opnd() != 0, ("empty mu list at ILOAD"));
  // mu list only has its own virtual var; use virtual var to get to 
  // defstmt to look at the chi list
  AUX_ID vaux = Sym()->Du_aux_id(mnode->Opnd());
  
  CODEREP *vsymcr = Sym()->Du_coderep(mnode->Opnd());
  if ( vsymcr == NULL )
    return NULL;

  STMTREP *dstmt = vsymcr->Defstmt();
  if (dstmt != NULL) {
    // go thru chi list to find the scalar var version
    CHI_NODE *cnode;
    CHI_LIST_ITER chi_iter;
    FOR_ALL_NODE(cnode, chi_iter, Init(dstmt->Chi_list())) {
      vaux = cnode->Aux_id();
      if (!Sym()->Is_real_var(vaux))
        continue;
      INT64 v_ofst;
      ST *v_st;
      Expand_ST_into_base_and_ofst(Sym()->St(vaux), 
				   Sym()->St_ofst(vaux), &v_st, &v_ofst);
      if (v_st == base_ccr->Tree()->Lda_base_st() &&
	  v_ofst == base_ccr->Scale() && 
	  0 == base_ccr->Tree()->Offset() && // TODO: not needed if canonicalize in preopt
	  Sym()->Aux_stab_entry(vaux)->Mclass() == 
		  Get_mtype_class(OPCODE_rtype(op)) &&
	  Sym()->Aux_stab_entry(vaux)->Byte_size() * 8 == 
		  MTYPE_size_min(OPCODE_desc(op)) &&
	  (opr == OPR_ILOAD && Sym()->Aux_stab_entry(vaux)->Bit_size() == 0 ||
	   OPCODE_desc(op) == MTYPE_BS && 
	   WN_field_id(wn) == Sym()->Field_id(vaux) ||
	   opr == OPR_ILDBITS && 
	   Sym()->Aux_stab_entry(vaux)->Bit_ofst() == WN_bit_offset(wn) &&
	   Sym()->Aux_stab_entry(vaux)->Bit_size() == WN_bit_size(wn))) {
        base_ccr->Tree()->DecUsecnt();	// for the LDA coderep
	_ssa->Value_number_mu_node(Sym()->Get_mem_mu_node(wn));
	if (! cnode->Live()) {
	  _ssa->Resurrect_chi(cnode);
          cnode->RESULT()->Set_defstmt(dstmt);
	}
        retv = cnode->RESULT();

	if (retv->Is_flag_set(CF_IS_ZERO_VERSION)) {
	  // create new coderep when it's the zero version
	  retv = Add_def(vaux,
			 0/*dummy*/,
			 dstmt, // use the defstmt of this chi 
			 OPCODE_rtype(op),
			 OPCODE_desc(op),
			 Sym()->St_ofst(vaux)/*base_ccr->Scale()*/,
			 TY_pointed(WN_load_addr_ty(wn)), WN_field_id(wn),TRUE);
	  retv->Set_flag(CF_DEF_BY_CHI);
	  retv->Set_flag(CF_INCOMPLETE_USES);
	  retv->Set_defchi(cnode);
	  cnode->Set_RESULT(retv);	// change the chi to point to this
	  dstmt->Recompute_has_zver();
	  // make new node the current version 
	  Sym()->Pop_coderep(vaux);
	  Sym()->Push_coderep(vaux, retv);
	}
	goto return_pt;
      }
    }

    // check if dstmt is actually the matching STID 
    retv = dstmt->Lhs();
    if (retv != NULL && retv->Kind() == CK_VAR) {
      INT64 v_ofst;
      ST *v_st;
      Expand_ST_into_base_and_ofst(Sym()->St(retv->Aux_id()),
				   Sym()->St_ofst(retv->Aux_id()), &v_st, &v_ofst);

      if (v_st == base_ccr->Tree()->Lda_base_st() &&
	  v_ofst == base_ccr->Scale() &&
	  0 == base_ccr->Tree()->Offset() && // TODO: not needed if canonicalize in preopt
	  Get_mtype_class(retv->Dtyp()) == Get_mtype_class(OPCODE_rtype(op)) &&
	  MTYPE_size_min(retv->Dsctyp()) == MTYPE_size_min(OPCODE_desc(op)) &&
	  (opr == OPR_ILOAD && ! retv->Bit_field_valid() &&
	   (WN_desc(wn) != MTYPE_BS && retv->Dsctyp() != MTYPE_BS ||
	    WN_desc(wn) == MTYPE_BS && WN_field_id(wn) == retv->Field_id()) ||
	   opr == OPR_ILDBITS && retv->Bit_field_valid() &&
	   retv->Bit_offset() == WN_bit_offset(wn) &&
	   retv->Bit_size() == WN_bit_size(wn))) {
	base_ccr->Tree()->DecUsecnt();	// for the LDA coderep
	_ssa->Value_number_mu_node(Sym()->Get_mem_mu_node(wn));
	goto return_pt;
      }
    }
  }
  else if (vsymcr->Is_flag_set(CF_DEF_BY_PHI)) {
    // go thru phi list to find the scalar var version
    PHI_LIST_ITER phi_iter;
    PHI_NODE *phi;
    FOR_ALL_ELEM(phi, phi_iter, Init(vsymcr->Defphi()->Bb()->Phi_list())) {
      vaux = phi->Aux_id();
      if (!Sym()->Is_real_var(vaux))
	continue;
      INT64 v_ofst;
      ST *v_st;
      Expand_ST_into_base_and_ofst(Sym()->St(vaux), 
				   Sym()->St_ofst(vaux), &v_st, &v_ofst);
      if (v_st == base_ccr->Tree()->Lda_base_st() &&
	  v_ofst == base_ccr->Scale() && 
	  0 == base_ccr->Tree()->Offset() && // TODO: not needed if canonicalize in preopt
	  Sym()->Aux_stab_entry(vaux)->Mclass() == 
		  Get_mtype_class(OPCODE_rtype(op)) &&
	  Sym()->Aux_stab_entry(vaux)->Byte_size() * 8 ==
		  MTYPE_size_min(OPCODE_desc(op)) &&
	  (opr == OPR_ILOAD && Sym()->Aux_stab_entry(vaux)->Bit_size() == 0 ||
	   OPCODE_desc(op) == MTYPE_BS && 
	   WN_field_id(wn) == Sym()->Field_id(vaux) ||
	   opr == OPR_ILDBITS && 
	   Sym()->Aux_stab_entry(vaux)->Bit_ofst() == WN_bit_offset(wn) &&
	   Sym()->Aux_stab_entry(vaux)->Bit_size() == WN_bit_size(wn))) {
        base_ccr->Tree()->DecUsecnt();	// for the LDA coderep
	_ssa->Value_number_mu_node(Sym()->Get_mem_mu_node(wn));
	if (! phi->Live()) 
	  _ssa->Resurrect_phi(phi);
        retv = phi->RESULT();
  
	if (retv->Is_flag_set(CF_IS_ZERO_VERSION)) {
	  // create new coderep when it's the zero version
	  retv = Add_def(vaux,
			 0/*dummy*/,
			 dstmt, // use the defstmt of this chi 
			 OPCODE_rtype(op),
			 OPCODE_desc(op),
			 Sym()->St_ofst(vaux)/*base_ccr->Scale()*/,
			 TY_pointed(WN_load_addr_ty(wn)), WN_field_id(wn),TRUE);
	  retv->Set_flag(CF_DEF_BY_PHI);
	  retv->Set_defphi(phi);
	  retv->Set_flag(CF_INCOMPLETE_USES);
	  // make new node the current version 
	  Sym()->Pop_coderep(vaux);
	  Sym()->Push_coderep(vaux, retv);
	  phi->Set_result(retv);	// change the phi to point to this
	}
	goto return_pt;
      }
    }
  }
  return NULL;

 return_pt:
  // insert necessary type conversion
  {
    /* CVTL-RELATED start (correctness) */
    retv = retv->Var_type_conversion(this, OPCODE_rtype(op),
				     OPCODE_desc(op), WN_ty(wn),
				     WN_field_id(wn));
    /* CVTL-RELATED finish */
    retv->Set_flag(CF_FOLDED_LDID); // hint for coloring
    retv->IncUsecnt();
    _num_iloadfolds++;
    return retv;
  }

}

// update the prefetch record
void
CODEMAP::Update_pref(CODEREP *ivar) const
{
  PF_POINTER *pf = ivar->Ivar_occ()->Pf_pointer();
  if (pf != NULL) {
    if (VISITED_CM(pf)) return;
    WN      *pref_wn;
    STMTREP *pref_stmt;
    if ((pref_wn = PF_PTR_wn_pref_1L(pf)) != NULL) {
      pref_stmt = (STMTREP*) WN_MAP_Get(_prefetch_map, pref_wn);
      // Once LNO starts processing pragma and olimit regions, the else
      // part is all that is required here. (put in an Is_True(!pref_stmt))
      if (pref_stmt == NULL) {
	DevWarn("532176, 457243, LNO is introducing cross-region prefetches");
	PF_PTR_wn_pref_1L(pf) = (WN *) NULL;
      } else
	PF_PTR_wn_pref_1L(pf) = (WN *) pref_stmt;
    }
    if ((pref_wn = PF_PTR_wn_pref_2L(pf)) != NULL) {
      pref_stmt = (STMTREP*) WN_MAP_Get(_prefetch_map, pref_wn);
      // Once LNO starts processing pragma and olimit regions, the else
      // part is all that is required here. (put in an Is_True(!pref_stmt))
      if (pref_stmt == NULL) {
	DevWarn("532176, 457243, LNO is introducing cross-region prefetches");
	PF_PTR_wn_pref_2L(pf) = (WN *) NULL;
      } else
	PF_PTR_wn_pref_2L(pf) = (WN *) pref_stmt;
    }
    SET_VISITED_CM(pf);
  }
}

// ====================================================================
// This overloaded member function recursively traverse the WN tree
// and build the CODEREP bottom up in the canonical form.  During this
// process, the constant part of the tree and the rest of the tree are
// kept in the CANON_CR object.  The return value tells if something 
// within the tree has been replaced due to copy propagation, so that
// the parent would call the simplifier for the entire tree.
// ====================================================================
BOOL
CODEMAP::Add_expr(WN *wn, OPT_STAB *opt_stab, STMTREP *stmt, CANON_CR *ccr,
		  COPYPROP *copyprop)
{
  // given a WN node, add it to the coderep hash and return the
  // coderep for it.
  CODEREP          *cr = Alloc_stack_cr(WN_kid_count(wn)+IVAR_EXTRA_NODE_CNT);
  CODEREP          *retv;
  const OPCODE      op = WN_opcode(wn);
  const OPERATOR    oper = WN_operator(wn);
  BOOL  propagated = FALSE;

  FmtAssert (OPCODE_is_expression(op) || OPCODE_is_fake(op),
	     ("CODEMAP::Hash: opcode %s is not an expression",OPCODE_name(op)));

  if (OPCODE_is_leaf(op)) {
    if (OPERATOR_is_scalar_load (oper)) {
      retv = Cur_def(wn, opt_stab);
      Is_True(retv->Kind()==CK_VAR || (retv->Kind()==CK_OP
				       && (retv->Opr()==OPR_CVT ||
					   retv->Opr()==OPR_CVTL)),
	      ("CODEMAP::Add_expr: return illegal cr from Cur_def"));
      CODEREP * retv_var = (retv->Kind()==CK_VAR) ? retv : retv->Opnd(0);
      if (retv_var->Is_var_volatile())
	stmt->Set_volatile_stmt();
      if (opt_stab->NULL_coderep(retv_var->Aux_id()))
        opt_stab->Push_coderep(retv_var->Aux_id(), retv_var);

      retv->IncUsecnt();
      if (WOPT_Enable_Input_Prop && !copyprop->Disabled()) {
        CODEREP *newtree = copyprop->Prop_var(retv_var, stmt->Bb(), TRUE, TRUE,TRUE/*in_array*/);
        if (newtree) {
	  if (retv->Kind() == CK_VAR) 
	    retv = newtree;
	  else {
	    CODEREP *cr = Alloc_stack_cr(0);
	    newtree->IncUsecnt();
	    cr->Init_expr(retv->Op(), newtree);
	    if (retv->Opr() == OPR_CVTL) cr->Set_offset(retv->Offset());

            if (newtree->Kind() == CK_CONST) {
	      FOLD ftmp;
	      CODEREP *crtmp = ftmp.Fold_Expr(cr);
	      if (crtmp != NULL) 
		retv = Hash_Const(crtmp);
	      else
		retv = Hash_Op(cr);
	    }
            else
              retv = Hash_Op(cr);
          }
          if (retv->Kind() == CK_CONST) {
	    retv->DecUsecnt();
            ccr->Set_tree(NULL);
            ccr->Set_scale(retv->Const_val());
            return TRUE;
          }

          if (retv->Kind() == CK_OP) {
            // if CRSIMP flag is turned on, the opr is +,-,
            // and the kid1 is a constant, pull the constant out.
            const OPERATOR opr = retv->Opr();
            if (WOPT_Enable_CRSIMP &&
                (opr == OPR_ADD || opr == OPR_SUB) &&
                retv->Get_opnd(1)->Kind() == CK_CONST) {
                  CODEREP *cr = retv->Get_opnd(0);
                  cr->DecUsecnt();
                  ccr->Set_tree(cr);
                  cr = retv->Get_opnd(1);
                  cr->DecUsecnt();
                  if (opr == OPR_SUB)
                    ccr->Set_scale(0 - cr->Const_val());
                  else
                    ccr->Set_scale(cr->Const_val());
                  return TRUE;
                }
	  } // if (retv->Kind() == CK_OP)
	}
      }
      if ((Split_64_Bit_Int_Ops || Only_Unsigned_64_Bit_Ops) &&
	  MTYPE_is_integral(retv->Dtyp()) &&
	  MTYPE_size_min(retv->Dtyp()) < MTYPE_size_min(WN_rtype(wn))) {
	retv = Add_unary_node(OPCODE_make_op(OPR_CVT, WN_rtype(wn), 
			 Mtype_TransferSize(retv->Dtyp(), WN_rtype(wn))), retv);
      }
      ccr->Set_tree(retv);
      ccr->Set_scale(0);
      return TRUE;
    }
    else if (oper == OPR_LDA) {
      INT64 ofst;
      ST *lda_st;
      ST *wn_st = opt_stab->St(WN_aux(wn));
      if (_phase == MAINOPT_PHASE &&
          (! opt_stab->Is_up_level_var(wn_st) || WOPT_Enable_Canon_Uplevel)) {
	Expand_ST_into_base_and_ofst(wn_st,
				     opt_stab->St_ofst(WN_aux(wn)), &lda_st, &ofst);
        ccr->Set_scale(ofst);
        ofst = 0;
      } else {
	lda_st = opt_stab->St(WN_aux(wn));
	ofst = opt_stab->St_ofst(WN_aux(wn));
        ccr->Set_scale(0);
      }
	
      Is_True(opt_stab->Base(WN_aux(wn)) != NULL, ("base st is null."));
      cr->Init_lda(OPCODE_rtype(op), WN_aux(wn), 
		   (INT32)ofst, (TY_IDX) WN_ty(wn), lda_st);
      retv = Hash_Lda(cr);
      ccr->Set_tree(retv);
      return FALSE;
    }
    else if (oper == OPR_LDA_LABEL) {
      cr->Init_lda(WN_rtype(wn), (IDTYPE) 0, WN_label_number(wn), 
		   (TY_IDX) WN_ty(wn), NULL);
      cr->Set_flag(CF_LDA_LABEL);
      retv = Hash_Lda(cr);
      ccr->Set_tree(retv);
      ccr->Set_scale(0);
      return FALSE;
    }
    else if (oper == OPR_INTCONST) {
      ccr->Set_tree(NULL);
      ccr->Set_scale(WN_get_const_val(wn));
      return FALSE;
    }
    else if (oper == OPR_CONST) {
      cr->Init_rconst(OPCODE_rtype(op), WN_sym(wn));
      retv = Hash_Rconst(cr);
      ccr->Set_tree(retv);
      ccr->Set_scale(0);
      return FALSE;
    }
    else {
      FmtAssert(FALSE,("CODEMAP::Add_expr: leaf opcode %s is not handled",
		       OPCODE_name(op)));
      return FALSE;
    }
  }
  else if (OPERATOR_is_scalar_iload (oper)) {
    CANON_CR base_ccr;
    Add_expr(WN_kid(wn, 0), opt_stab, stmt, &base_ccr, copyprop);
    base_ccr.Set_scale(base_ccr.Scale() + WN_offset(wn));
    base_ccr.Trim_to_16bits(WN_kid(wn, 0), this);
    if (base_ccr.Tree() != NULL &&
	base_ccr.Tree()->Kind() == CK_LDA )
    {
      {
	// is the result of the load volatile?
	if (Lod_TY_is_volatile(WN_ty(wn))) 
	  goto no_fold_iload;

      // see if we point at a volatile object
	TY_IDX load_addr_ty = WN_load_addr_ty(wn);
	if (TY_is_volatile(load_addr_ty))
	  goto no_fold_iload;

      // may point to a volatile object
	TY_IDX pointed = TY_pointed(load_addr_ty);
	if (TY_is_volatile(pointed))
	  goto no_fold_iload;

      // fold to LDID
	if (retv = Iload_folded(wn, &base_ccr)) {
	  if ((Split_64_Bit_Int_Ops || Only_Unsigned_64_Bit_Ops) &&
	      MTYPE_is_integral(retv->Dtyp()) &&
	      MTYPE_size_min(retv->Dtyp()) < MTYPE_size_min(WN_rtype(wn))) {
	    retv = Add_unary_node(OPCODE_make_op(OPR_CVT, WN_rtype(wn), 
			     Mtype_TransferSize(retv->Dtyp(), WN_rtype(wn))), retv);
	  }
	  ccr->Set_tree(retv);
	  ccr->Set_scale(0);
	  return FALSE;
	}
      }

      // jump here if we're not going to fold
      no_fold_iload: ;
    }

    // In IA-64, ILOAD should have zero offset
    if (! Use_Load_Store_Offset && base_ccr.Scale() != 0) {
      MTYPE typ = WN_rtype(WN_kid(wn, 0));
      CODEREP *retv = Add_const(typ, base_ccr.Scale());
      if (base_ccr.Tree() != NULL)
	retv = Add_bin_node(OPCODE_make_op(OPR_ADD, typ, MTYPE_V),
			    base_ccr.Tree(), retv);
      base_ccr.Set_tree(retv);
      base_ccr.Set_scale(0);
    }

    _ssa->Value_number_mu_node(opt_stab->Get_mem_mu_node(wn));
    CODEREP *lbase = base_ccr.Tree() ? base_ccr.Tree() : Add_const(Pointer_type,(INT64)0);
    retv = Add_idef(op, opt_stab->Get_occ(wn), NULL,
		    opt_stab->Get_mem_mu_node(wn), WN_rtype(wn),
		    WN_desc(wn), WN_ty(wn), WN_field_id(wn),
		    base_ccr.Scale(), (CODEREP *) WN_load_addr_ty(wn),
		    lbase, NULL);
    Is_True(retv->Kind()==CK_IVAR || (retv->Kind()==CK_OP
				      && (retv->Opr()==OPR_CVT ||
					  retv->Opr()==OPR_CVTL)),
	    ("CODEMAP::Add_expr: return illegal cr from Add_idef"));
    CODEREP *retv_ivar = (retv->Kind()==CK_IVAR) ? retv : retv->Opnd(0);
    if (retv_ivar->Is_ivar_volatile()) 
      stmt->Set_volatile_stmt();
    Update_pref(retv_ivar);
    if (WOPT_Enable_Input_Prop) {
      CODEREP *newtree = copyprop->Prop_ivar(retv_ivar, stmt->Bb(), TRUE, TRUE,TRUE/*in_array*/);
      if (newtree) {
	if (retv->Kind()==CK_IVAR)
	  retv = newtree;
	else {
	  CODEREP *cr = Alloc_stack_cr(0);
	  newtree->IncUsecnt();
	  cr->Init_expr(retv->Op(), newtree);
	  if (retv->Opr() == OPR_CVTL) cr->Set_offset(retv->Offset());
	  retv = Hash_Op(cr);
	}
	  
        if (newtree->Kind() == CK_CONST) {
	  if (retv->Kind() != CK_IVAR) {
	    FOLD ftmp;
	    CODEREP *crtmp = ftmp.Fold_Expr(retv);
	    if (crtmp != NULL) retv = Hash_Const(crtmp);
	  }
	  retv->DecUsecnt();
	  ccr->Set_tree(NULL);
	  ccr->Set_scale(retv->Const_val());
	  return TRUE;
        }
	if (retv->Kind() == CK_OP) {
	  // if CRSIMP flag is turned on, the opr is +,-,
	  // and the kid1 is a constant, pull the constant out.
	  const OPERATOR opr = retv->Opr();
	  if (WOPT_Enable_CRSIMP &&
	      (opr == OPR_ADD || opr == OPR_SUB) &&
	      retv->Get_opnd(1)->Kind() == CK_CONST) {
		CODEREP *cr = retv->Get_opnd(0);
		cr->DecUsecnt();
		ccr->Set_tree(cr);
		cr = retv->Get_opnd(1);
		cr->DecUsecnt();
		if (opr == OPR_SUB)
		  ccr->Set_scale(0 - cr->Const_val());
		else
		  ccr->Set_scale(cr->Const_val());
		return TRUE;
	      }
	} // if (retv->Kind() == CK_OP)
      }
      if ((Split_64_Bit_Int_Ops || Only_Unsigned_64_Bit_Ops) &&
	  MTYPE_is_integral(retv->Dtyp()) &&
	  MTYPE_size_min(retv->Dtyp()) < MTYPE_size_min(WN_rtype(wn))) {
        retv = Add_unary_node(OPCODE_make_op(OPR_CVT, WN_rtype(wn), 
		         Mtype_TransferSize(retv->Dtyp(), WN_rtype(wn))), retv);
      }
      ccr->Set_tree(retv);
      ccr->Set_scale(0);
      return TRUE;
    }
    if ((Split_64_Bit_Int_Ops || Only_Unsigned_64_Bit_Ops) &&
        MTYPE_is_integral(retv->Dtyp()) &&
        MTYPE_size_min(retv->Dtyp()) < MTYPE_size_min(WN_rtype(wn))) {
      retv = Add_unary_node(OPCODE_make_op(OPR_CVT, WN_rtype(wn), 
		       Mtype_TransferSize(retv->Dtyp(), WN_rtype(wn))), retv);
    }
    ccr->Set_tree(retv);
    ccr->Set_scale(0);
    return FALSE;
  }
  else if (oper == OPR_ILOADX) {
    Is_True(FALSE, ("CODEMAP::Add_expr: should never see OPR_ILOADX"));
    CODEREP *base = Add_expr(WN_kid(wn, 0), opt_stab, stmt, &propagated, 
				copyprop);
    CODEREP *index = Add_expr(WN_kid(wn, 1), opt_stab, stmt, &propagated,
				copyprop);
    _ssa->Value_number_mu_node(opt_stab->Get_mem_mu_node(wn));
    retv = Add_idef(op, opt_stab->Get_occ(wn), NULL,
		    opt_stab->Get_mem_mu_node(wn), WN_rtype(wn),
		    WN_desc(wn), WN_ty(wn), WN_field_id(wn),
		    (mINT32)(INTPTR)index, (CODEREP *) WN_load_addr_ty(wn), base, NULL);
    Is_True(retv->Kind()==CK_IVAR||(retv->Kind()==CK_OP
				    && (retv->Opr()==OPR_CVT ||
					retv->Opr()==OPR_CVTL)),
	    ("CODEMAP::Add_expr: return illegal cr from Add_idef"));
    CODEREP *retv_ivar = (retv->Kind()==CK_IVAR) ? retv : retv->Opnd(0);
    if (retv_ivar->Is_ivar_volatile())
      stmt->Set_volatile_stmt();
    ccr->Set_tree(retv);
    ccr->Set_scale(0);
    return FALSE;
  }
  else if (oper == OPR_MLOAD) {
    CANON_CR base_ccr;
    Add_expr(WN_kid(wn, 0), opt_stab, stmt, &base_ccr, copyprop);
    CODEREP *num_byte = Add_expr(WN_kid(wn, 1), opt_stab, stmt,
				 &propagated, copyprop);
    _ssa->Value_number_mu_node(opt_stab->Get_mem_mu_node(wn));
    base_ccr.Set_scale(base_ccr.Scale() + WN_offset(wn));
    base_ccr.Trim_to_16bits(WN_kid(wn, 0), this);

    // In IA-64, MLOAD should have zero offset
    if (! Use_Load_Store_Offset && base_ccr.Scale() != 0) {
      MTYPE typ = WN_rtype(WN_kid(wn, 0));
      CODEREP *retv = Add_const(typ, base_ccr.Scale());
      if (base_ccr.Tree() != NULL)
	retv = Add_bin_node(OPCODE_make_op(OPR_ADD, typ, MTYPE_V),
			    base_ccr.Tree(), retv);
      base_ccr.Set_tree(retv);
      base_ccr.Set_scale(0);
    }

    CODEREP *lbase = base_ccr.Tree() ? base_ccr.Tree() :
    				       Add_const(Pointer_type, (INT64) 0);
#ifndef linux
    FmtAssert(sizeof(num_byte) == sizeof(TY_IDX),
	      ("CODEMAP::Add_expr: Cannot union MLOAD size with "
	       "Ilod_base_ty"));
#endif /* linux */
    retv = Add_idef(op, opt_stab->Get_occ(wn), NULL,
		    opt_stab->Get_mem_mu_node(wn),
		    WN_rtype(wn),
		    WN_desc(wn),
		    WN_ty(wn), WN_field_id(wn), base_ccr.Scale(),
		    // num_byte in the following line is correct. See
		    // opt_htable.h, where the MLOAD/MSTORE size is
		    // unioned with the Ilod_base_ty. The only thing
		    // we need to check here is that we don't lose
		    // information (see the assertion preceding this
		    // Add_idef call).
		    // -- RK 971104
		    num_byte, lbase, NULL);
    Is_True(retv->Kind() == CK_IVAR ||
	    (retv->Kind()==CK_OP &&
	     (retv->Opr()==OPR_CVT || retv->Opr()==OPR_CVTL)),
	    ("CODEMAP::Add_expr: return illegal cr from Add_idef"));
    CODEREP *retv_ivar = (retv->Kind()==CK_IVAR) ? retv : retv->Opnd(0);
    if (retv_ivar->Is_ivar_volatile())
      stmt->Set_volatile_stmt();
    ccr->Set_tree(retv);
    ccr->Set_scale(0);
    return FALSE;
  }
  else if ( oper == OPR_PARM ) {
    CODEREP *kid = Add_expr(WN_kid0(wn), opt_stab, stmt, &propagated, copyprop);
    /* CVTL-RELATED start (correctness) */
#if 1
    // Attempt of fix 370390.  However, this breaks testn32/test_overall/longs.c
    // Because no CVT was inserted between the PARM node and LDID.
    cr->Init_ivar( op, WN_rtype(wn),
                   opt_stab->Get_occ(wn)/*occ*/, 
                   MTYPE_V/*dsctyp*/, WN_ty(wn)/*ldty*/,
                   kid/*lbase*/,
                   NULL/*sbase*/, WN_flag(wn)/*ofst*/, 0/*base_ty*/,
		   0/*field_id*/ );
#else
    cr->Init_ivar( op, kid->Dtyp(),
                   opt_stab->Get_occ(wn)/*occ*/, 
                   MTYPE_V/*dsctyp*/, WN_ty(wn)/*ldty*/,
                   kid/*lbase*/,
                   NULL/*sbase*/, WN_flag(wn)/*ofst*/, NULL/*base_ty*/,
		   0/*field_id*/);
#endif
    /* CVTL-RELATED finish */

    MU_NODE *mnode = opt_stab->Get_mem_mu_node(wn);
    if (mnode) {
      _ssa->Value_number_mu_node(mnode);
      cr->Set_ivar_mu_node(mnode);
    }
    retv = Hash_Ivar(cr);
    ccr->Set_tree(retv);
    ccr->Set_scale(0);
    return propagated; // return propagated instead of FALSE - transparent PARM
  }
  else if ( oper == OPR_CALL  ||
	    oper == OPR_ICALL ||
	    oper == OPR_INTRINSIC_CALL ||
	    oper == OPR_INTRINSIC_OP ||
	    oper == OPR_FORWARD_BARRIER ||
	    oper == OPR_BACKWARD_BARRIER) {
    if ( WOPT_Enable_Combine_Operations ) {
      WN *combined_operation;
      if ( Combine_Operations( wn, &combined_operation ) ) {
        return Add_expr( combined_operation, opt_stab, stmt, ccr,
		         copyprop );
      }
    }

    INT      kcnt = WN_kid_count(wn);
    cr->Init_op(WN_opcode(wn), kcnt);

    // In particular for CALLs and the like, the OPR_CALL itself
    // is entered as the first kid of the stmtrep, followed by
    // kids of OPR_CALL as the operands of the first cr.
    for (INT i = 0; i < kcnt; i++) {
      CODEREP *kid = Add_expr(WN_kid(wn, i), opt_stab, stmt, &propagated, copyprop);
      cr->Set_opnd(i, kid);
    }

    if (oper == OPR_INTRINSIC_CALL ) {
      cr->Set_intrinsic(WN_intrinsic(wn));
    }
    else if ( oper == OPR_INTRINSIC_OP ) {
      cr->Set_intrinsic(WN_intrinsic(wn));
      // pv324295
      if (propagated) { // may have propagated all constants to the INTR_OP
	FOLD ftmp;
	retv = ftmp.Fold_Expr(cr);
	if (retv == NULL) { // either not folded or Fold_Expr has not rehashed
	  retv = Hash_Op(cr);
	}
	if (retv->Kind() == CK_CONST) { // folded to constant
	  ccr->Set_tree(NULL);
	  ccr->Set_scale(retv->Const_val());
	  retv->DecUsecnt();
	  return FALSE;
	}
	if (retv->Kind() == CK_RCONST) {
	  ccr->Set_tree(retv);
	  ccr->Set_scale(0);
	  return FALSE;
	}
	cr = retv;
      }
    }
    retv = Hash_Op(cr);
    ccr->Set_tree(retv);
    ccr->Set_scale(0);
    return FALSE;
  }
  else if (oper == OPR_ASM_STMT) {
    // The statement level has saved aside the information from the
    // constraint and clobber pragmas so we can reconstruct them at
    // emit time. We can ignore that information here.

    INT      kcnt = WN_kid_count(wn) - 2;
    cr->Init_op(WN_opcode(wn), kcnt);

    // In particular for ASMs, like CALLs, the OPR_ASM_STMT itself
    // is entered as the first kid of the stmtrep, followed by the
    // OPR_ASM_INPUT kids of OPR_ASM_STMT as the operands of the first
    // cr.
    for (INT i = 0; i < kcnt; i++) {
      CODEREP *kid = Add_expr(WN_kid(wn, i + 2),
			      opt_stab,
			      stmt,
			      &propagated,
			      copyprop);
      cr->Set_opnd(i, kid);
    }

    retv = Hash_Op(cr);
    ccr->Set_tree(retv);
    ccr->Set_scale(0);
    return FALSE;
  }
  else if (oper == OPR_ASM_INPUT) {
    INT kcnt = WN_kid_count(wn);
    cr->Init_op(WN_opcode(wn), kcnt);
    for (INT i = 0; i < kcnt; ++i) {
      CODEREP *kid = Add_expr(WN_kid(wn, i),
			      opt_stab,
			      stmt,
			      &propagated,
			      copyprop);
      cr->Set_opnd(i, kid);
    }
    cr->Set_asm_opnd_num(WN_asm_opnd_num(wn));
    cr->Set_asm_constraint(WN_st_idx(wn));
    retv = Hash_Op(cr);
    ccr->Set_tree(retv);
    ccr->Set_scale(0);
    return FALSE;
  }
  else {
    if ( WOPT_Enable_Combine_Operations ) {
      WN *combined_operation;
      if ( Combine_Operations( wn, &combined_operation ) ) {
        return Add_expr( combined_operation, opt_stab, stmt, ccr,
		         copyprop );
      }
    }
    
    INT      kcnt = WN_kid_count(wn);
    cr->Init_op(WN_opcode(wn), kcnt);

    if (WOPT_Enable_CRSIMP) {
      // process linear operator with canonicalization
      switch (oper) {
      case OPR_ADD: case OPR_SUB:
	if ((MTYPE_type_class(OPCODE_rtype(op)) & MTYPE_CLASS_INTEGER) != 0)
	  return Canon_add_sub(wn, opt_stab, stmt, ccr, cr, copyprop);
	break;
      case OPR_NEG:
	if ((MTYPE_type_class(OPCODE_rtype(op)) & MTYPE_CLASS_INTEGER) != 0)
	  return Canon_neg(wn, opt_stab, stmt, ccr, cr, copyprop);
	break;
      case OPR_MPY:
	if ((MTYPE_type_class(OPCODE_rtype(op)) & MTYPE_CLASS_INTEGER) != 0)
	  return Canon_mpy(wn, opt_stab, stmt, ccr, cr, copyprop);
	break;
      case OPR_CVT:
	if ((MTYPE_type_class(OPCODE_rtype(op)) & MTYPE_CLASS_INTEGER) != 0)
	  return Canon_cvt(wn, opt_stab, stmt, ccr, cr, copyprop);
	break;
      default:
        break;
      }
    }

    // In particular for CALLs and the like, the OPR_CALL itself
    // is entered as the first kid of the stmtrep, followed by
    // kids of OPR_CALL as the operands of the first cr.

    BOOL prop_disabled = copyprop->Disabled();
    for (INT i = 0; i < kcnt; i++) {
      BOOL proped = FALSE;
      CODEREP *opnd =
        Add_expr(WN_kid(wn,i), opt_stab, stmt, &proped, copyprop);
      cr->Set_opnd(i, opnd);
      propagated |= proped;
    }

    switch (oper) {
      case OPR_ARRAY:
        cr->Set_elm_siz(WN_element_size(wn));
	break;
      case OPR_CVTL:
	cr->Set_offset((mINT32)WN_cvtl_bits(wn));
	break;
      case OPR_INTRINSIC_CALL:
     	cr->Set_intrinsic(WN_intrinsic(wn));
	break;
      case OPR_INTRINSIC_OP:
	cr->Set_intrinsic(WN_intrinsic(wn));
	break;
      case OPR_TAS:
	cr->Set_ty_index(WN_ty(wn));
	break;
      case OPR_EXTRACT_BITS:
      case OPR_COMPOSE_BITS:
	cr->Set_op_bit_offset(WN_bit_offset(wn));
	cr->Set_op_bit_size(WN_bit_size(wn));
	break;
    }

    BOOL do_canonicalization = TRUE;
    if (WOPT_Enable_Canon_Compare && _phase == MAINOPT_PHASE) {
      BOOL modified;
      do_canonicalization = !Canonicalize_compare(cr, stmt->Bb(), &modified);
    }

    // Fix 325467:  always try to simplify SELECT because some
    //  SELECT are generated in the opt_cfg.cxx pass with WHIRL simplification.
    if ( (propagated && do_canonicalization && kcnt < 3) ||
	 oper==OPR_SELECT)  
    {
      FOLD ftmp;
      retv = ftmp.Fold_Expr(cr);
      if (retv == NULL) { // either not folded or Fold_Expr has not rehashed
        retv = Hash_Op(cr, do_canonicalization);
      }
    }
    else {
      retv = Hash_Op(cr, do_canonicalization);
    }

    ccr->Set_tree(retv);
    ccr->Set_scale(0);
    return propagated;
  }
}

BOOL
STMTREP::Has_mu(void) const
{
  if (_opr == OPR_REGION) { // need to check what kind of region
    if (Bb()->Kind() == BB_REGIONSTART || Bb()->Kind() == BB_REGIONEXIT) {
      RID *rid = Bb()->Regioninfo()->Rid();
      Is_True(rid != NULL, ("STMTREP::Has_mu, NULL RID"));
      if (RID_TYPE_mp(rid) || RID_TYPE_eh(rid))
	return FALSE;
    }
  }
  return OPERATOR_has_mu(_opr); 
}

BOOL
STMTREP::Has_chi(void) const
{
  if (_opr == OPR_REGION) { // need to check what kind of region
    if (Bb()->Kind() == BB_REGIONSTART || Bb()->Kind() == BB_REGIONEXIT) {
      RID *rid = Bb()->Regioninfo()->Rid();
      Is_True(rid != NULL, ("STMTREP::Has_chi, NULL RID"));
      if (RID_TYPE_mp(rid) || RID_TYPE_eh(rid))
	return FALSE;
    }
  }
  return OPERATOR_has_chi(_opr); 
}

void
STMTREP::Enter_rhs(CODEMAP *htable, OPT_STAB *opt_stab, COPYPROP *copyprop, EXC *exc)
{
  // Create the internal representation for the rhs of this statement.
  OPCODE op = WN_opcode(Wn());
  OPERATOR opr = WN_operator(Wn());
  BOOL   proped = FALSE;

  switch (opr) {

  case OPR_FUNC_ENTRY:
  case OPR_ALTENTRY:
    FmtAssert( FALSE, ("ENTRY not expected") );
    break;

  case OPR_BLOCK:
  case OPR_DO_LOOP:
  case OPR_WHILE_DO:
  case OPR_DO_WHILE:
  case OPR_IF:
  case OPR_RETURN:
    // skip these cases
    break;

  case OPR_GOTO:
  case OPR_REGION_EXIT:
    Set_label_number(WN_label_number(Wn()));
    break;

  case OPR_LABEL:
    // WN_st is not touched by opt_sym at all
    Set_label_number(WN_label_number(Wn()));
    Set_label_flags(WN_label_flag(Wn()));
    // note that we do *not* do anything about loop_info nodes that
    // may be attached because we dealt with them when building the
    // cfg, and they are no longer associated with the label.
    break;

  case OPR_FALSEBR:
  case OPR_TRUEBR:
    // WN_st is not converted to ver_stab index!
    Set_rhs(htable->Add_expr(WN_kid0(Wn()),
			     opt_stab, this, &proped, copyprop));
#if 0 // not needed because same call done in Add_expr, and if calling
      // Fold_Expr again, will undo the canonicalization of compare (492340)
    // simplifies 1 < 10 type expressions
    if (WOPT_Enable_Input_Prop && proped) {
      FOLD ftmp;
      CODEREP *retv;
      if (WOPT_Enable_Fast_Simp)
	retv = ftmp.Fold_Expr(Rhs()); // look at top stmt
      else
	retv = ftmp.Fold_Tree(Rhs()); // look at whole RHS
      if (retv != NULL)
	Set_rhs(retv);
    }
#endif
    Set_label_number(WN_label_number(Wn()));
    break;

  case OPR_XPRAGMA:
    // pragma is IS_AFFINITY_XPRAGMA(wn)
    if (WN_pragma(Wn()) == WN_PRAGMA_COPYIN_BOUND) {
      copyprop->Set_disabled();
      CODEREP *cr = htable->Add_expr(WN_kid0(Wn()),
                                     opt_stab, this,  &proped, copyprop);
      Set_rhs(cr);
#if 0
      STMTREP *defstmt = cr->Defstmt();
      defstmt->Set_volatile_stmt();
#endif
      copyprop->Reset_disabled();
    }
    else
      Set_rhs(htable->Add_expr(WN_kid0(Wn()),
                               opt_stab, this, &proped, copyprop));
    break;

  case OPR_AGOTO:
  case OPR_ASSERT:
  case OPR_EVAL:
  case OPR_COMPGOTO:
  case OPR_RETURN_VAL:
    Set_rhs(htable->Add_expr(WN_kid0(Wn()),
                             opt_stab, this, &proped, copyprop));
    break;

  case OPR_CALL:
  case OPR_ICALL:
  case OPR_INTRINSIC_CALL:
  case OPR_FORWARD_BARRIER:
  case OPR_BACKWARD_BARRIER:
  case OPR_DEALLOCA:
  case OPR_ALLOCA:
    // WN_st, if present, is not converted to ver_stab index!
    Set_rhs(htable->Add_expr(Wn(), opt_stab, this, &proped, copyprop));
    break;

  case OPR_COMMENT:
  case OPR_PRAGMA:
    // don't do anything
    break;
  
  case OPR_IO:
  case OPR_REGION:
    {
      // any region nodes not taken out during construction of CFG are black
      // boxes.
      Set_black_box();
      WN *new_wn = WN_COPY_Tree_With_Map(Wn()); // copy the tree in case we delete WHIRL
      Set_black_box_wn( new_wn );
    }
    break;

  case OPR_STID:
  case OPR_STBITS:
  case OPR_ISTORE:
  case OPR_ISTBITS:
  case OPR_ISTOREX:
  case OPR_MSTORE:
    Set_rhs(htable->Add_expr(WN_kid0(Wn()), opt_stab, this, &proped, copyprop));
    if (WOPT_Enable_Input_Prop && proped) {
      FOLD ftmp;
      CODEREP *retv;
      if (WOPT_Enable_Fast_Simp)
	retv = ftmp.Fold_Expr(Rhs()); // look at top stmt
      else
	retv = ftmp.Fold_Tree(Rhs()); // look at whole RHS
      if (retv != NULL)
	Set_rhs(retv);
    }
    return;

  case OPR_PREFETCH:
    {
      CANON_CR base_ccr;
      htable->Add_expr(WN_kid0(Wn()), opt_stab, this, &base_ccr, copyprop);
      base_ccr.Set_scale(base_ccr.Scale() + WN_offset(Wn()));
      base_ccr.Trim_to_16bits(WN_kid0(Wn()), htable);

      // In IA-64, PREFETCH should have zero offset
      if (! Use_Load_Store_Offset && base_ccr.Scale() != 0) {
	MTYPE typ = WN_rtype(WN_kid(Wn(), 0));
	CODEREP *retv = htable->Add_const(typ, base_ccr.Scale());
	if (base_ccr.Tree() != NULL)
	  retv = htable->Add_bin_node
	    (OPCODE_make_op(OPR_ADD, typ, MTYPE_V), base_ccr.Tree(), retv);
	base_ccr.Set_tree(retv);
	base_ccr.Set_scale(0);
      }

      CODEREP *lbase = base_ccr.Tree() ? base_ccr.Tree() :
      		htable->Add_const(Pointer_type, (INT64) 0);
      Set_rhs(htable->Add_idef(op,
                               NULL,  // no occ_tab entry
                               NULL,  // no defining stmt
                               NULL,  // no mu-list
                               MTYPE_V, // no dtyp
                               MTYPE_V, // no desc
                               0,    // no TY
			       0,    // no field id
                               base_ccr.Scale(),
                               (CODEREP *) 0,    // no load_addr_ty
                               lbase,
                               NULL));
      WN_MAP_Set(htable->Prefetch_map(), Wn(), this);
      return;
    }

  case OPR_OPT_CHI:
    break;

  case OPR_ASM_STMT:
    {
      // Save away the information from the constraint and clobber
      // pragmas so we can reconstruct those pragmas at emit time.
      Set_asm_pragma(CXX_NEW(ASM_PRAGMA_INFO(htable->Mem_pool()),
			     htable->Mem_pool()));
      WN *prag = WN_first(WN_asm_clobbers(Wn()));
      while (prag != NULL) {
	FmtAssert(WN_pragma(prag) == WN_PRAGMA_ASM_CLOBBER,
		  ("Unknown pragma type for ASM clobber"));
	CLOBBER_PRAGMA_INFO info;
	if (WN_opcode(prag) == OPC_XPRAGMA) {
	  WN *kid = WN_kid0(prag);
	  FmtAssert(WN_operator(kid) == OPR_IDNAME,
		    ("Unknown kid operator for PREG ASM clobber"));
	  info.preg_st_idx = WN_st_idx(kid);
	  info.preg_number = WN_offset(kid);
	  info.clobber_string_idx = WN_pragma_arg2(prag);
	}
	else {
	  info.preg_st_idx = 0;
	  info.preg_number = 0;
	  info.clobber_string_idx = WN_pragma_arg1(prag);
	}
	Asm_pragma()->clobber_info.push_back(info);
	prag = WN_next(prag);
      }
      prag = WN_first(WN_asm_constraints(Wn()));
      while (prag != NULL) {
	FmtAssert(WN_pragma(prag) == WN_PRAGMA_ASM_CONSTRAINT,
		  ("Unknown pragma type for ASM output constraint"));
	CONSTRAINT_PRAGMA_INFO info;
	info.preg_st_idx = WN_st_idx(prag);
	info.constraint_st_idx = (ST_IDX) WN_pragma_arg1(prag);
	info.asm_neg_preg = WN_pragma_asm_copyout_preg(prag);
	info.asm_opnd_num = WN_pragma_asm_opnd_num(prag);
	Asm_pragma()->constraint_info.push_back(info);
	prag = WN_next(prag);
      }
      Set_rhs(htable->Add_expr(Wn(), opt_stab, this, &proped, copyprop));
      break;
    }

  case OPR_PREFETCHX:
  default:
    FmtAssert (FALSE,
	       ("STMTREP::Enter_rhs: opcode %s is not implemented yet",
		OPCODE_name(op)));
  }
}

// return TRUE if two STMTREPs have same LHS
BOOL
STMTREP::Same_lhs(const STMTREP *stmt) const
{
  if (this == NULL || stmt == NULL)
    return FALSE;
  if (Lhs() == NULL || stmt->Lhs() == NULL)
    return FALSE;
  if (Lhs()->Kind() != stmt->Lhs()->Kind())
    return FALSE;
  switch (Lhs()->Kind()) {
    case CK_VAR:
      return Lhs()->Aux_id() == stmt->Lhs()->Aux_id();
    case CK_IVAR:
      if (MTYPE_size_min(Lhs()->Dtyp())==MTYPE_size_min(stmt->Lhs()->Dtyp()) &&
	  (Lhs()->Istr_base() == stmt->Lhs()->Istr_base()) &&
	  (Lhs()->Offset() == stmt->Lhs()->Offset())) {
	if (Opr() == OPR_MLOAD) 
	  return Lhs()->Mload_size() == stmt->Lhs()->Mload_size();
	else
	  return TRUE;
      }
      return FALSE;
    default:
      return FALSE;
  }
}
  
void
STMTREP::Enter_lhs(CODEMAP *htable, OPT_STAB *opt_stab, COPYPROP *copyprop)
{
  IDTYPE du;
  OPCODE opc;
  OPERATOR opr = WN_operator(Wn());
  MTYPE  dtyp, dsctyp;
  WN    *copy_wn;
  CANON_CR base_ccr;
  TY_IDX ilod_base_ty;
  BOOL   proped = FALSE;

  Set_linenum(WN_Get_Linenum(Wn()));

  // No need to do anything for "black-box" statements
  if ( WN_is_black_box( Wn() ) )
    return;

  // Create the internal representation for the lhs of this statement.
  switch (opr) {
  case OPR_STID:
  case OPR_STBITS:
    du = WN_ver(Wn());
    FmtAssert(du != 0,
	      ("STMTMAP::Enter_lhs: WN_st wasn't set correctly"));
    dsctyp = _desc;
    // under U64 ISA, STID can still be signed; the following makes sure
    // first-touch is always unsigned
    if (htable->Phase() == MAINOPT_PHASE && 
	(Only_Unsigned_64_Bit_Ops && ! Delay_U64_Lowering)) 
      dsctyp = Mtype_TransferSign(MTYPE_U8, dsctyp);
    Set_lhs( htable->Add_def(opt_stab->Du_aux_id(du), opt_stab->Du_version(du),
			     this, WN_rtype(WN_kid0(Wn())), dsctyp,
			     WN_offset(Wn()), WN_ty(Wn()),
			     WN_field_id(Wn()), TRUE ) ); 

    if (opt_stab->Du_is_volatile(du)) { 
      // set the volatile attribute for this var, even if this
      // particular reference is not cast as volatile
      Lhs()->Set_var_volatile();
    }
    else {
      // only cache non-volatile codereps
      opt_stab->Du_set_coderep(du, Lhs());
    }

    opt_stab->Push_coderep(Lhs()->Aux_id(), Lhs());

    if (Rhs()->Kind() == CK_VAR &&
	Rhs()->Aux_id() == Lhs()->Aux_id() &&
	!Rhs()->Is_var_volatile() &&
	!Lhs()->Is_var_volatile())
      Set_identity_asgn();

    return;

  case OPR_ISTORE:
  case OPR_ISTBITS:
    {
      // ISTORE has no dtyp, we manufacture one from the rhs of its tree
      htable->Add_expr(WN_kid(Wn(), 1), opt_stab, this, &base_ccr, copyprop);
      CODEREP *baseexp = base_ccr.Tree();

      dtyp = WN_rtype(WN_kid0(Wn()));
      dsctyp = _desc;
      if (dsctyp != MTYPE_BS) {
        if (MTYPE_size_min(dsctyp) > MTYPE_size_min(dtyp)) {
	  // ISTORE does not like dtyp < dsctyp, make sure 
	  // they are of compatible size
	  dtyp = Mtype_from_class_size(dtyp, dsctyp);
        }
        dtyp = Mtype_from_class_size(dsctyp, dtyp);
      }
      opc = OPCODE_make_op(opr == OPR_ISTORE ? OPR_ILOAD : OPR_ILDBITS, dtyp, dsctyp);
      base_ccr.Set_scale(base_ccr.Scale() + WN_offset(Wn()));
      base_ccr.Trim_to_16bits(WN_kid(Wn(), 1), htable);
      if (base_ccr.Tree() != NULL && 
	  base_ccr.Tree()->Kind() == CK_LDA )
      {
	// is the result of the load volatile?
	if (TY_is_volatile(WN_ty(Wn()))) 
	  goto no_fold_istore;

	// may point to a volatile object
	if (TY_kind(WN_ty(Wn())) == KIND_POINTER) {
	  TY_IDX pointed = TY_pointed(WN_ty(Wn()));
	  if (TY_is_volatile(pointed))
	    goto no_fold_istore;
	}
      
	// fold to STID
	// go thru chi list to find the scalar var version
	CHI_NODE *cnode, *prev_cnode;
	CHI_LIST *chi_list = opt_stab->Get_generic_chi_list(Wn());
	CHI_LIST_ITER chi_iter;

	chi_iter.Init(chi_list);
	for (prev_cnode = NULL, cnode = chi_iter.First(); 
	     ! chi_iter.Is_Empty(); 
	     prev_cnode = cnode, cnode = chi_iter.Next()) {
	  if (! cnode->Live())
	    continue;
	  AUX_ID vaux = cnode->Aux_id();
	  if (!opt_stab->Is_real_var(vaux))
	    continue;
	  INT64 v_ofst;
	  ST *v_st;
	  Expand_ST_into_base_and_ofst(opt_stab->St(vaux), 
				       opt_stab->St_ofst(vaux), &v_st, &v_ofst);
	  if (v_st == base_ccr.Tree()->Lda_base_st() &&
	      v_ofst == base_ccr.Scale() && 
	      0 == base_ccr.Tree()->Offset() && // TODO: not needed if canonicalize in preopt
	      opt_stab->Aux_stab_entry(vaux)->Mclass() == 
	            Get_mtype_class(_desc) &&
	      opt_stab->Aux_stab_entry(vaux)->Byte_size() * 8 ==
	            MTYPE_size_min(_desc) &&
	      (opr == OPR_ISTORE && 
	       (opt_stab->Aux_stab_entry(vaux)->Bit_size() == 0 ||
		opt_stab->Aux_stab_entry(vaux)->Field_id() == WN_field_id(Wn())) ||
	       opr == OPR_ISTBITS &&
	       opt_stab->Aux_stab_entry(vaux)->Bit_ofst() == WN_bit_offset(Wn()) &&
	       opt_stab->Aux_stab_entry(vaux)->Bit_size() == WN_bit_size(Wn()))) {
	    VER_STAB_ENTRY *vse = opt_stab->Ver_stab_entry(cnode->Result());
	    base_ccr.Tree()->DecUsecnt();		// for the LDA coderep
	    Is_True(! vse->Real_use() ||
		    vse->Coderep() == NULL && vse->Type() == CHI_STMT, 
		    ("Error in ISTORE to STID folding in htable"));

	    Set_lhs( htable->Add_def(vaux, vse->Version(), this,
				     dtyp, dsctyp,
				     opt_stab->St_ofst(vaux)/*base_ccr.Scale()*/,
				     TY_pointed(WN_ty(Wn())),
				     WN_field_id(Wn()), TRUE) );
	    Lhs()->Set_flag(CF_FOLDED_LDID);

	    if (opt_stab->Du_is_volatile(cnode->Result())) { 
	      // set the volatile attribute for this var, even if this
	      // particular reference is not cast as volatile
	      Lhs()->Set_var_volatile();
	    }
	    else {
	      // only cache non-volatile codereps
	       opt_stab->Du_set_coderep(cnode->Result(), Lhs());
	    }

	    // if not zero version originally, set coderep that correspond to vers
	    if (! vse->Zero_vers()) {
	      vse->Set_Real_use();
	      vse->Set_type(WHIRL_STMT);
	    }
	    chi_list->Remove(prev_cnode, cnode); // remove cnode from chi list
	    // cnode must not be purged from memory because it will be
	    // used by ILOAD-folding (refererened thru VSE and DU).
	    cnode->Set_RESULT(Lhs()); 

	    // TODO: need to remove extra chi nodes
	    Set_op(OPCODE_make_op(opr == OPR_ISTORE ? OPR_STID : OPR_STBITS, MTYPE_V, dsctyp));
	    opt_stab->Push_coderep(vaux, Lhs());
	    htable->Inc_istorefolds();
	    return;
	  }
	}
      }

      // jump to here if not folding the istore
      no_fold_istore:;

      // In IA-64, ISTORE should have zero offset
      if (! Use_Load_Store_Offset && base_ccr.Scale() != 0) {
	MTYPE typ = WN_rtype(WN_kid(Wn(), 1));
	CODEREP *retv = htable->Add_const(typ, (INT64) base_ccr.Scale());
	if (base_ccr.Tree() != NULL)
	  retv = htable->Add_bin_node
	    (OPCODE_make_op(OPR_ADD, typ, MTYPE_V), base_ccr.Tree(), retv);
	base_ccr.Set_tree(retv);
	base_ccr.Set_scale(0);
      }

      ilod_base_ty = WN_ty(Wn());
      {
	CODEREP *lbase = ( base_ccr.Tree() ? base_ccr.Tree() :
			   htable->Add_const(Pointer_type, (INT64) 0) );
	Set_lhs( htable->Add_idef(opc, opt_stab->Get_occ(Wn()), 
				  this, NULL, dtyp, dsctyp, 
				  TY_pointed(ilod_base_ty),
				  WN_field_id(Wn()),
				  base_ccr.Scale(),
				  (CODEREP *) ilod_base_ty, NULL, lbase) );
      }
      if (Lhs()->Is_ivar_volatile())
	Set_volatile_stmt();
      htable->Update_pref(Lhs());
    }
    return;

  case OPR_ISTOREX:
    {
      Is_True(FALSE, ("STMTREP::Enter_lhs: should never see OPR_ISTOREX"));
      CODEREP *base = 
	htable->Add_expr(WN_kid(Wn(), 1), opt_stab, this, &proped, copyprop);
      CODEREP *index = htable->Add_expr(WN_kid(Wn(), 2), opt_stab, this,
	&proped, copyprop);
      
      dtyp = WN_rtype(WN_kid0(Wn()));
      dsctyp = _desc;
      //opc = OPCODE_make_op(OPR_ILOADX, dtyp, dsctyp);
      opc = OPCODE_make_op(OPR_ILOADX, dtyp, MTYPE_V);
      ilod_base_ty = WN_ty(Wn());
      CODEREP *lbase = base_ccr.Tree() ? base_ccr.Tree() :
      	htable->Add_const(Pointer_type,(INT64)0);
      Set_lhs( htable->Add_idef(opc, opt_stab->Get_occ(Wn()), 
				this, NULL,
				WN_rtype(WN_kid0(Wn())),
				_desc,
				TY_pointed(ilod_base_ty),
				WN_field_id(Wn()),
				(mINT32)(INTPTR)index,
				(CODEREP *) ilod_base_ty, NULL, lbase) );
      if (Lhs()->Is_ivar_volatile())
	Set_volatile_stmt();
    }
    return;

  case OPR_MSTORE:
    {
      htable->Add_expr(WN_kid(Wn(), 1), opt_stab, this, &base_ccr, copyprop);
      CODEREP *num_byte = htable->Add_expr(WN_kid(Wn(), 2), opt_stab, this,
					   &proped, copyprop);
      // rtype and type of MLOAD is indepedent of its operands type
      opc = OPC_MLOAD;
      base_ccr.Set_scale(base_ccr.Scale() + WN_offset(Wn()));
      base_ccr.Trim_to_16bits(WN_kid(Wn(), 1), htable);

      // In IA-64, MLOAD should have zero offset
      if (! Use_Load_Store_Offset && base_ccr.Scale() != 0) {
	MTYPE typ = WN_rtype(WN_kid(Wn(), 1));
	CODEREP *retv = htable->Add_const(typ, (INT64) base_ccr.Scale());
	if (base_ccr.Tree() != NULL)
	  retv = htable->Add_bin_node
	    (OPCODE_make_op(OPR_ADD, typ, MTYPE_V), base_ccr.Tree(), retv);
	base_ccr.Set_tree(retv);
	base_ccr.Set_scale(0);
      }

      CODEREP *lbase = ( base_ccr.Tree() ? base_ccr.Tree() :
			 htable->Add_const(Pointer_type, (INT64) 0) );
#ifndef linux
      FmtAssert(sizeof(num_byte) == sizeof(TY_IDX),
		("CODEMAP::Add_expr: Cannot union MSTORE size with "
		 "Ilod_base_ty"));
#endif /* linux */
      Set_lhs(htable->Add_idef(opc, opt_stab->Get_occ(Wn()), 
			       this, NULL,
			       MTYPE_M,
			       MTYPE_M,
			       WN_ty(Wn()),
			       WN_field_id(Wn()),
			       base_ccr.Scale(),
			       // num_byte in the following line is
			       // correct. See opt_htable.h, where the
			       // MLOAD/MSTORE size is unioned with the
			       // Ilod_base_ty. The only thing we need
			       // to check here is that we don't lose
			       // information (see the assertion
			       // preceding this Set_lhs call).
			       // -- RK 971104
			       num_byte, NULL, lbase));
      if (Lhs()->Is_ivar_volatile())
	Set_volatile_stmt();
    }
    return;

  case OPR_COMPGOTO:
  case OPR_EVAL:
    // don't have to do anything with these (all handled by rhs)
    return;

  case OPR_BLOCK:
  case OPR_DO_LOOP:
  case OPR_WHILE_DO:
  case OPR_DO_WHILE:
  case OPR_IF:
    // should not ever get here
    FmtAssert(FALSE, ("CODEMAP::Enter_lhs: Don't know how to handle"));
    return;

  case OPR_FALSEBR:
  case OPR_TRUEBR:
    Set_label_number(WN_label_number(Wn()));
    return;

  case OPR_FORWARD_BARRIER:
  case OPR_BACKWARD_BARRIER:
  case OPR_DEALLOCA:
    Set_pt_list(opt_stab->Get_occ(Wn())->Pt_list());
    return;

  case OPR_INTRINSIC_CALL:
    Set_call_flags(WN_call_flag(Wn()));
    return;

  case OPR_ICALL:
    Set_call_flags(WN_call_flag(Wn()));
    Set_ty(WN_ty(Wn()));
    return;

  case OPR_CALL:
    Set_call_flags(WN_call_flag(Wn()));
    if (OPCODE_has_aux(WN_opcode(Wn())))
      Set_st(opt_stab->St((IDTYPE)WN_st(Wn())));
    else
      Set_st(WN_st(Wn()));
    return;

  case OPR_ASM_STMT:
    Set_asm_string_idx(WN_st_idx(Wn()));
    _u4._asm_stmt_flags = _u3._wn->u1u2.uu.ua.asm_flag;
//  Set_asm_stmt_flags(WN_asm_flag(Wn()));
    return;

  case OPR_ASSERT:
  case OPR_RETURN:
  case OPR_RETURN_VAL:
  case OPR_PRAGMA:
  case OPR_XPRAGMA:
    copy_wn = WN_CopyNode(Wn());
    WN_map_id(copy_wn) = WN_map_id(Wn());
    Set_orig_wn(copy_wn);
    return;

  case OPR_REGION:
    copy_wn = WN_COPY_Tree_With_Map(Wn());
    Set_orig_wn(copy_wn);
    return;

  case OPR_PREFETCH:
    // pre-allocate the Prefetch wn
    copy_wn = WN_CopyNode(Wn());
    WN_map_id(copy_wn) = WN_map_id(Wn());
    Set_prefetch_wn(copy_wn);
    // update the pf_pointer to point to the STMTREP
    // opt_stab->Update_pf_list(Wn(), this);
    return;

  case OPR_GOTO:
  case OPR_AGOTO:
  case OPR_LABEL:
  case OPR_REGION_EXIT:
  case OPR_OPT_CHI:
  case OPR_EXC_SCOPE_BEGIN:
  case OPR_EXC_SCOPE_END:
    return;

  case OPR_PREFETCHX:
  default:
    Warn_todo("STMTREP::Enter_lhs: %s is not implemented yet",
	      OPCODE_name(WN_opcode(Wn())));
    return;
  }
}


void
CODEMAP::Init_var_phi_hash(void)
{
  INT32 hash_tbl_capacity = 10; // Cfg()->Last_bb_id() * Opt_stab()->
  _phi_id_map.Init(hash_tbl_capacity);
  Set_phi_hash_valid();

  CFG_ITER cfg_iter;
  BB_NODE *bb;
  // It is cheaper to loop through all the phi's twice than to enlarge
  // the table a bunch of times! If we are going to do SSAPRE (which
  // creates a bunch of new variables to hold expression temporaries),
  // we will ask for an initial table capacity of 3/2 the number of
  // phi's currently in the program. Otherwise we ask for a table just
  // big enough to hold the current number of phi's in the program.
  if (WOPT_Enable_Exp_PRE ||
      WOPT_Enable_Load_PRE ||
      WOPT_Enable_Store_PRE) {
    FOR_ALL_NODE( bb, cfg_iter, Init(Cfg()) ) {
      PHI_NODE *phi;
      PHI_LIST_ITER phi_iter;
      FOR_ALL_ELEM (phi, phi_iter, Init(bb->Phi_list())) {
	hash_tbl_capacity += 3;
      }
    }
    hash_tbl_capacity = hash_tbl_capacity / 2;
  }
  else {
    FOR_ALL_NODE( bb, cfg_iter, Init(Cfg()) ) {
      PHI_NODE *phi;
      PHI_LIST_ITER phi_iter;
      FOR_ALL_ELEM (phi, phi_iter, Init(bb->Phi_list())) {
	++hash_tbl_capacity;
      }
    }
  }
  _phi_id_map.Init(hash_tbl_capacity);
  Set_phi_hash_valid();
  FOR_ALL_NODE( bb, cfg_iter, Init(Cfg()) ) {
    PHI_NODE *phi;
    PHI_LIST_ITER phi_iter;
    FOR_ALL_ELEM (phi, phi_iter, Init(bb->Phi_list())) {
      Enter_var_phi_hash(phi);
    }
  }
}


void
CODEMAP::Verify_var_phi_hash(void)
{
#ifdef Is_True_On
  CFG_ITER cfg_iter(Cfg());
  BB_NODE *bb;
  FOR_ALL_NODE( bb, cfg_iter, Init() ) {
    PHI_NODE *phi;
    PHI_LIST_ITER phi_iter;
    FOR_ALL_ELEM (phi, phi_iter, Init(bb->Phi_list())) {
      FmtAssert(phi == Lookup_var_phi(phi->Bb(), phi->Aux_id()),
		("CODEMAP::Verify_var_phi_hash: phi %d at BB%d not found.", 
		 phi->Aux_id(), phi->Bb()->Id()));
	  
    }
  }
#endif
}


void
CODEMAP::Enter_var_phi_hash(PHI_NODE *phi)
{
  Is_True(Phi_hash_valid(),
	  ("CODEMAP::Enter_var_phi_hash:  hash not valid."));
  Is_True(_phi_id_map.Lookup(phi->Key()) == NULL,
	  ("CODEMAP::Enter_var_phi_hash:  already entered."));
  _phi_id_map.Insert(phi->Key(), phi);
}

PHI_NODE*
CODEMAP::Lookup_var_phi(const BB_NODE *bb, const AUX_ID id) const
{
  Is_True(Phi_hash_valid(), ("CODEMAP::Lookup_var_phi:  hash not valid."));
  PHI_KEY key;  key.Init(bb->Id(),id);
  return _phi_id_map.Lookup(key);
}

void
CODEMAP::Remove_var_phi_hash(PHI_NODE *phi)
{
  Is_True(Phi_hash_valid(), ("CODEMAP::Lookup_var_phi:  hash not valid."));
  _phi_id_map.Delete(phi->Key());
}

void
STMT_LIST::Print(FILE *fp) const
{
  STMTREP_CONST_ITER  stmt_iter(this);
  const    STMTREP   *tmp;
  INT32               last_line = 0;
  FOR_ALL_NODE(tmp, stmt_iter, Init()) {
    if (Srcpos_To_Line(tmp->Linenum()) != last_line &&
	Srcpos_To_Line(tmp->Linenum()) != 0) {
      last_line = Srcpos_To_Line(tmp->Linenum());
      fprintf(fp, ">LINE %d___\n", last_line);
      }
    tmp->Print(fp);
    }
}

// Get the pointer information from opt_stab
//
POINTS_TO *
CODEREP::Points_to(OPT_STAB *opt_stab) const  
{ 
  if (Kind() == CK_VAR) {
    return opt_stab->Points_to(Aux_id());
  }
  else if (Kind() == CK_LDA) {
    return opt_stab->Points_to(Lda_aux_id());
  }
  else if (Kind() == CK_IVAR) {
    // return opt_stab->Points_to(Ivar_aux_id());
    OCC_TAB_ENTRY *occ = Ivar_occ();
    if (occ)
      return Ivar_occ()->Points_to();
    return NULL;
  }
  else
    return NULL;
}

CODEREP *
CODEREP::Get_ivar_vsym(void) const
{
  if (Ivar_mu_node() != NULL) {
    return Ivar_mu_node()->OPND();
  } else {
    // PARM node has no chi list to search
    if (! OPERATOR_is_scalar_iload (Opr()) && Opr() != OPR_MLOAD)
      return NULL;

    // search the chi list
    Is_True(OPERATOR_is_scalar_istore (Ivar_defstmt()->Opr()) ||
	    Ivar_defstmt()->Opr() == OPR_MSTORE, ("IVAR not defined by ISTORE/MSTORE."));
    CHI_LIST_ITER chi_iter;
    CHI_NODE *cnode;
    CHI_LIST *chi_list = Ivar_defstmt()->Chi_list();
    FOR_ALL_NODE( cnode, chi_iter, Init(chi_list)) {
      if (cnode->Aux_id() == Ivar_occ()->Aux_id()) {
        // Is_True(cnode->Live(), ("CODEREP::Get_iver_vsym: cnode not live"));
	// Fix 510377!  vsym could be dead!  
	if (cnode->Live())
	  return cnode->RESULT();
	else
	  return NULL;
      }
    }
  }
  return NULL;
}

// Get the pointer information from opt_stab
//
POINTS_TO *
STMTREP::Points_to(OPT_STAB *opt_stab) const  
{ 
  const OPERATOR opr = Opr();

  if (OPERATOR_is_scalar_load (opr) ||
      OPERATOR_is_scalar_store (opr) ||
      OPERATOR_is_scalar_iload (opr) ||
      OPERATOR_is_scalar_istore (opr) ||
      opr == OPR_ILOADX  || opr == OPR_ISTOREX)
    return Lhs()->Points_to(opt_stab);
  else if ( opr == OPR_RETURN || opr == OPR_RETURN_VAL)
    return opt_stab->Points_to_globals();
  else
    return NULL;
}


// ====================================================================
// Determine if the statement redefines the variable (either directly,
// or through a chi)
// ====================================================================

BOOL
STMTREP::Redefines_var( AUX_ID var )
{
  if (OPERATOR_is_scalar_store (Opr())) {
    CODEREP *lhs = Lhs();
    if ( Lhs()->Aux_id() == var ) {
      return TRUE;
    }
  }

  // check for indirect definitions
  if ( Has_chi() ) {
    CHI_LIST_ITER chi_iter;
    CHI_NODE *cnode;
    CHI_LIST *chi_list = Chi_list();
    FOR_ALL_NODE( cnode, chi_iter, Init(chi_list)) {
      if ( cnode->Aux_id() == var ) {
	return TRUE;
      }
    }
  }

  // must not redefine the variable
  return FALSE;
}

// ====================================================================
// Determine if 'this' references the variable (either directly, or 
// through a mu)
// ====================================================================

BOOL
CODEREP::References_var( AUX_ID var )
{
  switch ( Kind() ) {
    case CK_VAR:
      if ( Aux_id() == var )
	return TRUE;
      else
	return FALSE;

    case CK_IVAR:
      if ( Ilod_base()->References_var(var) )
	return TRUE;
      if (Opr() == OPR_MLOAD) {
	if ( Mload_size()->References_var(var) )
	  return TRUE;
      }
      if (Ivar_mu_node() != NULL && Ivar_mu_node()->Aux_id() == var)
	return TRUE;
      return FALSE;

    case CK_OP:
      {
	for ( INT i = 0; i < Kid_count(); i++)
	  if ( Get_opnd(i)->References_var(var) ) 
	    return TRUE;
      }
      return FALSE;

    case CK_LDA:
    case CK_CONST:
    case CK_RCONST:
      return FALSE;

    default:
      FmtAssert( FALSE, ("CODEREP::References_var: invalid kind") );
  }

  // must not reference the variable
  return FALSE;	// probably should not get to this statement
}

// ====================================================================
// Determine if 'this' references the variable (either directly, or 
// through a mu/chi)
// ====================================================================

BOOL
STMTREP::References_var( AUX_ID var )
{
  // check rhs
  if ( Rhs() != NULL ) {
    if ( Rhs()->References_var( var ) )
      return TRUE;
  }
    
  // check lhs
  if ( Lhs() != NULL ) {
    if ( Lhs()->References_var( var ) )
      return TRUE;
  }
    
  // check for indirect references through chi's
  if ( Has_chi() ) {
    CHI_LIST_ITER chi_iter;
    CHI_NODE *cnode;
    CHI_LIST *chi_list = Chi_list();
    FOR_ALL_NODE( cnode, chi_iter, Init(chi_list)) {
      if ( cnode->Aux_id() == var ) {
	return TRUE;
      }
    }
  }

  // check for indirect references through mu's
  if ( Has_mu() ) {
    MU_LIST_ITER mu_iter;
    MU_NODE *mnode;
    FOR_ALL_NODE( mnode, mu_iter, Init(Mu_list()) ) {
      if ( mnode->Aux_id() == var )
	return TRUE;
    }
  }

  // must not reference the variable
  return FALSE;
}

// ====================================================================
// Print a bit position (formatted nicely)
// ====================================================================

char *
CODEREP::Print_bit(void) const
{
  static char buf[100];
  if (Bitpos() != ILLEGAL_BP)
    sprintf(buf,"%c%d",(Kind() == CK_VAR)?'V':'E',Bitpos());
  else
    sprintf(buf,"%d",Bitpos());
  return buf;
}

// Print a bit position (formatted nicely)
//
char *
STMTREP::Print_bit(void) const
{
  static char buf[100];
  if (Bitpos() != ILLEGAL_BP)
    sprintf(buf,"E%d",Bitpos());
  else
    sprintf(buf,"%d",Bitpos());
  return buf;
}

// ====================================================================
// Can_be_speculated - tell whether the expression tree can be safely
// moved to a point where it might be evaluated under circumstances
// that didn't entail its evaluation in the original program. There
// are several reasons why speculation might be unsafe:
// 1) The expression might contain an opcode that can cause an
//    exception;
// 2) Under scalar_formal_ref, the expression might contain an f90
//    formal parameter access to which might cause an exception; this
//    contingency is allowed for in OPT_STAB::Safe_to_speculate.
//
// Restriction: Valid only for rvalues.
// ====================================================================
BOOL
CODEREP::Can_be_speculated(OPT_STAB *opt_stab) const
{
  switch (Kind()) {
  case CK_VAR:
    return opt_stab->Safe_to_speculate(Aux_id());
  case CK_IVAR: {
    CODEREP *base = Ilod_base() ? Ilod_base() : Istr_base();
    if (! base->Can_be_speculated(opt_stab))
      return FALSE;
    if (OPCODE_Can_Be_Speculative(Op()))
      return TRUE;
    // special case fortran array parameters declared constant bound and with
    // constant subscript that is within bounds (see 669621)
    if (! IS_FORTRAN)
      return FALSE;
    if (! OPERATOR_is_scalar_iload (Opr()))
      return FALSE;
    if (base->Kind() != CK_VAR)
      return FALSE;
    AUX_STAB_ENTRY *psym = opt_stab->Aux_stab_entry(base->Aux_id());
    if (ST_sclass(psym->St()) != SCLASS_FORMAL)
      return FALSE;
    if (ST_is_optional_argument(psym->St()))
      return FALSE;
    TY_IDX ty_idx = ST_type(psym->St());
    if (TY_kind(ty_idx) != KIND_POINTER)
      return FALSE;
    ty_idx = TY_pointed(ty_idx);
    if (TY_kind(ty_idx) != KIND_ARRAY)
      return FALSE;
    if (TY_size(ty_idx) == 0)	// constant bounds
      return FALSE;
    if (Offset() < 0)
      return FALSE;		// will be out of bounds
    return (Offset() + MTYPE_byte_size(Dsctyp())) <= TY_size(ty_idx);
    }
  case CK_OP:
    if (!OPCODE_Can_Be_Speculative(Op())) {
      return FALSE;
    }
    else {
      for (INT i = 0; i < Kid_count(); i++) {
	if (!Opnd(i)->Can_be_speculated(opt_stab)) {
	  return FALSE;
	}
      }
      return TRUE;
    }
  case CK_LDA:
  case CK_CONST:
  case CK_RCONST:
    return TRUE;
  default:
    FmtAssert(FALSE, ("CODEREP::Can_be_speculated: unknown kind"));
    return FALSE;
  }
}

// ====================================================================
// Reset_isop_visited - reset a specific ISOP_VISITED bit in
// all the CK_OP nodes in the tree; can assume that if this flag is not 
// set, it is not set in the entire subtree rooted at that node.
// It passes through the CK_IVAR nodes to get to the CK_OPs underneath.
// Used by:
//	Contains_only_the_var()		ISOP_CONTAIN_VISITED
//	CODEREP::Antloc()		ISOP_ANTLOC_VISITED
//	CODEREP::Antloc_rec()		ISOP_ANTLOC_VISITED
//	BB_LOOP::Invariant_cr()		ISOP_INVARIANT_VISITED
//	BB_LOOP::Invariant_cr_rec()	ISOP_INVARIANT_VISITED
//	MAIN_EMITTER::Check_expr_resolves_to_const() ISOP_EMIT_CONST_VISITED
// ====================================================================
void
CODEREP::Reset_isop_visited(ISOP_FLAG flag)
{
  if (Kind() == CK_OP) {
    if (!Is_isop_flag_set(flag))
      return;
    for  (INT32 i = 0; i < Kid_count(); i++) 
      Opnd(i)->Reset_isop_visited(flag);
    Reset_isop_flag(flag);
  } else if (Kind() == CK_IVAR) {
    CODEREP *base1, *base2;
    if (base1 = Ilod_base())	// if load base exists, follow it
      base1->Reset_isop_visited(flag);
    if ((base2 = Istr_base()) && base1 != base2) // if store base != load
      base2->Reset_isop_visited(flag);		//    then follow it
    if (Opr() == OPR_MLOAD)
      Mload_size()->Reset_isop_visited(flag);
    if (Opr() == OPR_ILOADX)
      Index()->Reset_isop_visited(flag);
  }
}

// Antloc (public) - non-recursive wrapper for Antloc_rec (private)
// resets ISOP_ANTLOC_VISITED bit if Antloc_rec returns TRUE
BOOL
CODEREP::Antloc(BB_NODE *bb)
{
  BOOL ret = Antloc_rec(bb);
  if (ret && WOPT_Enable_Prune)
    Reset_isop_visited(ISOP_ANTLOC_VISITED);
  return ret;
}

// Antloc_rec (private):
// Test if the coderep is antloc by traversing the tree and test all
// terminal nodes are defined locally.
// NOTE: the setting an resetting of ISOP_ANTLOC_VISITED to control
//   the exponential behavior of this routine is modelled after
//   Contains_only_the_var in opt_prop.cxx.
BOOL
CODEREP::Antloc_rec(BB_NODE *bb)
{
  MU_LIST_ITER mu_iter;
  switch (Kind()) {
  case CK_VAR:
    if (Is_flag_set(CF_IS_ZERO_VERSION))
      Warn_todo("CODEREP::Antloc_rec: Testing Antloc for CR of zero version");
    if (Defstmt() == NULL)
      return TRUE;
    else if (Is_flag_set(CF_DEF_BY_PHI))
      return TRUE;
    else
      return Defstmt()->Bb() != bb;
  case CK_IVAR:
    {
      CODEREP *base = Ilod_base() ? Ilod_base() : Istr_base();
      if (!base->Antloc_rec(bb))
	return FALSE;
      MU_NODE *mnode = Ivar_mu_node();
      if (mnode && mnode->Is_Valid()) {
	if (!mnode->OPND()->Antloc_rec(bb))
	  return FALSE;
	// pv 385408, iload inserted by IRM has zero version vsym in mu
	// assume the worst case
	if (mnode->OPND()->Is_flag_set(CF_IS_ZERO_VERSION))
	  return FALSE;
      }
    }
    // Ivar_defstmt is NULL all the time!
    if (Ivar_defstmt() == NULL)
       return TRUE;
    else if (Is_flag_set(CF_DEF_BY_PHI))
       return TRUE;
    else
       return Ivar_defstmt()->Bb() != bb;
    
  case CK_OP:
    {
      if (Is_isop_flag_set(ISOP_ANTLOC_VISITED) && WOPT_Enable_Prune)
	return TRUE;
      for (INT32 i = 0; i < Kid_count(); i++) {
	if (! Opnd(i)->Antloc_rec(bb)) {
	  if (WOPT_Enable_Prune) {
	    for (INT32 j = 0; j < i; j++) // reset ones visited so far
	      Opnd(j)->Reset_isop_visited(ISOP_ANTLOC_VISITED);
	  }
	  return FALSE;
	}
      }
      if (WOPT_Enable_Prune)
	Set_isop_flag(ISOP_ANTLOC_VISITED);
    }
    return TRUE;
  case CK_LDA:
  case CK_CONST:
  case CK_RCONST:
    return TRUE;
  }
  return FALSE;
}


// Returns TRUE if this is divisable by cr
BOOL
CODEREP::Divisable(const CODEREP *cr, OPT_STAB *opt_stab) const
{
  // only handle constants
  if (cr->Kind() != CK_CONST || cr->Const_val() == 0)
    return FALSE;
  
  // Always divisable by +1 or -1.
  if (cr->Const_val() == 1)
    return TRUE;
  else if (cr->Const_val() == -1)
    return TRUE;

  switch (Kind()) {
  case CK_CONST:
    if ((Const_val() % cr->Const_val()) == 0)
      return TRUE;
    return FALSE;
  
  case CK_VAR: 
    {
      TY_IDX ty = Lod_ty();
      if (ty != 0 && TY_kind(ty) == KIND_POINTER) {
	TY_IDX ty_pointed = TY_pointed(ty);
	if (ty_pointed != ((TY_IDX) 0) &&
	    (TY_align(ty_pointed) % cr->Const_val()) == 0)
	  return TRUE;
      } 
      if (Defstmt() != NULL && !Is_flag_set((CR_FLAG) (CF_DEF_BY_PHI|CF_DEF_BY_CHI))) {
	return Defstmt()->Rhs()->Divisable(cr, opt_stab);
      }
    }
    return FALSE;
    
  case CK_LDA:
    {
      if ((Offset() % cr->Const_val()) != 0)
	return FALSE;
      ST *st = opt_stab->Aux_stab_entry(Lda_aux_id())->St();
      if (st) {
	INT32 align = Adjusted_Alignment(st);
	if ((align % cr->Const_val()) == 0)
	  return TRUE;
      }
    }
    return FALSE;	    
    
  case CK_OP:
    {
      OPERATOR opr = Opr();
      switch (opr) {
      case OPR_ADD:
      case OPR_SUB:
	return Opnd(0)->Divisable(cr, opt_stab) && Opnd(1)->Divisable(cr, opt_stab);
      case OPR_ARRAY:
	return Opnd(0)->Divisable(cr, opt_stab) && (Elm_siz() % cr->Const_val()) == 0;
      }
    }
    return FALSE;
  } 
  return FALSE;
}

// Given an expression tree, infer if the result computed is a system address;
// used by LFTR.  Potentially it is useful for strength reduction as well.
ADDRESSABILITY
CODEREP::Check_if_result_is_address(OPT_STAB *opt_stab) const
{
  ADDRESSABILITY res0, res1;

  if (! MTYPE_is_integral(Dtyp()))
    return ADDRESSABILITY_NOT_ADDRESS;

  switch (Kind()) {
  case CK_LDA: 
    return ADDRESSABILITY_IS_ADDRESS;
  case CK_CONST:
  case CK_RCONST:
    return ADDRESSABILITY_NOT_ADDRESS;
  case CK_VAR: {
    AUX_STAB_ENTRY *aux = opt_stab->Aux_stab_entry(Aux_id());
    // check if info cached in aux_stab_entry by EPRE
    if (aux->Is_address())
      return ADDRESSABILITY_IS_ADDRESS;
    if (aux->Not_address())
      return ADDRESSABILITY_NOT_ADDRESS;
    // determine according to Lod_ty()
    if (TY_kind(Lod_ty()) == KIND_POINTER) 
      return ADDRESSABILITY_IS_ADDRESS;
    return ADDRESSABILITY_UNKNOWN;
    }
  case CK_IVAR:
    // determine according to Ilod_ty()
    if (OPERATOR_is_scalar_iload (Opr()) &&
	TY_kind(Ilod_ty()) ==KIND_POINTER)
      return ADDRESSABILITY_IS_ADDRESS;
    return ADDRESSABILITY_UNKNOWN;
  case CK_OP: {
    OPERATOR opr = Opr();
    switch (opr) {
    // n-ary operators
    case OPR_ARRAY: return ADDRESSABILITY_IS_ADDRESS;
    // ternary operators
    case OPR_SELECT: 
      res1 = Opnd(1)->Check_if_result_is_address(opt_stab); 
      return (res1 == ADDRESSABILITY_UNKNOWN) ? 
			Opnd(2)->Check_if_result_is_address(opt_stab) : res1;
    // binary operators
    case OPR_SHL: case OPR_LSHR: case OPR_ASHR: 
    case OPR_LAND: case OPR_LIOR:
    case OPR_BAND: case OPR_BIOR: case OPR_BNOR: case OPR_BXOR: 
    case OPR_EQ: case OPR_NE: 
    case OPR_GE: case OPR_GT: case OPR_LE: case OPR_LT:
    case OPR_DIV: case OPR_MOD: case OPR_REM: case OPR_DIVREM:
    case OPR_MPY: case OPR_HIGHMPY: case OPR_XMPY:
      return ADDRESSABILITY_NOT_ADDRESS;
    case OPR_MAX: case OPR_MIN: case OPR_MINMAX:
      res0 = Opnd(0)->Check_if_result_is_address(opt_stab); 
      if (res0 == ADDRESSABILITY_UNKNOWN)
	return ADDRESSABILITY_UNKNOWN;
      return Opnd(1)->Check_if_result_is_address(opt_stab); 
    case OPR_ADD: case OPR_SUB:
      res0 = Opnd(0)->Check_if_result_is_address(opt_stab); 
      if (res0 == ADDRESSABILITY_UNKNOWN)
	return ADDRESSABILITY_UNKNOWN;
      res1 = Opnd(1)->Check_if_result_is_address(opt_stab); 
      if (res1 == ADDRESSABILITY_UNKNOWN)
	return ADDRESSABILITY_UNKNOWN;
      if (res0 == res1) {
        if (res0 == ADDRESSABILITY_IS_ADDRESS)
          return ADDRESSABILITY_NOT_ADDRESS;
	return res0;
      }
      else return ADDRESSABILITY_IS_ADDRESS;
    // unary operators
    case OPR_CVT: case OPR_TAS:
    case OPR_NEG: case OPR_PAREN: case OPR_MINPART: case OPR_MAXPART: 
    case OPR_PARM:
      return Opnd(0)->Check_if_result_is_address(opt_stab); 
    case OPR_CVTL:
    case OPR_ABS: case OPR_SQRT: case OPR_RSQRT: case OPR_RECIP: 
    case OPR_RND: case OPR_TRUNC: case OPR_CEIL: case OPR_FLOOR: 
    case OPR_BNOT: case OPR_LNOT: case OPR_LOWPART: case OPR_HIGHPART:
      return ADDRESSABILITY_NOT_ADDRESS;
    default: ;
    }
    return ADDRESSABILITY_UNKNOWN;
    }
  default: ;
  }
  return ADDRESSABILITY_UNKNOWN;
}

// Print STMTREP tree
//
void
STMTREP::Print(FILE *fp) const
{
  if (_opr == OPR_PRAGMA) {
    fprintf(fp, ">");	// mark line visually as htable dump
    fdump_tree( fp, Orig_wn() );
    return;
  }

  if (Has_mu()) {
    MU_NODE *mnode;
    MU_LIST_ITER mu_iter;
    fprintf(fp, " mu<");
    if (Mu_list()) {
      FOR_ALL_NODE(mnode, mu_iter, Init(Mu_list())) 
	if (mnode->OPND())  // Check if the MU has been deleted.
	  fprintf(fp, "%d/cr%d ", mnode->Aux_id(), mnode->OPND()->Coderep_id());
    }
    fprintf(fp, ">\n"); 
  }

  if (Rhs()) Rhs()->Print(1, fp);
  if (OPERATOR_is_scalar_istore (_opr) || _opr == OPR_ISTOREX ||
      _opr == OPR_MSTORE)
    Lhs()->Istr_base()->Print(1, fp);
  if (_opr == OPR_MSTORE)
    Lhs()->Mstore_size()->Print(1, fp);
  if (_opr == OPR_ISTOREX)
    Lhs()->Index()->Print(1, fp);
  Print_node(fp);
  if (OPERATOR_is_call(_opr)) 
    fprintf(fp, " call-flag:0x%x", Call_flags());
  fprintf(fp, " b=%s",Print_bit());
  fprintf(fp, " flags:0x%x", _flags&0x1f );
  fprintf(fp, " pj%d", Proj_op_uses());
  fprintf(fp, " Sid%d\n", Stmt_id());

  if (Has_chi()) {
    CHI_NODE *cnode;
    CHI_LIST_ITER chi_iter; 
    fprintf(fp,"chi <");
    FOR_ALL_NODE(cnode, chi_iter, Init(Chi_list())) {
      fprintf(fp, "%d", cnode->Aux_id());
      if (cnode->Live())  // Check if the MU has been deleted.
	fprintf(fp, "/cr%d/cr%d",
		cnode->RESULT()->Coderep_id(), cnode->OPND()->Coderep_id()); 
      fprintf(fp, " ");
    }
    fprintf(fp, "> 0x%p\n", this);
  }
}

// Print the STMTREP node
//
void
STMTREP::Print_node(FILE *fp) const
{
  switch (_opr) {
  case OPR_STBITS:
  case OPR_STID:
    fprintf(fp, ">");	// mark line visually as htable dump
    fprintf(fp, "%s %s %s sym%dv%d", OPERATOR_name(_opr),
	    MTYPE_name(Lhs()->Dtyp()), MTYPE_name(Lhs()->Dsctyp()),
	    Lhs()->Aux_id(), Lhs()->Version());
    fprintf(fp, " %d", Lhs()->Offset());
    fprintf(fp, " <u=%d cr%d>", Lhs()->Usecnt(), Lhs()->Coderep_id());
    break;
  case OPR_ISTORE:
  case OPR_ISTBITS:
    fprintf(fp, ">");	// mark line visually as htable dump
    fprintf(fp, "%s %d ", OPERATOR_name(_opr), Lhs()->Offset());
    fprintf(fp, " <u=%d cr%d>", Lhs()->Usecnt(), Lhs()->Coderep_id());
    break;
  case OPR_ISTOREX:
    fprintf(fp, ">");	// mark line visually as htable dump
    fprintf(fp, "%s", OPERATOR_name(_opr));
    fprintf(fp, " <u=%d cr%d>",Lhs()->Usecnt(), Lhs()->Coderep_id());
    break;
  case OPR_MSTORE:
    fprintf(fp, ">");	// mark line visually as htable dump
    fprintf(fp, "%s", OPERATOR_name(_opr));
    fprintf(fp, " <u=%d cr%d>",Lhs()->Usecnt(), Lhs()->Coderep_id());
    break;
  case OPR_TRUEBR:
  case OPR_FALSEBR:
  case OPR_LABEL:
  case OPR_GOTO:
  case OPR_REGION_EXIT:
    fprintf(fp, ">");	// mark line visually as htable dump
    fprintf(fp, "%s %d", OPERATOR_name(_opr), Label_number());
    break;
  default:
    fprintf(fp, ">");	// mark line visually as htable dump
    fprintf(fp, "%s", OPERATOR_name(_opr));
  }

  if ( Black_box() ) {
    fprintf(fp, " (black-box)");
  }
}

// print the STMTREP to a string
char *
STMTREP::Print_str(BOOL name_format)
{
  static char buf[100];

  buf[0] = '\0';

  if (!name_format) {
    if (Lhs())
      strcat(buf,Lhs()->Print_str(name_format));
    if (Rhs())
      strcat(buf,Rhs()->Print_str(name_format));
    return buf;
  }

  switch (_opr) {
    case OPR_STID:
    case OPR_STBITS:
      sprintf(buf,"%s sym%dv%d",
	      OPERATOR_name(_opr)+4,Lhs()->Aux_id(),Lhs()->Version());
      break;
    case OPR_ISTORE:
    case OPR_ISTBITS:
    case OPR_ISTOREX:
    case OPR_MSTORE:
      sprintf(buf,"%s",OPERATOR_name(_opr)+4);
      break;
    default:
      sprintf(buf,"%s",OPERATOR_name(_opr)+4);
  }
  if (strlen(buf) < Nchars) {	// pad with spaces to Nchars
    INT i;
    for (i = strlen(buf); i < Nchars; ++i)
      buf[i] = ' ';
    buf[i] = '\0';
  }
  return buf;
}

// Print out the CR
void
CODEMAP::Print_CR(CODEREP *cr, FILE *fp) const
{
  INT32 i;
  fprintf(fp, "%ccr%d b=%s ",(cr->Usecnt() <= 0) ? '*' : ' ',
          cr->Coderep_id(),cr->Print_bit());
  cr->Print_node(0, fp);
  if (cr->Kind() == CK_OP) {
    fprintf(fp, " kids<");
    for (i = 0; i < cr->Kid_count(); i++)
      fprintf(fp, "cr%d ", cr->Get_opnd(i)->Coderep_id());
    fprintf(fp, ">");
  } else if (cr->Kind() == CK_IVAR) {
    if (cr->Ilod_base()) {
      fprintf(fp, " ilod<cr%d>", cr->Ilod_base()->Coderep_id());
    }
    if (cr->Istr_base()) {
      fprintf(fp, " istr<cr%d>", cr->Istr_base()->Coderep_id());
    }
    MU_NODE *mnode = cr->Ivar_mu_node();
    if (mnode && mnode->OPND()) { // Check if the MU has been deleted.
      fprintf(fp, " mu<");
      fprintf(fp, "%d/cr%d", mnode->Aux_id(), mnode->OPND()->Coderep_id());
      fprintf(fp, ">");
    }
  }
  fprintf(fp, " 0x%p",cr);
  fprintf(fp, "\n");
}

//  Print out the statement rep
void
CODEMAP::Print_SR(STMTREP *stmt, FILE *fp) const
{
  fprintf(fp, " SR %p b=%s ", stmt, stmt->Print_bit());
  stmt->Print_node(fp);
  fprintf(fp, " kids<");
  if ((stmt->Opr() == OPR_CALL) ||
      (stmt->Opr() == OPR_ICALL) ||
      (stmt->Opr() == OPR_INTRINSIC_CALL)) {
    CODEREP *cr = stmt->Rhs();
    INT kcount = cr->Kid_count();
    for (INT i = 0; i < kcount; i++) {
      fprintf(fp, "cr%d ", cr->Get_opnd(i)->Coderep_id());
    }
  }
  if (OPERATOR_is_scalar_istore (stmt->Opr()) || stmt->Opr() == OPR_ISTOREX)
    fprintf(fp, "cr%d ", stmt->Lhs()->Coderep_id());
  if (stmt->Rhs())
    fprintf(fp, "cr%d ", stmt->Rhs()->Coderep_id());
  fprintf(fp, ">");
  if (stmt->Has_mu()) {
    MU_NODE *mnode;
    MU_LIST_ITER mu_iter;
    fprintf(fp, " mu<");
    if (stmt->Mu_list()) {
      FOR_ALL_NODE(mnode, mu_iter, Init(stmt->Mu_list())) 
	if (mnode->OPND())  // Check if the MU has been deleted.
	  fprintf(fp, "%d/cr%d ", mnode->Aux_id(), mnode->OPND()->Coderep_id());
    }
    fprintf(fp, ">\n");
  }
  if (stmt->Has_chi()) {
    CHI_NODE *cnode;
    CHI_LIST_ITER chi_iter;
    fprintf(fp, " chi<");
    if (stmt->Chi_list()) {
      FOR_ALL_NODE(cnode, chi_iter, Init(stmt->Chi_list())) {
	fprintf(fp, "%d", cnode->Aux_id());
	if (cnode->Live())  // Check if the MU has been deleted.
	  fprintf(fp, "/cr%d/cr%d", 
		cnode->RESULT()->Coderep_id(), cnode->OPND()->Coderep_id());
	fprintf(fp, " ");
      }
    }
    fprintf(fp, ">");
  }
  fprintf(fp," 0x%p" ,stmt);
  fprintf(fp, "\n");
}


// Print out the whole CODEMAP table.
//
void
CODEMAP::Print(FILE *fp) const
{
  CODEREP_ITER cr_iter;
  CODEREP *cr,*bucket;
  CODEMAP_ITER codemap_iter;
  INT32 count;
  INT32 scalar_var_count = 0;
  INT32 unique_vsym_count = 0;
  INT32 lda_vsym_count = 0;
  INT32 ivar_count = 0;
  
  UINT32 sum_bucket_len = 0;
  UINT32 ssq_bucket_len = 0;
  UINT32 max_bucket_len = 0;
  UINT32 num_nonempty_buckets = 0;

  fprintf(fp, "- - - Default vsym is sym%1d\n", Sym()->Default_vsym());
  fprintf(fp, "- - - Return vsym is sym%1d\n", Sym()->Return_vsym());

  // print coderep nodes in htable:
  count = 0;
  FOR_ALL_ELEM(bucket, codemap_iter, Init(this)) {
    UINT32 bucket_len = 0;
    if (bucket)
      fprintf(fp, "----bucket %d\n", codemap_iter.Cur());
    FOR_ALL_NODE(cr, cr_iter, Init(bucket)) {
      Print_CR(cr, fp);
      count++;
      if (cr->Kind() == CK_IVAR && cr->Opr() != OPR_PREFETCH) 
        ivar_count++;
      bucket_len++;
    }
    if (bucket_len != 0) {
      ++num_nonempty_buckets;
    }
    sum_bucket_len += bucket_len;
    ssq_bucket_len += bucket_len * bucket_len;
    max_bucket_len = MAX(max_bucket_len, bucket_len);
  }
  fprintf(fp, "%d of %d buckets are nonempty.\n",
	  num_nonempty_buckets, size);
  fprintf(fp, "Average nonempty bucket (chain) length: %g\n",
	  ((double) sum_bucket_len) / num_nonempty_buckets);
  fprintf(fp, "Average          bucket (chain) length: %g\n",
	  ((double) sum_bucket_len) / size);
  fprintf(fp, "Bucket len  (nonempty)  std. deviation: %g\n",
	  (double) sqrt(((double) num_nonempty_buckets * ssq_bucket_len -
			 sum_bucket_len * sum_bucket_len) /
			(num_nonempty_buckets * num_nonempty_buckets)));
  fprintf(fp, "Bucket length           std. deviation: %g\n",
	  (double) sqrt(((double) size * ssq_bucket_len -
			 sum_bucket_len * sum_bucket_len) /
			(size * size)));
  fprintf(fp, "Maximum         bucket          length: %u\n\n",
	  max_bucket_len);

  CFG_ITER cfg_iter;
  STMTREP *stmt;
  BB_NODE *bb;

  // print coderep nodes in aux_stab:
  AUX_ID i;
  AUX_STAB_ITER aux_stab_iter(Sym());
  FOR_ALL_NODE(i, aux_stab_iter, Init()) {
    AUX_STAB_ENTRY *aux = Sym()->Aux_stab_entry(i);
    fprintf(fp, "----aux_id %d\n", i);
    FOR_ALL_NODE(cr, cr_iter, Init(aux->Cr_list())) {
      Print_CR(cr, fp);
      count++;
      Is_True(cr->Kind() == CK_VAR, ("CODEMAP::Print: coderep in aux_stab not CK_VAR"));
      if (aux->Is_real_var())
        scalar_var_count++;
      else if (Sym()->Unique_vsym(cr->Aux_id()))
        unique_vsym_count++;
      else if (Sym()->Lda_vsym(cr->Aux_id()))
        lda_vsym_count++;
    }
  }

  fprintf(fp, "\nNumber of CODEREP nodes = %d (%d scalar vars, "
	  "%d unique vsyms, %d lda vsyms, %d ivars)\n", count,
	  scalar_var_count, unique_vsym_count, lda_vsym_count,
	  ivar_count);

  count = 0;
  FOR_ALL_NODE(bb, cfg_iter, Init(_cfg)) {
    // stupid way of doing a constructor only because Init() doesn't
    // take any argument.
    STMTREP_ITER stmt_iter(bb->Stmtlist());
    FOR_ALL_NODE(stmt, stmt_iter, Init()) {
      Print_SR(stmt, fp);
      count++;
    }
  }
  fprintf(fp, "\nNumber of STMTREP nodes = %d\n\n", count);
}

// Reset the ISOP_DCE_VISITED and ISOP_RETVSYM_VISITED flag
// in the CODEREP
//
void
CODEMAP::Reset_DCE_visited_flags()
{
  CODEREP_ITER cr_iter;
  CODEREP *cr,*bucket;
  CODEMAP_ITER codemap_iter;

  FOR_ALL_ELEM(bucket, codemap_iter, Init(this)) {
    FOR_ALL_NODE(cr, cr_iter, Init(bucket)) {
      if (cr->Kind() == CK_OP) {
	cr->Reset_isop_flag(ISOP_DCE_VISITED);
	cr->Reset_isop_flag(ISOP_RETVSYM_VISITED);
      }
    }
  }
}



// ====================================================================
// Determine if the assignment is " i = i " that can be deleted by 
//   by the joint effort of COPYPROP and DCE.
// ====================================================================
BOOL
STMTREP::Is_identity_assignment_removable() const
{
  if (! OPERATOR_is_scalar_store (Opr()))
    return FALSE;

  const CODEREP *lhs = Lhs();
  const CODEREP *rhs = Rhs();

  // store to a volatile location?
  if ( lhs->Is_var_volatile() )
    return FALSE;

  if (Has_zver())
    return FALSE;

  // statements of form i = i are not required (unless volatile)
  //  (even it stores a dedicated register)
  if (rhs->Kind()==CK_VAR && rhs->Aux_id()==lhs->Aux_id() &&
      !rhs->Is_var_volatile()) 
    return TRUE;

  return FALSE;
}

// ====================================================================
// Determine if statement has a reference to a volatile memory location
// ====================================================================

BOOL 
STMTREP::Contains_volatile_ref(const BVECTOR &cr_vol_map) const
{
   BOOL            pred = FALSE;
   CODEREP * const rhs = Rhs();
   CODEREP * const lhs = Lhs();
   
   if (OPCODE_is_fake(Op()))
   {
      for (INT32 i = 0; !pred && i < rhs->Kid_count(); i++)
	 if (cr_vol_map[rhs->Opnd(i)->Coderep_id()])
	    pred = TRUE;
   }
   else if ((rhs != NULL && cr_vol_map[rhs->Coderep_id()]))
   {
      pred = TRUE;
   }
   if (lhs != NULL && cr_vol_map[lhs->Coderep_id()])
   {
      pred = TRUE;
   }
   return pred;
} //STMTREP::Contains_volatile_ref


// ====================================================================
// Determine if statement has a chi whose result is zero version
// ====================================================================
BOOL
STMTREP::Has_zero_version_chi(void) const
{
  if (! Has_chi())
    return FALSE;
  CHI_LIST_ITER chi_iter;
  CHI_NODE *cnode;
  CHI_LIST *chi_list = Chi_list();
  FOR_ALL_NODE( cnode, chi_iter, Init(chi_list)) {
    if (cnode->Live() && 
        (cnode->RESULT()->Is_flag_set(CF_IS_ZERO_VERSION) ||
         cnode->RESULT()->Is_flag_set(CF_INCOMPLETE_USES)))
      return TRUE;
  }
  return FALSE;
}


void
CODEMAP::Insert_var_phi(CODEREP *new_lhs, BB_NODE *bb)
{
  BB_NODE_SET_ITER    df_iter;
  BB_NODE *bb_phi;
  AUX_ID aux = new_lhs->Aux_id();

  FOR_ALL_ELEM (bb_phi, df_iter, Init(bb->Dom_frontier())) {
    PHI_NODE *phi = Lookup_var_phi(bb_phi, aux);

    // Fix 471645:  phis inserted by EPRE/LPRE are annotated with the INCOMPLETE flag.
    // The flag means a phi might not be found on this definition's dominance frontier.
    // For this reason, Insert_var_phi should recursively visit all DF+ to check
    // if phi should be inserted.

    if (phi == NULL || !phi->Live()) {
      // Insert a new phi node at the dominator frontiers.
      if (phi == NULL) {
	phi = bb_phi->Phi_list()->New_phi_node(aux, Ssa()->Mem_pool(), bb_phi);

	// Enter into the var phi hash table.
	Enter_var_phi_hash(phi);

      } else {
	phi->Reset_dse_dead();
	phi->Reset_dce_dead();
	phi->Set_res_is_cr();
	phi->Set_live();
      }

      CODEREP *phi_res = Add_def(aux, -1, 
				 NULL, new_lhs->Dtyp(), new_lhs->Dsctyp(),
				 new_lhs->Offset(), new_lhs->Lod_ty(),
				 new_lhs->Field_id(), TRUE);
      phi_res->Set_flag(CF_DEF_BY_PHI);
      phi_res->Set_defphi(phi);
      phi->Set_live();
      phi->Set_result(phi_res);

      Is_Trace(Tracing(),
	   (TFile, "CODEMAP: insert var phi at BB%d for sym%d\n", bb->Id(),
	    aux));

      // Put zero versions at the phi opnds.
      CODEREP *zcr = Ssa()->Get_zero_version_CR(aux, Opt_stab(), 0);
      BB_NODE *pred;
      BB_LIST_ITER bb_iter;
      INT opnd = 0;
      FOR_ALL_ELEM (pred, bb_iter, Init(bb_phi->Pred())) {
	phi->Set_opnd(opnd, zcr);
	opnd++;
      }

      // Recursively introduce phi nodes
      Insert_var_phi(new_lhs, bb_phi);

    } else if (phi->Incomplete()) {

      phi->Reset_incomplete();  // phi will be inserted at each of its DF+

      // Recursively introduce phi nodes
      Insert_var_phi(new_lhs, bb_phi);
    }
  }
}

void STMTREP::Clone(STMTREP *sr, CODEMAP *htable, MEM_POOL *pool)
{
  bcopy(sr,this,sizeof(STMTREP));
  
  Set_Next(NULL); Set_Prev(NULL);

  switch (sr->Opr()) {
  case OPR_STID:
  case OPR_STBITS:
    {
      CODEREP *cr = sr->Lhs();
      Set_lhs(htable->Add_def(cr->Aux_id(), -1,
			      this, cr->Dtyp(), cr->Dsctyp(),
			      cr->Offset(), cr->Lod_ty(), cr->Field_id(),
			      true));
      if (sr->Has_mu()) {
	Set_mu_list( CXX_NEW(MU_LIST, pool));
	Mu_list()->Clone_mu_list(sr->Mu_list(), pool);
      } else
	Set_mu_list(NULL);

      if (sr->Has_chi()) {
	Set_chi_list( CXX_NEW(CHI_LIST, pool));
	Chi_list()->Clone_chi_list(sr->Chi_list(), pool);
      } else 
	Set_chi_list(NULL);
    }
    break;

  case OPR_ISTORE:
  case OPR_ISTBITS:
  case OPR_MSTORE:
    {
      CODEREP *cr = sr->Lhs();
      OCC_TAB_ENTRY *occ = (OCC_TAB_ENTRY*) CXX_NEW(OCC_TAB_ENTRY, pool);
      occ->Clone(cr->Ivar_occ(), pool);
      MU_NODE *mnode = NULL;
      Set_lhs(htable->Add_idef(cr->Op(), occ,
			       this,
			       NULL, cr->Dtyp(), cr->Dsctyp(), cr->Ilod_ty(),
			       cr->I_field_id(), cr->Offset(),
			       cr->Mload_size(), NULL, cr->Istr_base()));
      Set_chi_list( CXX_NEW(CHI_LIST, pool));
      Chi_list()->Clone_chi_list(sr->Chi_list(), pool);
    }
    break;

  case OPR_ISTOREX:
    Is_True(FALSE, ("OPR_ISTOREX not expected."));

  default:
    if (sr->Has_mu()) {
      Set_mu_list( CXX_NEW(MU_LIST, pool));
      Mu_list()->Clone_mu_list(sr->Mu_list(), pool);
    } else
      Set_mu_list(NULL);

    if (sr->Has_chi()) {
      Set_chi_list( CXX_NEW(CHI_LIST, pool));
      Chi_list()->Clone_chi_list(sr->Chi_list(), pool);
    } else 
      Set_chi_list(NULL);
  }

  if (sr->Has_chi()) {
    CHI_LIST_ITER chi_iter;
    CHI_NODE *cnode;
    FOR_ALL_NODE( cnode, chi_iter, Init(Chi_list())) {
      if (!cnode->Dse_dead()) {
	CODEREP *cr = cnode->RESULT();
	CODEREP *newdef = htable->Add_def(cr->Aux_id(), -1,
					  this, cr->Dtyp(), cr->Dsctyp(),
					  cr->Offset(), cr->Lod_ty(),
					  cr->Field_id(), TRUE);
	newdef->Set_flag(CF_DEF_BY_CHI);
	newdef->Set_defchi(cnode);
	cnode->Set_RESULT(newdef);
      }
    }
    Reset_has_zver();
  }
}




