//-*-c++-*-
// ====================================================================
// ====================================================================
//
// Module: opt_wn.cxx
// $Revision: 1.1.1.1 $
// $Date: 2002-05-22 20:06:52 $
// $Author: dsystem $
// $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/opt/opt_wn.cxx,v $
//
// Revision history:
//  12-SEP-94 shin - Original Version
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
// Description: Defines utilities for Optimizer
//
// ====================================================================
// ====================================================================


#ifdef USE_PCH
#include "opt_pch.h"
#endif // USE_PCH
#pragma hdrstop


#ifdef _KEEP_RCS_ID
#define opt_wn_CXX	"opt_wn.cxx"
static char *rcs_id = 	opt_wn_CXX"$Revision: 1.1.1.1 $";
#endif /* _KEEP_RCS_ID */

#include <sys/types.h>
#include <elf.h>         // for pu_info.h
#include "defs.h"
#include "tracing.h"
#include "mempool.h"
#include "opt_base.h"
#include "topcode.h"

#include "wn.h"
#include "wn_util.h"
#include "pu_info.h"
#include "stab.h"
#include "w2op.h"
#include "stblock.h"
#include "region_util.h"
#include "opt_wn.h"
#include "ir_reader.h"
#include "opt_sym.h"
#include "stab.h"
#include "opt_htable.h"
#include "opt_mu_chi.h"
#include "dep_graph.h"
#include "pf_cg.h"
#include "opt_alias_interface.h"
#include "opt_points_to.h"
#include "opt_alias_rule.h"
#include "config.h"		// Alias_Pointer_Parms
#include "config_opt.h"		// for Delay_U64_Lowering
#include "opt_cvtl_rule.h"
#include "opt_main.h"


STMT_ITER::STMT_ITER(WN *f)
{
  head = cur = f;
  for (tail = f; WN_next(tail) != NULL; tail = WN_next(tail));
}

void
STMT_ITER::Print( FILE *fp )
{
  for (First(); !Is_Empty(); Next()) {
    if (OPCODE_is_scf(WN_opcode(Cur())))
      fdump_wn_no_st( fp, Cur() );
    else
      fdump_tree_no_st( fp, Cur() );
  }
}

void
STMT_CONTAINER::Insert_before(WN *me, WN *wn)
{
  // and set head to nd if me is head
  WN *p, *n;
  if (me == NULL) return;
  p = WN_prev(me); n = me;
  if (p) {
    WN_next(p) = wn;
    WN_prev(wn) = p;
  }
  WN_prev(n) = wn;
  WN_next(wn) = n;
  if (me == head) head = wn;
}

void
STMT_CONTAINER::Insert_after (WN *me, WN *wn)
{
  // and set tail to wn if me is tail
  WN *p, *n;
  if (me == NULL) return;
  p = me; n = WN_next(me);
  WN_next(p) = wn;
  WN_prev(wn) = p;
  if (n) {
    WN_prev(n) = wn;
    WN_next(wn) = n;
  }
  if (me == tail) tail = wn;
}

void
STMT_CONTAINER::Insert_lst_before(WN *me, WN *wn_f, WN *wn_l)
{
  // and set head to nd if me is head
  WN *p, *n;
  if (me == NULL) return;
  if (wn_f == NULL) {
    // make sure that the last is null if the first was
    Is_True(wn_l == NULL, 
      ("STMT_CONTAINER::Insert_lst_before: wn_l non-null") );
    return;
  }
  p = WN_prev(me); n = me;
  if (p) WN_next(p) = wn_f;
  WN_prev(wn_f) = p;
  WN_prev(n) = wn_l;
  WN_next(wn_l) = n;
  if (me == head) head = wn_f;
}

void
STMT_CONTAINER::Insert_lst_after (WN *me, WN *wn_f, WN *wn_l)
{
  // and set tail to wn if me is tail
  WN *p, *n;
  if (me == NULL) return;
  if (wn_f == NULL) {
    // make sure that the last is null if the first was
    Is_True(wn_l == NULL, 
      ("STMT_CONTAINER::Insert_lst_after: wn_l non-null") );
    return;
  }
  p = me; n = WN_next(me);
  WN_next(p) = wn_f;
  WN_prev(wn_f) = p;
  if (n) WN_prev(n) = wn_l;
  WN_next(wn_l) = n;
  if (me == tail) tail = wn_l;
}

void
STMT_CONTAINER::Remove(WN *me)
{
  // and reset head/tail if it's head/tail
  WN *p, *n;
  if (me == NULL) return;
  p = WN_prev(me); n = WN_next(me);
  if (p) WN_next(p) = n;
  if (n) WN_prev(n) = p;

  WN_prev(me) = NULL;
  WN_next(me) = NULL;
  if (me == head) 
    head = n;
  if (me == tail)
    tail = p;
}

void
STMT_CONTAINER::Print(FILE *fp)
{
  STMT_ITER stmt_iter(head, tail);
  stmt_iter.Print(fp);
}


WN *WN_copy(WN *wn)
{
  return WN_COPY_Tree(wn);
}

MTYPE Mtype_from_class_size(MTYPE t1, MTYPE t2)
// return the mtype that is of the class of t1 but the size of t2;
// if t1 is neither signed or unsigned int, just return t1
{
  Is_True(t1 != MTYPE_BS && t2 != MTYPE_BS,
  	  ("Mtype_from_class_size: MTYPE_BS not handled here."));
  if ((MTYPE_type_class(t1) & MTYPE_CLASS_UNSIGNED_INTEGER) == 0)
    return t1;
  if (MTYPE_signed(t1))
    switch (MTYPE_size_best(t2)) {
    case 8: return MTYPE_I1;
    case 16: return MTYPE_I2;
    case 32: return MTYPE_I4;
    case 64: return MTYPE_I8;
    default: Is_True(FALSE, ("WN_mtype_from_class_size: unrecognized bit size for mtype"));
    }
  else
    switch (MTYPE_size_best(t2)) {
    case 8: return MTYPE_U1;
    case 16: return MTYPE_U2;
    case 32: return MTYPE_U4;
    case 64: return MTYPE_U8;
    default: Is_True(FALSE, ("WN_mtype_from_class_size: unrecognized bit size for mtype"));
    }
  return MTYPE_V;
}

BOOL WN_has_chi(const WN *wn, const REGION_LEVEL region_level)
{
  const OPCODE opc = WN_opcode(wn);
  if (opc == OPC_REGION) {
    Is_True(region_level > RL_UNKNOWN && region_level < RL_LAST,
	    ("WN_has_chi, region_level out of bounds"));
    RID *rid = REGION_get_rid(wn);
    Is_True(rid != NULL, ("WN_has_chi, NULL RID"));
    if (RID_TYPE_mp(rid) || RID_TYPE_eh(rid) ||
	// kludge for 7.2, see PV 457243
	region_level == RL_LNO_PREOPT || region_level == RL_PREOPT ||
	region_level == RL_IPA_PREOPT)
      return FALSE;
    else
      return TRUE;
  }
  return OPCODE_has_chi(opc);
}

BOOL OPERATOR_has_chi( OPERATOR opr )
{
  switch ( opr ) {
  case OPR_ISTORE:
  case OPR_ISTBITS:
  case OPR_MSTORE:
  case OPR_STID:
  case OPR_STBITS: 
  case OPR_CALL:
  case OPR_ICALL:
  case OPR_INTRINSIC_CALL:
  case OPR_IO:
  case OPR_FORWARD_BARRIER:
  case OPR_BACKWARD_BARRIER:
  case OPR_DEALLOCA:
  case OPR_OPT_CHI:
  case OPR_REGION: // black-box region only in CR or SR form
  case OPR_ASM_STMT:
    return TRUE;
  default:
    return FALSE;
  }
}

BOOL OPCODE_has_chi( OPCODE opc )
{
  return OPERATOR_has_chi(OPCODE_operator(opc));
}

// need the WN so we can tell a black box region (has mu) from a transparent
// MP region (no mu).
BOOL WN_has_mu( const WN *wn, const REGION_LEVEL region_level )
{
  const OPCODE opc = WN_opcode(wn);
  switch ( OPCODE_operator(opc) ) {
    case OPR_ILOAD:
    case OPR_ILDBITS:
    case OPR_MLOAD:
    case OPR_CALL:
    case OPR_ICALL:
    case OPR_INTRINSIC_CALL:
    case OPR_IO:
    case OPR_RETURN:
    case OPR_RETURN_VAL:
    case OPR_FORWARD_BARRIER:
    case OPR_BACKWARD_BARRIER:
    case OPR_REGION_EXIT:
      return TRUE;
    case OPR_REGION:	// this can be a black-box or MP region
    {
      Is_True(region_level > RL_UNKNOWN && region_level < RL_LAST,
	      ("WN_has_mu, region_level out of bounds"));
      RID *rid = REGION_get_rid(wn);
      Is_True(rid != NULL, ("WN_has_mu(), NULL rid"));
      if (RID_TYPE_mp(rid) || RID_TYPE_eh(rid) ||
	  // kludge for 7.2, see PV 457243
	  region_level == RL_LNO_PREOPT || region_level == RL_PREOPT ||
	  region_level == RL_IPA_PREOPT)
	return FALSE;
      else
	return TRUE;
    }
    case OPR_PARM:
      return (WN_Parm_By_Reference(wn));
    default:
      return FALSE;
  }
}

// version for CODEREP and STMTREP - uses OPERATOR
BOOL OPERATOR_has_mu( OPERATOR opr )
{
  switch ( opr ) {
    case OPR_ILOAD:
    case OPR_ILDBITS:
    case OPR_MLOAD:
    case OPR_CALL:
    case OPR_ICALL:
    case OPR_INTRINSIC_CALL:
    case OPR_IO:
    case OPR_RETURN:
    case OPR_RETURN_VAL:
    case OPR_FORWARD_BARRIER:
    case OPR_BACKWARD_BARRIER:
    case OPR_REGION: // black-box region only in CR or SR form
    case OPR_REGION_EXIT:
      return TRUE;
    case OPR_PARM:  // may or may not have mu
      return TRUE;
    default:
      return FALSE;
  }
}

BOOL OPCODE_has_mu( OPCODE opc )
{
  return OPERATOR_has_mu(OPCODE_operator(opc));
}

static WN_MAP  _wn_flag_map;

void WN_init_flags(MEM_POOL *pool)
{
  _wn_flag_map = WN_MAP32_Create(pool);
}

void WN_fini_flags(void) 
{
  WN_MAP_Delete(_wn_flag_map);
}

// Obtain the flags of the WN node 
//
INT32 Wn_flags(const WN *wn)
{
  return WN_MAP32_Get(_wn_flag_map, wn);
}

// Set the flags of the WN node
//
void Set_wn_flags(WN *wn, INT32 flags)
{
  WN_MAP32_Set(_wn_flag_map, wn, flags); 
}


//  LDID and STID may have their ST renamed to a VER.
//
BOOL WN_has_ver(const WN *wn)
{
  OPERATOR opr = WN_operator(wn);
  return (OPERATOR_is_scalar_load (opr) || OPERATOR_is_scalar_store (opr));
}

// The WN node with this operator need to be or has been converted to
// OPT_STAB index.
BOOL OPERATOR_has_aux(const OPERATOR opr)
{
  return (OPERATOR_is_scalar_load (opr) ||
	  OPERATOR_is_scalar_store (opr) ||
	  opr == OPR_LDA);
}

BOOL OPCODE_has_aux(const OPCODE opc)
{
  return OPERATOR_has_aux(OPCODE_operator(opc));
}

// The ST field of the WN node need to be or has been converted to
// OPT_STAB index.
extern BOOL WN_has_aux(const WN *wn)
{
  OPERATOR opr = WN_operator(wn);
  return OPERATOR_has_aux(opr);
}

ST *WN_sym(const WN *wn)
{
  Is_True((Wn_flags(wn) & WN_FLAG_ST_TYPE) == WN_ST_IS_SYM,
	  ("WN_st is not of type ST."));
  return WN_st(wn);
}


AUX_ID WN_aux(const WN *wn)
{
  Is_True((Wn_flags(wn) & WN_FLAG_ST_TYPE) == WN_ST_IS_AUX,
	  ("WN_st is not of type AUX."));
  return (AUX_ID) WN_st_idx(wn);
}


VER_ID WN_ver(const WN *wn)
{
  Is_True((Wn_flags(wn) & WN_FLAG_ST_TYPE) == WN_ST_IS_VER,
	  ("WN_st is not of type VER."));
  return (VER_ID) WN_st_idx(wn);
}


//  Keep track of the type in WN_st() field
//
void WN_set_aux(WN *wn, AUX_ID s)
{
#ifdef Is_True_On
  Set_wn_flags(wn, (Wn_flags(wn) & ~WN_FLAG_ST_TYPE) | WN_ST_IS_AUX);
#endif
  WN_st_idx(wn) = (ST_IDX) s;
}

//  Keep track of the type in WN_st() field
//
void WN_set_ver(WN *wn, VER_ID v)
{
#ifdef Is_True_On
  Set_wn_flags(wn, (Wn_flags(wn) & ~WN_FLAG_ST_TYPE) | WN_ST_IS_VER);
#endif
  WN_st_idx(wn) = (ST_IDX) v;
}

void WN_copy_stmap(WN *src, WN *dst)
{
  INT i;
  for (i = 0; i < WN_MAP_MAX; i++) { 
    if (Current_Map_Tab->_is_used[i]) {
      switch (Current_Map_Tab->_kind[i]) {
      case WN_MAP_KIND_VOIDP: {
        WN_MAP_Set(i, dst, WN_MAP_Get(i, src));
	break;
      }
      case WN_MAP_KIND_INT32: {
        WN_MAP32_Set(i, dst, WN_MAP32_Get(i, src));
	break;
      }
      case WN_MAP_KIND_INT64: {
        WN_MAP64_Set(i, dst, WN_MAP64_Get(i, src));
	break;
      }
      default:
	Is_True(FALSE, ("WN_copy_stmap: unknown map kind"));
      }
    }
  }
  if (!OPCODE_is_leaf(WN_opcode(src)))
    for (i = 0; i < WN_kid_count(src); i++)
      WN_copy_stmap(WN_kid(src, i), WN_kid(dst, i));
}


// Determine the register class of the WN node
INT32 Get_mtype_class(MTYPE mtype)
{
  INT32 mclass = MTYPE_type_class(mtype);
  // strip off unsigned-ness
  if (mclass == MTYPE_CLASS_UNSIGNED)
    mclass = MTYPE_CLASS_INTEGER;

  if (mclass == MTYPE_CLASS_UNSIGNED_INTEGER)
    mclass = MTYPE_CLASS_INTEGER;
  return mclass;
}

// ====================================================================
// Get a LDID opcode given an mtype.  This function assumes the
// caller wants a register sized result that is as small as possible
// (i.e., I1 type returns I4 register, but I8 type returns I8 register)
// and has the same signedness of the given type.
// ====================================================================

extern OPCODE 
Ldid_from_mtype( MTYPE mtype )
{
  switch ( mtype ) {
    case MTYPE_I1:	return OPC_I4I1LDID;
    case MTYPE_I2:	return OPC_I4I2LDID;
    case MTYPE_I4:	return OPC_I4I4LDID;
    case MTYPE_I8:	return OPC_I8I8LDID;
    case MTYPE_U1:	return OPC_U4U1LDID;
    case MTYPE_U2:	return OPC_U4U2LDID;
    case MTYPE_U4:	return OPC_U4U4LDID;
    case MTYPE_U8:	return OPC_U8U8LDID;
    case MTYPE_F4:	return OPC_F4F4LDID;
    case MTYPE_F8:	return OPC_F8F8LDID;
    case MTYPE_FQ:	return OPC_FQFQLDID;
    case MTYPE_C4:	return OPC_C4C4LDID;
    case MTYPE_C8:	return OPC_C8C8LDID;
    case MTYPE_CQ:	return OPC_CQCQLDID;

    case MTYPE_B:
    case MTYPE_F10:
    case MTYPE_F16:
    case MTYPE_STRING:
    case MTYPE_M:
    case MTYPE_V:
      FmtAssert( FALSE, ("Ldid_from_mtype: bad mtype: %s",
		 Mtype_Name(mtype)) );
      return OPCODE_UNKNOWN;

    case MTYPE_UNKNOWN:
    default:
      FmtAssert( FALSE, ("Ldid_from_mtype: unknown mtype: %d", mtype) );
      return OPCODE_UNKNOWN;
  }
}

// ====================================================================
// Get an MTYPE given an mtype class and size.
// ====================================================================

extern MTYPE
Mtype_from_mtype_class_and_size( INT mtype_class, INT bytes )
{
  // unsigned integer?
  if ( (mtype_class & MTYPE_CLASS_UNSIGNED) || 
       Only_Unsigned_64_Bit_Ops && ! Delay_U64_Lowering && (mtype_class & MTYPE_CLASS_INTEGER) ) {
    switch ( bytes ) {
      case 1: return MTYPE_U1;
      case 2: return MTYPE_U2;
      case 4: return MTYPE_U4;
      case 8: return MTYPE_U8;
    }
  }
  else if ( mtype_class & MTYPE_CLASS_INTEGER ) {
    switch ( bytes ) {
      case 1: return MTYPE_I1;
      case 2: return MTYPE_I2;
      case 4: return MTYPE_I4;
      case 8: return MTYPE_I8;
    }
  }
  else if ( mtype_class & MTYPE_CLASS_COMPLEX ) {
    switch ( bytes ) {
      case 8:  return MTYPE_C4;
      case 16: return MTYPE_C8;
      case 32: return MTYPE_CQ;
    }
  }
  else if ( mtype_class & MTYPE_CLASS_FLOAT ) {
    switch ( bytes ) {
      case 4:  return MTYPE_F4;
      case 8:  return MTYPE_F8;
      case 16: return MTYPE_FQ;
    }
  }

  return MTYPE_UNKNOWN;
}

// ====================================================================
// Get a LDID opcode given an mtype class and size.  This function 
// assumes the caller wants a register sized result that is as small as
// possible (i.e., INTEGER with size 1 returns I4 register, but INTEGER
// with size 8 return I8 register)
// ====================================================================

extern OPCODE 
Ldid_from_mtype_class_and_size( INT mtype_class, INT bytes )
{
  // unsigned integer?
  if ( (mtype_class & MTYPE_CLASS_UNSIGNED) || 
       Only_Unsigned_64_Bit_Ops && ! Delay_U64_Lowering && (mtype_class & MTYPE_CLASS_INTEGER) ) {
    switch ( bytes ) {
      case 1: return OPC_U4U1LDID;
      case 2: return OPC_U4U2LDID;
      case 4: return OPC_U4U4LDID;
      case 8: return OPC_U8U8LDID;
    }
  }
  else if ( mtype_class & MTYPE_CLASS_INTEGER ) {
    switch ( bytes ) {
      case 1: return OPC_I4I1LDID;
      case 2: return OPC_I4I2LDID;
      case 4: return OPC_I4I4LDID;
      case 8: return OPC_I8I8LDID;
    }
  }
  else if ( mtype_class & MTYPE_CLASS_COMPLEX ) {
    switch ( bytes ) {
      case 8:  return OPC_C4C4LDID;
      case 16: return OPC_C8C8LDID;
      case 32: return OPC_CQCQLDID;
    }
  }
  else if ( mtype_class & MTYPE_CLASS_FLOAT ) {
    switch ( bytes ) {
      case 4:  return OPC_F4F4LDID;
      case 8:  return OPC_F8F8LDID;
      case 16: return OPC_FQFQLDID;
    }
  }

  // if we get to here, it's bad news
  FmtAssert( FALSE, 
    ("Ldid_from_mtype_class_and_size: unknown class/size: %d/%d",
     mtype_class, bytes) );
  return OPCODE_UNKNOWN;
}
// ====================================================================
// Get a STID opcode given an mtype class and size.
// ====================================================================

extern OPCODE 
Stid_from_mtype_class_and_size( INT mtype_class, INT bytes )
{
  // unsigned integer?
  if ( (mtype_class & MTYPE_CLASS_UNSIGNED) || 
       Only_Unsigned_64_Bit_Ops && ! Delay_U64_Lowering && (mtype_class & MTYPE_CLASS_INTEGER) ) {
    switch ( bytes ) {
      case 1: return OPC_U1STID;
      case 2: return OPC_U2STID;
      case 4: return OPC_U4STID;
      case 8: return OPC_U8STID;
    }
  }
  else if ( mtype_class & MTYPE_CLASS_INTEGER ) {
    switch ( bytes ) {
      case 1: return OPC_I1STID;
      case 2: return OPC_I2STID;
      case 4: return OPC_I4STID;
      case 8: return OPC_I8STID;
    }
  }
  else if ( mtype_class & MTYPE_CLASS_COMPLEX ) {
    switch ( bytes ) {
      case 8:  return OPC_C4STID;
      case 16: return OPC_C8STID;
      case 32: return OPC_CQSTID;
    }
  }
  else if ( mtype_class & MTYPE_CLASS_FLOAT ) {
    switch ( bytes ) {
      case 4:  return OPC_F4STID;
      case 8:  return OPC_F8STID;
      case 16: return OPC_FQSTID;
    }
  }

  // if we get to here, it's bad news
  FmtAssert( FALSE, 
    ("Stid_from_mtype_class_and_size: unknown class/size: %d/%d",
     mtype_class, bytes) );
  return OPCODE_UNKNOWN;
}


// Initialize the LNO information for the main emitter.
void
Init_lno_info_for_main_emitter(void)
{
  if (Current_Dep_Graph != NULL)
    Current_Dep_Graph->Clear_Map();
}


// Dump the dependence graph (for debugging)
void
Print_dep_graph(FILE *fp)
{
  if (Current_Dep_Graph != NULL)
    Current_Dep_Graph->Print(fp);
}


// Obtain the LNO dep graph vertex id
INT32
WN_get_dep_graph_vertex(WN *wn)
{
  if (Current_Dep_Graph != NULL)
    return Current_Dep_Graph->Get_Vertex(wn);
  else
    return 0;
}


// Detach the WN node from the LNO dep graph.
void
WN_detach_wn_from_dep_graph(INT32 vertex)
{
  if (Current_Dep_Graph != NULL && vertex != 0)
    Current_Dep_Graph->Clear_Map_For_Vertex((VINDEX16) vertex);
}


// Add LNO info the WN node.
void
WN_add_lno_info(WN *wn, CODEREP *cr)
{
  Is_True(OPCODE_is_load(WN_opcode(wn)) || OPCODE_is_store(WN_opcode(wn)),
	  ("opcode is not load/store."));
  if (Current_Dep_Graph != NULL && cr->Kind() == CK_IVAR) {
    VINDEX16 vertex = OPCODE_is_load(WN_opcode(wn)) ? 
      cr->Ivar_occ()->Lno_dep_vertex_load() :
      cr->Ivar_occ()->Lno_dep_vertex_store() ;
    if (vertex != 0 && Current_Dep_Graph != NULL) {
      if (Current_Dep_Graph->Get_Wn(vertex) == NULL)
	Current_Dep_Graph->Set_Wn(vertex, wn);
      else {
	VINDEX16 newvertex = Current_Dep_Graph->Add_Vertex(wn);
	BOOL ok = Current_Dep_Graph->Copy_Vertex(vertex, newvertex);
	if (!ok) {
	  Current_Dep_Graph->Erase_Graph();
	  Current_Dep_Graph = NULL;
	}
      }
    }
  }
  PF_POINTER *pf;
  if ((pf = cr->Ivar_occ()->Pf_pointer()) != NULL) {
    if (!VISITED_EM(pf)) {
      SET_VISITED_EM(pf);
      if (PF_PTR_wn_pref_1L(pf) != NULL) {
	PF_PTR_wn_pref_1L(pf) = ((STMTREP *) PF_PTR_wn_pref_1L(pf))->Prefetch_wn();
	WN_MAP_Set(WN_MAP_PREFETCH, PF_PTR_wn_pref_1L(pf), pf);
      }
      if (PF_PTR_wn_pref_2L(pf) != NULL) {
	PF_PTR_wn_pref_2L(pf) = ((STMTREP *) PF_PTR_wn_pref_2L(pf))->Prefetch_wn();
	WN_MAP_Set(WN_MAP_PREFETCH, PF_PTR_wn_pref_2L(pf), pf);
      }
      WN_MAP_Set(WN_MAP_PREFETCH, wn, pf);
    }
  }
}


// Need to clone the LNO dep graph vertex when we clone the WN node
void
WN_dup_dep_vertex(WN *oldwn, WN *newwn)
{
  Is_True(OPCODE_is_load(WN_opcode(oldwn)) || OPCODE_is_store(WN_opcode(oldwn)),
	  ("opcode is not load/store."));
  Is_True(OPCODE_is_load(WN_opcode(newwn)) || OPCODE_is_store(WN_opcode(newwn)),
	  ("opcode is not load/store."));
  if (Current_Dep_Graph != NULL) {
    VINDEX16 vertex = Current_Dep_Graph->Get_Vertex(oldwn);
    if (vertex != 0) {
      VINDEX16 newvertex = Current_Dep_Graph->Add_Vertex(newwn);
      BOOL ok = Current_Dep_Graph->Copy_Vertex(vertex, newvertex);
      if (!ok) {
	Current_Dep_Graph->Erase_Graph();
	Current_Dep_Graph = NULL;
      }
    }
  }
}


//  Obtain the PF pointer
PF_POINTER *WN_get_pf_pointer(WN *wn)
{
  return (PF_POINTER *) WN_MAP_Get (WN_MAP_PREFETCH, wn);
}


//  Print the content of the PF_POINTER (for debugging)
void Print_pf_pointer(FILE *fp, PF_POINTER *p)
{
  fprintf(fp, "\tpref1=0x%p trip1=%d ", PF_PTR_wn_pref_1L(p), PF_PTR_lrnum_1L(p));
  fprintf(fp, "pref2=0x%p trip2=%d\n", PF_PTR_wn_pref_2L(p), PF_PTR_lrnum_2L(p));
}


// If a STRUCT contains both volatile and non-volatile fields, then
// then the entire STRUCT should be treated at volatile for the
// purpose of DSE and DCE.  But, the STRUCT should be treated as
// non-volatile for the purpose of generating MU and CHI nodes.
// (Addition of the field_id may make this a non-issue.)


// Check if a LDID/STID contains any volatility
BOOL
Lod_TY_is_volatile(TY_IDX ty)
{
  if (ty == TY_IDX_ZERO) return FALSE;
  if (TY_is_volatile(ty)) return TRUE;
  if (TY_kind(ty) == KIND_STRUCT) {
    if (!TY_fld (ty).Is_Null ()) {
      FLD_ITER fld_iter = Make_fld_iter (TY_fld (ty));
      do {
	TY_IDX fld_ty = FLD_type (fld_iter);
	if (Lod_TY_is_volatile(fld_ty))
	  return TRUE;
      } while (!FLD_last_field (fld_iter++));
    }
  }
  return FALSE;
}


// Check if a ILOAD/ISTORE accesses/contains any volatility
BOOL
Ilod_TY_is_volatile(TY_IDX ty)
{
  if (ty == TY_IDX_ZERO) return FALSE;
  if (TY_is_volatile(ty)) return TRUE;
  return Lod_TY_is_volatile(TY_pointed(ty));
}


// Obtain the constant value from the WHIRL and present it in INT64
INT64
WN_get_const_val(WN *wn)
{
  Is_True(WN_operator(wn) == OPR_INTCONST,
          ("WN_get_const_val: must be OPR_INTCONST"));
  union {
    mINT64  i8;
    mUINT64 u8;
    struct {
      mINT32  hi;
      mINT32  lo;
    } i4;
    struct {
      mUINT32  uhi;
      mUINT32  ulo;
    } u4;
    struct {
      mINT16  s4;
      mINT16  s3;
      mINT16  s2;
      mINT16  s1;
    } i2;
    struct {
      mUINT16 us4;
      mUINT16 us3;
      mUINT16 us2;
      mUINT16 us1;
    } u2;
    struct {
      mINT8  b8;
      mINT8  b7;
      mINT8  b6;
      mINT8  b5;
      mINT8  b4;
      mINT8  b3;
      mINT8  b2;
      mINT8  b1;
    } i1;
    struct {
      mINT8 ub8;
      mINT8 ub7;
      mINT8 ub6;
      mINT8 ub5;
      mINT8 ub4;
      mINT8 ub3;
      mINT8 ub2;
      mINT8 ub1;
    } u1;
  } val;

  val.i8 = WN_const_val(wn);
#if 0
  MTYPE rtype = WN_rtype(wn);
  switch (rtype) {
  case MTYPE_I1: 
    return (INT64) val.i1.b1;
  case MTYPE_I2: 
    return (INT64) val.i2.s1;
  case MTYPE_I4: 
    return (INT64) val.i4.lo;
  case MTYPE_I8: 
    return (INT64) val.i8;
  case MTYPE_U1: 
    return (INT64) val.u1.ub1;
  case MTYPE_U2: 
    return (INT64) val.u2.us1;
  case MTYPE_U4:
    return (INT64) val.u4.ulo;
  case MTYPE_U8: 
    return (INT64) val.u8;
  }
#endif
  return val.i8;
}

// for CODEREP
UINT Actual_data_size(CODEREP *cr, OPT_STAB *opt_stab, INT &signess)
{
  signess = 0;
  MTYPE  rtype = cr->Dtyp();
  INT    actual_size;

  if ((MTYPE_type_class(rtype) & MTYPE_CLASS_INTEGER) == 0)
    return MTYPE_size_min(rtype);

  switch (cr->Kind()) {
  case CK_CONST:
    {
      INT64 val = cr->Const_val();
      if ( (val & 0xFFFFFFFFFFFFFF80ll) == 0 ) {
	signess |= (SIGN_1_EXTD | SIGN_0_EXTD);
	return 8;
      } else if ( (val & 0xFFFFFFFFFFFFFF00ll) == 0 ) {
	signess |= SIGN_0_EXTD;
	return 8;
      } else if ( (val & 0xFFFFFFFFFFFFFF80ll) == 0xFFFFFFFFFFFFFF80ll ) {
	signess |= SIGN_1_EXTD;
	return 8;
      } else if ( (val & 0xFFFFFFFFFFFF8000ll) == 0 ) {
	signess |= (SIGN_1_EXTD | SIGN_0_EXTD);
	return 16;
      } else if ( (val & 0xFFFFFFFFFFFF0000ll) == 0 ) {
	signess |= SIGN_0_EXTD;
	return 16;
      } else if ( (val & 0xFFFFFFFFFFFF8000ll) == 0xFFFFFFFFFFFF8000ll ) {
	signess |= SIGN_1_EXTD;
	return 16;
      } else if ( (val & 0xFFFFFFFF80000000ll) == 0 ) {
	signess |= (SIGN_1_EXTD | SIGN_0_EXTD);
        return 32;
      } else if ( (val & 0xFFFFFFFF00000000ll) == 0 ) {
	signess |= SIGN_0_EXTD;
	return 32;
      } else if ( (val & 0xFFFFFFFF80000000ll) == 0xFFFFFFFF80000000ll ) {
	signess |= SIGN_1_EXTD;
      	return 32;
      }
      return MTYPE_size_min(rtype);
    }
  case CK_OP:
    {
      switch ( cr->Opr() ) {
      case OPR_CVTL:
	if (MTYPE_is_signed(rtype)) signess |= SIGN_1_EXTD;
	if (MTYPE_is_unsigned(rtype)) signess |= SIGN_0_EXTD;
	return cr->Offset();
      case OPR_CVT:
	{
	  switch ( cr->Op() ) {
	  case OPC_I4I8CVT:
	  case OPC_U4I8CVT:
	  case OPC_I4U8CVT:
	  case OPC_U4U8CVT:
	    signess |= SIGN_1_EXTD;
	    return MTYPE_size_min(rtype);
	  case OPC_I8U4CVT:
	  case OPC_U8U4CVT:
	    signess |= SIGN_0_EXTD;
	    return MTYPE_size_min(cr->Dsctyp());
	  default:
	    break;
	  }
	  return MTYPE_size_min(rtype);
	}
      case OPR_BAND:
	{
	  INT k0s = 0;
	  INT k1s = 0;
	  actual_size = MTYPE_size_min(rtype);
	  INT kid0_size = Actual_data_size(cr->Opnd(0), opt_stab, k0s);
	  INT kid1_size = Actual_data_size(cr->Opnd(1), opt_stab, k1s);
	  if (k0s & SIGN_0_EXTD) actual_size = MIN(actual_size, kid0_size);
	  if (k1s & SIGN_0_EXTD) actual_size = MIN(actual_size, kid1_size);
	  if (actual_size < MTYPE_size_min(rtype)) {
	    signess |= SIGN_0_EXTD;
	    return actual_size;
	  }
	  return MTYPE_size_min(rtype);
	}
      case OPR_ASHR:
      case OPR_LSHR:
	{
	  // The kid1 contains the number of bits.  
	  CODEREP *bits = cr->Opnd(1);
	  
	  // skip if kid 1 is not a constant.
	  if ( bits->Kind() == CK_CONST ) {
	    UINT bit_cnt = bits->Const_val();
	    if (MTYPE_size_min(rtype) == 32)
	      bit_cnt &= 0x1F;  // use the low 5 bits
	    else
	      bit_cnt &= 0x3F;
	    
	    INT ks = 0;
	    actual_size = Actual_data_size(cr->Opnd(0), opt_stab, ks);
	    actual_size -= bit_cnt;
	    if (actual_size < 0) actual_size = 0;
	    if (cr->Opr() == OPR_ASHR) {
	      signess |= SIGN_1_EXTD;
	    } else {
	      signess |= SIGN_0_EXTD;
	    }
	    if (actual_size > MTYPE_size_min(rtype)) // pv 364274
	      actual_size = MTYPE_size_min(rtype);
	    return actual_size;
	  }
	  break;
	}
      default:
	;
      }
      break;
    }
  case CK_VAR:
    { 
      AUX_STAB_ENTRY *aux_entry = opt_stab->Aux_stab_entry(cr->Aux_id());
      if (ST_class(aux_entry->St()) == CLASS_PREG) {
	if (aux_entry->Value_size() > 0) {
	  if (aux_entry->Is_sign_extd()) signess |= SIGN_1_EXTD;
	  if (aux_entry->Is_zero_extd()) signess |= SIGN_0_EXTD;
	  return aux_entry->Value_size();
	} else 
	  return MTYPE_size_min(rtype);
      } else {
	if (cr->Is_sign_extd()) 
	  signess |= SIGN_1_EXTD;
	else 
	  signess |= SIGN_0_EXTD;
	return aux_entry->Bit_size() ?
	    cr->Bit_size () : MTYPE_size_min(cr->Dsctyp());
      }
    }
  case CK_IVAR:
    {
      if (OPERATOR_is_scalar_iload (cr->Opr())) {
	if (cr->Is_sign_extd())
	  signess |= SIGN_1_EXTD;
	else 
	  signess |= SIGN_0_EXTD;
	return cr->Opr() == OPR_ILDBITS ?
	    cr->I_bit_size () : MTYPE_size_min(cr->Dsctyp());
      }
    }
  }

  return MTYPE_size_min(rtype);
}


// Find the type to use for Create_identity_assignment()
// Returns null if a reasonable type for a ldid/stid cannot be
// created.
//
extern TY_IDX 
Identity_assignment_type( AUX_STAB_ENTRY *sym )
{
  ST     *st  = sym->St();
  TY_IDX  ty = ST_type(st);

  // The following is a heuristic:
  //  if we try to get one element of an one-dimension or multi-dimension  array,
  //  use its element type!
  //
  while (TY_kind(ty) == KIND_ARRAY && sym->Byte_size() < TY_size(ty))
    ty = TY_AR_etype(ty);

  // the size needs to match
  if ( sym->Byte_size() != TY_size(ty) )
    return TY_IDX_ZERO;

  // if we don't have a simple type, i.e., we have struct/class,
  // determine if we can substitute a predefined type if it has
  // same characteristics (alignment,signedness,etc.)
  if ( ! Is_Simple_Type( ty ) ) {
    MTYPE mtype = Mtype_from_mtype_class_and_size(sym->Mclass(),
						  sym->Byte_size());

    if ( mtype == MTYPE_UNKNOWN )
      return TY_IDX_ZERO;

    TY_IDX newty = MTYPE_To_TY(mtype);

    // check alignment of replaced type
    if ( TY_align(ty) == TY_align(newty) )
      ty = newty;
    else
      return TY_IDX_ZERO;
  }

  return ty;
}


// Create an assignment of the form "i = i" for the symbol
//
WN *
Create_identity_assignment(AUX_STAB_ENTRY *sym, AUX_ID aux_id, TY_IDX ty)
{
  ST          *st  = sym->St();
  const OPCODE ldidop = Ldid_from_mtype_class_and_size(sym->Mclass(),
                                                       sym->Byte_size());
  const OPCODE stidop = Stid_from_mtype_class_and_size(sym->Mclass(),
                                                       sym->Byte_size());

  WN *rhs = WN_CreateLdid( ldidop, sym->St_ofst(), st, ty );
  WN *copy = WN_CreateStid( stidop, sym->St_ofst(), st, ty, rhs);

  if (sym->Bit_size () > 0) {
    if (sym->Field_id() == 0) { 
      WN_set_operator (rhs, OPR_LDBITS);
      WN_set_bit_offset_size (rhs, sym->Bit_ofst (), sym->Bit_size ());
  
      WN_set_operator (copy, OPR_STBITS);
      WN_set_bit_offset_size (copy, sym->Bit_ofst (), sym->Bit_size ());
    }
    else { // if field id != 0, then it is MTYPE_BS, not LD_BITS
      WN_set_desc(rhs, MTYPE_BS);
      WN_set_desc(copy, MTYPE_BS);
    }
  }
  WN_set_aux(rhs, aux_id);
  WN_set_aux(copy, aux_id);
  return copy;
}  



//  Obtain the mod-ref information from the uplevel procedure variable list
//
READ_WRITE 
Get_MP_modref(const WN *pragma_list, const POINTS_TO *pt,
	      const ALIAS_RULE *rule)
{
  Warn_todo("Get_MP_modref:  need to reimplement once the problem is fully understood.");

  POINTS_TO pt2;
  ST *st, *base;
  mINT64 ofst;

  // Fix 366737.
  // Hack to make mp-x6.f work!  Assume all parameter-addr are required by a MP-call.
  // This won't be necessary once we changed the F-param strategy after MR.
  if (IS_FORTRAN && pt->Const() && pt->Base_is_fixed() && ST_sclass(pt->Base()) == SCLASS_FORMAL)
    return READ;
  
  for (WN *wn = WN_first(pragma_list); wn != NULL; wn = WN_next(wn)) {
    Is_True(WN_pragma(wn) == WN_PRAGMA_ACCESSED_ID, ("Get_MP_modref:  not WN_PRAGMA_ACCESSED_ID."));
    st = WN_st(wn);
    if (st != NULL) {
      if ((WN_pragma_arg2(wn) & (ACCESSED_LOAD|ACCESSED_STORE)) != 0) {
	pt2.Init();
	pt2.Set_expr_kind(EXPR_IS_ADDR);
	pt2.Set_ofst_kind(OFST_IS_FIXED);
	pt2.Set_base_kind(BASE_IS_FIXED);
	Expand_ST_into_base_and_ofst(st, 0, &base, &ofst);
	pt2.Set_base(base);
	pt2.Set_byte_ofst(ofst);
	pt2.Set_byte_size(TY_size(ST_type(st)));
	pt2.Set_bit_ofst_size(0, 0);
	pt2.Set_named();
	// Fix 366737.
	// Hack to make mp-x6.f work!  Assume all parameter-addr are required by a MP-call.
	// This won't be necessary once we changed the F-param strategy after MR.
	if (rule->Aliased_Memop(pt, &pt2)) {
	  // hack to make it work!
	  if (IS_FORTRAN && ST_sclass(st) == SCLASS_FORMAL) 
	    return READ;
	  else
	    return READ_AND_WRITE;
	}
      }
      if ((WN_pragma_arg2(wn) & (ACCESSED_ILOAD|ACCESSED_ISTORE)) != 0) {
	pt2.Init();
	pt2.Set_expr_kind(EXPR_IS_ADDR);
	pt2.Set_ofst_kind(OFST_IS_FIXED);
	pt2.Set_base_kind(BASE_IS_FIXED);
	if (IS_FORTRAN && ST_sclass(st) == SCLASS_FORMAL && Alias_Pointer_Parms) {
	  pt2.Set_F_param();
	  pt2.Set_based_sym(st);
	  pt2.Set_base_kind(BASE_IS_UNKNOWN);
	  pt2.Set_global();
	  pt2.Set_named();   // For the Ragnarok option
	}
	if (rule->Aliased_Memop(pt, &pt2)) {
	  return READ_AND_WRITE;
	}
      }
    }
  }
  return NO_READ_NO_WRITE;
}


// Obtain the accessed id list for the nested procedure
//
WN *
Get_MP_accessed_id_list(const ST *st)
{
  PU_Info *pu_info;
  for (pu_info = PU_Info_child(Current_PU_Info);
       pu_info != NULL;
       pu_info = PU_Info_next(pu_info)) {
    if (PU_Info_proc_sym(pu_info) == ST_st_idx(st) &&
	PU_Info_state(pu_info, WT_TREE) == Subsect_InMem)
      return WN_func_varrefs(PU_Info_tree_ptr(pu_info));
  }
  return NULL;
}

// Check if we allow this opcode to be copy-propagated
BOOL 
Op_can_be_propagated(OPCODE op, OPT_PHASE phase)
{
  // Do not copy propagate things that LNO cannot handle in dependency tests
  // in preopt
  // TODO: need to remove this code after LNO is fixed to handle it in 
  // dependency analysis
  if (phase != MAINOPT_PHASE && !WOPT_Enable_Copy_Prop_LNO_Ops &&
      MTYPE_IS_INTEGER(OPCODE_rtype(op)) &&
      (OPCODE_operator(op) == OPR_DIV || OPCODE_operator(op) == OPR_REM)) 
    return FALSE;

  OPERATOR opr = OPCODE_operator(op);
  if (!WOPT_Enable_CSE_FP_comparison &&
      (opr == OPR_EQ || opr == OPR_NE ||
       opr == OPR_LT || opr == OPR_LE || 
       opr == OPR_GT || opr == OPR_GE) &&
      MTYPE_is_float(OPCODE_desc(op)))
    return FALSE;

  return TRUE;
}

// tell if the region has the given pragma
extern BOOL
Is_region_with_pragma(WN *wn, WN_PRAGMA_ID pragma_id)
{
  wn = WN_region_pragmas(wn);
  STMT_ITER stmt_iter;
  WN *stmt;

  //  Iterate through each statement
  FOR_ALL_ELEM (stmt, stmt_iter, Init(WN_first(wn), WN_last(wn))) {
    if (WN_operator(stmt) == OPR_PRAGMA &&
        (WN_pragma(stmt) == pragma_id))
      return TRUE;
  }
  return FALSE;
}


BOOL Is_hi_sign_extended(MTYPE result_ty, MTYPE desc_ty)
{
  Is_True(MTYPE_is_integral(result_ty),
          ("Is_hi_sign_extended: handles integral type only"));
  if (MTYPE_size_min(desc_ty) < MTYPE_size_min(result_ty) &&
      (MTYPE_size_min(result_ty) == MTYPE_size_min(MTYPE_I4) ||
       MTYPE_is_signed(result_ty)))
    return TRUE;

  if (MTYPE_is_signed(result_ty)) return TRUE;

  if (MTYPE_size_min(result_ty) == MTYPE_size_min(MTYPE_I4)) return TRUE;

  return FALSE;
}

BOOL Is_lo_sign_extended(MTYPE result_ty, MTYPE desc_ty)
{
  Is_True(MTYPE_is_integral(result_ty),
          ("Is_lo_sign_extended: handles integral type only"));
  if (MTYPE_size_min(desc_ty) < MTYPE_size_min(result_ty) &&
      MTYPE_is_signed(desc_ty))
    return TRUE;

  if (MTYPE_is_signed(result_ty)) return TRUE;

  return FALSE;
}

// this routine determines the signness, and use the size from the lod_typ
MTYPE
Type_for_saved_load(BOOL hi_ever_sign_extended,
                    BOOL lo_ever_sign_extended,
                    MTYPE lod_type)
{
  if (hi_ever_sign_extended) {
    if (lo_ever_sign_extended) {
      if (MTYPE_size_min(lod_type) == MTYPE_size_min(MTYPE_I4))
        return MTYPE_I4;
      else
        return MTYPE_I8;
    }
    else {
      if (MTYPE_size_min(lod_type) == MTYPE_size_min(MTYPE_I4))
        return MTYPE_U4;
      else
        return MTYPE_U8;
    }
  }
  Is_True(lo_ever_sign_extended == FALSE,
          ("Type_for_saved_load: do not expect to see lo_ever_sign_extended"));
  if (MTYPE_size_min(lod_type) == MTYPE_size_min(MTYPE_I4))
    return MTYPE_U4;
  else
    return MTYPE_U8;
}

void
Connect_cr_wn(WN_MAP *cr_wn_map, CODEREP *cr, WN *wn)
{
  WN_MAP_Set( *cr_wn_map, wn, cr );
}


// Return TRUE for "fake" operator.   Fake operator are WHIRL statements
// are implemented as expression in CODEMAP.
BOOL OPERATOR_is_fake(OPERATOR oper) 
{
  return (OPERATOR_is_call(oper) ||
	  oper == OPR_FORWARD_BARRIER ||
	  oper == OPR_BACKWARD_BARRIER ||
	  oper == OPR_ASM_STMT ||
	  oper == OPR_DEALLOCA); 
}

BOOL OPCODE_is_fake(OPCODE opc)
{
  return OPERATOR_is_fake(OPCODE_operator(opc));
}

BOOL OPERATOR_is_volatile(OPERATOR oper)
{
  return (oper == OPR_ALLOCA || 
	  oper == OPR_DEALLOCA ||
	  oper == OPR_FORWARD_BARRIER ||
	  oper == OPR_BACKWARD_BARRIER ||
	  // ASM_STMT shouldn't really be volatile; it should be able
	  // to move, be deleted if it's dead, etc. It is listed here
	  // as a hacky workaround for 753832, in which we try to do
	  // PRE on the ASM_INPUT kids of ASM_STMT.
	  oper == OPR_ASM_STMT ||
	  oper == OPR_ASM_INPUT ||
	  OPERATOR_is_call(oper));
}


BOOL OPCODE_is_volatile(OPCODE opc)
{
  return OPERATOR_is_volatile(OPCODE_operator(opc));
}
