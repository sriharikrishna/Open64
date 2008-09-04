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


#ifndef wutil_INCLUDED
#define wutil_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include "wintrinsic.h"
#include "wio.h"

/* ====================================================================
 * ====================================================================
 *
 *
 * Description:
 *
 * This module contains the external interfaces (data structures and
 * prototypes) for routines which provide utility services related to
 * Whirl intrinsics.
 *
 * ====================================================================
 * ====================================================================
 */

const char *get_intrinsic_name(INTRINSIC opcode);
const char *get_iostatement_name(IOSTATEMENT opcode);
const char *get_ioitem_name(IOITEM opcode);
const char *get_iolibrary_name(IOLIB opcode);

INTRINSIC   get_intrinsic_from_name(const char* name);
IOSTATEMENT get_iostatement_from_name(const char* name);
IOITEM      get_ioitem_from_name(const char* name);


#define INTRINSIC_name(op) get_intrinsic_name(op)
#define IOSTATEMENT_name(op) get_iostatement_name(op)
#define IOITEM_name(op) get_ioitem_name(op)

#define Name_To_INTRINSIC(nm) get_intrinsic_from_name(nm)
#define Name_To_IOSTATEMENT(nm) get_iostatement_from_name(nm)
#define Name_To_IOITEM(nm) get_ioitem_from_name(nm)


#ifdef __cplusplus
}
#endif
#endif /* wutil_INCLUDED */
