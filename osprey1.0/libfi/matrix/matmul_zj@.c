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


#pragma ident "@(#) libfi/matrix/matmul_zj@.c	92.1	07/09/99 15:18:08"

#include "matmul.h"

/*
 * Name of this entry point
 */
#define NAME _MATMUL_ZJ
/*
 * Name of routine called do computation (if any)
 * Replace trailing @ with leading underscore when f90 is gen compiler for mpp.
 */
#if defined(_UNICOS)
#define SUBNAME JDGEMMX@
#elif defined(UNIX_NAMING_CONVENTION)
#define SUBNAME _JDGEMMX
#else
#define SUBNAME jdgemmx__
#endif
/*
 * Type of constants alpha and beta
 */
#define RESULTTYPE	_f_real16

#if defined(_UNICOS)
#pragma _CRI duplicate _MATMUL_ZJ as MATMUL_ZJ@
#endif
void
NAME(DopeVectorType *RESULT, DopeVectorType *MATRIX_A, 
	DopeVectorType *MATRIX_B)
{
    void    SUBNAME();
    RESULTTYPE *Ar, *Ai;	/* pointers to real and imaginary parts of B */
    RESULTTYPE *Cr, *Ci;	/* pointers to real and imaginary parts of C */
    MatrixDimenType matdimdata, *MATDIM;
#if !defined(_SOLARIS) && !defined(_CRAYMPP) && !defined(_ABSOFT)
    const RESULTTYPE   one =  (RESULTTYPE) 1.0;
    const RESULTTYPE   zero = (RESULTTYPE) 0.0;
#else
    struct {
        _f_int4 ireal1;         /* first 32 bits of real part */
        _f_int4 irest2[7];      /* rest of 256-bit double complex */
        } one, zero;

    memset(&zero, 0, sizeof(zero));

    one = zero;
    one.ireal1 =  017777600000;
#endif

        MATDIM = (MatrixDimenType *) &matdimdata;

    /*
     * Parse dope vectors, and perform error checking.
     */

    _premult(RESULT, MATRIX_A, MATRIX_B, MATDIM);

    /*
     * Do real and imaginary parts separately.
     */

    Ar = (RESULTTYPE *) MATDIM->A;
    Ai = Ar + 1;
    MATDIM->inc1a *= 2;
    MATDIM->inc2a *= 2;

    Cr = (RESULTTYPE *) MATDIM->C;
    Ci = Cr + 1;
    MATDIM->inc1c *= 2;
    MATDIM->inc2c *= 2;

    /*
     * Perform the matrix multiplication.
     * Do the transposed problem: C' = B'*A'
     */

    /* real part */
    SUBNAME(&MATDIM->n, &MATDIM->m, &MATDIM->k, &one, MATDIM->B, &MATDIM->inc2b,
    	    &MATDIM->inc1b, Ar, &MATDIM->inc2a, &MATDIM->inc1a,
	    &zero, Cr, &MATDIM->inc2c, &MATDIM->inc1c);
    /* imaginary part */
    SUBNAME(&MATDIM->n, &MATDIM->m, &MATDIM->k, &one, MATDIM->B, &MATDIM->inc2b,
    	    &MATDIM->inc1b, Ai, &MATDIM->inc2a, &MATDIM->inc1a,
	    &zero, Ci, &MATDIM->inc2c, &MATDIM->inc1c);
    return;
}
