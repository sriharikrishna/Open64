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

#ifndef unparse_target_ftn_INCLUDED
#define unparse_target_ftn_INCLUDED
/* ====================================================================
 *
 * Module: unparse_target.h
 * $Revision: 1.1 $
 * $Date: 2003-06-12 15:27:49 $
 * $Author: broom $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/whirl2f/unparse_target_ftn.h,v $
 *
 * Revision history:
 *  06-Jun-03 - Original Version
 *
 * Description:
 *
 *     Runtime tests for the C target language.
 */

#include "unparse_target.h"
#include "whirl2f_common.h"
#include "symtab.h"

static const char *
W2CF_Get_Ftn_St_Name(const ST *st, const char *original_name)
{ 
   const char *extern_name;

   char *name_ptr;

   if (Stab_External_Linkage(st) && 
       !Stab_Is_Based_At_Common_Or_Equivalence(st) &&
       !(ST_sym_class(st) == CLASS_VAR && ST_is_namelist(st)))
   {
      /* Here we deal with a curiosity of the Fortran naming scheme for
       * external names:
       *
       *    + If the name ends with a '_', the name was without the '_'
       *      in the original Fortran source.
       *
       *    + If the name ends without a '_', the name was with a '$'
       *      suffix in the original Fortran source.
       *
       *    + Unless the external name was a namelist variable, then even
       *      though there isn't a trailing '_', don't emit a '$'.
       */
      extern_name = name_ptr =
	 strcpy(Get_Name_Buf_Slot(strlen(original_name)+2), original_name);
      
      /* Get the last character */
      while (name_ptr[1] != '\0')
	 name_ptr++;
       
      /* Correct the name-suffix */
      if (extern_name[0] != '_'  && name_ptr[0] == '_')
      {
	 if (name_ptr[-1] == '_')
	    name_ptr[-1] = '\0';
	 else
	    name_ptr[0] = '\0';
      }
      else if (!WN2F_F90_pu)
      {
	 name_ptr[1] = '$';
	 name_ptr[2] = '\0';
      }
   }
   else /* Not an external variable */
      extern_name = original_name;

   return extern_name;
   
} /* W2CF_Get_Ftn_St_Name */

class Unparse_Target_FTN : public Unparse_Target {
public:
	~Unparse_Target_FTN () {};

	const char *Make_Valid_Name(const char *name, BOOL allow_dot)
	{ return WHIRL2F_make_valid_name(name, WN2F_F90_pu && allow_dot); }

	const char *Get_St_Name(const ST *st, const char *original_name)
	{ return W2CF_Get_Ftn_St_Name (st, original_name); }

	BOOL Avoid_Common_Suffix(void)
	{
	  BOOL avoid = TRUE;
	  return avoid;
	}

};

#endif /* unparse_target_ftn_INCLUDED */
