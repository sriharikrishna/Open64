//-*-c++-*-
// ====================================================================
// ====================================================================
//
// Module: opt_emit_template.h
// $Revision: 1.1.1.1 $
// $Date: 2002-05-22 20:06:48 $
// $Author: dsystem $
// $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/opt/opt_emit_template.h,v $
//
// Revision history:
//  03-OCT-96 shin - Original Version
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
// ====================================================================
// ====================================================================


#ifndef opt_emit_template_INCLUDED
#define opt_emit_template_INCLUDED	"opt_emit_template.h"

#ifndef opt_combine_INCLUDED
#include "opt_combine.h"
#endif // opt_combine_INCLUDED

#ifndef opt_fold_INCLUDED
#include "opt_fold.h"
#endif // opt_fold_INCLUDED

#include "opt_project.h"

#include "be_symtab.h"		// for Preg_Home

#include "opt_cvtl_rule.h"
#include "wn_util.h"            // for WN_COPY_Tree_With_Map


template < class EMITTER >WN*
Gen_exp_wn(CODEREP *exp, EMITTER *emitter)
{
  WN      *wn;
  OPCODE   op;
  OPERATOR opr;
  BOOL     connect_cr_to_wn = TRUE;
  MTYPE    actual_type;
  MTYPE    actual_opnd_type;

  switch (exp->Kind()) {
  case CK_OP:
    opr = exp->Opr();
    switch (opr) {
    /* CVTL-RELATED start (performance) */
    case OPR_CVT:
      {
	if (WOPT_Enable_Cvt_Folding && !emitter->For_preopt()) {
	  INT    cvt_kind;
	  OPCODE opc;
	  cvt_kind =
	    Need_type_conversion(exp->Dsctyp(), exp->Dtyp(), &opc);//326120

	  CODEREP *kid = exp->Get_opnd(0);
	  if (cvt_kind == NEED_CVT) {
	    
	    if (MTYPE_is_integral(exp->Dsctyp())
		&& MTYPE_is_integral(exp->Dtyp())
		&& ! (Only_Unsigned_64_Bit_Ops && MTYPE_signed(exp->Dtyp()))
		&& MTYPE_size_min(kid->Dsctyp()) >= MTYPE_size_min(MTYPE_I4)
		&& ((kid->Kind() == CK_VAR && !kid->Is_var_volatile() &&
		     ST_class(emitter->Opt_stab()->
			      Aux_stab_entry(kid->Aux_id())->St()) != CLASS_PREG)
		    
		    ||
		    (kid->Kind() == CK_IVAR &&
		     OPERATOR_is_scalar_iload (kid->Opr()) &&
		     !kid->Is_ivar_volatile())
		    )
		) {
	      
	      MTYPE mtype_d = kid->Dtyp();
	      MTYPE mtype_dsc = kid->Dsctyp();
	      kid->Set_dtyp(exp->Dtyp());
	      if (exp->Dsctyp() == MTYPE_U4 && exp->Dtyp() == MTYPE_I8) {
		kid->Set_dtyp(MTYPE_U8);
	      }
	      kid->Set_dsctyp(Mtype_TransferSign(kid->Dtyp(), kid->Dsctyp()));
	      wn = Gen_exp_wn(kid, emitter);
	      kid->Set_dtyp(mtype_d);
	      kid->Set_dsctyp(mtype_dsc);
	      connect_cr_to_wn = FALSE;
	    } else {
	      WN* wn_kid = Gen_exp_wn(kid, emitter);
	      BOOL enabled = WN_Simplifier_Enable(TRUE);
	      wn = WN_CreateExp1(exp->Op(), wn_kid);
	      WN_Simplifier_Enable(enabled);
	      if (WN_opcode(wn) != exp->Op()) {
		connect_cr_to_wn = FALSE;
	      }
	    }

	  } else if (cvt_kind == NOT_AT_ALL) {
	    wn = Gen_exp_wn(kid, emitter);
	    connect_cr_to_wn = FALSE;
	  } else
	    Is_True(FALSE, ("Gen_exp_wn: Bad type sequence"));
	  break;
	} else {
	  WN    *opnd = Gen_exp_wn(exp->Get_opnd(0), emitter);
	  INT    cvt_kind;
	  OPCODE opc;

	  cvt_kind =
	    Need_type_conversion(exp->Dsctyp(), exp->Dtyp(), &opc);//326120
	  if (cvt_kind == NEED_CVT)
	    wn = WN_CreateExp1(exp->Op(), opnd);
	  else if (cvt_kind == NOT_AT_ALL) {
	    wn = opnd;
	    connect_cr_to_wn = FALSE;
	  }
	  else
	    Is_True(FALSE, ("Gen_exp_wn: Bad type sequence"));
	  break;
	}
      }
    case OPR_CVTL:
      {
	if (WOPT_Enable_Cvt_Folding && ! emitter->For_preopt()) {
	  CODEREP *kid = exp->Get_opnd(0);
	  WN      *opnd;
	  actual_type = Actual_cvtl_type(exp->Op(), exp->Offset());

	  if ((kid->Kind() == CK_VAR && !kid->Is_var_volatile() &&
	       ST_class(emitter->Opt_stab()->Aux_stab_entry(kid->Aux_id())
			->St()) != CLASS_PREG)
	      ||
	      (kid->Kind() == CK_IVAR &&
	       OPERATOR_is_scalar_iload (kid->Opr()) &&
	       !kid->Is_ivar_volatile())) {
	    actual_opnd_type = kid->Dsctyp();
	    if (MTYPE_is_integral(actual_opnd_type) &&
		MTYPE_is_integral(exp->Dtyp()) &&
		! (Only_Unsigned_64_Bit_Ops && MTYPE_signed(exp->Dtyp()))) {
	      if (MTYPE_size_min(actual_opnd_type)<MTYPE_size_min(MTYPE_I4) &&
		  MTYPE_size_min(actual_type)==MTYPE_size_min(actual_opnd_type)) {
		MTYPE mtype_d = kid->Dtyp();
		MTYPE mtype_dsc = kid->Dsctyp();
		kid->Set_dtyp(exp->Dtyp());
		kid->Set_dsctyp(Mtype_TransferSign(exp->Dtyp(), kid->Dsctyp()));
		wn = Gen_exp_wn(kid, emitter);
		kid->Set_dtyp(mtype_d);
		kid->Set_dsctyp(mtype_dsc);
		connect_cr_to_wn = FALSE;
		break;
	      } else if (MTYPE_size_min(actual_opnd_type) >= MTYPE_size_min(actual_type) &&
			 MTYPE_size_min(actual_type) < MTYPE_size_min(MTYPE_I4)) {
		INT64 offset1 = MTYPE_size_min(actual_opnd_type) >> 3;
		INT64 offset2 = MTYPE_size_min(actual_type) >> 3;
		INT32 offset_old = kid->Offset();
		INT32 offset_new = offset_old;
		// Target_Byte_Sex is declared in config.h
		if (Target_Byte_Sex == BIG_ENDIAN) 
		  offset_new = offset1 - offset2 + offset_old;
		if ( offset_new <= 0x8fffffff ) {
		  kid->Set_offset( offset_new );
		  MTYPE mtype_d = kid->Dtyp();
		  MTYPE mtype_dsc = kid->Dsctyp();
		  kid->Set_dtyp(exp->Dtyp());
		  kid->Set_dsctyp(actual_type);
		  wn = Gen_exp_wn(kid, emitter);
		  kid->Set_offset( offset_old );
		  kid->Set_dtyp(mtype_d);
		  kid->Set_dsctyp(mtype_dsc);
		  connect_cr_to_wn = FALSE;
		  break;
		}
	      }
	    }
	    opnd = Gen_exp_wn(kid, emitter);
	  }
	  else {
	    opnd = Gen_exp_wn(kid, emitter);
	    actual_opnd_type = Actual_result_type(opnd);
	  }

	  if (WN_operator(opnd) == OPR_INTCONST) {
	    MTYPE new_type =
	      Adjust_signed_type(OPCODE_rtype(exp->Op()), exp->Offset(), opnd);
	    if (new_type) actual_type = new_type;
	  }

#if 0 // the analysis in No_truncation_by_value_size  assumes state 
      // before BDCE, so this code has to be disabled now
	  if (actual_type == actual_opnd_type ||
	      (WOPT_Enable_Min_Type &&
	       kid->Kind() == CK_VAR &&
	       MTYPE_is_integral( actual_opnd_type ) &&
	       No_truncation_by_value_size(actual_type, 
					   MTYPE_is_signed(actual_type), 
					   kid, emitter->Opt_stab() )
	       )
	      ) {
	    wn = opnd;
	    connect_cr_to_wn = FALSE;
	  } else 
#endif
	  {
	    BOOL enabled = WN_Simplifier_Enable(TRUE);
	    wn = WN_CreateCvtl(exp->Op(), (INT16)exp->Offset(), opnd);
	    WN_Simplifier_Enable(enabled);
	    if (WN_opcode(wn) != exp->Op() || 
		WN_cvtl_bits(wn) != exp->Offset()) {
	      connect_cr_to_wn = FALSE;
	    }
	  }  

	  break;

	} else {
	  WN *opnd = Gen_exp_wn(exp->Get_opnd(0), emitter);
	  actual_type = Actual_cvtl_type(exp->Op(), exp->Offset());
	  actual_opnd_type = Actual_result_type(opnd);

	  if (exp->Get_opnd(0)->Kind() == CK_VAR ||
	      (exp->Get_opnd(0)->Kind() == CK_IVAR &&
	       OPERATOR_is_scalar_iload (exp->Get_opnd(0)->Opr())))
	    actual_opnd_type = exp->Get_opnd(0)->Dsctyp();

	  if (WN_operator(opnd) == OPR_INTCONST) {
	    MTYPE new_type =
	      Adjust_signed_type(OPCODE_rtype(exp->Op()),exp->Offset(),opnd);
	    if (new_type) actual_type = new_type;
	  }

	  if (actual_type != actual_opnd_type)
	    wn = WN_CreateCvtl(exp->Op(), (INT16)exp->Offset(), opnd);
	  else {
	    wn = opnd;
	    connect_cr_to_wn = FALSE;
	  }
	  break;
	}
      }
    /* CVTL-RELATED finish */
    case OPR_EXTRACT_BITS:
    case OPR_COMPOSE_BITS:
      {
	wn = WN_Create(exp->Op(), exp->Kid_count());
	WN_set_bit_offset_size(wn,exp->Op_bit_offset(),exp->Op_bit_size());
	for (INT i = 0; i < exp->Kid_count(); i++) {
	  CODEREP *opnd = exp->Get_opnd(i);
	  WN_kid(wn, i) = Gen_exp_wn(opnd, emitter);
	}
      }
      break;

    case OPR_ARRAY:
    case OPR_CALL:
    case OPR_ICALL: 
    case OPR_INTRINSIC_CALL: 
    case OPR_INTRINSIC_OP: 
    case OPR_FORWARD_BARRIER:
    case OPR_BACKWARD_BARRIER:
    case OPR_DEALLOCA:
      {
	wn = WN_Create(exp->Op(), exp->Kid_count());
	if (opr == OPR_ARRAY) {
	  WN_element_size(wn) = exp->Elm_siz();
	}
	else if ( opr == OPR_INTRINSIC_CALL || opr == OPR_INTRINSIC_OP ) {
	  WN_intrinsic(wn) = exp->Intrinsic();
	}

	for (INT i = 0; i < exp->Kid_count(); i++) {
	  CODEREP *opnd = exp->Get_opnd(i);
	  WN_kid(wn, i) = Gen_exp_wn(opnd, emitter);
	}
      }
      break;

    case OPR_ASM_STMT:
      {
	wn = WN_Create(exp->Op(), exp->Kid_count() + 2);
	for (INT i = 0; i < exp->Kid_count(); ++i) {
	  CODEREP *opnd = exp->Get_opnd(i);
	  WN_kid(wn, i + 2) = Gen_exp_wn(opnd, emitter);
	}
      }
      break;

    case OPR_ASM_INPUT:
      {
	wn = WN_Create(exp->Op(), exp->Kid_count());
	WN_asm_opnd_num(wn) = exp->Asm_opnd_num();
	WN_st_idx(wn) = exp->Asm_constraint();
	for (INT i = 0; i < exp->Kid_count(); ++i) {
	  CODEREP *opnd = exp->Get_opnd(i);
	  WN_kid(wn, i) = Gen_exp_wn(opnd, emitter);
	}
      }
      break;

    case OPR_TAS:
      wn = WN_Tas(exp->Dtyp(), 
		  exp->Ty_index(), 
		  Gen_exp_wn(exp->Get_opnd(0), emitter));

      // we may want to do a little tiny simplification
      if ( WOPT_Enable_Combine_Operations ) {
	WN *combined_operation;
	if ( Uncombine_Operations( wn, &combined_operation ) ) {
	  wn = combined_operation;
	}
      }
      break;

    default:
      {
	STMTREP *proj_defstmt;
	if (Projection_operation(exp->Op()) &&
	    exp->Opnd(0)->Kind() == CK_VAR) {
	  proj_defstmt = Proj_defstmt(exp->Opnd(0),
				      emitter->Opt_stab());
	}
	else {
	  proj_defstmt = NULL;
	}
	if (proj_defstmt != NULL &&
	    Stores_proj_op_to_temp(proj_defstmt,
				   emitter->Opt_stab()) &&
	    proj_defstmt->Proj_op_uses() == 1) {
	  // Defstmt of the operand of the projection op wasn't
	  // emitted because this use is the unique one. We should
	  // substitute back so we get the correct recombination in
	  // the routine Uncombine_operations (sic) below.
	  wn = WN_CreateExp1(exp->Op(),
			     Gen_exp_wn(exp->Opnd(0)->Defstmt()->Rhs(),
					emitter));
	}
	else if (exp->Kid_count() == 0) {
	  wn = WN_CreateExp0(exp->Op());
	} else if (exp->Kid_count() == 1) {
	  wn = WN_CreateExp1(exp->Op(),
			     Gen_exp_wn(exp->Get_opnd(0), emitter));
	} else if (exp->Kid_count() == 2) {
          WN *opnd0 = Gen_exp_wn(exp->Get_opnd(0), emitter);
          WN *opnd1 = Gen_exp_wn(exp->Get_opnd(1), emitter);
	  wn = WN_CreateExp2(exp->Op(), opnd0, opnd1);
	} else if (exp->Kid_count() == 3) {
          WN *opnd0 = Gen_exp_wn(exp->Get_opnd(0), emitter);
          WN *opnd1 = Gen_exp_wn(exp->Get_opnd(1), emitter);
          WN *opnd2 = Gen_exp_wn(exp->Get_opnd(2), emitter);
	  wn = WN_CreateExp3(exp->Op(), opnd0, opnd1, opnd2);
	} else {
	  Is_True(FALSE, ("Kid count > 3."));
	}

	// we may want to do a little tiny simplification
	if ( WOPT_Enable_Combine_Operations ) {
	  WN *combined_operation;
	  if ( Uncombine_Operations( wn, &combined_operation ) ) {
	    wn = combined_operation;
	  }
	}

      }
      break;
    }
    break;
  case CK_IVAR:
    if (exp->Opr() == OPR_MLOAD) {
      CODEREP *num_byte = exp->Mload_size();
      WN *kid0 = Gen_exp_wn(exp->Ilod_base(), emitter);
      WN *kid1 = Gen_exp_wn(num_byte, emitter);
      wn = WN_CreateMload(exp->Offset(), exp->Ilod_ty(), kid0, kid1);
      emitter->Alias_Mgr()->Gen_alias_id(wn, exp->Points_to(emitter->Opt_stab()));
      WN_set_field_id (wn, exp->I_field_id());
    }
    else if ( exp->Opr() == OPR_PARM ) {
      wn = Gen_exp_wn(exp->Ilod_base(), emitter);
      wn = WN_CreateParm(exp->Dtyp(), wn, exp->Ilod_ty(), exp->Offset());
      if (WN_Parm_By_Reference(wn)) {
	POINTS_TO *pt = exp->Points_to(emitter->Opt_stab());
	Is_True(pt != NULL, ("Reference parameter has NULL POINTS_TO."));
	emitter->Alias_Mgr()->Gen_alias_id(wn, pt);
      }
    }
    else if ( exp->Opr() == OPR_ILOADX ) {
      WN *kid0 = Gen_exp_wn(exp->Ilod_base(), emitter);
      WN *kid1 = Gen_exp_wn(exp->Index(), emitter);
      wn = WN_CreateIloadx(exp->Op(), exp->Ilod_ty(), exp->Ilod_base_ty(), kid0, kid1);
      emitter->Alias_Mgr()->Gen_alias_id(wn, exp->Points_to(emitter->Opt_stab()));
      if (emitter->Gen_lno_info())
        WN_add_lno_info(wn, exp); // for mainopt
    }
    else {
      WN *kid0 = Gen_exp_wn(exp->Ilod_base(), emitter);

      wn = WN_CreateIload (exp->Opr(), exp->Dtyp(), exp->Dsctyp(),
			   exp->Offset(), exp->Ilod_ty(),
			   exp->Ilod_base_ty(), kid0, exp->I_field_id());
      if (exp->Opr() == OPR_ILDBITS)
	  WN_set_bit_offset_size (wn, exp->I_bit_offset(), exp->I_bit_size());
      emitter->Alias_Mgr()->Gen_alias_id(wn, exp->Points_to(emitter->Opt_stab()));
      if (emitter->Gen_lno_info())
        WN_add_lno_info(wn, exp); // for mainopt
    } 
    break;
  case CK_LDA:
    if (exp->Is_flag_set(CF_LDA_LABEL)) {
      wn = WN_LdaLabel(exp->Dtyp(), exp->Offset());
    }
    else {
      wn = WN_CreateLda(OPR_LDA, exp->Dtyp(), MTYPE_V, exp->Offset(), exp->Lda_ty(), exp->Lda_base_st());
      // Generate a small amount of alias information for LDA nodes as
      // well, since LNO may use the LDA's as handles on symbols that it
      // wants to equivalence. In that situation, it may need to
      // invalidate certain interprocedural alias classes.
      // Note: We don't do a full-blown Gen_alias_id for these nodes
      // for a number of reasons. They aren't real memops, we can't tell
      // an access size from them, etc.
      IDTYPE ip_alias_class = exp->Points_to(emitter->Opt_stab())->Ip_alias_class();
      if (ip_alias_class != OPTIMISTIC_AC_ID &&
	  ip_alias_class != PESSIMISTIC_AC_ID) {
	WN_MAP32_Set(WN_MAP_ALIAS_CLASS, wn, ip_alias_class);
      }
    }
    break;
  case CK_VAR:
    {
      AUX_STAB_ENTRY *aux_entry = 
	emitter->Opt_stab()->Aux_stab_entry(exp->Aux_id());
      wn = WN_Create((aux_entry->Bit_size() > 0 && aux_entry->Field_id() == 0)
			? OPR_LDBITS : OPR_LDID,
		     exp->Dtyp(), exp->Dsctyp(), 0); 
      TY_IDX ty_idx = exp->Lod_ty();
      Is_True(ty_idx, ("Gen_exp_wn: NULL Lod_ty() for CK_VAR node"));
      UINT16 field_id = exp->Field_id();
      ST* st = aux_entry->St();
      // when loading from a preg, the size of the preg might be larger
      // than the size of the high-level type, so fix it.
      if (ST_class (st) == CLASS_PREG &&
	  TY_size (ty_idx) != MTYPE_byte_size (exp->Dsctyp())) {
	Set_TY_IDX_index (ty_idx,
			  TY_IDX_index(MTYPE_To_TY (exp->Dsctyp())));
	field_id = 0;
      }
      WN_set_ty(wn, ty_idx);
      WN_st_idx(wn) = ST_st_idx(st);
      if (aux_entry->Bit_size() > 0 && aux_entry->Field_id() == 0)
	WN_set_bit_offset_size (wn, exp->Bit_offset(), exp->Bit_size());
      else
	WN_set_field_id (wn, field_id);
	
      if (aux_entry->Is_non_dedicated_preg() &&
	  exp->Safe_to_renumber_preg()) {
	IDTYPE new_preg_num =
	  emitter->Preg_renumbering_map().Lookup(exp->Coderep_id());
	if (new_preg_num == 0) {
	  if (!aux_entry->Some_version_renumbered()) {
	    // We can use the base PREG number for this version
	    // because no one has used it yet.
	    aux_entry->Set_some_version_renumbered();
	    new_preg_num = exp->Offset();
	  }
	  else {
	    // We need to assign this version a new PREG number.
	    new_preg_num =
	      emitter->Opt_stab()->Alloc_preg(exp->Dtyp(),
					      "renumbered PREG",
					      NULL);
	  }
	  emitter->Preg_renumbering_map().Insert(exp->Coderep_id(),
						 new_preg_num);
	}
	WN_load_offset(wn) = new_preg_num;
      }
      else {
	WN_load_offset(wn) = exp->Offset();
      }
      emitter->Alias_Mgr()->Gen_alias_id(wn, exp->Points_to(emitter->Opt_stab()));
      // update the RVI home WN alias id
      if (ST_class(aux_entry->St()) != CLASS_PREG && aux_entry->Home_sym() != 0)
      {
	AUX_STAB_ENTRY *preg_entry = 
	  emitter->Opt_stab()->Aux_stab_entry(aux_entry->Home_sym());
	Is_True(ST_class(preg_entry->St()) == CLASS_PREG,
		("Gen_exp_wn: home preg is not a preg"));
	WN *preg_home = Preg_Home(preg_entry->St_ofst());
	if (preg_home) {
	  Copy_alias_info(emitter->Alias_Mgr(), wn, preg_home);
	}
      }
    }
    break;
  case CK_CONST:
    op = OPCODE_make_op(OPR_INTCONST, exp->Dtyp(), MTYPE_V);
    wn = WN_Create(op, 0);
    WN_const_val(wn) = exp->Const_val();
    break;
  case CK_RCONST:
    op = OPCODE_make_op(OPR_CONST, exp->Dtyp(), MTYPE_V);
    wn = WN_Create(op, 0);
    WN_st_idx(wn) = ST_st_idx(exp->Const_id());
    break;
  default:
    Warn_todo("Gen_exp_wn: CODEKIND is not implemented yet");
    break;
  }

  // connect up this cr and the resulting wn for def-use
  if ( emitter->Wn_to_cr_map() && connect_cr_to_wn )
    Connect_cr_wn( emitter->Wn_to_cr_map(), exp, wn );

  // When we'll later perform RVI, need to track information from
  // this coderep and this wn
  if ( emitter->Do_rvi() ) {
    if ( exp->Kind() == CK_VAR && ST_class(WN_st(wn)) != CLASS_PREG ) {
      Warn_todo( "CODEREP::Gen_wn: do not adjust bitpos by 1" );
      emitter->Rvi()->Map_bitpos( wn, exp->Bitpos()+1 );
    }
    if ( exp->Kind() == CK_IVAR && exp->Ivar_mu_node() != NULL )
      emitter->Rvi()->Map_mu_node( wn, exp->Ivar_mu_node() );
  }

  return wn;
}


template < class EMITTER >WN *
Gen_stmt_wn(STMTREP *srep, STMT_CONTAINER *stmt_container, EMITTER *emitter)
{
  // resource in emitter used are:
  // emitter->Cfg()->Rid()
  // emitter->Region_entry_stmt()
  // emitter->Opt_stab()
  // emitter->Alias_Mgr()
  // emitter->Wn_to_cr_map()
  // emitter->Mem_pool()
  // emitter->For_preopt()
  // emitter->Do_rvi()
  // emitter->Rvi()

  WN    *rwn = NULL;
  WN    *rhs_wn = NULL;
  WN    *base_wn = NULL;
  WN    *index_wn = NULL;
  OPCODE opcode, opc;

  switch (srep->Opr()) {
  case OPR_GOTO:
    rwn = WN_CreateGoto( srep->St(), srep->Label_number() );
    break;

  case OPR_REGION_EXIT:
    rwn = WN_CreateRegionExit( srep->St(), srep->Label_number() );
// RFE 454057
// TODO: these routines need to be rewritten to use SSA to be more exact.
// Only call for performance regions.
//    REGION_live_out_from_mu(emitter->Cfg()->Rid(),
//                            srep->Mu_list(),
//                            emitter->Opt_stab(),
//                            emitter->Alias_Mgr());
    break;

  case OPR_AGOTO:
    // FmtAssert( FALSE,
    //  ("Gen_stmt_wn: opcode OPR_AGOTO is not implemented yet") );
    rhs_wn = Gen_exp_wn( srep->Rhs(), emitter );
    rwn = WN_CreateAgoto(rhs_wn);
    break;
  case OPR_FALSEBR:
    rhs_wn = Gen_exp_wn( srep->Rhs(), emitter );
    rwn = WN_CreateFalsebr(srep->Label_number(), rhs_wn);
    break;
  case OPR_TRUEBR:
    rhs_wn = Gen_exp_wn( srep->Rhs(), emitter );
    rwn = WN_CreateTruebr(srep->Label_number(), rhs_wn);
    break;
  case OPR_COMPGOTO:
    Is_True( srep->Bb()->Switchinfo() != NULL,
      ("BB:%d has COMPGOTO, but no switchinfo", srep->Bb()->Id()) );
    {
      WN *default_wn = NULL;
      INT32 num_entries = srep->Bb()->Switchentries();
      WN *block_wn = WN_CreateBlock();

      // create a block of GOTOs (in reverse)
      for (INT32 num_entry = num_entries-1; num_entry >= 0; num_entry--) {
        WN_INSERT_BlockAfter( block_wn, NULL,
                             WN_CreateGoto((ST_IDX) NULL,
                                           srep->Bb()->Switchcase(num_entry)->Labnam() ) );
      }

      // create a default goto, if any
      if ( srep->Bb()->Switchdefault() != NULL ) {
        default_wn = 
          WN_CreateGoto((ST_IDX) NULL, srep->Bb()->Switchdefault()->Labnam());
      }
      rhs_wn = Gen_exp_wn( srep->Rhs(), emitter );
      rwn = WN_CreateCompgoto(num_entries, rhs_wn, block_wn, default_wn, 0);
    }
    break;

  case OPR_ASM_STMT:
    {
      // Reconstruct the clobber and constraint pragmas from the
      // information saved aside about them.

      rwn = Gen_exp_wn(srep->Rhs(), emitter);
      WN_st_idx(rwn) = srep->Asm_string_idx();
      WN_asm_flag(rwn) = srep->Asm_stmt_flags();

      WN *clobber_block = WN_CreateBlock();

      vector<CLOBBER_PRAGMA_INFO,
             mempool_allocator<CLOBBER_PRAGMA_INFO> >::const_iterator p;
      for (p = srep->Asm_pragma()->clobber_info.begin();
	   p != srep->Asm_pragma()->clobber_info.end();
	   ++p) {
	WN *prag;
	if (p->preg_st_idx == (ST_IDX) 0) {
	  prag = WN_CreatePragma(WN_PRAGMA_ASM_CLOBBER,
				 (ST_IDX) 0,
				 p->clobber_string_idx,
				 0);
	}
	else {
	  prag = WN_CreateXpragma(WN_PRAGMA_ASM_CLOBBER,
				  (ST_IDX) 0,
				  1);
	  WN_kid0(prag) = WN_CreateIdname(p->preg_number,
					  p->preg_st_idx);
	  WN_pragma_arg2(prag) = p->clobber_string_idx;
	}
	WN_INSERT_BlockAfter(clobber_block,
			     WN_last (clobber_block),
			     prag);
      }
      WN_asm_clobbers(rwn) = clobber_block;

      WN *output_constraint_block = WN_CreateBlock();

      vector<CONSTRAINT_PRAGMA_INFO,
             mempool_allocator<CONSTRAINT_PRAGMA_INFO> >::const_iterator q;
      for (q = srep->Asm_pragma()->constraint_info.begin();
	   q != srep->Asm_pragma()->constraint_info.end();
	   ++q) {
	WN *prag = WN_CreatePragma(WN_PRAGMA_ASM_CONSTRAINT,
				   emitter->Opt_stab()->St(q->preg_st_idx),
				   (INT32) q->constraint_st_idx,
				   q->asm_neg_preg,
				   q->asm_opnd_num);
	WN_INSERT_BlockAfter(output_constraint_block,
			     WN_last(output_constraint_block),
			     prag);
      }
      WN_asm_constraints(rwn) = output_constraint_block;
    }
    break;

  case OPR_CALL:
  case OPR_ICALL:
  case OPR_INTRINSIC_CALL:
  case OPR_FORWARD_BARRIER:
  case OPR_BACKWARD_BARRIER:
  case OPR_ALLOCA:
  case OPR_DEALLOCA:
    //rhs contains the whole node
    {
      OPCODE opc = srep->Op();
      OPERATOR opr = OPCODE_operator(opc);
      rwn = Gen_exp_wn( srep->Rhs(), emitter );
      if (opr == OPR_ICALL)
	WN_set_ty(rwn, srep->Ty());
      if (OPCODE_is_call(opc))
	WN_call_flag(rwn) = srep->Call_flags();
      if (OPCODE_has_sym(opc))
	WN_st_idx(rwn) = ST_st_idx(srep->St());
      if (opr == OPR_FORWARD_BARRIER ||
	  opr == OPR_BACKWARD_BARRIER ||
	  opr == OPR_DEALLOCA) 
	emitter->Alias_Mgr()->Gen_alias_id_list(rwn, srep->Pt_list());
    }
    break;

  case OPR_STID:
  case OPR_STBITS:
    {
      if (Stores_proj_op_to_temp(srep, emitter->Opt_stab()) &&
	  srep->Proj_op_uses() == 1) {
	// The RHS of this stmt will be (re)combined with a
	// projection operation at the unique point where the
	// srep->Lhs() is used. So we do nothing here.
	return NULL;
      }
      else {
	// lhs contains the LOD node, need to reconstruct the STID node
	CODEREP *rhs_cr = srep->Rhs();
	CODEREP *lhs = srep->Lhs();
	/* CVTL-RELATED start (performance) */
	if ( !emitter->For_preopt() &&
	     WOPT_Enable_Cvt_Folding &&
             rhs_cr->Kind() == CK_OP && 
	     ( rhs_cr->Opr() == OPR_CVT &&
	       MTYPE_is_integral( rhs_cr->Dsctyp() ) 
	      || rhs_cr->Opr() == OPR_CVTL ) &&
	     MTYPE_is_integral( rhs_cr->Dtyp() ) && 
             MTYPE_is_integral( lhs->Dsctyp() )
	    ) {
	  
	  MTYPE actual_type;
	  if (rhs_cr->Opr() == OPR_CVTL)
	    actual_type = Actual_cvtl_type(rhs_cr->Op(),rhs_cr->Offset());
	  else if (MTYPE_size_min(rhs_cr->Dsctyp()) <= MTYPE_size_min(rhs_cr->Dtyp()))
	    actual_type = rhs_cr->Dsctyp();
	  else
	    actual_type = rhs_cr->Dtyp();
	  if ( ST_class(emitter->Opt_stab()->St(lhs->Aux_id())) != CLASS_PREG
	      && MTYPE_size_min(lhs->Dsctyp())<=MTYPE_size_min(actual_type) ) {
	    rhs_cr = rhs_cr->Get_opnd(0);
	  } else {
	    FOLD ftmp;
	    CODEREP *cr;
	    cr = ftmp.Fold_Expr(rhs_cr);
	    if (cr) rhs_cr = cr;
	  }
	}  // PV 486445
	/* CVTL-RELATED finish */
	
	rhs_wn = Gen_exp_wn( rhs_cr, emitter );
	opcode = OPCODE_make_op(srep->Opr(), MTYPE_V, lhs->Dsctyp());
	AUX_STAB_ENTRY *aux_entry =
	  emitter->Opt_stab()->Aux_stab_entry(lhs->Aux_id());
	INT64 lhs_offset;
	if (aux_entry->Is_non_dedicated_preg() &&
	    lhs->Safe_to_renumber_preg()) {
	  lhs_offset =
	    emitter->Preg_renumbering_map().Lookup(lhs->Coderep_id());
	  if (lhs_offset == 0) {
	    if (!aux_entry->Some_version_renumbered()) {
	      aux_entry->Set_some_version_renumbered();
	      lhs_offset = lhs->Offset();
	    }
	    else {
	      lhs_offset =
		emitter->Opt_stab()->Alloc_preg(lhs->Dsctyp(),
						"renumbered PREG",
						NULL);
	    }
	    emitter->Preg_renumbering_map().Insert(lhs->Coderep_id(),
						   (IDTYPE) lhs_offset);
	  }
	}
	else {
	  lhs_offset = lhs->Offset();
	}
	ST* st = emitter->Opt_stab()->St(lhs->Aux_id());
	TY_IDX ty_idx = lhs->Lod_ty();
	UINT16 field_id = lhs->Field_id();
	// when saving to a preg, the size of the preg may be larger than
	// the size of the high-level type, need to reflect this.
	if (ST_class (st) == CLASS_PREG &&
	    TY_size(ty_idx) != MTYPE_byte_size (lhs->Dsctyp())) {
	  Set_TY_IDX_index (ty_idx,
			    TY_IDX_index(MTYPE_To_TY (lhs->Dsctyp())));
	  field_id = 0;
	}
	if (lhs->Dsctyp() == MTYPE_B && WN_rtype(rhs_wn) != MTYPE_B) {
	  Is_True(WN_operator(rhs_wn) == OPR_INTCONST,
	        ("Gen_stmt_wn: non-boolean value stored to boolean variable"));
	  WN_set_rtype(rhs_wn, MTYPE_B);
	}
	rwn = WN_CreateStid(opcode, lhs_offset, st, ty_idx, rhs_wn, field_id);
	if ( emitter->Do_rvi() ) {
	  Warn_todo( "Gen_stmt_wn: do not adjust bitpos by 1" );
	  emitter->Rvi()->Map_bitpos( rwn, lhs->Bitpos()+1 );
	}
	emitter->Alias_Mgr()->Gen_alias_id(rwn,
					   lhs->Points_to(emitter->Opt_stab()));
      }
    }
    break;
  case OPR_PREFETCH:
    {
      WN *addr_wn = Gen_exp_wn( srep->Rhs()->Ilod_base(), emitter );
      // Prefetch_wn already pre-allocated
      // update it with the canonicalized address
      rwn = srep->Prefetch_wn();
      WN_kid0(rwn) = addr_wn;
      WN_offset(rwn) = srep->Rhs()->Offset();
      break;
    }

  case OPR_ISTORE:
  case OPR_ISTBITS:
    {
      if (!emitter->For_preopt() &&
	  srep->Rhs()->Kind() == CK_IVAR &&
	  srep->Rhs()->Ilod_base() == srep->Lhs()->Istr_base() &&
	  srep->Rhs()->Offset() == srep->Lhs()->Offset() &&
	  MTYPE_size_min(srep->Rhs()->Dsctyp()) == MTYPE_size_min(srep->Lhs()->Dsctyp()) &&
	  !srep->Rhs()->Is_ivar_volatile() &&
	  !srep->Lhs()->Is_ivar_volatile()) 
	return FALSE;	// omit generating this istore

      CODEREP *rhs_cr = srep->Rhs();
      CODEREP *lhs = srep->Lhs();
      /* CVTL-RELATED start (performance) */
      if ( !emitter->For_preopt() &&
	  WOPT_Enable_Cvt_Folding &&
	  rhs_cr->Kind() == CK_OP && 
	  ( rhs_cr->Opr() == OPR_CVT &&
	   MTYPE_is_integral( rhs_cr->Dsctyp() ) 
	   || rhs_cr->Opr() == OPR_CVTL ) &&
	  MTYPE_is_integral( rhs_cr->Dtyp() ) && 
	  MTYPE_is_integral( lhs->Dsctyp() )
	  ) {
	MTYPE actual_type;
	if (rhs_cr->Opr() == OPR_CVTL)
	  actual_type = Actual_cvtl_type(rhs_cr->Op(),rhs_cr->Offset());
	else if (MTYPE_size_min(rhs_cr->Dsctyp()) <= MTYPE_size_min(rhs_cr->Dtyp()))
	  actual_type = rhs_cr->Dsctyp();
	else
	  actual_type = rhs_cr->Dtyp();
	if ( MTYPE_size_min(lhs->Dsctyp()) <= MTYPE_size_min(actual_type) ) {
	  rhs_cr = rhs_cr->Get_opnd(0);
	} else {
	  FOLD ftmp;
	  CODEREP *cr;
	  cr = ftmp.Fold_Expr(rhs_cr);
	  if (cr) rhs_cr = cr;
	}
      }  // PV 486445
      /* CVTL-RELATED finish */
      
      rhs_wn = Gen_exp_wn( rhs_cr, emitter );

      Is_True(lhs->Istr_base() != NULL,
	      ("Gen_stmt_wn: istr_base has NULL lhs"));
      base_wn = Gen_exp_wn( lhs->Istr_base(), emitter );
      opcode = OPCODE_make_op(srep->Opr(), MTYPE_V, lhs->Dsctyp());
      rwn = WN_Create(opcode, 2);
      WN_kid0(rwn) = rhs_wn;
      WN_kid1(rwn) = base_wn;
      WN_store_offset(rwn) = lhs->Offset();
      WN_set_ty(rwn, lhs->Ilod_base_ty());
      WN_set_field_id(rwn, lhs->I_field_id());
      emitter->Alias_Mgr()->
	Gen_alias_id(rwn, lhs->Points_to(emitter->Opt_stab()));
      if (emitter->Gen_lno_info())
	WN_add_lno_info(rwn, lhs); // for mainopt
      break;
    }

  case OPR_ISTOREX:
    {
      CODEREP *rhs_cr = srep->Rhs();
      CODEREP *lhs = srep->Lhs();
      /* CVTL-RELATED start (performance) */
      if ( !emitter->For_preopt() &&
	  WOPT_Enable_Cvt_Folding &&
	  rhs_cr->Kind() == CK_OP && 
	  ( rhs_cr->Opr() == OPR_CVT &&
	   MTYPE_is_integral( rhs_cr->Dsctyp() ) 
	   || rhs_cr->Opr() == OPR_CVTL ) &&
	  MTYPE_is_integral( rhs_cr->Dtyp() ) && 
	  MTYPE_is_integral( lhs->Dsctyp() )
	  ) {
	MTYPE actual_type = (rhs_cr->Opr() == OPR_CVT) ? 
	  rhs_cr->Dsctyp() : Actual_cvtl_type(rhs_cr->Op(),rhs_cr->Offset());
	if ( MTYPE_size_min(lhs->Dsctyp()) <= MTYPE_size_min(actual_type) ) {
	  rhs_cr = rhs_cr->Get_opnd(0);
	} else {
	  FOLD ftmp;
	  CODEREP *cr;
	  cr = ftmp.Fold_Expr(rhs_cr);
	  if (cr) rhs_cr = cr;
	}
      }  // PV 486445
      /* CVTL-RELATED finish */
	
      rhs_wn = Gen_exp_wn( rhs_cr, emitter );

      base_wn = Gen_exp_wn( lhs->Istr_base(), emitter );
      index_wn = Gen_exp_wn( lhs->Index(), emitter );
      opcode = OPCODE_make_op(OPR_ISTOREX, MTYPE_V, lhs->Dsctyp());
      rwn = WN_Create(opcode, 3);
      WN_kid0(rwn) = rhs_wn;
      WN_kid1(rwn) = base_wn;
      WN_kid(rwn, 2) = index_wn;
      WN_set_ty(rwn, lhs->Ilod_base_ty());
      emitter->Alias_Mgr()->
	Gen_alias_id(rwn, lhs->Points_to(emitter->Opt_stab()));
      if (emitter->Gen_lno_info())
	WN_add_lno_info(rwn, lhs); // for mainopt
      break;
    }

  case OPR_MSTORE:
    {
      rhs_wn = Gen_exp_wn( srep->Rhs(), emitter );
      CODEREP *num_bytes = srep->Lhs()->Mstore_size();
      WN *num_bytes_wn = Gen_exp_wn(num_bytes, emitter );
      base_wn = Gen_exp_wn( srep->Lhs()->Istr_base(), emitter );
      rwn = WN_CreateMstore(srep->Lhs()->Offset(),
                            srep->Lhs()->Ilod_ty(), rhs_wn, 
			    base_wn, num_bytes_wn);
      WN_set_field_id (rwn, srep->Lhs()->I_field_id ());
      emitter->Alias_Mgr()->Gen_alias_id(rwn,
                              srep->Lhs()->Points_to(emitter->Opt_stab()));
    }
    break;

  case OPR_LABEL:
    {
      WN *loop_info = NULL;
      if ( srep->Bb()->Label_loop_info() != NULL ) {
        if (emitter->For_preopt()) {
          // note that we don't know how to update this loop_info without
          // a valid loop.  We punt by creating a new one with no iv or
          // trip count.
          WN *old_info = srep->Bb()->Label_loop_info();
          loop_info = WN_CreateLoopInfo( NULL/*induction*/, NULL/*trip*/,
                                        WN_loop_trip_est(old_info),
                                        WN_loop_depth(old_info),
                                        WN_loop_flag(old_info) );
        }
        else { // ML_WHIRL_EMITTER
          loop_info = emitter->Build_loop_info( srep->Bb() );
        }
      }
      Is_True(srep->Label_number() != 0, ("No label number."));
      rwn = WN_CreateLabel(srep->Label_number(),
			   srep->Label_flags(), 
			   loop_info);
    }
    break;

  case OPR_ASSERT:
  case OPR_RETURN_VAL:
    rwn = WN_COPY_Tree_With_Map(srep->Orig_wn());
    rhs_wn = Gen_exp_wn( srep->Rhs(), emitter );
    WN_kid0(rwn) = rhs_wn;
    break;
  
  case OPR_RETURN:
  case OPR_PRAGMA:
    rwn = WN_COPY_Tree_With_Map(srep->Orig_wn());
    if (OPCODE_has_aux(srep->Op()))
      WN_st_idx(rwn) = ST_st_idx(emitter->Opt_stab()->St(WN_aux(rwn)));
    break;

  case OPR_XPRAGMA:
    rwn = WN_COPY_Tree_With_Map(srep->Orig_wn());
    if (OPCODE_has_aux(srep->Op()))
      WN_st_idx(rwn) = ST_st_idx(emitter->Opt_stab()->St(WN_aux(rwn)));
    rhs_wn = Gen_exp_wn( srep->Rhs(), emitter );
    WN_kid0(rwn) = rhs_wn;
    break;
  
  case OPR_EVAL:
    rhs_wn = Gen_exp_wn( srep->Rhs(), emitter );
    if (srep->Bb()->Kind() == BB_DOHEAD && srep->Bb()->Loop()->Trip_count_stmt() == srep) {
      IDTYPE preg = emitter->Opt_stab()->Alloc_preg(srep->Rhs()->Dtyp());
      ST *preg_st = MTYPE_To_PREG(srep->Rhs()->Dtyp());
      OPCODE opcode = OPCODE_make_op(OPR_STID, MTYPE_V, srep->Rhs()->Dtyp());
      rwn = WN_CreateStid(opcode,preg,preg_st,ST_type(preg_st),rhs_wn);
      emitter->Alias_Mgr()->Gen_alias_id(rwn, NULL);
      opcode = OPCODE_make_op(OPR_LDID, srep->Rhs()->Dtyp(), srep->Rhs()->Dtyp());
      WN *lwn = WN_CreateLdid(opcode,preg,preg_st,ST_type(preg_st));
      emitter->Alias_Mgr()->Gen_alias_id(lwn, NULL);
      srep->Bb()->Loop()->Set_wn_trip_count(lwn);
    } else
      rwn = WN_CreateEval(rhs_wn);
    break;

  case OPR_REGION: // black box region, previously processed
    rwn = srep->Black_box_wn(); 
    break;

  case OPR_OPT_CHI:  // the entry chi statement
    {
      BB_NODE *entry_bb = srep->Bb();
      Is_True(entry_bb->Kind() == BB_ENTRY ||
	      entry_bb->Kind() == BB_REGIONSTART, 
         ("Gen_stmt_wn: cannot find entry bb (%s) for OPR_OPT_CHI node",
	  srep->Bb()->Kind_name()));
      if (entry_bb->Kind() == BB_ENTRY)
	emitter->Connect_sr_wn( srep, entry_bb->Entrywn() );
      else {
        // region wn doesn't exist yet so delay connect_sr_wn
        // until Raise_func_entry
        emitter->Set_region_entry_stmt(srep);
      }
      return NULL;  // do not do anything more
    }
  case OPR_IO:	// one of the "black-box" statements
    rwn = WN_COPY_Tree_With_Map(srep->Black_box_wn());
    emitter->Alias_Mgr()->Gen_black_box_alias(rwn);
    break;

  default:
    FmtAssert(FALSE, ("Gen_stmt_wn: opcode %s is not implemented yet",
		      OPCODE_name(srep->Op())));
  }

  WN_Set_Linenum(rwn, srep->Linenum());

  if (emitter->Cfg()->Feedback())
    emitter->Cfg()->Feedback()->Emit_feedback( rwn, srep->Bb() );

  stmt_container->Append(rwn);

  if (emitter->Do_rvi()) {
    if ( WN_has_mu(rwn, emitter->Cfg()->Rgn_level()) && srep->Mu_list() != NULL ) {
      emitter->Rvi()->Map_mu_list( rwn, srep->Mu_list() );
    }
    if ( WN_has_chi(rwn, emitter->Cfg()->Rgn_level()) && srep->Chi_list() != NULL ) {
      emitter->Rvi()->Map_chi_list( rwn, srep->Chi_list() );
    }
  }

  // connect up this stmtrep and the resulting wn for def-use
  emitter->Connect_sr_wn( srep, rwn );
  return rwn;
}


template < class EMITTER > void
Gen_stmt_list_wn(STMT_LIST *stmt_list,
                 STMT_CONTAINER *stmt_container,
                 EMITTER *emitter)
{
  STMTREP_ITER stmt_iter(stmt_list);
  STMTREP  *tmp;
  FOR_ALL_NODE(tmp, stmt_iter, Init()) {
    WN *twn = Gen_stmt_wn(tmp, stmt_container, emitter);
  }
}


template < class EMITTER >void
Gen_bb_wn(BB_NODE *bb, EMITTER *emitter)
{
  // generate the WHIRL stmt list from the stmtlist
  STMT_CONTAINER stmt_cont(bb->Firststmt(), bb->Laststmt());
  Gen_stmt_list_wn(bb->Stmtlist(), &stmt_cont, emitter);
  bb->Set_firststmt(stmt_cont.Head());
  bb->Set_laststmt(stmt_cont.Tail());
  bb->Set_wngend();
}

#endif  // opt_emit_template_INCLUDED
