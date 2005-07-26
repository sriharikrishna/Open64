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

#include <values.h>
#include "defs.h"
#include "errors.h"
#include "gnu_config.h"
#include "gnu/flags.h"
extern "C" {
#include "gnu/system.h"
#include "gnu/tree.h"
#include "gnu/toplev.h"
#include "gnu/c-tree.h"
#include "gnu/upc-act.h"
}
#ifdef TARG_IA32
// the definition in gnu/config/i386/i386.h causes problem
// with the enumeration in common/com/ia32/config_targ.h
#undef TARGET_PENTIUM
#endif /* TARG_IA32 */

#include "symtab.h"
#include "strtab.h"
#include "tree_symtab.h"
#include "wn.h"
#include "wfe_expr.h"
#include "wfe_misc.h"
#include "wfe_dst.h"
#include "ir_reader.h"
#include <cmplrs/rcodes.h>

#include "glob.h"
#include "cxx_memory.h"

#include <math.h>

extern FILE *tree_dump_file; // For debugging only

extern INT pstatic_as_global;

extern string utoa(UINT64 i);

extern char* get_type(int mtype);

/* functions for the initialization of global data */
extern void add_shared_symbol(ST_IDX st, TY_IDX ty, int thread_dim);
extern void add_TLD_symbol(ST_IDX st, tld_pair_p info);

TY_IDX char_ty = 0; /* see tree_symtab.h */

/* UPC specific */

tree shared_ptr_tree;
extern int compiling_upc;

//std::map<ST*,bs_pair_p> upc_st_orig_ty;
std::map<ST_IDX,bs_pair_p> upc_st_orig_ty;

//vector storing all tld variables
//std::map<ST*, tld_pair_p> upc_tld;
std::map<ST_IDX, tld_pair_p> upc_tld;

/**
 *  Return the hash value of a string. 
 */
unsigned int hash_val(string s) {

  unsigned int length = s.size();
  unsigned int res = 0;
  for (int i = 0; i < length; i++) {
    res = s[i] + 31 * res;
  }
  return res;
}

static bool is_star(tree block) {
  return block != NULL && (TREE_INT_CST_LOW(block) ^ -1) == 0;
}


//For all that I can remember:
// return 1 if block = NULL, 
// otherwise return the value of the block tree 
int Type_Tree_Block_Size(tree type_tree) {

  tree block = 0;
  int bsize;
  switch (TREE_CODE (type_tree)) {
  case VOID_TYPE:
  case BOOLEAN_TYPE:
  case INTEGER_TYPE:
  case CHAR_TYPE:  
  case ENUMERAL_TYPE:
  case REAL_TYPE:
  case RECORD_TYPE:
  case UNION_TYPE:
  case COMPLEX_TYPE:
    block = TYPE_BLOCK_SIZE(type_tree); 
    if (block && TYPE_SIZE(type_tree)) {
      //return Get_Integer_Value(block) / Get_Integer_Value(TYPE_SIZE(type_tree));
      return Get_Integer_Value(block);
    }
    break;
  case ARRAY_TYPE:
    block = TYPE_BLOCK_SIZE(get_inner_array_type(type_tree));
    if (block) {
      int elt_size = Get_Integer_Value(TYPE_SIZE(get_inner_array_type(type_tree)));
      if (is_star(block)) {
	int ar_size = Get_Integer_Value(TYPE_SIZE(type_tree));
	if (threads_int == 0) {
	  //In dynamic env, block size is the same as the product of dimensions
	  bsize = ar_size / elt_size;
	} else {
	  //in static case, needs to use the formula
	  bsize = (ar_size / elt_size + threads_int - 1) / threads_int; 
	}
	if (bsize > max_bsize) {
	  error ("Maximum block size in this implementation is %d", max_bsize);
	}
      } else {
	//regular case 
	//return Get_Integer_Value(block) / elt_size;
	return Get_Integer_Value(block);
      }
    }
    break;
  case POINTER_TYPE:
  case REFERENCE_TYPE:
    block = TYPE_BLOCK_SIZE(type_tree);
    if (block) {
      int elt_size = Get_Integer_Value(TYPE_SIZE(type_tree));
      if (is_star(block)) {
	//can't have [*] in pointers
	error ("The [*] qualifier can only be used for arrays");
      }	
      //return Get_Integer_Value(block) / elt_size;
      return Get_Integer_Value(block);
    }
    break;
  default:
    break;
  }
  return 1;
}

static char*
Get_Name (tree node)
{
	static UINT anon_num = 0;
	static char buf[64];

	if (node == NULL) {
		++anon_num;
		sprintf(buf, ".anonymous.%d", anon_num);
		return buf;
	}
	else if (TREE_CODE (node) == IDENTIFIER_NODE)
		return IDENTIFIER_POINTER (node);
	else if (TREE_CODE (node) == TYPE_DECL)
		// If type has a typedef-name, the TYPE_NAME is a TYPE_DECL.
		return IDENTIFIER_POINTER (DECL_NAME (node));
	else
		FmtAssert(FALSE, ("Get_Name unexpected tree"));
		return NULL;
}

// idx is non-zero only for RECORD and UNION, when there is forward declaration

extern TY_IDX
Create_TY_For_Tree (tree type_tree, TY_IDX idx)
{
	if (TREE_CODE(type_tree) == ERROR_MARK)
		exit (RC_USER_ERROR);

	TY_IDX orig_idx = idx;
	if(TREE_CODE_CLASS(TREE_CODE(type_tree)) != 't') {
	    DevWarn("Bad tree class passed to Create_TY_For_Tree %c",
		TREE_CODE_CLASS(TREE_CODE(type_tree)));
	    return idx;
	}
	// for typedefs get the information from the base type
	if (TYPE_NAME(type_tree) &&
	    idx == 0 &&
	    (TREE_CODE(type_tree) == RECORD_TYPE ||
	     TREE_CODE(type_tree) == UNION_TYPE) &&
	    TREE_CODE(TYPE_NAME(type_tree)) == TYPE_DECL &&
	    TYPE_MAIN_VARIANT(type_tree) != type_tree) {
		idx = Get_TY (TYPE_MAIN_VARIANT(type_tree));
		if (TYPE_READONLY(type_tree))
			Set_TY_is_const (idx);
		if (TYPE_VOLATILE(type_tree))
			Set_TY_is_volatile (idx);
		// restrict qualifier not supported by gcc
	
		if (TYPE_SHARED(type_tree)) {
		  idx = Make_Shared_Type(idx, 
				   TYPE_BLOCK_SIZE(type_tree) ? 
				   Get_Integer_Value( TYPE_BLOCK_SIZE(type_tree)) : 1);
		 
		}
		TYPE_TY_IDX(type_tree) = idx;
		if(Debug_Level >= 2) {
		  struct mongoose_gcc_DST_IDX dst = 
		    Create_DST_type_For_Tree(type_tree,idx,orig_idx);
		  TYPE_DST_IDX(type_tree) = dst;
		}
		return idx;
	}

	TYPE_ID mtype;
	UINT64 tsize;
	BOOL variable_size = FALSE;
	tree type_size = TYPE_SIZE(type_tree);

	UINT align = TYPE_ALIGN(type_tree) / BITSPERBYTE;
	if (TREE_CODE(type_tree) == VOID_TYPE)
		tsize = 0;
	else
	if (type_size == NULL) {
		// incomplete structs have 0 size
		FmtAssert(TREE_CODE(type_tree) == ARRAY_TYPE 
			|| TREE_CODE(type_tree) == UNION_TYPE
			|| TREE_CODE(type_tree) == RECORD_TYPE,
			  ("Create_TY_For_Tree: type_size NULL for non ARRAY/RECORD"));
		tsize = 0;
	}
	else {
		if (TREE_CODE(type_size) != INTEGER_CST) {
			if (TREE_CODE(type_tree) == ARRAY_TYPE)
				DevWarn ("Encountered VLA at line %d", lineno);
			else
				Fail_FmtAssertion ("VLA at line %d not currently implemented", lineno);
			variable_size = TRUE;
			tsize = 0;
		}
		else
			tsize = Get_Integer_Value(type_size) / BITSPERBYTE;
	}
	switch (TREE_CODE(type_tree)) {
	case VOID_TYPE:
		idx = MTYPE_To_TY (MTYPE_V);	// use predefined type
		break;
	case BOOLEAN_TYPE:
	case INTEGER_TYPE:
	  switch (tsize) {
	  case 1:  mtype = MTYPE_I1; break;
	  case 2:  mtype = MTYPE_I2; break;
	  case 4:  mtype = MTYPE_I4; break;
	  case 8:  mtype = MTYPE_I8; break;
#ifdef _LP64
	  case 16:  mtype = MTYPE_I8; break;
#endif /* _LP64 */
	  default:  FmtAssert(FALSE, ("Get_TY unexpected size"));
	  }
	  
	  if (TREE_UNSIGNED(type_tree)) {
	    mtype = MTYPE_complement(mtype);
	  }
	  if (type_tree == char_type_node) {
	    if (char_ty == 0) {
	      TY& ty = New_TY (char_ty);
	      TY_Init (ty, tsize, KIND_SCALAR, MTYPE_I1, /* does it matter what mtype to use? */ 
		       Save_Str("char"));
	      if (compiling_upc) {
		/* borrow the ty_is_char flag since it's not being used */
		Set_TY_is_character(char_ty);
	      }
	    }
	    idx = char_ty;
	  } else {
	    idx = MTYPE_To_TY (mtype);	// use predefined type
	  }
	  Set_TY_align (idx, align);
	  break;
	case CHAR_TYPE:
		mtype = (TREE_UNSIGNED(type_tree) ? MTYPE_U1 : MTYPE_I1);
		idx = MTYPE_To_TY (mtype);	// use predefined type
		break;
	case ENUMERAL_TYPE:
		mtype = (TREE_UNSIGNED(type_tree) ? MTYPE_U4 : MTYPE_I4);
		idx = MTYPE_To_TY (mtype);	// use predefined type
		break;
	case REAL_TYPE:
		switch (tsize) {
		case 4:  mtype = MTYPE_F4; break;
		case 8:  mtype = MTYPE_F8; break;
#ifdef TARG_MIPS
		case 16: mtype = MTYPE_FQ; break;
#endif /* TARG_MIPS */
#ifdef TARG_IA64
		case 12: mtype = MTYPE_F10; break;
#endif /* TARG_IA64 */
#ifdef TARG_IA32
		case 12: mtype = MTYPE_F10; break;
#endif /* TARG_IA32 */
		default: FmtAssert(FALSE, ("Get_TY unexpected REAL_TYPE size %d", tsize));
		}
		idx = MTYPE_To_TY (mtype);	// use predefined type
		break;
	case COMPLEX_TYPE:
		switch (tsize) {
		case  8: mtype = MTYPE_C4; break;
		case 16: mtype = MTYPE_C8; break;
#ifdef TARG_MIPS
		case 32: mtype = MTYPE_CQ; break;
#endif /* TARG_MIPS */
#ifdef TARG_IA64
		case 24: mtype = MTYPE_C10; break;
#endif /* TARG_IA64 */
#ifdef TARG_IA32
		case 24: mtype = MTYPE_C10; break;
#endif /* TARG_IA32 */
		default:  FmtAssert(FALSE, ("Get_TY unexpected size"));
		}
		idx = MTYPE_To_TY (mtype);	// use predefined type
		break;
	case REFERENCE_TYPE:
	case POINTER_TYPE:
	  idx = Get_TY (TREE_TYPE(type_tree));
	  if (TYPE_READONLY(TREE_TYPE(type_tree))) {
	    Set_TY_is_const (idx);
	  }
	  if (TYPE_VOLATILE(TREE_TYPE(type_tree))) {
	    Set_TY_is_volatile (idx);
	  }
	  idx = Make_Pointer_Type (idx);
	  Set_TY_align (idx, align);
	  break;
	case ARRAY_TYPE:
		{	// new scope for local vars
		TY &ty = New_TY (idx);
		TY_Init (ty, tsize, KIND_ARRAY, MTYPE_M, 
			 Save_Str(Get_Name(TYPE_NAME(type_tree))) );
		//If block size is *, we need to fix the element type's block size first
		if (is_star(TYPE_BLOCK_SIZE(get_inner_array_type(type_tree)))) {
		  TYPE_BLOCK_SIZE(get_inner_array_type(type_tree)) = 
		    build_int_2(Type_Tree_Block_Size(type_tree), 0);
		}
		TY_IDX ety = Get_TY (TREE_TYPE(type_tree));
		if (TYPE_READONLY(TREE_TYPE(type_tree))) {
		  Set_TY_is_const (ety);
		}
		if (TYPE_VOLATILE(TREE_TYPE(type_tree))) {
		  Set_TY_is_volatile (ety);
		}		
		Set_TY_etype (ty, ety);
		
		Set_TY_align (idx, TY_align(TY_etype(ty)));
		// assumes 1 dimension
		// nested arrays are treated as arrays of arrays
		ARB_HANDLE arb = New_ARB ();
		ARB_Init (arb, 0, 0, 0);
		Set_TY_arb (ty, arb);
		Set_ARB_first_dimen (arb);
		Set_ARB_last_dimen (arb);
		Set_ARB_dimension (arb, 1);
		if (TREE_CODE(TYPE_SIZE(TREE_TYPE(type_tree))) == INTEGER_CST) {
			Set_ARB_const_stride (arb);
			Set_ARB_stride_val (arb, 
				Get_Integer_Value (TYPE_SIZE(TREE_TYPE(type_tree))) 
				/ BITSPERBYTE);
		}
		else {
			WN *swn;
			swn = WFE_Expand_Expr (TYPE_SIZE(TREE_TYPE(type_tree)));
			if (WN_opcode (swn) == OPC_U4I4CVT ||
			    WN_opcode (swn) == OPC_U8I8CVT) {
				swn = WN_kid0 (swn);
			}
			FmtAssert (WN_operator (swn) == OPR_LDID,
				("stride operator for VLA not LDID"));
			ST *st = WN_st (swn);
			TY_IDX ty_idx = ST_type (st);
			WN *wn = WN_CreateXpragma (WN_PRAGMA_COPYIN_BOUND,
						   (ST_IDX) NULL, 1);
			WN_kid0 (wn) = WN_Ldid (TY_mtype (ty_idx), 0, st, ty_idx);
			WFE_Stmt_Append (wn, Get_Srcpos());
			Clear_ARB_const_stride (arb);
			Set_ARB_stride_var (arb, (ST_IDX) ST_st_idx (st));
		}
		Set_ARB_const_lbnd (arb);
		Set_ARB_lbnd_val (arb, 0);
		if (type_size) {
		    if (TREE_CODE(TYPE_MAX_VALUE (TYPE_DOMAIN (type_tree))) ==
			INTEGER_CST) {
			Set_ARB_const_ubnd (arb);
			Set_ARB_ubnd_val (arb, Get_Integer_Value (
				TYPE_MAX_VALUE (TYPE_DOMAIN (type_tree)) ));
		    }
		    else {
			WN *uwn = WFE_Expand_Expr (TYPE_MAX_VALUE (TYPE_DOMAIN (type_tree)) );
			if (WN_opcode (uwn) == OPC_U4I4CVT ||
			    WN_opcode (uwn) == OPC_U8I8CVT) {
				uwn = WN_kid0 (uwn);
			}
			FmtAssert (WN_operator (uwn) == OPR_LDID,
				("bounds operator for VLA not LDID"));
			ST *st = WN_st (uwn);
			TY_IDX ty_idx = ST_type (st);
			WN *wn = WN_CreateXpragma (WN_PRAGMA_COPYIN_BOUND,
						   (ST_IDX) NULL, 1);
			WN_kid0 (wn) = WN_Ldid (TY_mtype (ty_idx), 0, st, ty_idx);
			WFE_Stmt_Append (wn, Get_Srcpos());
			Clear_ARB_const_ubnd (arb);
			Set_ARB_ubnd_var (arb, ST_st_idx (st));
		    }
		}
		else {
			Clear_ARB_const_ubnd (arb);
			Set_ARB_ubnd_val (arb, 0);
		}

		if (variable_size) {
			WN *swn, *wn;
			swn = WFE_Expand_Expr (type_size);
			if (TY_size(TY_etype(ty))) {
				if (WN_opcode (swn) == OPC_U4I4CVT ||
				    WN_opcode (swn) == OPC_U8I8CVT) {
					swn = WN_kid0 (swn);
				}
				FmtAssert (WN_operator (swn) == OPR_LDID,
					("size operator for VLA not LDID"));
				ST *st = WN_st (swn);
				TY_IDX ty_idx = ST_type (st);
				TYPE_ID mtype = TY_mtype (ty_idx);
				swn = WN_Div (mtype, swn, WN_Intconst (mtype, BITSPERBYTE));
				wn = WN_Stid (mtype, 0, st, ty_idx, swn);
				WFE_Stmt_Append (wn, Get_Srcpos());
			}
		}
	} // end array scope
	break;
	case RECORD_TYPE:
	case UNION_TYPE: {	// new scope for local vars
		TY &ty = (idx == TY_IDX_ZERO) ? New_TY(idx) : Ty_Table[idx];
		TY_Init (ty, tsize, KIND_STRUCT, MTYPE_M, 
			Save_Str(Get_Name(TYPE_NAME(type_tree))) );
		
		if (TREE_CODE(type_tree) == UNION_TYPE) {
			Set_TY_is_union(idx);
		}
		if (align == 0) align = 1;	// in case incomplete type
		Set_TY_align (idx, align);
		// set idx now in case recurse thru fields
		TYPE_TY_IDX(type_tree) = idx;	

		// to handle nested structs and avoid entering flds
		// into wrong struct, make two passes over the fields.
		// first create the list of flds for the current struct,
		// but don't follow the nested types.  Then go back thru
		// the fields and set the fld_type, recursing down into
		// nested structs.
  		Set_TY_fld (ty, FLD_HANDLE());
		FLD_IDX first_field_idx = Fld_Table.Size ();
		tree field;
		FLD_HANDLE fld;
		for (field = TREE_PURPOSE(type_tree); 
			field;
			field = TREE_CHAIN(field) )
		{
			if (TREE_CODE(field) == TYPE_DECL) {
				DevWarn ("got TYPE_DECL in field list");
				continue;
			}
			if (TREE_CODE(field) == CONST_DECL) {
				DevWarn ("got CONST_DECL in field list");
				continue;
			}
			fld = New_FLD ();
			FLD_Init (fld, Save_Str(Get_Name(DECL_NAME(field))), 
				0, // type
				Get_Integer_Value(DECL_FIELD_OFFSET(field)) +
				Get_Integer_Value(DECL_FIELD_BIT_OFFSET(field))
					/ BITSPERBYTE );
#ifdef OLDCODE
			if ( ! DECL_BIT_FIELD(field)
				&& Get_Integer_Value(DECL_SIZE(field)) > 0
				&& Get_Integer_Value(DECL_SIZE(field))
				  != (TY_size(Get_TY(TREE_TYPE(field))) 
					* BITSPERBYTE) )
			{
				// for some reason gnu doesn't set bit field
				// when have bit-field of standard size
				// (e.g. int f: 16;).  But we need it set
				// so we know how to pack it, because 
				// otherwise the field type is wrong.
				DevWarn("field size %d doesn't match type size %d", 
					Get_Integer_Value(DECL_SIZE(field)),
					TY_size(Get_TY(TREE_TYPE(field)))
						* BITSPERBYTE );
				DECL_BIT_FIELD(field) = 1;
			}
			if (DECL_BIT_FIELD(field)) {
				Set_FLD_is_bit_field (fld);
				// bofst is remaining bits from byte offset
				Set_FLD_bofst (fld, 
//					Get_Integer_Value(DECL_FIELD_OFFSET(field)) +
					Get_Integer_Value(
						DECL_FIELD_BIT_OFFSET(field))
					% BITSPERBYTE );
				Set_FLD_bsize (fld, Get_Integer_Value(DECL_SIZE(field)));
			}
#endif /* OLDCODE */
		}
  		FLD_IDX last_field_idx = Fld_Table.Size () - 1;
		if (last_field_idx >= first_field_idx) {
			Set_TY_fld (ty, FLD_HANDLE (first_field_idx));
			Set_FLD_last_field (FLD_HANDLE (last_field_idx));
		}
		// now set the fld types.
		fld = TY_fld(ty);
		for (field = TREE_PURPOSE(type_tree);
			field;
			field = TREE_CHAIN(field))
		{
			if (TREE_CODE(field) == TYPE_DECL)
				continue;
			if (TREE_CODE(field) == CONST_DECL)
				continue;
			if ( ! DECL_BIT_FIELD(field)
				&& Get_Integer_Value(DECL_SIZE(field)) > 0
				&& Get_Integer_Value(DECL_SIZE(field))
				  != (TY_size(Get_TY(TREE_TYPE(field))) 
					* BITSPERBYTE) )
			{
				// for some reason gnu doesn't set bit field
				// when have bit-field of standard size
				// (e.g. int f: 16;).  But we need it set
				// so we know how to pack it, because 
				// otherwise the field type is wrong.
				DevWarn("field size %d doesn't match type size %d", 
					Get_Integer_Value(DECL_SIZE(field)),
					TY_size(Get_TY(TREE_TYPE(field)))
						* BITSPERBYTE );
				DECL_BIT_FIELD(field) = 1;
			}
			if (DECL_BIT_FIELD(field)) {
				Set_FLD_is_bit_field (fld);
				// bofst is remaining bits from byte offset
				Set_FLD_bofst (fld, 
//					Get_Integer_Value(DECL_FIELD_OFFSET(field)) +
					Get_Integer_Value(
						DECL_FIELD_BIT_OFFSET(field))
					% BITSPERBYTE );
				Set_FLD_bsize (fld, Get_Integer_Value(DECL_SIZE(field)));
			}
			TY_IDX fty_idx = Get_TY(TREE_TYPE(field));
			if ((TY_align (fty_idx) > align) || (TY_is_packed (fty_idx)))
				Set_TY_is_packed (ty);
			Set_FLD_type(fld, fty_idx);
			fld = FLD_next(fld);
		}
		/* UPC specific */
		if (compiling_upc) {
		  if(strcmp(TY_name(ty), shared_ptr_name) == 0) {
		    shared_ptr_idx = idx;
		    shared_ptr_tree = type_tree;
		  } 
		}
		} // end record scope
		break;
	case METHOD_TYPE:
		DevWarn ("Encountered METHOD_TYPE at line %d", lineno);
	case FUNCTION_TYPE:
		{	// new scope for local vars
		tree arg;
		INT32 num_args;
		TY &ty = New_TY (idx);
		TY_Init (ty, 0, KIND_FUNCTION, MTYPE_UNKNOWN, 0); 
		Set_TY_align (idx, 1);
		TY_IDX ret_ty_idx;
		TY_IDX arg_ty_idx;
		TYLIST tylist_idx;

		// allocate TYs for return as well as parameters
		// this is needed to avoid mixing TYLISTs if one
		// of the parameters is a pointer to a function

		ret_ty_idx = Get_TY(TREE_TYPE(type_tree));
		for (arg = TYPE_ARG_TYPES(type_tree);
		     arg;
		     arg = TREE_CHAIN(arg))
			arg_ty_idx = Get_TY(TREE_VALUE(arg));

		// if return type is pointer to a zero length struct
		// convert it to void
		if (!WFE_Keep_Zero_Length_Structs    &&
		    TY_mtype (ret_ty_idx) == MTYPE_M &&
		    TY_size (ret_ty_idx) == 0) {
			// zero length struct being returned
		  	DevWarn ("function returning zero length struct at line %d", lineno);
			ret_ty_idx = Be_Type_Tbl (MTYPE_V);
		}

		Set_TYLIST_type (New_TYLIST (tylist_idx), ret_ty_idx);
		Set_TY_tylist (ty, tylist_idx);
		for (num_args = 0, arg = TYPE_ARG_TYPES(type_tree);
		     arg;
		     num_args++, arg = TREE_CHAIN(arg))
		{
			arg_ty_idx = Get_TY(TREE_VALUE(arg));
			if (!WFE_Keep_Zero_Length_Structs    &&
			    TY_mtype (arg_ty_idx) == MTYPE_M &&
			    TY_size (arg_ty_idx) == 0) {
				// zero length struct passed as parameter
				DevWarn ("zero length struct encountered in function prototype at line %d", lineno);
			}
			else
				Set_TYLIST_type (New_TYLIST (tylist_idx), arg_ty_idx);
		}
		if (num_args)
		{
			Set_TY_has_prototype(idx);
			if (arg_ty_idx != Be_Type_Tbl(MTYPE_V))
			{
				Set_TYLIST_type (New_TYLIST (tylist_idx), 0);
				Set_TY_is_varargs(idx);
			}
			else
				Set_TYLIST_type (Tylist_Table [tylist_idx], 0);
		}
		else
			Set_TYLIST_type (New_TYLIST (tylist_idx), 0);
		} // end FUNCTION_TYPE scope
		break;
	default:
		FmtAssert(FALSE, ("Get_TY unexpected tree_type"));
	}

	if (TYPE_SHARED(type_tree)) {
	  idx = Make_Shared_Type(idx, Type_Tree_Block_Size(type_tree), 
	    TYPE_STRICT(type_tree) ? STRICT_CONSISTENCY :
	    TYPE_RELAXED(type_tree) ? RELAXED_CONSISTENCY : NO_CONSISTENCY);    
	}

	// restrict qualifier not supported by gcc
	//UPC
	if (TYPE_SHARED(type_tree) /*&& TREE_CODE(type_tree) != ARRAY_TYPE*/)
		Set_TY_is_shared (idx);
	TYPE_TY_IDX(type_tree) = idx;
	
	if(Debug_Level >= 2) {
	  struct mongoose_gcc_DST_IDX dst = 
	    Create_DST_type_For_Tree(type_tree,idx,orig_idx);
	  TYPE_DST_IDX(type_tree) = dst;
	}

	return idx;
}

static string get_qualifier(TY_IDX ty) {

  string type_str = "";
  if (TY_is_const(ty)) {
    type_str += "const ";
  }
  if (TY_is_volatile(ty)) {
    type_str += "volatile ";
  }
  return type_str;
}

static string getTypeStr(TY_IDX idx) {

  string type_str;

  switch (TY_kind(idx)) {
  case KIND_VOID:
    type_str = "void";
    break;
  case KIND_SCALAR: {
    if (TY_is_character(idx)) {
      //FIXME: this comparison would not work if the type has qualifiers (const, etc.)  
      //how do we get around this?
      type_str = "char";
    } else {
      type_str = get_type(TY_mtype(idx));
    }
    type_str = get_qualifier(idx) + type_str;
    break;
  }
  case KIND_POINTER: {
    TY_IDX pointed = TY_pointed(idx);
    if (TY_is_shared(pointed)) {
      if (TY_is_pshared(pointed)) {
	  type_str = "upcr_pshared_ptr_t";
      } else {
	  type_str = "upcr_shared_ptr_t";
      }
    } else if (TY_kind(pointed) == KIND_FUNCTION) {
      //function pointers requires some special care
      TYLIST_IDX params = TY_parms(pointed);
      type_str = getTypeStr(TY_ret_type(pointed)); //return type
      type_str += "(*)(";
      bool first = true;
      while (Tylist_Table[params] != 0) {
	type_str += getTypeStr(TYLIST_item(Tylist_Table[params]));
	params = TYLIST_next(params);
	if (Tylist_Table[params] != 0)
	  type_str += ",";
      }
      type_str += ")";
    } else {
      type_str = getTypeStr(pointed) + "* ";
      type_str += get_qualifier(idx);
    }
    break;
  }
  case KIND_ARRAY: 
    if (TY_size(idx) > 0) {
      UINT64 eltSize = TY_size(TY_etype(idx));
      UINT64 length = TY_size(idx) / eltSize;
      type_str = "[" + utoa(length) + "]";
    } else {
      type_str = "[]";
    }
    type_str += getTypeStr(TY_etype(idx));
    break;

  case KIND_STRUCT: {
#if 0 //fzhao
    type_str = (TY_is_union(idx) ? "union " : "struct ") + (string) TY_name(idx);
    //we need to remove the "." from the TY_name.  This should match with whatever WHIRL2C_make_valid_c_name
    //returns as long as anonymous structs have names like ".anonymous.i"
    for (int i = type_str.find("."); i >= 0; i = type_str.find(".")) { 
      type_str = type_str.replace(i, 1, "");
      }
#endif
    char * name = TY_name(idx);
    if (strncmp(name, "T ", 2) == 0) {
      /* We have an anonymous struct, use its typedef name */
      type_str = (string) (name + 2);
    } else {
      type_str = (TY_is_union(idx) ? "union " : "struct ") + (string) name;
      //we need to remove the "." from the TY_name.  This should match with whatever WHIRL2C_make_valid_c_name
      //returns as long as anonymous structs have names like ".anonymous.i"
      for (int i = type_str.find("."); i >= 0; i = type_str.find(".")) {
        // for anonymous struct (defined via typedefs), we always output their definition
        Set_TY_is_written(idx);
        type_str = type_str.replace(i, 1, "");
      }
    }
    break;
  }

  default:
    Is_True(false, ("IN TREE_SYMTAB, getTypeStr:  UNEXPECTED TYPE"));    
  }

  return type_str;
}

ST*
Create_ST_For_Tree (tree decl_node)
{
  TY_IDX     ty_idx;
  ST*        st;
  string name;
  ST_SCLASS  sclass;
  ST_EXPORT  eclass;
  SYMTAB_IDX level;

  if (TREE_CODE(decl_node) == ERROR_MARK)
    exit (RC_USER_ERROR);

  if (DECL_NAME (decl_node)) {
    tree enclosing_decl = DECL_CONTEXT(decl_node);
    name = IDENTIFIER_POINTER (DECL_NAME (decl_node));
    if (compiling_upc) {
      string suffix = "";
      bool pstatic = enclosing_decl != NULL && TREE_STATIC(decl_node) && TREE_CODE(enclosing_decl) == FUNCTION_DECL;
      bool fstatic = enclosing_decl == NULL && !TREE_PUBLIC(decl_node);
      //rename static local/global variables by adding the hash value of (fn name + file name) to the variable
      if (pstatic) {
	suffix += "_" + (string) IDENTIFIER_POINTER(DECL_NAME(enclosing_decl));
      }
      if (pstatic || fstatic) {
	char buf[MAX_PATH];
	realpath(Orig_Src_File_Name, buf);
	suffix += (string) buf;
	name += "_" + utoa(hash_val(suffix)) + "_";
      }
    }
  } else {
    DevWarn ("no name for DECL_NODE");
    name = "__unknown__";
  }

  switch (TREE_CODE(decl_node)) {
    
  case FUNCTION_DECL:
    {
      TY_IDX func_ty_idx = Get_TY(TREE_TYPE(decl_node));
      
      if (DECL_WIDEN_RETVAL (decl_node)) {
	/*
          extern tree long_long_integer_type_node;
          extern tree long_long_unsigned_type_node;
	*/
	tree type_tree = TREE_TYPE(decl_node);
	tree ret_type_tree = TREE_TYPE (type_tree);
	TY_IDX ret_ty_idx = Get_TY(ret_type_tree);
	if (MTYPE_signed (TY_mtype (ret_ty_idx)))
	  TREE_TYPE (type_tree) = long_long_integer_type_node;
	else
	  TREE_TYPE (type_tree) = long_long_unsigned_type_node;
	TY_IDX old_func_ty_idx = func_ty_idx;
	func_ty_idx = Create_TY_For_Tree (type_tree, TY_IDX_ZERO);
	TREE_TYPE (type_tree) = ret_type_tree;
	TYPE_TY_IDX(type_tree) = old_func_ty_idx;
      }
      
      sclass = SCLASS_EXTERN;
      eclass = TREE_PUBLIC(decl_node) ? EXPORT_PREEMPTIBLE : EXPORT_LOCAL;
      level  = GLOBAL_SYMTAB+1;
      
      PU_IDX pu_idx;
      PU&    pu = New_PU (pu_idx);
      
      PU_Init (pu, func_ty_idx, level);
      
      st = New_ST (GLOBAL_SYMTAB);
      char *an;
      
      if (compiling_upc) {
	Save_Upc_Rts_Calls((char *) name.c_str(), st);
	an = IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl_node));
	Upc_Translate_Name((char *) name.c_str(), &an);
        if (strcmp(an, "main") == 0) {
	  an = "user_main";
        }
      }  else {
	an = IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl_node));
      }
      
      ST_Init (st,
	       Save_Str (an),
	       CLASS_FUNC, sclass, eclass, TY_IDX (pu_idx));

    }
    break;
    
  case PARM_DECL:
  case VAR_DECL:
    {
      if (TREE_CODE(decl_node) == PARM_DECL) {
	sclass = SCLASS_FORMAL;
	eclass = EXPORT_LOCAL;
	level = CURRENT_SYMTAB;
      }
      else {
	if (DECL_CONTEXT (decl_node) == 0) {
	  if (TREE_PUBLIC (decl_node)) {
	    if (DECL_INITIAL(decl_node))
	      sclass = SCLASS_DGLOBAL;
	    else if (TREE_STATIC(decl_node)) {
	      if (flag_no_common || DECL_SECTION_NAME(decl_node))
		sclass = SCLASS_UGLOBAL;
	      else
		sclass = SCLASS_COMMON;
	    }
	    else
	      sclass = SCLASS_EXTERN;
	    eclass = EXPORT_PREEMPTIBLE;
	  }
	  else {
	    sclass = SCLASS_FSTATIC;
	    eclass = EXPORT_LOCAL;
	  }
	  level = GLOBAL_SYMTAB;
	}
	else {
	  if (DECL_EXTERNAL(decl_node)) {
	    sclass = SCLASS_EXTERN;
	    level  = GLOBAL_SYMTAB;
	    eclass = EXPORT_PREEMPTIBLE;
	  }
	  else {
	    if (TREE_STATIC (decl_node)) {
	      sclass = SCLASS_PSTATIC;
	      if (pstatic_as_global)
		level = GLOBAL_SYMTAB;
	      else
		level = CURRENT_SYMTAB;
	    }
	    else {
	      sclass = SCLASS_AUTO;
	      level = decl_node->decl.symtab_idx ?
		decl_node->decl.symtab_idx : CURRENT_SYMTAB;
              }
	    eclass = EXPORT_LOCAL;
	  }
	}
      }
      st = New_ST (level);
      ty_idx = Get_TY (TREE_TYPE(decl_node));
      
      // UPC

      if (TY_kind(ty_idx) == KIND_SCALAR && TREE_THIS_SHARED(decl_node)) {
	//WEI: Don't do this, since blocksize is part of the type
#if 0  
	ty_idx = MTYPE_To_Shared_TY (TY_mtype(ty_idx));
#endif
	TYPE_TY_IDX(TREE_TYPE(decl_node)) = ty_idx;
      }


      if (TY_kind (ty_idx) == KIND_ARRAY &&
            TREE_STATIC (decl_node) &&
            DECL_INITIAL (decl_node) == FALSE &&
            TY_size (ty_idx) == 0) {
	Set_TY_size (ty_idx, TY_size (Get_TY (TREE_TYPE (TREE_TYPE (decl_node)))));
      }
      if (TY_mtype (ty_idx) == MTYPE_M &&
	  Aggregate_Alignment > 0 &&
	  Aggregate_Alignment > TY_align (ty_idx))
	Set_TY_align (ty_idx, Aggregate_Alignment);
      // qualifiers are set on decl nodes
      if (TREE_READONLY(decl_node)) 
	Set_TY_is_const (ty_idx);
      if (TREE_THIS_VOLATILE(decl_node))
	Set_TY_is_volatile (ty_idx);

      ST_Init (st, Save_Str(name.c_str()), CLASS_VAR, sclass, eclass, ty_idx);
      if (TREE_CODE(decl_node) == PARM_DECL) {
	Set_ST_is_value_parm(st);
      }

      /* UPC specific */
      if(compiling_upc) {
	if (TREE_CODE(decl_node) == VAR_DECL) {
	  bool shared = TY_is_shared(ty_idx);
	  unsigned int thread_dim = 0;
	  tree decl_type = TREE_TYPE(decl_node);	  
	  if (TY_kind(ty_idx) == KIND_POINTER && TY_is_shared(TY_pointed(ty_idx))) {
	    //a local pointer to shared data
	    if (is_star(TYPE_BLOCK_SIZE(TREE_TYPE(decl_type)))) {
	      error("[*] qualifier may not be used in declaration of pointers");
	    }
	  }
	  
	  if (TY_is_shared(ty_idx)) {
	    int bsize = Get_Type_Block_Size(ty_idx);
	    if (TREE_CODE(decl_type) == ARRAY_TYPE) {
	      //find the dimension that contains THREADS, and perform the appropriate checks
	      int i = 1;
	      for (tree base_type = decl_type; TREE_CODE(base_type) == ARRAY_TYPE; base_type = TREE_TYPE(base_type), i++) {
		if (UPC_TYPE_HAS_THREADS(base_type)) {
		  thread_dim = i;
		  break;
		}
	      }
	      if (bsize == 0 && thread_dim > 0) {
		//we have an error here, since arrays with indef block size can't have threads in its dimension
		error("In the dynamic translation environment, THREADS may not appear in the declarations of shared arrays with indefinite block size. Offending variable: %s", name.c_str());
	      }
	      if (bsize != 0 && threads_int == 0 && thread_dim == 0) {
		error("In the dynamic translation environment, THREADS must appear exactly once in declaratons of shared arrays with definite block size.  Offending variable: %s", name.c_str());
	      }
	    } else if (is_star(TYPE_BLOCK_SIZE(decl_type))) {
	      error("[*] qualifier is only allowed for arrays");
	    }
	    //cout << "SHARED: " << ST_name(st) << " " << ty_idx << " " << ST_st_idx(st) << endl;
	    upc_st_orig_ty[ST_st_idx(st)] = CXX_NEW(bs_pair_t(ty_idx, thread_dim), &MEM_src_pool);
	      
	  } else if ((DECL_CONTEXT(decl_node) == 0 || TREE_STATIC(decl_node))) {
	    if (name != "MYTHREAD" && name != "THREADS" &&
		name != "UPCR_SHARED_SIZE" && name != "UPCR_PSHARED_SIZE") { 	      
	      string type_str = getTypeStr(ty_idx);
	      upc_tld[ST_st_idx(st)] = CXX_NEW(tld_pair_t(type_str, "NONE", TY_size(ty_idx)), &MEM_src_pool);
	      //cout << "TLD: " << ST_name(st) << " " << ty_idx << " " << ST_st_idx(st) << endl;
	    }
	  }
	}
	
	if (strcmp(name.c_str(), "MYTHREAD") == 0) 
	  upc_mythread_st = st;
      }
    }
    break;
    
  default:
    {
      Fail_FmtAssertion ("Create_ST_For_Tree: unexpected tree type");
    }
      break;
  }

  DECL_ST(decl_node) = st;

  if ((DECL_WEAK (decl_node)) && (TREE_CODE (decl_node) != PARM_DECL)) {
    Set_ST_is_weak_symbol (st);
/*
    if (TREE_CODE (decl_node) == FUNCTION_DECL)
      Set_ST_sclass (st, SCLASS_TEXT);
*/
  }

  if (DECL_SECTION_NAME (decl_node)) {
    if (TREE_CODE (decl_node) == FUNCTION_DECL)
      level = GLOBAL_SYMTAB;
    ST_ATTR_IDX st_attr_idx;
    ST_ATTR&    st_attr = New_ST_ATTR (level, st_attr_idx);
    ST_ATTR_Init (st_attr, ST_st_idx (st), ST_ATTR_SECTION_NAME,
                  Save_Str (TREE_STRING_POINTER (DECL_SECTION_NAME (decl_node))));
    Set_ST_has_named_section (st);
  }

  if (DECL_SYSCALL_LINKAGE (decl_node)) {
	Set_PU_has_syscall_linkage (Pu_Table [ST_pu(st)]);
  }
  if(Debug_Level >= 2) {
     struct mongoose_gcc_DST_IDX dst =
       Create_DST_decl_For_Tree(decl_node,st);
     DECL_DST_IDX(decl_node) = dst;
  }
  return st;
}
