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


//-*-c++-*-
/* ====================================================================
 * Module: privatize_common.cxx
 * $Revision: 1.1.1.1 $
 * $Date: 2002-05-22 20:06:32 $
 * $Author: dsystem $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/com/privatize_common.h,v $
 *
 * Revision history:
 *  11-12-97 : First created by Dave Kohr
 *
 * Description:
 * fixes bug involving COMMON/inlining/MP privatization
 *
 * Exported functions:
 * Rename_Privatized_COMMON()
 * ST_Source_COMMON_Block()
 * ==================================================================== */

#ifndef privatize_common_INCLUDED
#define privatize_common_INCLUDED

#ifndef defs_INCLUDED
#include "defs.h"
#endif

#ifndef wn_INCLUDED
#include "wn.h"
#endif

#ifndef stab_INCLUDED
#include "stab.h"
#endif

#ifndef cxx_template_INCLUDED
#include "cxx_template.h"

#endif

#ifndef cxx_hash_INCLUDED
#include "cxx_hash.h"
#endif


typedef HASH_TABLE<ST *, ST *> RENAMING_MAP;

typedef HASH_TABLE<ST *, BOOL> ST_SET;

class RENAMING_SCOPE {
  enum { NUM_HASH_ELEMENTS = 1021 };
public:
  MEM_POOL *pool; // for all memory allocation by this object
  WN *priv_list;  // pragma block containing privatization pragmas; is
                  // NULL for a scope outside any parallel regions with
                  // PRIVATE clauses
  RENAMING_MAP map; // map of original to renamed COMMON ST's
  ST_SET ignore;  // ST's that don't need to be renamed
  HASH_TABLE<WN *, BOOL> old_prags; // original PRIVATE pragmas in priv_list
                                    // for STs that must be renamed
  RENAMING_SCOPE(WN *_priv_list, MEM_POOL *_pool)
    : priv_list(_priv_list), pool(_pool), map(NUM_HASH_ELEMENTS, _pool),
      ignore(NUM_HASH_ELEMENTS, _pool), old_prags(NUM_HASH_ELEMENTS, _pool)
    { }
  ~RENAMING_SCOPE();
};

typedef STACK<RENAMING_SCOPE *> RENAMING_STACK;

extern void
Rename_Privatized_COMMON(WN *wn, RENAMING_STACK *stack);

extern ST *
ST_Source_COMMON_Block(ST *st, ST **split);

#endif  // #ifndef privatize_common_INCLUDED
