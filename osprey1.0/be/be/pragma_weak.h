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
 *
 * Module: pragma_weak.c
 * $Revision: 1.1.1.1 $
 * $Date: 2002-05-22 20:06:20 $
 * $Author: dsystem $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/be/pragma_weak.h,v $
 *
 * Revision history:
 *  4-Jan-96 - Original Version
 *
 * Description:
 *  Header for code linked in with be.so that uses routines defined in
 *  another dso. This header should be the last in the include list
 *  so that the external declaration preceeds it.
 *
 * ====================================================================
 */

#pragma weak Assign_alias_id
#pragma weak Copy_alias_info
#pragma weak Get_symbol_info_for_cvt_io
#pragma weak Points_to
#pragma weak Points_to_copy
#pragma weak Valid_alias
#pragma weak Aliased
