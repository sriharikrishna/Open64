/* -*-Mode: C++;-*- */
/* $Id: ir_a2b_util.h,v 1.1 2005-01-12 22:37:14 eraxxon Exp $ */
/* * BeginRiceCopyright *****************************************************
 * 
 * ******************************************************* EndRiceCopyright */

/* ====================================================================
 * ====================================================================
 *
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/common/com/ir_a2b_util.h,v $
 * $Revision: 1.1 $
 * $Date: 2005-01-12 22:37:14 $
 *
 * Nathan Tallent.
 *
 * Description:
 *
 * Utilities for converting between ascii and whirl.
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef ir_a2b_util_INCLUDED
#define ir_a2b_util_INCLUDED


#include "defs.h"
#include "cxx_memory.h"			// for CXX_NEW
#include "errors.h"			// for Fail_FmtAssertion

/* ====================================================================
 *
 * Simple scheme for translating an enumerated type (values between 0
 * and n) to a unique string and vice-versa.  Should be quite adequate
 * for Open64.
 *
 * enumerated type to string: constant time; no additional memory
 *   allocation
 *
 * string to enumerated type: O(log n) where n is usually quite
 *   small. The first time the routine is called, an auxilliary table of
 *   size n pointers is allocated and sorted (O(n*log n)) to be later
 *   used with binary-search.
 *
 * ====================================================================
 */

// Given an enumerated type, 
//   enum TY_KIND {
//     KIND_APPLES,
//     KIND_ORANGES,
//     KIND_PEARS,
//     KIND_COUNT 
//   };     
//
// the user defines a table mapping the enumerated type to unique
// strings.  The table should have an entry for every value between 0
// and n and should use ir_a2b::i2s_tbl_entry_t as the base class of
// the entry type:
//
//   struct myentry_t : public ir_a2b::i2s_tbl_entry_t
//    ... 
//   };
//
//   myentry_t TYKindToNameTbl[KIND_COUNT] = 
//     myentry_t(KIND_APPLES,  "APPLES"),
//     myentry_t(KIND_ORANGES, "ORANGES"),
//     myentry_t(KIND_PEARS,   "PEARS")
//   };
//
// Given this table, the mappings can be performed using the routines
//   ir_a2b::MapIntToString(...)
//   ir_a2b::MapStringToInt(...)
//
// See symtab.cxx for one set of examples.

namespace ir_a2b {

//----------------------------------------------------------------------
// i2s_tbl_entry_t
//----------------------------------------------------------------------

// i2s_tbl_entry_t: The virtual base class for an integer->string
// table entry.
struct i2s_tbl_entry_t {
  i2s_tbl_entry_t() { }
  i2s_tbl_entry_t(const i2s_tbl_entry_t& x) { }
  virtual ~i2s_tbl_entry_t() { }

  // get the integer representation for the entry
  virtual INT getEnumVal() = 0;
  
  // get the string for the entry
  virtual const char* getStr() = 0;
  
  // cmp: for use with qsort(), bsearch()
  static int 
  cmp(const void * entry1, const void * entry2) {
    i2s_tbl_entry_t* e1 = (i2s_tbl_entry_t*)entry1;
    i2s_tbl_entry_t* e2 = (i2s_tbl_entry_t*)entry2;
    return strcmp(e1->getStr(), e2->getStr());
  }
  
};


//----------------------------------------------------------------------
// MapIntToString
//----------------------------------------------------------------------

// MapIntToString: Given 1) a i2s_tbl_entry_t table (sorted by
// enumeration value) and the number of entries therein and 2) an
// enumeration value, return a string representation.
//
// N.B.: 'table' is not part of the template-parameter-list since we
// only need a separate instantiation for each TBL_ENTRY_T.
template <typename TBL_ENTRY_T>
const char*
MapIntToString(TBL_ENTRY_T* table, INT tablesz, INT val)
{
  if (0 <= val && val < tablesz) {
    return table[val].getStr();
  }
  else {
    char* buf = CXX_NEW_ARRAY(char, 128, Malloc_Mem_Pool);
    const char* first = table[0].getStr();
    sprintf(buf, "No-entry-for-'%d'-in-enumerated-type-containing-'%s'", 
	    val, first);
    return buf; 
    // N.B.: this is a harmless memory leak.  We choose this instead
    // of using what would turn into several static buffers and an
    // increased memory footprint.
  }
}

//----------------------------------------------------------------------
// MapStringToInt
//----------------------------------------------------------------------

// MapStringToInt: Given 1) a i2s_tbl_entry_t table (sorted by
// enumeration value) and the number of entries therein and 2) a
// string, return an enumeration value.
//
// N.B.: 'table' is part of the template-parameter-list to ensure
// there is a separate instantiation for each distinct source table.
template <typename TBL_ENTRY_T, TBL_ENTRY_T* table, INT tablesz>
INT
MapStringToInt(const char* str)
{
  struct key_t : public i2s_tbl_entry_t {
    key_t(const char* s_) : s(s_) { }
    virtual ~key_t() { }
    
    INT getEnumVal() { return 0; }
    const char* getStr() { return s; }
    
    const char* s;
  };

  // A copy of 'table', but sorted by string (instead of integer) value
  static TBL_ENTRY_T* Str2IntTbl = NULL; 
  
  // Compute translation table on demand (shallow copy of source table is fine)
  if (!Str2IntTbl) {
    Str2IntTbl = CXX_NEW_ARRAY(TBL_ENTRY_T, tablesz, Malloc_Mem_Pool);
    memcpy(Str2IntTbl, table, sizeof(TBL_ENTRY_T) * tablesz);
    qsort(Str2IntTbl, tablesz, sizeof(TBL_ENTRY_T), i2s_tbl_entry_t::cmp);
  }
  
  // Use binary search to find the enumeration value
  key_t key(str);
  TBL_ENTRY_T* found = 
    (TBL_ENTRY_T*)bsearch(&key, Str2IntTbl, tablesz, 
			  sizeof(TBL_ENTRY_T), i2s_tbl_entry_t::cmp);
  if (found) {
    return found->getEnumVal();
  }
  else {
    Fail_FmtAssertion("programming error mapping string to enum value.");
  }
}


}; // namespace ir_a2b


#endif /* ir_a2b_util_INCLUDED */
