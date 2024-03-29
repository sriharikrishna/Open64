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
 * Module: vlog10.c
 * $Revision: 1.1.1.1 $
 * $Date: 2002-05-22 20:09:59 $
 * $Author: dsystem $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/libm/vlog10.c,v $
 *
 * Revision history:
 *  05-Feb-96 - Original Version
 *
 * Description:	source code for vector common logarithm function
 *
 * ====================================================================
 * ====================================================================
 */

static char *rcs_id = "$Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/libm/vlog10.c,v $ $Revision: 1.1.1.1 $";

#include "libm.h"

#ifdef mips
extern	void	vlog10(double *, double *, long, long, long);

#pragma weak vlog10 = __vlog10
#endif

#ifdef __GNUC__
extern  void  __vlog10(double *, double *, long, long, long);
void    vlog10() __attribute__ ((weak, alias ("__vlog10")));
#endif

extern	const du	__logtabhi[];
extern	const du	__logtablo[];
extern	const du	__log_ru[];

static const long long	twop7 =
{0x4060000000000000ll};

static	const du	twopm7 =
{D(0x3f800000, 0x00000000)};

static const du	log2_lead =
{D(0x3fe62e42, 0xfefa4000)};

static const du	log2_trail =
{D(0xbd48432a, 0x1b0e2634)};

static const du	Scaleup =
{D(0x43300000, 0x00000000)};

/* coefficients for polynomial approximation of log(1 + t) on +/- 1/256   */

static const du	P[] =
{
{D(0x3ff00000, 0x00000000)},
{D(0xbfe00000, 0x00000001)},
{D(0x3fd55555, 0x55509ba5)},
{D(0xbfcfffff, 0xffeb6526)},
{D(0x3fc999b4, 0xdfed6fe4)},
{D(0xbfc55576, 0x66472e04)},
};

static const du	Loge =
{D(0x3fdbcb7b, 0x1526e50e)};

static const	du	Qnan =
{D(QNANHI, QNANLO)};

static const	du	Inf =
{D(0x7ff00000, 0x00000000)};

static const	du	Neginf =
{D(0xfff00000, 0x00000000)};

#define MAXEXP	0x7ffu

#define	MINEXP	0x001u


/* ====================================================================
 *
 * FunctionName		vlog10
 *
 * Description		computes vector common logarithm of arg
 *
 * ====================================================================
 */

#ifdef _SW_PIPELINE

/* If compiler supports software pipelining, use this algorithm; note that
 * denormal args are not supported.
 */

void
__vlog10( double *x, double *y, long count, long stridex, long stridey )
{
#ifdef _32BIT_MACHINE

unsigned int	ix;

#else

unsigned long long ix;

#endif

long	i;
int	j;
int	m;
int	k;
double	u;
double	t;
double	xmu;
double	q;
double	l_lead, l_trail;
double	w;
double	result;

	/* i = 0, 1, ..., n-1; y[i*stridey] = log(x[i*stridex])	*/

	for ( i=0; i<count; i++ )
	{
#ifdef _PREFETCH
#pragma prefetch_ref=*(x+8)
#pragma prefetch_ref=*(y+8)
#endif

		/* extract exponent and sign of x for some quick screening */

		ix = *(unsigned long long *)x;	/* copy arg to a long long */

		m = (ix >> DMANTWIDTH);		/* shift off mantissa	*/
		j = m - MINEXP;

		m -= DEXPBIAS;

		/* normalize x and compute the nearest 1/128th to x */
	
		ix &= (DSIGNMASK & DEXPMASK);	/* mask off sign and exponent 
						 * bits of x
						 */
		ix |= twop7;	/* set exponent of x to 0x406 */

		/* adjust scaled arg	*/

		LL2DBL(ix, w);

		k = ROUND(w);

		u = k;

		k -= 128;

		xmu = twopm7.d*(w - u);

		t = __log_ru[k].d*xmu;

		/* avoid loss of significance for values of x near two
		   by adjusting index; effectively u is divided by two.
		   The logtable has been adjusted for this.
		*/

		if ( k > 64 )
			m++;

		q = ((((P[5].d*t + P[4].d)*t + P[3].d)*t + 
				P[2].d)*t + P[1].d)*(t*t);

		l_lead = __logtabhi[k].d;
		l_trail = __logtablo[k].d;

		l_lead += m*log2_lead.d;
		l_trail += m*log2_trail.d;

		result = l_lead + (t + (q + l_trail));

		result *= Loge.d;

		/* take care of negative args, NaNs, and Infinities     */

		if ( j >= (MAXEXP-MINEXP) )
			result = Qnan.d;

		if ( *x == 0.0 )
			result = Neginf.d;

		if ( *x == Inf.d )
			result = Inf.d;

		*y = result;

		y += stridey;

		x += stridex;
	}
}

#else

void
__vlog10( double *x, double *y, long count, long stridex, long stridey )
{
#ifdef _32BIT_MACHINE

unsigned int	ix;

#else

unsigned long long ix;

#endif

long	i;
int	j;
int	m;
int	k;
double	u;
double	t;
double	xmu;
double	q;
double	l_lead, l_trail;
double	w, z;
double	result;

	/* i = 0, 1, ..., n-1; y[i*stridey] = log(x[i*stridex])	*/

	for ( i=0; i<count; i++ )
	{
		/* extract exponent and sign of x for some quick screening */

		z = *x;
		w = z;

#ifdef _32BIT_MACHINE

		DBLHI2INT(w, ix);	/* copy arg to an int	*/
#else
		DBL2LL(w, ix);
#endif
		m = (ix >> DMANTWIDTH);		/* shift off mantissa	*/
		j = m;

		if ( m == 0 )
		{
			z *= Scaleup.d;
			w = z;

#ifdef _32BIT_MACHINE

			DBLHI2INT(w, ix); /* copy scaled arg to an int   */
#else
			DBL2LL(w, ix);	/* copy scaled arg to a long long */
#endif
			m = (ix >> DMANTWIDTH); /* shift off mantissa	*/
			m -= 52;	/* adjust for scaling	*/
		}

		m -= DEXPBIAS;

		/* normalize x and compute the nearest 1/128th to x */
	
		ix &= (DSIGNMASK & DEXPMASK);	/* mask off sign and exponent 
						 * bits of x
						 */
		ix |= twop7;	/* set exponent of x to 0x406 */

		/* adjust scaled arg	*/

#ifdef _32BIT_MACHINE

		INT2DBLHI(ix, w);
#else
		LL2DBL(ix, w);
#endif
		k = ROUND(w);

		u = k;

		k -= 128;

		xmu = twopm7.d*(w - u);

		t = __log_ru[k].d*xmu;

		/* avoid loss of significance for values of x near two
		   by adjusting index; effectively u is divided by two.
		   The logtable has been adjusted for this.
		*/

		if ( k > 64 )
			m++;

		q = ((((P[5].d*t + P[4].d)*t + P[3].d)*t + 
				P[2].d)*t + P[1].d)*(t*t);

		l_lead = __logtabhi[k].d;
		l_trail = __logtablo[k].d;

		l_lead += m*log2_lead.d;
		l_trail += m*log2_trail.d;

		result = l_lead + (t + (q + l_trail));

		result *= Loge.d;

		/* take care of negative args, NaNs, and Infinities     */

		if ( j >= MAXEXP )
			result = Qnan.d;

		if ( z == 0.0 )
			result = Neginf.d;

		if ( z == Inf.d )
			result = Inf.d;

		*y = result;

		y += stridey;

		x += stridex;
	}
}
#endif

