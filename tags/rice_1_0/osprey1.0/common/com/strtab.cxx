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



#ifdef USE_PCH
#include "common_com_pch.h"
#endif /* USE_PCH */
#pragma hdrstop

#include <alloca.h>
#include <strings.h>

#ifndef _USE_STL_EXT
#include <map>
#else
#include <hash_map>			// stl hash table
#endif


#ifdef _SGI_SGI
#include <utility>    // for pair< > needed by sgi CC 
#endif

using namespace std;

#include "defs.h"
#include "errors.h"
#include "cxx_memory.h"			// for CXX_NEW

#include "strtab.h"

// The string table is implmeneted as a single character buffer, re-allocated
// if necessary.  Byte 0 of this buffer is alwasy NULL.  Each string is
// copied to the buffer once.  Duplicates are not entered.  A simple hash
// table is used to for quick lookup.  The hash table is a fixed size
// array of STR_BUCKET, each of which a dynamic array of <idx,size> pair.
// The size field is truncated to 8 bits.  It is used for 
// quick comparision to minimize the need to call strcmp.

// we support two types of strings: C-style null-terminated string and
// character array with a specified size (may contain null character within
// the array.

typedef UINT32 STR_INDEX;

#ifdef linux

#define STR_INDEX_size(xxx) (xxx & 0xff)
#define STR_INDEX_index(xxx) (xxx >> 8)

#else

// given a STR_INDEX (which is a 32-bit UINT), the following two functions
// extract the offset of the string in string table (first 24 bits), and the 
// size of the string (not real size, but a truncated_to_8_bits size). 

static inline mUINT8
STR_INDEX_size (STR_INDEX idx)		{ return idx & 0xff; }
static inline mUINT32
STR_INDEX_index (STR_INDEX idx)		{ return idx >> 8; }

#endif /* linux */

// given the offset of a string in string table, and the string size, 
// this function makes a 32-bit hybrid index (basically the reverse
// of the above two functions).

static inline STR_INDEX
make_STR_INDEX (UINT32 size, UINT32 idx)
{
    // if string length larger than 0xff, just use 0xff
    if (size > 0xff)
	size = 0xff;
    return (STR_INDEX) ((idx << 8) | size);
}

struct NULL_TERMINATED_STRING
{
    static char *get_str (char *buffer) {
	return buffer;
    }

    static const char *get_str (const char *buffer) {
	return buffer;
    }

    static UINT32 get_length (const char *buffer) {
	return strlen (buffer);
    }

    static UINT32 get_buffer_length (UINT32 length) {
	return length + 1;		// length + null character
    }


    static void copy (const char *str, UINT32 length, char *buffer) {
	memcpy (buffer, str, length+1);
    }
};


union UNALIGN_INT32
{
    char ch[sizeof(UINT32)];
    UINT32 n;

    UNALIGN_INT32 (UINT32 x) : n (x) {}

    UNALIGN_INT32 (const char *s) {
	for (INT32 i = 0; i < sizeof(UINT32); ++i)
	    ch[i] = *s++;
    }
};


struct CHARACTER_ARRAY
{
    // If the character array is less then 0xff bytes, we store the size in
    // The First Byte Followed By The Array.  Otherwise, The First Byte Is Set
    // To 0xff And The Next 4 Bytes Hold The Size Of The Array As An Unsigned
    // Integer, And The Array Follows.

    static const char *get_str (const char *buffer) {
	if (*buffer != 0xff)
	    return buffer + 1;
	else
	    return buffer + sizeof(UINT32) + 1;
    }

    static char *get_str (char *buffer) {
	if (*buffer != 0xff)
	    return buffer + 1;
	else
	    return buffer + sizeof(UINT32) + 1;
    }

    static UINT32 get_length (const char *buffer) {
	if (*buffer != 0xff)

// Solaris workaround
// Solaris CC treats "char" and "unsigned char" differently! Actually here
// buffer is treated as "signed char", so when buffer >= 128, it treats
// buffer as a negative number, and later when it returns buffer to an
// "unsigned integer", that integer becomes a HUGE number.
// IRIX compiler seems to default "char" as "unsigned char".
// This bug only causes problem when length >= 128, for example, when
// running "ir_b2a heat.B".
//
// change char to unsigned char should solve this problem!
// Don't know how many other places still have this kind of problem!
// 
#if defined(_SOLARIS_SOLARIS) && !defined(__GNUC__)
{           
	    unsigned char c = *buffer;
	    return c;
}
#else
	    return *buffer;
#endif
	else {
	    UNALIGN_INT32 unalign (buffer + 1);
	    return unalign.n;
	}
    }

    static UINT32 get_buffer_length (UINT32 length) {
	return length < 0xff ? length + 1 : length + 1 + sizeof(UINT32);
    }

    static void copy (const char *str, UINT32 length, char *buffer) {
	if (length < 0xff) {
	    *buffer++ = length;
	} else {
	    *buffer++ = 0xff;
	    UNALIGN_INT32 unalign (length);
	    for (INT32 i = 0; i < sizeof(UINT32); ++i)
		*buffer++ = unalign.ch[i];
	}
	memcpy (buffer, str, length);
    }

}; // CHARACTER_ARRAY


template <class STR>
struct STR_TAB
{

    char *buffer;       // main buffer storing all strings in string table
    STR_IDX last_idx;   // end of current used storage
    UINT32 buffer_size;  

    // the following 4 function objects are required for setting up the
    // hash table declared after them.

// hash_key is the "key" of hashtable. STR_INDEX is the "data" of hashtable. 
// they two compose pair<key, STR_INDEX>, which can serve as the "value type"
// in map (created by me).

    // hash key, which is a pair of string pointer and string length
    struct hash_key {
	const char *str;
	mUINT32 size;

	hash_key () {}
	
	hash_key (const char *s, UINT32 l) : str (s), size (l) {}
	
	hash_key (const hash_key& p) : str (p.str), size (p.size) {}
    };


// Solaris CC workaround
// the hash function is trashed when using map instead of hash_map
#ifdef _USE_STL_EXT

    // hash function
    struct hash {
	UINT32 operator() (const hash_key &key) const {
	    UINT32 h = 0;
	    const UINT32 length = key.size;
	    const char *str = key.str;
	    for (UINT32 i = 0; i < length; ++i)
		h = 5 * h + str[i];
	    return h;
	}
    };
#endif
	    
// this function takes a STR_INDEX and assemble a "hash_key" of the
// correponding string. Note although the new hash_key.str points to
// some address in the big buffer instead of the original string (in
// source file or somewhere?), but the newly created "hash_key" will 
// still evaluate equally with the original string because of my new
// key_compare structure.

#ifndef _USE_STL_EXT
    struct Extract_key {
        const STR_TAB<STR>& strtab;   

        Extract_key (const STR_TAB<STR>& tab) : strtab (tab) {}
    
        hash_key operator() (STR_INDEX str_index) const {
            const char *str = strtab.buffer + STR_INDEX_index (str_index);
            if (STR_INDEX_size (str_index) < 0xff) {
                return hash_key (STR::get_str (str),
                                      STR_INDEX_size (str_index));
            } else {
                return hash_key (STR::get_str (str), STR::get_length (str));
            }
        }
    };

#else
   // how the hashtable convert a STR_INDEX into a hash_key   
    struct extract_key {
	const STR_TAB<STR>& strtab;

	extract_key (const STR_TAB<STR>& tab) : strtab (tab) {}
	
	hash_key operator() (STR_INDEX str_index) const {
	    const char *str = strtab.buffer + STR_INDEX_index (str_index);
	    if (STR_INDEX_size (str_index) < 0xff) {
		return hash_key (STR::get_str (str),
				      STR_INDEX_size (str_index));
	    } else {
		return hash_key (STR::get_str (str), STR::get_length (str));
	    }
	}
    };
#endif

// Solaris CC workaround
// a key_compare function is added for 'map'
// also added this data member into STR_TAB because of other changes
// made to STR_TAB 's constructor.

#ifndef _USE_STL_EXT
   Extract_key extract_key;

   struct js_compare {
        bool operator() (const hash_key& key1, const hash_key& key2) const {
            if(key1.size < key2.size) return true;
            return  strcmp(key1.str, key2.str) < 0;
             }
   };

#else

    // equality of two hash_keys
    struct equal {
	BOOL operator() (const hash_key& key1, const hash_key& key2) const {
	    return (key1.size == key2.size &&
		    key1.str[0] == key2.str[0] &&
		    memcmp (key1.str, key2.str, key1.size) == 0);
	}
    };
#endif

// hashtable<> is an extention to standard C++ STL, which Solaris CC does not
// support. This template is defined in "stl_hashtable.h". According to SGI,
// it is only an implementation for hash_map and hash_set. Here it is used as
// an interface.  

// STR_INDEX is defined as unsigned int, so it must be the "data". hash_key
// is a struct containing a string and it must be the real "key". The hash
// function, extract_kay function and equal_key function can be trashed if I
// replace it with "map".

// The hashtable is only used in "constructor" (5 lines below), size(), find_or_
// insert(), insert_unique(), begin(), end(), these member functions. I hope
// "map" also has similar functions.

#ifndef _USE_STL_EXT
    typedef std::map<hash_key, STR_INDEX, js_compare> HASHTABLE;
#else
    typedef std::hashtable<STR_INDEX, hash_key, hash, extract_key, equal> HASHTABLE;
#endif

    typedef pair<HASHTABLE::iterator, bool> RETURN_INSERT;

    HASHTABLE hash_table;

    // constructor

// Solaris CC workaround
// the new constructor for hash_table cannot contain extract_kay (*this), 
// so I had to move it out. 

#ifndef _USE_STL_EXT
    STR_TAB(UINT bucket_size) : buffer (NULL), last_idx (0), buffer_size (0),
				extract_key (*this), hash_table(js_compare())
				{ };
#else
    STR_TAB (UINT bucket_size) : hash_table (bucket_size, hash (), equal (),
                                             extract_key (*this)),
                            buffer (NULL), last_idx (0), buffer_size (0) {}
#endif

    // operations
    
    void reserve (UINT32 size) {
	if (size > buffer_size - last_idx) {
	    // realloc
	    while (size > buffer_size - last_idx) {
		if (buffer_size < 0x100000)
		    buffer_size *= 2;
		else
		    buffer_size += 0x80000;
	    }
	    buffer = (char *) MEM_POOL_Realloc (Malloc_Mem_Pool, buffer, 0,
						buffer_size); 
	}
    }
    
    void init_hash ();

    UINT32 insert (const char *str, UINT32 size);

    UINT32 insert (const char *str) {
	return insert (str, strlen (str));
    }
    
    void copy_str (const char *str, UINT32 size);

}; // STR_TAB




// when inserting a new string/size into the string table
//   first  - copy the string into stringtable's buffer
//   second - calculate its STR_INDEX according to its offset and size
//   third  - try to insert its STR_INDEX/hash_key into hash_table
//   fourth - if a same hash_key already exists, un-insert

template <class STR>
UINT32
STR_TAB<STR>::insert (const char *str, UINT32 size)
{
    UINT32 index = last_idx;

    copy_str (str, size);  //copy str into buffer at offset 'index'

    STR_INDEX new_index = make_STR_INDEX (size, index); 
    //calculate STR_INDEX for the newly copied string in buffer

// check this function out! I think, if two same strings have been inserted 
// into the buffer, then they must have different STR_INDEX because their 
// offset in the buffer differ. Then how could hash_table::find_or_insert() 
// figure out the old string's STR_INDEX? There must be something to do with
// hash_key and Equal_key. Yes, see stl_hashtable.h for details.
#ifdef _USE_STL_EXT

    STR_INDEX old_index = hash_table.find_or_insert (new_index);
#endif

// map has a different way to check whether there was already an element whose
// key is the same as the one to be inserted
#ifndef _USE_STL_EXT
    
    RETURN_INSERT return_insert;
    return_insert = hash_table.insert(make_pair( extract_key(new_index),
                                                 new_index ));
    
    if ( return_insert.second == false )  {
#else
    if (new_index != old_index) {
#endif
	// already inserted, reset buffer and return
	last_idx = index;

// this is a very compilcated expression, maybe later I declare a temp
// iterator of <hash_key, STR_INDEX> which hold the return value of 
// map::insert(), then I should be able to avoid this extra call.

#ifndef _USE_STL_EXT
        return STR_INDEX_index ( (return_insert.first)->second );
#else
        return STR_INDEX_index (old_index);
#endif

    }   else
	return index;

} // STR_TAB<STR>::insert


template <class STR>
void
STR_TAB<STR>::copy_str (const char *str, UINT32 size)
{
    UINT32 buffer_size = STR::get_buffer_length (size);
    reserve (buffer_size);
    STR::copy (str, size, buffer + last_idx);
    last_idx += buffer_size;
} // STR_TAB::copy_str


template <class STR>
void
STR_TAB<STR>::init_hash ()
{
    UINT32 idx = 1;			// first entry always null
    while (idx < last_idx) {

	UINT32 length = STR::get_length (buffer + idx);

// map doesn't have insert_unique() member function
// but its insert() will only insert elements with unique key anyway.

#ifndef _USE_STL_EXT
        STR_INDEX new_index = make_STR_INDEX (length, idx);
        hash_table.insert(make_pair(extract_key(new_index), new_index));
#else
	hash_table.insert_unique (make_STR_INDEX (length, idx));
#endif

	idx += STR::get_buffer_length (length);
    }
} // STR_TAB<STR>::init_hash


template <class STRTAB>
static inline void
initialize_strtab (STRTAB& strtab, UINT32 size)
{
    strtab.buffer_size = size;
    strtab.buffer = (char *) MEM_POOL_Alloc (Malloc_Mem_Pool, size);
    strtab.buffer[0] = 0;
    strtab.last_idx = 1;
}


template <class STRTAB>
static inline void
initialize_strtab (STRTAB& strtab, const char *buf, UINT32 size)
{

    if (strtab.hash_table.size() != 0)
        strtab.hash_table.erase(strtab.hash_table.begin(),
                                strtab.hash_table.end());
    strtab.buffer_size = size;
    strtab.buffer = (char *) MEM_POOL_Alloc (Malloc_Mem_Pool, size);
    bcopy (buf, strtab.buffer, size);
    strtab.last_idx = size;
    strtab.init_hash ();
    
} // Initialize_Strtab
    

// Global String table
static STR_TAB<NULL_TERMINATED_STRING> Strtab (1000);			       

STRING_TABLE Str_Table;

UINT32
STR_Table_Size ()
{
    return Strtab.last_idx;
}

// init an empty table; use by producer (e.g., front end)

void
Initialize_Strtab (UINT32 size)
{
    initialize_strtab (Strtab, size);
} 

// initialized the string table with the strtab from an input file
void
Initialize_Strtab (const char *buf, UINT32 size)
{
    initialize_strtab (Strtab, buf, size);
} // Initialize_Strtab


STR_IDX
Save_Str (const char *str)
{
    if (str == NULL)
	return 0;

    return Strtab.insert (str);

} // Save_Str


STR_IDX
Save_Str2 (const char *s1, const char *s2)
{
    UINT len = strlen (s1) + strlen(s2) + 1;
    char *new_str = (char *) alloca (len);
    strcpy (new_str, s1);
    strcat (new_str, s2);
    return Save_Str (new_str);
} // Save_Str2

STR_IDX
Save_Str2i (const char *s1, const char *s2, UINT i)
{
    UINT len = strlen (s1) + strlen(s2) + 17;
    char *new_str = (char *) alloca (len);
    sprintf(new_str, "%s%s%d", s1, s2, i);
    return Save_Str (new_str);
} // Save_Str2


char *
Index_To_Str (STR_IDX idx)
{
    Is_True (idx < Strtab.last_idx, ("Invalid STR_IDX"));
    return NULL_TERMINATED_STRING::get_str (Strtab.buffer + idx);
}

// character array table
const UINT32 TCON_STRTAB_HASH_SIZE = 512;
static STR_TAB<CHARACTER_ARRAY> TCON_strtab (TCON_STRTAB_HASH_SIZE);

UINT32
TCON_strtab_size ()
{
    return TCON_strtab.last_idx;
}

char *
TCON_strtab_buffer ()
{
    return TCON_strtab.buffer;
}

// init an empty table; use by producer
void
Initialize_TCON_strtab (UINT32 size)
{
    initialize_strtab (TCON_strtab, size);
}

// init the TCON strtab from an input file
void
Initialize_TCON_strtab (const char *buf, UINT32 size)
{
    initialize_strtab (TCON_strtab, buf, size);
} 



// add string of length "len"; string might not be null-terminated
UINT32
Save_StrN (const char *s1, UINT32 len)
{
    if (len == 0)
	return 0;

    return TCON_strtab.insert (s1, len);
} // Save_StrN

char *
Index_to_char_array (UINT32 idx)
{
    Is_True (idx < TCON_strtab.last_idx, ("Invalid TCON str index"));
    return CHARACTER_ARRAY::get_str (TCON_strtab.buffer + idx);
}


#ifdef MONGOOSE_BE

template <class STR, class MAP>
void
merge_strtab (STR_TAB<STR>& strtab, const char *buf, UINT32 size, MAP& map)
{
    map[0] = 0;
    UINT32 idx = 1;
    while (idx < size) {
	const char *str = STR::get_str (buf + idx);
	UINT32 size = STR::get_length (buf + idx);
	UINT32 new_idx = strtab.insert (str, size);
	map[idx] = new_idx;
	idx += STR::get_buffer_length (size);
    }
} // merge_strtab


// merge in a string table buffer from an input file, return a map from
// the old STR_IDX to the new (merged) STR_IDX

void
Merge_Strtab (const char *buf, UINT32 size, STR_IDX_MAP& map)
{
    merge_strtab (Strtab, buf, size, map);
}

// merge in a string table buffer from an input file, return a map from
// the old STR_IDX to the new (merged) STR_IDX

void
Merge_TCON_Strtab (const char *buf, UINT32 size, STR_IDX_MAP& map)
{
    merge_strtab (TCON_strtab, buf, size, map);
}

#endif // MONGOOSE_BE

