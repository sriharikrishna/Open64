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


/* $Header: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/libU77/time_.c,v 1.1.1.1 2002-05-22 20:09:16 dsystem Exp $ */
/*
 *
 * return the current time as an integer
 *
 * calling sequence:
 *	integer time
 *	i = time()
 * where:
 *	i will receive the current GMT in seconds.
 *
 * return the current time as a character string
 *
 * calling sequence:
 *	charater timebuf*8
 *	call time(timebuf)
 * where:
 *	timebuf will receive the current time in the format  hh:mm:ss
 *
 */

#include <sys/types.h>
#if defined(_SYSV) || defined(_SYSTYPE_SVR4)
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <string.h>

extern time_t 
#if defined(__ia64) || defined(__ia32)
__attribute__ ((weak)) time_(void)
#else
time_(void)
#endif
{
	return(time(NULL));
}

extern void
#if defined(__ia64) || defined(__ia32)
_TIME (char timebuf[])
#else
time_vms (char timebuf[])
#endif
{
	time_t t;

	t = time(0);
	strncpy(timebuf,ctime(&t)+11,8);
}
