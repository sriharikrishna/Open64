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
 * Module: cwh_preg.h
 * $Revision: 1.2 $
 * $Date: 2002-07-12 16:45:09 $
 * $Author: fzhao $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/crayf90/sgi/cwh_preg.h,v $
 *
 * Revision history:
 *  dd-mmm-95 - Original Version
 *
 * Description: Exports preg mangement functions.
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef CIR_WH_PREG_INCLUDED
#define CIR_WH_PREG_INCLUDED

#ifdef _KEEP_RCS_ID
static char *rcs_id = "$Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/crayf90/sgi/cwh_preg.h,v $ $Revision: 1.2 $";
#endif /* _KEEP_RCS_ID */


/* PREG details, returned when allocated */

typedef struct {
  TY_IDX  preg_ty;
  ST * preg_st;
  PREG_NUM preg;
} PREG_det ;

extern void fe_preg_init (void) ;
extern PREG_det cwh_preg_next_preg (TYPE_ID    btype,
				    char     * name,
				    WN       * home_wn ) ;

extern WN * cwh_preg_temp_save(char * name, WN  * expr ) ;

#endif /* CIR_WH_PREG_INCLUDED */

