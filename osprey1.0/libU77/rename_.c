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

/* --------------------------------------------------- */
/* | All Rights Reserved.                            | */
/* --------------------------------------------------- */
/* $Header: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/libU77/rename_.c,v 1.1.1.1 2002-05-22 20:09:16 dsystem Exp $ */
/* $Header: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/libU77/rename_.c,v 1.1.1.1 2002-05-22 20:09:16 dsystem Exp $ */
/*
char	id_rename[] = "@(#)rename_.c	1.2";
 *
 * rename a file atomically
 *
 * synopsis:
 *	integer function rename (from, to)
 *	character*(*) from, to
 *
 * where:
 *	return value will be zero normally, an error number otherwise.
 */

#include <sys/types.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/param.h>
#ifndef	MAXPATHLEN
#define MAXPATHLEN	128
#endif
#include "cmplrs/f_errno.h"
#include "externals.h"

extern int
rename_ (char *from, char *to, int frlen, int tolen)
{
	char	*frbuf, *tobuf;

	if (frlen <= 0 || tolen <= 0 || *from == ' ' || *to == ' ')
		return ((errno = F_ERARG));
	if (!bufarg && !(bufarg=malloc(bufarglen=frlen+tolen+2)))
		return((errno=F_ERSPACE));
	else if (bufarglen <= frlen+tolen+1 && !(bufarg=realloc(bufarg, 
			bufarglen=frlen+tolen+2)))
		return((errno=F_ERSPACE));
	frbuf = bufarg;
	tobuf = &bufarg[frlen+1];
	g_char (from, frlen, frbuf);
	g_char (to, tolen, tobuf);
	if (rename (frbuf, tobuf) != 0)
		return (errno);
	return (0);
}
