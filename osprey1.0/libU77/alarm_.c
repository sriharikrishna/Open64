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
/* $Header: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/libU77/alarm_.c,v 1.1.1.1 2002-05-22 20:09:16 dsystem Exp $ */
/* $Header: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/libU77/alarm_.c,v 1.1.1.1 2002-05-22 20:09:16 dsystem Exp $ */
/*
 *
 * set an alarm time, arrange for user specified action, and return.
 *
 * calling sequence:
 *	integer	flag
 *	f77external alfunc
 *	lastiv = alarm (intval, alfunc)
 * where:
 *	intval	= the alarm interval in seconds; 0 turns off the alarm.
 *	alfunc	= the function to be called after the alarm interval,
 *
 *	The returned value will be the time remaining on the last alarm.
 */

#include <unistd.h>
#include <signal.h>

extern int 
alarm_(int *sec, void (* proc)())
{
	register unsigned	lt;

	lt = alarm(1000);	/* time to maneuver */

	if (*sec)
		signal(SIGALRM, proc);

	alarm(*sec);
	return(lt);
}
