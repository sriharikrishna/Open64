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
/* $Header: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/libU77/symlnk_.c,v 1.1.1.1 2002-05-22 20:09:16 dsystem Exp $ */
/* $Header: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/libU77/symlnk_.c,v 1.1.1.1 2002-05-22 20:09:16 dsystem Exp $ */
/*
char id_symlnk[] = "@(#)symlnk_.c	1.1";
 *
 * make a symbolic link to a file
 *
 * calling sequence:
 *	ierror = symlnk(name1, name2)
 * where:
 *	name1 is the pathname of an existing file
 *	name2 is a pathname that will become a symbolic link to name1
 *	ierror will be 0 if successful; a system error code otherwise.
 */

#include <malloc.h>
#include <unistd.h>
#include <sys/param.h>
#include "cmplrs/f_errno.h"

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	128
#endif
#include "externals.h"

extern int 
symlnk_ (char *name1, char *name2, int n1len, int n2len)
{
	char *buf1, *buf2;

	if (!bufarg && !(bufarg=malloc(bufarglen=n1len+n2len+2)))
#ifdef __sgi
	{
		errno=F_ERSPACE;
		return(-1);
	}
#else
		return((errno=F_ERSPACE));
#endif
	else if (bufarglen <= n1len+n2len+1 && !(bufarg=realloc(bufarg, bufarglen=n1len+n2len+2)))
#ifdef __sgi
	{
		errno=F_ERSPACE;
		return(-1);
	}
#else
		return((errno=F_ERSPACE));
#endif
	buf1 = bufarg;
	buf2 = &bufarg[n1len+1];
	g_char(name1, n1len, buf1);
	g_char(name2, n2len, buf2);
	if (buf1[0] == '\0' || buf2[0] == '\0')
#ifdef __sgi
	{
		errno=F_ERARG;
		return(-1);
	}
#else
		return((errno=F_ERARG));
#endif
	if (symlink(buf1, buf2) != 0)
		return(-1);
	return(0);
}
