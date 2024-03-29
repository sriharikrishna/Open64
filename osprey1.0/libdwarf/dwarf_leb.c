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



#include "config.h"
#include "dwarf_incl.h"
#include <stdio.h>


/*
    decode ULEB
*/
Dwarf_Unsigned
_dwarf_decode_u_leb128 (
    Dwarf_Small     	*leb128,
    Dwarf_Word      	*leb128_length
)
{
    Dwarf_Small     	byte;
    Dwarf_Word		word_number;
    Dwarf_Unsigned  	number;
    Dwarf_Sword  	shift;
    Dwarf_Sword		byte_length;

    if ((*leb128 & 0x80) == 0) {
	if (leb128_length != NULL) *leb128_length = 1;
	return(*leb128);
    }
    else if ((*(leb128 + 1) & 0x80) == 0) {
	if (leb128_length != NULL) *leb128_length = 2;

	word_number = *leb128 & 0x7f;
	word_number |= (*(leb128 + 1) & 0x7f) << 7;
	return(word_number);
    }
    else if ((*(leb128 + 2) & 0x80) == 0) {
	if (leb128_length != NULL) *leb128_length = 3;

	word_number = *leb128 & 0x7f;
	word_number |= (*(leb128 + 1) & 0x7f) << 7;
	word_number |= (*(leb128 + 2) & 0x7f) << 14;
	return(word_number);
    }
    else if ((*(leb128 + 3) & 0x80) == 0) {
	if (leb128_length != NULL) *leb128_length = 4;

	word_number = *leb128 & 0x7f;
	word_number |= (*(leb128 + 1) & 0x7f) << 7;
	word_number |= (*(leb128 + 2) & 0x7f) << 14;
	word_number |= (*(leb128 + 3) & 0x7f) << 21;
	return(word_number);
    }

    number = 0;
    shift = 0;
    byte_length = 1;
    byte = *(leb128);
    for (;;) {
	number |= (byte & 0x7f) << shift;
	shift += 7;

	if ((byte & 0x80) == 0) {
	    if (leb128_length != NULL) *leb128_length = byte_length;
	    return(number);
	}

	byte_length++;
	byte = *(++leb128);
    }
}


/*
    decode SLEB
*/
Dwarf_Signed
_dwarf_decode_s_leb128 (
    Dwarf_Small     	*leb128,
    Dwarf_Word          *leb128_length
)
{
    Dwarf_Small     	byte = *leb128;
    Dwarf_Sword		word_number = 0;
    Dwarf_Signed    	number;
    Dwarf_Bool	    	sign = 0;
    Dwarf_Bool		ndone = true;
    Dwarf_Sword  	shift = 0;
    Dwarf_Sword		byte_length = 0;

    while (byte_length++ < 4) {
	sign = byte & 0x40;
	word_number |= (byte & 0x7f) << shift;
	shift += 7;

	if ((byte & 0x80) == 0) {
	    ndone = false;
	    break;
	}
	byte = *(++leb128);
    }

    number = word_number;
    while (ndone) {
	sign = byte & 0x40;
        number |= (byte & 0x7f) << shift;
        shift += 7;

	if ((byte & 0x80) == 0) {
	    break;
	}

	    /* 
		Increment after byte has been placed in
	        number on account of the increment already
	        done when the first loop terminates.  That
	        is the fourth byte is picked up and byte_length
	        updated in the first loop.  So increment not
	        needed in this loop if break is taken.
	    */
	byte_length++;
        byte = *(++leb128);
    }

    if ((shift < sizeof(Dwarf_Signed)*8) && sign) 
	number |= - (1 << shift);

    if (leb128_length != NULL) *leb128_length = byte_length;
    return(number);
}


