
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
 * Module: rcis.c
 * $Revision: 1.1.1.1 $
 * $Date: 2002-05-22 20:10:00 $
 * $Author: dsystem $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/libm/mips/rcis.c,v $
 *
 * Revision history:
 *  05-Sep-96 - Original Version
 *
 * Description:	source code for rcis function
 *
 * ====================================================================
 * ====================================================================
 */

#ifdef _CALL_MATHERR
#include <stdio.h>
#include <math.h>
#include <errno.h>
#endif

#include "libm.h"
#include "complex.h"

#ifdef mips
extern	complex	__rcis(float);

#pragma weak __rcis = __libm_rcis
#endif

#ifdef __GNUC__
extern  complex  __libm_rcis(float);
complex    __rcis() __attribute__ ((weak, alias ("__libm_rcis")));
#endif

/* coefficients for polynomial approximation of sin on +/- pi/4 */

static const du  S[] =
{
{D(0x3ff00000, 0x00000000)},
{D(0xbfc55554, 0x5268a030)},
{D(0x3f811073, 0xafd14db9)},
{D(0xbf29943e, 0x0fc79aa9)},
};

/* coefficients for polynomial approximation of cos on +/- pi/4 */

static const du  C[] =
{
{D(0x3ff00000, 0x00000000)},
{D(0xbfdffffb, 0x2a77e083)},
{D(0x3fa553e7, 0xf02ac8aa)},
{D(0xbf5644d6, 0x2993c4ad)},
};

static const du  rpiby2 =
{D(0x3fe45f30, 0x6dc9c883)};

static const du  piby2hi =
{D(0x3ff921fb, 0x50000000)};

static const du  piby2lo =
{D(0x3e5110b4, 0x611a6263)};

static const	fu	Qnan = {QNANF};

static const fu Inf = {0x7f800000};


/* ====================================================================
 *
 * FunctionName    __libm_rcis
 *
 * Description    computes cos(arg) + i*sin(arg)
 *
 * Note:  Routine __libm_rcis() computes cosf(arg) + i*sinf(arg) and should
 *	  be kept in sync with sinf() and cosf() so that it returns 
 *	  the same result as one gets by calling cosf() and sinf() 
 *	  separately.
 *
 * ====================================================================
 */

complex
__libm_rcis(float x)
{
int  n;
int  ix, xpt;
double  dx, xsq;
double  dn;
double  sinpoly, cospoly;
complex  result;
#ifdef _CALL_MATHERR
struct exception	exstruct;
#endif

	FLT2INT(x, ix);	/* copy arg to an integer */

	xpt = (ix >> (MANTWIDTH-1));
	xpt &= 0x1ff;

	/* xpt is exponent(x) + 1 bit of mantissa */
	
	if ( xpt < 0xfd )
	{
		/* |x| < .75 */

		if ( xpt >= 0xe6 )
		{
			/* |x| >= 2^(-12) */

			dx = x;

			xsq = dx*dx;

			result.real = ((C[3].d*xsq + C[2].d)*xsq + C[1].d)*xsq + C[0].d;
			result.imag = ((S[3].d*xsq + S[2].d)*xsq + S[1].d)*(xsq*dx) + dx;

			return ( result );
		}
		else
		{
			result.real = 1.0f;
			result.imag = x;

			return ( result );
		}

	}

	if (xpt < 0x136)
	{
		/*  |x| < 2^28  */

		dx = x;
		dn = dx*rpiby2.d;

		n = ROUND(dn);
		dn = n;

		dx = dx - dn*piby2hi.d;
		dx = dx - dn*piby2lo.d;  /* dx = x - n*piby2 */

		xsq = dx*dx;

		sinpoly = ((S[3].d*xsq + S[2].d)*xsq + S[1].d)*(xsq*dx) + dx;
		cospoly = ((C[3].d*xsq + C[2].d)*xsq + C[1].d)*xsq + C[0].d;

		if ( n&1 )
		{
			if ( n&2 )
			{
				/*
				 *  n%4 = 3
				 *  result is sin(x) - i*cos(x)
				 */

				result.real = sinpoly;
				result.imag = -cospoly;
			}
			else
			{
				/*
				 *  n%4 = 1
				 *  result is -sin(x) + i*cos(x)
				 */

				result.real = -sinpoly;
				result.imag = cospoly;
			}

			return (result);
		}

		if ( n&2 )
		{
			/*
			 *  n%4 = 2
			 *  result is -cos(x) - i*sin(x)
			 */

			result.real = -cospoly;
			result.imag = -sinpoly;
		}
		else
		{
			/*
			 *  n%4 = 0
			 *  result is cos(x) + i*sin(x)
			 */

			result.real = cospoly;
			result.imag = sinpoly;
		}

		return ( result );
	}

	if ( (x != x) || (fabsf(x) == Inf.f) )
	{
		/* x is a NaN or +/-inf; return a pair of quiet NaNs */

#ifdef _CALL_MATHERR

                exstruct.type = DOMAIN;
                exstruct.name = "cosf";
                exstruct.arg1 = x;
                exstruct.retval = Qnan.f;

                if ( matherr( &exstruct ) == 0 )
                {
                        fprintf(stderr, "domain error in cosf\n");
                        SETERRNO(EDOM);
                }

		result.real = exstruct.retval;

                exstruct.type = DOMAIN;
                exstruct.name = "sinf";
                exstruct.arg1 = x;
                exstruct.retval = Qnan.f;

                if ( matherr( &exstruct ) == 0 )
                {
                        fprintf(stderr, "domain error in sinf\n");
                        SETERRNO(EDOM);
                }

		result.imag = exstruct.retval;

                return ( result );
#else
		NAN_SETERRNO(EDOM);
		
		result.real = result.imag = Qnan.f;

		return ( result );
#endif
	}

	/* just give up and return 0.0, setting errno = ERANGE */

#ifdef _CALL_MATHERR

		exstruct.type = TLOSS;
		exstruct.name = "cosf";
		exstruct.arg1 = x;
		exstruct.retval = 0.0f;

		if ( matherr( &exstruct ) == 0 )
		{
			fprintf(stderr, "range error in cosf (total loss \
of significance)\n");
			SETERRNO(ERANGE);
		}

		result.real = exstruct.retval;

		exstruct.type = TLOSS;
		exstruct.name = "sinf";
		exstruct.arg1 = x;
		exstruct.retval = 0.0f;

		if ( matherr( &exstruct ) == 0 )
		{
			fprintf(stderr, "range error in sinf (total loss \
of significance)\n");
			SETERRNO(ERANGE);
		}

		result.imag = exstruct.retval;

		return ( result );

#else
		SETERRNO(ERANGE);

		result.real = result.imag = 0.0f;

		return ( result );
#endif

}

