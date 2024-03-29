//-*-c++-*-
// ====================================================================
// ====================================================================
//
// Module: opt_revise_ssa.cxx
// $Revision: 1.1.1.1 $
// $Date: 2002-05-22 20:06:50 $
// $Author: dsystem $
// $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/opt/opt_revise_ssa.cxx,v $
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
// Package that performs a certain transformation on the code that
// involves the introduction of new scalar variables. The coderep
// representation is updated and the new scalar variables are entered
// into opt_stab and put into SSA form.
// 
// The package consists of 3 steps:
// 1. Analyze the program to identify the new scalar variables that
//    need to be created due to the current transformation.  The
//    transformations currently supported that are the source of the
//    new scalar variables are:
//	a. ILOAD-LDA folding
//	b. lowering of {LD,ST}BITS into EXTRACT_BITS and COMPOSE_BITS
//    The new scalar variables are entered into opt_stab.
// 2. Statically transform the program for the type of transformation,
//    while introducing the new scalar variables into the program.  At
//    the same time, insert mu's and chi's at points in the program
//    that aliases with the new scalars.
// 3. Put the new scalar variables into SSA form by running the SSA
//    construction algorithm on the new scalars: (a) phi insertion,
//    (b) SSA renaming.
//
// ====================================================================
// ====================================================================


#ifdef USE_PCH
#include "opt_pch.h"
#endif // USE_PCH
#pragma hdrstop


#include "defs.h"
#include "errors.h"
#include "erglob.h"
#include "glob.h"	// for Cur_PU_Name
#include "mempool.h"
#include "tracing.h"	/* for TFile */
#include "cxx_memory.h"

#include "opt_defs.h"
#include "idx_32_set.h"
#include "opt_cfg.h"
#include "opt_ssa.h"
#include "opt_main.h"
#include "opt_mu_chi.h"
#include "opt_sym.h"
#include "opt_htable.h"
#include "opt_alias_rule.h"


extern void dump_tree(WN *wn);
extern void Rename_CODEMAP(COMP_UNIT *);


class OPT_REVISE_SSA {
private:
  OPT_STAB   *_opt_stab;        // the optimizer symtab
  CFG        *_cfg;             // the control flow graph
  CODEMAP    *_htable;          // the hash table
  MEM_POOL   *_loc_pool;	// the local memory pool 
  BOOL	      _tracing;

  AUX_ID      _first_new_aux_id;// point to first opt_stab entry created by this
  BOOL	      _has_bitfield;	// flag to short-circuit some steps if there is
				// no bitfield in program
  BOOL	      _has_lda_indirect;// flag to short-circuit some steps if there is
				// no lda-indirects in program
  BOOL	      _default_vsym_used; // flag to know whether new use of the
				// default vsym is introduced by this phase
  IDX_32_SET *_symbols_to_revise; // set of AUX_IDs of the symbols introduced
				  // whose SSA versions need to be revised

  OCC_TAB_ENTRY *Get_new_occ(CODEREP *cr, BOOL is_store);
  void Find_scalars_from_lowering_bitfld_cr(CODEREP *cr);
  void Find_scalars_from_lda_iloads(CODEREP *cr);
  void Update_phis(BB_NODE *bb);
  void Update_chi_list_for_old_var(STMTREP *stmt, AUX_ID i);
  void Insert_mu_and_chi_list_for_new_var(STMTREP *stmt, AUX_ID i);
  void Update_mu_and_chi_list(STMTREP *stmt);
  CODEREP *Create_EXTRACT_BITS(INT bit_offset, INT bit_size, 
			       CODEREP *x, MTYPE rtype);
  CODEREP *Create_COMPOSE_BITS(INT bit_offset, INT bit_size, 
			       CODEREP *v, CODEREP *rhs);
  CODEREP *Form_extract(CODEREP *cr);
  CODEREP *Fold_lda_iloads(CODEREP *cr);
  void Delete_chi(AUX_ID aux_id, STMTREP *stmt);

  OPT_REVISE_SSA(void);               // REQUIRED UNDEFINED UNWANTED methods
  OPT_REVISE_SSA(const OPT_REVISE_SSA&); // REQUIRED UNDEFINED UNWANTED methods
  OPT_REVISE_SSA& operator = (const OPT_REVISE_SSA&); // REQUIRED UNDEFINED UNWANTED methods

public:
  OPT_REVISE_SSA( OPT_STAB *opt_stab, CODEMAP *htable, CFG *cfg, MEM_POOL *lpool, INT trace_flag): 
	_opt_stab(opt_stab), _htable(htable), _cfg(cfg), _loc_pool(lpool) {
    _first_new_aux_id = opt_stab->Lastidx() + 1;
    _tracing = Get_Trace(TP_GLOBOPT, trace_flag);
    _has_bitfield = FALSE;
    _has_lda_indirect = FALSE;
    _default_vsym_used = FALSE;
    _symbols_to_revise = CXX_NEW(IDX_32_SET(opt_stab->Lastidx(), lpool, OPTS_FALSE), lpool);;
    }
  ~OPT_REVISE_SSA(void) {}

  BOOL Has_bitfield(void) const { return _has_bitfield; }
  BOOL Has_lda_indirect(void) const { return _has_lda_indirect; }

  void Find_scalars_from_lowering_bitfld(void);
  void Find_scalars_from_lda_indirects(void);
  void Form_extract_compose(void);
  void Fold_lda_indirects(void);
  void Rename_new_scalars(COMP_UNIT *cunit);
}; // end of class OPT_REVISE_SSA

// ====================================================================
// - - - - - - Step 1 stuff starts here - - - - - -
// ====================================================================

// ====================================================================
// Get_new_occ - called using an ILDBITS node
// ====================================================================
OCC_TAB_ENTRY *
OPT_REVISE_SSA::Get_new_occ(CODEREP *cr, BOOL is_store)
{
  WN *wn = WN_Create(cr->Op(), 1);  // OCC_TAB_ENTRY needs a wn
  bzero(wn, sizeof(WN));
  WN_set_operator(wn, OPR_ILOAD);
  WN_set_desc(wn, cr->Dsctyp());
  WN_set_rtype(wn, cr->Dtyp());
  WN_store_offset(wn) = cr->Offset();
  WN_set_ty(wn, cr->Ilod_ty());
  WN_set_load_addr_ty(wn, cr->Ilod_base_ty());
  WN_set_field_id(wn, 0);
  if (_tracing)
    fdump_wn(TFile, wn);

#if 0  // cannot use Identify_vsym because it requires address expr in wn also
  AUX_ID vsym_id = _opt_stab->Identify_vsym(wn);
#endif
  AUX_ID vsym_id;	// the vsym to be used for the new ivar node
   // first, see if we can use the same vsym as the bit-field ivar
  AUX_ID old_vsym_id = cr->Ivar_occ()->Aux_id();
  AUX_STAB_ENTRY *old_vsym = _opt_stab->Aux_stab_entry(old_vsym_id);
  if (old_vsym->Is_real_var() || old_vsym->Stype() == VT_LDA_VSYM) {
    if (old_vsym->Bit_size() == 0) {
      vsym_id = old_vsym_id;  // can use same vsym since the old vsym is 
			      // not bit-field-specific
      if (is_store)
        _symbols_to_revise->Union1D(vsym_id);
    }
    else {
      vsym_id = _opt_stab->Find_vsym_with_base_ofst_and_size(old_vsym->Base(), 
		      old_vsym->Base_byte_ofst(), old_vsym->Byte_size(), 0, 0);
      if (vsym_id == 0) { // use default vsym
        if (_opt_stab->Default_vsym() == 0) { // Setup default vsym
	  AUX_ID dft_vsym = _opt_stab->Create_vsym(EXPR_IS_ANY);
	  _opt_stab->Set_default_vsym(dft_vsym);
	  // Update the POINTS_TO with default vsym info
	  _opt_stab->Aux_stab_entry(dft_vsym)->Points_to()->Set_default_vsym();
        }
        vsym_id = _opt_stab->Default_vsym();
        _default_vsym_used = TRUE;
      }
      _symbols_to_revise->Union1D(vsym_id);
    }
  }
  else {
    vsym_id = old_vsym_id;
    if (is_store)
      _symbols_to_revise->Union1D(vsym_id);
  }

//if (_tracing)
//  _opt_stab->Print_aux_entry(vsym_id, TFile);

  OCC_TAB_ENTRY *occ = _opt_stab->Enter_occ_tab(wn, vsym_id);
  occ->Points_to()->Copy_fully(_opt_stab->Aux_stab_entry(vsym_id)->Points_to());
  return occ;
}

// ====================================================================
//  Find_scalars_from_lowering_bitfld_cr - descend the tree to look for LDBITS
//  nodes.  The new scalar variables formed due to lowering them are entered 
//  into opt_stab.
// ====================================================================
void
OPT_REVISE_SSA::Find_scalars_from_lowering_bitfld_cr(CODEREP *cr)
{
  INT i;
  switch (cr->Kind()) {
  case CK_CONST:
  case CK_RCONST:
  case CK_LDA:
    return;
  case CK_VAR: 
    if (cr->Bit_field_valid()) {
      WN wn;
      bzero(&wn, sizeof(WN));
      WN_set_operator(&wn, OPR_LDID);
      WN_set_desc(&wn, cr->Dsctyp());
      WN_set_rtype(&wn, cr->Dtyp());
      WN_store_offset(&wn) = cr->Offset();
      WN_st_idx(&wn) = ST_st_idx(_opt_stab->St(cr->Aux_id()));
      WN_set_ty(&wn, cr->Lod_ty());
      WN_set_field_id(&wn, 0);
      WN_map_id(&wn) = 0;
      if (_tracing)
        fdump_wn(TFile, &wn);

      AUX_ID idx = _opt_stab->Enter_symbol(WN_operator(&wn), WN_st(&wn), 
				       WN_offset(&wn), WN_ty(&wn), FALSE, &wn);
      Is_True(idx != 0, ("Find_scalars_from_lowering_bitfld_cr:: index from Enter_symbol is 0"));
      _symbols_to_revise->Union1D(idx);
      AUX_STAB_ENTRY *aux = _opt_stab->Aux_stab_entry(idx);
      aux->Points_to()->Set_expr_kind(EXPR_IS_ADDR);
      aux->Points_to()->Set_named();

      cr->Set_scalar_aux_id(idx);
      _has_bitfield = TRUE;
      if (_tracing && idx >= _first_new_aux_id)
        _opt_stab->Print_aux_entry(idx, TFile);
    }
    return;
  case CK_IVAR:
    Find_scalars_from_lowering_bitfld_cr(cr->Ilod_base());
    if (cr->Opr() == OPR_MLOAD)
      Find_scalars_from_lowering_bitfld_cr(cr->Mload_size());
    if (cr->Opr() == OPR_ILDBITS) {
      cr->Set_scalar_ivar_occ(Get_new_occ(cr, FALSE));
      _has_bitfield = TRUE;
    }
    return;
  case CK_OP:
    for (i = 0; i < cr->Kid_count(); i++) 
      Find_scalars_from_lowering_bitfld_cr(cr->Opnd(i));
    return;
  }
}

// ====================================================================
//  Find_scalars_from_lowering_bitfld - conduct a pass over the program to
//  look for the {I,}{LD,ST}BITS nodes.  For each such node, create a new
//  entry in _new_scalar_nodes.
// ====================================================================
void
OPT_REVISE_SSA::Find_scalars_from_lowering_bitfld(void)
{
  CFG_ITER cfg_iter(_cfg);
  BB_NODE *bb;
  INT32 i;

  // debugging
  if (_tracing)
    fprintf(TFile, "%sOPT_REVISE_SSA:: Scalars due to lowering bitfields:\n(new scalars start at aux id %d)\n%s", 
	    DBar, _first_new_aux_id, DBar );

  // visit all blocks 
  FOR_ALL_NODE( bb, cfg_iter, Init() ) {
    STMTREP_ITER stmt_iter(bb->Stmtlist());
    STMTREP *stmt;
    FOR_ALL_NODE(stmt, stmt_iter, Init()) {
      OPERATOR opr = stmt->Opr();
      CODEREP *rhs = stmt->Rhs();
      if (OPERATOR_is_call(opr) || opr == OPR_ASM_STMT) {
	for (i = 0; i < rhs->Kid_count(); i++) 
	  Find_scalars_from_lowering_bitfld_cr(rhs->Opnd(i));
	continue;
      }
      if (rhs) {
	if (opr == OPR_PREFETCH) 
	  Find_scalars_from_lowering_bitfld_cr(rhs->Ilod_base());
	else Find_scalars_from_lowering_bitfld_cr(rhs);
      }
      if (OPERATOR_is_store(opr)) {
	CODEREP *lhs = stmt->Lhs();
        switch (opr) {
	case OPR_STBITS: {
	  _symbols_to_revise->Union1D(lhs->Aux_id()); // pv 805267

          WN wn;
          bzero(&wn, sizeof(WN));
	  WN_set_operator(&wn, OPR_STID);
	  WN_set_desc(&wn, stmt->Desc());
	  WN_set_rtype(&wn, MTYPE_V);
	  WN_store_offset(&wn) = lhs->Offset();
	  WN_st_idx(&wn) = ST_st_idx(_opt_stab->St(lhs->Aux_id()));
	  WN_set_ty(&wn, lhs->Lod_ty());
	  WN_set_field_id(&wn, 0);
	  WN_map_id(&wn) = 0;
	  if (_tracing)
	    fdump_wn(TFile, &wn);

	  AUX_ID idx = _opt_stab->Enter_symbol(WN_operator(&wn), WN_st(&wn), 
				       WN_offset(&wn), WN_ty(&wn), FALSE, &wn);
	  Is_True(idx != 0, ("Find_scalars_from_lowering_bitfld:: index from Enter_symbol is 0"));
	  _symbols_to_revise->Union1D(idx);
	  AUX_STAB_ENTRY *aux = _opt_stab->Aux_stab_entry(idx);
	  aux->Points_to()->Set_expr_kind(EXPR_IS_ADDR);
	  aux->Points_to()->Set_named();

	  lhs->Set_scalar_aux_id(idx);
	  _has_bitfield = TRUE;
	  if (_tracing && idx >= _first_new_aux_id)
	    _opt_stab->Print_aux_entry(idx, TFile);
	  break;
	  }
        case OPR_MSTORE:
	  Find_scalars_from_lowering_bitfld_cr(lhs->Mstore_size());
	  // fall thru
        case OPR_ISTORE:
        case OPR_ISTBITS:
	  Find_scalars_from_lowering_bitfld_cr(lhs->Istr_base());
	  if (opr == OPR_ISTBITS) {
	    lhs->Set_scalar_ivar_occ(Get_new_occ(lhs, TRUE));
	    _has_bitfield = TRUE;
	  }
	  break;
        default: ;
        }
      }
    }
  }
}

// ====================================================================
//  Find_scalars_from_lda_iloads - descend the tree to look for 
//  LDA-ILOAD/ILDBITS nodes.  The new direct variables formed due to lowering 
//  them are entered into opt_stab.
// ====================================================================
void
OPT_REVISE_SSA::Find_scalars_from_lda_iloads(CODEREP *cr)
{
  INT i;
  switch (cr->Kind()) {
  case CK_CONST:
  case CK_RCONST:
  case CK_LDA:
  case CK_VAR: 
    return;
  case CK_IVAR: {
    WN wn;
    Find_scalars_from_lda_iloads(cr->Ilod_base());
    if (cr->Opr() == OPR_MLOAD) {
      Find_scalars_from_lda_iloads(cr->Mload_size());
      return;
    }
    if (cr->Ilod_base()->Kind() != CK_LDA || 
	cr->Is_ivar_volatile() ||
	cr->Opr() == OPR_PARM)
      return;
    // this indirect can be folded
    CODEREP *lda = cr->Ilod_base();
    WN_set_operator(&wn, cr->Opr() == OPR_ILOAD ? OPR_LDID : OPR_LDBITS);
    WN_set_desc(&wn, cr->Dsctyp());
    WN_set_rtype(&wn, cr->Dtyp());
    WN_store_offset(&wn) = cr->Offset() + lda->Offset();
    WN_st_idx(&wn) = ST_st_idx(_opt_stab->St(lda->Lda_aux_id()));
    WN_set_ty(&wn, cr->Ilod_ty());
    WN_set_field_id(&wn, cr->I_field_id());
    WN_map_id(&wn) = 0;
    if (_tracing)
      fdump_wn(TFile, &wn);

    AUX_ID idx = _opt_stab->Enter_symbol(WN_operator(&wn), WN_st(&wn), 
				 WN_offset(&wn), WN_ty(&wn), FALSE, &wn);
    Is_True(idx != 0, ("Find_scalars_from_lda_iloads:: index from Enter_symbol is 0"));
    _symbols_to_revise->Union1D(idx);
    AUX_STAB_ENTRY *aux = _opt_stab->Aux_stab_entry(idx);
    aux->Points_to()->Set_expr_kind(EXPR_IS_ADDR);
    aux->Points_to()->Set_named();

    cr->Set_scalar_aux_id(idx);
    _has_lda_indirect = TRUE;
    if (_tracing && idx >= _first_new_aux_id)
      _opt_stab->Print_aux_entry(idx, TFile);
    return;
    }
  case CK_OP:
    for (i = 0; i < cr->Kid_count(); i++) 
      Find_scalars_from_lda_iloads(cr->Opnd(i));
    return;
  }
}

// ====================================================================
//  Find_scalars_from_lda_indirects - conduct a pass over the program to
//  look for ILOAD/ILDBITS/ISTORE/ISTBITS nodes whose base is an LDA.  
//  For each such node, create a new entry in _new_scalar_nodes.
// ====================================================================
void
OPT_REVISE_SSA::Find_scalars_from_lda_indirects(void)
{
  CFG_ITER cfg_iter(_cfg);
  BB_NODE *bb;
  INT32 i;

  // debugging
  if (_tracing)
    fprintf(TFile, "%sOPT_REVISE_SSA:: Scalars due to lda indirects:\n(new scalars start at aux id %d)\n%s", 
	    DBar, _first_new_aux_id, DBar );

  // visit all blocks 
  FOR_ALL_NODE( bb, cfg_iter, Init() ) {
    STMTREP_ITER stmt_iter(bb->Stmtlist());
    STMTREP *stmt;
    FOR_ALL_NODE(stmt, stmt_iter, Init()) {
      OPERATOR opr = stmt->Opr();
      CODEREP *rhs = stmt->Rhs();
      if (OPERATOR_is_call(opr) || opr == OPR_ASM_STMT) {
	for (i = 0; i < rhs->Kid_count(); i++) 
	  Find_scalars_from_lda_iloads(rhs->Opnd(i));
	continue;
      }
      if (rhs) {
	if (opr == OPR_PREFETCH) 
	  Find_scalars_from_lda_iloads(rhs->Ilod_base());
	else Find_scalars_from_lda_iloads(rhs);
      }
      if (OPERATOR_is_store(opr)) {
	CODEREP *lhs = stmt->Lhs();
        switch (opr) {
        case OPR_MSTORE:
	  Find_scalars_from_lda_iloads(lhs->Mstore_size());
	  Find_scalars_from_lda_iloads(lhs->Istr_base());
	  break;
        case OPR_ISTORE:
        case OPR_ISTBITS: {
	  WN wn;
	  if (lhs->Istr_base()->Kind() != CK_LDA) {
	    Find_scalars_from_lda_iloads(lhs->Istr_base());
	    break;
	  }
	  if (lhs->Is_ivar_volatile())
	    break;
	  // this indirect can be folded
	  CODEREP *lda = lhs->Istr_base();
          bzero(&wn, sizeof(WN));
	  WN_set_operator(&wn, opr == OPR_ISTORE ? OPR_STID : OPR_STBITS);
	  WN_set_desc(&wn, stmt->Desc());
	  WN_set_rtype(&wn, MTYPE_V);
	  WN_store_offset(&wn) = lhs->Offset() + lda->Offset();
	  WN_st_idx(&wn) = ST_st_idx(_opt_stab->St(lda->Lda_aux_id()));
	  WN_set_ty(&wn, lhs->Ilod_ty());
	  WN_set_field_id(&wn, lhs->I_field_id());
	  WN_map_id(&wn) = 0;
	  if (_tracing)
	    fdump_wn(TFile, &wn);

	  AUX_ID idx = _opt_stab->Enter_symbol(WN_operator(&wn), WN_st(&wn), 
				       WN_offset(&wn), WN_ty(&wn), FALSE, &wn);
	  Is_True(idx != 0, ("Find_scalars_from_lda_indirects:: index from Enter_symbol is 0"));
	  _symbols_to_revise->Union1D(idx);
	  AUX_STAB_ENTRY *aux = _opt_stab->Aux_stab_entry(idx);
	  aux->Points_to()->Set_expr_kind(EXPR_IS_ADDR);
	  aux->Points_to()->Set_named();

	  lhs->Set_scalar_aux_id(idx);
	  _has_lda_indirect = TRUE;
	  if (_tracing && idx >= _first_new_aux_id)
	    _opt_stab->Print_aux_entry(idx, TFile);
	  break;
	  }
        default: ;
        }
      }
    }
  }
}

// ====================================================================
// - - - - - - Step 2 stuff starts here - - - - - -
// ====================================================================

// ====================================================================
// Update_phis - for pre-existing variables, if a phi is zero version,
// need to change to non-zero version
// ====================================================================
void
OPT_REVISE_SSA::Update_phis(BB_NODE *bb)
{
  PHI_NODE *phi;
  PHI_LIST_ITER phi_iter;
  CODEREP *phi_res;

  // iterate thru each phi node
  FOR_ALL_ELEM (phi, phi_iter, Init(bb->Phi_list())) {
    AUX_ID i = phi->Aux_id();
    if (i >= _first_new_aux_id)
      continue;
    if (! _symbols_to_revise->MemberP(i))
      continue;
    // it is a pre-existing variable
    if (! phi->Live() || phi->RESULT()->Is_flag_set(CF_IS_ZERO_VERSION)) {
      phi->Set_live();
      phi->Reset_dse_dead();
      phi->Reset_dce_dead();
      AUX_STAB_ENTRY *sym = _opt_stab->Aux_stab_entry(i);
      MTYPE rtype = Mtype_from_mtype_class_and_size(sym->Mclass(), 
						    sym->Byte_size());
      MTYPE desc = rtype;
      phi_res = _htable->Add_def(i, -1, NULL, rtype, desc, 
			 _opt_stab->St_ofst(i), MTYPE_To_TY(rtype), 0, TRUE);
      phi_res->Set_flag(CF_DEF_BY_PHI);
      phi_res->Set_defphi(phi);
      phi->Set_result(phi_res);
      if (! phi->Res_is_cr()) {
	BB_NODE *pred;
	BB_LIST_ITER bb_iter;
	FOR_ALL_ELEM(pred, bb_iter, Init(bb->Pred())) {
	  phi->Set_opnd(bb_iter.Idx(), 
			_htable->Ssa()->Get_zero_version_CR(i, _opt_stab, 0));
	}
      }
    }
  }
}

// ====================================================================
// Update_chi_list_for_old_var - if there is alias, there must have been
// a pre-existing chi node; make it live and non-zero-version
// ====================================================================
void
OPT_REVISE_SSA::Update_chi_list_for_old_var(STMTREP *stmt, AUX_ID i)
{
  // find the chi node if it's there and set it live and non-zero version
  CHI_LIST_ITER chi_iter;
  CHI_NODE *cnode;
  CODEREP *chi_res;
  AUX_STAB_ENTRY *sym;
  MTYPE rtype;
  FOR_ALL_NODE(cnode, chi_iter, Init(stmt->Chi_list())) 
    if (cnode->Aux_id() == i) {
      if (! cnode->Live() || cnode->RESULT()->Is_flag_set(CF_IS_ZERO_VERSION)) {
	BOOL originally_dead = ! cnode->Live();
	cnode->Set_live(TRUE);
	cnode->Set_dse_dead(FALSE);
        sym = _opt_stab->Aux_stab_entry(i);
	rtype = Mtype_from_mtype_class_and_size(sym->Mclass(),sym->Byte_size());
	MTYPE desc = rtype;
	chi_res = _htable->Add_def(i, -1, stmt, rtype, desc, 
			   _opt_stab->St_ofst(i), MTYPE_To_TY(rtype), 0, TRUE);
	chi_res->Set_flag(CF_DEF_BY_CHI);
	chi_res->Set_defchi(cnode);
	cnode->Set_RESULT(chi_res);
	if (originally_dead)
	  cnode->Set_OPND(_htable->Ssa()->Get_zero_version_CR(i, _opt_stab, 0));
	stmt->Recompute_has_zver();
      }
      else if (cnode->RESULT()->Dtyp() == MTYPE_UNKNOWN) { // fix the types
	chi_res = cnode->RESULT();
        sym = _opt_stab->Aux_stab_entry(i);
	rtype = Mtype_from_mtype_class_and_size(sym->Mclass(),sym->Byte_size());
	chi_res->Set_dtyp(rtype);
	chi_res->Set_dsctyp(rtype);
	chi_res->Set_lod_ty(MTYPE_To_TY(rtype));
      }
      return;
    }
}

// ====================================================================
// Insert_mu_and_chi_list_for_new_var - mu's and chi's only need to be inserted 
// at the statement level for the given variable; this is for new symbols only
// ====================================================================
void
OPT_REVISE_SSA::Insert_mu_and_chi_list_for_new_var(STMTREP *stmt, AUX_ID i)
{
  AUX_ID cur;
  BOOL need_chi = FALSE;
  BOOL need_mu = FALSE;
  OPERATOR opr = stmt->Opr();
  AUX_STAB_ENTRY *aux = _opt_stab->Aux_stab_entry(i);
  if (OPERATOR_is_store(opr)) {
    switch(opr) {
    case OPR_STID:
      cur = stmt->Lhs()->Aux_id();
      if (cur == i)
	break;
      if (ST_class(_opt_stab->Aux_stab_entry(cur)->St()) == CLASS_PREG)
	break;
      if (_opt_stab->Rule()->Aliased_Memop_By_Analysis( aux->Points_to(),
			      _opt_stab->Aux_stab_entry(cur)->Points_to()))
	need_chi = TRUE;
      break;
    case OPR_ISTORE:
    case OPR_MSTORE:
      cur = stmt->Lhs()->Ivar_occ()->Aux_id();
      if (_opt_stab->Rule()->Aliased_Memop(aux->Points_to(),
				stmt->Lhs()->Ivar_occ()->Points_to(),
				aux->Points_to()->Ty(),
				stmt->Ty()))
	need_chi = TRUE;
      break;
    default: ;
    }
  }
  else if (opr == OPR_RETURN) {
    if (! _opt_stab->Aux_stab_entry(i)->Points_to()->Local() ||
	_opt_stab->Local_static(i))
      need_mu = TRUE;
  }
  else if (opr == OPR_OPT_CHI && i == _opt_stab->Default_vsym())
    need_chi = TRUE;
  else if (OPERATOR_is_call(opr) || opr == OPR_ASM_STMT || opr == OPR_REGION ||
	   opr == OPR_FORWARD_BARRIER || opr == OPR_BACKWARD_BARRIER ||
	   opr == OPR_DEALLOCA || opr == OPR_OPT_CHI) {
    if (! _opt_stab->Aux_stab_entry(i)->Points_to()->Local() ||
	_opt_stab->Local_static(i)) {
      need_chi = TRUE;
      if (opr != OPR_OPT_CHI && opr != OPR_ASM_STMT)
	need_mu = TRUE;
    }
    else {
      // if it alias with any node in the chi list, then there is alias
      CHI_LIST_ITER chi_iter;
      CHI_NODE *cnode;
      FOR_ALL_NODE(cnode, chi_iter, Init(stmt->Chi_list())) {
        if (! cnode->Live())
	  continue;
        AUX_STAB_ENTRY *psym = _opt_stab->Aux_stab_entry(cnode->Aux_id());
        if (_opt_stab->Rule()->Aliased_Ofst_Rule(aux->Points_to(),
					         psym->Points_to())) {
	  need_chi = TRUE;
	  if (opr != OPR_OPT_CHI)
	    need_mu = TRUE;
	  break;
        }
      }
    }
  }

  if (need_mu) {
    if (stmt->Mu_list() == NULL) 
      stmt->Set_mu_list(CXX_NEW(MU_LIST, _htable->Mem_pool()));
    MU_NODE *newmu = stmt->Mu_list()->New_mu_node(i, _htable->Mem_pool());
    newmu->Set_OPND(_htable->Ssa()->Get_zero_version_CR(i, _opt_stab, 0));
  }
  if (need_chi) {
    CODEREP *chi_res;
    if (stmt->Chi_list() == NULL) 
      stmt->Set_chi_list(CXX_NEW(CHI_LIST, _htable->Mem_pool()));
    CHI_NODE *newchi = stmt->Chi_list()->New_chi_node(i, _htable->Mem_pool());
    newchi->Set_live(TRUE);
    newchi->Set_dse_dead(FALSE);
    AUX_STAB_ENTRY *sym = _opt_stab->Aux_stab_entry(i);
    MTYPE rtype = Mtype_from_mtype_class_and_size(sym->Mclass(), 
						  sym->Byte_size());
    MTYPE desc = rtype;
    chi_res = _htable->Add_def(i, -1, stmt, rtype, desc, 
			 _opt_stab->St_ofst(i), MTYPE_To_TY(rtype), 0, TRUE);
    chi_res->Set_flag(CF_DEF_BY_CHI);
    chi_res->Set_defchi(newchi);
    newchi->Set_RESULT(chi_res);
    newchi->Set_OPND(_htable->Ssa()->Get_zero_version_CR(i, _opt_stab, 0));
  }
}

// ====================================================================
// Update_mu_and_chi_list - only mu's and chi's at the statement level need
// to be updated
// ====================================================================
void
OPT_REVISE_SSA::Update_mu_and_chi_list(STMTREP *stmt)
{
  if (! stmt->Has_chi() && ! stmt->Has_mu())
    return;
  AUX_ID i;

  // this loop for pre-existing variables
  IDX_32_SET_ITER iter;
  iter.Init(_symbols_to_revise);
  for (i = iter.First_elem(); ! iter.Is_Empty(); i = iter.Next_elem()) {
    if (i >= _first_new_aux_id)
      break;
    if (_opt_stab->Aux_stab_entry(i)->Is_volatile())
      continue; // volatiles do not appear in any mu and chi
    Update_chi_list_for_old_var(stmt, i);
  }

  // this loop for new variables
  for (i = _first_new_aux_id; i <= _opt_stab->Lastidx(); i++) {
    if (_opt_stab->Aux_stab_entry(i)->Is_volatile())
      continue; // volatiles do not appear in any mu and chi
    Insert_mu_and_chi_list_for_new_var(stmt, i);
  }
}

// ====================================================================
// Create_EXTRACT_BITS - create an EXTRACT node 
// ====================================================================
CODEREP *
OPT_REVISE_SSA::Create_EXTRACT_BITS(INT bit_offset, INT bit_size, 
				    CODEREP *v, MTYPE rtype)
{
  CODEREP stack_cr; 
  stack_cr.Init_expr(OPCODE_make_op(OPR_EXTRACT_BITS, rtype, MTYPE_V), v);
  stack_cr.Set_op_bit_offset(bit_offset);
  stack_cr.Set_op_bit_size(bit_size);
  return _htable->Rehash(&stack_cr);
}

// ====================================================================
// Create_COMPOSE_BITS - create a COMPOSE node 
// ====================================================================
CODEREP *
OPT_REVISE_SSA::Create_COMPOSE_BITS(INT bit_offset, INT bit_size, 
				    CODEREP *v, CODEREP *rhs)
{
  CODEREP stack_cr; 
  stack_cr.Init_expr(OPCODE_make_op(OPR_COMPOSE_BITS, v->Dtyp(), MTYPE_V), v);
  stack_cr.Set_kid_count(2);
  stack_cr.Set_opnd(1, rhs);
  stack_cr.Set_op_bit_offset(bit_offset);
  stack_cr.Set_op_bit_size(bit_size);
  return _htable->Rehash(&stack_cr);
}

// ====================================================================
// Delete_chi - delete the chi node with the given aux_id from stmt's chi list
// ====================================================================
void 
OPT_REVISE_SSA::Delete_chi(AUX_ID aux_id, STMTREP *stmt)
{
  CHI_NODE *cnode, *prev_cnode;
  CHI_LIST_ITER chi_iter;

  chi_iter.Init(stmt->Chi_list());
  for (prev_cnode = NULL, cnode = chi_iter.First();
       ! chi_iter.Is_Empty();
       prev_cnode = cnode, cnode = chi_iter.Next()) {
    if (cnode->Aux_id() != aux_id)
      continue;
    stmt->Chi_list()->Remove(prev_cnode, cnode);
    stmt->Recompute_has_zver();
    return;
  }
}

// ====================================================================
// Form_extract - look for {I,}LDBITS in the tree and change them to
// EXTRACT_BITS. Return the rehashed coderep node whenever it is not the
// original node; otherwise return NULL.
// ====================================================================
CODEREP *
OPT_REVISE_SSA::Form_extract(CODEREP *cr)
{
  CODEREP *x, *x2;
  CODEREP *new_cr = Alloc_stack_cr(cr->Extra_ptrs_used());
  BOOL need_rehash;
  INT32 i;
  switch (cr->Kind()) {
  case CK_CONST:
  case CK_RCONST:
  case CK_LDA:
    return NULL;
  case CK_VAR:
    if (cr->Bit_field_valid()) {
      // generate a zero version of the new scalar variable
      x = _htable->Ssa()->Get_zero_version_CR(cr->Scalar_aux_id(), _opt_stab,0);
      AUX_STAB_ENTRY *sym = _opt_stab->Aux_stab_entry(cr->Scalar_aux_id());
      x->Set_dtyp(cr->Dtyp());
      x->Set_dsctyp(Mtype_from_mtype_class_and_size(MTYPE_type_class(cr->Dtyp()), sym->Byte_size()));
      return Create_EXTRACT_BITS(cr->Bit_offset(), cr->Bit_size(), x, cr->Dtyp());
    }
    return NULL;
  case CK_IVAR:
    x = Form_extract(cr->Ilod_base());
    if (x)
      cr->Set_ilod_base(x);
    if (cr->Opr() == OPR_MLOAD) {
      x2 = Form_extract(cr->Mload_size());
      if (x2)
        cr->Set_mload_size(x2);
    }
    else x2 = NULL;
    if (cr->Opr() != OPR_ILDBITS) {
      if (x || x2) { // need rehash
        new_cr->Copy(*cr);	
        new_cr->Set_istr_base(NULL);
        new_cr->Set_usecnt(0);
        if (x)
	  new_cr->Set_ilod_base(x);
        if (x2)
	  new_cr->Set_mload_size(x2);
        new_cr->Set_ivar_occ(cr->Ivar_occ());
        cr->DecUsecnt();
        return _htable->Rehash(new_cr);
      }
      return NULL;
    }
    // for the ILDBITS node, first form the ILOAD node
    new_cr->Copy(*cr);
    new_cr->Set_opr(OPR_ILOAD);
    new_cr->Set_istr_base(NULL);
    new_cr->Set_i_field_id(0);
    new_cr->Set_usecnt(0);
    if (x)
      new_cr->Set_ilod_base(x);
    new_cr->Set_ivar_occ(cr->Scalar_ivar_occ());
    // create a new mu node for the ivar
    new_cr->Set_ivar_mu_node(CXX_NEW(MU_NODE(*cr->Ivar_mu_node()),
				     _htable->Mem_pool()));
    // needs a zero version vsym for the ivar mu node
    new_cr->Ivar_mu_node()->Set_OPND(_htable->Ssa()->Get_zero_version_CR(new_cr->Ivar_occ()->Aux_id(), _opt_stab, 0));
    cr->DecUsecnt();
    x = _htable->Rehash(new_cr);
    return Create_EXTRACT_BITS(cr->I_bit_offset(), cr->I_bit_size(), x, 
			       cr->Dtyp());
  case CK_OP:
    need_rehash = FALSE;
    new_cr->Copy(*cr);
    new_cr->Set_usecnt(0);
    OPERATOR opr = cr->Opr();
    // call recursively
    for (i = 0; i < cr->Kid_count(); i++) {
      x = Form_extract(cr->Opnd(i));
      if (x) {
	need_rehash = TRUE;
	new_cr->Set_opnd(i, x);
      }
      else new_cr->Set_opnd(i, cr->Opnd(i));
    }
    if (need_rehash) {
      cr->DecUsecnt();
      return _htable->Rehash(new_cr);
    }
    return NULL;
  }
}

// ====================================================================
//  Form_extract_compose - do a pass over the program; at each statement that
//  has alias, insert mu and chi for the new scalar variables; translate
//  {I,}{LD,ST}BITS into EXTRACT_BITS and COMPOSE_BITS.
// ====================================================================
void
OPT_REVISE_SSA::Form_extract_compose(void)
{
  CFG_ITER cfg_iter(_cfg);
  BB_NODE *bb;
  INT32 i;
  CODEREP *x, *v;

  // visit all blocks 
  FOR_ALL_NODE( bb, cfg_iter, Init() ) {
    Update_phis(bb);

    // iterate thru each statement
    STMTREP_ITER stmt_iter(bb->Stmtlist());
    STMTREP *stmt;
    FOR_ALL_NODE(stmt, stmt_iter, Init()) {
      OPERATOR opr = stmt->Opr();
      CODEREP *rhs = stmt->Rhs();
      if (OPERATOR_is_call(opr) || opr == OPR_ASM_STMT) {
	for (i = 0; i < rhs->Kid_count(); i++) {
	  x = Form_extract(rhs->Opnd(i));
	  if (x)
	    rhs->Set_opnd(i, x);
	}
      }
      else {
        if (rhs) {
	  if (opr == OPR_PREFETCH) {
	    x = Form_extract(rhs->Ilod_base());
	    if (x)
	      rhs->Set_ilod_base(x);
	  }
	  else {
	    x = Form_extract(rhs);
	    if (x) {
	      stmt->Set_rhs(x);
	      rhs = x;
	    }
	  }
	}
      }

      if (OPERATOR_is_store(opr)) {
	CODEREP *lhs = stmt->Lhs();
        switch (opr) {
        case OPR_MSTORE:
	  x = Form_extract(lhs->Mstore_size());
	  if (x)
	    lhs->Set_mstore_size(x);
	  // fall thru
        case OPR_ISTORE:
        case OPR_ISTBITS:
	  x = Form_extract(lhs->Istr_base());
	  if (x)
	    lhs->Set_istr_base(x);
	  break;
        default: ;
        }
	
	// lower STBITS and ISTBITS
	if (opr == OPR_STBITS) {

	  // Add a new chi node for the old STBITS lhs symbol (pv 805267)
	  if (stmt->Chi_list() == NULL)
	    stmt->Set_chi_list(CXX_NEW(CHI_LIST, _htable->Mem_pool()));
	  CHI_NODE *newchi
	    = stmt->Chi_list()->New_chi_node(lhs->Aux_id(),
					     _htable->Mem_pool());
	  newchi->Set_live(TRUE);
	  newchi->Set_dse_dead(FALSE);
	  lhs->Set_flag(CF_DEF_BY_CHI);
	  lhs->Set_defchi(newchi);
	  newchi->Set_RESULT(lhs);
	  newchi->Set_OPND(_htable->Ssa()->Get_zero_version_CR(lhs->Aux_id(),
							       _opt_stab, 0));

	  // generate a load of the zero version of the new scalar variable
	  v = _htable->Ssa()->Get_zero_version_CR(lhs->Scalar_aux_id(),
						  _opt_stab, 0);
	  AUX_STAB_ENTRY *sym = _opt_stab->Aux_stab_entry(lhs->Scalar_aux_id());
	  v->Set_dtyp(lhs->Dtyp());
	  v->Set_dsctyp(Mtype_from_mtype_class_and_size(MTYPE_type_class(lhs->Dtyp()), sym->Byte_size()));
	  stmt->Set_rhs(Create_COMPOSE_BITS(lhs->Bit_offset(), lhs->Bit_size(), v, rhs));
	  // generate a new version of the new scalar variable
	  stmt->Set_lhs(_htable->Add_def(lhs->Scalar_aux_id(), -1, stmt, 
	    lhs->Dtyp(), lhs->Dsctyp(), lhs->Offset(), lhs->Lod_ty(), 0, TRUE));
	  stmt->Set_opr(OPR_STID);
	  if (v->Aux_id() < _first_new_aux_id)
	    Delete_chi(v->Aux_id(), stmt);
	} 
	else if (opr == OPR_ISTBITS) {
	  // first form the ILOAD node
	  CODEREP *new_cr = Alloc_stack_cr(lhs->Extra_ptrs_used());
	  new_cr->Copy(*lhs);
	  new_cr->Set_opr(OPR_ILOAD);
	  new_cr->Set_ilod_base(lhs->Istr_base());
	  new_cr->Set_istr_base(NULL);
	  new_cr->Set_ivar_defstmt(NULL);
	  new_cr->Set_i_field_id(0);
	  new_cr->Set_usecnt(1);
	  new_cr->Set_ivar_occ(lhs->Scalar_ivar_occ());
	  // create a new mu node for the ivar
	  MU_NODE *mu = CXX_NEW(MU_NODE, _htable->Mem_pool());
	  mu->Set_aux_id(lhs->Scalar_ivar_occ()->Aux_id());
	  // needs a zero version vsym for the ivar mu node
	  mu->Set_OPND(_htable->Ssa()->Get_zero_version_CR(new_cr->Ivar_occ()->Aux_id(), _opt_stab, 0));
	  new_cr->Set_ivar_mu_node(mu);
	  v = _htable->Rehash(new_cr);
	  stmt->Set_rhs(Create_COMPOSE_BITS(lhs->I_bit_offset(), lhs->I_bit_size(), v, rhs));

	  // generate a new version of the new ivar node 
	  stmt->Set_lhs(_htable->Add_idef(
			  OPCODE_make_op(OPR_ILOAD, lhs->Dtyp(), lhs->Dsctyp()),
			  lhs->Scalar_ivar_occ(), stmt, NULL/*mu*/,
			  lhs->Dtyp(), lhs->Dsctyp(), lhs->Ilod_ty(), 0, 
			  lhs->Offset(), (CODEREP *) lhs->Ilod_base_ty(), NULL, 
			  lhs->Istr_base()));
	  stmt->Set_opr(OPR_ISTORE);
	}
      }

      // insert mu's and chi's for the new scalar variables for this statement
      Update_mu_and_chi_list(stmt);
    }
  }
}

// ====================================================================
// Fold_lda_iloads - look for ILOADS/ILDBITS in the tree whose base is LDA,
// and change it to LDID/LDBITS. Return the rehashed coderep node whenever 
// it is not the original node; otherwise return NULL.
// ====================================================================
CODEREP *
OPT_REVISE_SSA::Fold_lda_iloads(CODEREP *cr)
{
  CODEREP *x, *x2;
  CODEREP *new_cr = Alloc_stack_cr(cr->Extra_ptrs_used());
  BOOL need_rehash;
  INT32 i;
  switch (cr->Kind()) {
  case CK_CONST:
  case CK_RCONST:
  case CK_LDA:
  case CK_VAR:
    return NULL;
  case CK_IVAR:
    x = Fold_lda_iloads(cr->Ilod_base());
    if (x)
      cr->Set_ilod_base(x);
    if (cr->Opr() == OPR_MLOAD) {
      x2 = Fold_lda_iloads(cr->Mload_size());
      if (x2)
        cr->Set_mload_size(x2);
    }
    else x2 = NULL;
    if (x || x2) { // need rehash
      new_cr->Copy(*cr);	
      new_cr->Set_istr_base(NULL);
      new_cr->Set_usecnt(0);
      if (x)
        new_cr->Set_ilod_base(x);
      if (x2)
        new_cr->Set_mload_size(x2);
      new_cr->Set_ivar_occ(cr->Ivar_occ());
      cr->DecUsecnt();
      return _htable->Rehash(new_cr);
    }
    if (cr->Ilod_base()->Kind() != CK_LDA || 
	cr->Is_ivar_volatile() ||
	cr->Opr() == OPR_PARM || 
	cr->Opr() == OPR_MLOAD)
      return NULL;
    // this indirect load can be folded
    // generate a load of the zero version of the new scalar variable
    x = _htable->Ssa()->Get_zero_version_CR(cr->Scalar_aux_id(), _opt_stab, 0);
    x->Set_dtyp(cr->Dtyp());
    x->Set_dsctyp(cr->Dsctyp());
    x->Set_lod_ty(TY_pointed(cr->Ilod_base_ty()));
    x->Set_field_id(cr->I_field_id());
    if (cr->Dsctyp() == MTYPE_BS) // cannot use offset from opt_stab
      x->Set_offset(cr->Offset()+cr->Ilod_base()->Offset());
    if (cr->Opr() == OPR_ILDBITS) 
      x->Set_bit_field_valid();
    cr->DecUsecnt();
    return x;
  case CK_OP:
    need_rehash = FALSE;
    new_cr->Copy(*cr);
    new_cr->Set_usecnt(0);
    OPERATOR opr = cr->Opr();
    // call recursively
    for (i = 0; i < cr->Kid_count(); i++) {
      x = Fold_lda_iloads(cr->Opnd(i));
      if (x) {
	need_rehash = TRUE;
	new_cr->Set_opnd(i, x);
      }
      else new_cr->Set_opnd(i, cr->Opnd(i));
    }
    if (need_rehash) {
      cr->DecUsecnt();
      return _htable->Rehash(new_cr);
    }
    return NULL;
  }
}

// ====================================================================
//  Fold_lda_indirects - do a pass over the program; at each statement that
//  has alias, insert mu and chi for the new scalar variables; fold
//  ISTORE or ISTBITS based on LDA to STID and STBITS.
// ====================================================================
void
OPT_REVISE_SSA::Fold_lda_indirects(void)
{
  CFG_ITER cfg_iter(_cfg);
  BB_NODE *bb;
  INT32 i;
  CODEREP *x, *v;

  // visit all blocks 
  FOR_ALL_NODE( bb, cfg_iter, Init() ) {
    Update_phis(bb);

    // iterate thru each statement
    STMTREP_ITER stmt_iter(bb->Stmtlist());
    STMTREP *stmt;
    FOR_ALL_NODE(stmt, stmt_iter, Init()) {
      OPERATOR opr = stmt->Opr();
      CODEREP *rhs = stmt->Rhs();
      if (OPERATOR_is_call(opr) || opr == OPR_ASM_STMT) {
	for (i = 0; i < rhs->Kid_count(); i++) {
	  x = Fold_lda_iloads(rhs->Opnd(i));
	  if (x)
	    rhs->Set_opnd(i, x);
	}
      }
      else {
        if (rhs) {
	  if (opr == OPR_PREFETCH) {
	    x = Fold_lda_iloads(rhs->Ilod_base());
	    if (x)
	      rhs->Set_ilod_base(x);
	  }
	  else {
	    x = Fold_lda_iloads(rhs);
	    if (x) {
	      stmt->Set_rhs(x);
	      rhs = x;
	    }
	  }
	}
      }

      if (OPERATOR_is_store(opr)) {
	CODEREP *lhs = stmt->Lhs();
	AUX_ID vaux;
        switch (opr) {
        case OPR_MSTORE:
	  x = Fold_lda_iloads(lhs->Mstore_size());
	  if (x)
	    lhs->Set_mstore_size(x);
	  x = Fold_lda_iloads(lhs->Istr_base());
	  if (x)
	    lhs->Set_istr_base(x);
	  break;
        case OPR_ISTORE:
        case OPR_ISTBITS:
	  if (lhs->Istr_base()->Kind() != CK_LDA) {
	    x = Fold_lda_iloads(lhs->Istr_base());
	    if (x)
	      lhs->Set_istr_base(x);
	    break;
	  }
	  if (lhs->Is_ivar_volatile())
	    break;
	  // this indirect can be folded
	  // first, generate a new version of the new scalar variable
	  vaux = lhs->Scalar_aux_id();
	  stmt->Set_lhs(_htable->Add_def(vaux, -1, stmt, 
			lhs->Dtyp(), lhs->Dsctyp(), 
			_opt_stab->St_ofst(vaux),
			TY_pointed(lhs->Ilod_base_ty()), lhs->I_field_id(), 
			TRUE));
	  if (lhs->Dsctyp() == MTYPE_BS) // cannot use offset from opt_stab
	    stmt->Lhs()->Set_offset(lhs->Offset()+lhs->Istr_base()->Offset());
	  if (stmt->Opr() == OPR_ISTORE)
	    stmt->Set_opr(OPR_STID);
	  else {
	    stmt->Set_opr(OPR_STBITS);
	    stmt->Lhs()->Set_bit_field_valid();
	  }
	  if (vaux < _first_new_aux_id)
	    Delete_chi(vaux, stmt);
	  break;
        default: ;
        }
      }

      // insert mu's and chi's for the new scalar variables for this statement
      Update_mu_and_chi_list(stmt);
    }
  }
}

// ====================================================================
// - - - - - - Step 3 stuff starts here - - - - - -
// ====================================================================

// ====================================================================
//  Rename_new_scalars - 
// ====================================================================
void
OPT_REVISE_SSA::Rename_new_scalars(COMP_UNIT *cunit)
{
  Rename_CODEMAP(cunit);
}

// ====================================================================
// - - - - - Drivers for this package for the two different tasks - - - - - 
// ====================================================================

// ====================================================================
//  Lower_to_extract_compose - 
// ====================================================================
void	       
COMP_UNIT::Lower_to_extract_compose(void)
{
  MEM_POOL revise_ssa_pool;

  OPT_POOL_Initialize(&revise_ssa_pool, "revise ssa pool", FALSE, OPT_LOWER_FLAG);
  OPT_POOL_Push(&revise_ssa_pool, OPT_LOWER_FLAG);

  {
    OPT_REVISE_SSA opt_revise_ssa(Opt_stab(), Htable(), Cfg(), 
				  &revise_ssa_pool, OPT_LOWER_FLAG);
    opt_revise_ssa.Find_scalars_from_lowering_bitfld();
    if (opt_revise_ssa.Has_bitfield()) {
      opt_revise_ssa.Form_extract_compose();
      opt_revise_ssa.Rename_new_scalars(this);
    }
  }

  OPT_POOL_Pop(&revise_ssa_pool, OPT_LOWER_FLAG);
  OPT_POOL_Delete(&revise_ssa_pool, OPT_LOWER_FLAG);

  if ( Get_Trace(TP_GLOBOPT, OPT_LOWER_FLAG)) {
    fprintf( TFile, "%sAfter COMP_UNIT::Lower_to_extract_compose\n%s",
             DBar, DBar );
    Cfg()->Print(TFile);
  }
}

// ====================================================================
//  Fold_lda_iload_istore - 
// ====================================================================
void	       
COMP_UNIT::Fold_lda_iload_istore(void)
{
  MEM_POOL revise_ssa_pool;

  OPT_POOL_Initialize(&revise_ssa_pool, "revise ssa pool", FALSE, FOLD_DUMP_FLAG);
  OPT_POOL_Push(&revise_ssa_pool, FOLD_DUMP_FLAG);

  {
    OPT_REVISE_SSA opt_revise_ssa(Opt_stab(), Htable(), Cfg(), 
				  &revise_ssa_pool, FOLD_DUMP_FLAG);
    opt_revise_ssa.Find_scalars_from_lda_indirects();
    if (opt_revise_ssa.Has_lda_indirect()) {
      opt_revise_ssa.Fold_lda_indirects();
      opt_revise_ssa.Rename_new_scalars(this);
    }
  }

  OPT_POOL_Pop(&revise_ssa_pool, FOLD_DUMP_FLAG);
  OPT_POOL_Delete(&revise_ssa_pool, FOLD_DUMP_FLAG);

  if ( Get_Trace(TP_GLOBOPT, FOLD_DUMP_FLAG)) {
    fprintf( TFile, "%sAfter COMP_UNIT::Fold_lda_iload_istore\n%s",
             DBar, DBar );
    Cfg()->Print(TFile);
  }
}
