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


/* ====================================================================
 * ====================================================================
 *
 *  Module: cg_loop_mii.h
 *  $Revision: 1.1.1.1 $
 *  $Date: 2002-05-22 20:06:23 $
 *  $Author: dsystem $
 *  $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/cg/cg_loop_mii.h,v $
 *
 *  Revision history:
 *   16-Oct-91 - Original Version
 *
 *  Synopsis:
 *
 *      Calculate CG_LOOP_min_ii for the given loop, maximal costs of
 *      within SCC paths as a side effect.  The costs are useful
 *      during scheduling for pruning the backtracking search.
 *
 *  Interface Description:
 *
 *      Exported variables:
 *
 *          INT32 CG_LOOP_min_ii
 *
 *              Set by this module to a lower bound on the II to
 *              search for a schedule.
 *
 *          INT CG_LOOP_res_min_ii
 *
 *              Set by this module to a lower bound on the II as
 *              dictated by resource constraints (i.e., not counting
 *		recurrences).
 *
 *          INT CG_LOOP_rec_min_ii
 *
 *              Set by this module to a lower bound on the II as
 *              dictated by recurring constraints (i.e., not counting
 *		resources).
 *
 *      Exported functions:
 *
 *	    void CG_LOOP_Calculate_Min_Resource_II(
 *              BB *loop_body,
 *              OP_LIST *loop_overhead,
 *		BOOL ignore_prefetches,
 *		BOOL ignore_prefetch_strides
 *          )
 *		Calculate CG_LOOP_min_ii based solely on resources (i.e.,
 *		ignoring recurrences).  This is used to recalculate
 *		the min ii as recurrences are fixed.  If <ignore_prefetches>
 *		is TRUE, prefetches will be ignored when computing the
 *		resource usage.  If <ignore_prefetch_strides> is FALSE,
 *		each prefetch will be counted as 1/stride prefetches.
 *
 *		Results in: CG_LOOP_min_ii, CG_LOOP_res_min_ii.
 *
 *	    void CG_LOOP_Calculate_Min_Recurrence_II(
 *              BB *loop_body,
 *		BOOL ignore_non_def_mem_deps
 *          )
 *		Calculate CG_LOOP_min_ii, including restrictions caused
 *		by recurrences.  This differs from CG_LOOP_Calculate_Max_
 *		Costs_And_Min_II in that it assumes the resource min
 *		II has already been calculated, and in that it doesn't
 *		try to fix recurrences.  It's used to recalculate the
 *		min II after fixing recurrences introduces new SCCs.
 *		If <ignore_non_def_mem_deps> is TRUE, non-definite memory
 *		dependences will not be considered in self-recurrences
 *		(others should have been filtered out by
 *		CG_LOOP_Make_Strongly_Connected_Components).
 *
 *              Requires: CG_LOOP_Make_Strongly_Connected_Components.
 *
 *		Results in: CG_LOOP_min_ii.
 *
 *          void CG_LOOP_Calculate_Max_Costs_And_Min_II(
 *              BB *loop_body,
 *              OP_LIST *loop_overhead,
 *		BOOL ignore_prefetches,
 *		BOOL ignore_prefetch_strides,
 *		BOOL ignore_non_def_mem_deps
 *          )
 *
 *              Calculate CG_LOOP_min_ii and the maximal costs of intra-SCC
 *              paths.  This includes the calculations of the values
 *              traditionally called "MII" and "MRII", i.e., both
 *              resources and recurrences are taken into account.  If
 *		<ignore_prefetches> is TRUE, prefetch OPs will be ignored
 *		when computing resource usage.  If <ignore_prefetch_strides>
 *		is FALSE, each prefetch will be counted as 1/stride prefetches.
 *		If <ignore_non_def_mem_deps> is TRUE, non-definite memory
 *		dependences will not be considered in self-recurrences
 *		(others should have been filtered out by
 *		CG_LOOP_Make_Strongly_Connected_Components).
 *
 *              Requires: CG_LOOP_Make_Strongly_Connected_Components.
 *
 *              Results in: CG_LOOP_min_ii, and OP_scc_{descendents,ancestors}
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef CG_LOOP_MII_INCLUDED
#define CG_LOOP_MII_INCLUDED

#ifdef _KEEP_RCS_ID
static const char cg_loop_mii_rcs_id[] = "$Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/cg/cg_loop_mii.h,v $ $Revision: 1.1.1.1 $";
#endif /* _KEEP_RCS_ID */

extern INT32       CG_LOOP_min_ii;
extern INT32       CG_LOOP_rec_min_ii;
extern INT32       CG_LOOP_res_min_ii;

extern void
CG_LOOP_Calculate_Min_Resource_II(
  BB *loop_body,
  OP_LIST *loop_overhead,
  BOOL ignore_prefetches,
  BOOL ignore_prefetch_strides
);

extern void
CG_LOOP_Calculate_Min_Recurrence_II(
  BB *loop_body,
  BOOL ignore_non_def_mem_deps
);

extern void
CG_LOOP_Calculate_Max_Costs_And_Min_II(
  BB *loop_body,
  OP_LIST *loop_overhead,
  BOOL ignore_prefetches,
  BOOL ignore_prefetch_strides,
  BOOL ignore_non_def_mem_deps
);

#endif /* CG_LOOP_MII_INCLUDED */
