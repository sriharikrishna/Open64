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


/* $Header: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/libF77/z_div.c,v 1.1.1.1 2002-05-22 20:09:13 dsystem Exp $ */
#include "cmplrs/host.h"
#include <math.h>
#include "moremath.h"
#include <stdlib.h>
#include "cmplx.h"

dcomplex __zdiv(double_t adreal, double_t adimag, double_t bdreal, double_t bdimag)
{
  double_t ratio, den;
  double_t abr, abi;
  dcomplex c;
  
  if( (abr = bdreal) < 0.)
    abr = - abr;
  if( (abi = bdimag) < 0.)
    abi = - abi;
  if( abr <= abi ) {
    ratio = bdreal / bdimag ;
    den = bdimag * (1 + ratio*ratio);
    c.dreal = (adreal*ratio + adimag) / den;
    c.dimag = (adimag*ratio - adreal) / den;
  } else {
    ratio = bdimag / bdreal ;
    den = bdreal * (1 + ratio*ratio);
    c.dreal = (adreal + adimag*ratio) / den;
    c.dimag = (adimag - adreal*ratio) / den;
  }
  return c;
}

void z_div(dcomplex *c, dcomplex *a, dcomplex *b)
{
  *c = __zdiv(a->dreal, a->dimag, b->dreal, b->dimag);
}
