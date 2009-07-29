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


/* ====================================================================
 * ====================================================================
 *
 *
 * Revision history:
 *  12-Apr-95 - Original Version
 *
 * Description:
 *
 *   Translate a WN load/store subtree to Fortran by means of an inorder 
 *   recursive descent traversal of the WHIRL IR.  Note that the routines
 *   handle statements and expressions are in separate source files.
 *   Recursive translation of WN nodes should only use WN2F_Translate()!
 *
 * ====================================================================
 * ====================================================================
 */

#ifdef _KEEP_RCS_ID
/*REFERENCED*/
#endif

#include <climits>

#include "whirl2f_common.h"
#include "PUinfo.h"          /* In be/whirl2c directory */
#include "pf_cg.h"
#include "wn2f.h"
#include "st2f.h"
#include "ty2f.h"
#include "tcon2f.h"
#include "wn2f_load_store.h"
#include "ty_ftn.h"

#define DEB_Whirl2f_IR_TY_WN2F_Arrsection_Slots 0
#define DEB_Whirl2f_IR_TY_WN2F_Arrsection_Slots_st1 0

extern BOOL W2F_Only_Mark_Loads; /* Defined in w2f_driver.c */
static void WN2F_Block(TOKEN_BUFFER tokens, ST * st, STAB_OFFSET off,WN2F_CONTEXT context) ;

static WN *WN2F_ZeroInt_Ptr = NULL;
static WN *WN2F_OneInt_Ptr = NULL;
TY_IDX fld_type_z = 0;

#define WN2F_INTCONST_ZERO\
   (WN2F_ZeroInt_Ptr == NULL? WN2F_ZeroInt_Ptr = WN2F_Initiate_ZeroInt() \
                            : WN2F_ZeroInt_Ptr)
#define WN2F_INTCONST_ONE\
   (WN2F_OneInt_Ptr == NULL? WN2F_OneInt_Ptr = WN2F_Initiate_OneInt() \
                            : WN2F_OneInt_Ptr)

void WN2F_Arrsection_Slots(TOKEN_BUFFER tokens, WN *wn,TY_IDX array_ty,WN2F_CONTEXT context,
                            BOOL parens);
void WN2F_Array_Slots(TOKEN_BUFFER tokens, WN *wn,TY_IDX array_ty,WN2F_CONTEXT context,BOOL parens);

/*------------------------- Utility Functions ------------------------*/
/*--------------------------------------------------------------------*/

static ST *
WN2F_Get_Named_Param(const WN *pu, const char *param_name)
{
   /* Find a parameter with a matching name, if possible, otherwise
    * return NULL.
    */
   ST *param_st = NULL;
   INT param, num_formals;

   if (WN_opcode(pu) == OPC_ALTENTRY)
      num_formals = WN_kid_count(pu);
   else
      num_formals = WN_num_formals(pu);

   /* Search through the parameter ST entries
    */
   for (param = 0; param_st == NULL && param < num_formals; param++)
   {
      if (ST_name(WN_st(WN_formal(pu, param))) != NULL &&
	  strcmp(ST_name(WN_st(WN_formal(pu, param))), param_name) == 0)
	 param_st = WN_st(WN_formal(pu, param));
   }
   return param_st;
} /* WN2F_Get_Named_Param */

static void
WN2F_Translate_StringLEN(TOKEN_BUFFER tokens, ST *param_st)
{
   INT dim;
   TY_IDX param_ty = (TY_Is_Pointer(ST_type(param_st))? 
                   TY_pointed(ST_type(param_st)) : ST_type(param_st));

   Append_Token_String(tokens, "LEN");
   Append_Token_Special(tokens, '(');
   Append_Token_String(tokens, W2CF_Symtab_Nameof_St(param_st));

   if (TY_Is_Array(param_ty) && !TY_Is_Character_String(param_ty))
   {
      /* Append index values (any arbitrary value will do for each dimension)
       */
      Append_Token_Special(tokens, '(');


      ARB_HANDLE arb_base = TY_arb(param_ty);
      dim = ARB_dimension(arb_base) - 1; 

      while ( dim >= 0)
      {
	ARB_HANDLE arb = arb_base[dim];

         Append_Token_String(tokens, "1");
	 if (dim-- > 0)
            Append_Token_Special(tokens, ',');
      }
      Append_Token_Special(tokens, ')');
   }
   else
   {
      ASSERT_WARN(TY_Is_Character_String(param_ty), 
                  (DIAG_W2F_EXPECTED_PTR_TO_CHARACTER,
		    "WN2F_Translate_StringLEN"));
   }
   Append_Token_Special(tokens, ')');
} /* WN2F_Translate_StringLEN */

static WN *
WN2F_Initiate_ZeroInt(void)
{
   static char ZeroInt [sizeof (WN)];
   WN       *wn = (WN*) &ZeroInt;
   OPCODE    opcode = OPCODE_make_op(OPR_INTCONST, MTYPE_I4, MTYPE_V);

   memset(wn, '\0', sizeof(WN));
   WN_set_opcode(wn, opcode);
   WN_set_kid_count(wn, 0);
   WN_map_id(wn) =  WN_MAP_UNDEFINED;
   WN_const_val(wn) = 0LL;
   return wn;
} /* WN2F_Initiate_ZeroInt */

static WN *
WN2F_Initiate_OneInt(void)
{
   static char OneInt [sizeof (WN)];
   WN       *wn = (WN*) &OneInt;
   OPCODE    opcode = OPCODE_make_op(OPR_INTCONST, MTYPE_I4, MTYPE_V);

   memset(wn, '\0', sizeof(WN));
   WN_set_opcode(wn, opcode);
   WN_set_kid_count(wn, 0);
   WN_map_id(wn) =  WN_MAP_UNDEFINED;
   WN_const_val(wn) = 1LL;
   return wn;
} /* WN2F_Initiate_ZeroInt */


static BOOL
WN2F_Expr_Plus_Literal(TOKEN_BUFFER tokens,
		       WN          *wn,
		       INT64        literal,
		       WN2F_CONTEXT context)
{
   /* Returns TRUE if the resultant value is constant and different
    * from zero.
    */
   const BOOL parenthesize = !WN2F_CONTEXT_no_parenthesis(context);
   BOOL       is_const = TRUE;
   INT64      value;
   
   if (WN_opc_operator(wn) == OPR_INTCONST)
      value = WN_const_val(wn) + literal;
   else if (WN_opc_operator(wn) == OPR_CONST)
      value = Targ_To_Host(STC_val(WN_st(wn))) + literal;
   else
      is_const = FALSE;
   
   if (is_const)
   {
    if (WN_opc_operator(wn) == OPR_INTCONST) {
     switch (TCON_ty(Host_To_Targ(WN_opc_rtype(wn), value)))
      {
            case MTYPE_I1:
            case MTYPE_I2:
            case MTYPE_I4:
            case MTYPE_I8:
              if (TCON_ival(Host_To_Targ(WN_opc_rtype(wn), value))<0)  {
                  Append_Token_Special(tokens, '(');
                  TCON2F_translate(tokens,
                                   Host_To_Targ(WN_opc_rtype(wn), value),
                                   FALSE /*is_logical*/);
                   Append_Token_Special(tokens, ')');
                 }
               else
                  TCON2F_translate(tokens,
                                   Host_To_Targ(WN_opc_rtype(wn), value),
                                   FALSE/*is_logical*/);
               break;

              default:
                  TCON2F_translate(tokens,
                                   Host_To_Targ(WN_opc_rtype(wn), value),
                                   FALSE/*is_logical*/);

                   break;

             } /*switch*/

    } else {  //WN_opc_operator(wn) == OPR_CONST
     ; //Shouldn't be here
    
    }
 
   }
   else
   {
      if (parenthesize)
      {
	 reset_WN2F_CONTEXT_no_parenthesis(context);
	 Append_Token_Special(tokens, '(');
      }
   if (WN_opc_operator(wn) == OPR_IMPLICIT_BND)
        Append_Token_Special(tokens, '*');
   else
      WN2F_translate(tokens, wn, context);

      if (parenthesize)
	 Append_Token_Special(tokens, ')');
   }

   return is_const && (value != 0LL);
} /* WN2F_Expr_Plus_Literal */



static WN2F_STATUS
WN2F_OLD_Den_Arr_Idx(TOKEN_BUFFER tokens, 
			   WN          *idx_expr, 
			   WN2F_CONTEXT context)
{
   const BOOL   parenthesize = !WN2F_CONTEXT_no_parenthesis(context);
   TOKEN_BUFFER tmp_tokens;
   BOOL         non_zero, cexpr_is_lhs;
   WN          *nexpr, *cexpr;
   INT64        plus_value;
   
   /* Given an index expression, translate it to Fortran and append
    * the tokens to the given token-buffer.  If the value of the idx
    * expression is "v", then the appended tokens should represent
    * the value "v+1".  This denormalization moves the base of the
    * array from index zero to index one.
    */
   if (WN_opc_operator(idx_expr) == OPR_ADD && 
       (WN_is_constant_expr(WN_kid1(idx_expr)) || 
	WN_is_constant_expr(WN_kid0(idx_expr))))
   {
      /* Do the "e+c" ==> "e+(c+1)" translation, using the property
       * that addition is commutative.
       */
      if (WN_is_constant_expr(WN_kid1(idx_expr)))
      {
	 cexpr = WN_kid1(idx_expr);
	 nexpr = WN_kid0(idx_expr);
      }
      else /* if (WN_is_constant_expr(WN_kid0(idx_expr))) */
      {
	 cexpr = WN_kid0(idx_expr);
	 nexpr = WN_kid1(idx_expr);
      }
      tmp_tokens = New_Token_Buffer();
      non_zero = WN2F_Expr_Plus_Literal(tmp_tokens, cexpr, 1LL, context);
      if (non_zero)
      {
	 if (parenthesize)
	 {
	    reset_WN2F_CONTEXT_no_parenthesis(context);
	    Append_Token_Special(tokens, '(');
	 }
	 WN2F_translate(tokens, nexpr, context);
	 Append_Token_Special(tokens, '+');
	 Append_And_Reclaim_Token_List(tokens, &tmp_tokens);
	 if (parenthesize)
	    Append_Token_Special(tokens, ')');
      }
      else
      {
	 Reclaim_Token_Buffer(&tmp_tokens);
	 WN2F_translate(tokens, nexpr, context);
      }
   }
   else if (WN_opc_operator(idx_expr) == OPR_SUB && 
	    (WN_is_constant_expr(WN_kid1(idx_expr)) || 
	     WN_is_constant_expr(WN_kid0(idx_expr))))
   {
      /* Do the "e-c" ==> "e-(c-1)" or the  "c-e" ==> "(c+1)-e"
       * translation.
       */
      cexpr_is_lhs = WN_is_constant_expr(WN_kid0(idx_expr));
      if (!cexpr_is_lhs)
      {
	 cexpr = WN_kid1(idx_expr);
	 nexpr = WN_kid0(idx_expr);
	 plus_value = -1LL;
      }
      else
      {
	 cexpr = WN_kid0(idx_expr);
	 nexpr = WN_kid1(idx_expr);
	 plus_value = 1LL;
      }
	
      /* Do the "e-c" ==> "e-(c-1)" or the  "c-e" ==> "(c+1)-e"
       * translation.
       */
      tmp_tokens = New_Token_Buffer();
      non_zero = 
	 WN2F_Expr_Plus_Literal(tmp_tokens, cexpr, plus_value, context);
      if (non_zero)
      {
	 if (parenthesize)
	 {
	    reset_WN2F_CONTEXT_no_parenthesis(context);
	    Append_Token_Special(tokens, '(');
	 }
	 if (!cexpr_is_lhs)
	 {
	    WN2F_translate(tokens, nexpr, context);
	    Append_Token_Special(tokens, '-');
	    Append_And_Reclaim_Token_List(tokens, &tmp_tokens);
	 }
	 else
	 {
	    Append_And_Reclaim_Token_List(tokens, &tmp_tokens);
	    Append_Token_Special(tokens, '-');
	    WN2F_translate(tokens, nexpr, context);
	 }
	 if (parenthesize)
	    Append_Token_Special(tokens, ')');
      }
      else
      {
	 Reclaim_Token_Buffer(&tmp_tokens); 
	 if (cexpr_is_lhs)
	 {
	    if (parenthesize)
	    {
	       reset_WN2F_CONTEXT_no_parenthesis(context);
	       Append_Token_Special(tokens, '(');
	    }
	    Append_Token_Special(tokens, '-');
	    WN2F_translate(tokens, nexpr, context);
	    if (parenthesize)
	       Append_Token_Special(tokens, ')');
	 }
	 else
	 {
	    WN2F_translate(tokens, nexpr, context);
	 }
      }
   }
   else
   {
      WN2F_Expr_Plus_Literal(tokens, idx_expr, 1LL, context);
   }
   return EMPTY_WN2F_STATUS;
} /* WN2F_OLD_Den_Arr_Idx */


static WN2F_STATUS
WN2F_Denormalize_Array_Idx(TOKEN_BUFFER tokens, 
			   WN          *idx_expr, 
			   WN2F_CONTEXT context)
{
   const BOOL   parenthesize = !WN2F_CONTEXT_no_parenthesis(context);
   TOKEN_BUFFER tmp_tokens;
   BOOL         non_zero, cexpr_is_lhs;
   WN          *nexpr, *cexpr;
   INT64        plus_value;
   
   /* Given an index expression, translate it to Fortran and append
    * the tokens to the given token-buffer.  If the value of the idx
    * expression is "v", then the appended tokens should represent
    * the value "v+1".  This denormalization moves the base of the
    * array from index zero to index one.
    */
if (idx_expr==NULL) return EMPTY_WN2F_STATUS;

   if (WN_opc_operator(idx_expr) == OPR_ADD && 
       (WN_is_constant_expr(WN_kid1(idx_expr)) || 
	WN_is_constant_expr(WN_kid0(idx_expr))))
   {
      /* Do the "e+c" ==> "e+(c+1)" translation, using the property
       * that addition is commutative.
       */
      if (WN_is_constant_expr(WN_kid1(idx_expr)))
      {
	 cexpr = WN_kid1(idx_expr);
	 nexpr = WN_kid0(idx_expr);
      }
      else /* if (WN_is_constant_expr(WN_kid0(idx_expr))) */
      {
	 cexpr = WN_kid0(idx_expr);
	 nexpr = WN_kid1(idx_expr);
      }
      tmp_tokens = New_Token_Buffer();
      non_zero = WN2F_Expr_Plus_Literal(tmp_tokens, cexpr, 0LL, context);
      if (non_zero)
      {
	 if (parenthesize)
	 {
	    reset_WN2F_CONTEXT_no_parenthesis(context);
	    Append_Token_Special(tokens, '(');
	 }
	 WN2F_translate(tokens, nexpr, context);
	 Append_Token_Special(tokens, '+');
	 Append_And_Reclaim_Token_List(tokens, &tmp_tokens);
	 if (parenthesize)
	    Append_Token_Special(tokens, ')');
      }
      else
      {
	 Reclaim_Token_Buffer(&tmp_tokens);
	 WN2F_translate(tokens, nexpr, context);
      }
   }
   else if (WN_opc_operator(idx_expr) == OPR_SUB && 
	    (WN_is_constant_expr(WN_kid1(idx_expr)) || 
	     WN_is_constant_expr(WN_kid0(idx_expr))))
   {
      /* Do the "e-c" ==> "e-(c-1)" or the  "c-e" ==> "(c+1)-e"
       * translation.
       */
      cexpr_is_lhs = WN_is_constant_expr(WN_kid0(idx_expr));
      if (!cexpr_is_lhs)
      {
	 cexpr = WN_kid1(idx_expr);
	 nexpr = WN_kid0(idx_expr);
	 plus_value = 0LL;
      }
      else
      {
	 cexpr = WN_kid0(idx_expr);
	 nexpr = WN_kid1(idx_expr);
	 plus_value = 0LL;
      }
	
      /* Do the "e-c" ==> "e-(c-1)" or the  "c-e" ==> "(c+1)-e"
       * translation.
       */
      tmp_tokens = New_Token_Buffer();
      non_zero = 
	 WN2F_Expr_Plus_Literal(tmp_tokens, cexpr, plus_value, context);
      if (non_zero)
      {
	 if (parenthesize)
	 {
	    reset_WN2F_CONTEXT_no_parenthesis(context);
	    Append_Token_Special(tokens, '(');
	 }
	 if (!cexpr_is_lhs)
	 {
	    WN2F_translate(tokens, nexpr, context);
	    Append_Token_Special(tokens, '-');
	    Append_And_Reclaim_Token_List(tokens, &tmp_tokens);
	 }
	 else
	 {
	    Append_And_Reclaim_Token_List(tokens, &tmp_tokens);
	    Append_Token_Special(tokens, '-');
	    WN2F_translate(tokens, nexpr, context);
	 }
	 if (parenthesize)
	    Append_Token_Special(tokens, ')');
      }
      else
      {
	 Reclaim_Token_Buffer(&tmp_tokens); 
	 if (cexpr_is_lhs)
	 {
	    if (parenthesize)
	    {
	       reset_WN2F_CONTEXT_no_parenthesis(context);
	       Append_Token_Special(tokens, '(');
	    }
	    Append_Token_Special(tokens, '-');
	    WN2F_translate(tokens, nexpr, context);
	    if (parenthesize)
	       Append_Token_Special(tokens, ')');
	 }
	 else
	 {
	    WN2F_translate(tokens, nexpr, context);
	 }
      }
   }
   else
   {
      WN2F_Expr_Plus_Literal(tokens, idx_expr, 0LL, context); 
   }
   return EMPTY_WN2F_STATUS;
} /* WN2F_Denormalize_Array_Idx */


static void
WN2F_Normalize_Idx_To_Onedim(TOKEN_BUFFER tokens, 
			     WN*          wn,
			     WN2F_CONTEXT context)
{
   INT32 dim1, dim2;

   /* Parenthesize the normalized index expressions */
   reset_WN2F_CONTEXT_no_parenthesis(context);

   for (dim1 = 0; dim1 < WN_num_dim(wn); dim1++)
   {
      if (dim1 > 0)
	 Append_Token_Special(tokens, '+');

      /* Multiply the index expression with the product of the sizes
       * of subordinate dimensions, where a higher dimension-number
       * means a more subordinate dimension.  Do not parenthesize the
       * least significant index expression.
       */   
      if (dim1+1 == WN_num_dim(wn))
	 set_WN2F_CONTEXT_no_parenthesis(context);
      WN2F_Denormalize_Array_Idx(tokens, WN_array_index(wn, dim1), context);
      for (dim2 = dim1+1; dim2 < WN_num_dim(wn); dim2++)
      {
	 Append_Token_Special(tokens, '*');
	 (void)WN2F_translate(tokens, WN_array_dim(wn, dim2), context);
      } /*for*/
   } /*for*/
} /* WN2F_Normalize_Idx_To_Onedim */


static void
WN2F_Substring(TOKEN_BUFFER tokens, 
	       INT64        string_size,
	       WN          *lower_bnd,
	       WN          *substring_size,
	       WN2F_CONTEXT context)
{
   /* Given a substring offset from the base of a character string 
    * (lower_bnd), the size of the whole string, and the size of the
    * substring, generate the notation necessary as a suffix to the
    * string reference to denote the substring.
    */
   if (WN_opc_operator(lower_bnd) != OPR_INTCONST      ||
       WN_const_val(lower_bnd) != 0                    ||
       WN_opc_operator(substring_size) != OPR_INTCONST ||
       WN_const_val(substring_size) != string_size)
   {
      /* Need to generate substring expression "(l+1:l+size)" */
      Append_Token_Special(tokens, '(');
      set_WN2F_CONTEXT_no_parenthesis(context);
/*      WN2F_Denormalize_Array_Idx(tokens, lower_bnd, context);*/

      WN2F_OLD_Den_Arr_Idx(tokens, lower_bnd, context);

      reset_WN2F_CONTEXT_no_parenthesis(context);
      Append_Token_Special(tokens, ':');
      if (WN_opc_operator(lower_bnd) != OPR_INTCONST ||
	  WN_const_val(lower_bnd) != 0)
      {
	 WN2F_translate(tokens, lower_bnd, context);
	 Append_Token_Special(tokens, '+');
      }
      WN2F_translate(tokens, substring_size, context);
      Append_Token_Special(tokens, ')');
   }
} /* WN2F_Substring */


static void
WN2F_Get_Substring_Info(WN **base,         /* Possibly OPR_ARRAY node (in/out) */
			TY_IDX *string_ty, /* The string type (out) */
			WN **lower_bnd,    /* The lower bound index (out) */
		        WN **length )
{
   /* There are two possibilities concerning the array base expressions.
    * It can be a pointer to a complete character-string (array) or it
    * can be a pointer to a character within a character-string (single
    * character).  In the first instance, the offset off the base of 
    * string is zero.  In the latter case, the offset is given by the
    * array indexing operation.
    */
   TY_IDX ptr_ty = WN_Tree_Type(*base);

   *string_ty = TY_pointed(ptr_ty);

   if (TY_size(*string_ty) == 1 && 
       !TY_Is_Array(*string_ty) &&
       WN_opc_operator(*base) == OPR_ARRAY)
   {
      /* Let the base of the string be denoted as the base of the array
       * expression.
       */
      *string_ty = TY_pointed(WN_Tree_Type(WN_kid0(*base)));
      *lower_bnd = WN_array_index(*base, 0);
      *length    = WN_kid1(*base);
      *base = WN_kid0(*base);
   }
   else if (WN_opc_operator(*base) == OPR_ARRAY &&
	    TY_Is_Array(*string_ty)             &&
	    TY_AR_ndims(*string_ty) == 1        &&
	    TY_Is_Character_String(*string_ty)  &&
	    !TY_ptr_as_array(Ty_Table[ptr_ty]))
   {
      /* Presumably, the lower bound is given by the array operator
       */
      *lower_bnd = WN_array_index(*base, 0);
      *length    = WN_kid1(*base);
      *base = WN_kid0(*base);
   }
   else
   {
      *lower_bnd = WN2F_INTCONST_ZERO;
      *length    = WN2F_INTCONST_ZERO;
   }
} /* WN2F_Get_Substring_Info */

static WN *
WN2F_Find_Base(WN *addr)
{
  /* utility to find base of address tree */

  WN *res = addr;

  switch (WN_operator(addr))
  {
    case OPR_ARRAY: 
    case OPR_ILOAD:
    res=WN_kid0(addr);
    break;

    case OPR_ADD:
      if (WN_operator(WN_kid0(addr)) == OPR_INTCONST)
	res = WN2F_Find_Base(WN_kid1(addr));
      else
	res = WN2F_Find_Base(WN_kid0(addr));
    break;

  default:
    res = addr;
    break;
  }
  return res;
}

extern BOOL
WN2F_Is_Address_Preg(WN * ad ,TY_IDX ptr_ty)
{
  /* Does this look like a preg or variable being used as an address ? */
  /* These are propagated by opt/pfa                                   */

  BOOL is_somewhat_address_like = TY_kind(ptr_ty) == KIND_POINTER;
  
  if (TY_kind(ptr_ty) == KIND_SCALAR) 
  {
    TYPE_ID tid = TY_mtype(ptr_ty);

    is_somewhat_address_like |= (MTYPE_is_pointer(tid)) || (tid == MTYPE_I8) || (tid == MTYPE_I4) ;
  }

  if (is_somewhat_address_like)
  {
    WN * wn = WN2F_Find_Base(ad);
    
    if (WN_operator(wn) == OPR_LDID) 
    {
      ST * st = WN_st(wn) ;
      if (ST_class(st) == CLASS_PREG)
	return TRUE ;
      
      if (ST_class(st) == CLASS_VAR) 
      {
	if (TY_kind(ptr_ty) == KIND_SCALAR)
	  return TRUE;
	
	if (TY_kind(WN_ty(wn)) == KIND_SCALAR)
	{
	  TYPE_ID wtid = TY_mtype(WN_ty(wn));
	  
	  /* Looks like a Cray pointer (I4/I8) ? */
	  
	  if ((wtid == MTYPE_I8)|| (wtid == MTYPE_I4))
	    if (ad != wn)
	      return TRUE ;
	  
	  /* Looks like a VAR with a U4/U8? used  */
	  /* only with offsets, or FORMALs would  */
	  /* qualify, if intrinsic mtype          */
	  
	  if (MTYPE_is_pointer(wtid))
	    if (TY_kind(ST_type(st)) != KIND_SCALAR)
	      return TRUE;
	}
      }
    }
  }
  return FALSE;
}

/*---------------------- Prefetching Comments ------------------------*/
/*--------------------------------------------------------------------*/

static void
WN2F_Append_Prefetch_Map(TOKEN_BUFFER tokens, WN *wn)
{
   PF_POINTER* pfptr;
   const char *info_str;
   
   pfptr = (PF_POINTER*)WN_MAP_Get(WN_MAP_PREFETCH, wn);
   info_str = "prefetch (ptr, lrnum): ";
   if (pfptr->wn_pref_1L)
   {
      info_str = 
	 Concat2_Strings(    info_str,
          Concat2_Strings(   "1st <", 
           Concat2_Strings(  Ptr_as_String(pfptr->wn_pref_1L),
            Concat2_Strings( ", ",
             Concat2_Strings(WHIRL2F_number_as_name(pfptr->lrnum_1L),
			     ">")))));
   }
   if (pfptr->wn_pref_2L)
   {
      info_str = 
	 Concat2_Strings(    info_str,
          Concat2_Strings(   "2nd <", 
           Concat2_Strings(  Ptr_as_String(pfptr->wn_pref_2L),
            Concat2_Strings( ", ",
             Concat2_Strings(WHIRL2F_number_as_name(pfptr->lrnum_2L),
			     ">")))));
   }
   Append_Token_String(tokens, info_str);
} /* WN2F_Append_Prefetch_Map */


/*----------------------- Exported Functions ------------------------*/
/*--------------------------------------------------------------------*/

void WN2F_Load_Store_initialize(void)
{
   /* Nothing to do at the moment */
} /* WN2F_Load_Store_initialize */


void WN2F_Load_Store_finalize(void)
{
   /* Nothing to do at the moment */
} /* WN2F_Load_Store_finalize */


extern WN2F_STATUS
WN2F_pstore(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
{
   TOKEN_BUFFER  lhs_tokens;
   TOKEN_BUFFER  rhs_tokens;
   TY_IDX        base_ty;
   ASSERT_DBG_FATAL(WN_opc_operator(wn) == OPR_PSTORE,
                    (DIAG_W2F_UNEXPECTED_OPC, "WN2F_pstore"));

   /* Get the base address into which we are storing a value */
   base_ty = WN_Tree_Type(WN_kid1(wn));
   if (!TY_Is_Pointer(base_ty))
      base_ty = WN_ty(wn);

   /* Get the lhs of the assignment (dereference address) */
      lhs_tokens = New_Token_Buffer();

      set_WN2F_CONTEXT_has_no_arr_elmt(context);

      WN2F_Offset_Memref(lhs_tokens,
                         WN_kid1(wn),           /* base-symbol */
                         base_ty,               /* base-type */
                         TY_pointed(WN_ty(wn)), /* object-type */
                         WN_store_offset(wn),   /* object-ofst */
                         context);
      reset_WN2F_CONTEXT_has_no_arr_elmt(context);

   /* The rhs */
   rhs_tokens = New_Token_Buffer();
   if (TY_is_logical(Ty_Table[TY_pointed(WN_ty(wn))]))
   {
      set_WN2F_CONTEXT_has_logical_arg(context);
      WN2F_translate(rhs_tokens, WN_kid0(wn), context);
      reset_WN2F_CONTEXT_has_logical_arg(context);
   }
   else
     {

      WN2F_translate(rhs_tokens, WN_kid0(wn), context);
     }

   /* See if we need to apply a "char" conversion to the rhs
    */
   if (TY_Is_Character_String(W2F_TY_pointed(WN_ty(wn), "PSTORE lhs")) &&
       TY_Is_Integral(WN_Tree_Type(WN_kid0(wn))))
   {
     
      Prepend_Token_Special(rhs_tokens, '(');
      Prepend_Token_String(rhs_tokens, "char");
      Append_Token_Special(rhs_tokens, ')');
   }

   /* Assign the rhs to the lhs.
    */
   if (Identical_Token_Lists(lhs_tokens, rhs_tokens))
   {
      /* Ignore this redundant assignment statement! */
      Reclaim_Token_Buffer(&lhs_tokens);
      Reclaim_Token_Buffer(&rhs_tokens);
   }
   else
   {
      /* See if there is any prefetch information with this store,
       * and if so insert information about it as a comment preceeding
       * the store.
       */
      if (W2F_Emit_Prefetch && WN_MAP_Get(WN_MAP_PREFETCH, wn))
      {
         Append_F77_Comment_Newline(tokens, 1, TRUE/*indent*/);
         WN2F_Append_Prefetch_Map(tokens, wn);
      }

      /* The assignment statement on a new line */
      WN2F_Stmt_Newline(tokens, NULL/*label*/, WN_linenum(wn), context);
      Append_And_Reclaim_Token_List(tokens, &lhs_tokens);
      Append_Token_String(tokens,"=>");
      Append_And_Reclaim_Token_List(tokens, &rhs_tokens);
   }

   return EMPTY_WN2F_STATUS;
} /* WN2F_pstore */

extern WN2F_STATUS 
WN2F_istore(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
{
   TOKEN_BUFFER  lhs_tokens;
   TOKEN_BUFFER  rhs_tokens;
   TY_IDX        base_ty;
   TY_IDX        object_ty;
   
   ASSERT_DBG_FATAL(WN_opc_operator(wn) == OPR_ISTORE, 
		    (DIAG_W2F_UNEXPECTED_OPC, "WN2F_istore"));

   /* Get the base address into which we are storing a value */
   base_ty = WN_Tree_Type(WN_kid1(wn));
   object_ty = TY_pointed(WN_ty(wn));

   if (!TY_Is_Pointer(base_ty))
      base_ty = WN_ty(wn);

//For pointer PP(:,...,:)
   if (TY_kind(TY_pointed(base_ty))==KIND_POINTER &&
        TY_is_f90_deferred_shape(TY_pointed(base_ty))) 
            base_ty = TY_pointed(base_ty);

   if (TY_kind(object_ty)==KIND_POINTER &&
        TY_is_f90_deferred_shape(object_ty)) 
            object_ty = TY_pointed(object_ty);

   /* Get the lhs of the assignment (dereference address) */
   lhs_tokens = New_Token_Buffer();
   if (WN_operator(WN_kid1(wn)) == OPR_LDA ||
       WN_operator(WN_kid1(wn)) == OPR_LDID )
            set_WN2F_CONTEXT_has_no_arr_elmt(context);
#if 0 
   WN2F_Offset_Memref(lhs_tokens, 
		      WN_kid1(wn),           /* base-symbol */
		      base_ty,               /* base-type */
		      object_ty, /* object-type */
		      WN_store_offset(wn),   /* object-ofst */
		      context);
#else 
      WN2F_translate(lhs_tokens, WN_kid1(wn), context);
#endif

    reset_WN2F_CONTEXT_has_no_arr_elmt(context); 

   /* The rhs */
   rhs_tokens = New_Token_Buffer();

   if (TY_is_logical(Ty_Table[TY_pointed(WN_ty(wn))]))
   {
      set_WN2F_CONTEXT_has_logical_arg(context);
      WN2F_translate(rhs_tokens, WN_kid0(wn), context);
      reset_WN2F_CONTEXT_has_logical_arg(context);
   }
   else
      WN2F_translate(rhs_tokens, WN_kid0(wn), context);

   /* See if we need to apply a "char" conversion to the rhs
    */
#if 0 
   if (TY_Is_Character_String(W2F_TY_pointed(WN_ty(wn), "ISTORE lhs")) &&
       TY_Is_Integral(WN_Tree_Type(WN_kid0(wn))))
   {
      Prepend_Token_Special(rhs_tokens, '(');
      Prepend_Token_String(rhs_tokens, "char");
      Append_Token_Special(rhs_tokens, ')');
   }
#endif

   /* Assign the rhs to the lhs.
    */
   if (Identical_Token_Lists(lhs_tokens, rhs_tokens))
   {
      /* Ignore this redundant assignment statement! */
      Reclaim_Token_Buffer(&lhs_tokens);
      Reclaim_Token_Buffer(&rhs_tokens);
   }
   else
   {
      /* See if there is any prefetch information with this store,
       * and if so insert information about it as a comment preceeding
       * the store.
       */
      if (W2F_Emit_Prefetch && WN_MAP_Get(WN_MAP_PREFETCH, wn))
      {
	 Append_F77_Comment_Newline(tokens, 1, TRUE/*indent*/);
	 WN2F_Append_Prefetch_Map(tokens, wn);
      }

      /* The assignment statement on a new line */
      WN2F_Stmt_Newline(tokens, NULL/*label*/, WN_linenum(wn), context);
      Append_And_Reclaim_Token_List(tokens, &lhs_tokens);
      Append_Token_Special(tokens, '=');
      Append_And_Reclaim_Token_List(tokens, &rhs_tokens);
   }

   fld_type_z = 0;

   return EMPTY_WN2F_STATUS;
} /* WN2F_istore */

WN2F_STATUS 
WN2F_istorex(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
{
   ASSERT_DBG_WARN(FALSE, (DIAG_UNIMPLEMENTED, "WN2F_istorex"));
   WN2F_Stmt_Newline(tokens, NULL/*label*/, WN_linenum(wn), context);
   Append_Token_String(tokens, WN_opc_name(wn));

   return EMPTY_WN2F_STATUS;
} /* WN2F_istorex */

WN2F_STATUS 
WN2F_mstore(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
{
   TOKEN_BUFFER  lhs_tokens;
   TOKEN_BUFFER  rhs_tokens;
   TY_IDX        base_ty;

   /* Note that we make the assumption that this is just like an 
    * ISTORE, and handle it as though it were.  We do not handle
    * specially assignment-forms where the lhs is incompatible with
    * the rhs, since we assume this will never happen for Fortran
    * and we cannot easily get around this like we do in C (i.e.
    * with cast expressions.
    */
   ASSERT_DBG_FATAL(WN_opc_operator(wn) == OPR_MSTORE, 
		    (DIAG_W2F_UNEXPECTED_OPC, "WN2F_mstore"));
#if 0
   ASSERT_DBG_WARN(WN_opc_operator(WN_kid0(wn)) == OPR_MLOAD,
		    (DIAG_W2F_UNEXPECTED_OPC, "rhs of WN2F_mstore"));

   //TODO: scalar expression allowed, but array/structure assignment assumed
   // with constant ie: should put out doloop?... call OFFSET_Memref?
#endif

   /* Get the base address into which we are storing a value */
   base_ty = WN_Tree_Type(WN_kid1(wn));
   if (!TY_Is_Pointer(base_ty))
      base_ty = WN_ty(wn);

   /* Get the lhs of the assignment (dereference address) */
   lhs_tokens = New_Token_Buffer();
#if 0  
   WN2F_Offset_Memref(lhs_tokens, 
		      WN_kid1(wn),           /* base-symbol */
		      base_ty,               /* base-type */
		      TY_pointed(WN_ty(wn)), /* object-type */
		      WN_store_offset(wn),   /* object-ofst */
		      context);
#else
   WN2F_translate(lhs_tokens, WN_kid1(wn), context);
#endif 
   
   
   /* The rhs */
   rhs_tokens = New_Token_Buffer();
   WN2F_translate(rhs_tokens, WN_kid0(wn), context);

   /* Assign the rhs to the lhs.
    */
   if (Identical_Token_Lists(lhs_tokens, rhs_tokens))
   {
      /* Ignore this redundant assignment statement! */
      Reclaim_Token_Buffer(&lhs_tokens);
      Reclaim_Token_Buffer(&rhs_tokens);
   }
   else
   {
      /* The assignment statement on a new line */
      WN2F_Stmt_Newline(tokens, NULL/*label*/, WN_linenum(wn), context);
      Append_And_Reclaim_Token_List(tokens, &lhs_tokens);
      Append_Token_Special(tokens, '=');
      Append_And_Reclaim_Token_List(tokens, &rhs_tokens);
   }


   return EMPTY_WN2F_STATUS;
} /* WN2F_mstore */

WN2F_STATUS 
WN2F_stid(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
{
   TOKEN_BUFFER lhs_tokens, rhs_tokens;
   const BOOL parenthesize = !WN2F_CONTEXT_no_parenthesis(context);
   TY_IDX base_ty;
   TY_IDX object_ty;


    if (parenthesize)
       set_WN2F_CONTEXT_no_parenthesis(context);
   
   ASSERT_DBG_FATAL(WN_opc_operator(wn) == OPR_STID, 
		    (DIAG_W2F_UNEXPECTED_OPC, "WN2F_stid"));

   if (W2F_OpenAD 
       && 
       ST_is_temp_var(WN_st(wn))
       && 
       (
	strncmp("select_expr_",ST_name(WN_st(wn)),12)!=0
	&& 
	!ST_keep_in_openad(WN_st(wn))))
      // emit assignments for symbols marked as to be kept and values of select expressions.
      // the hard coded name (see cwg_stmt.cxx:2271) 
      // is awful but not particularly worse than the other 
      // hacks done for select constructs
    return EMPTY_WN2F_STATUS;

   /* Get the lhs of the assignment */
   lhs_tokens = New_Token_Buffer();
   if (ST_class(WN_st(wn)) == CLASS_PREG)
   {
      ST2F_Use_Preg(lhs_tokens, ST_type(WN_st(wn)), WN_store_offset(wn));
   }
   else if (ST_sym_class(WN_st(wn))==CLASS_VAR && ST_is_not_used(WN_st(wn)))
   {
      /* This is a redundant assignment statement, so determined
       * by IPA, so just assign it to a temporary variable
       * instead.
       */
      UINT tmp_idx = Stab_Lock_Tmpvar(WN_ty(wn), &ST2F_Declare_Tempvar);
      Append_Token_String(lhs_tokens, W2CF_Symtab_Nameof_Tempvar(tmp_idx));
      Stab_Unlock_Tmpvar(tmp_idx);
   }
   else
   {
      base_ty = ST_type(WN_st(wn));
      if (TY_kind(base_ty)==KIND_POINTER &&
          TY_is_f90_pointer(base_ty)) 
         ;
      else
          base_ty = Stab_Pointer_To(base_ty);

      object_ty = WN_ty(wn);
      if ((TY_kind(object_ty)==KIND_POINTER) &&
          ( TY_is_f90_pointer(object_ty)))
           object_ty = TY_pointed(object_ty);
   
      WN2F_Offset_Symref(lhs_tokens, 
			 WN_st(wn),                        /* base-symbol */
                         base_ty,                          /* base_type   */
			 object_ty,                        /* object-type */
			 WN_store_offset(wn),              /* object-ofst */
			 context);
   }
   
   /* The rhs */
   rhs_tokens = New_Token_Buffer();

//   const BOOL p11arenthesize = !WN2F_CONTEXT_no_parenthesis(context);

   if (TY_is_logical(Ty_Table[WN_ty(wn)]))
   {
      set_WN2F_CONTEXT_has_logical_arg(context);
      WN2F_translate(rhs_tokens, WN_kid0(wn), context);
      reset_WN2F_CONTEXT_has_logical_arg(context);
   }
   else {
    
//      set_WN2F_CONTEXT_no_parenthesis(context);
      WN2F_translate(rhs_tokens, WN_kid0(wn), context);
//      reset_WN2F_CONTEXT_no_parenthesis(context);
     }

   /* See if we need to apply a "char" conversion to the rhs
    */

   if (TY_Is_Character_String(WN_ty(wn)) && 
       TY_Is_Integral(WN_Tree_Type(WN_kid0(wn))))
   {
      Prepend_Token_Special(rhs_tokens, '(');
      Prepend_Token_String(rhs_tokens, "char");
      Append_Token_Special(rhs_tokens, ')');
   }

   /* Assign the rhs to the lhs.
    */


   if (!WN2F_CONTEXT_emit_stid(context) &&
       Identical_Token_Lists(lhs_tokens, rhs_tokens))
   {
      /* Ignore this redundant assignment statement! */
      Reclaim_Token_Buffer(&lhs_tokens);
      Reclaim_Token_Buffer(&rhs_tokens);
   }
   else
   { 
      /* The assignment statement on a new line */
      WN2F_Stmt_Newline(tokens, NULL/*label*/, WN_linenum(wn), context);
      Append_And_Reclaim_Token_List(tokens, &lhs_tokens);
      Append_Token_Special(tokens, '=');
      Append_And_Reclaim_Token_List(tokens, &rhs_tokens);
   }


   return EMPTY_WN2F_STATUS;
} /* WN2F_stid */


WN2F_STATUS
WN2F_pstid(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
{
   TOKEN_BUFFER lhs_tokens, rhs_tokens;

   ASSERT_DBG_FATAL(WN_opc_operator(wn) == OPR_PSTID,
                    (DIAG_W2F_UNEXPECTED_OPC, "WN2F_pstid"));

   /* Get the lhs of the assignment */
   lhs_tokens = New_Token_Buffer();
   if (ST_class(WN_st(wn)) == CLASS_PREG)
   {
      ST2F_Use_Preg(lhs_tokens, ST_type(WN_st(wn)), WN_store_offset(wn));
   }
   else if (ST_sym_class(WN_st(wn))==CLASS_VAR && ST_is_not_used(WN_st(wn)))
   {
      /* This is a redundant assignment statement, so determined
       * by IPA, so just assign it to a temporary variable
       * instead.
       */
      UINT tmp_idx = Stab_Lock_Tmpvar(WN_ty(wn), &ST2F_Declare_Tempvar);
      Append_Token_String(lhs_tokens, W2CF_Symtab_Nameof_Tempvar(tmp_idx));
      Stab_Unlock_Tmpvar(tmp_idx);
   }
   else
   {
      WN2F_Offset_Symref(lhs_tokens,
                         WN_st(wn),                        /* base-symbol */
                         Stab_Pointer_To(ST_type(WN_st(wn))),/* base-type */
                         WN_ty(wn),                        /* object-type */
                         WN_store_offset(wn),              /* object-ofst */
                         context);
   }

   /* The rhs */
   rhs_tokens = New_Token_Buffer();
   if (TY_is_logical(Ty_Table[WN_ty(wn)]))
   {
      set_WN2F_CONTEXT_has_logical_arg(context);
      WN2F_translate(rhs_tokens, WN_kid0(wn), context);
      reset_WN2F_CONTEXT_has_logical_arg(context);
   }
   else
      WN2F_translate(rhs_tokens, WN_kid0(wn), context);

   /* See if we need to apply a "char" conversion to the rhs
    */
   if (TY_Is_Character_String(WN_ty(wn)) &&
       TY_Is_Integral(WN_Tree_Type(WN_kid0(wn))))
   {
      Prepend_Token_Special(rhs_tokens, '(');
      Prepend_Token_String(rhs_tokens, "char");
      Append_Token_Special(rhs_tokens, ')');
   }

   /* Assign the rhs to the lhs.
    */
   if (!WN2F_CONTEXT_emit_stid(context) &&
       Identical_Token_Lists(lhs_tokens, rhs_tokens))
   {
      /* Ignore this redundant assignment statement! */
      Reclaim_Token_Buffer(&lhs_tokens);
      Reclaim_Token_Buffer(&rhs_tokens);
   }
   else
   {
      /* The assignment statement on a new line */
      WN2F_Stmt_Newline(tokens, NULL/*label*/, WN_linenum(wn), context);
      Append_And_Reclaim_Token_List(tokens, &lhs_tokens);
      Append_Token_String(tokens,"=>");
      Append_And_Reclaim_Token_List(tokens, &rhs_tokens);
   }

   return EMPTY_WN2F_STATUS;
} /* WN2F_pstid */


WN2F_STATUS 
WN2F_iload(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
{
   TY_IDX base_ty;
   TY_IDX object_ty;
   
   /* Note that we handle this just like we do the lhs of an ISTORE.
    */
   ASSERT_DBG_FATAL(WN_opc_operator(wn) == OPR_ILOAD, 
		    (DIAG_W2F_UNEXPECTED_OPC, "WN2F_iload"));

  
   if (WN_operator(WN_kid0(wn))==OPR_STRCTFLD){ //place-hold for pointer field
         WN2F_translate(tokens, WN_kid0(wn), context);   
         return EMPTY_WN2F_STATUS;
     }

   /* Special case for Purple (address values have no meaning, so emit
    * symbolic values for them).
    */
   if (W2F_Only_Mark_Loads && !TY_Is_Pointer(WN_ty(wn)))
   {
      char buf[64];
      sprintf(buf, "#<%p>#", wn);
      Append_Token_String(tokens, buf);
      return EMPTY_WN2F_STATUS;
   }

   /* Get the type of the base from which we are loading */
   base_ty = WN_Tree_Type(WN_kid0(wn));
   object_ty = TY_pointed(WN_load_addr_ty(wn));

   if (!TY_Is_Pointer(base_ty))
      base_ty = WN_load_addr_ty(wn);
    
    /*For pointer PP(:,...,:) */
   if (TY_kind(TY_pointed(base_ty))==KIND_POINTER &&
        TY_is_f90_deferred_shape(TY_pointed(base_ty))) 
            base_ty = TY_pointed(base_ty);

   if (TY_kind(object_ty)==KIND_POINTER &&
        TY_is_f90_deferred_shape(object_ty)) 
            object_ty = TY_pointed(object_ty);
    
   /* Get the object to be loaded (dereference address) */
   if (WN_opc_operator(WN_kid0(wn)) == OPR_LDA ||
       WN_opc_operator(WN_kid0(wn)) == OPR_LDID)
          set_WN2F_CONTEXT_has_no_arr_elmt(context);
#if 0 
   WN2F_Offset_Memref(tokens, 
		      WN_kid0(wn),                     /* base-symbol */
		      base_ty,                         /* base-type */
		      object_ty,                       /* object-type */
		      WN_load_offset(wn),              /* object-ofst */
		      context);
#else 
         WN2F_translate(tokens, WN_kid0(wn), context);   
#endif
    reset_WN2F_CONTEXT_has_no_arr_elmt(context);

   /* See if there is any prefetch information with this load, and 
    * if so insert information about it as a comment on a separate
    * continuation line.
    */
   if (W2F_Emit_Prefetch && WN_MAP_Get(WN_MAP_PREFETCH, wn))
   {
      Set_Current_Indentation(Current_Indentation()+3);
      Append_F77_Indented_Continuation(tokens);
      Append_Token_Special(tokens, '!');
      WN2F_Append_Prefetch_Map(tokens, wn);
      Set_Current_Indentation(Current_Indentation()-3); 
      Append_F77_Indented_Continuation(tokens);
   }
   
   return EMPTY_WN2F_STATUS;
} /* WN2F_iload */

WN2F_STATUS 
WN2F_iloadx(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
{
   ASSERT_DBG_WARN(FALSE, (DIAG_UNIMPLEMENTED, "WN2F_iloadx"));
   Append_Token_String(tokens, WN_opc_name(wn));

   return EMPTY_WN2F_STATUS;
} /* WN2F_iloadx */


WN2F_STATUS 
WN2F_mload(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
{
   TY_IDX base_ty;
   
   /* This should only appear the as the rhs of an ISTORE.  Treat
    * it just like an ILOAD.
    */
   ASSERT_DBG_FATAL(WN_opc_operator(wn) == OPR_MLOAD, 
		    (DIAG_W2F_UNEXPECTED_OPC, "WN2F_mload"));

   /* Special case for Purple (cannot be a pointer value) */
   if (W2F_Only_Mark_Loads)
   {
      char buf[64];
      sprintf(buf, "#<%p>#", wn);
      Append_Token_String(tokens, buf);
      return EMPTY_WN2F_STATUS;
   }

   /* Get the type of the base from which we are loading */
   base_ty = WN_Tree_Type(WN_kid0(wn));
   if (!TY_Is_Pointer(base_ty))
      base_ty = WN_ty(wn);

   /* Get the object to be loaded */
   WN2F_Offset_Memref(tokens, 
		      WN_kid0(wn),                     /* base-symbol */
		      base_ty,                         /* base-type */
		      TY_pointed(WN_ty(wn)), /* object-type */
		      WN_load_offset(wn),              /* object-ofst */
		      context);
   return EMPTY_WN2F_STATUS;
} /* WN2F_mload */


WN2F_STATUS 
WN2F_ldid(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
{
   const BOOL deref = WN2F_CONTEXT_deref_addr(context);
   TY_IDX    base_ptr_ty;
   TY_IDX    object_ty;
   
   ASSERT_DBG_FATAL(WN_opc_operator(wn) == OPR_LDID, 
		    (DIAG_W2F_UNEXPECTED_OPC, "WN2F_ldid"));

   /* Special recognition of a LEN intrinsic call
    */
   if (WN_load_offset(wn) == 0             &&
       TY_Is_Integral(WN_ty(wn))           &&
       ST_sclass(WN_st(wn))==SCLASS_FORMAL &&
       ST_is_value_parm(WN_st(wn))         &&
       strncmp(ST_name(WN_st(wn)), ".length.", strlen(".length.")) == 0)
   {
      /* Search the current PU parameters for a name that matches the
       * part of the ST_name that follows the ".length." prefix.
       */
      ST *st_param = 
	 WN2F_Get_Named_Param(PUinfo_current_func,
			      ST_name(WN_st(wn)) + strlen(".length."));

      if (st_param != NULL)
      {
	 WN2F_Translate_StringLEN(tokens, st_param);
	 return EMPTY_WN2F_STATUS;
      }
   }

   /* Special case for Purple direct loads, except direct loads of
    * addresses (address values have no meaning in the purple output).
    */
   if (W2F_Only_Mark_Loads && !TY_Is_Pointer(WN_ty(wn)))
   {
      char buf[64];
      sprintf(buf, "#<%p>#", wn);
      Append_Token_String(tokens, buf);
      return EMPTY_WN2F_STATUS;
   }

   if (ST_class(WN_st(wn)) == CLASS_PREG ) 
   {
      char buffer[64];
      STAB_OFFSET addr_offset = WN_load_offset(wn);
      object_ty = PUinfo_Preg_Type(ST_type(WN_st(wn)), addr_offset);


      if (addr_offset == -1 ) {
         switch (TY_mtype(Ty_Table[WN_ty(wn)])) {
         case MTYPE_I8:
         case MTYPE_U8:
         case MTYPE_I1:
         case MTYPE_I2:
         case MTYPE_I4:
         case MTYPE_U1:
         case MTYPE_U2:
         case MTYPE_U4:
            sprintf(buffer, "reg%d", First_Int_Preg_Return_Offset);
            Append_Token_String(tokens, buffer);
            break;
         case MTYPE_F4:
         case MTYPE_F8:
         case MTYPE_FQ:
         case MTYPE_C4:
         case MTYPE_C8:
         case MTYPE_CQ:
            sprintf(buffer, "reg%d", First_Float_Preg_Return_Offset);
            Append_Token_String(tokens, buffer);
            break;
         case MTYPE_M:
            Fail_FmtAssertion ("MLDID of Return_Val_Preg not allowed in middle"
               " of expression");
            break;
         default:
            Fail_FmtAssertion ("Unexpected type in WN2C_ldid()");
	    break;
         } 
      }
      else  
      {
         ST2F_Use_Preg(tokens, ST_type(WN_st(wn)), WN_load_offset(wn));
      } 
   } 
   else     
   {
      /* Get the base and object type symbols.
       */
      if (deref && TY_Is_Pointer(ST_type(WN_st(wn))))
      {
	 /* Expect the loaded type to be a pointer to the type of object
	  * to be dereferenced.  The only place (besides declaration sites)
	  * where we expect to have to specially handle ptr_as_array 
	  * objects.
	  */
	 if (TY_ptr_as_array(Ty_Table[WN_ty(wn)]))
	    object_ty = Stab_Array_Of(TY_pointed(WN_ty(wn)), 0/*size*/);
	 else
	    object_ty = TY_pointed(WN_ty(wn));

	 /* There are two possibilities for the base type:  A regular 
	  * pointer or a pointer to be treated as a pointer to an array.
	  * In either case, the "base_ptr_ty" is a pointer to the 
	  * derefenced base type.  
	  *
	  * Note that this does not handle a pointer to a struct to be
	  * treated as an array of structs, where the object type and
	  * offset denote a member of the struct, since WN2F_Offset_Symref() 
	  * cannot access a struct member through an array access.
	  */
	 if (TY_ptr_as_array(Ty_Table[ST_type(WN_st(wn))]))
	    base_ptr_ty = 
	       Stab_Pointer_To(Stab_Array_Of(TY_pointed(ST_type(WN_st(wn))),
							0/*size*/));
	 else
	    base_ptr_ty = ST_type(WN_st(wn));
      }
      else
      {
	 /* Either not a dereference, or possibly a dereference off a 
	  * record/map/common/equivalence field.  The base symbol is
	  * not a pointer, and any dereferencing on a field will occur
	  * in WN2F_Offset_Symref().
	  */
	 object_ty = WN_ty(wn);
         if ((TY_kind(object_ty)==KIND_POINTER) &&
              (TY_is_f90_pointer(object_ty)))
                  object_ty = TY_pointed(object_ty);

         base_ptr_ty = ST_type(WN_st(wn));
         if ((TY_kind(base_ptr_ty)==KIND_POINTER) &&
              (TY_is_f90_pointer(base_ptr_ty)))
                 ;
         else 
	     base_ptr_ty = Stab_Pointer_To(base_ptr_ty);
      }

      if (!deref && STAB_IS_POINTER_REF_PARAM(WN_st(wn)))
      {
	 /* Since we do not wish to dereference a load of a reference 
          * parameter, this must mean we are taking the address of the
          * parameter.
          */

      }
      set_WN2F_CONTEXT_has_no_arr_elmt(context);
      WN2F_Offset_Symref(tokens, 
			 WN_st(wn),           /* base-symbol */
			 base_ptr_ty,         /* base-type */
			 object_ty,           /* object-type */
			 WN_load_offset(wn),  /* object-ofst */
			 context);
      reset_WN2F_CONTEXT_has_no_arr_elmt(context);

      if (!deref && STAB_IS_POINTER_REF_PARAM(WN_st(wn)))
      {
      }
   }
   return EMPTY_WN2F_STATUS;
} /* WN2F_ldid */


WN2F_STATUS 
WN2F_lda(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
{
  const BOOL deref = WN2F_CONTEXT_deref_addr(context);
  
  ASSERT_DBG_FATAL(WN_opc_operator(wn) == OPR_LDA, 
		   (DIAG_W2F_UNEXPECTED_OPC, "WN2F_lda"));
  ASSERT_DBG_FATAL(ST_class(WN_st(wn)) != CLASS_PREG, 
		   (DIAG_W2F_CANNOT_LDA_PREG));

  TY_IDX object_ty;
  
  if (!deref)
    {
      /* A true address-of operation */
      set_WN2F_CONTEXT_no_parenthesis(context);
    }
  
  /* Sometimes we need to deal with buggy WHIRL code, where the TY
   * associated with an LDA is not a pointer type.  For such cases
   * we infer a type here.
   */

  if (TY_Is_Pointer(WN_ty(wn)) )
    {
      object_ty = TY_pointed(WN_ty(wn));
    }
  else
    {
      /* May be wrong, but the best we can do under these exceptional */
      /* circumstances. */
	
      object_ty = ST_type(WN_st(wn));
    }


  ST * st = WN_st(wn);
  TY_IDX ty ;
  TY_IDX ty_1;
  reset_WN2F_CONTEXT_deref_addr(context);

  if (ST_sym_class(st) == CLASS_BLOCK)
  {
    WN2F_Block(tokens,st,WN_lda_offset(wn),context);
  }
  else 
  {
    if (TY_is_f90_pointer(ST_type(st)))
         ty_1= TY_pointed(ST_type(st));
    else
         ty_1= ST_type(st);
     ty =  Stab_Pointer_To(ty_1);

   set_WN2F_CONTEXT_has_no_arr_elmt(context);
    WN2F_Offset_Symref(tokens, 
		       WN_st(wn),                           /* base-symbol */
		       ty,                                  /* base type   */
		       object_ty,                           /* object-type */
		       WN_lda_offset(wn),                   /* object-ofst */
		       context);
    reset_WN2F_CONTEXT_has_no_arr_elmt(context);
  }

  return EMPTY_WN2F_STATUS;
} /* WN2F_lda */

 WN2F_STATUS
 WN2F_arrayexp(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
{
 WN    * kid;
 kid    = WN_kid0(wn);
 WN2F_translate(tokens, kid, context);

return EMPTY_WN2F_STATUS;
}

/******************************************************************************\
|*                                                                            *|
|* for array section triplet node,kid0 is lower bound,it should plus 1LL for  *|
|* adjusted bound,upper bound=kid0+k1*k2                                      *|
|* kid0 evaluates to the starting integer value of the progression.	      *|
|* kid1 evaluates to an integer value that gives the stride in the progression*|
|* kid2 evaluates to the number of values in the progression       	      *|
|*                                                                            *|
\******************************************************************************/
 
WN2F_STATUS
WN2F_triplet(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
{
   WN      *kid0;
   WN      *kid1;
   WN      *kid2;
  kid0=WN_kid0(wn);
  kid1=WN_kid1(wn);
  kid2=WN_kid2(wn);
  (void)WN2F_Denormalize_Array_Idx(tokens,kid0,context);
 if ((WN_opc_operator(kid2) == OPR_INTCONST) &&
     (WN_const_val(kid2)==INT_MIN) )
   Append_Token_Special(tokens, ':');
 else  {
  Append_Token_Special(tokens, ':');
  if (WN_opc_operator(kid0) == OPR_INTCONST &&
      WN_opc_operator(kid1) == OPR_INTCONST &&
      WN_opc_operator(kid2) == OPR_INTCONST) {
     if ((WN_const_val(kid0)+WN_const_val(kid2)*WN_const_val(kid1))>=INT_MAX)
       TCON2F_translate(tokens,
                 Host_To_Targ(MTYPE_I8,   
                       WN_const_val(kid0)+
                       WN_const_val(kid2)*
                       WN_const_val(kid1)),
                      FALSE);
    else    

      TCON2F_translate(tokens,
                 Host_To_Targ(MTYPE_I4,   
                       WN_const_val(kid0)+
                       WN_const_val(kid2)*
                       WN_const_val(kid1)),
                      FALSE);

   } 
   else 
       if (WN_opc_operator(kid0) == OPR_INTCONST &&
           WN_opc_operator(kid1) == OPR_INTCONST ) {
           if (WN_const_val(kid1)==1) {
               if (WN_const_val(kid0)== 0) {
                   WN2F_translate(tokens, kid2, context);
                 }
               else {
                     WN2F_translate(tokens, kid1, context);
                     Append_Token_Special(tokens, '+');
                     WN2F_translate(tokens, kid2, context); }
            }
            else {
            if (WN_const_val(kid0)== 0){
            WN2F_translate(tokens, kid1, context);
             Append_Token_Special(tokens, '*');
             WN2F_translate(tokens, kid2, context); }
             else {
                  WN2F_translate(tokens, kid0, context);
                  Append_Token_Special(tokens, '+');
                  WN2F_translate(tokens, kid1, context);
                  Append_Token_Special(tokens, '*');
                  WN2F_translate(tokens, kid2, context); }
            }
          }
          else 
          if (WN_opc_operator(kid1) == OPR_INTCONST &&
              WN_opc_operator(kid2) == OPR_INTCONST) {
                   WN2F_translate(tokens, kid0, context);
                   Append_Token_Special(tokens, '+');
       
          if ((WN_const_val(kid1)*WN_const_val(kid2))>=INT_MAX)
 
                   TCON2F_translate(tokens,
                                Host_To_Targ(MTYPE_I8,   
                                           WN_const_val(kid1)*
                                           WN_const_val(kid2)),
                                           FALSE);
          else 
                   TCON2F_translate(tokens,
                                Host_To_Targ(MTYPE_I4,   
                                           WN_const_val(kid1)*
                                           WN_const_val(kid2)),
                                           FALSE);
          }
          else 
          if (WN_opc_operator(kid0) == OPR_INTCONST &&
              WN_opc_operator(kid2) == OPR_INTCONST) {
              if (WN_const_val(kid2)==1) {
               if (WN_const_val(kid0)== 0) {
                   WN2F_translate(tokens, kid1, context);
                }
               else {
                     WN2F_translate(tokens, kid0, context);
                     Append_Token_Special(tokens, '+');
                     WN2F_translate(tokens, kid1, context); 
                    }
            }
            else {
            if (WN_const_val(kid0)== 0){
            WN2F_translate(tokens, kid2, context);
             Append_Token_Special(tokens, '*');
             WN2F_translate(tokens, kid1, context); }
             else {
                  WN2F_translate(tokens, kid0, context);
                  Append_Token_Special(tokens, '+');
                  WN2F_translate(tokens, kid1, context);
                  Append_Token_Special(tokens, '*');
                  WN2F_translate(tokens, kid2, context); }
              }
            }
           else 
           if (WN_opc_operator(kid0) == OPR_INTCONST){ 
              if (WN_const_val(kid0)==0) {
                  WN2F_translate(tokens, kid1, context);
                  Append_Token_Special(tokens, '*');
                  WN2F_translate(tokens, kid2, context);}
              else {
                 WN2F_translate(tokens, kid0, context);
                 Append_Token_Special(tokens, '+');
                 WN2F_translate(tokens, kid1, context);
                 Append_Token_Special(tokens, '*');
                 WN2F_translate(tokens, kid2, context);
                 }
            }
            else 
            if (WN_opc_operator(kid1) == OPR_INTCONST){
                WN2F_translate(tokens, kid0, context);
                Append_Token_Special(tokens, '+');
                if (WN_const_val(kid1)==1){
                    WN2F_translate(tokens, kid2, context);}
                 else {
                      WN2F_translate(tokens, kid1, context);
                      Append_Token_Special(tokens, '*');
                      WN2F_translate(tokens, kid2, context);
                        }
            }
            else
            if (WN_opc_operator(kid2) == OPR_INTCONST) {
                  WN2F_translate(tokens, kid0, context);
                  Append_Token_Special(tokens, '+');
                  if (WN_const_val(kid2)==1)
                   WN2F_translate(tokens, kid1, context);
                  else
                    {
                            WN2F_translate(tokens, kid2, context);
                            Append_Token_Special(tokens, '*');
                             WN2F_translate(tokens, kid1, context);
                    }
              }
         if ((WN_opc_operator(kid1) == OPR_INTCONST) && 
               (WN_const_val(kid1)==1))  {
         } else {
                Append_Token_Special(tokens, ':');
                WN2F_translate(tokens, kid1, context);
               } 
    }  
    return EMPTY_WN2F_STATUS;

    }
/******************************************************************************\
|*                                                                            *|
|*                                                                            *|
\******************************************************************************/

WN2F_STATUS
WN2F_src_triplet(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
{
   WN      *kid0;
   WN      *kid1;
   WN      *kid2;
  kid0=WN_kid0(wn);
  kid1=WN_kid1(wn);
  kid2=WN_kid2(wn);
  WN2F_translate(tokens, kid0, context);
  Append_Token_Special(tokens, ':');
  WN2F_translate(tokens, kid1, context); 

  if (WN_operator(kid2) == OPR_INTCONST &&
      WN_const_val(kid2) == 1)
      ;
  else {
        Append_Token_Special(tokens, ':');
        WN2F_translate(tokens, kid2, context); 
       }

  return EMPTY_WN2F_STATUS;

    }

WN2F_STATUS
WN2F_arrsection(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
{
   /* Note that array indices have been normalized to assume the
    * array is based at index zero.  Since a base at index 1 is
    * the default for Fortran, we denormalize to base 1 here.
    */
   BOOL  deref = WN2F_CONTEXT_deref_addr(context);
   WN    * kid;
   TY_IDX ptr_ty;
   TY_IDX array_ty;


   ASSERT_DBG_FATAL(WN_opc_operator(wn) == OPR_ARRAY,
                    (DIAG_W2F_UNEXPECTED_OPC, "WN2F_array"));

   /* Only allow taking the address of an array element for F90!
    *
    */
#if 0
 else
      ASSERT_DBG_WARN(deref,
                      (DIAG_UNIMPLEMENTED,
                       "taking the address of an array element"));
#endif

   /* Get the array or, for ptr-as-array types, the element type */

   kid    = WN_kid0(wn);
   if (WN_operator(kid) == OPR_ILOAD  &&
       WN_operator(WN_kid0(kid)) == OPR_STRCTFLD) 
          kid = WN_kid0(kid) ;

   ptr_ty = WN_Tree_Type(kid);

        

   if (WN2F_Is_Address_Preg(kid,ptr_ty))
   {
       /* a preg or sym has been used as an address, usually after optimization
*/
       /* don't know base type, or anything else so use OPR_ARRAY to generate bounds
*/

     WN2F_translate(tokens, kid, context);
     WN2F_Arrsection_Slots(tokens,wn,ptr_ty,context,TRUE);
   }
   else
   {

     if (WN_operator(kid)==OPR_STRCTFLD)
          array_ty = WN_ty(kid);
     else
          array_ty = W2F_TY_pointed(ptr_ty, "base of OPC_ARRAY");
    
     if (WN_opc_operator(kid) == OPR_LDID       &&
         ST_sclass(WN_st(kid)) == SCLASS_FORMAL &&
         !ST_is_value_parm(WN_st(kid))          &&
         WN_element_size(wn) == TY_size(array_ty)       &&
         WN_num_dim(wn) == 1                            &&
         WN_opc_operator(WN_array_index(wn, 0)) == OPR_INTCONST &&
         WN_const_val(WN_array_index(wn, 0)) == 0       &&
         !TY_ptr_as_array(Ty_Table[WN_ty(kid)])           &&
         (!TY_Is_Array(array_ty) ||
          TY_size(TY_AR_etype(array_ty)) < TY_size(array_ty)))
     {
         /* This array access is just a weird representation for an implicit
          * reference parameter dereference.  Ignore the array indexing.
          */

       WN2F_translate(tokens, kid, context);
     }
     else if (!TY_ptr_as_array(Ty_Table[ptr_ty]) && TY_Is_Character_String(array_ty))
     {
         /* We assume that substring accesses are treated in the handling
          * of intrinsic functions, except when the substrings are to be
          * handled as integral types and thus are encountered here.
          */
#if 0
       if (!WN2F_F90_pu)
       {
         Append_Token_String(tokens, "ichar");
         Append_Token_Special(tokens, '(');
       }
# endif 

       WN2F_String_Argument(tokens, wn, WN2F_INTCONST_ONE, context);
# if 0

       if (!WN2F_F90_pu)
         Append_Token_Special(tokens, ')');
# endif

     }
     else /* A regular array access */
     {
           /* Get the base of the object to be indexed into, still using
            * WN2F_CONTEXT_deref_addr(context).
            */
       WN2F_translate(tokens, kid, context);
       reset_WN2F_CONTEXT_deref_addr(context);

   if ( WN2F_CONTEXT_has_no_arr_elmt(context))
            ;
   else
         WN2F_arrsection_bounds(tokens,wn,array_ty,context);
     }
   }
   return EMPTY_WN2F_STATUS;
} /* WN2F_arrsection */


WN2F_STATUS
WN2F_where(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
{
WN  *kid;
 WN2F_Stmt_Newline(tokens, NULL/*label*/, WN_linenum(wn), context);
 Append_Token_String(tokens,"WHERE");
 Append_Token_Special(tokens, '(');
  kid  =WN_kid0(wn);
 WN2F_translate(tokens, kid, context);
 Append_Token_Special(tokens, ')');
 kid   =WN_kid1(wn);
 WN2F_translate(tokens, kid, context);
 kid   = WN_kid2(wn);
 WN2F_Stmt_Newline(tokens, NULL/*label*/, WN_Get_Linenum(wn), context);
 Append_Token_String(tokens,"END WHERE");
 WN2F_translate(tokens, kid, context);
 return EMPTY_WN2F_STATUS;
}


WN2F_STATUS
WN2F_array(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
{
   /* Note that array indices have been normalized to assume the
    * array is based at index zero.  Since a base at index 1 is
    * the default for Fortran, we denormalize to base 1 here.
    */
   BOOL  deref = WN2F_CONTEXT_deref_addr(context);

   WN    * kid;
   TY_IDX ptr_ty;
   TY_IDX array_ty;

   ASSERT_DBG_FATAL(WN_opc_operator(wn) == OPR_ARRAY, 
		    (DIAG_W2F_UNEXPECTED_OPC, "WN2F_array"));

   /* Only allow taking the address of an array element for F90!
    *
    */
#if 0
 else
      ASSERT_DBG_WARN(deref,
		      (DIAG_UNIMPLEMENTED, 
		       "taking the address of an array element"));
#endif

   /* Get the array or, for ptr-as-array types, the element type */

   kid    = WN_kid0(wn);

   if (WN_operator(kid)==OPR_ILOAD &&
          WN_operator(WN_kid0(kid))==OPR_STRCTFLD ) //F90 pointer
        kid = WN_kid0(kid);

   ptr_ty = WN_Tree_Type(kid);

   if (WN2F_Is_Address_Preg(kid,ptr_ty))
   {
       /* a preg or sym has been used as an address, usually after optimization      */
       /* don't know base type, or anything else so use OPR_ARRAY to generate bounds */
     WN2F_translate(tokens, kid, context);
     WN2F_Array_Slots(tokens,wn, ptr_ty, context,TRUE);     
/* need to take example to see if it's OK to use ptr_ty here*/
   } 
   else 
   {
     array_ty = W2F_TY_pointed(ptr_ty, "base of OPC_ARRAY");

     if (WN_opc_operator(kid) == OPR_LDID       &&
	 ST_sclass(WN_st(kid)) == SCLASS_FORMAL &&
	 !ST_is_value_parm(WN_st(kid))          &&
	 WN_element_size(wn) == TY_size(array_ty)       &&
	 WN_num_dim(wn) == 1                            &&
	 WN_opc_operator(WN_array_index(wn, 0)) == OPR_INTCONST &&
	 WN_const_val(WN_array_index(wn, 0)) == 0       &&
	 !TY_ptr_as_array(Ty_Table[WN_ty(kid)])           &&
	 (!TY_Is_Array(array_ty) || 
	  TY_size(TY_AR_etype(array_ty)) < TY_size(array_ty)))
     {
	 /* This array access is just a weird representation for an implicit
	  * reference parameter dereference.  Ignore the array indexing.
	  */
       WN2F_translate(tokens, kid, context);
     }
     else if (!TY_ptr_as_array(Ty_Table[ptr_ty]) && 
                    TY_Is_Character_String(array_ty) )
     {
	 /* We assume that substring accesses are treated in the handling
	  * of intrinsic functions, except when the substrings are to be
	  * handled as integral types and thus are encountered here.
	  */
# if 0
       if (!WN2F_F90_pu)
       {
	 Append_Token_String(tokens, "ichar");
	 Append_Token_Special(tokens, '(');
       }
# endif

       WN2F_String_Argument(tokens, wn, WN2F_INTCONST_ONE, context);

# if 0
       if (!WN2F_F90_pu)
	 Append_Token_Special(tokens, ')');
# endif

     }
     else /* A regular array access */
     {
	   /* Get the base of the object to be indexed into, still using
	    * WN2F_CONTEXT_deref_addr(context).
	    */
//     if (WN_operator(kid) == OPR_ADD || WN_operator(kid)==OPR_ARRAY)
     if (WN_operator(kid) == OPR_ADD)
      {

     STAB_OFFSET offset =(WN_operator(kid) == OPR_ADD)?WN_const_val(WN_kid1(kid)):0; 

       WN2F_translate(tokens,WN_kid0(kid),context);

#if 0 
       FLD_PATH_INFO *fld_path;
       if (!fld_type_z)
            fld_type_z = array_ty; 
       fld_path = TY2F_Get_Fld_Path(fld_type_z,fld_type_z, offset);

       if (fld_path){
           TY2F_Fld_Separator(tokens);
           FLD_HANDLE f (fld_path->fld);
            fld_type_z = FLD_type(f); 

            while (TY_Is_Pointer(fld_type_z))
               fld_type_z = TY_pointed(fld_type_z);
       
            if (TY_kind(fld_type_z) == KIND_ARRAY)
                 fld_type_z = TY_etype(fld_type_z);

            Append_Token_String(tokens,
                           TY2F_Fld_Name(f,FALSE,FALSE));
           
         } else
          Append_Token_String(tokens,
                               Number_as_String(offset,
                                                "<field-at-offset=%lld>"));
#endif 

        }
     else {
           WN2F_translate(tokens, kid, context);
          }

       reset_WN2F_CONTEXT_deref_addr(context);
       WN2F_array_bounds(tokens,wn,array_ty,context);
     }
   }
   return EMPTY_WN2F_STATUS;
} /* WN2F_array */


void
WN2F_Arrsection_Slots(TOKEN_BUFFER tokens, WN *wn,TY_IDX array_ty,WN2F_CONTEXT context,BOOL parens)
{
  INT32 dim;
  WN * kidz;
  INT32 co_dim;
  INT32 array_dim;
  TY_IDX ttyy;
  ARB_HANDLE arb_base;
   WN* kid;


  /* Gets bounds from the slots of an OPC_ARRSECTION node  */

  /* Append the "denormalized" indexing expressions in reverse order
   * of the way they occur in the indexing expression, since Fortran
   * employs column-major array layout, meaning the leftmost indexing
   * expression represents array elements laid out in contiguous
   * memory locations.
   */

  ttyy = array_ty;

  if (TY_Is_Pointer(ttyy))  
     ttyy=TY_pointed(ttyy);
  if (TY_is_f90_pointer(ttyy))
     ttyy = TY_pointed(ttyy);

   arb_base = TY_arb(ttyy);

  dim =  ARB_dimension(arb_base);
  co_dim = ARB_co_dimension(arb_base);


  if (dim >  WN_num_dim(wn) ) {

     array_dim = dim-co_dim;
     co_dim = 0;
 }
 else {
         dim =  WN_num_dim(wn);
        array_dim = dim;
       }


if (array_dim>0) {
  if (parens)
  {
    Append_Token_Special(tokens, '(');
    set_WN2F_CONTEXT_no_parenthesis(context);
  }
# if 0 /* add co_array dimensions */
  for (dim = WN_num_dim(wn)-1; dim >= 0; dim--)
  {
    if (WN_opc_operator(WN_array_index(wn, dim))==OPR_SRCTRIPLET) {
          WN2F_translate(tokens, WN_array_index(wn, dim), context);    
      } 
    else {
    (void)WN2F_Denormalize_Array_Idx(tokens,
                                     WN_array_index(wn, dim),
                                     context);
    }
    if (dim > 0)
      Append_Token_Special(tokens, ',');
  }
# endif
  for (dim = WN_num_dim(wn)-1; dim >= co_dim; dim--)
  {
    if (WN_opc_operator(WN_array_index(wn, dim))==OPR_SRCTRIPLET) {
          WN2F_translate(tokens, WN_array_index(wn, dim), context);
      }
    else {
    (void)WN2F_Denormalize_Array_Idx(tokens,
                                     WN_array_index(wn, dim),
                                     context);
    }
    if (dim > co_dim)
      Append_Token_Special(tokens, ',');
  }
  if (parens)
    Append_Token_Special(tokens, ')');
}

if (co_dim > 0) {

    if (parens)
    Append_Token_Special(tokens, '[');

  for (dim = co_dim-1; dim >= 0; dim--)
  {
    if (WN_opc_operator(WN_array_index(wn, dim))==OPR_SRCTRIPLET) {
          WN2F_translate(tokens, WN_array_index(wn, dim), context);
      }
    else {
    (void)WN2F_Denormalize_Array_Idx(tokens,
                                     WN_array_index(wn, dim),
                                     context);
    }
    if (dim > 0)
      Append_Token_Special(tokens, ',');
  }


  if (parens)
    Append_Token_Special(tokens, ']');
 }
}

void
WN2F_Array_Slots(TOKEN_BUFFER tokens, WN *wn,TY_IDX array_ty,WN2F_CONTEXT context,BOOL parens)
{
  INT32 dim;
  WN * kid;
  INT32 co_dim;
  INT32 array_dim;
  ARB_HANDLE arb_base;
  TY_IDX ttyy;


  /* get array's rank and co_rank information from kid0 of wn
   * kid0 should be OPR_LDA
   *coarray reference is legal without co_rank
   *so we have to use dim plus kid_number to 
   *see if there is co_rank or not
   */

  kid = WN_kid0(wn);

  if (WN_operator(kid)==OPR_LDA)
    {
      ttyy = array_ty;

      if (TY_Is_Pointer(ttyy))
          ttyy =TY_pointed(ttyy);
      if (TY_is_f90_pointer(ttyy))
          ttyy = TY_pointed(ttyy);

      arb_base = TY_arb(ttyy);

     dim =  ARB_dimension(arb_base);
     co_dim = ARB_co_dimension(arb_base);
    } else {
     co_dim =0;
     dim = WN_num_dim(wn);
    }


   if (dim >  WN_num_dim(wn) ) {
      array_dim = dim-co_dim;
      co_dim = 0;
     }
    else {
         dim =  WN_num_dim(wn);
         array_dim = dim-co_dim;
       }



  /* Gets bounds from the slots of an OPC_ARRAY node  */

  /* Append the "denormalized" indexing expressions in reverse order
   * of the way they occur in the indexing expression, since Fortran
   * employs column-major array layout, meaning the leftmost indexing
   * expression represents array elements laid out in contiguous 
   * memory locations.
   */

 if (array_dim > 0 ) {
    Append_Token_Special(tokens, '(');
    set_WN2F_CONTEXT_no_parenthesis(context);


  for (dim =  WN_num_dim(wn)-1; dim >= co_dim; dim--)
  {
    (void)WN2F_Denormalize_Array_Idx(tokens,
                                     WN_array_index(wn, dim),
                                     context);
       
    if (dim > co_dim)
      Append_Token_Special(tokens, ',');
  }


    Append_Token_Special(tokens, ')');
  } 

  /* for co_rank */

  if (co_dim > 0) {
     Append_Token_Special(tokens, '[');
     for (dim =  co_dim-1; dim >= 0; dim--)
     {
       (void)WN2F_Denormalize_Array_Idx(tokens,
                                        WN_array_index(wn, dim),
                                        context);
       
       if (dim > 0)
       Append_Token_Special(tokens, ',');
    }

    Append_Token_Special(tokens, ']');
 
    }
 }

void
WN2F_arrsection_bounds(TOKEN_BUFFER tokens, WN *wn, TY_IDX array_ty,WN2F_CONTEXT context)
{
  /* This prints the array subscript expression. It was part of
   * WN2F_array, but was split so it could be used for bounds
   * of structure components.
   */

  INT32 dim;

  if (TY_is_f90_pointer(array_ty))
       array_ty = TY_pointed(array_ty);//Sept

  if (TY_Is_Array(array_ty) && TY_AR_ndims(array_ty) >= WN_num_dim(wn))
    {
      /* Cannot currently handle differing element sizes at place of
       * array declaration versus place of array access (TODO?).
       */

      ASSERT_DBG_WARN((TY_size(TY_AR_etype(array_ty)) == WN_element_size(wn)) ||
                      WN_element_size(wn) < 0 ||
                      TY_size(TY_AR_etype(array_ty)) == 0,
                      (DIAG_UNIMPLEMENTED,
                       "access/declaration mismatch in array element size"));

      WN2F_Arrsection_Slots(tokens,wn,array_ty,context,TRUE);

      /* We handle the case when an array is declared to have more
       * dimensions than that given by this array addressing expression.
       */
# if 0 //could be co_array object

      if (TY_AR_ndims(array_ty) > WN_num_dim(wn))
        {
          /* Substitute in '1' for the missing dimensions */
          for (dim = TY_AR_ndims(array_ty) - WN_num_dim(wn); dim > 0; dim--)
            {
              Append_Token_Special(tokens, ',');
              Append_Token_String(tokens, "1");
            }
        }
# endif
    }
  else /* Normalize array access to assume a single dimension */
    {
      ASSERT_DBG_WARN(!TY_Is_Array(array_ty) || TY_AR_ndims(array_ty) == 1,
                      (DIAG_UNIMPLEMENTED,
                       "access/declaration mismatch in array dimensions"));

      WN2F_Normalize_Idx_To_Onedim(tokens, wn, context);
    }

}


void
WN2F_array_bounds(TOKEN_BUFFER tokens, WN *wn, TY_IDX array_ty,WN2F_CONTEXT context)
{
  /* This prints the array subscript expression. It was part of
   * WN2F_array, but was split so it could be used for bounds 
   * of structure components.
   */
 
  INT32 dim;
  WN  * kid; 

//   Append_Token_Special(tokens, '(');
//  set_WN2F_CONTEXT_no_parenthesis(context);

   if (TY_is_f90_pointer(array_ty))
        array_ty = TY_pointed(array_ty); 

  if (TY_Is_Array(array_ty) && TY_AR_ndims(array_ty) >= WN_num_dim(wn) || TRUE) 
    {
      /* Cannot currently handle differing element sizes at place of
       * array declaration versus place of array access (TODO?).
       */

      ASSERT_DBG_WARN((TY_size(TY_AR_etype(array_ty)) == WN_element_size(wn)) ||
		      WN_element_size(wn) < 0 ||
		      TY_size(TY_AR_etype(array_ty)) == 0,
		      (DIAG_UNIMPLEMENTED, 
		       "access/declaration mismatch in array element size"));

      WN2F_Array_Slots(tokens,wn,array_ty,context,FALSE);

      /* We handle the case when an array is declared to have more 
       * dimensions than that given by this array addressing expression.
       * COMMENT ABOVE IS FROM ORIGINAL VERSION ,belowing added by zhao
       * this could be happend when co_rank doesn't appear,don't need add
       * 
       */
#if 0 
      if (TY_AR_ndims(array_ty) > WN_num_dim(wn))
	{
     
	  /* Substitute in '1' for the missing dimensions */
	  for (dim = TY_AR_ndims(array_ty) - WN_num_dim(wn); dim > 0; dim--)
	    {
	      Append_Token_Special(tokens, ',');
	      Append_Token_String(tokens, "1");
	    }
	}
#endif

    }
  else /* Normalize array access to assume a single dimension */
    {
      ASSERT_DBG_WARN(!TY_Is_Array(array_ty) || TY_AR_ndims(array_ty) == 1,
		      (DIAG_UNIMPLEMENTED, 
		       "access/declaration mismatch in array dimensions"));

     WN2F_Normalize_Idx_To_Onedim(tokens, wn, context);
    }
//   Append_Token_Special(tokens, ')');
}

/*----------- Character String Manipulation Translation ---------------*/
/*---------------------------------------------------------------------*/

void
WN2F_String_Argument(TOKEN_BUFFER  tokens,
		     WN           *base_parm,
		     WN           *length,
		     WN2F_CONTEXT  context)
{
   /* Append the tokens denoting the substring expression represented
    * by the base-expression.
    *
    * There are two possibilities concerning the array base 
    * expressions.  It can be a pointer to a complete character-
    * string (array) or it can be a pointer to a character within 
    * a character-string (single character).  In the first instance,
    * the offset off the base of string is zero.  In the latter 
    * case, the offset is given by the array indexing operation.
    *
    * NOTE: In some cases (notably for IO_ITEMs), we may try to 
    * derive a substring off an OPC_VINTRINSIC_CALL node or a
    * VCALL node.  This should only happend when the returned value
    * is the first argument and the length is the second argument.
    */
   WN   *base = WN_Skip_Parm(base_parm);
   WN   *lower_bnd;
   WN   *length_new;
   WN   *arg_expr;
   TY_IDX str_ty;
   INT64 str_length;

   /* Skip any INTR_ADRTMP and INTR_VALTMP nodes */
   if (WN_opc_operator(base) == OPR_INTRINSIC_OP &&
       (INTR_is_adrtmp(WN_intrinsic(base)) || 
	INTR_is_valtmp(WN_intrinsic(base))))
   {
      base = WN_kid0(base);
   }


   if (WN_operator(base) == OPR_CVTL)  
   {
     /* probably CHAR(INT) within IO stmt. convert via CHAR & process rest elsewhere */

      Append_Token_Special(tokens, '(');
      Append_Token_String(tokens, "char");
      WN2F_translate(tokens,WN_kid0(base),context);
      Append_Token_Special(tokens, ')');
      return;
   }
  
 
   /* Handle VCALLs specially, since the string information is given
    * by the first two arguments to the call.  Note that we can 
    * always assume a lower bound of zero for these, as we never 
    * generate code for the return-address.  This should only occur
    * within an IO stmt.  Note that the type of VCALLs must be 
    * accessed in the context of an ADRTMP or VALTMP.
    */
   if (WN_opcode(base) == OPC_VCALL ||
       WN_opcode(base) == OPC_VINTRINSIC_CALL)
   {
      arg_expr  = WN_Skip_Parm(WN_kid1(base));
      lower_bnd = WN2F_INTCONST_ZERO;

      /* fixed size string? */

      if (WN_opc_operator(arg_expr) == OPR_INTCONST)
	 str_length = WN_const_val(arg_expr);
      else
	 str_length = -1 ;  

      set_WN2F_CONTEXT_deref_addr(context);
      WN2F_translate(tokens, base, context);
      reset_WN2F_CONTEXT_deref_addr(context);

   } 
   else 
   {
     /* A regular address expression as base */
#if 0
      WN2F_Get_Substring_Info(&base, &str_ty, &lower_bnd,&length_new);

      /* Was this a character component of an array of derived type? */
      /* eg: vvv(2)%ccc(:)(1:5) - offset to ccc is added above base, */
      /* ADD(8,ARRAY(2,LDA VVV)) with array section for CCC on top   */
      /* of the ADD, and the substring above the array section. Take */
      /* the substring off the top, and process the rest             */

      if (TY_kind(str_ty) == KIND_STRUCT) 
      {
	FLD_PATH_INFO *fld_path ;
	FLD_HANDLE fld;
	TY_IDX  ty_idx ; 

	TY & ty = New_TY(ty_idx);

	TY_Init (ty, 1, KIND_SCALAR, MTYPE_U1, Save_Str(".w2fch."));
	Set_TY_is_character(ty);

	fld_path = TY2F_Get_Fld_Path(str_ty, 
				     ty_idx,
				     WN2F_Sum_Offsets(base));

        fld = TY2F_Last_Fld(fld_path);
	TY2F_Free_Fld_Path(fld_path);

	/* call memref for FLD offset, otherwise the ADD is */
	/* just another binary op                           */
	WN2F_Offset_Memref(tokens, 
			   WN_kid0(base),
			   WN_Tree_Type(base),
                           FLD_type(fld),
			   0,
			   context);
      } 
      else 
      {
	str_length = TY_size(str_ty);

        /* with optimization, may not have useful address TY 
         * when TreeType will return array of U1 from SubstringInfo */

	ASSERT_DBG_WARN(TY_Is_Character_String(str_ty) || TY_Is_Array_Of_UChars(str_ty),
			(DIAG_W2F_EXPECTED_PTR_TO_CHARACTER,
			 "WN2F_String_Argument"));

	/* Get the string base and substring notation for the argument.  */
	set_WN2F_CONTEXT_deref_addr(context);
	WN2F_translate(tokens, base, context);
	reset_WN2F_CONTEXT_deref_addr(context);
      }

if (length_new != WN2F_INTCONST_ZERO && !WN2F_CONTEXT_has_no_arr_elmt(context))
      WN2F_Substring(tokens, 
		     str_length,
		     lower_bnd,
//		     WN_Skip_Parm(length),
		     length_new,
		     context);
#else

  {
    WN * base1;
       if (WN_operator(base)==OPR_ARRAY)
               base1 = WN_kid0(base);
       else 
       if (WN_operator(base)==OPR_ARRAYEXP   &&
           WN_operator(WN_kid0(base))==OPR_ARRAY)
               base1 = WN_kid0(WN_kid0(base));
       else 
               base1 = base;

	WN2F_translate(tokens, base1, context);
   } 

        WN2F_Get_Substring_Info(&base, &str_ty, &lower_bnd,&length_new);
        str_length = TY_size(str_ty);

   if (length_new != WN2F_INTCONST_ZERO && !WN2F_CONTEXT_has_no_arr_elmt(context))
          WN2F_Substring(tokens, 
		         str_length,
		         lower_bnd,
		         length_new,
		         context);
   else 
        reset_WN2F_CONTEXT_has_no_arr_elmt(context);
#endif
      return ;
   }
} /* WN2F_String_Argument */


/*----------- Miscellaneous  routines ---------------------------------*/
/*---------------------------------------------------------------------*/

static void
WN2F_Block(TOKEN_BUFFER tokens, ST * st, STAB_OFFSET offset,WN2F_CONTEXT context)
{
  /* An ST of CLASS_BLOCK may appear in f90 IO, at -O2 */
  /* put out something for the whirl browser           */

  ST2F_use_translate(tokens,st);

  if (offset != 0)
  {
      Append_Token_Special(tokens, '+');
      Append_Token_String(tokens, Number_as_String(offset, "%lld"));
  }
}


WN2F_STATUS 
WN2F_strctfld(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
{
   TY_IDX ty1,ty2;
   char * fld_name;
   FLD_HANDLE  fld;
   UINT       field_id ;

   ASSERT_DBG_FATAL(WN_opc_operator(wn) == OPR_STRCTFLD, 
		    (DIAG_W2F_UNEXPECTED_OPC, "WN2F_strctfld"));

 if (!WN_kid0(wn))
    Append_Token_String(tokens,"Null kid here");
 else 
    WN2F_translate(tokens,WN_kid0(wn),context);

    ty2 = WN_load_addr_ty(wn);
    Is_True (TY_kind(ty2) == KIND_STRUCT, ("expecting KIND_STRUCT"));
    field_id = WN_field_id(wn);
    fld = TY_fld(ty2); 
    field_id--;
    while (field_id && !FLD_last_field(fld)) {
        --field_id ;
        fld = FLD_next(fld);
    }
   
//TODO: add assertion about field_id ? Must be resonable value based
//      on the structure and field type 
    fld_name = FLD_name(fld);
    Append_Token_Special(tokens,'%'); 
    Append_Token_String(tokens,fld_name); 

   return EMPTY_WN2F_STATUS;
}


WN2F_STATUS 
WN2F_comma(TOKEN_BUFFER tokens, WN *wn, WN2F_CONTEXT context)
 {
    WN2F_translate(tokens,WN_kid1(wn),context);
    return EMPTY_WN2F_STATUS;
 }
