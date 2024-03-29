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


/* =======================================================================
 * =======================================================================
 *
 *  Module: note.c
 *  $Revision: 1.1.1.1 $
 *  $Date: 2002-05-22 20:06:26 $
 *  $Author: dsystem $
 *  $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/cg/note.cxx,v $
 *
 *  Revision comments:
 *
 *  8-Mar-1994 - Initial version
 *
 *  Description:
 *  ============
 *
 *  Compiler to user note implementation.
 *
 * =======================================================================
 * =======================================================================
 */


#include "defs.h"
#include "config.h"
#include "tracing.h"
#include "erglob.h"

#include "tn.h"
#include "bb.h"
#include "op.h"

#include "note.h"

/* =======================================================================
 *
 *  NOTE_Add_To_BB
 *
 *  See interface description.
 *
 * =======================================================================
 */
void
NOTE_Add_To_BB(
  BB *bb,
  NOTE_HANDLER handler,
  NOTE_INFO   *info
)
{
  NOTE *comment = TYPE_MEM_POOL_ALLOC(NOTE,&MEM_pu_pool);
  NOTE_handler(comment) = handler;
  NOTE_info(comment) = info;
  BB_Add_Annotation (bb, ANNOT_NOTE, comment);
}      


/* =======================================================================
 *
 *  NOTE_BB_Act
 *
 *  See interface description.
 *
 * =======================================================================
 */
void
NOTE_BB_Act(
    BB          *bb,
    NOTE_ACTION  action,
    FILE        *file
)
{
  ANNOTATION *ant;

  for (ant = ANNOT_First (BB_annotations(bb), ANNOT_NOTE);
       ant != NULL;
       ant = ANNOT_Next (ant, ANNOT_NOTE))
  {
    NOTE_ACT(ANNOT_note(ant), action, file);
  }
}


/* =======================================================================
 *
 *  NOTE_Retrieve_Note_For_Handler
 *
 *  See interface description.
 *
 * =======================================================================
 */
NOTE_INFO *
NOTE_Retrieve_Note_For_Handler(
    BB          *bb,
    NOTE_HANDLER  handler
)
{
  ANNOTATION *ant;

  for (ant = ANNOT_First (BB_annotations(bb), ANNOT_NOTE);
       ant != NULL;
       ant = ANNOT_Next (ant, ANNOT_NOTE))
  {
    if (NOTE_handler(ANNOT_note(ant)) == handler)
      return NOTE_info(ANNOT_note(ant));
  }
  return NULL;
}
