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


#pragma ident "@(#) libfi/matrix/matmul_c4z.c	92.1	07/09/99 15:18:08"

#include "matmul.h"

/*
 * Name of this entry point:
 */
#define NAME _MATMUL_C4Z

/*
 * Subroutine called to do computation:
 */
#if defined(UNICOS) || defined(UNIX_NAMING_CONVENTION)
#define SUBNAME	_S4DGEMMX
#else
#define SUBNAME	s4dgemmx__
#endif

/*
 * Type of constants alpha and beta
 */
#define RESULTTYPE	_f_real16

void
NAME(DopeVectorType * RESULT, DopeVectorType * MATRIX_A,
     DopeVectorType * MATRIX_B)
{
    void    SUBNAME();

    _f_real4	*Ar, *Ai;
    RESULTTYPE	*Br, *Bi;
    RESULTTYPE	*Cr, *Ci;
    MatrixDimenType matdimdata, *MATDIM;

#if !defined(_SOLARIS) && !defined(_CRAYMPP) && !defined(_ABSOFT)
    const RESULTTYPE   neg_one =  (RESULTTYPE) (-1.0);
    const RESULTTYPE   one =  (RESULTTYPE) 1.0;
    const RESULTTYPE   zero = (RESULTTYPE) 0.0;
#else
    struct {
        _f_int4 ireal1;         /* first 32 bits of real part */
        _f_int4 irest2[7];      /* rest of 256-bit double complex */
        } neg_one, one, zero;

    memset(&zero, 0, sizeof(zero));

    one = zero;
    one.ireal1 =  017777600000;

    neg_one = zero;
    neg_one.ireal1=  037777600000;
#endif

        MATDIM = (MatrixDimenType *) &matdimdata;

    /*
     * Parse dope vectors, and perform error checking.
     */

    _premult(RESULT, MATRIX_A, MATRIX_B, MATDIM);

    /*
     * Do the real and imaginary parts separately.
     */

    Ar = (_f_real4 *) MATDIM->A;
    Ai = Ar + 1;
    MATDIM->inc1a *=2;
    MATDIM->inc2a *=2;

    Br = (RESULTTYPE *) MATDIM->B;
    Bi = Br + 1;
    MATDIM->inc1b *= 2;
    MATDIM->inc2b *= 2;

    Cr = (RESULTTYPE *) MATDIM->C;
    Ci = Cr + 1;
    MATDIM->inc1c *= 2;
    MATDIM->inc2c *= 2;

    /*
     * Perform the matrix multiplication.
     * Note:
     * (Cr + Ci*i) = (Ar + Ai*i)*(Br + Bi*i)
     *             = (Ar*Br - Ai*Bi) + (Ar*Bi + Ai*Br)*i
     */

    /* real part */
    SUBNAME(&MATDIM->m,&MATDIM->n,&MATDIM->k,&one,Ar,&MATDIM->inc1a,
        &MATDIM->inc2a,Br,&MATDIM->inc1b,&MATDIM->inc2b,&zero,Cr,
	&MATDIM->inc1c,&MATDIM->inc2c);
    SUBNAME(&MATDIM->m,&MATDIM->n,&MATDIM->k,&neg_one,Ai,&MATDIM->inc1a,
        &MATDIM->inc2a,Bi,&MATDIM->inc1b,&MATDIM->inc2b,&one,Cr,
	&MATDIM->inc1c,&MATDIM->inc2c);

    /* imaginary part */
    SUBNAME(&MATDIM->m,&MATDIM->n,&MATDIM->k,&one,Ar,&MATDIM->inc1a,
        &MATDIM->inc2a,Bi,&MATDIM->inc1b,&MATDIM->inc2b,&zero,Ci,
        &MATDIM->inc1c,&MATDIM->inc2c);
    SUBNAME(&MATDIM->m,&MATDIM->n,&MATDIM->k,&one,Ai,&MATDIM->inc1a,
        &MATDIM->inc2a,Br,&MATDIM->inc1b,&MATDIM->inc2b,&one,Ci,
	&MATDIM->inc1c,&MATDIM->inc2c);
}
