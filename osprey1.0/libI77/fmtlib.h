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


#ifndef __FMTLIB_H__
#define __FMTLIB_H__
/* $Header: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/libI77/fmtlib.h,v 1.1.1.1 2002-05-22 20:09:15 dsystem Exp $ */
extern char *icvt(int, int *, int *, char *);
extern char *llcvt(ftnll , int *, int *, char *);
extern char *ozcvt(unsigned char *, int , int *, int, char * );
#ifdef I90
extern char *bcvt(unsigned char *, int , int *, char * );
#endif

#define MAXOCTLENGTH (32*8+2)/3+1	/* allow 32-byte items */

#endif /* !__FMTLIB_H__ */
