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
 *  Module: gcm.h
 *  $Revision: 1.1.1.1 $
 *  $Date: 2002-05-22 20:06:25 $
 *  $Author: dsystem $
 *  $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/cg/gcm.h,v $
 *
 *  Description:
 *  ============
 *
 *  Interface to GCM.
 *
 * =======================================================================
 * =======================================================================
 */
#ifndef gcm_INCLUDED
#define gcm_INCLUDED

#include "hb_sched.h" 	/* for HBS_TYPE */

/* Define the different priority conditions for choosing the basic blocks 
   for GCM phase */
#define SORT_BY_BB_FREQ		0x01
#define SORT_BY_BB_PARALLELISM	0x02
#define SORT_BY_REG_USAGE	0x04

/* bunch of auxiliary flags */
#define GCM_ARRAY_SPEC_LIMIT  	PAD_SIZE_LIMIT
  
typedef UINT8 PRIORITY_TYPE;

/* Define the different conditions for GCM */
#define GCM_BEFORE_GRA          0x01
#define GCM_MINIMIZE_REGS       0x02
#define GCM_USE_LOCAL_SCHED     0x04
#define GCM_USE_SCHED_EST       0x08
#define GCM_AFTER_GRA           0x10

typedef UINT8	GCM_TYPE;

/* GCM flags: for different code motion possibilites */
#define GCM_NONE		0x0000 // no motion
#define GCM_EQUIV_FWD    	0x0001 // equivalence motion (dominators only)
#define GCM_EQUIV_BKWD      	0x0002 // equivalence motion (post-dom only)
#define GCM_SPEC_ABOVE    	0x0004 // spec. execution above split
#define GCM_SPEC_BELOW     	0x0008 // spec. executiob below merge
#define GCM_DUP_ABOVE     	0x0010 // code duplication below split
#define GCM_DUP_BELOW      	0x0020 // code duplication above merge
#define GCM_CIRC_ABOVE 		0x0040 // do circular code motion (above) the
                                       // loop branch back condition
#define GCM_EQUIV          	(GCM_EQUIV_FWD | GCM_EQUIV_BKWD)

// GCM internal flags: 
#define GCM_SCHED_BB            0x1000 // need to schedule/reschedule bb
#define GCM_BB_ALIGNED          0x2000 // state to maintain that the bb will
                                       // will be aligned in future
#define GCM_BB_MEM_BARRIER	0x4000 // if <bb> has memory barrier instrs.

// auxiliary flags to maintain the internal state of the <bb> during 
// different phases of the GCM.
#define BB_GCM_NONE(bbsch)		(BBSCH_flags(bbsch) & GCM_NONE)
#define Set_BB_GCM_NONE(bbsch)		(BBSCH_flags(bbsch) |= GCM_NONE)
#define Reset_BB_GCM_NONE(bbsch) 	(BBSCH_flags(bbsch) &= ~GCM_NONE)
#define BB_EQUIV_FWD(bbsch)		(BBSCH_flags(bbsch) & GCM_EQUIV_FWD)
#define Set_BB_EQUIV_FWD(bbsch)		(BBSCH_flags(bbsch) |= GCM_EQUIV_FWD)
#define Reset_BB_EQUIV_FWD(bbsch) 	(BBSCH_flags(bbsch) &= ~GCM_EQUIV_FWD)
#define BB_EQUIV_BKWD(bbsch)		(BBSCH_flags(bbsch) & GCM_EQUIV_BKWD)
#define Set_BB_EQUIV_BKWD(bbsch)	(BBSCH_flags(bbsch) |= GCM_EQUIV_BKWD)
#define Reset_BB_EQUIV_BKWD(bbsch)	(BBSCH_flags(bbsch) &= ~GCM_EQUIV_BKWD)
#define BB_SPEC_ABOVE(bbsch)		(BBSCH_flags(bbsch) & GCM_SPEC_ABOVE)
#define Set_BB_SPEC_ABOVE(bbsch)	(BBSCH_flags(bbsch) |= GCM_SPEC_ABOVE)
#define Reset_BB_SPEC_ABOVE(bbsch)	(BBSCH_flags(bbsch) &= ~GCM_SPEC_ABOVE)
#define BB_SPEC_BELOW(bbsch)		(BBSCH_flags(bbsch) & GCM_SPEC_BELOW)
#define Set_BB_SPEC_BELOW(bbsch)	(BBSCH_flags(bbsch) |= GCM_SPEC_BELOW)
#define Reset_BB_SPEC_BELOW(bbsch)	(BBSCH_flags(bbsch) &= ~GCM_SPEC_BELOW)
#define BB_DUP_ABOVE(bbsch)		(BBSCH_flags(bbsch) & GCM_DUP_ABOVE)
#define Set_BB_DUP_ABOVE(bbsch)		(BBSCH_flags(bbsch) |= GCM_DUP_ABOVE)
#define Reset_BB_DUP_ABOVE(bbsch)	(BBSCH_flags(bbsch) &= ~GCM_DUP_ABOVE)
#define BB_DUP_BELOW(bbsch)		(BBSCH_flags(bbsch) & GCM_DUP_BELOW)
#define Set_BB_DUP_BELOW(bbsch)		(BBSCH_flags(bbsch) |= GCM_DUP_BELOW)
#define Reset_BB_DUP_BELOW(bbsch)	(BBSCH_flags(bbsch) &= ~GCM_DUP_BELOW)
#define BB_CIRC_ABOVE(bbsch)		(BBSCH_flags(bbsch) & GCM_CIRC_ABOVE)
#define Set_BB_CIRC_ABOVE(bbsch)	(BBSCH_flags(bbsch) |= GCM_CIRC_ABOVE)
#define Reset_BB_CIRC_BELOW(bbsch)      (BBSCH_flags(bbsch) &= ~GCM_CIRC_ABOVE)

// Internal flags
#define BB_SCHEDULE(bbsch)              (BBSCH_flags(bbsch) & GCM_SCHED_BB)
#define Set_BB_SCHEDULE(bbsch)          (BBSCH_flags(bbsch) |= GCM_SCHED_BB)
#define Reset_BB_SCHEDULE(bbsch)        (BBSCH_flags(bbsch) &= ~GCM_SCHED_BB)
#define BB_ALIGNED(bbsch)               (BBSCH_flags(bbsch) & GCM_BB_ALIGNED)
#define Set_BB_ALIGNED(bbsch)           (BBSCH_flags(bbsch) |= GCM_BB_ALIGNED)
#define Reset_BB_ALIGNED(bbsch)         (BBSCH_flags(bbsch) &= ~GCM_BB_ALIGNED)
#define BB_MEM_BARRIER(bbsch)    	(BBSCH_flags(bbsch) & GCM_BB_MEM_BARRIER)
#define Set_BB_MEM_BARRIER(bbsch)	(BBSCH_flags(bbsch) |= GCM_BB_MEM_BARRIER)
#define Reset_BB_MEM_BARRIER(bbsch)	(BBSCH_flags(bbsch) &= ~GCM_BB_MEM_BARRIER)

// Other flags:
#define OP_visited(o)			OP_flag1(o)
#define Set_OP_visited(o)    		Set_OP_flag1(o)
#define Reset_OP_visited(o)		Reset_OP_flag1(o)
#define OP_moved(o)			OP_flag1(o)
#define Set_OP_moved(o)			Set_OP_flag1(o)
#define Reset_OP_moved(o)		Reset_OP_flag1(o)

/* GCM flags (cgdriver options) */
extern BOOL GCM_POST_Spec_Loads;
extern BOOL GCM_PRE_Spec_Loads;
extern BOOL GCM_Use_Sched_Est;
extern GCM_TYPE Cur_Gcm_Type;
extern BOOL GCM_Forw_Circ_Motion;
extern BOOL GCM_POST_Force_Scheduling;

extern INT32 GCM_From_BB;
extern INT32 GCM_To_BB;
extern INT32 GCM_Result_TN;
extern BOOL CG_Skip_GCM;

extern void GCM_Schedule_Region (HBS_TYPE locs_type);

extern void GCM_Fill_Branch_Delay_Slots (void);

#endif /* gcm_INCLUDED */
