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


/* =======================================================================
 * =======================================================================
 *
 *  Module: ebo_util.h
 *  $Revision: 1.1.1.1 $
 *  $Date: 2002-05-22 20:06:24 $
 *  $Author: dsystem $
 *  $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/cg/ebo_util.h,v $
 *
 *  Revision comments:
 *
 *  17-June-1998 - Initial version
 *
 *  Description:
 *  ============
 *
 *  EBO_tn_available
 *	Return TRUE iff the TN associated with the tninfo entry is
 *	currently available for use.
 *
 *  OP_effectively_copy
 *	Return TRUE iff <op> is a copy operation.
 *
 *  EBO_hash_op
 *	This routine will generate a hash value for the OP.
 *
 *  add_to_hash_table
 *	This routine will add a new expression to the hash table.
 *	While doing so, it will create defining EBO_TN_INFO entries
 *	for the result of the OP.
 *
 *  tn_registers_identical
 *	This routine compares 2 TN's to determine if they represent the
 *	same register.
 *
 *  locate_opinfo_entry
 *	This routine will locate the EBO_OP_INFO entry that
 *	is associated with a particular EBO_TN_INFO entry.
 *	It is used to locate the OP that defines the current value
 *	that is in a TN.
 *
 *  mark_tn_live_into_BB
 *	This routine performs the book keeping work associated with
 *	creating a reference to a TN that crosses block boundaries.
 *
 *   tn_has_live_def_into_BB
 *	This routine checks to see if a TN has a valid definition that
 *	is input to the current block.
 *
 * =======================================================================
 * =======================================================================
 */


inline
BOOL
EBO_tn_available(BB *bb,
                 EBO_TN_INFO *tninfo)
{
  TN *tn;
  BB *tn_bb;

 /* Safety check. */
  if ((tninfo == NULL) ||
      (tninfo->local_tn == NULL)) return FALSE;
  tn = tninfo->local_tn;
  tn_bb = tninfo->in_bb;
 /* Constants are always available. */
  if (TN_Is_Constant(tn)) return TRUE;
 /* Does a TN look-up get us back to where we start? */
  if (get_tn_info(tn) != tninfo) return FALSE;
 /* Global TN's aren't supported at low levels of optimization. */
  if ((Opt_Level < 2) && (bb != tn_bb)) return FALSE;
 /* Some forms of data movement aren't supported.
    - Mainly those involving dedicated TN's.
    So restrict use of registers across block boundaries
    unless registers have already been assigned.  */
  if (!EBO_in_peep &&
      ((bb != tn_bb) &&
       has_assigned_reg(tn))) return FALSE;
  return TRUE;
}


inline
BOOL
OP_effectively_copy(OP *op)
{
  if (OP_copy(op)) return TRUE;
  if (CGTARG_Is_Copy(op)) return TRUE;
  if (EBO_Copy_Operand(op) >= 0) return TRUE;
  return FALSE;
}



inline
INT
EBO_hash_op (OP *op,
             EBO_TN_INFO **based_on_tninfo)
{
  INT hash_value = 0;
  if (OP_memory(op)) {
    TN * spill_tn = NULL;
    hash_value = EBO_DEFAULT_MEM_HASH;
    if (OP_no_alias(op)) hash_value = EBO_NO_ALIAS_MEM_HASH;
    if (OP_load(op)) {
      spill_tn = OP_result(op,0);
    } else if (OP_store(op)) {
      spill_tn = OP_opnd(op,TOP_Find_Operand_Use(OP_code(op), OU_storeval));
    }
    if (spill_tn && TN_has_spill(spill_tn)) hash_value = EBO_SPILL_MEM_HASH;
  } else if (OP_effectively_copy(op)) {
    hash_value = EBO_COPY_OP_HASH;
  } else {
    INT opcount = OP_opnds(op);
    INT opndnum;
    hash_value =
#ifdef TARG_IA64
                 ((OP_results(op) == 2) &&
                  ((OP_result(op,0) != NULL) &&
                   ((OP_result(op,0) == True_TN) ||
                    (TN_register_class(OP_result(op,0)) == ISA_REGISTER_CLASS_predicate)))) ? 0 :
#endif
                 (INT)OP_code(op);
    for (opndnum = OP_has_predicate(op)?1:0; opndnum < opcount; opndnum++) {
      hash_value+=(INT)(INTPS)based_on_tninfo[opndnum];
    }
    hash_value = EBO_RESERVED_OP_HASH + EBO_EXP_OP_HASH(hash_value);
  }
  return hash_value;
}



inline
void
add_to_hash_table ( BOOL in_delay_slot,
                    OP *op,
                    EBO_TN_INFO **actual_tninfo,
                    EBO_TN_INFO **optimal_tninfo)
{
  INT hash_value = EBO_hash_op(op, optimal_tninfo);
  INT idx;
  EBO_OP_INFO *opinfo;
  BB *bb = OP_bb(op);
  TN *op_predicate_tn;
  EBO_TN_INFO *op_predicate_tninfo;

 /* Create a new opinfo entry and add the new op to the hash table. */
  opinfo = get_new_opinfo(op);
  opinfo->in_bb = OP_bb(op);
  opinfo->in_op = op;
  opinfo->in_delay_slot = in_delay_slot;
  opinfo->hash_index = hash_value;
  opinfo->same  = EBO_opinfo_table[hash_value];

 /* Define the result TN's and set the result fields. */
  if (OP_has_predicate(op)) {
    op_predicate_tn = OP_opnd(op,OP_PREDICATE_OPND);
    op_predicate_tninfo = actual_tninfo[OP_PREDICATE_OPND];
  } else {
    op_predicate_tn = NULL;
    op_predicate_tninfo = NULL;
  }

  for (idx = 0; idx < OP_results(op); idx++) {
    EBO_TN_INFO *tninfo = NULL;
    TN *tnr = OP_result(op, idx);
    if ((tnr != NULL) && (tnr != True_TN) && (tnr != Zero_TN)) {
      tninfo = tn_info_def (bb, op, tnr, op_predicate_tn, op_predicate_tninfo);
      tninfo->in_opinfo = opinfo;
    }
    opinfo->actual_rslt[idx] = tninfo;
  }

 /* Copy all the tninfo entries for the operands. */
  for (idx = 0; idx < OP_opnds(op); idx++) {
    opinfo->actual_opnd[idx] = actual_tninfo[idx];
    opinfo->optimal_opnd[idx] = optimal_tninfo[idx];
  }

  EBO_opinfo_table[hash_value] = opinfo;

  if (EBO_Trace_Data_Flow) {
    #pragma mips_frequency_hint NEVER
    fprintf(TFile,"%sCreate new opinfo entry at %d for:\n\t",EBO_trace_pfx,hash_value);
    Print_OP_No_SrcLine(op);
  }
}




inline
BOOL
tn_registers_identical (TN *tn1, TN *tn2)
{
  return ((tn1 == tn2) ||
          ((TN_is_register(tn1) && TN_is_register(tn2) &&
           (TN_is_dedicated(tn1) || (TN_register(tn1) != REGISTER_UNDEFINED)) &&
           (TN_is_dedicated(tn2) || (TN_register(tn2) != REGISTER_UNDEFINED)) &&
	   (TN_register_and_class(tn1) == TN_register_and_class(tn2)))));
}




inline
EBO_OP_INFO *
find_opinfo_entry (OP *op)
{
  EBO_OP_INFO *opinfo;
  INT limit_search = 50;  /* Give up after a while. */

  for (opinfo = EBO_last_opinfo;
       ((opinfo != NULL) && (limit_search != 0));
       opinfo = opinfo->prior, limit_search--) {
    if (opinfo->in_op == op) return opinfo;
  }

  return NULL;
}




inline
EBO_OP_INFO *
locate_opinfo_entry (EBO_TN_INFO *tninfo)
{

  if ((tninfo != NULL) && (tninfo->in_op != NULL)) {
    return (tninfo->in_opinfo != NULL) ? tninfo->in_opinfo : find_opinfo_entry(tninfo->in_op);
  }

  return NULL;
}




inline
void
EBO_OPS_predicate(TN *predicate_tn, OPS *ops)
{
  OP *next_op = OPS_first(ops);
  while (next_op != NULL) {
    if (OP_has_predicate(next_op)) {
      Set_OP_opnd(next_op, OP_PREDICATE_OPND, predicate_tn);
    }
    next_op = OP_next(next_op);
  }
}




inline
void
EBO_Exp_COPY(TN *predicate_tn, TN *tgt_tn, TN *src_tn, OPS *ops)
{
  Exp_COPY(tgt_tn, src_tn, ops);
  if (predicate_tn != NULL) {
    EBO_OPS_predicate (predicate_tn, ops);
  }
}




inline
void
mark_tn_live_into_BB (TN *tn, BB *into_bb, BB *outof_bb)
/* -----------------------------------------------------------------------
 * mark_tn_live_into_BB
 *
 * Mark a TN as "live into" the blocks that are between the two arguments.
 *
 * Assume that each block interving has exactly one predecessor.
 * 
 *-----------------------------------------------------------------------
 */
{
  if (into_bb != outof_bb) {
    BB *in_bb;
    BOOL already_global = TRUE;

    if (!TN_is_global_reg(tn)) {
      GTN_UNIVERSE_Add_TN(tn);
      already_global = FALSE;
    }

    if (EBO_in_peep) {
      if (!REG_LIVE_Into_BB( TN_register_class(tn), TN_register(tn), into_bb)) {
        ISA_REGISTER_CLASS tn_class = TN_register_class(tn);
        REGISTER tn_reg = TN_register(tn);
        FmtAssert((has_assigned_reg(tn)),
                  ("No assigned register in ebo utility mark_tn_live_into_BB"));

       /* Mark REGISTER as being used in intervening blocks. */
        in_bb = into_bb;
        while ((in_bb != NULL) && (in_bb != outof_bb)) {
          REG_LIVE_Update( tn_class, tn_reg, in_bb);
          in_bb = BB_First_Pred(in_bb);
        }

      }
    } else {
     /* Record TN information. */

     /* Always mark it used. */
      GRA_LIVE_Add_Live_Use_GTN(into_bb, tn);

     /* May also need to propagate liveness information. */
      if (!GRA_LIVE_TN_Live_Into_BB(tn, into_bb)) {
        GRA_LIVE_Add_Live_In_GTN(into_bb, tn);
        GRA_LIVE_Add_Defreach_In_GTN(into_bb, tn);

        if (!already_global) {
         /* If it was a local TN, it was defined in the "outof"
            block.  Otherwise, the Def information was already
            determined for this TN and we shouldn't change it. */
          GRA_LIVE_Add_Live_Def_GTN(outof_bb, tn);
        }
        GRA_LIVE_Add_Live_Out_GTN(outof_bb, tn);
        GRA_LIVE_Add_Defreach_Out_GTN(outof_bb, tn);

       /* Mark TN as being carried into and out of intervening blocks. */
        in_bb = BB_First_Pred(into_bb);
        while ((in_bb != NULL) && (in_bb != outof_bb)) {
          GRA_LIVE_Add_Live_In_GTN(in_bb, tn);
          GRA_LIVE_Add_Defreach_In_GTN(in_bb, tn);
          GRA_LIVE_Add_Live_Out_GTN(in_bb, tn);
          GRA_LIVE_Add_Defreach_Out_GTN(in_bb, tn);
          in_bb = BB_First_Pred(in_bb);
        }

      }
    }
  }
}


inline
BOOL
tn_has_live_def_into_BB (TN *tn, BB *into_bb)
/* -----------------------------------------------------------------------
 * tn_has_live_def_into_BB
 *
 * Return A TRUE if a TN has a reaching definition into the specified block.
 *
 *-----------------------------------------------------------------------
 */
{
  if (EBO_in_peep) {
    return TRUE;  /* There is no way to know. */
  } else {
    return (TN_is_dedicated(tn) ||
            GTN_SET_MemberP(BB_defreach_in(into_bb), tn));
  }
}
