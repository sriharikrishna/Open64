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


/* -*-Mode: c++;-*- (Tell emacs to use c++ mode) */
#ifndef strtab_INCLUDED
#define strtab_INCLUDED
/* ====================================================================
 * ====================================================================
 *
 * Module: strtab.h
 * $Revision: 1.4 $
 * $Date: 2006-11-11 22:26:50 $
 *
 * Description:
 *
 * String table external interface.
 *
 * ====================================================================
 * ====================================================================
 */



#ifndef symtab_idx_INCLUDED
#include "symtab_idx.h"
#endif

//#include "HashTable.h"
//using namespace stlCompatibility;

// From a user point of view, the string table is a collection of unique
// strings, each of which can be indentified by STR_IDX.  The actual
// implementation is transparent to the user.

// init an empty table; use by producer (e.g., front end)
// size is string buffer size in bytes
extern void
Initialize_Strtab (UINT32 size);

// initialized the string table with the strtab from an input file
extern void
Initialize_Strtab (const char *buf, UINT32 size);

// add string to string table
extern STR_IDX
Save_Str (const char *str);

// concat two strings and then add to string table
extern STR_IDX
Save_Str2 (const char *s1, const char *s2);

// concat two strings and an int and add to string table
extern STR_IDX
Save_Str2i (const char *s1, const char *s2, UINT i);

extern char *
Index_To_Str (STR_IDX idx);

struct STRING_TABLE
{
    char &operator[] (STR_IDX idx) {
	return *Index_To_Str (idx);
    }
};

// New code should use Str_Table[idx] to get the string from an index
extern STRING_TABLE Str_Table;

extern UINT32
STR_Table_Size ();

// for compatibility
inline char *
Index_To_Str (STR_IDX idx, void *)	{ return Index_To_Str (idx); }

#define Str_To_Index(s,t)	(s)

// for character array table
extern UINT32
TCON_strtab_size ();

extern char *
TCON_strtab_buffer ();

extern void
Initialize_TCON_strtab (UINT32 size);

extern void
Initialize_TCON_strtab (const char *buf, UINT32 size);

// (similar to strncpy) s1 might not be null-termianted
extern UINT32
Save_StrN (const char *s1, UINT32 len);

extern char *
Index_to_char_array (UINT32 idx);

#ifdef MONGOOSE_BE

#include <map>
using namespace std;

#ifndef mempool_allocator_INCLUDED
#include "mempool_allocator.h"
#endif

// merge in a strtab read from an input file with the current string table;
// returns a mapping from the string indices of the "buf" to the 
// corresponding string indices in the merged table.

class STR_IDX_MAP {
private:


  struct STR_IDX_compare {
    bool operator() (const STR_IDX idx1, const STR_IDX idx2) const
    {
      return idx1 < idx2;
    }};
  
  typedef map<STR_IDX, STR_IDX, STR_IDX_compare, mempool_allocator<pair<const STR_IDX,STR_IDX> > > 
    rep_type;

  rep_type rep;  

public:

  STR_IDX_MAP ( rep_type::size_type n = 0,
		rep_type::key_compare cmp  = rep_type::key_compare(),
                rep_type::allocator_type a = rep_type::allocator_type() )
    : rep(cmp)
    { }

  
  STR_IDX& operator[](STR_IDX idx) { return rep[idx]; }
  STR_IDX operator[](STR_IDX idx) const {
    rep_type::const_iterator i = rep.find(idx);
    return i != rep.end() ? i->second : STR_IDX(0);
  }
  
};

extern void
Merge_Strtab (const char *buf, UINT32 size, STR_IDX_MAP& map);

extern void
Merge_TCON_Strtab (const char *buf, UINT32 size, STR_IDX_MAP& map);

#endif // MONGOOSE_BE


#endif /* strtab_INCLUDED */
