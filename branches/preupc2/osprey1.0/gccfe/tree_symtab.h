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


/* translate gnu decl trees to symtab references */

#ifndef tree_symtab_INCLUDED
#define tree_symtab_INCLUDED

#include <cmplrs/rcodes.h>
#include <map>
#include <string>
#include <upc_symtab_utils.h>

#define MAX_PATH 500

/* mark the "char" type, which should be distinct from MTYPE_I1 or MTYPE_U1 */
extern TY_IDX char_ty;

/* Calculate the block size of a type */
extern int Type_Tree_Block_Size(tree type_tree);

extern TY_IDX Create_TY_For_Tree (tree, TY_IDX idx = TY_IDX_ZERO);
extern "C" ST* Create_ST_For_Tree (tree);

typedef struct bs_pair {
  TY_IDX  t_idx;
  unsigned int thread_dim; //for arrays, it's the dimension that refers to THREADS
  bs_pair(TY_IDX t, unsigned int d) : t_idx(t), thread_dim(d) {}
}  bs_pair_t;
typedef bs_pair_t * bs_pair_p;

typedef struct tld_pair {
  string type;
  string init_exp;
  int size;
  tld_pair(string t, string i, int s) : type(t), init_exp(i), size(s) {}
} tld_pair_t;
typedef tld_pair_t* tld_pair_p;

extern std::map<ST_IDX, bs_pair_p> upc_st_orig_ty;
extern std::map<ST_IDX, tld_pair_p> upc_tld;

extern unsigned int hash_val(string s);

/* 
 * either return a previously created TY_IDX associated with a type,
 * or create a new one.
 */
inline TY_IDX
Get_TY (tree type_tree)
{
  TY_IDX result = 0; 
  
  if (TREE_CODE(type_tree) == ERROR_MARK)
    exit (RC_USER_ERROR);
  TY_IDX idx = TYPE_TY_IDX(type_tree);
  if (idx != 0) { 
    bool shared_conflict = compiling_upc && TYPE_SHARED(type_tree) && !TY_is_shared(idx);
    if (shared_conflict) {
      //create the equivalent shared type
      result = Make_Shared_Type (idx, Type_Tree_Block_Size(type_tree), 
				 TYPE_STRICT(type_tree) ? STRICT_CONSISTENCY :
				 TYPE_RELAXED(type_tree) ? RELAXED_CONSISTENCY : NO_CONSISTENCY);
    } else {
      if (TREE_CODE(type_tree) == RECORD_TYPE ||
	  TREE_CODE(type_tree) == UNION_TYPE) {
	FLD_HANDLE elt_fld = TY_fld(idx);
	if (elt_fld.Is_Null()) 
	  result = Create_TY_For_Tree (type_tree, idx); // forward declared
	else 
	  result = idx;
      } else {
	//can safely reuse the type entry
	result = idx;
      }
    } 
  } else { 
    result = Create_TY_For_Tree (type_tree, TY_IDX_ZERO);
    /* if(compiling_upc && shared_ptr_idx && TYPE_SHARED(type_tree) &&  */
/*        result != shared_ptr_idx) {  */
/*       TYPE_ORIG_TY_IDX(type_tree) = result; */
/*       TYPE_TY_IDX(type_tree) = shared_ptr_idx; */
/*       result = shared_ptr_idx; */
/*     } */
  }
  return result;
}

/*
 * either return a previously created ST associated with a
 * var-decl/parm-decl/function_decl, or create a new one.
 */
inline ST *
Get_ST (tree decl_tree)
{
	ST *st = DECL_ST(decl_tree);
        if (st != NULL) {
		if (TREE_CODE(decl_tree) == VAR_DECL &&
		    ST_sclass(st) == SCLASS_EXTERN   &&
		    !ST_is_weak_symbol(st)           &&
		    !DECL_EXTERNAL(decl_tree)        &&
		    !DECL_INITIAL(decl_tree))
			Set_ST_sclass (st, SCLASS_UGLOBAL);
        }
	else
		st = Create_ST_For_Tree (decl_tree);
	if ((CURRENT_SYMTAB > GLOBAL_SYMTAB + 1) &&
	    ((TREE_CODE(decl_tree) == VAR_DECL) ||
	     (TREE_CODE(decl_tree) == PARM_DECL)) &&
	    (ST_level(st) < CURRENT_SYMTAB) &&
	    (ST_level(st) > GLOBAL_SYMTAB)) {
		Set_ST_has_nested_ref (st);
		ST *base_st = st;
		while (base_st != ST_base (base_st)) {
			base_st = ST_base (base_st);
			Set_ST_has_nested_ref (base_st);
		}
	}
	return st;
}

#endif