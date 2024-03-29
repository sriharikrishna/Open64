/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if 
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston MA 02111-1307, USA.

  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/


/* ====================================================================
 * ====================================================================
 *
 * Module: iter.c
 * $Revision: 1.1.1.1 $
 * $Date: 2002-05-22 20:06:20 $
 * $Author: dsystem $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/be/iter.cxx,v $
 *
 * Revision history:
 *  3-MAR-96 dahl - Original Version
 *
 * Description:
 *	PU iterator for BE driver.
 *
 * ====================================================================
 * ====================================================================
 */

#define rbi_CXX	"iter.c"
#ifdef _KEEP_RCS_ID
static char *rcs_id = iter_C"$Revision: 1.1.1.1 $";
#endif /* _KEEP_RCS_ID */

#include <elf.h>	/* for Elf64_Word */
#include "defs.h"
#include "stab.h"
#include "pu_info.h"	/* PU_Info */
#include "iter.h"	/* PU_INFO_ITER */
#include "wn.h"
#include "be_util.h"	/* Reset_Current_PU_Count, Advance_Current_PU_Count */
#include "errors.h"	/* FmtAssert */

/* ==================================================================== */
/* iterator functions for PU loop in back-end				*/
/* ==================================================================== */

static void Pu_Push(PU_INFO_ITER *iter, PU_Info *pu_tree)
{
  PU_INFO_ITER_stack(iter) = pu_tree;
  PU_INFO_ITER_sp(iter)++;
  FmtAssert(PU_INFO_ITER_sp(iter) <= PU_INFO_ITER_stack_size,
	    ("Pu_Push, stack overflow - too many nested procedures"));
}

static PU_Info *Pu_Pop(PU_INFO_ITER *iter)
{
  if (PU_INFO_ITER_sp(iter) == 0) {
    Is_True(0,("Pu_Pop, stack underflow, problem with nested procedure"));
    return NULL;
  }
  PU_INFO_ITER_sp(iter)--;
  return PU_INFO_ITER_stack(iter);
}

void Pu_Init(PU_INFO_ITER *iter, PU_Info *pu_tree)
{
  PU_INFO_ITER_sp(iter) = 0;
  PU_INFO_ITER_current(iter) = pu_tree;
  Reset_Current_PU_Count();
}

BOOL Pu_While(PU_INFO_ITER *iter)
{
  return PU_INFO_ITER_current(iter) != NULL;
}

/* check for child PUs added by MP lowering */
void Pu_Next(PU_INFO_ITER *iter)
{
  if (PU_Info_child(PU_INFO_ITER_current(iter))) {
    Pu_Push(iter, PU_INFO_ITER_current(iter));
    PU_INFO_ITER_current(iter) = PU_Info_child(PU_INFO_ITER_current(iter));
  } else {
    PU_INFO_ITER_current(iter) = PU_Info_next(PU_INFO_ITER_current(iter));
    if (PU_INFO_ITER_current(iter) == NULL && PU_INFO_ITER_sp(iter)) {
      while (PU_INFO_ITER_sp(iter)) {
	PU_INFO_ITER_current(iter) = Pu_Pop(iter);
	if (PU_Info_next(PU_INFO_ITER_current(iter)))
	  break;
      }
      PU_INFO_ITER_current(iter) = PU_Info_next(PU_INFO_ITER_current(iter));
    }
  }
  Advance_Current_PU_Count();
}
