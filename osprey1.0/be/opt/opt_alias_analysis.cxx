//-*-c++-*-
// ====================================================================
// ====================================================================
//
// Module: opt_alias_analysis.cxx
// $Revision: 1.1.1.1 $
// $Date: 2002-05-22 20:06:46 $
// $Author: dsystem $
// $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/opt/opt_alias_analysis.cxx,v $
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
// Description:
//
// opt_alias_analysis.cxx contains a collection of methods, mostly in
// the class OPT_STAB, that generate the MU and CHI nodes and the
// POINTS_TO pointer information, builds the occurrence table, and
// inserts virtual symbols into the optimizer symbol table.  This alias
// analysis is performed in two passes.
//
// Flow free analysis (FFA) creates the initial MU and CHI lists and
// initializes the POINTS_TO data.  It also gen4erate the occurrence
// table and the virtual symbols.  It is invoked through Compute_FFA.
//
// Flow sensitive analysis (FSA) uses def-use (DU) chains to improve
// the information obtained during FFA.  It is invoked through
// Compute_FSA, but only after the constructions of SSA representation.
// 
// ====================================================================
//
// Implementation:
//
//  Allocate_mu_chi_and_virtual_var
//  Transfer_alias_class_to_occ_and_aux
//  Generate_mu_and_chi_list
// Compute_FFA / Compute_FFA_for_copy
//
//   Compute_FSA_stmt_or_expr
//  Compute_FSA_dominator_order
// Compute_FSA
// 
// ====================================================================
// ====================================================================


#ifdef USE_PCH
#include "opt_pch.h"
#endif // USE_PCH
#pragma hdrstop


#include "defs.h"			// INT32, INT64

#include "opt_defs.h"
#include "opt_config.h"
#include "stab.h"
#include "targ_sim.h"
#include "tracing.h"			// trace flags
#include "wn_util.h"
#include "optimizer.h"
#include "errors.h"
#include "erbe.h"
#include "stblock.h"
#include "config.h"			// Alias_Pointer_Parms
#include "config_opt.h"
#include "config_ipa.h"			// IPA_Enable_Alias_Class
#include "region_util.h"		// RID, POINTS_TO_SET
#include "srcpos.h"
#include "opt_base.h"			// FOR_ALL_NODE
#include "opt_util.h"
#include "opt_sym.h"			// aux_id
#include "opt_ssa.h"
#include "opt_cfg.h"			// Cfg()
#include "opt_mu_chi.h"			// OCC_TAB_ENTRY, New_mu_node
#include "opt_points_to.h"		// Is_nested_call, etc
#include "opt_alias_class.h"		// alias classification stuff
#include "opt_alias_rule.h"
#include "region_alias_templates.h"	// REGION_search_set
#include "opt_dbg.h"                    // for g_comp_unit
#include "opt_main.h"                   // for COMP_UNIT

extern "C" {
#include "bitset.h"
}


// ======================================================================
//
// Simplify_Pointer(WN *wn_addr, POINTS_TO *ai) analyzes the pointer
// address wn_addr and assigns appropriate values to the POINTS_TO *ai.
//
// Simplify_Pointer is called only below by Analyze_Base_Flow_Free,
// Analyze_Base_Flow_Sensitive, and Create_barrier_def.  It invokes three
// helper procedures:
//
// Analyze_Range is given the POINTS_TO *pt for the ARRAY base and
// modified it to incorporate the ARRAY index.
//
// Simplify_Pointer_Arith handles pointer address computations involving
// addition and subtraction.
//
// Simplify_Pointer_Ver follows the DU chain to expand the pointer
// expression, and is only invoked during flow sensitive analysis.
// 
// ======================================================================


//  Analyze range of OPR_ARRAY
//
void OPT_STAB::Analyze_Range(WN *wn_arr, POINTS_TO *pt)
{
  Is_True(WN_operator(wn_arr) == OPR_ARRAY, ("WN operator is not ARRAY."));

  Is_True(pt->Bit_Size() == 0, ("OPR_ARRAY should not address bit fields"));

  // Nothing can done if the base offset is not fixed.
  if (pt->Ofst_kind() != OFST_IS_FIXED || !pt->Is_pointer())
    return;

  mINT64 product = WN_element_size(wn_arr);

  if (product < 0) {
    // Negative element size signifies non-contiguous array. There are
    // no address bounds implicit in a non-contiguous array access, so
    // we can't do anything. (bug 708002)
    return;
  }

  // Compute the lower and upper bound
  mINT64 upper = WN_element_size(wn_arr);
  mINT64 lower = 0;
  INT32 n_dim = WN_num_dim(wn_arr);

  // scan from lower dimension to higher dimension
  //
  for (INT32 i = n_dim; i >= 1; i--) {
    WN *wn_dim = WN_kid(wn_arr,i);
    WN *wn_index = WN_kid(wn_arr, i + n_dim);
    
    if ( WN_operator(wn_dim) != OPR_INTCONST || 
	 WN_const_val(wn_dim) == 1 ||
	 WN_const_val(wn_dim) == 0) {
      // unbound array 
      pt->Set_byte_ofst(0);
      pt->Set_byte_size(0);
      pt->Set_ofst_kind( OFST_IS_UNKNOWN );
      return;
    }
    if ( WN_operator(wn_index) == OPR_INTCONST) {
      lower = product * WN_const_val(wn_index) + lower;
      upper = product * WN_const_val(wn_index) + upper;
    } else {
      // Is this right for intermediate dims? It looks wrong for,
      // e.g., x[4][i][2]. -- RK
      lower = 0;
      upper = product * WN_const_val(wn_dim);
    }
    product *= WN_const_val(wn_dim);
  }
  
  // e.g., 
  // len is 400 bytes. ofst = 0, size = 400
  // element size is 4 bytes.
  // access mem range from byte 0 to 399.
  // the pointer range is 0,4,8, ... 396.   396 == 400 - 4.
  upper = upper - WN_element_size(wn_arr);
  Is_True(upper >= lower, ("range error in OPT_STAB::Analyze_Range."));

  pt->Set_byte_ofst( pt->Byte_Ofst() + lower );
  // According to the comment above, the following line sets the size
  // to be the true size minus the element size. In particular, for a
  // single-element array, the size will be zero. Is this right? -- RK
  pt->Set_byte_size( upper - lower );
  pt->Set_ofst_kind( OFST_IS_FIXED );
  return;
}


//  Simplify Pointer Arithmetics
//    + and - are allowed in pointer arithmetics
//    the pointer portion is followed indefinitely
//    the integer portion is followed one step.
//
void OPT_STAB::Simplify_Pointer_Arith(WN *wn_expr, POINTS_TO *ai)
{
  OPERATOR opr = WN_operator(wn_expr);
  switch (opr) {
    
  case OPR_INTCONST:
    ai->Set_expr_kind(EXPR_IS_INT);
    ai->Set_const_val(WN_const_val(wn_expr));
    CHECK_POINTS_TO(ai);
    break;

  case OPR_LDBITS:
  case OPR_ILDBITS:
  case OPR_LDID:
  case OPR_ILOAD:
    {
      TY_IDX ty = WN_ty(wn_expr);
      if (TY_kind(ty) == KIND_POINTER) {
	Simplify_Pointer(wn_expr, ai);
      } else {
	ai->Set_expr_kind(EXPR_IS_INT);
      }
    }
    CHECK_POINTS_TO(ai);
    break; 
    
  case OPR_LDA:
  case OPR_ARRAY:
    Simplify_Pointer(wn_expr, ai);
    CHECK_POINTS_TO(ai);
    break;

  case OPR_PAREN:
    Simplify_Pointer_Arith(WN_kid0(wn_expr), ai);
    break;
			   
  case OPR_NEG:
    {
      POINTS_TO alias0;
      alias0.Init();
      Simplify_Pointer_Arith(WN_kid0(wn_expr), &alias0);
      if (alias0.Expr_kind() == EXPR_IS_INT)
	ai->Copy_fully(alias0);
      else {
	ai->Set_expr_kind(EXPR_IS_UNKNOWN);
	ai->Set_base_kind(BASE_IS_UNKNOWN);
	ai->Set_ofst_kind(OFST_IS_UNKNOWN);
      }
      break;
    }

  case OPR_ADD:
    {
      POINTS_TO alias0;
      POINTS_TO alias1;
      
      alias0.Init();
      alias1.Init();
      alias0.Set_expr_kind(EXPR_IS_UNKNOWN);
      alias1.Set_expr_kind(EXPR_IS_UNKNOWN);
      
      Simplify_Pointer_Arith(WN_kid0(wn_expr), &alias0);
      Simplify_Pointer_Arith(WN_kid1(wn_expr), &alias1);

      ai->Set_expr_kind(EXPR_IS_UNKNOWN);
      if (alias0.Expr_kind() == EXPR_IS_BEING_PROCESSED) {
	ai->Copy_fully(alias0);
      } else if (alias0.Expr_kind() == EXPR_IS_ADDR) {
	if (alias1.Expr_kind() == EXPR_IS_INT) {
	  ai->Copy_fully(alias0);
	  if (alias0.Ofst_kind() == OFST_IS_FIXED && alias1.Int_is_constant())
	    ai->Shift_ofst( alias1.Int_const_val());
	  else
	    ai->Set_ofst_kind(OFST_IS_UNKNOWN);
	} else if (alias1.Expr_kind() == EXPR_IS_ADDR) {
	  ai->Copy_fully(alias0);
	  ai->Meet(&alias1, (ST *) wn_expr);
	}
      } else if (alias0.Expr_kind() == EXPR_IS_INT) {
	if (alias1.Expr_kind() == EXPR_IS_ADDR) {
	  ai->Copy_fully(alias1);
	  if (alias1.Ofst_kind() == OFST_IS_FIXED && alias0.Int_is_constant())
	    ai->Shift_ofst( alias0.Int_const_val());
	  else
	    ai->Set_ofst_kind(OFST_IS_UNKNOWN);
	} else if (alias1.Expr_kind() == EXPR_IS_INT) {
	  ai->Set_expr_kind(EXPR_IS_INT);
	}
      }
      if (ai->Expr_kind() == EXPR_IS_UNKNOWN) {
	ai->Set_base_kind(BASE_IS_UNKNOWN);
	ai->Set_ofst_kind(OFST_IS_UNKNOWN);
      }
      CHECK_POINTS_TO(ai);
    }
    break;
    
  case OPR_SUB: 
    {
      POINTS_TO alias0;
      POINTS_TO alias1;
      
      alias0.Init();
      alias1.Init();
      alias0.Set_expr_kind(EXPR_IS_UNKNOWN);
      alias1.Set_expr_kind(EXPR_IS_UNKNOWN);
      
      Simplify_Pointer_Arith(WN_kid0(wn_expr), &alias0);
      Simplify_Pointer_Arith(WN_kid1(wn_expr), &alias1);
      
      ai->Set_expr_kind(EXPR_IS_UNKNOWN);
      if (alias0.Expr_kind() == EXPR_IS_BEING_PROCESSED) {
	ai->Copy_fully(alias0);
      } else if (alias0.Expr_kind() == EXPR_IS_ADDR) {
	if (alias1.Expr_kind() == EXPR_IS_INT) {
	  ai->Copy_fully(alias0);
	  if (alias0.Ofst_kind() == OFST_IS_FIXED && alias1.Int_is_constant())
	    ai->Shift_ofst( - alias1.Int_const_val());
	  else
	    ai->Set_ofst_kind(OFST_IS_UNKNOWN);
	} else if (alias1.Expr_kind() == EXPR_IS_ADDR) {
	  ai->Copy_fully(alias0);
	  ai->Meet(&alias1, (ST *) wn_expr);
	}
      } else if (alias0.Expr_kind() == EXPR_IS_INT) {
	if (alias1.Expr_kind() == EXPR_IS_INT) {
	  ai->Set_expr_kind(EXPR_IS_INT);
	}
      }
      if (ai->Expr_kind() == EXPR_IS_UNKNOWN) {
	ai->Set_base_kind(BASE_IS_UNKNOWN);
	ai->Set_ofst_kind(OFST_IS_UNKNOWN);
      }
      CHECK_POINTS_TO(ai);
    }
    break;

    // assume these operators always an integer (not an address)
  case OPR_ABS:
  case OPR_SQRT:
  case OPR_RSQRT:
  case OPR_RECIP:
  case OPR_REALPART:
  case OPR_IMAGPART:
  case OPR_RND:
  case OPR_TRUNC:
  case OPR_CEIL:
  case OPR_FLOOR:
  case OPR_BNOT:
  case OPR_LNOT:
  case OPR_COMPLEX:
  case OPR_MPY:   
  case OPR_DIV:
  case OPR_MOD:
  case OPR_REM:
  case OPR_MAX:
  case OPR_MIN:
  case OPR_EQ:
  case OPR_NE:
  case OPR_GE:
  case OPR_GT:
  case OPR_LE:
  case OPR_LT:
  case OPR_BAND:
  case OPR_BIOR:
  case OPR_BXOR:
  case OPR_LAND:
  case OPR_LIOR:
  case OPR_SHL:
  case OPR_ASHR:
  case OPR_LSHR:
  case OPR_MADD:
  case OPR_MSUB:
  case OPR_NMADD:
  case OPR_NMSUB:
  case OPR_CVT:
  case OPR_CVTL:
    ai->Set_expr_kind(EXPR_IS_INT);
    ai->Set_base_kind(BASE_IS_UNKNOWN);
    ai->Set_ofst_kind(OFST_IS_UNKNOWN);
    CHECK_POINTS_TO(ai);
    break;

  default:
    // no useful information
    ai->Set_expr_kind(EXPR_IS_UNKNOWN);
    ai->Set_base_kind(BASE_IS_UNKNOWN);
    ai->Set_ofst_kind(OFST_IS_UNKNOWN);
    CHECK_POINTS_TO(ai);
    break;
  }
}



//  Follow the DU Chain to expand the pointer expresssion.
//
void OPT_STAB::Simplify_Pointer_Ver(VER_ID ver, POINTS_TO *ai)
{
  INT32 vtype = Ver_stab_entry(ver)->Type();
  POINTS_TO  *pt = Ver_stab_entry(ver)->Points_to();
  
  //  There is already some information associated with this version.
  //  Simply return it.
  if (pt) {
    ai->Copy_fully(pt);
    CHECK_POINTS_TO(ai);
    return;
  }

  //  Figure it out now.
  //
  switch (vtype) {
  case ENTRY_STMT:
    {
      if ( Is_volatile(Ver_stab_entry(ver)->Aux_id()) ) {
	// the pointer is itself volatile, so we don't know what
	// it points to.
	ai->Set_expr_kind(EXPR_IS_ADDR);
	ai->Set_base_kind(BASE_IS_DYNAMIC);
	ai->Set_ofst_kind(OFST_IS_FIXED);
	ai->Set_byte_ofst(0);
	ai->Set_byte_size(0);
	ai->Set_bit_ofst_size(0, 0);
	ai->Set_base((ST*)ver);
      }
      else {
	ai->Set_expr_kind(EXPR_IS_ADDR);
	ai->Set_base_kind(BASE_IS_DYNAMIC);
	ai->Set_ofst_kind(OFST_IS_FIXED);
	ai->Set_byte_ofst(0);
	ai->Set_byte_size(0);
	ai->Set_bit_ofst_size(0, 0);
	ai->Set_global();
	ai->Set_base((ST*)ver);
      }
    }
    break;
  case WHIRL_STMT:
    {
      WN *def = Ver_stab_entry(ver)->Wn();
      if (def) {
	FmtAssert ( WN_operator(def) == OPR_STID,
		    ("Simplify_Pointer_Ver: def must be STID.") );
	pt = CXX_NEW(POINTS_TO, &_ver_pool);
	pt->Init();
	pt->Set_expr_kind(EXPR_IS_BEING_PROCESSED);
	Ver_stab_entry(ver)->Set_points_to(pt);
	Simplify_Pointer(WN_kid0(def), ai);
	pt->Copy_fully(ai);
	// reset pt IS_BEING_PROCESSED
	if (pt->Expr_kind() == EXPR_IS_BEING_PROCESSED) {
	  pt->Set_expr_kind(EXPR_IS_UNKNOWN);
	  CHECK_POINTS_TO(pt);
	}
      }
    }
    break;
  case PHI_STMT:
    {
      // if the information from all opnds (excluding the result) agree,
      //   then we can use them
      INT32 in_degree = Ver_stab_entry(ver)->Bb()->Pred()->Len();
      BB_NODE *bb = Ver_stab_entry(ver)->Bb();
      PHI_NODE *phi = Ver_stab_entry(ver)->Phi();
      VER_ID opnd_vid;
      POINTS_TO *opnd_pt, pt2;
      POINTS_TO summary_pt;
    
      // allocate a POINTS_TO to the PHI function
      pt = CXX_NEW(POINTS_TO, &_ver_pool);
      pt->Init();
      pt->Set_expr_kind(EXPR_IS_BEING_PROCESSED);
      Ver_stab_entry(ver)->Set_points_to(pt);

      // Summarize the PHI opnds into the result
      summary_pt.Init();
      summary_pt.Set_expr_kind(EXPR_IS_ANY);
      BB_NODE *pred;
      BB_LIST_ITER bb_iter;
      INT32 i = -1;
      FOR_ALL_ELEM(pred, bb_iter, Init(bb->Pred())) {
	i++;
	opnd_vid = phi->Opnd(i);
	opnd_pt = Ver_stab_entry(opnd_vid)->Points_to();
	if (opnd_pt == NULL) {  // if opnd is not analyzed, analyze it now.
	  pt2.Init();
	  Simplify_Pointer_Ver(opnd_vid, &pt2);
	  opnd_pt = &pt2;
	} 

	// Only accepts EXPR_IS_BEING_PROCESSED for the phi opnds defined
	// in BBs dominated by the phi result.  This guarantees
	// EXPR_IS_BEING_PROCESSED is resolved in the dominator tree order
	// without the need to iterate.
	// 
	if (opnd_pt->Expr_kind() == EXPR_IS_BEING_PROCESSED &&
	    ! bb->Dominates(pred)) {
	      opnd_pt->Set_expr_kind(EXPR_IS_UNKNOWN);
	      opnd_pt->Set_base_kind(BASE_IS_UNKNOWN);
	      opnd_pt->Set_ofst_kind(OFST_IS_UNKNOWN);
	      opnd_pt->Reset_attr();
	    }
	// summarize the info into the result field.
	summary_pt.Meet(opnd_pt, (ST *) phi);

	// once we determine the POINTS_TO is not some address or we have
	// no idea what the base could be, GIVE UP!
	if (summary_pt.Expr_kind() != EXPR_IS_ANY && 
	    (summary_pt.Expr_kind() != EXPR_IS_ADDR ||
	     (summary_pt.Expr_kind() == EXPR_IS_ADDR
	      && summary_pt.Base_kind() == BASE_IS_UNKNOWN))) {
	  summary_pt.Reset_attr();
	  break;
	}
      }
      if (summary_pt.Expr_kind() == EXPR_IS_ANY ||
	  summary_pt.Expr_kind() == EXPR_IS_BEING_PROCESSED) {
	Warn_todo("analyze why it is EXPR_IS_ANY / EXPR_IS_BEING_PROCESSED.");
	summary_pt.Set_expr_kind(EXPR_IS_UNKNOWN);
	summary_pt.Set_base_kind(BASE_IS_UNKNOWN);
	summary_pt.Set_ofst_kind(OFST_IS_UNKNOWN);
	summary_pt.Reset_attr();
      }
      // Copy the result into the return value.
      pt->Copy_fully(summary_pt);
      ai->Copy_fully(summary_pt);
      
      if ( Get_Trace(TP_GLOBOPT, ALIAS_DUMP_FLAG)) {
	BB_NODE *pred;
	BB_LIST_ITER bb_iter;
	INT32 i = -1;
	FOR_ALL_ELEM(pred, bb_iter, Init(bb->Pred())) {
	  i++;
	  fprintf(TFile, "   phi opnd %d  BB %d  ver %d: ",
		  i, pred->Id(), phi->Opnd(i));
	  if (Ver_stab_entry(phi->Opnd(i))->Points_to())
	    Ver_stab_entry(phi->Opnd(i))->Points_to()->Print(TFile);
	  else
	    fprintf(TFile, " no points to.\n");
	}
	fprintf(TFile, "phi result ver %d BB %d aux %d: ", ver, bb->Id(), 
		Ver_stab_entry(ver)->Aux_id());
	ai->Print(TFile);
      }
    }
    break;
  case CHI_STMT:
    // Bypass the CHI_STMT if the chi statement is deleted.
    if (Ver_stab_entry(ver)->Synonym()) {
      Simplify_Pointer_Ver( Ver_stab_entry(ver)->Synonym(), ai);
    } else {
      CHI_NODE *chi = Ver_stab_entry(ver)->Chi();
      ai->Set_expr_kind(EXPR_IS_ADDR);
      ai->Set_base_kind(BASE_IS_DYNAMIC);
      ai->Set_ofst_kind(OFST_IS_FIXED);
      ai->Set_byte_ofst(0);
      ai->Set_byte_size(0);
      ai->Set_bit_ofst_size(0, 0);
      ai->Set_base( (ST *) chi );
    }
    break;
  default:
    Warn_todo("unknown ver type");
  }  

  if ( Get_Trace(TP_GLOBOPT, ALIAS_DUMP_FLAG)) {
    fprintf(TFile, "ver %d aux %d: ", ver, Ver_stab_entry(ver)->Aux_id());
    ai->Print(TFile);
  }
  CHECK_POINTS_TO(ai);
}


//  Simplify a pointer expression
//
void OPT_STAB::Simplify_Pointer(WN *wn_addr, POINTS_TO *ai)
{
  OPERATOR opr = WN_operator(wn_addr);
  switch (opr) {
    
  case OPR_ARRAY:
    Simplify_Pointer(WN_kid0(wn_addr), ai);
    Analyze_Range(wn_addr, ai);
    break;
  case OPR_LDA:
    ai->Analyze_Lda_Base( wn_addr, *this);
    break;
  case OPR_LDBITS:
  case OPR_LDID:
    if (FSA()) {
      VER_ID ver = WN_ver(wn_addr);
      AUX_ID aux = Ver_stab_entry(ver)->Aux_id();
      ST *st = Aux_stab_entry(aux)->St();
      TY_IDX ty = ST_type(st);
      
      // Fix 623777:  allow propagation of restricted attribute.
      // Use the first restrict pointer on the use-def chain.
      if (TY_is_restrict(ty)) {
	ai->Analyze_ST_as_base(st, WN_offset(wn_addr), WN_ty(wn_addr));
	ai->Set_ofst_kind(OFST_IS_UNKNOWN);
      } else
	Simplify_Pointer_Ver(ver, ai);    // Follow the DU chain
    } else if (FFA()) {
      ai->Analyze_Ldid_Base(wn_addr, *this);
    }
    break;
  case OPR_ADD:  
  case OPR_SUB: 
  case OPR_NEG:
    Simplify_Pointer_Arith(wn_addr, ai);
    break;
  case OPR_CVT:
  case OPR_PAREN:
    Simplify_Pointer(WN_kid0(wn_addr), ai);
    break;
  case OPR_ILOAD:
  case OPR_ILDBITS:
    ai->Set_expr_kind(EXPR_IS_ADDR);
    ai->Set_base_kind(BASE_IS_DYNAMIC);
    ai->Set_ofst_kind(OFST_IS_FIXED);
    ai->Set_byte_ofst(0);
    ai->Set_byte_size(0);
    ai->Set_bit_ofst_size(0, 0);
    ai->Set_base( (ST *) wn_addr );
    break;
  default:     // no useful information
    ai->Set_expr_kind(EXPR_IS_UNKNOWN);
    ai->Set_base_kind(BASE_IS_UNKNOWN);
    ai->Set_ofst_kind(OFST_IS_UNKNOWN);
    break;
  }     
  CHECK_POINTS_TO(ai);
}


// ======================================================================
//
// Analyze_Base_Flow_Free performs flow free alias analysis on ILOAD,
// ISTORE, MLOAD, and MSTORE expressions.  It is invoked only by
// Compute_FSA_stmt_or_expr below.
//
// Analyze_Base_Flow_Sensitive performs flow sensative alias analysis
// on ILOAD, ISTORE, MLOAD, and MSTORE expressions.  It is invoked only
// by Allocate_mu_chi_and_virtual_var below.
//
// Collect_f90_pointer_info is a helper procedure invoked only by
// Analyze_Base_Flow_Free.  It determines whether or not to set the
// POINTS_TO flags PT_ATTR_F90_POINTER and PT_ATTR_NOT_F90_POINTER.
//
// ======================================================================


void
OPT_STAB::Collect_f90_pointer_info(POINTS_TO *pt, const WN *wn)
{
  Is_True(OPERATOR_is_scalar_iload (WN_operator(wn)) ||
	  WN_operator(wn) == OPR_MLOAD ||
	  OPERATOR_is_scalar_istore (WN_operator(wn)) ||
	  WN_operator(wn) == OPR_MSTORE,
	  ("OPT_STAB::Collect_f90_pointer_info: "
	   "illegal opcode"));

  TY_IDX addr_ty;

  switch (WN_operator(wn)) {
  case OPR_ILOAD:
  case OPR_ILDBITS:
    addr_ty = WN_load_addr_ty(wn);
    break;
  default:
    addr_ty = WN_ty(wn);
    break;
  }
  if (TY_is_f90_pointer(Ty_Table[addr_ty])) {
    pt->Set_known_f90_pointer();
  }
  else {
    pt->Set_known_not_f90_pointer();
  }
}


// Analyze indirect load/store with fixed bases.
//
void OPT_STAB::Analyze_Base_Flow_Free(POINTS_TO *pt, WN *wn)
{
  pt->Set_expr_kind(EXPR_IS_ADDR);
  pt->Set_base_kind(BASE_IS_UNKNOWN);
  pt->Set_ofst_kind(OFST_IS_UNKNOWN);
  pt->Reset_attr();
  switch (WN_operator(wn)) {
  case OPR_ILDBITS:
  case OPR_ILOAD:
  case OPR_MLOAD:
    Simplify_Pointer(WN_kid0(wn), pt);
    break;
  case OPR_ISTORE:
  case OPR_ISTBITS:
  case OPR_MSTORE:
    Simplify_Pointer(WN_kid1(wn), pt);
    break;
  case OPR_ILOADX:
  case OPR_ISTOREX:
    FmtAssert ( FALSE, ("ILOADX/ISTOREX not handled.") );
  }
  pt->Shift_ofst(WN_offset(wn));
  pt->Lower_to_base(wn);
  pt->Set_ty(WN_object_ty(wn));
  Collect_f90_pointer_info(pt, wn);
  Update_From_Restricted_Map(wn, pt);
}


// Analyze indirect load/store with var bases.
//  -- depending on information on DU-chain
//
void OPT_STAB::Analyze_Base_Flow_Sensitive(POINTS_TO *pt, WN *wn)
{
  POINTS_TO ai;
  ai.Init();
  ai.Set_expr_kind(EXPR_IS_ADDR);
  ai.Set_base_kind(BASE_IS_UNKNOWN);
  ai.Set_ofst_kind(OFST_IS_UNKNOWN);

  switch (WN_operator(wn)) {
  case OPR_ILDBITS:
  case OPR_ILOAD:
  case OPR_MLOAD:
    Simplify_Pointer(WN_kid0(wn), &ai);
    if (ai.Expr_kind() == EXPR_IS_ADDR) {
      if (ai.Base_kind() != BASE_IS_UNKNOWN) {
	pt->Set_expr_kind(EXPR_IS_ADDR);
	pt->Set_base_kind(ai.Base_kind());
	pt->Set_ofst_kind(ai.Ofst_kind());
	pt->Set_base(ai.Base());
	pt->Set_byte_ofst(ai.Byte_Ofst());
	pt->Set_byte_size(ai.Byte_Size());
	// Bit ofst/size should be 0 (assert?)
	pt->Set_bit_ofst_size(ai.Bit_Ofst(), ai.Bit_Size());
	pt->Set_attr(ai.Attr());
	pt->Shift_ofst(WN_offset(wn));
	pt->Lower_to_base(wn);
      } else if (ai.Restricted()) {
	pt->Set_expr_kind(EXPR_IS_ADDR);
	pt->Set_restricted();
	pt->Set_based_sym(ai.Based_sym());
      }
    }
    break;
  case OPR_ISTORE:
  case OPR_ISTBITS:
  case OPR_MSTORE:
    Simplify_Pointer(WN_kid1(wn), &ai);
    if (ai.Expr_kind() == EXPR_IS_ADDR) {
      if (ai.Base_kind() != BASE_IS_UNKNOWN) {
	pt->Set_expr_kind(EXPR_IS_ADDR);
	pt->Set_base_kind(ai.Base_kind());
	pt->Set_ofst_kind(ai.Ofst_kind());
	pt->Set_base(ai.Base());
	pt->Set_byte_ofst(ai.Byte_Ofst());
	pt->Set_byte_size(ai.Byte_Size());
	pt->Set_bit_ofst_size(ai.Bit_Ofst(), ai.Bit_Size());
	pt->Set_attr(ai.Attr());
	pt->Shift_ofst(WN_offset(wn));
	pt->Lower_to_base(wn);
      } else  if (ai.Restricted()) {
	pt->Set_expr_kind(EXPR_IS_ADDR);
	pt->Set_restricted();
	pt->Set_based_sym(ai.Based_sym());
      }
    }
    break;
  case OPR_ILOADX:
  case OPR_ISTOREX:
    FmtAssert ( FALSE, ("ILOADX/ISTOREX not handled.") );
  }
  CHECK_POINTS_TO(pt);
}


// ======================================================================
//
// Analyze_Based_Pointer analyzes the pointer of the indirect load or
// store (useful if restricted alias is on).  It is called only by
// Compute_FSA_stmt_or_expr below.
//
// Find_Based_Pointer is a helper function invoked only by
// Analyze_Based_Pointer.  Find_Based_Pointer searches for the symbol
// from which the address for this OP is determined.  It returns the ST
// entry and the depth.
//
// ======================================================================


//  Simplify a pointer expression
//
ST *
OPT_STAB::Find_Based_Pointer(WN *wn, INT *depth)
{
  OPERATOR opr = WN_operator(wn);
  switch (opr) {
    
  case OPR_ARRAY:
    return Find_Based_Pointer(WN_kid0(wn), depth);

  case OPR_LDA:
    return NULL;  

  case OPR_LDBITS:
  case OPR_LDID:
    // If the LDID is a ST with zero offset.
    if (WN_offset(wn) == 0) {
      TY_IDX ty = WN_ty(wn);
      if (TY_kind(ty) == KIND_POINTER) {
	VER_ID ver_id = WN_ver(wn);
	AUX_ID aux_id = Ver_stab_entry(ver_id)->Aux_id();
	ST *st = St(aux_id);
	if (ST_sclass(st) != SCLASS_REG) {
	  *depth = 0;
	  return st;
	}
      }
    } else {
      VER_ID ver_id = WN_ver(wn);
      AUX_ID aux_id = Ver_stab_entry(ver_id)->Aux_id();
      ST *st = St(aux_id);
      if (ST_sclass(st) == SCLASS_REG &&
	  FSA() &&
	  WHIRL_STMT == Ver_stab_entry(ver_id)->Type()) {
	WN *def = Ver_stab_entry(ver_id)->Wn();
	if (def) {
	  FmtAssert ( OPERATOR_is_scalar_store (WN_operator(def)),
		      ("Find_Based_Pointer: def must be STID.") );
	  return Find_Based_Pointer(WN_kid0(def),depth);
	}
      }
    }
    return NULL;

  case OPR_ILOAD:
  case OPR_ILDBITS:
    if (Alias_Pointer_Disjoint) {
      ST *tmp_st = Find_Based_Pointer(WN_kid0(wn),depth);
      *depth = *depth + 1;
      return tmp_st;
    }
    return NULL;

  case OPR_ADD:  
  case OPR_SUB: 
    ST *based_pointer0, *based_pointer1;
    INT depth0, depth1;
    based_pointer0 = Find_Based_Pointer(WN_kid0(wn), &depth0);
    based_pointer1 = Find_Based_Pointer(WN_kid1(wn), &depth1);
    if (based_pointer0 && !based_pointer1) {
      *depth = depth0;
      return based_pointer0;
    }
    if (based_pointer1 && !based_pointer0) {
      *depth = depth1;
      return based_pointer1;
    }
    return NULL;

  default:     // no useful information
    return NULL;
  }     
}


// Analyze the pointer of the indirect load/store.
//   -- useful if restricted alias is on.
//
// This routine is called only during FSA.
//
void OPT_STAB::Analyze_Based_Pointer(POINTS_TO *pt, WN *addr_wn)
{
  Is_True(FSA(), ("Analyze_Based_Pointer is FSA-specific"));

  if (!Rule()->Rule_enabled(RAG_RESTRICTED_RULE) &&
      !Rule()->Rule_enabled(RAG_PARMS_RULE))
    return;
 
  // has sufficient information, restrict rule is useless.
  if ( // pt->Base_kind() == BASE_IS_FIXED ||
      pt->F_param() ||
      pt->Restricted() ||
      pt->Unique_pt())
    return;

  INT depth = 0;
 
  ST *st = Find_Based_Pointer(addr_wn, &depth);
  if (st != NULL && !ST_is_temp_var(st))
    if (Rule()->Rule_enabled(RAG_RESTRICTED_RULE) ||
	Rule()->Rule_enabled(IBM_DISJOINT_RULE) ||
	Rule()->Rule_enabled(RAG_UNNAMED_RULE) ||
	ST_is_value_parm(st)) {
      pt->Set_based_sym(st);
      pt->Set_based_sym_depth(depth);
    }
}


// ======================================================================


// Create the defs list for a barrier from its IDNAME list
POINTS_TO_LIST*
OPT_STAB::Create_barrier_defs( WN *wn )
{
  INT32 is_dealloca = WN_operator(wn) == OPR_DEALLOCA;
  if (WN_kid_count(wn) == 0) return NULL;
  if (is_dealloca && WN_kid_count(wn) == 1) return NULL;

  POINTS_TO_LIST *ptl = CXX_NEW(POINTS_TO_LIST,mem_pool);

  for (INT i = is_dealloca; i < WN_kid_count(wn); i++) {
    POINTS_TO *points_to = CXX_NEW(POINTS_TO,mem_pool);
    points_to->Init();
    Simplify_Pointer(WN_kid(wn,i), points_to);
    points_to->Lower_to_base(NULL);
    ptl->Prepend( points_to, mem_pool );
  }
  return ptl;
}


// ======================================================================


static void Add_to_mu_chi(OCC_TAB_ENTRY *occ, AUX_ID aux_id,
			  MEM_POOL *mem_pool, BOOL no_dups, 
			  BOOL generate_mu, BOOL generate_chi)
{
  if (no_dups) {
    if (generate_mu)
      occ->Stmt_mu_list()->New_mu_node_no_dups(aux_id, mem_pool);
    if (generate_chi)
      occ->Stmt_chi_list()->New_chi_node_no_dups(aux_id, mem_pool);
  } else {
    if (generate_mu)
      occ->Stmt_mu_list()->New_mu_node(aux_id, mem_pool);
    if (generate_chi)
      occ->Stmt_chi_list()->New_chi_node(aux_id, mem_pool);
  }
}


/*************************************************************************

Semantics of BARRIER

(1) Barriers (both named and unnamed) do not affect variables that
are normally un-modifiable or not visible in the source program.
e.g.,
   constants or read-only variables,
   pregs,
   the index variable of do-loops enclosing the barrier,
   base address of SCLASS_FORMAL.

(2) Barriers have no effect on volatile variables.

(3) An unnamed barrier affects all variables,
including all local variables, global variables, formal parameters, ...
excluding the variables described in (1).

(3a) Unnamed barrier can be optimized by examining MP pragma that
describes which variables are private or shared.  Private variables are not
affected by unnamed barriers.  A MP pragma affects
the status of the STs list in the pragma, but it does not affect the status
of variable aliased through the dereference of the STs.  In other words,
there is no way to use a pragma to change the private/shared status of a
Fortran SCLASS_FORMAL parameter.

(3b) Automatic variables that is not marked ST_IS_SHARED_AUTO is LOCAL.
Such variables are not affected by unnamed barrier.

(4) A named barrier carries a list of lvalues.   The barrier affects all
variables that can be referenced through any of its lvalues.  It is an error
to put lvalue of any variable described in (1) as kid of any named barrier.
A named barrier is independent of MP pragma.

(5) A barrier does imply liveness.  In the following example, if x is
affected by the barrier, the optimizer is not allowed to delete STID x 
even if there is no other use of x in the procedure.    The reason is
that another thread of PU is executing and might reference the variable.

       INTCONST 0
     STID x
        LDA x
     BARRIER


Maps BARRIERS into use/def:

BARRIERs cannot be modelled accurately using use/def.
To prevent LOAD/STORE moving forward a FORWARD_BARRIER,
the FORWARD_BARRIER is modelled as a may-def. To force
the STORE to be homed before the FORWARD_BARRIER, the
FORWARD_BARRIER is modelled as a may-use.

Similiarly, a BACKWARD_BARRIER is modelled as a pair
of may-use and may-def.  That means in the optimizer,
there is no difference between FORWARD_BARRIER and
BACKWARD_BARRIER.

If the optimizer wants to allow code motion 
of a LOAD backward crossing a FORWARD_BARRIER affecting the LOAD,
the code motion phase needs to ignore the might-def of the FORWARD_BARRIER
as a special case.


************************************************************************/


BOOL 
OPT_STAB::Not_affected_by_barrier(AUX_ID aux, BB_NODE *bb)
{
  AUX_STAB_ENTRY *psym = &aux_stab[aux];
  
  if (IS_FORTRAN && Var_is_loop_index(aux, bb))
    return TRUE;

  // SCLASS_FORMAL are constant, but we cannot set the constant
  // flag because CG lowerer will generate an assignment to it!
  //   -Raymond 10/14/98  
  if (IS_FORTRAN && psym->St() &&
      ST_sclass(psym->St()) == SCLASS_FORMAL)  // base addr of formal
    return TRUE;
  
  // Volatile do not appear in any mu and chi
  if (psym->Is_volatile() && !psym->Is_virtual() )
    return TRUE;

  if (psym->Is_preg()) 
    return TRUE;

  POINTS_TO *pt = psym->Points_to();
  if (pt->Const())
    return TRUE;

  return FALSE;
}


// is_mp_barrier is TRUE for FORWARD_BARRIER and BACKWARD_BARRIER
// is_mp_barrier is FALSE for DEALLOCA
void
OPT_STAB::Compute_barrier_mu_chi( OCC_TAB_ENTRY *occ, POINTS_TO_LIST *defs,
				  BB_NODE *bb, BOOL no_dups, 
				  BOOL generate_mu, BOOL generate_chi, 
				  BOOL is_mp_barrier)
{
  AUX_STAB_ITER aux_stab_iter(this);
  AUX_ID auxid;

  // Fix 499285:  a barrier cannot ref/mod a loop index variable.
  // Fix 622174:  return_vsym and default_vsym must be included in
  // the chi list of BARRIER.  Update_iload_vsym relies on it to 
  // determine the current version.

  if (WOPT_Enable_Strong_Barrier) {
    
    // for debugging!

    FOR_ALL_NODE(auxid, aux_stab_iter, Init()) {

      if (Not_affected_by_barrier(auxid, bb))
	continue;
      
      Add_to_mu_chi(occ, auxid, mem_pool, no_dups, generate_mu, generate_chi);
    }
  } else if (defs != NULL) {
    // For named barriers
    //
    FOR_ALL_NODE(auxid, aux_stab_iter, Init()) {

      if (Not_affected_by_barrier(auxid, bb))
	continue;

      AUX_STAB_ENTRY *psym = &aux_stab[auxid];
      POINTS_TO *aux_pt = psym->Points_to();
      
      if (!is_mp_barrier) {
	// OPCODE is OPR_DEALLOCA:  dealloca does not affect named variables.
	if (aux_pt->Named())
	  continue;
      }

      // check all the definitions
      POINTS_TO_ITER def_iter;
      POINTS_TO_NODE *defn;
      FOR_ALL_NODE( defn, def_iter, Init(defs)) {
	if ( Rule()->Aliased_Memop( aux_pt, defn->Pt() ) ) {
	  Add_to_mu_chi(occ, auxid, mem_pool, no_dups, generate_mu, generate_chi);
	  break;
	}
      }
    }
  } else {
    FOR_ALL_NODE(auxid, aux_stab_iter, Init()) {

      if (Not_affected_by_barrier(auxid, bb))
	continue;

      AUX_STAB_ENTRY *psym = &aux_stab[auxid];
      POINTS_TO *aux_pt = psym->Points_to();

      if (!is_mp_barrier) {
	// OPCODE is OPR_DEALLOCA:  dealloca does not affect named variables.
	if (aux_pt->Named())
	  continue;
      } else {
	if (aux_pt->Local() && 
	    (psym->St() && !ST_is_shared_auto(*psym->St())) &&
	    (psym->St() && ST_sclass(psym->St()) != SCLASS_FORMAL_REF))
	  continue;
      }
	
      Add_to_mu_chi(occ, auxid, mem_pool, no_dups, generate_mu, generate_chi);
    }
  }
}


// ======================================================================


// Determine what other variables are referenced/defined by this
// io statement.  Add to the mu/chi lists.
//
void OPT_STAB::Compute_black_box_mu_chi( const WN *wn, OCC_TAB_ENTRY *occ)
{
  const OPCODE opc = WN_opcode(wn);

  Is_True( OPCODE_is_black_box(opc),
    ("OPT_STAB::Compute_black_box_mu_chi: Bad opcode: %s",
     OPCODE_name(opc)) );

  // does this black box affect anything?
  POINTS_TO_LIST *refs = Black_box_refs( wn );
  POINTS_TO_LIST *defs = Black_box_defs( wn );
  POINTS_TO_ITER ref_iter;
  POINTS_TO_ITER def_iter;
  POINTS_TO_NODE *refn, *defn;
  AUX_STAB_ITER aux_stab_iter(this);
  AUX_ID auxid;

  FOR_ALL_NODE(auxid, aux_stab_iter, Init()) {

    AUX_STAB_ENTRY *psym = &aux_stab[auxid];
    if ((psym->Is_real_var() && !psym->Is_volatile()) ||
	psym->Is_virtual()) {

      POINTS_TO *aux_pt = psym->Points_to();

      if (aux_pt->Not_addr_saved()) {
	// check all the references
	if ( refs != NULL ) FOR_ALL_NODE( refn, ref_iter, Init(refs)) {
	  if ( Rule()->Aliased_Memop( aux_pt, refn->Pt() ) ) {
	    occ->Stmt_mu_list()->New_mu_node(auxid, mem_pool);
	    break;
	  }
	}
	// check all the definitions
	if ( defs != NULL ) FOR_ALL_NODE( defn, def_iter, Init(defs)) {
	  if ( Rule()->Aliased_Memop( aux_pt, defn->Pt() ) ) {
	    occ->Stmt_chi_list()->New_chi_node(auxid, mem_pool);
	    break;
	  }
	}
      } else {
	// Fortran IO stmt has some side-effect on certain variable that
	// is not passed.  Those variable are marked addr_saved.
	// See 345192.
	occ->Stmt_mu_list()->New_mu_node(auxid, mem_pool);
	occ->Stmt_chi_list()->New_chi_node(auxid, mem_pool);
      } 
    }
  } // end loop through opt_stab
}


// ====================================================================
//
// Create_entry_chi creates the chi list (the OPR_OPT_CHI node) for the
// entry block ENTRY_BB.
//
// Create_entry_chi_stmt is invoked by Create_entry_chi to create and
// insert the OPR_OPT_CHI statement into the block.
//
// ====================================================================


//  Create the chi statement in this bb
WN *OPT_STAB::Create_entry_chi_stmt(BB_NODE *bb)
{
  STMT_CONTAINER stmt_con( bb->Firststmt(), bb->Laststmt());
  Is_True(bb->Kind() == BB_ENTRY || bb->Kind() == BB_REGIONSTART,
          ("OPT_STAB::Create_entry_chi_stmt: not a ENTRY_BB"));

  OPCODE opc = OPCODE_make_op(OPR_OPT_CHI, MTYPE_V, MTYPE_V );
  WN *wn = WN_Create(opc, 0);
  WN_Set_Linenum(wn, bb->Linenum()); // grab linenum of entry from entry bb
  Enter_occ_tab(wn, 0); // need mu for global ref and chi at this node.
  stmt_con.Prepend(wn);

  bb->Set_firststmt(stmt_con.Head());
  bb->Set_laststmt(stmt_con.Tail());
  return wn;
}


void
OPT_STAB::Create_entry_chi(void)
{
  CFG_ITER cfg_iter;
  BB_NODE *bb;

  FOR_ALL_ELEM (bb, cfg_iter, Init(Cfg())) {
    if ((bb->Kind() == BB_ENTRY && bb->Entrywn()) ||
	bb->Kind() == BB_REGIONSTART) {

      // Don't create entry CHI for mp regions or transparent regions
      // 	that are not entries.
      // rid is the rid for this bb
      // Cfg()->Rid() is the rid for the pu/region that contains entry point
      // NOTE: rid and _cfg->Rid() are the same if bb is the entry to the
      //	pu/region we are processing. They are different when we
      //	process a pu/region with another transparent region inside.
      if (bb->Kind() == BB_REGIONSTART) {
	RID *rid = bb->Regioninfo()->Rid();
	if (rid != Cfg()->Rid()) // rid is transparent or black box
	  continue;
      }

      WN *chi_stmt = Create_entry_chi_stmt(bb);
      CHI_LIST *chi = Get_stmt_chi_list(chi_stmt);
      AUX_STAB_ITER aux_stab_iter(this);
      AUX_ID auxid;
      FOR_ALL_NODE(auxid, aux_stab_iter, Init()) {
        AUX_STAB_ENTRY *sym = Aux_stab_entry(auxid);
	if ( !sym->Is_volatile() ) {
	  CHI_NODE *cnode = chi->New_chi_node(auxid, Occ_pool());
	  cnode->Set_opnd(auxid);
	  cnode->Set_result(auxid);
	  cnode->Set_live(TRUE);
	}
      }
    }
  }
}


// ======================================================================


// Compute mu and chi lists for a black box region (mu_and_chi=TRUE) or
// exit mu for regions being processed (mu_and_chi=FALSE --> just mu).
//
// One tricky part is that partial stack layout has been done by cg and
// so the POINTS_TOs we get now will have STs that are .SP based. The
// POINTS_TOs in the boundary set are the old ones before they are lowered
// and so we need to lower them here (Lower_to_base).
void
OPT_STAB::Compute_region_mu_chi(WN *wn, RID *rid, BOOL mu_and_chi, BB_NODE *bb)
{
  OCC_TAB_ENTRY *occ = Get_occ(wn);

  Is_True(occ != NULL, ("OPT_STAB::Compute_region_mu_chi, NULL occ"));
  Is_True(rid, ("OPT_STAB::Compute_region_mu_chi, NULL rid"));
  Is_True(!RID_TYPE_transparent(rid),
	  ("OPT_STAB::Compute_region_mu_chi, transparent region, RGN %d",
	   RID_id(rid)));
  if (mu_and_chi) {
    Is_True(REGION_consistency_check(wn), ("OPT_STAB::Compute_region_mu_chi"));
    Is_True(rid == REGION_get_rid(wn),
	    ("OPT_STAB::Compute_region_mu_chi, wrong RID"));
  }
  if (RID_bounds_exist(rid) == REGION_BOUND_UNKNOWN) // LNO's Preopt
    return;

  Is_Trace(_rgn_trace, (TFile,
		       "===== OPT_STAB::Compute_region_mu_chi, RGN %d %s\n",
		       RID_id(rid), mu_and_chi ? "(mu and chi)":"(mu only)"));
  Is_Trace_cmd(_rgn_trace, RID_set_print(TFile, rid));

  // call Lower_to_base to lower boundary sets
  POINTS_TO_SET *pt_tmp;
  for (pt_tmp=RID_used_in(rid); pt_tmp; pt_tmp=pt_tmp->Next)
    pt_tmp->Pt->Lower_to_base(NULL);
  for (pt_tmp=RID_def_in_live_out(rid); pt_tmp; pt_tmp=pt_tmp->Next)
    pt_tmp->Pt->Lower_to_base(NULL);

  // go through aux stab looking for references
  AUX_STAB_ITER aux_stab_iter(this);
  AUX_ID aux_id;
  FOR_ALL_NODE(aux_id, aux_stab_iter, Init()) {

    AUX_STAB_ENTRY *psym = &aux_stab[aux_id];

    // look at real vars (that are not volatile) and virtual vars
    if ((psym->Is_real_var() && !psym->Is_volatile()) || psym->Is_virtual()) {
      
      BOOL aliased = FALSE;

      POINTS_TO *pt = Points_to(aux_id);
      Is_True(pt == psym->Points_to(),
	      ("Compute_region_mu_chi, POINTS_TO problem"));
      ST *st = aux_stab[aux_id].St(); // use original ST, not lowered one

      if (st && ST_class(st) == CLASS_PREG) {

	if (REGION_search_preg_set(RID_pregs_in(rid),
				   aux_stab[aux_id].St_ofst())) {
	  aliased = TRUE;
	} else {
	  for (INT i=0; i<RID_num_exits(rid); i++) {
	    if (REGION_search_preg_set(RID_pregs_out_i(rid, i),
				       aux_stab[aux_id].St_ofst())) {
	      aliased = TRUE;
	      break;
	    }
	  }
	}

      } // if (st && ST_class(st) == CLASS_PREG)
      else { // not a preg

	if (!(pt->Restricted() || pt->Unique_pt() || pt->F_param())) {
	  aliased = (RID_aliased_to_indirects(rid) &&
		     Rule()->Aliased_with_Indirect(pt)) ||
		       (RID_aliased_to_globals(rid) &&
			Rule()->Aliased_with_Global(pt)) ||
			  (RID_contains_uplevel(rid) && st &&
			    Is_up_level_var(st));
	}
	if (!aliased) {
	  // if generating mu only, look only at exit list
	  if (mu_and_chi &&
	      REGION_search_set(RID_used_in(rid), comp_aliased(pt, Rule())))
	    aliased = TRUE;
	  if ((!aliased || !mu_and_chi) &&
	      REGION_search_set(RID_def_in_live_out(rid),
				comp_aliased(pt, Rule())))
	    aliased = TRUE;
	}
	  
      } // else part of if (st && ST_class(st) == CLASS_PREG)

      // if it is aliased, add to mu and chi lists
      if (aliased) {
	aux_stab[aux_id].Set_disable_local_rvi();

	Is_True(occ->Stmt_mu_list()->Search_mu_node(aux_id) == NULL,
		("Compute_region_mu_chi, duplicate mu node"));
	occ->Stmt_mu_list()->New_mu_node(aux_id, mem_pool);

	if (mu_and_chi) {
	  Is_True(occ->Stmt_chi_list()->Search_chi_node(aux_id) == NULL,
		("Compute_region_mu_chi, duplicate chi node"));
	  occ->Stmt_chi_list()->New_chi_node(aux_id, mem_pool);
	}

	Is_Trace(_rgn_trace, (TFile,
		    "===== Compute_region_mu_chi, RGN %d, adding "
		    "aux_id %d to %s\n", RID_id(rid), aux_id,
		    mu_and_chi ? "mu and chi" : "mu"));
      }

    } // if ((psym->Is_real_var() && !psym->Is_volatile()) ...

  } // FOR_ALL_NODE(aux_id, aux_stab_iter, Init())

  // check if the barrier bit is set
  // if so, add pretty much everything to mu and chi
  if (RID_contains_barrier(rid)) {
    Is_Trace(_rgn_trace, (TFile,
			  "OPT_STAB::Compute_region_mu_chi, found a barrier"));
    Compute_barrier_mu_chi(occ, NULL, bb, TRUE/*no dups*/, TRUE, mu_and_chi, TRUE);
  }

  // previously processed region contains a return, need exit mu
  if (mu_and_chi && RID_has_return(rid))
    Generate_exit_mu(wn);

  if (_rgn_trace) {
    Print(TFile);
    fprintf(TFile,"===== OPT_STAB::Compute_region_mu_chi, RGN %d, "
	    "mu result\n",RID_id(rid));
    occ->Stmt_mu_list()->Print(TFile);
    if (mu_and_chi) {
      fprintf(TFile,"===== OPT_STAB::Compute_region_mu_chi, RGN %d, "
	      "chi result\n",RID_id(rid));
      occ->Stmt_chi_list()->Print(TFile);
    }
  }
}


// ======================================================================
// NOTE: No other code appears to invoke the following four procedures
// ======================================================================


// add a POINTS_TO for the aux_id to the boundary sets for this rid
void
OPT_STAB::REGION_add_to_bound(RID *rid, AUX_ID aux_id, BOOL empty)
{
  // skip return vsym, note: do not skip default vsym
  if (aux_id == Return_vsym())
    return;

  ST *st = Points_to(aux_id)->Base();
  if (st == NULL) // virtual vars
    return;
  Is_True(st != NULL,
	  ("OPT_STAB::REGION_add_to_bound, NULL st"));

  if (st && ST_class(st) == CLASS_PREG) { // add a preg
    // REGION_add_preg checks for duplicates, creates one from
    // region pool if needed
    BOOL ret = REGION_add_preg_in(rid, Points_to(aux_id)->Byte_Ofst(),
				  ST_btype(st));
    for (INT i=0; i<RID_num_exits(rid); i++)
      ret |= REGION_add_preg_out(rid, i, Points_to(aux_id)->Byte_Ofst(),
				 ST_btype(st));
    Is_Trace(_rgn_trace && ret,(TFile,"===== OPT_STAB::REGION_add_to_bound "
		"(RGN %d), adding preg aux_id %d\n",RID_id(rid),aux_id));
  } else {
    if (empty) { // create a new one
      REGION_add_aux_id_points_to(&RID_used_in(rid), aux_id);
      REGION_add_aux_id_points_to(&RID_def_in_live_out(rid), aux_id);
      Is_Trace(_rgn_trace,(TFile,
	   "===== OPT_STAB::REGION_add_to_bound (RGN %d, empty=T), "
	   "adding vir var aux_id %d\n",RID_id(rid),aux_id));
    } else { // check for duplicates first
      BOOL ret = REGION_merge_aux_id_points_to(&RID_used_in(rid), aux_id);
      ret |= REGION_merge_aux_id_points_to(&RID_def_in_live_out(rid), aux_id);
      Is_Trace(_rgn_trace && ret, (TFile,
	   "===== OPT_STAB::REGION_add_to_bound (RGN %d, empty=F), "
	   "adding vir var aux_id %d\n",RID_id(rid),aux_id));
    }
  }
}


// verify a POINTS_TO for the aux_id is in the boundary sets for this rid
// based on OPT_TAB::REGION_add_to_bound()
// called only when Is_True_On is defined.
// returns TRUE if everything is ok, FALSE or asserts if not ok.
BOOL
OPT_STAB::REGION_verify_bound(RID *rid, AUX_ID aux_id)
{
  // if we are in Preopt there are no boundaries yet, PV 457243
  if (!RID_bounds_exist(rid))
    return TRUE;

  // can't really check anything if these bits are set
  if (RID_aliased_to_indirects(rid) || RID_aliased_to_globals(rid))
    return TRUE;

  // skip all variables that are not real
  AUX_STAB_ENTRY *psym = Aux_stab_entry(aux_id);
  if (!psym->Is_real_var())
    return TRUE;

  ST *st = Points_to(aux_id)->Base();
  if (st == NULL) // virtual vars
    return TRUE;
  Is_True(st != NULL, ("OPT_STAB::REGION_verify_bound, NULL st"));

  if (st && ST_class(st) == CLASS_PREG) { // pregs
#if 1
    // ignore all pregs, they can be created by IVR or goto conversion
    return TRUE;
#else
    PREG_NUM pr = psym->St_ofst();

    // ignore all dedicated pregs, they come from black regions
    if (pr < 72)
      return TRUE;

    Is_True(REGION_search_preg_set(RID_pregs_in(rid), pr),
	 ("OPT_STAB::REGION_verify_bound, PREG %d not in preg-in set, RGN %d",
	  pr,RID_id(rid)));
    if (RID_pregs_out(rid)) {
      for (INT i=0; i<RID_num_exits(rid); i++)
	Is_True(REGION_search_preg_set(RID_pregs_out_i(rid, i), pr),
	  ("OPT_STAB::REGION_verify_bound, PREG %d not in preg-in set, RGN %d",
		 pr,RID_id(rid)));
    }
#endif
  } else { // variables

    // if const LDA, ignore
    if (Points_to(aux_id)->Const())
      return TRUE;

    // if the aliased to global bit it set then ignore globals
    if (Points_to(aux_id)->Global() && RID_aliased_to_globals(rid))
      return TRUE;

    // it better be in the boundary set
    Is_True(REGION_search_set(RID_used_in(rid),
			      comp_same_pt(Points_to(aux_id), Rule())),
       ("OPT_STAB::REGION_verify_bound, AUX_ID %d not in live-in set, RGN %d",
	aux_id, RID_id(rid)));
    Is_True(REGION_search_set(RID_def_in_live_out(rid),
			      comp_same_pt(Points_to(aux_id), Rule())),
       ("OPT_STAB::REGION_verify_bound, AUX_ID %d not in live-out set, RGN %d",
	aux_id, RID_id(rid)));
  }
  return TRUE;
}


// create a POINTS_TO_SET from an aux_id and add it to a region boundary set
void
OPT_STAB::REGION_add_aux_id_points_to(POINTS_TO_SET **pset, AUX_ID aux_id)
{
  POINTS_TO_SET *ptr = TYPE_OPT_POOL_ALLOC(POINTS_TO_SET,&REGION_mem_pool,-1);
  ptr->Pt = Points_to_copy(Points_to(aux_id), &REGION_mem_pool);
  ptr->Next = *pset;	// prepend to set
  *pset = ptr;
}


// create a POINTS_TO_SET from an aux_id and add it to a region boundary set
// check for duplicates, returns TRUE if it added a new one
BOOL
OPT_STAB::REGION_merge_aux_id_points_to(POINTS_TO_SET **pset, AUX_ID aux_id)
{
  // first compare to existing boundary set
  if (REGION_search_set(*pset, comp_same_pt(Points_to(aux_id), Rule())))
    return FALSE;

  // add to boundary set
  REGION_add_aux_id_points_to(pset, aux_id);
  return TRUE;
}


// ======================================================================


// track list of values referenced (for RVI)
//
void
OPT_STAB::Update_aux_id_list(AUX_ID vp_idx) 
{
  // update the aux_id list for RVI 

  const BS *alias_set = Rule()->Alias_Set_Indirect(this); // all scalars + virtuals
  AUX_ID_LIST *alist = CXX_NEW(AUX_ID_LIST, mem_pool);
  alist->Clear();
  aux_stab[vp_idx].Set_aux_id_list(alist);
  for (AUX_ID idx = BS_Choose( alias_set );
       idx != (AUX_ID) BS_CHOOSE_FAILURE;
       idx = BS_Choose_Next ( alias_set, idx )) {
	 // exclude non-real variables
	 if ( ! Is_real_var(idx) ) continue;

	 if (Rule()->Aliased_Memop(aux_stab[vp_idx].Points_to(), 
				   aux_stab[idx].Points_to())) 
	   alist->New_aux_id_node(idx, mem_pool);
       }
}


// ======================================================================
//
// Allocate_mu_chi_and_virtual_var traverses the subtree rooted at wn
// and generates new entries in the occurrence table for appropriate
// WNs, and generates virtual symbols (vsyms) for indirect loads and
// stores.
//
// Adjust_vsym uses the POINTS_TO information in occ to update the vsym
// with index vp_idx.  This may require greating a new vsym.  The index
// of the resulting vsym is returned.  Adjust_vsym is a helper procedure
// for Allocate_mu_chi_and_virtual_var.
//
// ======================================================================


AUX_ID
OPT_STAB::Adjust_vsym(AUX_ID vp_idx, OCC_TAB_ENTRY *occ)
{
  POINTS_TO *pt = aux_stab[vp_idx].Points_to();
  BOOL use_default_vsym = FALSE;

  // Special handling for Unique_pt() and Restricted() here. Return
  // a vsym based on the Based_sym() of occ->Points_to(), and don't
  // call POINTS_TO::Meet in this sticky case. Do we need to do
  // anything here for F_param()?
  if ((WOPT_Enable_Unique_Pt_Vsym &&
       occ->Points_to()->Unique_pt()) ||
      occ->Points_to()->Restricted()) {
    Is_True(vp_idx != Default_vsym(),
	    ("Adjust_vsym: Default vsym must not be vsym for "
	     "Unique_pt() or Restricted()"));
    Is_True(pt->Based_sym() == occ->Points_to()->Based_sym(),
	    ("Adjust_vsym: unique vsym should have been set up by "
	     "Indentify_vsym"));
    if (pt->Based_sym() == occ->Points_to()->Based_sym()) {
      return vp_idx;
    }
    else {
#if 0
      vp_idx = Find_vsym_with_base(occ->Based_sym());
      if (vp_idx == NULL) {
	vp_idx = Create_vsym(EXPR_IS_ANY);
	AUX_STAB_ENTRY *vsym = Aux_stab_entry(vp_idx);
	vsym->Points_to()->Set_based_sym(occ->Based_sym());
	// What about vsym->Set_stype(something) here?
      }
#else
      // Replace the Based_sym() in the AUX_STAB_ENTRY with the
      // Based_sym() of the OCC_TAB_ENTRY. This should happen only
      // once, and we should never switch it back!
      pt->Set_based_sym(occ->Points_to()->Based_sym());
#endif
      return vp_idx;
    }
  }

  if (vp_idx != Default_vsym()) {
    if (pt->Expr_kind() != EXPR_IS_ANY &&
	pt->Base_kind() != occ->Points_to()->Base_kind() &&
	(pt->Based_sym() == NULL ||
	 pt->Based_sym() != occ->Points_to()->Based_sym())) 
      use_default_vsym = TRUE;
    else if (aux_stab[vp_idx].Is_real_var()) {
      // check for out-of-bound array access of a scalar.  PV 378384.
      POINTS_TO tmp_pt;
      tmp_pt.Init();
      tmp_pt.Copy_fully(pt);
      tmp_pt.Meet(occ->Points_to(), NULL);
      if (tmp_pt.Expr_kind() != EXPR_IS_ADDR    ||
	  tmp_pt.Base_kind() != BASE_IS_FIXED   ||
	  tmp_pt.Ofst_kind() != OFST_IS_FIXED   ||
	  tmp_pt.Base()      != pt->Base()      ||
	  tmp_pt.Byte_Ofst() != pt->Byte_Ofst() ||
	  tmp_pt.Byte_Size() != pt->Byte_Size() ||
	  tmp_pt.Bit_Ofst()  != pt->Bit_Ofst()  ||
	  tmp_pt.Bit_Size()  != pt->Bit_Size())
	use_default_vsym = TRUE;
    }
    if (use_default_vsym) {
      if (Default_vsym() == 0) {
	Set_default_vsym(Create_vsym(EXPR_IS_ANY));
	// Update the POINTS_TO with default vsym info
	Aux_stab_entry(Default_vsym())->Points_to()->Set_default_vsym();
      }
      vp_idx = Default_vsym();
      occ->Set_aux_id(vp_idx);
      pt = aux_stab[vp_idx].Points_to();
    }
  }

  // update alias info for the virtual variable
  pt->Meet(occ->Points_to(), NULL);

  // always preserve the default_vsym attribute
  if (vp_idx == Default_vsym())
    pt->Set_default_vsym();

  Is_True(vp_idx == Default_vsym() || pt->Base_is_fixed() ||
	  pt->Based_sym() || aux_stab[vp_idx].Unique_vsym(),
	  ("OPT_STAB::Adjust_vsym: base is disrupted."));

  return vp_idx;
}


//  Flow Free Alias Analysis for the STMT and its sub-tree.
//
void OPT_STAB::Allocate_mu_chi_and_virtual_var(WN *wn, BB_NODE *bb)
{
  if (wn == NULL)
    return;

  const OPCODE   opc = WN_opcode(wn);
  const OPERATOR opr = OPCODE_operator(opc);
  Is_True(!OPCODE_is_scf(opc) || opc == OPC_REGION,
	  ("OPT_STAB::Allocate_mu_chi_and_virtual_var: Wn is SCF %s",
	   OPCODE_name(opc)));
  Is_True(opc != OPC_BLOCK, ("Wn is a OPR_BLOCK."));

  AUX_ID vp_idx;
  OCC_TAB_ENTRY *occ;

  switch ( opr ) {
  case OPR_LDBITS:
  case OPR_LDID:
    break;
    
  case OPR_STID:     // ST in the same alias group are affected
  case OPR_STBITS:
    if (!aux_stab[WN_aux(wn)].Points_to()->No_alias()) {
      occ = Enter_occ_tab(wn, WN_aux(wn));
      // The following was Copy_non_sticky_info
      occ->Points_to()->Copy_fully(aux_stab[WN_aux(wn)].Points_to());
    }
    break;

  // From a conversation with Raymond, here is how the following stuff
  // should really be done: First we should call
  // Analyze_base_flow_free to fill in the POINTS_TO with any
  // Base/Based_sym information, and then we should call Identify_vsym
  // to determine the vsym. The functionality of Adjust_vsym should be
  // combined with Identify_vsym. -- RK 981021

  case OPR_ILOAD:
  case OPR_ILDBITS:
  case OPR_MLOAD:
    vp_idx = Identify_vsym(wn);
    occ = Enter_occ_tab(wn, vp_idx);
    Analyze_Base_Flow_Free(occ->Points_to(), wn);
    vp_idx = Adjust_vsym(vp_idx, occ);
    break;
    
  case OPR_ISTORE:
  case OPR_ISTBITS:
  case OPR_MSTORE:
    vp_idx = Identify_vsym(wn);
    occ = Enter_occ_tab(wn, vp_idx);
    Analyze_Base_Flow_Free(occ->Points_to(), wn);
    vp_idx = Adjust_vsym(vp_idx, occ);
    break;
    
  case OPR_ICALL:
  case OPR_CALL:	// ST that are addr_taken or external affected
  case OPR_INTRINSIC_CALL:
    Enter_occ_tab(wn, 0); // need mu for global ref and chi at this node.
    break;

  case OPR_INTRINSIC_OP:
    // no more mu-list for INTRINSIC_OP
    break;

  case OPR_PARM:
    if ( WN_Parm_By_Reference(wn) ) {
      vp_idx = Identify_vsym(wn);
      occ = Enter_occ_tab(wn, vp_idx); // treat OPR_PARM as ILOAD
      Is_True(WN_kid0(wn) != NULL, ("bad OPR_PARM node."));
      occ->Points_to()->Analyze_Parameter_Base(WN_kid0(wn), *this);
      Update_From_Restricted_Map(wn, occ->Points_to());
      // Hack to get around the fact that Analyze_Parameter_Base
      // (usually) reinitializes the POINTS_TO...
      Is_True(occ->Points_to()->Alias_class() == OPTIMISTIC_AC_ID ||
	      occ->Points_to()->Alias_class() == Alias_classification()->Alias_class(wn) ||
	      (occ->Points_to()->Alias_class() == PESSIMISTIC_AC_ID &&
	       WOPT_Alias_Class_Limit != UINT32_MAX),
	      ("Allocate_mu_chi_and_virtual_var: Alias class %ld for PARM "
	       "not consistent (%ld).",
	       occ->Points_to()->Alias_class(),
	       Alias_classification()->Alias_class(wn)));
      occ->Points_to()->Set_alias_class(Alias_classification()->Alias_class(wn));
      vp_idx = Adjust_vsym(vp_idx, occ);
    } else {
      Is_True( WN_Parm_By_Value(wn),
	("OPT_STAB::Allocate_mu_chi_and_virtual_var: not by value") );
    }
    break;
    
  case OPR_IO:		// a "black-box" operator
  case OPR_FORWARD_BARRIER:
  case OPR_BACKWARD_BARRIER:
  case OPR_DEALLOCA:
  case OPR_PREFETCH:
  case OPR_RETURN:
  case OPR_RETURN_VAL:
    Enter_occ_tab(wn, 0);
    break;

  case OPR_REGION: 	// init occ tab entry for mu/chi of black box region
    RID *rid;
    rid = REGION_get_rid(wn);
    if (RID_level(rid) >= Rgn_level()) {
      Enter_occ_tab(wn, 0);
      return;		// don't visit rest of black box
    }
    break;

  case OPR_REGION_EXIT:	// look for region exit goto
    Enter_occ_tab(wn,0);
    break;

  case OPR_ASM_STMT:
    Enter_occ_tab(wn,0);
    break;
    
  default:
    break;
  }

  // Traverse the children of wn
  if ( opc == OPC_COMPGOTO ) {
    // only first kid is important, others are control-flow stuff
    Allocate_mu_chi_and_virtual_var(WN_kid(wn,0),bb);
  }
  else if (!OPCODE_is_black_box(opc)) {
    INT32 i = (opr == OPR_ASM_STMT ? 2 : 0);
    for (; i < WN_kid_count(wn); i++)
      Allocate_mu_chi_and_virtual_var(WN_kid(wn,i),bb);
  }
}


// ======================================================================
//
// Generate_mu_and_chi_list generates the list of MU and CHI nodes for
// each WN in the subtree rooted at WN, using alias class and POINTS_TO
// information to identify symbols (and vsyms) that cannot alias.  It has
// a number of helper procedures:
//
// Var_is_loop_index returns TRUE if idx is the loop index of one the
// loops enclosing bb.
//
// Has_read_only_parm returns TRUE if idx is a read-only parameters
//
// Generate_call_mu_chi_by_ref generates the MU and CHI node lists for
// procedure calls by reference.
//
// Generate_call_mu_chi_by_value generates the MU and CHI node lists for
// procedure calls by value.
//
// Add_nested_call_mu_chi adds an extra requirement for nested functions.
//
// Generate_asm_mu_chi generates the MU and CHI node lists for ASMs.
//
// Generate_exit_mu generates the MU node list for RETURN and RETURN_VAL.
//
// ======================================================================


// Returns TRUE if idx is the loop index of one the loops 
// enclosing bb.
BOOL OPT_STAB::Var_is_loop_index(AUX_ID idx, BB_NODE *bb)
{
  // this only applies to Fortran
  if (!IS_FORTRAN)
    return FALSE;

  // if idx is the index var of any loop, return FALSE
  if (!aux_stab[idx].Loop_index())
    return FALSE;

  BB_LOOP *loop = bb->Innermost();
  // check the loops enclosing this bb.
  while (loop != NULL) {
    if (loop->Index() != NULL && St(idx) == WN_sym(loop->Index()))
      return TRUE;
    loop = loop->Parent();
  }
  return FALSE;
}


//  CALL BY REFERENCE 
void
OPT_STAB::Generate_call_mu_chi_by_ref(WN *wn, ST *call_st, 
				      MU_LIST *mu, CHI_LIST *chi, 
				      INT32 num_parms, BB_NODE *bb)
{
  AUX_ID idx;
  const BS *alias_set;

  alias_set = Rule()->Alias_Set_Call_By_Ref(this);
  for (idx = BS_Choose( alias_set );
       idx != (AUX_ID) BS_CHOOSE_FAILURE;
       idx = BS_Choose_Next ( alias_set, idx ))  {
    
    // Volatile do not appear in any mu and chi
    if (Aux_stab_entry(idx)->Is_volatile() &&
	!Aux_stab_entry(idx)->Is_virtual() )
      continue;
    
    READ_WRITE how = Rule()->Aliased_with_Call(call_st,
					       WN_call_flag(wn),
					       aux_stab[idx].Points_to());
    
    // skip the following if function has no parm_mod
    if (how != READ_AND_WRITE)
      for (INT32 i = 0; i < num_parms; i++) {
	WN *actual = WN_actual(wn,i);
	if ( WN_Parm_By_Reference(actual) ) {
	  OCC_TAB_ENTRY *p_occ = Get_occ(actual);
	  POINTS_TO     *pt = p_occ->Points_to();
	  if (Rule()->Aliased_Memop(pt, aux_stab[idx].Points_to())) {
	    how = READ_AND_WRITE;
	    break;
	  }
	}
      }

    if (how & READ)
      mu->New_mu_node(idx, Occ_pool());

    if ((how & WRITE) && IS_FORTRAN && !Var_is_loop_index(idx, bb))
      chi->New_chi_node(idx, Occ_pool());
  }
}


// Returns TRUE if idx is a read-only parameters
//
BOOL
OPT_STAB::Has_read_only_parm(AUX_ID idx, WN *wn, INT32 num_parms)
{
  if (!OPT_IPA_addr_analysis)
    return FALSE;  // disable this analysis for debugging purpose

  if (!PU_ipa_addr_analysis(Get_Current_PU()))
    return FALSE;  // fast-path: don't waste time because no read-only flag will be set
	
  for (INT32 i = 0; i < num_parms; i++) {
    WN *actual = WN_actual(wn,i);
    Is_True(WN_operator(actual) == OPR_PARM, 
	    ("OPT_STAB::Has_read_only_parm: not a PARM node."));
    
    WN *lda = WN_kid0(actual);
    if (WN_operator(lda) != OPR_LDA)
      continue;
    AUX_ID lda_idx = WN_aux(lda);

    if (WN_Parm_Read_Only(actual)) {
      if (idx == lda_idx) return TRUE;
      if (Is_real_var(idx) || Lda_vsym(idx)) {
	ST *st = Aux_stab_entry(idx)->St();
	INT64 ofst = Aux_stab_entry(idx)->St_ofst();
	INT64 size = Aux_stab_entry(idx)->Byte_size();
	ST *parm_st = Aux_stab_entry(lda_idx)->St();
	INT64 parm_ofst = Aux_stab_entry(lda_idx)->St_ofst();
	INT64 parm_size = Aux_stab_entry(lda_idx)->Byte_size();
	// if <st,ofst,size> of idx is sub-range of <st,ofst,size> of lda_idx
	if (st == parm_st && 
	    ofst >= parm_ofst && 
	    size > 0 &&
	    parm_size > 0 &&
	    (ofst + size) <= (parm_ofst + parm_size)) {
	  return TRUE;
	}
      }
    }
  }
  return FALSE;
}


//  Call by value
void
OPT_STAB::Generate_call_mu_chi_by_value(WN *wn, ST *call_st, 
					MU_LIST *mu, CHI_LIST *chi, 
					INT32 num_parms)
{
  AUX_ID idx;
  const BS *alias_set;

  alias_set = Rule()->Alias_Set_Call_By_Value(this);
  for (idx = BS_Choose( alias_set );
       idx != (AUX_ID) BS_CHOOSE_FAILURE;
       idx = BS_Choose_Next ( alias_set, idx )) {

    // Volatile do not appear in any mu and chi
    if (Aux_stab_entry(idx)->Is_volatile() &&
	!Aux_stab_entry(idx)->Is_virtual() )
      continue;

    POINTS_TO *pt = aux_stab[idx].Points_to();
    READ_WRITE how = Rule()->Aliased_with_Call(call_st,
					       WN_call_flag(wn),
					       pt);
    
    if ((how & WRITE) && // no need to do the following if !write-able
	pt->Not_addr_saved()) { // the only way is modification thru the parameters

      if (Has_read_only_parm(idx, wn, num_parms)) {
	how = READ;
	if (Get_Trace(TP_GLOBOPT, ALIAS_DUMP_FLAG)) {
	  fprintf(TFile, "<alias> Remove the chi node with aux id %d"
		  " due to PARM_READ_ONLY.\n", idx);
	}
      }
    }

    if (how & READ)
      mu->New_mu_node(idx, Occ_pool());
    if (how & WRITE)
      chi->New_chi_node(idx, Occ_pool());
  }
}


void
OPT_STAB::Add_nested_call_mu_chi(WN *wn, ST *call_st,
				 MU_LIST *mu, CHI_LIST *chi)
{
  ST *nested_func = Is_nested_call(wn, *this);

  if ( nested_func != NULL ) {
    WN *pragma_list = Get_MP_accessed_id_list(nested_func);
    AUX_ID idx;
    AUX_STAB_ITER aux_stab_iter(this);

    FOR_ALL_NODE(idx, aux_stab_iter, Init()) {
      // Volatile do not appear in any mu and chi
      if (Aux_stab_entry(idx)->Is_volatile() &&
	  !Aux_stab_entry(idx)->Is_virtual() )
	continue;

      // only consider local variables
      if (!aux_stab[idx].Points_to()->Local() &&
	  !aux_stab[idx].Points_to()->F_param())
	continue;

      if ( pragma_list != NULL && WOPT_Enable_MP_varref) {
	READ_WRITE how =
	  (READ_WRITE) Get_MP_modref(pragma_list, 
				     aux_stab[idx].Points_to(), Rule());
	if (how & READ) {
	  mu->New_mu_node_no_dups(idx, Occ_pool());
	  aux_stab[idx].Set_has_nested_ref();
	}
	if (how & WRITE) {
	  chi->New_chi_node_no_dups(idx, Occ_pool());
	  aux_stab[idx].Set_has_nested_ref();
	}
      } else {
	// we didn't find a pragma list, so we can limit
	// ourselves to just those that have been marked as
	// having a reference in an nested subroutine
	if ( Aux_stab_entry(idx)->Has_nested_ref() ) {
	  mu->New_mu_node_no_dups(idx, Occ_pool());
	  chi->New_chi_node_no_dups(idx, Occ_pool());
	} else if ( aux_stab[idx].Points_to()->F_param() ) {
	  // WORKAROUND for 345188!
	  mu->New_mu_node_no_dups(idx, Occ_pool());
	  chi->New_chi_node_no_dups(idx, Occ_pool());
	}
      }
    }
  } // end if nested_func
}


// asm without "memory" clobber can change only its output operands,
// which are represented in two ways: 1) store statements following
// the ASM, which will be handled by the general processing (no
// special asm-oriented processing); and 2) address expressions
// specified as inputs to the asm, but that originated as
// "m"-constrained output operands. These are what we have to hunt
// down and recognize here.
void
OPT_STAB::Generate_asm_mu_chi(WN *wn, MU_LIST *mu, CHI_LIST *chi)
{
  AUX_ID idx;
  // fix 762155: very variable can be modified by asm-statement.
  // const BS *alias_set = Rule()->Alias_Set_Asm(this);

  BOOL asm_clobbers_mem = WN_Asm_Clobbers_Mem(wn);
  
  AUX_STAB_ITER aux_stab_iter(this);
  FOR_ALL_NODE(idx, aux_stab_iter, Init()) {
    
    if (Aux_stab_entry(idx)->Is_volatile() &&
	!Aux_stab_entry(idx)->Is_virtual()) {
      continue;
    }
    
    // PREG are not allowed to be in mu/chi list
    // (they cannot be modified by asm statements).
    if (Aux_stab_entry(idx)->Is_preg()) {
      if (!Aux_stab_entry(idx)->Is_dedicated_preg())
	continue;
    } else {
      if (!asm_clobbers_mem)    // non-volatile asm cannot modify memory
	continue;
    }
    
    READ_WRITE how = Rule()->Aliased_with_Asm(wn, aux_stab[idx].Points_to());

    if (how & READ) {
      mu->New_mu_node(idx, Occ_pool());
    }
    if (how & WRITE) {
      chi->New_chi_node(idx, Occ_pool());
    }

    Aux_stab_entry(idx)->Set_disable_local_rvi();
  }

  WN *prag = WN_first(WN_asm_constraints(wn));
  while (prag != NULL) {
    chi->New_chi_node_no_dups(WN_aux(prag), Occ_pool());
    prag = WN_next(prag);
    // this is a register, so no need to set disable_local_rvi.
  }
}


// generate the exit mu, used for returns and previously processed
// regions with returns in them
void
OPT_STAB::Generate_exit_mu(WN *wn)
{
  // Update RETURN mu-list
  AUX_ID idx, vp_idx = Return_vsym();
  MU_LIST *mu = Get_stmt_mu_list(wn);
  mu->New_mu_node(vp_idx, Occ_pool());

  // track list of values referenced here (for RVI)
  if (aux_stab[vp_idx].Aux_id_list() == NULL) 
    Update_aux_id_list(vp_idx);

  // Exit mu list contains all referenced globals and virtual var
  // (excluding the default vsym).

  AUX_STAB_ITER aux_stab_iter(this);
  FOR_ALL_NODE(idx, aux_stab_iter, Init()) {
    AUX_STAB_ENTRY *psym = Aux_stab_entry(idx);
      
    // examining all variables (including volatile virtuals)
    // but excluding the default vsym.

    if (psym->Is_volatile()) continue;
    if (idx == Default_vsym()) continue;

    ST *st;
    if (!psym->Points_to()->Local()) {
      mu->New_mu_node(idx, mem_pool);
    }
    // for copy-out parameters
    // Does this need a pv790096 fix also?  Don't think so....
    else if ((st = psym->Points_to()->Based_sym()) != NULL
	     && ST_sclass(st) == SCLASS_FORMAL
	     && !ST_is_value_parm(st)) {
      mu->New_mu_node(idx, mem_pool);
    }
    // for copy-out parameters
    // pv790096 -- if PU has syscall_linkage attribute, treat all
    // paramenters as live out
    else if ((st = psym->St()) != NULL
	     && (ST_sclass(st) == SCLASS_FORMAL_REF
		 || (ST_sclass(st) == SCLASS_FORMAL
		     && PU_has_syscall_linkage(Get_Current_PU())))) {
      mu->New_mu_node(idx, mem_pool);
    }
    // for externals not caught by if (!psym->Points_to()->Local ...)
    // above.
    else if (Local_static(idx)) {
      mu->New_mu_node(idx, mem_pool);
    }
  }
}


//  Flow Free Alias Analysis for the STMT and its sub-tree.
//
void
OPT_STAB::Generate_mu_and_chi_list(WN *wn, BB_NODE *bb)
{
  if (wn == NULL)
    return;
  
  const OPCODE   opc = WN_opcode(wn);
  const OPERATOR opr = OPCODE_operator(opc);
  Is_True(!OPCODE_is_scf(opc) || opc == OPC_REGION,
	  ("OPT_STAB::Generate_mu_and_chi_list: Wn is SCF %s",
	   OPCODE_name(opc)));
  Is_True(opc != OPC_BLOCK, ("Wn is a OPR_BLOCK."));

  AUX_ID         cur, idx, vp_idx, st_idx;
  CHI_LIST      *chi;
  MU_LIST       *mu;
  const BS      *alias_set;
  OCC_TAB_ENTRY *occ;
  TY_IDX         wn_ty;
  RID           *rid;
  
  switch ( opr ) {
  case OPR_LDID:
  case OPR_LDBITS:
    break;
    
  case OPR_STBITS:
  case OPR_STID:     // ST in the same alias group are affected
    // Create mu and chi list if it has a ST group
    {
      st_idx = WN_aux(wn);
      POINTS_TO *pt = aux_stab[st_idx].Points_to();
      if (pt->No_alias()) 
	break;

      chi = Get_mem_chi_list(wn);
      if (cur = aux_stab[st_idx].St_group()) {
	while (cur && cur != st_idx) {
	  // Volatile do not appear in any mu and chi
	  if (!Aux_stab_entry(cur)->Is_volatile() &&
	      Aux_stab_entry(cur)->Is_real_var()) {
	    if (Rule()->Aliased_Ofst_Rule(aux_stab[st_idx].Points_to(),
					  aux_stab[cur].Points_to())) 
	      chi->New_chi_node(cur, Occ_pool());
	  }
	  cur = aux_stab[cur].St_group();
	}
      }
    
      alias_set = Virtual_var();
      for (idx = BS_Choose( alias_set );
	   idx != (AUX_ID) BS_CHOOSE_FAILURE;
	   idx = BS_Choose_Next ( alias_set, idx )) {
	if (!Aux_stab_entry(idx)->Is_real_var() &&
	    Rule()->Aliased_Memop(aux_stab[st_idx].Points_to(),
				  aux_stab[idx].Points_to())) 
	  chi->New_chi_node(idx, Occ_pool());
      }

      if (aux_stab[st_idx].Points_to()->Weak() ||
	  aux_stab[st_idx].Points_to()->Weak_base()) {
	AUX_ID auxid;
	AUX_STAB_ITER aux_stab_iter(this);
	FOR_ALL_NODE(auxid, aux_stab_iter, Init()) {
	  if (auxid == st_idx) continue;
	  AUX_STAB_ENTRY *psym = &aux_stab[auxid];
	  if ((psym->Is_real_var() && !psym->Is_volatile()) ||
	      psym->Is_virtual()) {
	    POINTS_TO *aux_pt = psym->Points_to();
	    if (Rule()->Aliased_Memop( aux_stab[st_idx].Points_to(), aux_pt))
	      chi->New_chi_node_no_dups(auxid, Occ_pool());
	  }
	}
      }
    
      break;
    }

  case OPR_ILDBITS:
  case OPR_ILOAD:
  case OPR_MLOAD:
    occ = Get_occ(wn);
    Is_True(!WOPT_Enable_Alias_Classification ||
	    REGION_has_black_regions(g_comp_unit->Rid()) ||
	    occ->Points_to()->Alias_class() != OPTIMISTIC_AC_ID,
	    ("indirect load has OPTIMISTIC_AC_ID"));
    Is_True(!WOPT_Enable_Alias_Classification ||
	    (WOPT_Alias_Class_Limit != UINT32_MAX) ||
	    REGION_has_black_regions(g_comp_unit->Rid()) ||
	    occ->Points_to()->Alias_class() != PESSIMISTIC_AC_ID,
	    ("indirect load has PESSIMISTIC_AC_ID"));
    vp_idx = occ->Aux_id();
    occ->New_mem_mu_node(vp_idx, Occ_pool());

    // track list of values referenced here (for RVI)
    if (aux_stab[vp_idx].Aux_id_list() == NULL) 
      Update_aux_id_list(vp_idx);
      
    break;
    
  case OPR_ISTORE:    // ST that are addr_taken are affected
  case OPR_ISTBITS:
  case OPR_MSTORE:
    occ = Get_occ(wn);
    Is_True(!WOPT_Enable_Alias_Classification ||
	    REGION_has_black_regions(g_comp_unit->Rid()) ||
	    occ->Points_to()->Alias_class() != OPTIMISTIC_AC_ID,
	    ("indirect store has OPTIMISTIC_AC_ID"));
    Is_True(!WOPT_Enable_Alias_Classification ||
	    (WOPT_Alias_Class_Limit != UINT32_MAX) ||
	    REGION_has_black_regions(g_comp_unit->Rid()) ||
	    occ->Points_to()->Alias_class() != PESSIMISTIC_AC_ID,
	    ("indirect store has PESSIMISTIC_AC_ID"));
    vp_idx = occ->Aux_id();
    chi = occ->Mem_chi_list();

    // track list of values referenced here (for RVI)
    if (aux_stab[vp_idx].Aux_id_list() == NULL) 
      Update_aux_id_list(vp_idx);
      
    alias_set = Rule()->Alias_Set_Indirect(this);
    wn_ty = WN_object_ty(wn);
    for (idx = BS_Choose( alias_set );
	 idx != (AUX_ID) BS_CHOOSE_FAILURE;
	 idx = BS_Choose_Next ( alias_set, idx )) {

      // Volatile do not appear in any mu and chi
      if (!Aux_stab_entry(idx)->Is_volatile() ||
	  Aux_stab_entry(idx)->Is_virtual() ) {
	
	if (idx == vp_idx)
	  chi->New_chi_node(idx, Occ_pool());
	else if (Rule()->Aliased_Memop(occ->Points_to(),
				       aux_stab[idx].Points_to(),
				       wn_ty,
				       aux_stab[idx].Points_to()->Ty())) {
	  if (!IS_FORTRAN || !Var_is_loop_index(idx, bb))
	    chi->New_chi_node(idx, Occ_pool());
	}
      }
    }
    break;

  case OPR_INTRINSIC_OP:
    {
      for (INT32 i = 0; i < WN_kid_count(wn); i++) {
	occ = Get_occ(WN_kid(wn, i));
	if (occ != NULL) {
	  vp_idx = occ->Aux_id(); 
	  occ->New_mem_mu_node(vp_idx, Occ_pool()); 
	  if (aux_stab[vp_idx].Aux_id_list() == NULL) 
	    Update_aux_id_list(vp_idx);
	}
      }
      break;
    }
    
  case OPR_ICALL:
  case OPR_CALL:	// ST that are addr_taken or external affected
  case OPR_INTRINSIC_CALL:
    {
      ST *call_st = NULL;
      INT32 num_parms = WN_kid_count(wn);

      if (opr == OPR_CALL)
	call_st = WN_sym(wn);
      else if (opr == OPR_ICALL)
	num_parms--;

      occ = Get_occ(wn);
      mu = Get_stmt_mu_list(wn);
      chi = Get_stmt_chi_list(wn);

      if (Rule()->Call_by_reference()) 
	Generate_call_mu_chi_by_ref(wn, call_st, mu, chi, num_parms, bb);
      else
	Generate_call_mu_chi_by_value(wn, call_st, mu, chi, num_parms);

      // Add extra requirement for nested procedures
      Add_nested_call_mu_chi(wn, call_st, mu, chi);
      break;
    }

  case OPR_PARM:
    // generating the mu-list for parameters.
    if ( WN_Parm_By_Reference(wn) ) {
      occ = Get_occ(wn);
      AUX_ID vp_idx = occ->Aux_id();
      // With virtual var, PARM only cares about its own vp.
      occ->New_mem_mu_node(vp_idx, Occ_pool());
      // track list of values referenced here (for RVI)
      if (!aux_stab[vp_idx].Is_real_var() &&
	  aux_stab[vp_idx].Aux_id_list() == NULL) 
	Update_aux_id_list(vp_idx);
    }
    else {
      Is_True( WN_Parm_By_Value(wn),
	       ("OPT_STAB::Generate_mu_and_chi_list: not by value") );
    }

    break;
    
  case OPR_IO:		// a "black-box" operator
    // black-box statements present particular problems
    Is_True( OPCODE_is_black_box(opc), ("OPR_IO is not black box opr."));
    occ = Get_occ(wn);
    Compute_black_box_mu_chi(wn, occ);
    break;

  case OPR_REGION: 
    Is_True(PU_has_region(Get_Current_PU()),
	    ("OPT_STAB::Generate_mu_and_chi_list, region encountered"));
    rid = REGION_get_rid(wn);
    if (RID_level(rid) >= Rgn_level()) {
      // compute mu and chi for black region
      // NOTE: this call also lowers the POINTS_TOs in the boundary sets
      Compute_region_mu_chi(wn, rid, TRUE/*both mu and chi*/, bb);
      return;
    }
    break;

  case OPR_REGION_EXIT: // check if it's a region exit, need a mu
    // need to distinguish two cases:
    //   1) it is a region exit for the region being processed (ok to have mu)
    //   2) it is a region exit for a transparent region inside the region
    //		being processed (no mu wanted)
    // transparency test: is the BB a non-BB_REGIONEXIT or 
    //			  BB is a BB_REGIONEXIT but rid different from
    //				Cfg()->Rid()
    if (bb->Kind() == BB_REGIONEXIT) {
      RID *rid2 = bb->Regioninfo()->Rid();
      rid = Cfg()->Rid();
      if (rid == rid2 && !RID_TYPE_transparent(rid)) {
	// boundary sets exist, use those to generate MUs
	Compute_region_mu_chi(wn, rid, FALSE/*just mu*/, bb);
      }
    }
    break;
    
  case OPR_RETURN: 
  case OPR_RETURN_VAL: 
    Generate_exit_mu(wn);
    break;
  
  case OPR_FORWARD_BARRIER:
  case OPR_BACKWARD_BARRIER:
  case OPR_DEALLOCA:
    {
      occ = Get_occ(wn);
      POINTS_TO_LIST *ptl = Create_barrier_defs(wn); 
      occ->Set_pt_list(ptl);
      Compute_barrier_mu_chi(Get_occ(wn), ptl, bb, 
			     FALSE/*don't check for dups*/, 
			     opr != OPR_DEALLOCA, TRUE, opr != OPR_DEALLOCA);
      CHI_LIST *chi_list = Get_stmt_chi_list(wn);
      CHI_NODE *cnode;
      CHI_LIST_ITER chi_iter;
      if (chi_list) {
	FOR_ALL_NODE( cnode, chi_iter, Init(chi_list) ) {
	  AUX_ID v = cnode->Aux_id();
	  aux_stab[v].Set_disable_local_rvi();	  
	}
      }
    }
    break;

  case OPR_ASM_STMT:
    {
      mu = Get_stmt_mu_list(wn);
      chi = Get_stmt_chi_list(wn);
      Generate_asm_mu_chi(wn, mu, chi);
    }
    break;

  default:
    break;
  }
  
  if ( opc == OPC_COMPGOTO ) {
    // only first kid is important, others are control-flow stuff
    Generate_mu_and_chi_list(WN_kid(wn,0), bb);
  }
  else if ( ! OPCODE_is_black_box( opc )) {
    INT32 i = (opr == OPR_ASM_STMT ? 2 : 0);
    for (; i < WN_kid_count(wn); i++)
      Generate_mu_and_chi_list(WN_kid(wn,i), bb);
  }
}


// ======================================================================
//
// Compute_FFA is invoked by Pre_Optimizer in opt_main.cxx to perform
// flow free pointer analysis.  Compute_FFA creates the initial MU and
// CHI lists and initializes the POINTS_TO data.
//
// Compute_FFA_for_copy performs flow free analysis for a copy statement
// which is being inserted after the normal FFA analysis, but before FSA
// analysis.
//
// Both of these procedures make two passes over the PU.  During the
// first pass, Allocate_mu_chi_and_virtual_var is invoked on each WN
// statement to create occurance table entries and virtual symbols, and
// Transfer_alias_class_to_occ_and_aux is invoked to copy the optimizer
// and interprocedure alias class numbers into the optimizer's symbol
// tables.  During the second pass, Generate_mu_and_chi_list is invoked
// to create the MU and CHI node lists for all WHIRL nodes.
//
// Transfer_alias_class_to_occ_and_aux is invoked only by Compute_FFA.
// It transfers optimizer and interprocedure alias class information from
// the ALIAS_CLASSIFICATION class to the optimizer's symbol tables.  It
// returns TRUE iff some interprocedure alias class information was found.
//
// ======================================================================


// Do what the name says. Also return TRUE if and only if some
// interprocedural alias class information was found.
BOOL
OPT_STAB::Transfer_alias_class_to_occ_and_aux(RID *const rid,
					      WN  *const wn)
{
  BOOL   found_ip_alias_class_info = FALSE;
  OPCODE opc = WN_opcode(wn);

  // BLOCK is allowed; it shows up as a kid of COMPGOTO, for example.
  // Is_True(opc != OPC_BLOCK,
  //         ("Transfer_alias_class_to_occ_and_aux: BLOCK not allowed"));
  if (!OPCODE_is_black_box(opc)) {
    OPERATOR opr = OPCODE_operator(opc);
    if (OPCODE_is_load(opc) ||
	OPCODE_is_store(opc) ||
	opr == OPR_LDA ||
	(opr == OPR_PARM && WN_Parm_By_Reference(wn))) {
      if (OPERATOR_is_scalar_load (opr) || OPERATOR_is_scalar_store (opr) ||
	  opr == OPR_LDA) {
	POINTS_TO *sym_pt = Aux_stab_entry(WN_aux(wn))->Points_to();
	// Since we know this is a POINTS_TO for a directly-accessed
	// variable, it cannot alias with any memory allocated by
	// alloca().
	sym_pt->Set_not_alloca_mem();
	if (WOPT_Enable_Alias_Classification &&
	    !REGION_has_black_regions(rid)) {
	  if (sym_pt->Alias_class() == OPTIMISTIC_AC_ID) {
	    Is_True(WOPT_Enable_Tail_Recur ||	// Tail recursion elim introduces PREGs.
		    (opr == OPR_LDA &&
		     ST_class(Aux_stab_entry(WN_aux(wn))->st) == CLASS_FUNC) ||
		    Aux_stab_entry(WN_aux(wn))->Is_dedicated_preg(),
		    ("Transfer_alias_class_to_occ_and_aux: class should have "
		     "been set during Classify_memops"));
	    // sym_pt->Set_alias_class(Alias_classification()->Alias_class(wn));
	  }
	  else {
	    Is_True((WOPT_Alias_Class_Limit != UINT32_MAX) ||
		    (sym_pt->Alias_class() ==
		     Alias_classification()->Alias_class(wn)) ||
		    // The following can happen because control flow
		    // like IF(blah) { x = ... } got
		    // turned into x = (blah ? ... : x) using SELECT.
		    (Alias_classification()->Alias_class(wn) ==
		     OPTIMISTIC_AC_ID),
		    ("Transfer_alias_class_to_occ_and_aux: Inconsistent alias class"));
	  }
	}
	if (sym_pt->Ip_alias_class() == OPTIMISTIC_AC_ID) {
#if Is_True_On
	  if (Get_Trace(TP_GLOBOPT, ALIAS_DUMP_FLAG)) {
	    fprintf(TFile, "IP alias class %d for ",
		    WN_MAP32_Get(WN_MAP_ALIAS_CLASS, wn));
	    fdump_tree_no_st(TFile, wn);
	  }
#endif
	  IDTYPE ip_alias_class = (IDTYPE) WN_MAP32_Get(WN_MAP_ALIAS_CLASS, wn);
	  sym_pt->Set_ip_alias_class(ip_alias_class);
	  if (ip_alias_class != OPTIMISTIC_AC_ID) {
	    found_ip_alias_class_info = TRUE;
	  }
	}
	else {
	  // We can get more than one alias class for the same aux
	  // ID when LNO decides to save space by equivalencing two
	  // different local arrays that weren't originally
	  // equivalenced by the user and whose live ranges don't
	  // overlap. In this situation, we throw out the
	  // interprocedural alias class information and assume that
	  // per-PU alias classification will distinguish all the
	  // alias behavior of these arrays. This assumption is
	  // justified by the fact that LNO will do this only for
	  // arrays that can be analyzed completely by looking just at
	  // this PU.
	  //
	  // Note: There still could be a correctness issue if LNO
	  // overlaps two different arrays that are given different
	  // aux ID's in wopt (which can happen if their accesses
	  // remain indirect rather than being converted to
	  // STID/LDID). In that case, we might say the two arrays
	  // don't alias even though they really do, and we could end
	  // up incorrectly scheduling across a dependence between
	  // them. For now, this issue is not addressed. The easiest
	  // way to address it is to put a routine in the alias
	  // manager that erases all the alias information associated
	  // with a given WHIRL node, and have LNO call this routine
	  // for all WHIRL nodes that may refer to the objects that it
	  // decides to overlap.
	  //
	  // 981106: Now the invalidation of information is
	  // implemented in LNO, so we return to the assertion instead
	  // of the DevWarn.
  	  Is_True(!WOPT_Enable_Debug_Inconsistent_Ip_Class ||
		  (WOPT_Ip_Alias_Class_Limit != UINT32_MAX) ||
		  (sym_pt->Ip_alias_class() ==
		   (IDTYPE) WN_MAP32_Get(WN_MAP_ALIAS_CLASS, wn)) ||
		  (WN_MAP32_Get(WN_MAP_ALIAS_CLASS, wn) == OPTIMISTIC_AC_ID),
		  ("Transfer_alias_class_to_occ_and_aux: Inconsistent IP alias class: %lu vs. %lu\n"
		   "Someone (LNO:Equivalence_arrays?) forgot to call Note_Invalid_IP_Alias_Class?",
		   sym_pt->Ip_alias_class(), WN_MAP32_Get(WN_MAP_ALIAS_CLASS, wn)));
	  // The following tests essentially the same condition as the
	  // Is_True just above; it is for the production compiler
	  // only, or for when WOPT_Enable_Debug_Inconsistent_Ip_Class
	  // is turned off.
	  if ((sym_pt->Ip_alias_class() != PESSIMISTIC_AC_ID) &&
	      (sym_pt->Ip_alias_class() !=
	       (IDTYPE) WN_MAP32_Get(WN_MAP_ALIAS_CLASS, wn)) &&
	      (WN_MAP32_Get(WN_MAP_ALIAS_CLASS, wn) != OPTIMISTIC_AC_ID)) {
	    // This should never happen, but in the production compiler
	    // we don't abort if it does. So recover gracefully with no
	    // information. If there is an overlap, throw out the
	    // interprocedural alias information.
	    //
	    // We can get into this situation when data layout is
	    // already done for globals and there are out of bounds
	    // references to global arrays in the input code.
	    DevWarn("Transfer_alias_class_to_occ_and_aux: Inconsistent IP alias class: %u vs. %u",
		   sym_pt->Ip_alias_class(), WN_MAP32_Get(WN_MAP_ALIAS_CLASS, wn));
	    // DevWarn("Looks like someone merged unrelated items "
	    //         "(LNO:Equivalence_Arrays?)");
	    sym_pt->Set_ip_alias_class(PESSIMISTIC_AC_ID);
	  }
	}
      }
      else {
	Is_True(Get_occ(wn) != NULL,
		("Transfer_alias_class_to_occ_and_aux: Indirect memop should have "
		 "OCC_TAB_ENTRY"));
      }
      // Direct memops can have OCC_TAB_ENTRY's corresponding to their
      // vsyms. Set the alias class information in those POINTS_TO's,
      // too, as well as for indirect memops.
      OCC_TAB_ENTRY *occ = Get_occ(wn);
      if (occ != NULL) {
	POINTS_TO *occ_pt = occ->Points_to();
	IDTYPE alias_class = Alias_classification()->Alias_class(wn);
	occ_pt->Set_alias_class(alias_class);

	IDTYPE ip_alias_class = WN_MAP32_Get(WN_MAP_ALIAS_CLASS, wn);
	occ_pt->Set_ip_alias_class(ip_alias_class);
	if (ip_alias_class != OPTIMISTIC_AC_ID) {
	  found_ip_alias_class_info = TRUE;
	}

	if ((Alias_classification()->Non_alloca_memop(alias_class)) &&
	    (alias_class <= WOPT_Alias_Class_Limit)) {
	  occ_pt->Set_not_alloca_mem();
	}

	POINTS_TO *vsym_pt = Aux_stab_entry(occ->Aux_id())->Points_to();
	// Do some assertion checking that was suggested by the
	// technique used in an old version of this code...
	//
	// Also, note that Unique_pt() guarantees that the vsym
	// corresponding to this OCC_TAB_ENTRY has a single consistent
	// alias class.
	//
	// The above comment is an error. Unique_pt() does not
	// guarantee that the vsym has a single consistent alias
	// class, because the Based_sym() can be assigned to more than
	// once. Unique_pt() means only that the Based_sym() is the
	// unique handle on the memory pointed to, not that it points
	// to only one block of memory. Because of this, most of the
	// assertions inside the following if () { ... } turn out to
	// be false in general, and the only things left inside there
	// are a DevWarn and the code that gives the vsym's POINTS_TO
	// the Local() attribute in some situations.
	//
	// Note that setting the Local() attribute may seem premature,
	// too, since a second (and third, and ...) alias class may
	// come along and be associated with the vsym, but in fact if
	// alias classification managed to prove that memops belonging
	// to any of these classes cannot be live out, then it must
	// have proved that none of them is live out. This is because
	// the base pointer must be local and must be
	// ST_PT_TO_UNIQUE_MEM. -- RK 981116
	if (vsym_pt->Unique_pt()) {
	  Is_True(Aux_stab_entry(occ->Aux_id())->Is_virtual(),
		  ("Transfer_alias_class_to_occ_and_aux: Robert thought "
		   "Unique_pt POINTS_TO would be virtual"));
	  // The following assertion can fail when we compile
	  // code that dereferences a PT_TO_UNIQUE_MEM pointer that is
	  // never made to point to anything (never defined).
	  // Such pointers should always be defined by entry chi,
	  // though, so the assertion should be OK.
	  //
	  // 981116: Assertion removed because it really asserts the
	  // wrong thing, as exposed by bug 651815. The assertion says
	  // "this pointer can point to only one block of memory,"
	  // which is wrong. The correct thing to say is, essentially,
	  // "Only this pointer can point to the memory it points to."
	  //
	  // The 651815 case began with both properties, i.e., a
	  // one-to-one correspondence between the pointer and its
	  // memory. But LNO's tiling duplicated some code including
	  // the memory allocation for that pointer. As a result, in
	  // two separate non-overlapping live ranges, the pointer
	  // points to two different blocks of memory. In one live
	  // range, the base pointer was replaced by a preg, but of
	  // course the memops in that live range had their restricted
	  // map entries left intact (which is as it should be). As a
	  // result, alias analysis assigns the same vsym to memops
	  // from both live ranges (because the restricted map
	  // correctly says they are based on the same base pointer),
	  // but alias classification proves that memops from one live
	  // range cannot alias with memops from the other because the
	  // code after LNO uses two separate base pointers (the
	  // unique pointer in one case and the preg in the
	  // other). The right solution is to throw away the alias
	  // class information in the vsym for these memops. The
	  // Unique_pt() attribute gives us all the information we
	  // could hope for anyway because LNO will not introduce a
	  // case where the live ranges of the original base pointer
	  // and the copy overlap. The discarding of the alias class
	  // information (and the IP alias class information) happens
	  // inside POINTS_TO::Meet_info_from_alias_class(). -- RK
#if 0
	  Is_True((vsym_pt->Alias_class() == OPTIMISTIC_AC_ID) ||
		  (vsym_pt->Alias_class() == alias_class),
		  ("Transfer_alias_class_to_occ_and_aux: Inconsistent alias "
		   "class for vsym"));
#endif
	  // Note: Since the above assertion doesn't hold in general,
	  if (alias_class != OPTIMISTIC_AC_ID &&
	      alias_class != PESSIMISTIC_AC_ID &&
	      !Alias_classification()->Writable_by_call(alias_class) &&
	      alias_class <= WOPT_Alias_Class_Limit) {
	    // The memory object cannot be touched by callees of this
	    // routine, and the aux_id for this vsym might not have
	    // existed at the time of
	    // Alias_classification()->Classify_memops(). As a result,
	    // this vsym's aux_id would not have been marked as
	    // inaccessible to callees by alias classification, and we
	    // have to mark it that way now.
	    Set_inaccessible_to_callees(BS_Union1D(Inaccessible_to_callees(),
						   occ->Aux_id(), mem_pool));
	    // This object can't live outside the present PU. Mark it
	    // as local so we don't generate mu/chi for it at CALL and
	    // RETURN points. This is easier than checking the
	    // Alias_class() at every place where we might generate
	    // such a mu or chi node.
	    vsym_pt->Set_local();
	    if (Get_Trace(TP_GLOBOPT, ALIAS_DUMP_FLAG)) {
	      fprintf(TFile, "Xfer_ac: Aux ID %d is local unique_pt vsym\n",
		      occ->Aux_id());
	    }
	    if (WOPT_Alias_Class_Limit != UINT32_MAX) {
	      Lmt_DevWarn(1,
			  ("Alias class used to set Inaccessible_to_callees"));
	    }
	  }
	  // The following assertion removed 981116; see comments
	  // above for the per-PU case that explain why. -- RK
#if 0
	  Is_True((vsym_pt->Ip_alias_class() == OPTIMISTIC_AC_ID) ||
		  (vsym_pt->Ip_alias_class() ==
		   WN_MAP32_Get(WN_MAP_ALIAS_CLASS, wn)) ||
		  (WN_MAP32_Get(WN_MAP_ALIAS_CLASS, wn) == OPTIMISTIC_AC_ID),
		  ("Transfer_alias_class_to_occ_and_aux: Inconsistent IP alias "
		   "class for vsym"));
#endif
#if Is_True_On
	  if ((WOPT_Ip_Alias_Class_Limit == UINT32_MAX) &&
	      (WN_MAP32_Get(WN_MAP_ALIAS_CLASS, wn) ==
	       OPTIMISTIC_AC_ID) &&
	      (vsym_pt->Ip_alias_class() != OPTIMISTIC_AC_ID)) {
	    DevWarn("Someone (LNO?) introduced a WN without setting "
		    "its IP alias class");
	  }
#endif
	}

	// Perform the part of POINTS_TO::Meet that is related to
	// alias class information, since Meet operations will already
	// have been done before we finalize the full set of
	// POINTS_TO's for the program. As a special case, this has
	// the effect that when the aux ID for this memop's vsym is
	// guaranteed to have a single consistent alias class, we
	// transfer the alias class info onto the vsym corresponding
	// to the memop.
	vsym_pt->Meet_info_from_alias_class(occ_pt);
      }
    }
    for (UINT i = 0; i < WN_kid_count(wn); ++i) {
      found_ip_alias_class_info |= Transfer_alias_class_to_occ_and_aux(rid, WN_kid(wn, i));
    }
  }
  return found_ip_alias_class_info;
}


//  Flow Free Alias Analysis for the PU
//    This function examines the PU twice.  The first time it creates
//    the MU and CHI lists for scalar variables, and also identifies
//    the mapping from ILOAD/ISTORE to virtual variables and summarize
//    the alias behavior of the set of ILOAD/ISTORE into the virtual 
//    variable. The second pass inserts the virtual variables into
//    the MU and CHI lists.
//
void OPT_STAB::Compute_FFA(RID *const rid)
{
  CFG_ITER cfg_iter;
  STMT_ITER stmt_iter;
  BB_NODE *bb;
  WN *wn;
  BOOL found_ip_alias_class_info = FALSE;

  // Set current analysis mode to be FFA (used by Simplify_Pointer ...)
  Set_FFA();

  FOR_ALL_ELEM (bb, cfg_iter, Init(_cfg)) {
    FOR_ALL_ELEM (wn, stmt_iter, Init(bb->Firststmt(), bb->Laststmt())) {
      Allocate_mu_chi_and_virtual_var(wn, bb);
      found_ip_alias_class_info |= Transfer_alias_class_to_occ_and_aux(rid, wn);
    }
  }


  // The following code is last-minute for v7.3 beta, and should be
  // deleted and replaced with something smarter after that
  // release. This is related to the I/O lowering issue for bug
  // 663761.
#ifndef NOT_73_BETA
  // We must enforce the following invariant in the presence of real
  // variables whose only direct loads and stores have no IPA alias
  // class information: Every statement that may modify the real
  // variable must also be seen as having the potential to modify
  // every virtual variable aliased with that scalar. This invariant
  // is to ensure that copy propagation doesn't introduce overlapped
  // live ranges.
  //
  // Here is how we enforce the invariant: We iterate through the
  // AUX_STAB entries from highest (newest) to lowest (oldest) aux
  // ID. Initially all CHAIN_SEEN flags are false. When we find a
  // variable with PROP_CHAIN_SEEN==FALSE, we set chain_head to the
  // current aux ID and iterate down its ST_chain doing the following
  // thing for each variable we find:
  //   o) Set PROP_CHAIN_SEEN = TRUE;
  //   o) For each virtual variable with non-NULL St() field and
  //      having some IP alias class information, we add the triple
  //      <chain_head, aux ID, IP alias class> to the stack of virtual
  //      variables to process.
  //
  // To finish up, we take triples one at a time from the stack of
  // virtual variables to process, and for each triple <chain_head,
  // vsym_aux_id, ip_alias_class>, we propagate the IP alias class to
  // each overlapping real variable in the St_chain that lacks IP
  // alias class information.

  AUX_STAB_REVERSE_ITER aux_stab_reverse_iter(this);
  AUX_ID                chain_head;

  // NOTE: Should probably include pair.h and vector.h.

  typedef pair<AUX_ID , AUX_STAB_ENTRY *> CHAIN_HEAD_VIRTUAL_PAIR;
  typedef vector<CHAIN_HEAD_VIRTUAL_PAIR,
                 mempool_allocator<CHAIN_HEAD_VIRTUAL_PAIR> > IP_ALIAS_CLASS_VIRTUALS;
  IP_ALIAS_CLASS_VIRTUALS ip_alias_class_virtuals;

  FOR_ALL_NODE(chain_head, aux_stab_reverse_iter, Init()) {
    AUX_STAB_ENTRY *chain_head_sym = Aux_stab_entry(chain_head);
    if (!chain_head_sym->Prop_chain_seen()) {
      AUX_ID aux_id = chain_head;
      if (chain_head_sym->St_chain() != (AUX_ID) 0) {
	while (aux_id != 0) {
	  AUX_STAB_ENTRY *chain_sym = Aux_stab_entry(aux_id);
	  if (chain_sym->Is_virtual() &&
	      (chain_sym->Points_to()->Ip_alias_class() != OPTIMISTIC_AC_ID)) {
	    ip_alias_class_virtuals.
	      push_back(CHAIN_HEAD_VIRTUAL_PAIR(chain_head, chain_sym));
	  }
	  chain_sym->Set_prop_chain_seen();
	  aux_id = chain_sym->St_chain();
	}
      }
    }
  }

  IP_ALIAS_CLASS_VIRTUALS::iterator chain_head_virtual_pair;
  for (chain_head_virtual_pair = ip_alias_class_virtuals.begin();
       chain_head_virtual_pair != ip_alias_class_virtuals.end();
       ++chain_head_virtual_pair) {
    AUX_STAB_ENTRY *chain_head_sym;
    AUX_STAB_ENTRY *virtual_sym = chain_head_virtual_pair->second;
    AUX_ID next = chain_head_virtual_pair->first;

    do {
      chain_head_sym = Aux_stab_entry(next);
      if (Rule()->Aliased_Ofst_Rule(chain_head_sym->Points_to(),
				    virtual_sym->Points_to())) {
	IDTYPE sym_alias_class =
	  chain_head_sym->Points_to()->Ip_alias_class();
	IDTYPE virtual_sym_alias_class =
	  virtual_sym->Points_to()->Ip_alias_class();
	Is_True((virtual_sym_alias_class == PESSIMISTIC_AC_ID) ||
		(virtual_sym_alias_class == sym_alias_class) ||
		(sym_alias_class == OPTIMISTIC_AC_ID) ||
		// The following can happen now that the assertion
		// containing !WOPT_Enable_Debug_Inconsistent_Ip_Class
		// is relaxed in Transfer_alias_class_to_occ_and_aux,
		// and now that the canonicalization for variables
		// with inconsistent IP alias class info is disabled
		// (see bug 666220).
		(sym_alias_class == PESSIMISTIC_AC_ID),
		("IP alias class info inconsistent for %s",
		 ST_name(chain_head_sym->St())));
	if (sym_alias_class == OPTIMISTIC_AC_ID) {
	  // It is guaranteed that virtual_sym_alias_class !=
	  // OPTIMISTIC_AC_ID from the stack-pushing loop above.
	  DevWarn("IP alias class info lost and restored for part of %s",
		  ST_name(chain_head_sym->St()));
	  chain_head_sym->Points_to()->Set_ip_alias_class(virtual_sym_alias_class);
	}
	else if (sym_alias_class == PESSIMISTIC_AC_ID) {
	  // The virtual variable must not alias less than the
	  // scalar. Discard the alias class information on the
	  // virtual.
	  DevWarn("IP alias class info discarded by overlap for %s",
		  ST_name(chain_head_sym->St()));
	  virtual_sym->Points_to()->Set_ip_alias_class(PESSIMISTIC_AC_ID);
	}
      }
      next = chain_head_sym->St_chain();
    } while (next != (AUX_ID) 0);
  }
#endif // NOT_73_BETA
  
  // Disable ANSI rule for virtual variables
  if (Default_vsym() != 0) 
    Aux_stab_entry(Default_vsym())->Points_to()->Set_ty(0);

  if (Return_vsym() != 0) 
    Aux_stab_entry(Return_vsym())->Points_to()->Set_ty(0);

  // update the precomputed alias sets
  Update_alias_set_with_virtual_var();

  FOR_ALL_ELEM (bb, cfg_iter, Init(_cfg)) {
    FOR_ALL_ELEM (wn, stmt_iter, Init(bb->Firststmt(), bb->Laststmt())) {
      Generate_mu_and_chi_list(wn, bb);
    }
  }

  if (Get_Trace(TP_GLOBOPT, ALIAS_DUMP_FLAG)) {
    fprintf(TFile, "%sPOINTS_TO after flow free alias analysis\n%s",
	    DBar,DBar);
    Print_alias_info(TFile);
  }
}


//  Flow-free analysis for a copy statement which is being inserted
//  after we've done the normal processing.
//
void OPT_STAB::Compute_FFA_for_copy( WN *wn, BB_NODE *bb, BOOL complete_list )
{
  // Set current analysis mode to be FFA (used by Simplify_Pointer ...)
  Set_FFA();
  Allocate_mu_chi_and_virtual_var(wn, bb);
  if (complete_list) 
    Generate_mu_and_chi_list(wn, bb);
  else {
    CHI_LIST *chi = Get_mem_chi_list(wn);
    chi->New_chi_node(Default_vsym(), Occ_pool());
  }
}


// ======================================================================
//
// Compute_FSA is invoked by SSA::Pointer_Alias_Analysis in opt_ssa.cxx
// to perform flow sensitive pointer analysis.  Compute_FSA uses DU info
// to update the POINTS_TO values already determined during flow free
// analysis.
//
// Compute_FSA uses two helper procedures:
//
// Compute_FSA_dominator_order traverses the CFG basic blocks in
// reverse dominator tree order, and invokes Compute_FSA_stmt_or_expr on
// each WN in each block.
//
// Compute_FSA_stmt_or_expr performs the flow sensitive analysis on a
// WN statement and its subtree.
//
// ======================================================================


//  Flow sensitive analysis for the STMT and its subtree.
//
void
OPT_STAB::Compute_FSA_stmt_or_expr(WN *wn)
{
  if (wn == NULL)
    return;

  const OPCODE   opc = WN_opcode(wn);
  const OPERATOR opr = OPCODE_operator(opc);
  if (opc == OPC_REGION)
    return;
  Is_True(!OPCODE_is_scf(opc),
	  ("OPT_STAB::Compute_FSA_stmt_or_expr: Wn is SCF %s",
	   OPCODE_name(opc)));
  
  OCC_TAB_ENTRY *occ;
  if (OPERATOR_is_scalar_iload (opr) || opr == OPR_MLOAD) {
    // to be consistent with OPR_ISTORE
    occ = Get_occ(wn);

    if (Phase() == MAINOPT_PHASE) {
      // Found the original restrict mapping.
      // This map overrides any new unique pointer/restricted pointer
      // discovered later.
      Verify_Restricted_Map(wn, occ->Points_to());
    }

    if (occ->Points_to()->Base_kind() == BASE_IS_UNKNOWN &&
        !occ->Points_to()->F_param()) {

      BOOL is_unique_pt = occ->Points_to()->Unique_pt();
      BOOL is_restricted = occ->Points_to()->Restricted();
      ST *based_sym = occ->Points_to()->Based_sym();

      Analyze_Base_Flow_Sensitive(occ->Points_to(), wn);
      Is_True(Rule()->Aliased_Memop(occ->Points_to(), occ->Points_to())
              || occ->Points_to()->Const(),
	       ("iload not aliased to itself!"));

      // restore the original unique/restrict pointer attributes
      if (is_unique_pt) {
	occ->Points_to()->Set_unique_pt();
	occ->Points_to()->Set_based_sym(based_sym);
      }
      if (is_restricted) {
	occ->Points_to()->Set_restricted();
	occ->Points_to()->Set_based_sym(based_sym);
      }

      if (WOPT_Enable_Update_Vsym)
	Update_iload_vsym(occ);
    }

    if (occ->Points_to()->Based_sym() == NULL)
      Analyze_Based_Pointer(occ->Points_to(), WN_kid0(wn));
  }

  if (OPERATOR_is_scalar_istore (opr) || opr == OPR_MSTORE) {
    BOOL need_update_chi = FALSE;
    CHI_LIST_ITER chi_iter;
    CHI_NODE *cnode;
    occ = Get_occ(wn);

    // Found the original restrict mapping.
    // This map overrides any new unique pointer/restricted pointer
    // discovered later.
    //
    if (Update_From_Restricted_Map(wn, occ->Points_to()))
      need_update_chi = TRUE;

    if (occ->Points_to()->Base_kind() == BASE_IS_UNKNOWN &&
        !occ->Points_to()->F_param()) {
      
      BOOL is_unique_pt = occ->Points_to()->Unique_pt();
      BOOL is_restricted = occ->Points_to()->Restricted();
      ST *based_sym = occ->Points_to()->Based_sym();

      Analyze_Base_Flow_Sensitive(occ->Points_to(), wn);

      if (is_unique_pt) {
	occ->Points_to()->Set_unique_pt();
	occ->Points_to()->Set_based_sym(based_sym);
      }
      if (is_restricted) {
	occ->Points_to()->Set_restricted();
	occ->Points_to()->Set_based_sym(based_sym);
      }

      if (occ->Points_to()->Const()) {
        USRCPOS srcpos;
        USRCPOS_srcpos(srcpos) = WN_Get_Linenum(wn);
        ErrMsgLine(EC_Store_Const, USRCPOS_linenum(srcpos));
      } else {
        Is_True(Rule()->Aliased_Memop(occ->Points_to(), occ->Points_to()),
  	      ("istore not aliased to itself!"));
      }

      if (WOPT_Enable_Update_Vsym)
	Update_istore_vsym(occ);

      need_update_chi = TRUE;
    }

    if (occ->Points_to()->Based_sym() == NULL) {
      Analyze_Based_Pointer(occ->Points_to(), WN_kid1(wn));
      if (occ->Points_to()->Based_sym() != NULL) 
	need_update_chi = TRUE;
    }

    // Fix 704226:  need to update the chi-list after retrieving the
    // restricted or unique flag from RESTRICTED_MAP.
    //
    if (need_update_chi) {
      CHI_LIST *chi_list = occ->Mem_chi_list();
      CHI_NODE *prev_cnode = NULL;
      cnode = chi_list->Head();
      while (cnode != NULL) {
	AUX_ID v = cnode->Aux_id();
	// no need to remove UNIQUE_MEM for the chi list. 
	if (aux_stab[v].Stype() != VT_UNIQUE_VSYM &&
	    !Rule()->Aliased_Memop(occ->Points_to(),
				   aux_stab[v].Points_to())) {
	  Ver_stab_entry(cnode->Result())->Set_synonym( cnode->Opnd());
	  if (Get_Trace(TP_GLOBOPT, ALIAS_DUMP_FLAG))
	    fprintf(TFile, "<alias> Remove the chi node that defines %d.\n",
		    cnode->Result());
	  chi_list->Remove(prev_cnode, cnode);
	  if (prev_cnode != NULL)
	    cnode = prev_cnode->Next();
	  else
	    cnode = chi_list->Head();
	} else {
	  prev_cnode = cnode;
	  cnode = prev_cnode->Next();
	}
      }
    }
  }

  // Traverse the children of WN
  if ( opc == OPC_COMPGOTO ) {
    // only first kid is important, others are control-flow stuff
    Compute_FSA_stmt_or_expr(WN_kid(wn, 0));
  }
  else if ( ! OPCODE_is_black_box( opc ) ) {
    INT32 i = (opr == OPR_ASM_STMT ? 2 : 0);
    for (; i < WN_kid_count(wn); i++)
      Compute_FSA_stmt_or_expr(WN_kid(wn, i));
  }
}


//  Proprocess all BBs in the reverse dominator_tree order
//
void OPT_STAB::Compute_FSA_dominator_order(BB_NODE *bb)
{
  BB_NODE *dom_bb;
  BB_LIST_ITER dom_bb_iter;
  STMT_ITER stmt_iter;
  WN *wn;
  
  FOR_ALL_ELEM(dom_bb, dom_bb_iter, Init(bb->Dom_bbs())) 
    Compute_FSA_dominator_order(dom_bb);

  FOR_ALL_ELEM (wn, stmt_iter, Init(bb->Firststmt(), bb->Laststmt())) {
    Compute_FSA_stmt_or_expr(wn);
  }
}


//  Compute flow sensitive analysis
//     Determine the variable that a pointer points to.
//     Determine if the pointer can point to local variables  
//
void OPT_STAB::Compute_FSA(void)
{
  Set_FSA();   // set alias analysis mode to flow sensitive.
  Compute_FSA_dominator_order(_cfg->Entry_bb());
}


// ======================================================================


void OPT_STAB::Update_return_mu(void)
{
  // update the RETURN mu-lists
  // -- remove the local static that are always defined before use.
  // TODO -- remove the FORTRAN parameters that are not modified.
  //  Fred may have implemented this somewhere.

  BS *use_before_def = BS_Create_Empty(aux_stab.Lastidx()+1, mem_pool);
  if (Cfg()->Fake_entry_bb() != NULL) {
    BB_NODE *bb; 
    BB_LIST_ITER bb_iter;
    FOR_ALL_ELEM (bb, bb_iter, Init(Cfg()->Fake_entry_bb()->Succ())) {
      if (bb->Kind() == BB_ENTRY) {
	WN *entry_chi = bb->Firststmt();
	Is_True(WN_operator(entry_chi) == OPR_OPT_CHI, ("cannot find entry chi."));
	CHI_LIST *chi_list = Get_generic_chi_list(entry_chi);
	CHI_NODE *cnode;
	CHI_LIST_ITER chi_iter;
	FOR_ALL_NODE( cnode, chi_iter, Init(chi_list) ) {
	  if (cnode->Live()) 
	    use_before_def = BS_Union1D(use_before_def, cnode->Aux_id(), mem_pool);
	}
      }
    }
  } else {
    BB_NODE *bb = Cfg()->Entry_bb();
    WN *entry_chi = bb->Firststmt();
    Is_True(WN_operator(entry_chi) == OPR_OPT_CHI, ("cannot find entry chi."));
    CHI_LIST *chi_list = Get_generic_chi_list(entry_chi);
    CHI_NODE *cnode;
    CHI_LIST_ITER chi_iter;
    FOR_ALL_NODE( cnode, chi_iter, Init(chi_list) ) {
      if (cnode->Live()) 
	use_before_def = BS_Union1D(use_before_def, cnode->Aux_id(), mem_pool);
    }
  }
  if (Cfg()->Fake_exit_bb() != NULL) {
    BB_NODE *bb = Cfg()->Exit_bb();
    WN *wn = bb->Laststmt();
    if (wn != NULL && 
	(WN_operator(wn) == OPR_RETURN || WN_operator(wn) == OPR_RETURN_VAL)) {
      MU_LIST *mu_list = Get_stmt_mu_list(wn);
      MU_NODE *prev_mnode = NULL;
      MU_NODE *mnode = mu_list->Head();
      while (mnode != NULL) {
	AUX_ID idx = mnode->Aux_id();
	if (Local_static(idx) && !BS_MemberP(use_before_def, idx)) {
	  if (Get_Trace(TP_GLOBOPT, ALIAS_DUMP_FLAG))
	    fprintf(TFile, "<alias> Remove the mu node with aux id %d.\n", idx);
	  mu_list->Remove(prev_mnode, mnode);
	  if (prev_mnode != NULL)
	    mnode = prev_mnode->Next();
	  else 
	    mnode = mu_list->Head();
	} else {
	  prev_mnode = mnode;
	  mnode = prev_mnode->Next();
	}
      }
    }
  } else {
    BB_NODE *bb = Cfg()->Exit_bb();
    WN *wn = bb->Laststmt();
    if (wn != NULL && 
	(WN_operator(wn) == OPR_RETURN || WN_operator(wn) == OPR_RETURN_VAL)) {
      MU_LIST *mu_list = Get_stmt_mu_list(wn);
      MU_NODE *prev_mnode = NULL;
      MU_NODE *mnode = mu_list->Head();
      while (mnode != NULL) {
	AUX_ID idx = mnode->Aux_id();
	if (Local_static(idx) && !Addr_saved(idx) && !BS_MemberP(use_before_def, idx)) {
	  if (Get_Trace(TP_GLOBOPT, ALIAS_DUMP_FLAG))
	    fprintf(TFile, "<alias> Remove the mu node with aux id %d.\n", idx);
	  mu_list->Remove(prev_mnode, mnode);
	  if (prev_mnode != NULL)
	    mnode = prev_mnode->Next();
	  else 
	    mnode = mu_list->Head();
	} else {
	  prev_mnode = mnode;
	  mnode = prev_mnode->Next();
	}
      }
    }
  }
}


// ======================================================================


//  The following are Alias rule that return a bitset 
//  representing aliased var.  The use of bitset allows
//  more efficient processing in some cases.
//


//  Return the set of var possibly aliased by an generic indirect
//  memory access.   The list can be refined by calling Aliased_Memop again.
//
const BS *ALIAS_RULE::Alias_Set_Indirect(const OPT_STAB *aux_stab) const
{
  return aux_stab->Indirect();
}


//  Return a set of var possibly aliased by a call.
//  The list can be refined by calling Aliased_with_Call again.
//
const BS *ALIAS_RULE::Alias_Set_Call_By_Value(const OPT_STAB *aux_stab) const
{
  return aux_stab->Call_by_value();
}


//  Return a set of parameter possibly aliased by a call.
//  The list can be refined by calling Aliased_with_Call again.
//
const BS *ALIAS_RULE::Alias_Set_Call_By_Ref(const OPT_STAB *aux_stab) const
{
  return aux_stab->Call_by_ref();
}

// Return a set of variables possibly aliased with an ASM statement.
const BS *ALIAS_RULE::Alias_Set_Asm(const OPT_STAB *aux_stab) const
{
  return aux_stab->Asm_alias();
}


// update with alias-set with virtual variables
//
void
OPT_STAB::Update_alias_set_with_virtual_var(void)
{
  IDX_32 idx;
  
  if (Default_vsym() != 0) {
    BS *vv = Virtual_var();
    vv = BS_Union1D(vv, Default_vsym(), mem_pool);
    Set_virtual_var(vv);
  }
  
  if (Return_vsym() != 0) {
    BS *vv = Virtual_var();
    vv = BS_Union1D(vv, Return_vsym(), mem_pool);
    Set_virtual_var(vv);
  }

  BS *call_by_value = Call_by_value();
  call_by_value = BS_UnionD(call_by_value, Virtual_var(), mem_pool);
  call_by_value = BS_DifferenceD(call_by_value,
				 Inaccessible_to_callees());
  Set_call_by_value(call_by_value);

  BS *call_by_ref = Call_by_ref();

#if Is_True_On
  if (Get_Trace(TP_GLOBOPT, ALIAS_DUMP_FLAG)) {
    fprintf(TFile, "Update_alias_set_with_virtual_var: call_by_ref: ");
    BS_Print(call_by_ref, TFile);
  }
#endif	// Is_True_On
  
  call_by_ref = BS_UnionD(call_by_ref, Virtual_var(), mem_pool);

#if Is_True_On
  if (Get_Trace(TP_GLOBOPT, ALIAS_DUMP_FLAG)) {
    fprintf(TFile, "\nafter adding virtual vars: ");
    BS_Print(call_by_ref, TFile);
  }
#endif	// Is_True_On

  call_by_ref = BS_DifferenceD(call_by_ref,
			       Inaccessible_to_callees());

#if Is_True_On
  if (Get_Trace(TP_GLOBOPT, ALIAS_DUMP_FLAG)) {
    fprintf(TFile, "\n");
  }
#endif	// Is_True_On

  Set_call_by_ref(call_by_ref);

  BS *asm_alias_set = Asm_alias();
  asm_alias_set = BS_UnionD(asm_alias_set, Virtual_var(), mem_pool);
  Set_asm_alias(asm_alias_set);
  
  BS *indirect = Indirect();
  indirect = BS_UnionD(indirect, Virtual_var(), mem_pool);
  Set_indirect(indirect);
  
  if (Get_Trace(TP_GLOBOPT, ALIAS_DUMP_FLAG)) {
    fprintf(TFile, "Return vsym is %d\n", Return_vsym());
    fprintf(TFile, "\nAddr_saved bitset:  ");
    BS_Print(Addr_saved(),TFile);
    fprintf(TFile, "\nAddr_passed bitset:  ");
    BS_Print(Addr_passed(),TFile);
    fprintf(TFile, "\nAddr_used bitset:  ");
    BS_Print(Addr_used_locally(),TFile);
    fprintf(TFile, "\nExternal bitset:  ");
    BS_Print(External(),TFile);    
    fprintf(TFile, "\nIndirect bitset:  ");
    BS_Print(Indirect(),TFile);    
    fprintf(TFile, "\nCall_by_value   bitset:  ");
    BS_Print(Call_by_value(),TFile);    
    fprintf(TFile, "\nCall_by_ref  bitset:  ");
    BS_Print(Call_by_ref(),TFile);    
    fprintf(TFile, "\n");
  }
}


//  Print out all the POINTS_TO alias information
//
void OPT_STAB::Print_alias_info(FILE *fp) 
{
  AUX_ID i;
  AUX_STAB_ITER aux_stab_iter(this);
  
  FOR_ALL_NODE(i, aux_stab_iter, Init()) {
    AUX_STAB_ENTRY *psym = &aux_stab[i];
    if (psym->Is_real_var() || psym->Is_virtual()) {
      fprintf(fp, "aux_id=%d ", i);
      psym->Points_to()->Print(fp);
    }
  }
}

// C interface to POINTS_TO::Print()
void Print_points_to(FILE *fp, POINTS_TO *ptmp)
{
  if (ptmp)
    ptmp->Print(fp);
  else
    fprintf(fp,"<NULL>\n");
}

