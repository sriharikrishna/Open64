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
 * Module: cwh_data.h
 * $Revision: 1.3 $
 * $Date: 2002-07-19 14:25:46 $
 * $Author: fzhao $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/crayf90/sgi/cwh_data.h,v $
 *
 * Revision history:
 *  dd-mmm-95 - Original Version
 *
 * Description: Exports interfaces from cwh_data.c
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef CWH_DATA_INCLUDED
#define CWH_DATA_INCLUDED


#ifdef _KEEP_RCS_ID
static char *rcs_id = "$Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/crayf90/sgi/cwh_data.h,v $ $Revision: 1.3 $";
#endif /* _KEEP_RCS_ID */

extern void cwh_data_emit_symbol_inits(SYMTAB_IDX level);

#endif /* CWH_DATA_INCLUDED */
/* #define DATA_DEBUG 1 */

