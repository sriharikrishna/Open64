/*

  Copyright (C) 1999-2001, Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2.1 of the GNU Lesser General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  Further, any
  license provided herein, whether implied or otherwise, is limited to 
  this program in accordance with the express provisions of the 
  GNU Lesser General Public License.  

  Patent licenses, if any, provided herein do not apply to combinations 
  of this program with other product or programs, or any other product 
  whatsoever.  This program is distributed without any warranty that the 
  program is delivered free of the rightful claim of any third person by 
  way of infringement or the like.  

  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston MA 02111-1307, USA.

*/

/* $Header: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/libU77/unlink_.c,v 1.1.1.1 2002-05-22 20:09:16 dsystem Exp $ */
/*
 *
 * unlink (remove) a file
 *
 * calling sequence:
 *	integer unlink
 *	ierror = unlink(filename)
 * where:
 *	ierror will be a returned status (0 == OK)
 *	filename is the file to be unlinked
 */

#include "cmplrs/f_errno.h"
#include "cmplrs/host.h"
#include <sys/param.h>
#include <unistd.h>
#include <stdlib.h>
#include "externals.h"

int32
unlink_(char *fname, int32 namlen)
{

	if (!bufarg && !(bufarg=malloc(bufarglen=namlen+1)))
		return((errno=F_ERSPACE));
	else if (bufarglen <= namlen && !(bufarg=realloc(bufarg, bufarglen=namlen+1)))
		return((int32)(errno=F_ERSPACE));
	g_char(fname, namlen, bufarg);
	return( (int32) unlink(bufarg) );
}