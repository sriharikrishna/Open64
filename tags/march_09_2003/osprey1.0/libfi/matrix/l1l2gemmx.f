C
C
C  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.
C
C  This program is free software; you can redistribute it and/or modify it
C  under the terms of version 2.1 of the GNU Lesser General Public License 
C  as published by the Free Software Foundation.
C
C  This program is distributed in the hope that it would be useful, but
C  WITHOUT ANY WARRANTY; without even the implied warranty of
C  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
C
C  Further, this software is distributed without any warranty that it is
C  free of the rightful claim of any third person regarding infringement 
C  or the like.  Any license provided herein, whether implied or 
C  otherwise, applies only to this software file.  Patent licenses, if
C  any, provided herein do not apply to combinations of this program with 
C  other software, or any other product whatsoever.  
C
C  You should have received a copy of the GNU Lesser General Public 
C  License along with this program; if not, write the Free Software 
C  Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307, 
C  USA.
C
C  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
C  Mountain View, CA 94043, or:
C
C  http://www.sgi.com
C
C  For further information regarding this notice, see:
C
C  http://oss.sgi.com/projects/GenInfo/NoticeExplan
C
C


      subroutine _L1L2GEMMX
     $                       ( m, n, k, alpha, a, inc1a, inc2a,
     $                       b, inc1b, inc2b, beta, c, inc1c, inc2c )
CDIR$ ID "@(#) libfi/matrix/l1l2gemmx.f	92.0	10/08/98 14:37:14"
      entry L1L2GEMMX_
     $                       ( m, n, k, alpha, a, inc1a, inc2a,
     $                       b, inc1b, inc2b, beta, c, inc1c, inc2c )
      logical(KIND=2) alpha, beta
C     logical*2       alpha, beta
      logical(KIND=2) temp
C     logical*2       temp
      logical(KIND=1) a( * )
C     logical*1       a( * )
      logical(KIND=2) b( * )
C     logical*2       b( * )
      logical(KIND=2) c( * )
C     logical*2       c( * )

      include         "lgemmx.h"
