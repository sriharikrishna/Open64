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
 *
 * Revision history:
 *  dd-mmm-95 - Original Version
 *
 * Description:
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef CWH_INTRIN_INCLUDED
#define CWH_INTRIN_INCLUDED

#ifdef _KEEP_RCS_ID
#endif /* _KEEP_RCS_ID */


extern WN * cwh_intrin_call(INTRINSIC intr, INT16 numargs, WN ** k, WN **sz, BOOL *v ,TYPE_ID bt ) ;
extern WN * cwh_intrin_op(INTRINSIC intr, INT16 numargs, WN ** k, WN **sz, BOOL *v ,TYPE_ID bt ) ;

extern WN * cwh_intrin_wrap_ref_parm(WN *wa, TY_IDX  ty) ;
extern WN * cwh_intrin_wrap_char_parm(WN *wa, WN *sz ) ;
extern WN * cwh_intrin_wrap_value_parm(WN *wn) ;
extern ST * cwh_intrin_make_intrinsic_symbol(char *name, TYPE_ID ty);

#endif /* CWH_INTRIN_INCLUDED */

