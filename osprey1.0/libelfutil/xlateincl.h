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



/*
  This is a uniform set of base includes for the xlate code
  This is the #include in the c files.
  xlateincl.h

  $Revision: 1.1.1.1 $
  $Date: 2002-05-22 20:09:22 $

*/

#ifndef _LP64
#include <sgidefs.h>
#endif /* _LP64 */
#include <stdio.h> /* for debug printf and for NULL decl */
#include "xlatebase.h"
#include "syn.h"
#include <elf.h>
#include <libelf.h>
#include <dwarf.h>
#include <libdwarf.h>
#include <libXlate.h>
#include <cmplrs/xlate.h>
#include <cmplrs/leb128.h>
#include <bstring.h> /* for bzero */
#include <string.h> /* for memcpy */
#include <stdlib.h>
#include "xlateTypes.h"
#include "xlate_proTypes.h"
#include "xlatexterns.h"
#include "xlate_pro_externs.h"
