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

/* $Header: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/libU77/ctime_.c,v 1.1.1.1 2002-05-22 20:09:16 dsystem Exp $ */
/*
 *
 * convert system time to ascii string
 *
 * calling sequence:
 *	character*24 string, ctime
 *	integer clock
 *	string = ctime (clock)
 * where:
 *	string will receive the ascii equivalent of the integer clock time.
*/

#include <sys/types.h>
#include <time.h>
#include "externals.h"

extern void
ctime_ (char *str, int len, int *clock)
{
	char *s = ctime((time_t *)clock);
	s[24] = '\0';
	b_char(s, str, len);
}
