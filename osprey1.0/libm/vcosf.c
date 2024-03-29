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


/* ====================================================================
 * ====================================================================
 *
 * Module: vcosf.c
 * $Revision: 1.1.1.1 $
 * $Date: 2002-05-22 20:09:59 $
 * $Author: dsystem $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/libm/vcosf.c,v $
 *
 * Revision history:
 *  01-Dec-94 - Original Version
 *
 * Description:	source code for vector cosine function
 *
 * ====================================================================
 * ====================================================================
 */

#include "libm.h"

#ifdef mips
extern	void	vfcos(float *, float *, long, long, long);
extern	void	vcosf(float *, float *, long, long, long);

#pragma weak vfcos = __vcosf
#pragma weak vcosf = __vcosf
#endif

#ifdef __GNUC__
extern  void  __vcosf(float *, float *, long, long, long);
void    vcosf() __attribute__ ((weak, alias ("__vcosf")));
#endif

static const du	rpiby2 =
{D(0x3fe45f30, 0x6dc9c883)};

static const du	piby2hi =
{D(0x3ff921fb, 0x50000000)};

static const du	piby2lo =
{D(0x3e5110b4, 0x611a6263)};

static const fu	Twop28 = {0x4d800000};

static const	fu	Qnan = {QNANF};

/* coefficients for polynomial approximation of sin on +/- pi/4 */

static const du	P[] =
{
{D(0x3ff00000, 0x00000000)},
{D(0xbfc55554, 0x5268a030)},
{D(0x3f811073, 0xafd14db9)},
{D(0xbf29943e, 0x0fc79aa9)},
};

/* coefficients for polynomial approximation of cos on +/- pi/4 */

static const du	Q[] =
{
{D(0x3ff00000, 0x00000000)},
{D(0xbfdffffb, 0x2a77e083)},
{D(0x3fa553e7, 0xf02ac8aa)},
{D(0xbf5644d6, 0x2993c4ad)},
};



/* ====================================================================
 *
 * FunctionName		vcosf
 *
 * Description		computes vector cosine of arg
 *
 * ====================================================================
 */

void
__vcosf( float	*x, float *y, long count, long stridex, long stridey )
{
long	i;
int	n;
float	arg;
double	dx;
double	xsq;
double	sinpoly, cospoly;
float	result;
double	dn;

	/* i = 0, 1, ..., count-1; y[i*stridey] = cosf(x[i*stridex]) */

	for ( i=0; i<count; i++ )
	{
#ifdef _PREFETCH
#pragma prefetch_ref=*(x+8)
#pragma prefetch_ref=*(y+8)
#endif

		arg = *x;

		dx = arg;

		/* for large args, return 0.0	*/

		if ( fabsf(arg) >= Twop28.f )
			dx = 0.0;

		if ( arg != arg )
			dx = 0.0;

		/*  reduce argument to +/- pi/4  */
	
		dn = dx*rpiby2.d;
	
		n = ROUND(dn);
		dn = n;
	
		dx = dx - dn*piby2hi.d;
		dx = dx - dn*piby2lo.d;
	
		xsq = dx*dx;

		cospoly = ((Q[3].d*xsq + Q[2].d)*xsq + Q[1].d)*xsq + Q[0].d;
	
		sinpoly = ((P[3].d*xsq + P[2].d)*xsq + P[1].d)*(xsq*dx) + dx;

		result = cospoly;

		if ( n&1 )
		{
			result = -sinpoly;
			n--;
		}

		if ( n&2 )
		{
			result = -result;
		}
	
		if ( arg != arg )
		{
			result = Qnan.f;
		}
	
		*y = result;
	
		x += stridex;
		y += stridey;
	}
}

