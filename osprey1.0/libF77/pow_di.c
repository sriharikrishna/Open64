/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2.1 of the GNU Lesser General Public License 
  as published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU Lesser General Public 
  License along with this program; if not, write the Free Software 
  Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307, 
  USA.

  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/


/* $Header: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/libF77/pow_di.c,v 1.1.1.1 2002-05-22 20:09:13 dsystem Exp $ */
#include "cmplrs/host.h"
#include <math.h>
#include "moremath.h"

double_t __powdi(double_t x, int32 n)
{
double_t pow;

pow = 1;

if(n != 0)
	{
	if(n < 0)
		{
		if(x == 0)
			{
			return(pow);
			}
		n = -n;
		x = 1/x;
		}
	for( ; ; )
		{
		if(n & 01)
			pow *= x;
		if(n >>= 1)
			x *= x;
		else
			break;
		}
	}
return(pow);
}

double_t __powdl(double_t x, int64 n)
{
double_t pow;

pow = 1;

if(n != 0)
	{
	if(n < 0)
		{
		if(x == 0)
			{
			return(pow);
			}
		n = -n;
		x = 1/x;
		}
	for( ; ; )
		{
		if(n & 01)
			pow *= x;
		if(n >>= 1)
			x *= x;
		else
			break;
		}
	}
return(pow);
}

/* By-reference versions for backward compatibility. */

double_t pow_di(double_t *ap, int32 *bp)
{
double_t pow;
pow=__powdi(*ap,*bp);
return pow;
}

double_t pow_dl(double_t *ap, int64 *bp)
{
double_t pow;
pow=__powdl(*ap,*bp);
return pow;
}
