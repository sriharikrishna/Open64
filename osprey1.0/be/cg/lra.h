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
 *  Module: lra.h
 *  $Revision: 1.1.1.1 $
 *  $Date: 2002-05-22 20:06:26 $
 *  $Author: dsystem $
 *  $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/cg/lra.h,v $
 *
 *  Description:
 *  ============
 *
 *  Local Register Allocation.
 *
 * =======================================================================
 * =======================================================================
 */

#ifndef lra_INCLUDED
#define lra_INCLUDED

#include "bb.h"
#include "op.h"
#include "register.h"

/* Initialize LRA data structures for PU/region. */
extern void LRA_Init (void);

/* Do Local register allocation for a region. The <lra_for_pu> parameter
 * indicates if the region is the complete procedure.
 */
extern void LRA_Allocate_Registers (BOOL lra_for_pu);


/* Compute the number of local registers that will be required for each 
 * register class for this <bb>. This information is stored internally
 * in LRA and can be accessed later by a call to LRA_Register_Request.
 */
extern mINT8 *LRA_Compute_Register_Request (BB *bb, MEM_POOL *pool);

/* Compute the fatpoint for a block and optionally, return an array
 * of the combined register requirements at each op in the block.
 */
extern void LRA_Estimate_Fat_Points (BB* bb, mINT8* fatpoint,
				     INT* regs_in_use, MEM_POOL* pool);

/* Returns the number of registers LRA is requesting from GRA for
 * the class <cl> in the basic block <bb>. If we run the scheduling
 * pass before register allocation for the bb, this returns an 
 * accurate estimate of how many registers LRA needs. Otherwise,
 * it is just a fixed number based on some heuristics.
 */
extern INT LRA_Register_Request (BB *bb,  ISA_REGISTER_CLASS cl);

/* Allocate registers for any unallocated TNs in <ops>.
 * <bb> is used to determine what temps are available.
 */
extern void Assign_Temp_Regs (OPS *ops, BB *bb);

#endif /* lra_INCLUDED */
