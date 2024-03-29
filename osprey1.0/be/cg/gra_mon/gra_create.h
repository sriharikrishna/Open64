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

/*                    Create Basic GRA Data Structures
 *                    ================================
 *
 *  Exported functions:
 *
 *      void GRA_Create()
 *
 *          Traverse the flow grap building GRA's basic data structures:
 *
 *              1. The GRA_REGIONs that represent the separately preallocated
 *                 parts of the PU and the compliment.
 *
 *              2. The GRA_BBs that encapsulate the BBs.  GRA_bb_map is
 *                 set up to map from BBs to GRA_BBs.
 *
 *              3. The LRANGEs that represent register candidates.
 *                 GRA_tn_lrange_map is set up to map from TNs to LRANGEs.
 *
 *              4. The LUNITs that represent a BB within a particular
 *                 LRANGE
 *
 *      void GRA_Delete()
 *                 
 *          Destroy all the above  when we are done with it.
 */


/*
 *  $Revision: 1.1.1.1 $
 *  $Date: 2002-05-22 20:06:27 $
 *  $Author: dsystem $
 *  $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/cg/gra_mon/gra_create.h,v $
 */


#ifndef CREATE_INCLUDED
#define CREATE_INCLUDED

#ifndef CREATE_RCS_ID
#define CREATE_RCS_ID
#ifdef _KEEP_RCS_ID
static char *create_rcs_id = "$Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/cg/gra_mon/gra_create.h,v $ $Revision: 1.1.1.1 $";
#endif
#endif

extern void GRA_Create(void);
extern void GRA_Delete(void);

#endif
