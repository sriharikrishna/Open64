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

#ifndef unparse_target_INCLUDED
#define unparse_target_INCLUDED
/* ====================================================================
 *
 * Module: unparse_target.h
 * $Revision: 1.2 $
 * $Date: 2003-06-13 23:05:29 $
 * $Author: broom $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/whirl2c/unparse_target.h,v $
 *
 * Revision history:
 *  06-Jun-03 - Original Version
 *
 * Description:
 *
 *     Runtime tests for the target language are abstracted by this pure
 *     virtual class.
 *
 *     Specific language unparsers create a derived class that implements the
 *     tests for target language, and the main routine creates an instance
 *     of that derived class which is queried as required by the general
 *     purpose support routines.
 *
 *     This is a work in progress.  For now, at least, there is a global
 *     pointer to this object, to enable incremental conversion of the
 *     original implementation, which was based on conditional compilation
 *     depending on the target language.  The intent is to eliminate all
 *     conditional compilation based on "ifdef BUILD_WHIRL2x".
 *     Eventually, it might be better to have this object passed through the
 *     unparser as a parameter, or to make the unparser itself a derived class
 *     of this class.
 * ====================================================================
 */

#include "symtab.h"

class Unparse_Target {
public:
	virtual ~Unparse_Target () {};

	/* If name==NULL, then return NULL;  otherwise, if a valid name,
	 * then keep it unaltered; otherwise, construct a valid name
	 * in a new Name_Buf by removing invalid characters (never return
	 * NULL for this case)
	 */
	virtual const char *Make_Valid_Name(const char *name, BOOL allow_dot) = 0;

	virtual const char *Get_St_Name(const ST *st, const char *original_name) = 0;
	virtual const char *Intrinsic_Name(INTRINSIC intr_opc) = 0;

        /* Some globals eg: COMMONs, functions,  should get the same name */
        /* if they appear in the global symbol table more than once. They */
        /* may have been created by different PUs.                        */
	virtual BOOL Avoid_Common_Suffix(void) = 0;

        virtual BOOL Reduce_Const_Ptr_Exprs(void) = 0;

	virtual BOOL Enter_Symtab_Pointee_Names(void) = 0;

	virtual BOOL Is_Binary_Or_Tertiary_Op (char c) = 0;
};

extern Unparse_Target *W2X_Unparse_Target;

#endif /* unparse_target_INCLUDED */
