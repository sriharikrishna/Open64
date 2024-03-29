//-*-c++-*-
// ====================================================================
// ====================================================================
//
// Module: opt_leaf_iter.cxx
// $Revision: 1.1.1.1 $
// $Date: 2002-05-22 20:06:49 $
// $Author: dsystem $
// $Source: /isms/cmplrs.src/osprey1.0/be/opt/RCS/opt_leaf_iter.cxx
//
// ====================================================================
//
// Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of version 2 of the GNU General Public License as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it would be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// Further, this software is distributed without any warranty that it
// is free of the rightful claim of any third person regarding
// infringement  or the like.  Any license provided herein, whether
// implied or otherwise, applies only to this software file.  Patent
// licenses, if any, provided herein do not apply to combinations of
// this program with other software, or any other product whatsoever.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
//
// Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
// Mountain View, CA 94043, or:
//
// http://www.sgi.com
//
// For further information regarding this notice, see:
//
// http://oss.sgi.com/projects/GenInfo/NoticeExplan
//
// ====================================================================
// ====================================================================


#ifdef USE_PCH
#include "opt_pch.h"
#endif // USE_PCH
#pragma hdrstop


#include "opt_leaf_iter.h"

void
Expand_onto_stack(STACK<CODEREP *> &s, CODEREP *const cr)
{
  switch (cr->Kind()) {
  case CK_OP:
    { // Stupid scoping to avoid bogus warning about i.
      // Not a leaf; iterate over leaves of kids.
      for (mINT16 i = 0; i < cr->Kid_count(); ++i) {
	Expand_onto_stack(s, cr->Opnd(i));
      }
    }
    break;

  case CK_IVAR:
    // Not a leaf; iterate over leaves of LHS (if any) and RHS.
    if (cr->Ilod_base() != NULL) {
      Expand_onto_stack(s, cr->Ilod_base());
    }
    if (cr->Istr_base() != NULL) {
      Expand_onto_stack(s, cr->Istr_base());
    }
    break;

  case CK_VAR:
  case CK_LDA:
  case CK_CONST:
  case CK_RCONST:
    // A leaf. Iterate over this node.
    s.Push(cr);
    break;

  default:
    FmtAssert(FALSE,
	      ("CODEREP_KIDS_ITER::Expand_onto_stack: Bad Kind()"));
    break;
  }
}

void
Expand_onto_stack(STACK<WN *> &s, WN *const wn)
{
  Is_True(OPCODE_is_expression(WN_opcode(wn)),
	  ("Expand_onto_stack: WN_opcode must be expression op"));

  if (WN_kid_count(wn) == 0) {
    // A leaf. Iterate over this node.
    s.Push(wn);
  }
  else {
    // Not a leaf. Iterate over leaves of kids.
    for (mINT16 i = 0; i < WN_kid_count(wn); i++) {
      Expand_onto_stack(s, WN_kid(wn, i));
    }
  }
}

