/*

  Copyright (C) 2003 Rice University. All rights reserved.

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

*/

#ifndef unparse_target_c_INCLUDED
#define unparse_target_c_INCLUDED
/* ====================================================================
 *
 * Module: unparse_target.h
 * $Revision: 1.2 $
 * $Date: 2003-06-13 23:05:29 $
 * $Author: broom $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/whirl2c/unparse_target_c.h,v $
 *
 * Revision history:
 *  06-Jun-03 - Original Version
 *
 * Description:
 *
 *     Runtime tests for the C target language.
 */

#include "unparse_target.h"
#include "whirl2c_common.h"
#include "intrn_info.h"
#include "token_names.h"

class Unparse_Target_C : public Unparse_Target {
public:
	~Unparse_Target_C () {};

	const char *Make_Valid_Name(const char *name, BOOL allow_dot)
	{ return WHIRL2C_make_valid_c_name(name); }

	const char *Get_St_Name(const ST *st, const char *original_name)
	{ return original_name; }

	const char *Intrinsic_Name(INTRINSIC intr_opc)
	{
	   const char *name;
	   
	   Is_True(INTRINSIC_FIRST<=intr_opc && intr_opc<=INTRINSIC_LAST,
		   ("Intrinsic Opcode (%d) out of range", intr_opc)); 
	   if (INTRN_c_name(intr_opc) != NULL)
	      name = INTRN_c_name(intr_opc);
	   else if (INTRN_rt_name(intr_opc) != NULL)
	      name = INTRN_rt_name(intr_opc);
	   else
	   {
	      Is_True(FALSE, 
		      ("Expected \"high_level\" or \"rt\" name in WN_intrinsic_name()"));
	      name =
		 Concat3_Strings("<INTR: ", Number_as_String(intr_opc, "%lld"), ">");
	   }

	   return name;
	}

	BOOL Avoid_Common_Suffix(void)
	{
	  return FALSE;
	}

        BOOL Reduce_Const_Ptr_Exprs(void)
	{
	  return TRUE;
	}

	BOOL Enter_Symtab_Pointee_Names(void)
	{
	  return FALSE;
	}

	BOOL Is_Binary_Or_Tertiary_Op (char c)
	{
          return (c==PLUS          || \
                  c==MINUS         || \
                  c==MULTIPLY      || \
                  c==DIVIDE        || \
                  c==BITAND        || \
                  c==BITOR         || \
                  c==MODULUS       || \
                  c==EQUAL         || \
                  c==NOT           || \
                  c==QUESTION_MARK || \
                  c==COLON         || \
                  c==LESS_THAN     || \
                  c==LARGER_THAN);
	}
};

#endif /* unparse_target_c_INCLUDED */
