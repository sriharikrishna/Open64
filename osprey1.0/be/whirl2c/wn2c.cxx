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
 * Module: wn2c.c
 * $Revision: 1.22 $
 * $Date: 2004-10-12 15:23:29 $
 * $Author: fzhao $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/whirl2c/wn2c.cxx,v $
 *
 * Revision history:
 *  07-Oct-94 - Original Version
 *
 * Description:
 *
 *   Translate a WN subtree to C by means of an inorder recursive
 *   descent traversal of the WHIRL IR.
 *
 *   Handle declarations, variable references, type-casts, and
 *   constants by using st2c.h, ty2c.h, and tcon2c.h respectively.
 *
 * ====================================================================
 * ====================================================================
 */

#ifdef _KEEP_RCS_ID
static char *rcs_id = "$Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/whirl2c/wn2c.cxx,v $ $Revision: 1.22 $";
#endif /* _KEEP_RCS_ID */


#include <alloca.h>
#include "whirl2c_common.h"
#include "mempool.h"
#include "const.h"
#include "pf_cg.h"
#include "region_util.h"
#include "w2cf_parentize.h"
#include "PUinfo.h"
#include "wn2c.h"
#include "wn2c_pragma.h"
#include "st2c.h"
#include "ty2c.h"
#include "tcon2c.h"
#include "wn2c_pragma.h"
#include "unparse_target.h"

#if defined(__GNUC__) && (__GNUC__ == 3) 
# define USING_HASH_SET 1
# include <ext/hash_set>
  using namespace __gnu_cxx;
#elif defined(__sgi) && !defined(__GNUC__)
# define USING_HASH_SET 1
# include <hash_set>
#else
# include <set>
#endif

#define WN_pragma_nest(wn) WN_pragma_arg1(wn)

/*-------------------- Some general purpose macros --------------------*/
/*---------------------------------------------------------------------*/


/* This determines whether or not we need an explicit cast to convert
 * a value of type t1 to a value of type t2.
 */
#define WN2C_assignment_compatible_types(lhs_ty, rhs_ty) \
   Stab_Assignment_Compatible_Types(lhs_ty, rhs_ty, \
				    FALSE, /*check_quals*/ \
				    FALSE, /*check_scalars*/ \
				    FALSE) /*ptrs_as_scalars*/

#define WN2C_compatible_lvalues(lhs_ty, rhs_ty) \
   Stab_Assignment_Compatible_Types(lhs_ty, rhs_ty, \
				    FALSE, /*check_quals*/ \
				    TRUE,  /*check_scalars*/ \
				    FALSE) /*ptrs_as_scalars*/

#define WN2C_arithmetic_compatible_types(t1, t2) \
   Stab_Identical_Types(t1, t2, \
			FALSE, /*check_quals*/ \
			TRUE,  /*check_scalars*/ \
		        FALSE) /*ptrs_as_scalars*/

#define WN2C_compatible_qualified_types(t1, t2) \
   Stab_Identical_Types(t1, t2, \
			TRUE,  /*check_quals*/ \
			TRUE,  /*check_scalars*/ \
		        FALSE) /*ptrs_as_scalars*/

#define WN2C_array_lvalue_as_ptr(ptr_to_array, ptr) \
   (TY_Is_Pointer(ptr)                    && \
    TY_Is_Pointer(ptr_to_array)           && \
    TY_Is_Array(TY_pointed(ptr_to_array)) && \
    WN2C_arithmetic_compatible_types(TY_AR_etype(TY_pointed(ptr_to_array)), \
				     TY_pointed(ptr)))

static BOOL
WN2C_is_pointer_diff(OPCODE op, const WN *kid0, const WN *kid1)
{
   TY_IDX ty0, ty1;
   BOOL  is_pointer_diff = FALSE;

   if (OPCODE_operator(op) == OPR_ASHR  &&
       WN_operator(kid0) == OPR_SUB &&
       WN_operator(kid1) == OPR_INTCONST)
   {
      ty0 = WN_Tree_Type(WN_kid0(kid0));
      ty1 = WN_Tree_Type(WN_kid1(kid0));

      if (TY_Is_Pointer(ty0) && TY_Is_Pointer(ty1) &&
	  TY_size(TY_pointed(ty0)) == TY_size(TY_pointed(ty1)) &&
	  TY_size(TY_pointed(ty0)) >> WN_const_val(kid1) == 1)
	 is_pointer_diff = TRUE;
   }
   return is_pointer_diff;
} /* WN2C_is_pointer_diff */


/*---------------- General Purpose static variables -------------------*/
/*---------------------------------------------------------------------*/

/* The name of a temporary automatic variable used to hold function
 * return values.
 */
static const char WN2C_Purple_Region_Name[] = "prp___region";
static const char WN2C_Return_Value_Name[] = "_RetVal";
static BOOL       WN2C_Used_Return_Value = FALSE;


/*------------- Information about call/return sites -------------------*/
/*---------------------------------------------------------------------*/

/* Lists of return and call sites for the current PU,
 * initialized by means of "PUinfo.h" facilities.
 */
static const RETURNSITE *WN2C_Next_ReturnSite = NULL;
static const CALLSITE   *WN2C_Prev_CallSite = NULL;

typedef struct SCALAR_C_NAME
{
   const char *real_name;
   const char *pseudo_name;
} SCALAR_C_NAME;


/* This name is used to wrap a union around a block containing
 * equivalence fields.  Should only occur when translating
 * Fortran to C.
 */
const char TY2C_Aligned_Block_Name[] = "__block";

#define MTYPE_PREDEF MTYPE_F16

static char Name_Unknown_Type[] = "__UNKNOWN_TYPE";
static const SCALAR_C_NAME Scalar_C_Names[MTYPE_PREDEF+1] =
   {{"void",               Name_Unknown_Type},  /* MTYPE_UNKNOWN 0 */
    {"char",               "_BOOLEAN"},         /* MTYPE_B = 1 */
    {"signed char",        "_INT8"},            /* MTYPE_I1 = 2 */
    {"signed short",       "_INT16"},           /* MTYPE_I2 = 3 */
    {"signed int",         "_INT32"},           /* MTYPE_I4 = 4 */
    {"signed long long",   "_INT64"},           /* MTYPE_I8 = 5 */
    {"unsigned char",      "_UINT8"},           /* MTYPE_U1 = 6 */
    {"unsigned short",     "_UINT16"},          /* MTYPE_U2 = 7 */
    {"unsigned int",       "_UINT32"},          /* MTYPE_U4 = 8 */
    {"unsigned long long", "_UINT64"},          /* MTYPE_U8 = 9 */
    {"float",              "_IEEE32"},          /* MTYPE_F4 = 10 */
    {"double",             "_IEEE64"},          /* MTYPE_F8 = 11 */
    {Name_Unknown_Type,    "_IEEE80"},          /* MTYPE_F10 = 12 */
    {Name_Unknown_Type,    "_IEEE128"}  /* MTYPE_F16 = 13 = MTYPE_PREDEF */
   }; /* Scalar_C_Names */


static BOOL 
WN2C_Skip_Stmt(const WN *stmt)
{
   return ((W2C_No_Pragmas && \
            (WN_operator(stmt) == OPR_PRAGMA || 
             WN_operator(stmt) == OPR_XPRAGMA) &&
	    WN_pragma(stmt) != WN_PRAGMA_PREAMBLE_END) || /* For purple */\

           WN2C_Skip_Pragma_Stmt(stmt) ||

           (!W2C_Emit_Prefetch &&
	    (WN_operator(stmt) == OPR_PREFETCH ||
	     WN_operator(stmt) == OPR_PREFETCHX)) ||

	   (WN2C_Next_ReturnSite != NULL &&
	    (stmt == RETURNSITE_store1(WN2C_Next_ReturnSite) ||
	     stmt == RETURNSITE_store2(WN2C_Next_ReturnSite))) ||

	   (WN2C_Prev_CallSite != NULL &&
	    (stmt == CALLSITE_store1(WN2C_Prev_CallSite) ||
	     stmt == CALLSITE_store2(WN2C_Prev_CallSite))) ||

	   (WN_operator(stmt) == OPR_COMMENT && /* special comment */
	    strcmp(Index_To_Str(WN_GetComment(stmt)), "ENDLOOP") == 0)
	   );
} /* WN2C_Skip_Stmt */


/*-------------------- Function handle for each OPR -------------------*/
/*---------------------------------------------------------------------*/

/* Forward declaration of functions to translate WHIRL to C.
*/
static STATUS WN2C_ignore(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_unsupported(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_binaryop(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_unaryop(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_func_entry(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_block(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_region(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_switch(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_compgoto(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_do_loop(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_do_while(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_while_do(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_if(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_goto(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_condbr(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_return(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_return_val(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_label(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_exc_scope_end(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_exc_scope_begin(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_istore(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_istorex(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_mstore(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_stid(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_call(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_eval(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_prefetch(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_comment(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_iload(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_iloadx(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_mload(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_array(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_intrinsic_op(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_tas(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_select(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_cvt(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_cvtl(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_realpart(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_imagpart(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_paren(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_complex(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_bnor(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_madd(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_msub(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_nmadd(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_nmsub(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_ldid(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_lda(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_const(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_intconst(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_parm(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_comma(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_rcomma(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_alloca(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_dealloca(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_extract_bits(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);
static STATUS WN2C_compose_bits(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context);

typedef STATUS (*WN2C_HANDLER_FUNC)(TOKEN_BUFFER, const WN*, CONTEXT);

/* WN2C_Opr_Handle[] maps an OPR (../common/com/opcode_gen_core.h)
 * to the function that translates it to C.  It is dynamically 
 * initialized in WN2C_initialize(), by means of the table:
 * WN2C_Opr_Handler_Map[].  Operators we cannot handle in whirl2c are
 * handled by WN2C_unsupported().
 */
#define NUMBER_OF_OPERATORS (OPERATOR_LAST + 1)
static WN2C_HANDLER_FUNC WN2C_Opr_Handler[NUMBER_OF_OPERATORS];

typedef struct Opr2handler
{
   OPERATOR           opr;
   WN2C_HANDLER_FUNC  handler;
} OPR2HANDLER;

#define NUMBER_OF_OPR2HANDLER_MAPS \
   (sizeof(WN2C_Opr_Handler_Map) / sizeof(OPR2HANDLER))

static const OPR2HANDLER WN2C_Opr_Handler_Map[] =
{
   {OPR_FUNC_ENTRY, &WN2C_func_entry},
   {OPR_BLOCK, &WN2C_block},
   {OPR_REGION, &WN2C_region},
   {OPR_REGION_EXIT, &WN2C_goto},
   {OPR_COMPGOTO, &WN2C_compgoto},
   {OPR_SWITCH, &WN2C_switch},
   {OPR_DO_LOOP, &WN2C_do_loop},
   {OPR_DO_WHILE, &WN2C_do_while},
   {OPR_WHILE_DO, &WN2C_while_do},
   {OPR_IF, &WN2C_if},
   {OPR_GOTO, &WN2C_goto},
   {OPR_CASEGOTO, &WN2C_goto},
   {OPR_FALSEBR, &WN2C_condbr},
   {OPR_TRUEBR, &WN2C_condbr},
   {OPR_RETURN, &WN2C_return},
   {OPR_RETURN_VAL, &WN2C_return_val},
   {OPR_LABEL, &WN2C_label},
   {OPR_EXC_SCOPE_BEGIN, &WN2C_exc_scope_begin},
   {OPR_EXC_SCOPE_END, &WN2C_exc_scope_end},
   {OPR_ISTORE, &WN2C_istore},
   {OPR_ISTOREX, &WN2C_istorex},
   {OPR_MSTORE, &WN2C_mstore},
   {OPR_STID, &WN2C_stid},
   {OPR_CALL, &WN2C_call},
   {OPR_INTRINSIC_CALL, &WN2C_call},
   {OPR_ICALL, &WN2C_call},
   {OPR_PICCALL, &WN2C_call},
   {OPR_EVAL, &WN2C_eval},
   {OPR_PRAGMA, &WN2C_pragma},
   {OPR_XPRAGMA, &WN2C_pragma},
   {OPR_PREFETCH, &WN2C_prefetch},
   {OPR_PREFETCHX, &WN2C_prefetch},
   {OPR_COMMENT, &WN2C_comment},
   {OPR_ILOAD, &WN2C_iload},
   {OPR_ILOADX, &WN2C_iloadx},
   {OPR_MLOAD, &WN2C_mload},
   {OPR_ARRAY, &WN2C_array},
   {OPR_INTRINSIC_OP, &WN2C_intrinsic_op},
   {OPR_TAS, &WN2C_tas},
   {OPR_SELECT, &WN2C_select},
   {OPR_CSELECT, &WN2C_select},
   {OPR_CVT, &WN2C_cvt},
   {OPR_CVTL, &WN2C_cvtl},
   {OPR_NEG, &WN2C_unaryop},
   {OPR_ABS, &WN2C_unaryop},
   {OPR_SQRT, &WN2C_unaryop},
   {OPR_REALPART, &WN2C_realpart},
   {OPR_IMAGPART, &WN2C_imagpart},
   {OPR_PAREN, &WN2C_paren},
   {OPR_RND, &WN2C_unaryop},
   {OPR_TRUNC, &WN2C_unaryop},
   {OPR_CEIL, &WN2C_unaryop},
   {OPR_FLOOR, &WN2C_unaryop},
   {OPR_BNOT, &WN2C_unaryop},
   {OPR_LNOT, &WN2C_unaryop},
   {OPR_ADD, &WN2C_binaryop},
   {OPR_SUB, &WN2C_binaryop},
   {OPR_MPY, &WN2C_binaryop},
   {OPR_DIV, &WN2C_binaryop},
   {OPR_MOD, &WN2C_binaryop},
   {OPR_REM, &WN2C_binaryop},
   {OPR_MAX, &WN2C_binaryop},
   {OPR_MIN, &WN2C_binaryop},
   {OPR_BAND, &WN2C_binaryop},
   {OPR_BIOR, &WN2C_binaryop},
   {OPR_BNOR, &WN2C_bnor},
   {OPR_BXOR, &WN2C_binaryop},
   {OPR_LAND, &WN2C_binaryop},
   {OPR_LIOR, &WN2C_binaryop},
   {OPR_CAND, &WN2C_binaryop},
   {OPR_CIOR, &WN2C_binaryop},
   {OPR_SHL, &WN2C_binaryop},
   {OPR_ASHR, &WN2C_binaryop},
   {OPR_LSHR, &WN2C_binaryop},
   {OPR_COMPLEX, &WN2C_complex},
   {OPR_RECIP, &WN2C_unaryop},
   {OPR_RSQRT, &WN2C_unaryop},
   {OPR_MADD, &WN2C_madd},
   {OPR_MSUB, &WN2C_msub},
   {OPR_NMADD, &WN2C_nmadd},
   {OPR_NMSUB, &WN2C_nmsub},
   {OPR_EQ, &WN2C_binaryop},
   {OPR_NE, &WN2C_binaryop},
   {OPR_GT, &WN2C_binaryop},
   {OPR_GE, &WN2C_binaryop},
   {OPR_LT, &WN2C_binaryop},
   {OPR_LE, &WN2C_binaryop},
   {OPR_LDID, &WN2C_ldid},
   {OPR_LDA, &WN2C_lda},
   {OPR_CONST, &WN2C_const},
   {OPR_INTCONST, &WN2C_intconst},
   {OPR_PARM, &WN2C_parm},
   {OPR_TRAP, &WN2C_ignore},
   {OPR_ASSERT, &WN2C_ignore},
   {OPR_FORWARD_BARRIER, &WN2C_ignore},
   {OPR_BACKWARD_BARRIER, &WN2C_ignore},
   {OPR_COMMA, &WN2C_comma},
   {OPR_RCOMMA, &WN2C_rcomma},
   {OPR_ALLOCA, &WN2C_alloca},
   {OPR_DEALLOCA, &WN2C_dealloca},
   {OPR_EXTRACT_BITS, &WN2C_extract_bits},
   {OPR_COMPOSE_BITS,&WN2C_compose_bits}
}; /* WN2C_Opr_Handler_Map */


/*------- C names for binary and unary arithmetic operations ----------*/
/*---------------------------------------------------------------------*/


/* Assume the C name for a WHIRL arithmetic/logical operation
 * begins with an underscore ('_') when it is implemented as
 * a function;  otherwise assume we have a builtin infix C
 * operator for the operation.
 */
#define WN2C_IS_INFIX_OP(opc) \
   ((WN2C_Opc2cname[opc]!=NULL)? (WN2C_Opc2cname[opc][0]!='_') : FALSE)

#define WN2C_IS_FUNCALL_OP(opc) \
   ((WN2C_Opc2cname[opc]!=NULL)? (WN2C_Opc2cname[opc][0]=='_') : FALSE)


/* Mapping from opcodes to C names for arithmetic/logical operations.
 * An empty (NULL) C name will occur for non-arithmetic/-logical 
 * opcodes, which must be handled by special handler-functions.
 * This mapping is dynamically initialized, based on 
 * WN2C_Opc2cname_Map[], in WN2C_initialize().
 */
#define NUMBER_OF_OPCODES (OPCODE_LAST+1)
static const char *WN2C_Opc2cname[NUMBER_OF_OPCODES];
   

typedef struct Opc2Cname_Map
{
   OPCODE      opc;
   const char *cname;
} OPC2CNAME_MAP;

#define NUMBER_OF_OPC2CNAME_MAPS \
   sizeof(WN2C_Opc2cname_Map) / sizeof(OPC2CNAME_MAP)
static const OPC2CNAME_MAP WN2C_Opc2cname_Map[] =
{
  {OPC_U8NEG, "-"},
  {OPC_FQNEG, "-"},
  {OPC_I8NEG, "-"},
  {OPC_U4NEG, "-"},
  {OPC_CQNEG, "_CQNEG"},
  {OPC_F8NEG, "-"},
  {OPC_C8NEG, "_C8NEG"},
  {OPC_I4NEG, "-"},
  {OPC_F4NEG, "-"},
  {OPC_C4NEG, "_C4NEG"},
  {OPC_I4ABS, "_I4ABS"},
  {OPC_F4ABS, "_F4ABS"},
  {OPC_FQABS, "_FQABS"},
  {OPC_I8ABS, "_I8ABS"},
  {OPC_F8ABS, "_F8ABS"},
  {OPC_F4SQRT, "_F4SQRT"},
  {OPC_C4SQRT, "_C4SQRT"},
  {OPC_FQSQRT, "_FQSQRT"},
  {OPC_CQSQRT, "_CQSQRT"},
  {OPC_F8SQRT, "_F8SQRT"},
  {OPC_C8SQRT, "_C8SQRT"},
  {OPC_I4F4RND, "_I4F4RND"},
  {OPC_I4FQRND, "_I4FQRND"},
  {OPC_I4F8RND, "_I4F8RND"},
  {OPC_U4F4RND, "_U4F4RND"},
  {OPC_U4FQRND, "_U4FQRND"},
  {OPC_U4F8RND, "_U4F8RND"},
  {OPC_I8F4RND, "_I8F4RND"},
  {OPC_I8FQRND, "_I8FQRND"},
  {OPC_I8F8RND, "_I8F8RND"},
  {OPC_U8F4RND, "_U8F4RND"},
  {OPC_U8FQRND, "_U8FQRND"},
  {OPC_U8F8RND, "_U8F8RND"},
  {OPC_I4F4TRUNC, "_I4F4TRUNC"},
  {OPC_I4FQTRUNC, "_I4FQTRUNC"},
  {OPC_I4F8TRUNC, "_I4F8TRUNC"},
  {OPC_U4F4TRUNC, "_U4F8TRUNC"},
  {OPC_U4FQTRUNC, "_U4F8TRUNC"},
  {OPC_U4F8TRUNC, "_U4F8TRUNC"},
  {OPC_I8F4TRUNC, "_I8F4TRUNC"},
  {OPC_I8FQTRUNC, "_I8FQTRUNC"},
  {OPC_I8F8TRUNC, "_I8F8TRUNC"},
  {OPC_U8F4TRUNC, "_U8F8TRUNC"},
  {OPC_U8FQTRUNC, "_U8F8TRUNC"},
  {OPC_U8F8TRUNC, "_U8F8TRUNC"},
  {OPC_I4F4CEIL, "_I4F4CEIL"},
  {OPC_I4FQCEIL, "_I4FQCEIL"},
  {OPC_I4F8CEIL, "_I4F8CEIL"},
  {OPC_U4F4CEIL, "_U4F8CEIL"},
  {OPC_U4FQCEIL, "_U4F8CEIL"},
  {OPC_U4F8CEIL, "_U4F8CEIL"},
  {OPC_I8F4CEIL, "_I8F4CEIL"},
  {OPC_I8FQCEIL, "_I8FQCEIL"},
  {OPC_I8F8CEIL, "_I8F8CEIL"},
  {OPC_U8F4CEIL, "_U8F4CEIL"},
  {OPC_U8FQCEIL, "_U8FQCEIL"},
  {OPC_U8F8CEIL, "_U8F8CEIL"},
  {OPC_I4F4FLOOR, "_I4F4FLOOR"},
  {OPC_I4FQFLOOR, "_I4FQFLOOR"},
  {OPC_I4F8FLOOR, "_I4F8FLOOR"},
  {OPC_U4F4FLOOR, "_U4F4FLOOR"},
  {OPC_U4FQFLOOR, "_U4FQFLOOR"},
  {OPC_U4F8FLOOR, "_U4F8FLOOR"},
  {OPC_I8F4FLOOR, "_I8F4FLOOR"},
  {OPC_I8FQFLOOR, "_I8FQFLOOR"},
  {OPC_I8F8FLOOR, "_I8F8FLOOR"},
  {OPC_U8F4FLOOR, "_U8F4FLOOR"},
  {OPC_U8FQFLOOR, "_U8FQFLOOR"},
  {OPC_U8F8FLOOR, "_U8F8FLOOR"},
  {OPC_F8F8FLOOR,"_F8F8FLOOR"}, 
  {OPC_I4BNOT, "~"},
  {OPC_U8BNOT, "~"},
  {OPC_I8BNOT, "~"},
  {OPC_U4BNOT, "~"},
// >> WHIRL 0.30: replaced OPC_LNOT by OPC_B and OP_I4 variants
// TODO WHIRL 0.30: get rid of OPC_I4 variants
  {OPC_BLNOT, "!"},
  {OPC_I4LNOT, "!"},
// << WHIRL 0.30: replaced OPC_LNOT by OPC_B and OP_I4 variants
  {OPC_U8ADD, "+"},
  {OPC_FQADD, "+"},
  {OPC_I8ADD, "+"},
  {OPC_U4ADD, "+"},
  {OPC_CQADD, "_CQADD"},
  {OPC_F8ADD, "+"},
  {OPC_C8ADD, "_C8ADD"},
  {OPC_I4ADD, "+"},
  {OPC_F4ADD, "+"},
  {OPC_C4ADD, "_C4ADD"},
  {OPC_U8SUB, "-"},
  {OPC_FQSUB, "-"},
  {OPC_I8SUB, "-"},
  {OPC_U4SUB, "-"},
  {OPC_CQSUB, "_CQSUB"},
  {OPC_F8SUB, "-"},
  {OPC_C8SUB, "_C8SUB"},
  {OPC_I4SUB, "-"},
  {OPC_F4SUB, "-"},
  {OPC_C4SUB, "_C4SUB"},
  {OPC_U8MPY, "*"},
  {OPC_FQMPY, "*"},
  {OPC_I8MPY, "*"},
  {OPC_U4MPY, "*"},
  {OPC_CQMPY, "_CQMPY"},
  {OPC_F8MPY, "*"},
  {OPC_C8MPY, "_C8MPY"},
  {OPC_I4MPY, "*"},
  {OPC_F4MPY, "*"},
  {OPC_C4MPY, "_C4MPY"},
  {OPC_U8DIV, "/"},
  {OPC_FQDIV, "/"},
  {OPC_I8DIV, "/"},
  {OPC_U4DIV, "/"},
  {OPC_CQDIV, "_CQDIV"},
  {OPC_F8DIV, "/"},
  {OPC_C8DIV, "_C8DIV"},
  {OPC_I4DIV, "/"},
  {OPC_F4DIV, "/"},
  {OPC_C4DIV, "_C4DIV"},
  {OPC_I4MOD, "_I4MOD"},
  {OPC_U8MOD, "%"},
  {OPC_I8MOD, "_I8MOD"},
  {OPC_U4MOD, "%"},
  {OPC_I4REM, "%"},
  {OPC_U8REM, "%"},
  {OPC_I8REM, "%"},
  {OPC_U4REM, "%"},
  {OPC_I4MAX, "_I4MAX"},
  {OPC_U8MAX, "_U8MAX"},
  {OPC_F4MAX, "_F4MAX"},
  {OPC_FQMAX, "_FQMAX"},
  {OPC_I8MAX, "_I8MAX"},
  {OPC_U4MAX, "_U4MAX"},
  {OPC_F8MAX, "_F8MAX"},
  {OPC_I4MIN, "_I4MIN"},
  {OPC_U8MIN, "_U8MIN"},
  {OPC_F4MIN, "_F4MIN"},
  {OPC_FQMIN, "_FQMIN"},
  {OPC_I8MIN, "_I8MIN"},
  {OPC_U4MIN, "_U4MIN"},
  {OPC_F8MIN, "_F8MIN"},
  {OPC_I4BAND, "&"},
  {OPC_U8BAND, "&"},
  {OPC_I8BAND, "&"},
  {OPC_U4BAND, "&"},
  {OPC_I4BIOR, "|"},
  {OPC_U8BIOR, "|"},
  {OPC_I8BIOR, "|"},
  {OPC_U4BIOR, "|"},
  {OPC_I4BXOR, "^"},
  {OPC_U8BXOR, "^"},
  {OPC_I8BXOR , "^"},
  {OPC_U4BXOR, "^"},
// >> WHIRL 0.30: replaced OPC_LAND, OPC_LIOR, OPC_CAND, OPC_CIOR by OPC_B and OP_I4 variants
// TODO WHIRL 0.30: get rid of OPC_I4 variants
  {OPC_BLAND, "&&"},
  {OPC_I4LAND, "&&"},
  {OPC_BLIOR, "||"},
  {OPC_I4LIOR, "||"},
  {OPC_BCAND, "&&"},
  {OPC_I4CAND, "&&"},
  {OPC_BCIOR, "||"},
  {OPC_I4CIOR, "||"},
// << WHIRL 0.30: replaced OPC_LAND, OPC_LIOR, OPC_CAND, OPC_CIOR by OPC_B and OP_I4 variants
  {OPC_I4SHL, "_I4SHL"},
  {OPC_U8SHL, "_U8SHL"},
  {OPC_I8SHL, "_I8SHL"},
  {OPC_U4SHL, "_U4SHL"},
  {OPC_I4ASHR, "_I4ASHR"},
  {OPC_U8ASHR, "_U8ASHR"},
  {OPC_I8ASHR, "_I8ASHR"},
  {OPC_U4ASHR, "_U4ASHR"},
  {OPC_I4LSHR, "_I4LSHR"},
  {OPC_U8LSHR, "_U8LSHR"},
  {OPC_I8LSHR, "_I8LSHR"},
  {OPC_U4LSHR, "_U4LSHR"},
  {OPC_F4RECIP, "_F4RECIP"},
  {OPC_C4RECIP, "_C4RECIP"},
  {OPC_FQRECIP, "_FQRECIP"},
  {OPC_CQRECIP, "_CQRECIP"},
  {OPC_F8RECIP, "_F8RECIP"},
  {OPC_C8RECIP, "_C8RECIP"},
  {OPC_F4RSQRT, "_F4RSQRT"},
  {OPC_C4RSQRT, "_C4RSQRT"},
  {OPC_FQRSQRT, "_FQRSQRT"},
  {OPC_CQRSQRT, "_CQRSQRT"},
  {OPC_F8RSQRT, "_F8RSQRT"},
  {OPC_C8RSQRT, "_C8RSQRT"},
// >> WHIRL 0.30: Replaced OPC_T1{EQ,NE,GT,GE,LT,LE} by OP_BT1 and OPC_I4T1 variants
// TODO WHIRL 0.30: get rid of OPC_I4T1 variants
  {OPC_BU8EQ, "=="},
  {OPC_BFQEQ, "=="},
  {OPC_BI8EQ, "=="},
  {OPC_BU4EQ, "=="},
  {OPC_BCQEQ, "=="},
  {OPC_BF8EQ, "=="},
  {OPC_BC8EQ, "=="},
  {OPC_BI4EQ, "=="},
  {OPC_BF4EQ, "=="},
  {OPC_BC4EQ, "=="},
  {OPC_BU8NE, "!="},
  {OPC_BFQNE, "!="},
  {OPC_BI8NE, "!="},
  {OPC_BU4NE, "!="},
  {OPC_BCQNE, "!="},
  {OPC_BF8NE, "!="},
  {OPC_BC8NE, "!="},
  {OPC_BI4NE, "!="},
  {OPC_BF4NE, "!="},
  {OPC_BC4NE, "!="},
  {OPC_BI4GT, ">"},
  {OPC_BU8GT, ">"},
  {OPC_BF4GT, ">"},
  {OPC_BFQGT, ">"},
  {OPC_BI8GT, ">"},
  {OPC_BU4GT, ">"},
  {OPC_BF8GT, ">"},
  {OPC_BI4GE, ">="},
  {OPC_BU8GE, ">="},
  {OPC_BF4GE, ">="},
  {OPC_BFQGE, ">="},
  {OPC_BI8GE, ">="},
  {OPC_BU4GE, ">="},
  {OPC_BF8GE, ">="},
  {OPC_BI4LT, "<"},
  {OPC_BU8LT, "<"},
  {OPC_BF4LT, "<"},
  {OPC_BFQLT, "<"},
  {OPC_BI8LT, "<"},
  {OPC_BU4LT, "<"},
  {OPC_BF8LT, "<"},
  {OPC_BI4LE, "<="},
  {OPC_BU8LE, "<="},
  {OPC_BF4LE, "<="},
  {OPC_BFQLE, "<="},
  {OPC_BI8LE, "<="},
  {OPC_BU4LE, "<="},
  {OPC_BF8LE, "<="},

  {OPC_I4U8EQ, "=="},
  {OPC_I4FQEQ, "=="},
  {OPC_I4I8EQ, "=="},
  {OPC_I4U4EQ, "=="},
  {OPC_I4CQEQ, "=="},
  {OPC_I4F8EQ, "=="},
  {OPC_I4C8EQ, "=="},
  {OPC_I4I4EQ, "=="},
  {OPC_I4F4EQ, "=="},
  {OPC_I4C4EQ, "=="},

  {OPC_I8U8EQ, "=="},
  {OPC_I8FQEQ, "=="},
  {OPC_I8I8EQ, "=="},
  {OPC_I8U4EQ, "=="},
  {OPC_I8CQEQ, "=="},
  {OPC_I8F8EQ, "=="},
  {OPC_I8C8EQ, "=="},
  {OPC_I8I4EQ, "=="},
  {OPC_I8F4EQ, "=="},
  {OPC_I8C4EQ, "=="},

  {OPC_U4U8EQ, "=="},
  {OPC_U4FQEQ, "=="},
  {OPC_U4I8EQ, "=="},
  {OPC_U4U4EQ, "=="},
  {OPC_U4CQEQ, "=="},
  {OPC_U4F8EQ, "=="},
  {OPC_U4C8EQ, "=="},
  {OPC_U4I4EQ, "=="},
  {OPC_U4F4EQ, "=="},
  {OPC_U4C4EQ, "=="},

  {OPC_U8U8EQ, "=="},
  {OPC_U8FQEQ, "=="},
  {OPC_U8I8EQ, "=="},
  {OPC_U8U4EQ, "=="},
  {OPC_U8CQEQ, "=="},
  {OPC_U8F8EQ, "=="},
  {OPC_U8C8EQ, "=="},
  {OPC_U8I4EQ, "=="},
  {OPC_U8F4EQ, "=="},
  {OPC_U8C4EQ, "=="},

  {OPC_I4U8NE, "!="},
  {OPC_I4FQNE, "!="},
  {OPC_I4I8NE, "!="},
  {OPC_I4U4NE, "!="},
  {OPC_I4CQNE, "!="},
  {OPC_I4F8NE, "!="},
  {OPC_I4C8NE, "!="},
  {OPC_I4I4NE, "!="},
  {OPC_I4F4NE, "!="},
  {OPC_I4C4NE, "!="},
 
  {OPC_I8U8NE, "!="},
  {OPC_I8FQNE, "!="},
  {OPC_I8I8NE, "!="},
  {OPC_I8U4NE, "!="},
  {OPC_I8CQNE, "!="},
  {OPC_I8F8NE, "!="},
  {OPC_I8C8NE, "!="},
  {OPC_I8I4NE, "!="},
  {OPC_I8F4NE, "!="},
  {OPC_I8C4NE, "!="},

  {OPC_U4U8NE, "!="},
  {OPC_U4FQNE, "!="},
  {OPC_U4I8NE, "!="},
  {OPC_U4U4NE, "!="},
  {OPC_U4CQNE, "!="},
  {OPC_U4F8NE, "!="},
  {OPC_U4C8NE, "!="},
  {OPC_U4I4NE, "!="},
  {OPC_U4F4NE, "!="},
  {OPC_U4C4NE, "!="},

  {OPC_U8U8NE, "!="},
  {OPC_U8FQNE, "!="},
  {OPC_U8I8NE, "!="},
  {OPC_U8U4NE, "!="},
  {OPC_U8CQNE, "!="},
  {OPC_U8F8NE, "!="},
  {OPC_U8C8NE, "!="},
  {OPC_U8I4NE, "!="},
  {OPC_U8F4NE, "!="},
  {OPC_U8C4NE, "!="},

  {OPC_I4I4GT, ">"},
  {OPC_I4U8GT, ">"},
  {OPC_I4F4GT, ">"},
  {OPC_I4FQGT, ">"},
  {OPC_I4I8GT, ">"},
  {OPC_I4U4GT, ">"},
  {OPC_I4F8GT, ">"},

  {OPC_I8I4GT, ">"},
  {OPC_I8U8GT, ">"},
  {OPC_I8F4GT, ">"},
  {OPC_I8FQGT, ">"},
  {OPC_I8I8GT, ">"},
  {OPC_I8U4GT, ">"},
  {OPC_I8F8GT, ">"},

  {OPC_U4I4GT, ">"},
  {OPC_U4U8GT, ">"},
  {OPC_U4F4GT, ">"},
  {OPC_U4FQGT, ">"},
  {OPC_U4I8GT, ">"},
  {OPC_U4U4GT, ">"},
  {OPC_U4F8GT, ">"},

  {OPC_U8I4GT, ">"},
  {OPC_U8U8GT, ">"},
  {OPC_U8F4GT, ">"},
  {OPC_U8FQGT, ">"},
  {OPC_U8I8GT, ">"},
  {OPC_U8U4GT, ">"},
  {OPC_U8F8GT, ">"},

  {OPC_I4I4GE, ">="},
  {OPC_I4U8GE, ">="},
  {OPC_I4F4GE, ">="},
  {OPC_I4FQGE, ">="},
  {OPC_I4I8GE, ">="},
  {OPC_I4U4GE, ">="},
  {OPC_I4F8GE, ">="},

  {OPC_I8I4GE, ">="},
  {OPC_I8U8GE, ">="},
  {OPC_I8F4GE, ">="},
  {OPC_I8FQGE, ">="},
  {OPC_I8I8GE, ">="},
  {OPC_I8U4GE, ">="},
  {OPC_I8F8GE, ">="},

  {OPC_U4I4GE, ">="},
  {OPC_U4U8GE, ">="},
  {OPC_U4F4GE, ">="},
  {OPC_U4FQGE, ">="},
  {OPC_U4I8GE, ">="},
  {OPC_U4U4GE, ">="},
  {OPC_U4F8GE, ">="},

  {OPC_U8I4GE, ">="},
  {OPC_U8U8GE, ">="},
  {OPC_U8F4GE, ">="},
  {OPC_U8FQGE, ">="},
  {OPC_U8I8GE, ">="},
  {OPC_U8U4GE, ">="},
  {OPC_U8F8GE, ">="},

  {OPC_I4I4LT, "<"},
  {OPC_I4U8LT, "<"},
  {OPC_I4F4LT, "<"},
  {OPC_I4FQLT, "<"},
  {OPC_I4I8LT, "<"},
  {OPC_I4U4LT, "<"},
  {OPC_I4F8LT, "<"},

  {OPC_I8I4LT, "<"},
  {OPC_I8U8LT, "<"},
  {OPC_I8F4LT, "<"},
  {OPC_I8FQLT, "<"},
  {OPC_I8I8LT, "<"},
  {OPC_I8U4LT, "<"},
  {OPC_I8F8LT, "<"},

  {OPC_U4I4LT, "<"},
  {OPC_U4U8LT, "<"},
  {OPC_U4F4LT, "<"},
  {OPC_U4FQLT, "<"},
  {OPC_U4I8LT, "<"},
  {OPC_U4U4LT, "<"},
  {OPC_U4F8LT, "<"},

  {OPC_U8I4LT, "<"},
  {OPC_U8U8LT, "<"},
  {OPC_U8F4LT, "<"},
  {OPC_U8FQLT, "<"},
  {OPC_U8I8LT, "<"},
  {OPC_U8U4LT, "<"},
  {OPC_U8F8LT, "<"},

  {OPC_I4I4LE, "<="},
  {OPC_I4U8LE, "<="},
  {OPC_I4F4LE, "<="},
  {OPC_I4FQLE, "<="},
  {OPC_I4I8LE, "<="},
  {OPC_I4U4LE, "<="},
  {OPC_I4F8LE, "<="},

  {OPC_I8I4LE, "<="},
  {OPC_I8U8LE, "<="},
  {OPC_I8F4LE, "<="},
  {OPC_I8FQLE, "<="},
  {OPC_I8I8LE, "<="},
  {OPC_I8U4LE, "<="},
  {OPC_I8F8LE, "<="},

  {OPC_U4I4LE, "<="},
  {OPC_U4U8LE, "<="},
  {OPC_U4F4LE, "<="},
  {OPC_U4FQLE, "<="},
  {OPC_U4I8LE, "<="},
  {OPC_U4U4LE, "<="},
  {OPC_U4F8LE, "<="},

  {OPC_U8I4LE, "<="},
  {OPC_U8U8LE, "<="},
  {OPC_U8F4LE, "<="},
  {OPC_U8FQLE, "<="},
  {OPC_U8I8LE, "<="},
  {OPC_U8U4LE, "<="},
  {OPC_U8F8LE, "<="}


// << WHIRL 0.30: Replaced OPC_T1{EQ,NE,GT,GE,LT,LE} by OP_BT1 and OPC_I4T1 variants
}; /* WN2C_Opc2Cname_Map */

/*------------------ Statement newline directives ----------------------*/
/*----------------------------------------------------------------------*/

static void 
WN2C_Stmt_Newline(TOKEN_BUFFER tokens,
		  SRCPOS       srcpos)
{
   if (W2C_Emit_Linedirs)
      Append_Srcpos_Directive(tokens, srcpos);
   Append_Indented_Newline(tokens, 1);
   if (W2C_File[W2C_LOC_FILE] != NULL)
      Append_Srcpos_Map(tokens, srcpos);
} /* WN2C_Stmt_Newline */


/*--------------------- hidden utility routines -----------------------*/
/*---------------------------------------------------------------------*/

static STATUS
WN2C_lvalue_wn(TOKEN_BUFFER tokens,
	       const WN    *wn,          /* Base address for lvalue */
	       TY_IDX       addr_ty,     /* Type of base address */
	       TY_IDX       object_ty,   /* Type of object to be loaded */
	       STAB_OFFSET  addr_offset, /* Offset of object from base */
	       CONTEXT      context);


static void
WN2C_incr_indentation_for_stmt_body(const WN *body)
{
   /* The indentation of blocks as bodies of other statements or
    * functions is handled by the processing of the block in
    * WN2C_block(), so no increment needs be done here.
    */
   if (WN_operator(body) != OPR_BLOCK)
      Increment_Indentation();
} /* WN2C_incr_indentation_for_stmt_body */


static void 
WN2C_decr_indentation_for_stmt_body(const WN *body)
{
   /* The indentation of blocks as bodies of other statements or
    * functions is handled by the processing of the block in
    * WN2C_block(), so no decrement needs be done here.
    */
   if (WN_operator(body) != OPR_BLOCK)
      Decrement_Indentation();
} /* WN2C_decr_indentation_for_stmt_body */


static TOKEN_BUFFER
WN2C_generate_cast(TY_IDX cast_to,         /* Cast expr to this ty */
		   BOOL   pointer_to_type) /* TYs are pointed to */
{
   /* Generate a cast to preceede the existing tokens.  If 
    * pointer_to_type==TRUE, then we get:
    *
    *   (<cast_to> *)<tokens>
    *
    * Otherwise, we'll get:
    *
    *   (<cast_to>)<tokens>
    */
   TOKEN_BUFFER ty_buffer = New_Token_Buffer();
   
   if (pointer_to_type)
      Append_Token_Special(ty_buffer, '*');
   if (TY_Is_Array_Or_Function(cast_to))
      WHIRL2C_parenthesize(ty_buffer);

   TY2C_translate_unqualified(ty_buffer, cast_to);
   WHIRL2C_parenthesize(ty_buffer);
   return ty_buffer;
} /* WN2C_generate_cast */


static void
WN2C_prepend_cast(TOKEN_BUFFER tokens,          /* Expression to be cast */
		  TY_IDX       cast_to,         /* Cast expr to this ty */
		  BOOL         pointer_to_type) /* TYs are pointed to */
{
   TOKEN_BUFFER ty_buffer = WN2C_generate_cast(cast_to, pointer_to_type);
   Prepend_And_Reclaim_Token_List(tokens, &ty_buffer);
} /* WN2C_prepend_cast */


static void
WN2C_append_cast(TOKEN_BUFFER tokens,          /* Expression to be cast */
		 TY_IDX       cast_to,         /* Cast expr to this ty */
		 BOOL         pointer_to_type) /* TYs are pointed to */
{
   TOKEN_BUFFER ty_buffer = WN2C_generate_cast(cast_to, pointer_to_type);
   Append_And_Reclaim_Token_List(tokens, &ty_buffer);
} /* WN2C_append_cast */


static void
WN2C_Assign_Complex_Const(TOKEN_BUFFER tokens,
			  const char  *lhs_name,
			  TCON         realpart,
			  TCON         imagpart)
{
   /* Get real_part */
   Append_Token_Special(tokens, '('); /* getting real part */
   Append_Token_String(tokens, lhs_name);
   Append_Token_Special(tokens, '.');
   Append_Token_String(tokens, TY2C_Complex_Realpart_Name);
   Append_Token_Special(tokens, '=');
   TCON2C_translate(tokens, realpart);
   Append_Token_Special(tokens, ')'); /* gotten real part */
   Append_Token_Special(tokens, ',');

   /* Get imaginary_part */
   Append_Token_Special(tokens, '('); /* getting imaginary part */
   Append_Token_String(tokens, lhs_name);
   Append_Token_Special(tokens, '.');
   Append_Token_String(tokens, TY2C_Complex_Imagpart_Name);
   Append_Token_Special(tokens, '=');
   TCON2C_translate(tokens, imagpart);
   Append_Token_Special(tokens, ')'); /* gotten imaginary part */
   Append_Token_Special(tokens, ',');
} /* WN2C_Assign_Complex_Const */


static TOKEN_BUFFER
WN2C_Translate_Arithmetic_Operand(const WN*wn, 
				  TY_IDX   result_ty, 
				  CONTEXT  context)
{
   TOKEN_BUFFER opnd_tokens = New_Token_Buffer();
   TY_IDX       opnd_ty = WN_Tree_Type(wn);

   if (!WN2C_arithmetic_compatible_types(result_ty, opnd_ty))
   {
      CONTEXT_set_top_level_expr(context);
      (void)WN2C_translate(opnd_tokens, wn, context);
      WHIRL2C_parenthesize(opnd_tokens);
      WN2C_prepend_cast(opnd_tokens, result_ty, FALSE);
   }
   else
      (void)WN2C_translate(opnd_tokens, wn, context);
   
   return opnd_tokens;
} /* WN2C_Translate_Arithmetic_Operand */


static STATUS
WN2C_address_add(TOKEN_BUFFER tokens, 
		 OPCODE       opcode,  
		 TY_IDX       expr_ty,
		 WN          *wn0,
		 WN          *wn1,
		 CONTEXT      context)
{
   /* We make an attempt at retaining pointer types for ptr
    * arithmetics, where we expect ptr expressions to be of
    * one of the following forms:
    *
    *    1)  ptr + expr
    *    2)  ptr + expr*const
    *    1)  ptr + const
    *
    * and we modify the const whenever we can to account for the
    * element size.  First, see if we can find a suitable constant
    * expression.
    */
   TOKEN_BUFFER opnd_tokens;  /* Temporary buffer for translating operand */
   UINT64       old_intconst = 0;
   WN          *intconst = NULL;
   BOOL         top_level_expr = CONTEXT_top_level_expr(context);
   TY_IDX       wn0_ty = WN_Tree_Type(wn0);
   TY_IDX       wn1_ty = WN_Tree_Type(wn1);
   TY_IDX       given_lvalue_ty = (CONTEXT_lvalue_type(context)? 
				   CONTEXT_given_lvalue_ty(context) : 0);
   
   Is_True(OPCODE_operator(opcode) == OPR_ADD, 
	   ("Unexpected kind of pointer expression in WN2C_address_add()"));

   CONTEXT_reset_needs_lvalue(context);  /* Need actual value of subexprs */

   intconst = WN_Get_PtrAdd_Intconst(wn0, wn1, TY_pointed(expr_ty));
   if (intconst != NULL)
   {
      /* Normalize intconst, which may be an artbitrary expression 
       * if (TY_size(TY_pointed(expr_ty)) == 1).
       *
       * We already know that it must be divisable by the size of the
       * object pointed to by this expression.
       */
      if (TY_size(TY_pointed(expr_ty)) > 1)
      {
	 Is_True(WN_operator(intconst) == OPR_INTCONST, 
		 ("Expected INTCONST in WN2C_address_add()"));

	 old_intconst = WN_const_val(intconst);
	 WN_const_val(intconst) = 
	    WN_const_val(intconst) / TY_size(TY_pointed(expr_ty));
      }
      /* Fall through and handle the pointer addition below....
       */
   }
   else if (given_lvalue_ty == (TY_IDX) 0)
   {
      /* This may occur when the address expression is not dereferenced,
       * but occurs as an argument in a function call or in a pointer
       * assignment.  Do the offset using (char*), and cast the result
       * to a (void*).
       */
      top_level_expr = FALSE;
      WN2C_append_cast(tokens, 
			Stab_Mtype_To_Ty(MTYPE_V), TRUE/*ptr_to_type*/);

      if (TY_Is_Pointer(wn0_ty))
	 wn0_ty = Stab_Pointer_To(Stab_Mtype_To_Ty(MTYPE_U1));
      else
	 wn1_ty = Stab_Pointer_To(Stab_Mtype_To_Ty(MTYPE_U1));
   }
   else
   {
      /* This is a special case.  The ADD represents array indexing,
       * a struct offset, or a combination thereof.  First, try to 
       * handle this as a struct offset.
       */
      if (WN_operator(wn0) == OPR_INTCONST && 
	  TY_size(TY_pointed(expr_ty)) > WN_const_val(wn0) &&
	  TY_Is_Structured(TY_pointed(expr_ty)))
      {
	 /* Assume a struct offset or an array index */
	 return WN2C_lvalue_wn(tokens,
			       wn1,               /* Base address */
			       expr_ty,           /* Type of base address */
			       given_lvalue_ty,   /* Type of object */
			       WN_const_val(wn0), /* Offset from base */
			       context);
      }
      else if (WN_operator(wn1) == OPR_INTCONST && 
	       TY_size(TY_pointed(expr_ty)) > WN_const_val(wn1) &&
	       TY_Is_Structured(TY_pointed(expr_ty)))
      {
	 /* Assume a struct offset or an array index */
	 return WN2C_lvalue_wn(tokens,
			       wn0,               /* Base address */
			       expr_ty,           /* Type of base address */
			       given_lvalue_ty,   /* Type of object */
			       WN_const_val(wn1), /* Offset from base */
			       context);
      }
      else
      {
	 /* Do the pointer operation based on the given_lvalue_ty.
	  * It may not be pretty, but it should work.
	  */
	 intconst = WN_Get_PtrAdd_Intconst(wn0, wn1, given_lvalue_ty);
	 if (intconst != NULL)
	 {
	    /* Normalize the intconst.  We already know that it must be 
	     * divisable by the size of the object pointed to by this 
	     * expression.
	     */
	    if (TY_size(given_lvalue_ty) > 1)
	    {
	       Is_True(WN_operator(intconst) == OPR_INTCONST, 
		       ("Expected INTCONST in WN2C_address_add()"));

	       old_intconst = WN_const_val(intconst);
	       WN_const_val(intconst) = 
		  WN_const_val(intconst) / TY_size(given_lvalue_ty);
	    }

	    if (TY_Is_Pointer(wn0_ty))
	       wn0_ty = Stab_Pointer_To(given_lvalue_ty);
	    else
	       wn1_ty = Stab_Pointer_To(given_lvalue_ty);
	 }
	 else
	 {
	    /* Last resort */
	    top_level_expr = FALSE;
	    WN2C_append_cast(tokens, given_lvalue_ty, TRUE/*ptr_to_type*/);

	    if (TY_Is_Pointer(wn0_ty))
	       wn0_ty = Stab_Pointer_To(Stab_Mtype_To_Ty(MTYPE_U1));
	    else
	       wn1_ty = Stab_Pointer_To(Stab_Mtype_To_Ty(MTYPE_U1));
	 }

	 /* Fall through and handle the pointer addition below....
	  */
	 
      } /* if struct or array offset */
   } /* a regular pointer addition */

   /* Append the expression to "tokens", parenthesized if nested within
    * another expression.
    */
   CONTEXT_reset_top_level_expr(context); /* Subexprs are not top-level */
   if (!top_level_expr)
      Append_Token_Special(tokens, '(');

   if (!TY_Is_Pointer(wn0_ty))
      CONTEXT_reset_lvalue_type(context);
   opnd_tokens = WN2C_Translate_Arithmetic_Operand(wn0, wn0_ty, context);
   Append_And_Reclaim_Token_List(tokens, &opnd_tokens);

   Append_Token_Special(tokens, '+');

   if (TY_Is_Pointer(wn0_ty) && given_lvalue_ty != (TY_IDX) 0)
      CONTEXT_set_lvalue_type(context);
   opnd_tokens = WN2C_Translate_Arithmetic_Operand(wn1, wn1_ty, context);
   Append_And_Reclaim_Token_List(tokens, &opnd_tokens);

   if (!top_level_expr)
      Append_Token_Special(tokens, ')');

   /* Restore the intconst value */
   if (old_intconst > 0 && intconst != NULL)
      WN_const_val(intconst) = old_intconst;

   return EMPTY_STATUS;
} /* WN2C_address_add */


static STATUS
WN2C_infix_op(TOKEN_BUFFER tokens, 
	      OPCODE       opcode, 
	      TY_IDX       result_ty,
	      WN          *wn0,
	      WN          *wn1,
	      CONTEXT      context)
{
   /* This is assumed to be a binary operator, unless wn0==NULL,
    * in which case we treat it as a unary operator.  The operator
    * will be inserted infix and is assumed to consist of a sequence
    * of special character tokens.
    */
   const char  *op_char;
   const BOOL   binary_op = (wn0 != NULL);
   const BOOL   top_level_expr = CONTEXT_top_level_expr(context);
   const MTYPE  descriptor_mtype = OPCODE_desc(opcode);
   TY_IDX       wn0_ty;       /* Expected type of wn0 */
   TY_IDX       wn1_ty;       /* Expected type of wn1 */
   TOKEN_BUFFER opnd_tokens;  /* Temporary buffer for translating operand */

   /* Get the expected types for the two operands, dependent on whether
    * or not we have a descriptor type.
    */
   if (descriptor_mtype == MTYPE_V)
   {
      wn0_ty = wn1_ty = result_ty;
   }
   else
   {
      wn0_ty = wn1_ty = Stab_Mtype_To_Ty(descriptor_mtype);
   }

   /* Do the simplest forms of simplifications */
   if (OPCODE_operator(opcode) == OPR_NEG &&
       (WN_operator(wn1) == OPR_CONST || 
	WN_operator(wn1) == OPR_INTCONST))
   {
      TCON tcon, tcon1;
      BOOL folded;
      
      if (WN_operator(wn1) == OPR_CONST)
	 tcon1 = STC_val(WN_st(wn1));
      else /* (WN_operator(wn1) == OPR_INTCONST) */
	 tcon1 = Host_To_Targ(WN_opc_rtype(wn1), WN_const_val(wn1));
      tcon = Targ_WhirlOp(opcode, tcon1, tcon1, &folded);

      if (folded)
	 TCON2C_translate(tokens, tcon);
      else
      {
	 /* Explicitly do the operation */
	 for (op_char = WN2C_Opc2cname[opcode]; *op_char != '\0'; op_char++)
	    Append_Token_Special(tokens, *op_char);
	 Append_Token_Special(tokens, '(');
	 TCON2C_translate(tokens, tcon1);
	 Append_Token_Special(tokens, ')');
      }
      return EMPTY_STATUS;
   }
   else if (OPCODE_operator(opcode) == OPR_MPY)
   {
      if (WN_operator(wn0) == OPR_INTCONST && 
	  WN_const_val(wn0) == 1LL)
      {
	 opnd_tokens = WN2C_Translate_Arithmetic_Operand(wn1, wn1_ty, context);
	 Append_And_Reclaim_Token_List(tokens, &opnd_tokens);
	 return EMPTY_STATUS;
      }
      else if (WN_operator(wn1) == OPR_INTCONST && 
	       WN_const_val(wn1) == 1LL)
      {
	 opnd_tokens = WN2C_Translate_Arithmetic_Operand(wn0, wn0_ty, context);
	 Append_And_Reclaim_Token_List(tokens, &opnd_tokens);
	 return EMPTY_STATUS;
      }
   }

   /* Handle special pointer operations */
   if (binary_op && WN2C_is_pointer_diff(opcode, wn0, wn1))
   {
      opcode = WN_opcode(wn0); /* OPC_SUB */
      wn1 = WN_kid1(wn0);      /* ptr0 */
      wn0 = WN_kid0(wn0);      /* ptr1 */
      wn0_ty = WN_Tree_Type(wn0);
      wn1_ty = WN_Tree_Type(wn1);
   }
   else if (binary_op && TY_Is_Pointer(result_ty))
   {
      return WN2C_address_add(tokens, 
			      opcode,  
			      result_ty,
			      wn0,
			      wn1,
			      context);
   }

   /* Append the expression to "tokens", parenthesized if nested within
    * another expression.
    */
   CONTEXT_reset_top_level_expr(context); /* Subexprs are not top-level */
   if (!top_level_expr)
      Append_Token_Special(tokens, '(');

   if (binary_op)
   {
      opnd_tokens = 
	 WN2C_Translate_Arithmetic_Operand(wn0, wn0_ty, context);
      Append_And_Reclaim_Token_List(tokens, &opnd_tokens);
   }
       
   /* Operation */
   for (op_char = WN2C_Opc2cname[opcode]; *op_char != '\0'; op_char++)
      Append_Token_Special(tokens, *op_char);

   /* Second operand, or only operand for unary operation */
   opnd_tokens = WN2C_Translate_Arithmetic_Operand(wn1, wn1_ty, context);
   Append_And_Reclaim_Token_List(tokens, &opnd_tokens);

   if (!top_level_expr)
      Append_Token_Special(tokens, ')');
   
   return EMPTY_STATUS;
} /* WN2C_infix_op */


static STATUS
WN2C_funcall_op(TOKEN_BUFFER tokens, 
		OPCODE       opcode,
		WN          *wn0, 
		WN          *wn1, 
		CONTEXT      context)
{
   /* This is assumed to be a binary operator, unless wn0==NULL,
    * in which case we treat it as a unary operator.  The operator
    * will be applied as a function/macro call and is viewed as a
    * string token.
    */
   const TY_IDX result_ty = Stab_Mtype_To_Ty(OPCODE_rtype(opcode));
   TY_IDX       descriptor_ty = Stab_Mtype_To_Ty(OPCODE_desc(opcode));
   const BOOL   binary_op = (wn0 != NULL);
   TOKEN_BUFFER opnd_tokens;

   /* ASHR may appear here, and may part of a ptr-diff.  Treat
    * it specially here.
    */
   if (binary_op && WN2C_is_pointer_diff(opcode, wn0, wn1))
      return WN2C_infix_op(tokens, opcode, result_ty, wn0, wn1, context);

   CONTEXT_reset_needs_lvalue(context); /* Need actual value of subexprs */

   /* If there is no descriptor type, assume the operands should be
    * of the same type as the result.
    */
   if (TY_kind(descriptor_ty) == KIND_VOID)
      descriptor_ty = result_ty;
   
   /* Operator */
   Append_Token_String(tokens, WN2C_Opc2cname[opcode]);

   CONTEXT_set_top_level_expr(context);
   Append_Token_Special(tokens, '(');

   /* First operand */
   if (binary_op)
   {
      opnd_tokens = 
	 WN2C_Translate_Arithmetic_Operand(wn0, descriptor_ty, context);
      Append_And_Reclaim_Token_List(tokens, &opnd_tokens);
      Append_Token_Special(tokens, ',');
   }

   /* Second operand, or only operand for unary operation */
   opnd_tokens =
      WN2C_Translate_Arithmetic_Operand(wn1, descriptor_ty, context);
   Append_And_Reclaim_Token_List(tokens, &opnd_tokens);

   Append_Token_Special(tokens, ')');
   
   return EMPTY_STATUS;
} /* WN2C_funcall_op */


static TY_IDX
WN2C_MemAccess_Type(TY_IDX      base_ty,
		    TY_IDX      load_ty,
		    MTYPE       load_mtype,
		    STAB_OFFSET offset)
{
   /* Return the type of an object to be loaded/stored at the given
    * offset from a base object of type base_ty.  
    *
    * Should the load_ty be compatible with the base_ty, or should
    * the base_ty not be a struct, then just return the base_ty; otherwise,
    * if the base_ty is a struct, and a field is found at the given
    * offset of the load_ty or load_mtype, then return the FLD_type; 
    * Otherwise we have an unexpected case: If the base_ty has the
    * same size as the load_mtype, then return the base_ty; otherwise
    * otherwise return the load_ty.  
    *
    * This routine is made as general as possible, to handle all the
    * typing issues that may conceivably be encountered for loads and
    * stores.
    */
   TY2C_FLD_INFO fld_info;

   if (TY_Is_Array(base_ty))
   {
      /* Usually we do not access an array, but instead we access an element
       * of an array.  Use the load_ty, even if this also is an array (in
       * which case we are taking the address of an array?).
       */
      return load_ty;
   }
   else if (!TY_Is_Structured(base_ty) || 
	    WN2C_compatible_lvalues(load_ty, base_ty))
   {
      return base_ty;
   }
   else
   {
      /* May be accessing an element in a struct */
      fld_info = TY2C_get_field_info(base_ty, 
				     load_ty, 
				     load_mtype,
				     offset);
      if (!fld_info.found_fld.Is_Null ())
      {
	 Reclaim_Token_Buffer(&fld_info.select_tokens);
	 return FLD_type(fld_info.found_fld);
      }
      else if (Stab_Mtype_To_Ty(load_mtype) != (TY_IDX) 0 &&
	       TY_size(Stab_Mtype_To_Ty(load_mtype)) == TY_size(base_ty))
      {
	 return base_ty; 
      }
      else
      {
	 return load_ty;
      }
   }
} /* WN2C_MemAccess_Type */


static void
WN2C_SymAccess_Type(TY_IDX     *base_addr_ty,
		    TY_IDX     *object_ty,
		    TY_IDX      base_ty,
		    TY_IDX      load_ty,
		    MTYPE       load_mtype,
		    STAB_OFFSET offset)
{
   /* Return the type of an object_ty to be loaded/stored at the given
    * offset from a base object of type base_ty.  Since this is a
    * symbol access (LDID or STID) we do not have direct access to the
    * type of address at which to load/store, so we also derive that
    * here.  The address must observe the qualifiers (volatiles) of 
    * the load_ty.  When this routine returns:
    *
    *    !WN2C_compatible_lvalues(base_addr_ty, Stab_Pointer_To(base_ty))
    *
    * this implies that the lvalue of the symbol must be cast to a
    * different address-type (base_addr_ty) before the load can occur.
    * 
    * Otherwise, this routine is similar to the routine used for general
    * indirect loads and stores.
    */
   TY2C_FLD_INFO fld_info;
   TY_IDX        actual_loaded_ty = Stab_Mtype_To_Ty(load_mtype);

   if (TY_Is_Array(base_ty) &&
       WN2C_compatible_lvalues(load_ty, TY_AR_etype(base_ty)))
   {
      /* Accessing an element in an array */
      *object_ty = load_ty;
      if (TY_is_volatile(TY_AR_etype(base_ty)) == TY_is_volatile(load_ty))
	 *base_addr_ty = Stab_Pointer_To(base_ty);
      else
	 *base_addr_ty = Stab_Pointer_To(load_ty);
   } 
   else if (!TY_Is_Structured(base_ty) || 
	    WN2C_compatible_lvalues(load_ty, base_ty))
   {
      /* Seemingly interchangable types */
      if (actual_loaded_ty != (TY_IDX) 0                      &&
	  !WN2C_compatible_lvalues(actual_loaded_ty, load_ty) &&
	  WN2C_compatible_lvalues(actual_loaded_ty, base_ty))
      {
	 /* This should never really happen, but to ensure graceful recovery
	  * with continued code-generation we just emit what seems most
	  * sensible. This is a WHIRL BUG?
	  */
	 *object_ty = base_ty;
	 *base_addr_ty = Stab_Pointer_To(base_ty);
      }
      else // Normal case
      {
	 *object_ty = load_ty;
	 if (TY_is_volatile(base_ty) == TY_is_volatile(load_ty))
	    *base_addr_ty = Stab_Pointer_To(base_ty);
	 else
	    *base_addr_ty = Stab_Pointer_To(load_ty);
      }
   }
   else
   {
      /* May be accessing an element in a struct */
      fld_info = TY2C_get_field_info(base_ty, 
				     load_ty, 
				     load_mtype,
				     offset);
      if (!fld_info.found_fld.Is_Null ())
      {
	 const TY_IDX ftype = FLD_type(fld_info.found_fld);

	 Reclaim_Token_Buffer(&fld_info.select_tokens);
	 if (WN2C_compatible_lvalues(load_ty, ftype) &&
	     TY_is_volatile(ftype) == TY_is_volatile(load_ty))
	 {
	    *object_ty = ftype;
	    *base_addr_ty = Stab_Pointer_To(base_ty);
	 }
	 else
	 {
	    *object_ty = load_ty;
	    *base_addr_ty = Stab_Pointer_To(load_ty);
	 }
      }
      else if (actual_loaded_ty != (TY_IDX) 0                &&
	       TY_size(actual_loaded_ty) == TY_size(base_ty) &&
	       TY_is_volatile(base_ty) == TY_is_volatile(load_ty))
      {
	 /* This should never really happen, but to ensure graceful recovery
	  * with continued code-generation we just emit what seems most
	  * sensible.
	  */
	 *object_ty = base_ty;
	 *base_addr_ty = Stab_Pointer_To(base_ty);
      }
      else
      {
	 /* This should never really happen, but to ensure graceful recovery
	  * with continued code-generation we just emit what seems most
	  * sensible.
	  */
	 *object_ty = load_ty;
	 *base_addr_ty = Stab_Pointer_To(load_ty);
      }
   }
} /* WN2C_SymAccess_Type */



static void
WN2C_append_addr_plus_const(TOKEN_BUFFER tokens, /* Base address expression */
			    INT64        element_size,
			    STAB_OFFSET  byte_offset)
{
   /* Generate an address expression as follows:
    *
    *   <tokens> + byte_offset/element_size
    *
    * We assume the byte_offset is non-zero and an exact multiple of the 
    * element_size.
    */
    if (element_size == 0) {
      /* this is a KLUDGE, please fix this right.
	 for now, with void *, the size of void is null so
	 we fake it with element_size
       */
      element_size = 1;
   }
   Is_True(element_size!=0 && byte_offset!=0 && byte_offset%element_size==0,
	   ("Illegal address increment in WN2C_addr_plus()"));
   
   Append_Token_Special(tokens, '+');
   TCON2C_translate(tokens, 
		    Host_To_Targ(MTYPE_I8, byte_offset/element_size));
   WHIRL2C_parenthesize(tokens);
} /* WN2C_append_addr_plus_const */


static void
WN2C_append_addr_plus_expr(TOKEN_BUFFER tokens, /* Base address expression */
			   INT64        element_size,
			   TOKEN_BUFFER *byte_offset)
{
   /* Generate an address expression as follows:
    *
    *   <tokens> + byte_offset/element_size
    *
    * We assume the byte_offset is an exact multiple of the 
    * element_size.
    */
   Is_True(element_size!=0,
	   ("Illegal element size in WN2C_addr_plus()"));
   
   Append_Token_Special(tokens, '+');
   Append_And_Reclaim_Token_List(tokens, byte_offset);
   if (element_size != 1)
   {
      Append_Token_Special(tokens, '/');
      TCON2C_translate(tokens, Host_To_Targ(MTYPE_I8, element_size));
   }
   WHIRL2C_parenthesize(tokens);
} /* WN2C_append_addr_plus_expr */


static STATUS
WN2C_based_lvalue(TOKEN_BUFFER expr_tokens,    /* lvalue or addr expr */
		  BOOL         expr_is_lvalue, /* an lvalue expr? */
		  TY_IDX       expr_ty,        /* type of expr */
		  TY_IDX       addr_ty,        /* type of base of object */
		  TY_IDX       object_ty,      /* type of object */
		  STAB_OFFSET  addr_offset)    /* offset of object from base */
{
   /* This function takes an address (or lvalue) expression, and
    * adds in a given offset doing type conversions as is necessary
    * based on mismatches between the three types passed in.  We use
    * field-selection ('.' or '->') instead of casting and/or offset 
    * calculations whenever possible.  The resultant value in 
    * expr_tokens should either be an lvalue of type object_ty, or
    * an address value of type (object_ty *).
    *
    * At the various stages of this function, we update the local
    * state to reflect what is currently represented in expr_tokens.
    * This implementation detail makes it easier to assert about
    * what is the current state, and aids both the readability and
    * debuggability of this function.
    *
    * The returned status information indicates whether the resultant
    * expression denotes an (addressable) lvalue or not.
    */
   STATUS        status = EMPTY_STATUS;
   BOOL          incompat_addr_tys; /* expr_ty   vs addr_ty */
   BOOL          incompat_obj_tys;  /* object_ty vs TY_pointed(addr_ty) */
   TY_IDX        base_obj_ty;
   TY2C_FLD_INFO field_info;
   field_info.select_tokens = NULL;
   field_info.found_fld = FLD_HANDLE();

   /* PRECONDITION: */
   Is_True(expr_ty   != (TY_IDX) 0 &&
	   addr_ty   != (TY_IDX) 0 && TY_Is_Pointer(addr_ty) &&
	   object_ty != (TY_IDX) 0,
	   ("Expected non-null types in WN2C_based_lvalue()"));

   /* Determine type compatibility between the expression type (expr_ty),
    * its expected type (addr_ty), and the type of object being accessed
    * object_ty.  While the addr_ty must be a pointer type, the expr_ty
    * may be any value that may be casted to a pointer type.
    */
   base_obj_ty = TY_pointed(addr_ty);
   incompat_addr_tys = !WN2C_assignment_compatible_types(addr_ty, expr_ty);
   incompat_obj_tys = !WN2C_compatible_lvalues(object_ty, base_obj_ty);

   /* Correct the address of the base object, if necessary.  Note that
    * we permit any kind of expr_ty, while the addr_ty must always be
    * a pointer type.
    */
   if (incompat_addr_tys)
   {
      if (WN2C_array_lvalue_as_ptr(expr_ty,  /* ptr to array */
				   addr_ty)) /* ptr to etype */
      {
	 /* Note that an lvalue of an array is interpreted in C as a
	  * pointer to the array element type, so no cast is necessary
	  * once these types are compatible.  We no longer consider
	  * this an lvalue, since the caller of this routine should not
	  * take the address of the result to get the address value.
	  */
      }
      else
      {
	 /* Cast the address value to the appropriate pointer type */
	 if (expr_is_lvalue)
	 {
	    Prepend_Token_Special(expr_tokens, '&');
	 }
	 WN2C_prepend_cast(expr_tokens, addr_ty, FALSE/*ptr_to_type*/);
	 WHIRL2C_parenthesize(expr_tokens);
      } /* if (treat-array-as-pointer) */

      /* Update status */
      expr_is_lvalue = FALSE;
      expr_ty = addr_ty;
      incompat_addr_tys = FALSE;
   }
   else if (WN2C_array_lvalue_as_ptr(addr_ty,            /* ptr to array */
				     Stab_Pointer_To(object_ty))) /* ptr */
   {
      /* An array lvalue is to be interpreted in C as a pointer to 
       * the array element type, so no cast is necessary
       * once these types are compatible.  We no longer consider
       * this an lvalue, since the caller of this routine should not
       * take the address of the result to get the address value.
       */
      expr_is_lvalue = FALSE;
      expr_ty = addr_ty = Stab_Pointer_To(object_ty);
      base_obj_ty = object_ty;
      incompat_obj_tys = FALSE;
   } /* if (incompat_addr_tys) */

   /* Update status */
   expr_ty = addr_ty;
   incompat_addr_tys = FALSE;

   /* Determine whether or not the object type is a field in a 
    * struct/class/union at the given offset, in which case we
    * should use field-selection instead of an offset calculation.
    */
   if (incompat_obj_tys && TY_Is_Structured(base_obj_ty))
   {
      field_info =
	 TY2C_get_field_info(base_obj_ty, 
			     object_ty, 
			     TY_mtype(object_ty), 
			     addr_offset);
   }

   /* Finally, get the address or lvalue corresponding to the object_ty
    * and addr_offset.
    */
   if (!field_info.found_fld.Is_Null ())
   {
      WHIRL2C_parenthesize(expr_tokens);
      if (expr_is_lvalue)
	 Append_Token_Special(expr_tokens, '.');
      else
	 Append_Token_String(expr_tokens, "->");
      Append_And_Reclaim_Token_List(expr_tokens, &field_info.select_tokens);
      expr_is_lvalue = TRUE;
   }
   else if (addr_offset != (STAB_OFFSET) 0 || incompat_obj_tys)
   {
      if (expr_is_lvalue)
	 Prepend_Token_Special(expr_tokens, '&');

      if (addr_offset != 0 && TY_size(object_ty) != 0 && addr_offset%TY_size(object_ty) != 0)
      {
	 /* Use cast and pointer arithmetics to give us:
	  *
	  *   (object_ty *)((char *)&<expr_tokens> + addr_offset)
	  */
	 WN2C_prepend_cast(expr_tokens, 
			   Stab_Mtype_To_Ty(MTYPE_I1), 
			   TRUE/*ptr_to_type*/);
	 WHIRL2C_parenthesize(expr_tokens);
	 WN2C_append_addr_plus_const(expr_tokens, 
				     TY_size(Stab_Mtype_To_Ty(MTYPE_I1)),
				     addr_offset);
	 WN2C_prepend_cast(expr_tokens, object_ty, TRUE/*ptr_to_type*/);
      }
      else
      {
	 /* Use cast and pointer arithmetics to give us:
	  *
	  *   (object_ty *)&<expr_tokens> + addr_offset/sizeof(object_ty)
	  */
	//if (incompat_obj_tys && !expr_is_lvalue)
	if (incompat_obj_tys)
	 {
	    WHIRL2C_parenthesize(expr_tokens);

	    WN2C_prepend_cast(expr_tokens, object_ty, TRUE/*ptr_to_type*/);
	 }

	 if (addr_offset != 0)
	    WN2C_append_addr_plus_const(expr_tokens, 
					TY_size(object_ty), 
					addr_offset);
      }
      expr_is_lvalue = FALSE;
   } /* if (use-cast-and-ptr-arithmetics) */

   if (expr_is_lvalue)
      STATUS_set_lvalue(status);
   return status;
} /* WN2C_based_lvalue */


static STATUS
WN2C_lvalue_st(TOKEN_BUFFER tokens,
	       const ST    *st,          /* Base symbol for lvalue */
	       TY_IDX       addr_ty,     /* Type of base object */
	       TY_IDX       object_ty,   /* Type of object */
	       STAB_OFFSET  addr_offset, /* Offset of object from base */
	       CONTEXT      context)
{
   /* PRECONDITION: tokens is an empty initialized buffer.
    *
    * Translate an object reference based at the given ST into
    * a C lvalue or an address expression, putting tokens into "tokens".
    * Return with STATUS_is_lvalue when the resultant expression is a 
    * true addressable lvalue, rather than an address calculation.
    *
    * When we return with STATUS_is_lvalue, the caller must prepend
    * the '&' operator to get the address of the value.  When we
    * return with !STATUS_is_lvalue, the caller must prepend the
    * '*' operator to access the value of the address.
    */
   STATUS status;
   TY_IDX base_ptr_ty;

   Is_True(ST_sym_class(st) != CLASS_PREG, 
	   ("Did not expect a preg in WN2C_lvalue_st()"));

   if (ST_sym_class(st) == CLASS_CONST)
   {
      TCON2C_translate(tokens, STC_val(st));
      status = EMPTY_STATUS; /* not an lvalue */
   } else if (Compile_Upc && strcmp(ST_name(st), "MYTHREAD") == 0) {
     //WEI: treat them specially since they can be used either with U4 or I4
     Append_Token_String(tokens, WN_intrinsic_name(INTRN_MYTHREAD));
     Append_Token_String(tokens, "()");
     STATUS_set_lvalue(status);
   } else if (Compile_Upc && strcmp(ST_name(st), "THREADS") == 0) {
     Append_Token_String(tokens, WN_intrinsic_name(INTRN_THREADS));
     Append_Token_String(tokens, "()");
     STATUS_set_lvalue(status);
   } else
     {
      /* Get the variable name */
      if (Stab_Is_Based_At_Common_Or_Equivalence(st))
      {
	 addr_offset += ST_ofst(st);
	 st = ST_base(st);
	 addr_ty = Stab_Pointer_To(ST_type(st));
      }

      if (ST_is_split_common(st))
      {
	 addr_offset += Stab_Full_Split_Offset(st);
	 st = ST_full(st);
	 addr_ty = Stab_Pointer_To(ST_type(st));
      }

      ST2C_use_translate(tokens, st, context);
      base_ptr_ty = Stab_Pointer_To(ST_sym_class(st) == CLASS_FUNC ?
                                                 ST_pu_type(st) : ST_type(st));

      /* Get the object as an lvalue or as an address expression.
       */
      status = WN2C_based_lvalue(tokens,      /* base variable name */
				 TRUE,        /* tokens represent an lvalue */
				 base_ptr_ty, /* type of base variable addr */
				 addr_ty,     /* expected type of base addr */
				 object_ty,   /* expected type of object */
				 addr_offset);/* offset of object from base */
   }
   return status;
} /* WN2C_lvalue_st */


static STATUS
WN2C_lvalue_wn(TOKEN_BUFFER tokens,
	       const WN    *wn,          /* Base address for lvalue */
	       TY_IDX       addr_ty,     /* Type of base address */
	       TY_IDX       object_ty,   /* Type of object to be loaded */
	       STAB_OFFSET  addr_offset, /* Offset of object from base */
	       CONTEXT      context)
{
   /* PRECONDITION: tokens is an empty initialized buffer.
    *
    * Translate an object reference based at the given WN tree into
    * a C lvalue or an address expression, putting tokens into "tokens".
    * Return with STATUS_is_lvalue when the resultant expression is a 
    * true addressable lvalue, rather than an address calculation.
    *
    * When we return with STATUS_is_lvalue, the caller must prepend
    * the '&' operator to get the address of the value.  When we
    * return with !STATUS_is_lvalue, the caller must prepend the
    * '*' operator to access the value of the address.
    */
   STATUS status;
   TY_IDX tree_type = WN_Tree_Type(wn);

   /* Always try to get an lvalue representation for the address 
    * expression.
    */
   CONTEXT_set_needs_lvalue(context);
   CONTEXT_reset_top_level_expr(context);
   if (WN_operator(wn) == OPR_ARRAY)
   {
      TY_IDX array_base_ty = WN_Tree_Type(WN_kid0(wn));

      if (!TY_Is_Pointer(array_base_ty))
      {
	 /* A special case, which we handle but perhaps it should 
	  * be considered an error:  The array base is not a pointer
	  * type.
	  */
	 CONTEXT_set_array_basetype(context);
	 CONTEXT_set_given_base_ty(context, addr_ty);
	 tree_type = addr_ty;
      }
   }
   else if (WN_operator(wn) == OPR_ADD &&
	    TY_Is_Pointer(tree_type)       &&
	    WN_Get_PtrAdd_Intconst(WN_kid0(wn), 
				   WN_kid1(wn), 
				   TY_pointed(tree_type)) == NULL)
   {
      /* An add representing a struct field selection or an array 
       * element indexing operation.  Go straight for the object type
       * (with potentially ugly code).  See WN2C_address_add() for
       * details.
       */
      CONTEXT_set_lvalue_type(context);
      CONTEXT_set_given_lvalue_ty(context, object_ty);
      addr_ty = tree_type = Stab_Pointer_To(object_ty);
   }

   status = WN2C_translate(tokens, wn, context);

   /* Get the object as an lvalue or as an address expression.
    */
   status = WN2C_based_lvalue(tokens,       /* base expression */
			      STATUS_is_lvalue(status),
                                            /* tokens represent an lvalue */
			      tree_type,    /* type of base expression */
			      addr_ty,      /* expected type of base address */
			      object_ty,    /* expected type of object */
			      addr_offset); /* offset of object from base */

   return status;
} /* WN2C_lvalue_wn */


static void 
WN2C_create_ref_param_lda(WN *lda, const WN *ldid)
{
   const TY_IDX lhs_addr_ty = Stab_Pointer_To(WN_ty(ldid));

   bzero(lda, sizeof(WN));
   WN_set_opcode(lda, OPCODE_make_op(OPR_LDA, TY_mtype(lhs_addr_ty), MTYPE_V));
   WN_set_kid_count(lda, 0);
   WN_map_id(lda)      = (WN_MAP_ID) (-1);
   WN_load_offset(lda) = 0;
   WN_st_idx(lda)      = WN_st_idx(ldid);
   WN_set_ty(lda, lhs_addr_ty);
} /* WN2C_create_ref_param_lda */


static void 
WN2C_Append_Preg(TOKEN_BUFFER tokens, 
		 const ST    *st,       /* preg */
		 PREG_IDX     preg_idx, /* preg index */
		 TY_IDX       ty,       /* desired type */
		 CONTEXT      context)
{
   /* Given a preg (st, idx), append an expression referencing the preg
    * as the desired type to the token list.
    */
   TY_IDX       preg_ty;
   TOKEN_BUFFER preg_tokens;
   
   Is_True(ST_sym_class(st) == CLASS_PREG, 
	   ("Expected preg in WN2C_Append_Preg()"));
   
   /* Put the name of the preg into the preg_tokens */
   preg_tokens = New_Token_Buffer();
   preg_ty = PUinfo_Preg_Type(ST_type(st), preg_idx);
   ST2C_Use_Preg(preg_tokens, preg_ty, preg_idx, context);
   
   /* If the preg is declared with a btype different from that of
    * a pointer type, then cast it to an integral type with the
    * pointer btype, before casting it to the pointer type.
    */
   if (TY_Is_Pointer(ty) && TY_mtype(ty) != TY_mtype(preg_ty))
   {
      WN2C_prepend_cast(preg_tokens, 
			Stab_Mtype_To_Ty(TY_mtype(ty)), 
			FALSE/*ptr_to_type*/);
   }
   
   /* Now cast the preg to the appropriate type, unless the preg
    * is already arithmetically compatible with the desired type.
    */
   if (!WN2C_arithmetic_compatible_types(preg_ty, ty))
      WN2C_prepend_cast(preg_tokens, ty, FALSE/*ptr_to_type*/);

   Append_And_Reclaim_Token_List(tokens, &preg_tokens);
} /* WN2C_Append_Preg */


static void
WN2C_Load_From_PregIdx(TOKEN_BUFFER tokens,
		       const ST    *preg1,     /* Base address preg */
		       PREG_IDX     preg_idx1, 
		       const ST    *preg2,     /* Byte offset preg */
		       PREG_IDX     preg_idx2,
		       TY_IDX       object_ty,
		       CONTEXT      context)
{
   /* Load the contents of the pregs, add them together, cast the 
    * resultant address to a pointer to object_ty, and dereference.
    */
   TOKEN_BUFFER base_buffer, offset_buffer;

   /* Get the base address into which we are storing, as a pointer,
    * and cast it to a pointer to a character.
    */
   base_buffer = New_Token_Buffer();
   WN2C_Append_Preg(base_buffer, 
		    preg1, 
		    preg_idx1, 
		    Stab_Pointer_To(Stab_Mtype_To_Ty(MTYPE_U1)),
		    context);
   
   /* Get the offset value */
   offset_buffer = New_Token_Buffer();
   WN2C_Append_Preg(offset_buffer, 
		    preg2, 
		    preg_idx2, 
		    ST_type(preg2),
		    context);

   /* Add in the offset, giving us a parenthesized address expression. */
   WN2C_append_addr_plus_expr(base_buffer, 1/*byte size*/, &offset_buffer);

   /* Cast the resultant address to a pointer to the expected result
    * type, and dereference.
    */
   WN2C_prepend_cast(base_buffer, object_ty, TRUE/*pointer_to_type*/);
   Prepend_Token_Special(base_buffer, '*'); /* Dereference */

   Append_And_Reclaim_Token_List(tokens, &base_buffer);
} /* WN2C_Load_From_PregIdx */


static void
WN2C_Append_Assignment(TOKEN_BUFFER  tokens, 
		       TOKEN_BUFFER *lhs_tokens, 
		       const WN     *rhs,
		       TY_IDX        assign_ty,
		       CONTEXT       context)
{
   TOKEN_BUFFER rhs_buffer = New_Token_Buffer();
   TY_IDX       rhs_ty = WN_Tree_Type(rhs);

   if (WN_operator(rhs) == OPR_LDID || WN_operator(rhs) == OPR_ILOAD) {
     if (WN_field_id(rhs) != 0 && TY_kind(rhs_ty) == KIND_STRUCT) {
       rhs_ty = Get_Field_Type(rhs_ty, WN_field_id(rhs));
     }
   }
   //cout << "TYPE ASSIGNED TO: " << assign_ty << endl;
   /* Get the rhs expression */
   if (!WN2C_assignment_compatible_types(assign_ty, rhs_ty))
   {
      if (!TY_Is_Structured(assign_ty) && !TY_Is_Structured(rhs_ty))
      {
	 (void)WN2C_translate(rhs_buffer, rhs, context);
	 WHIRL2C_parenthesize(rhs_buffer);
	 WN2C_prepend_cast(rhs_buffer,
			   assign_ty,
			   FALSE/*pointer_to_type*/);
      }
      else
      {
	 /* Assign the rhs to a temporary, cast the address of the
	  * temporary to the assign_ty, then dereference.
	  */
	 const UINT  tmp_idx = Stab_Lock_Tmpvar(rhs_ty, 
						ST2C_Declare_Tempvar);
	 const char *tmpvar_name = W2CF_Symtab_Nameof_Tempvar(tmp_idx);
	 
	 /* Assign rhs to temporary */
	 Append_Token_String(tokens, tmpvar_name);
	 Append_Token_Special(tokens, '=');
	 (void)WN2C_translate(tokens, rhs, context);
	 Append_Token_Special(tokens, ';');
	 WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));

	 /* Rhs is the dereferenced casted temporary */
	 Append_Token_Special(rhs_buffer, '&');
	 Append_Token_String(rhs_buffer, tmpvar_name);
	 WN2C_prepend_cast(rhs_buffer,
			   assign_ty,
			   TRUE/*pointer_to_type*/);
	 Prepend_Token_Special(rhs_buffer, '*');
	 Stab_Unlock_Tmpvar(tmp_idx);
      }
   }
   else
   {
      (void)WN2C_translate(rhs_buffer, rhs, context);
   }
   
   /* Wrap it up by assigning the rhs to the lhs */
   Append_And_Reclaim_Token_List(tokens, lhs_tokens);
   Append_Token_Special(tokens, '=');
   Append_And_Reclaim_Token_List(tokens, &rhs_buffer);
} /* WN2C_Append_Assignment */


static void
WN2C_Normalize_Idx_To_Onedim(TOKEN_BUFFER tokens, 
			     const WN    *wn,
			     CONTEXT      context)
{
   /* Express a multidimensional array index as a one-dimensional
    * array index.  The resulting expression denotes the offset into
    * an array in terms of number of elements.
    */
   static char  dim_string[128];
   char        *dim_stringp = &dim_string[0];
   INT32        dim1, dim2;
 
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
	 CONTEXT_set_top_level_expr(context);
      (void)WN2C_translate(tokens, WN_array_index(wn, dim1), context);
      for (dim2 = dim1+1; dim2 < WN_num_dim(wn); dim2++)
      {
	 Append_Token_Special(tokens, '*');
	 (void)WN2C_translate(tokens, WN_array_dim(wn, dim2), context);
      } /*for*/

      if (W2C_Emit_Adims)
      {
	 /* Indicate in the output which dimension this is */
	 sprintf(dim_stringp, "/*Dim%d*/", WN_num_dim(wn)-dim1);
	 Append_Token_String(tokens, dim_stringp);
      }
   } /*for*/

} /* WN2C_Normalize_Idx_To_Onedim */


static void
WN2C_append_label_name(TOKEN_BUFFER tokens, const WN *wn)
{
   /* There is no way to restore user-defined label names, so just use
    * the compiler generated numbers, prefixed by an underscore.
    */
   Append_Token_String(tokens, WHIRL2C_number_as_c_name(WN_label_number(wn)));
} /* WN2C_append_label_name */




static void
WN2C_Append_Symtab_Types(TOKEN_BUFFER tokens, UINT lines_between_decls)
{
   /* When "tokens" is NULL, we append the type declarations directly 
    * to the W2C_File[W2C_DOTH_FILE] (at file-level); otherwise, append them
    * them to the given token-list.
    */
   TY_IDX       ty,ty_idx;
   TOKEN_BUFFER tmp_tokens;

   //WEI: This code is obviously broken, but should we fix it?
   //FMZ: we fix the bug--June 30,2004

   /* Declare structure types. --*/
   for (ty = 1; ty < TY_Table_Size(); ty++)
   {
     ty_idx = ty<<8;
     if (TY_Is_Structured(ty_idx)       &&
	 !TY_split(Ty_Table[ty_idx])    &&
	 !TY_is_translated_to_c(ty_idx)  &&
	  !Stab_Reserved_Ty(ty_idx))
      {
	 tmp_tokens = New_Token_Buffer();

         Set_TY_is_written(ty_idx); 
         //this should  be done in frontend,will fix later--FMZ

//	 TY2C_translate_unqualified(tmp_tokens, ty);
         //misuse this ty as TY_IDX--FMZ

	 TY2C_translate_unqualified(tmp_tokens, ty_idx);

#if 0 //not correct code---FMZ
	 Append_Token_Special(tmp_tokens, ';');
	 Append_Indented_Newline(tmp_tokens, lines_between_decls);
	 if (tokens != NULL)
	    Append_And_Reclaim_Token_List(tokens, &tmp_tokens);
	 else {
	    Write_And_Reclaim_Tokens(W2C_File[W2C_DOTH_FILE], 
				     NULL, /* No srcpos map */
				     &tmp_tokens);
	 }
#endif

      }
   }
}  /* WN2C_Append_Symtab_Types */


static void
WN2C_Append_Symtab_Consts(TOKEN_BUFFER tokens, 
			  BOOL         use_const_tab,
			  UINT         lines_between_decls,
			  CONTEXT      context)
{
   /* When "tokens" is NULL, we append the consts directly to the
    * W2C_File[W2C_DOTH_FILE] (at file-level); otherwise, append
    * them to the given token-list.
    */
   const ST    *st;
   TOKEN_BUFFER tmp_tokens;

   /* Declare any STFL_IS_CONST_VAR variables */
   ST_IDX st_idx;
   FOREACH_SYMBOL(CURRENT_SYMTAB, st, st_idx)
   {
//      if ((ST_sym_class(st) == CLASS_VAR && ST_is_const_var(st)) || 
//	  (ST_sym_class(st) == CLASS_CONST))
// Only " ST_IS_CONST_VAR" need to be dumpout.
// if "ST_sym_class(st) == CLASS_CONST",we never need to have explicit 
// declaration for the symbol in unparsed code----fzhao

     if (ST_sym_class(st) == CLASS_VAR && ST_is_const_var(st)) 
     
      {
	 tmp_tokens = New_Token_Buffer();
	 ST2C_decl_translate(tmp_tokens, st, context);
	 Append_Token_Special(tmp_tokens, ';');
	 Append_Indented_Newline(tmp_tokens, lines_between_decls);
	 if (tokens != NULL)
	    Append_And_Reclaim_Token_List(tokens, &tmp_tokens);
	 else {
	   Write_And_Reclaim_Tokens(W2C_File[W2C_DOTH_FILE], 
				    NULL, /* No srcpos map */
				    &tmp_tokens);
	 }
      }
   }
} /* WN2C_Append_Symtab_Consts */

struct eqstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};

static hash_set<const char*, hash<const char*>, eqstr> upcr_internal;
static bool init = false;

static bool lookup(const char* s) {
  return upcr_internal.find(s) != upcr_internal.end(); 
}

static void init_map() {
  
  if (init) {
    return;
  }

  upcr_internal.clear();

  //initialize the table
  upcr_internal.insert("UPCR_LOCAL_ALLOC");
  upcr_internal.insert("UPCR_GLOBAL_ALLOC");
  upcr_internal.insert("UPCR_ALL_ALLOC");
  upcr_internal.insert("UPCR_FREE");
  upcr_internal.insert("UPCR_GLOBAL_LOCK_ALLOC");
  upcr_internal.insert("UPCR_ALL_LOCK_ALLOC");
  upcr_internal.insert("UPCR_LOCK");
  upcr_internal.insert("UPCR_LOCK_ATTEMPT");
  upcr_internal.insert("UPCR_UNLOCK");
  upcr_internal.insert("UPCR_LOCK_FREE");
  upcr_internal.insert("UPCR_PUT_SHARED");
  upcr_internal.insert("UPCR_PUT_PSHARED");
  upcr_internal.insert("UPCR_PUT_NB_SHARED");
  upcr_internal.insert("UPCR_PUT_NB_PSHARED");
  upcr_internal.insert("UPCR_GET_SHARED");
  upcr_internal.insert("UPCR_GET_NB_SHARED");
  upcr_internal.insert("UPCR_GET_PSHARED");
  upcr_internal.insert("UPCR_GET_NB_PSHARED");
  upcr_internal.insert("UPCR_WAIT_SYNCNB");
  upcr_internal.insert("UPCR_TRY_SYNCNB");
  upcr_internal.insert("UPCR_PUT_SHARED_VAL");
  upcr_internal.insert("UPCR_PUT_NB_SHARED_VAL");
  upcr_internal.insert("UPCR_PUT_PSHARED_VAL");
  upcr_internal.insert("UPCR_PUT_NB_PSHARED_VAL");
  upcr_internal.insert("UPCR_GET_SHARED_VAL");
  upcr_internal.insert("UPCR_GET_NB_SHARED_VAL");
  upcr_internal.insert("UPCR_GET_PSHARED_VAL");
  upcr_internal.insert("UPCR_GET_NB_PSHARED_VAL");
  upcr_internal.insert("UPCR_WAIT_SYNCNB_VALGET");
  upcr_internal.insert("UPCR_PUT_SHARED_FLOATVAL");
  upcr_internal.insert("UPCR_PUT_PSHARED_FLOATVAL");
  upcr_internal.insert("UPCR_PUT_SHARED_DOUBLEVAL");
  upcr_internal.insert("UPCR_PUT_PSHARED_DOUBLEVAL");
  upcr_internal.insert("UPCR_GET_SHARED_FLOATVAL");
  upcr_internal.insert("UPCR_GET_PSHARED_FLOATVAL");
  upcr_internal.insert("UPCR_GET_SHARED_DOUBLEVAL");
  upcr_internal.insert("UPCR_GET_PSHARED_DOUBLEVAL");
  upcr_internal.insert("UPCR_MEMGET");
  upcr_internal.insert("UPCR_MEMPUT");
  upcr_internal.insert("UPCR_MEMSET");
  upcr_internal.insert("UPCR_NB_MEMGET");
  upcr_internal.insert("UPCR_NB_MEMPUT");
  upcr_internal.insert("UPCR_NB_MEMSET");
  upcr_internal.insert("UPCR_MEMCPY");
  upcr_internal.insert("UPCR_NB_MEMCPY");
  upcr_internal.insert("UPCR_ISNULL_SHARED");
  upcr_internal.insert("UPCR_ISNULL_PSHARED");
  upcr_internal.insert("UPCR_SHARED_TO_LOCAL");
  upcr_internal.insert("UPCR_PSHARED_TO_LOCAL");
  upcr_internal.insert("UPCR_SHARED_TO_PSHARED");
  upcr_internal.insert("UPCR_PSHARED_TO_SHARED");
  upcr_internal.insert("UPCR_SHARED_RESETPHASE");
  upcr_internal.insert("UPCR_THREADOF_SHARED");
  upcr_internal.insert("UPCR_THREADOF_PSHARED");
  upcr_internal.insert("UPCR_PHASEOF_SHARED");
  upcr_internal.insert("UPCR_PHASEOF_PSHARED");
  upcr_internal.insert("UPCR_ADDRFIELD_SHARED");
  upcr_internal.insert("UPCR_ADDRFIELD_PSHARED");
  upcr_internal.insert("UPCR_ADD_SHARED");
  upcr_internal.insert("UPCR_ADD_PSHAREDI");
  upcr_internal.insert("UPCR_ADD_PSHARED1");
  upcr_internal.insert("UPCR_ISEQUAL_SHARED_SHARED");
  upcr_internal.insert("UPCR_ISEQUAL_SHARED_PSHARED");
  upcr_internal.insert("UPCR_ISEQUAL_PSHARED_PSHARED");
  upcr_internal.insert("UPCR_SUB_SHARED");
  upcr_internal.insert("UPCR_SUB_PSHAREDI");
  upcr_internal.insert("UPCR_SUB_PSHARED1");
  upcr_internal.insert("upcr_hasAffinity");
  upcr_internal.insert("upcr_barrier");
  

  init = true;
}

static void
WN2C_Append_Symtab_Vars(TOKEN_BUFFER tokens, 
			INT          lines_between_decls,
			CONTEXT      context)
{
   /* When "tokens" is NULL, we append the vars directly to the
    * W2C_File[W2C_DOTH_FILE] (at file-level); otherwise, append
    * them to the given token-list.
    */
   const ST    *st;
   TOKEN_BUFFER var_tokens;
   TOKEN_BUFFER func_tokens;
   TOKEN_BUFFER tmp_tokens;
   ST_IDX       st_idx;

   init_map();
   
   var_tokens = New_Token_Buffer();
   func_tokens = New_Token_Buffer();

   /* Declare identifiers from the new symbol table, provided they
    * represent functions or variables that are either defining
    * global definition or that have been referenced in this 
    * compilation unit.
    */
   FOREACH_SYMBOL(CURRENT_SYMTAB, st, st_idx)
   {
     //WEI: ignore static or global variables, since they're handled specially by the initialization script
     //For SCLASS_PSTATIC, we additionally check that the symbol name does not include ".init" (such name is 
     //reserved for temp symbols representing array initializers, and need to be outputed)
     bool global = ST_sym_class(st) == CLASS_VAR && 
       ((ST_sclass(st) == SCLASS_PSTATIC && strstr(ST_name(st), ".init") == NULL) ||
	ST_sclass(st) == SCLASS_FSTATIC || 
       ST_sclass(st) == SCLASS_UGLOBAL || ST_sclass(st) == SCLASS_DGLOBAL || 
       ST_sclass(st) == SCLASS_EXTERN || ST_sclass(st) == SCLASS_COMMON);
     //WEI: for testing, block all extern functions that returns void for now
     bool extern_void_fun = (ST_sym_class(st) == CLASS_FUNC && ST_sclass(st) == SCLASS_EXTERN); 
     extern_void_fun = extern_void_fun && (TY_mtype(TY_ret_type(ST_pu_type(st))) != MTYPE_F8);
     global = global || extern_void_fun;

     TY_IDX st_ty  = ST_class(st) == CLASS_VAR ? ST_type(st) :
       ST_class(st) == CLASS_FUNC ? ST_pu_type(st) : ST_type(st);
//     cout << "For ST: " << ST_name(st) << " " << st_ty << " " << TY_name(st_ty) << endl;      
     if (!ST_is_not_used(st)                         &&
	 ST_sclass(st) != SCLASS_FORMAL              && 
	 ST_sclass(st) != SCLASS_FORMAL_REF          &&
	 ((ST_sym_class(st) == CLASS_VAR && !ST_is_const_var(st)) || 
	  ST_sym_class(st) == CLASS_FUNC)            &&
	 !Stab_Reserved_St(st)                       &&
	 !Stab_Is_Based_At_Common_Or_Equivalence(st) &&
	 !Stab_Is_Common_Block(st)                   &&
	 !Stab_Is_Equivalence_Block(st)              &&
	 !global &&
	 !(ST_sym_class(st) == CLASS_FUNC && lookup(ST_name(st))) &&
	 (Stab_External_Def_Linkage(st)                        || 
	  (ST_sym_class(st) == CLASS_VAR && ST_sclass(st) == SCLASS_CPLINIT) ||
	  BE_ST_w2fc_referenced(st)) || 
          global )
       {
	 tmp_tokens = New_Token_Buffer();
	 if (ST_is_weak_symbol(st))
	 {
	    ST2C_weakext_translate(tmp_tokens, st, context);
	 }
	 else
	 {
	    ST2C_decl_translate(tmp_tokens, st, context);
	    Append_Token_Special(tmp_tokens, ';');
	 }

	 Append_Indented_Newline(tmp_tokens, lines_between_decls);

         if (ST_sym_class(st) == CLASS_FUNC)
               Append_And_Reclaim_Token_List(func_tokens, &tmp_tokens);
         else
               Append_And_Reclaim_Token_List(var_tokens, &tmp_tokens);
#if 0
	 if (tokens != NULL)
	     Append_And_Reclaim_Token_List(tokens, &tmp_tokens);
	 else 
             Write_And_Reclaim_Tokens(W2C_File[W2C_DOTH_FILE], 
				          NULL, /* No srcpos map */
				          &tmp_tokens);
#endif
	 }
       
       }  /*FOREACH */

        if (tokens != NULL){
            if (func_tokens != NULL)
                  Append_And_Reclaim_Token_List(tokens, &func_tokens);
            if (var_tokens != NULL)
                  Append_And_Reclaim_Token_List(tokens, &var_tokens);
          }
         else {
            if (func_tokens != NULL)
                Write_And_Reclaim_Tokens(W2C_File[W2C_DOTH_FILE],
                                          NULL, /* No srcpos map */
                                          &func_tokens);
             if (var_tokens != NULL)
                 Write_And_Reclaim_Tokens(W2C_File[W2C_DOTH_FILE],
                                          NULL, /* No srcpos map */
                                          &var_tokens);
            }

} /* WN2C_Append_Symtab_Vars */


static void
WN2C_Declare_Return_Variable(TOKEN_BUFFER tokens)
{
   /* Declare the return variable; i.e. a temporary variable to hold
    * the function return value.  This is necessary to unify a return
    * value split up between two return pseudo-registers.
    */
   TOKEN_BUFFER tmp_tokens;

   /* Declare a variable to hold the function return value */
   tmp_tokens = New_Token_Buffer();
   Append_Token_String(tmp_tokens, WN2C_Return_Value_Name);
   TY2C_translate_unqualified(tmp_tokens, PUINFO_RETURN_TY);
   Append_Token_Special(tmp_tokens, ';');
   Append_And_Reclaim_Token_List(tokens, &tmp_tokens);
} /* WN2C_Declare_Return_Variable */


static void
WN2C_Declare_Return_Parameter(TOKEN_BUFFER tokens, CONTEXT context)
{
   /* We are in the scope of a function that returns its value into
    * the location pointed to by its first (implicit) parameter.  We
    * do not want the parameter to be part of the function declaration,
    * and instead declare a local variable to represent the return value.
    * The declaration is local to the current_func in the given context.
    */
   
   /* First, declare a variable to hold the function return value */
   WN2C_Declare_Return_Variable(tokens);
   Append_Indented_Newline(tokens, 1);

   /* Next, declare the return parameter as a local variable, initialized
    * to point to the return value.
    */
   ST2C_decl_translate(tokens, PUINFO_RETURN_PARAM, context);
   Append_Token_Special(tokens, '=');
   Append_Token_Special(tokens, '&');
   Append_Token_String(tokens, WN2C_Return_Value_Name);
   Append_Token_Special(tokens, ';');
   Append_Indented_Newline(tokens, 1);
} /* WN2C_Declare_Return_Parameter */


static void
WN2C_Store_Return_Reg(TOKEN_BUFFER tokens,
		      const char  *var_name,
		      STAB_OFFSET  var_offset,
		      MTYPE        preg_mtype,
		      PREG_IDX     preg_offset,
		      CONTEXT      context)
{
   /* Store the preg value into the given variable at the given
    * offset.
    */
   const TY_IDX preg_ty = Stab_Mtype_To_Ty(preg_mtype);
   TOKEN_BUFFER tmp_tokens = New_Token_Buffer();
   STATUS       status = EMPTY_STATUS;
   
   /* Cast the lhs to the type of the preg, and dereference the
    * resultant address.
    */
   Append_Token_String(tmp_tokens, var_name);
   status = WN2C_based_lvalue(tmp_tokens,   /* base variable name */
			      TRUE,         /* tokens represent an lvalue */
			      Stab_Pointer_To(PUINFO_RETURN_TY),
			                    /* type of base address */
			      Stab_Pointer_To(PUINFO_RETURN_TY),
			                    /* expected type of base address */
			      preg_ty,      /* expected type of object */
			      var_offset);  /* offset of object from base */

   if (!STATUS_is_lvalue(status))
      Prepend_Token_Special(tmp_tokens, '*');
   Append_And_Reclaim_Token_List(tokens, &tmp_tokens);
   
   /* Assign the preg to the lhs */
   Append_Token_Special(tokens, '=');
   ST2C_Use_Preg(tokens, preg_ty, preg_offset, context);
   Append_Token_Special(tokens, ';');
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
} /* WN2C_Store_Return_Reg */


static void
WN2C_Load_Return_Reg(TOKEN_BUFFER tokens,
		     TY_IDX       return_ty,
		     const char  *var_name,
		     STAB_OFFSET  var_offset,
		     MTYPE        preg_mtype,
		     PREG_IDX     preg_offset,
		     CONTEXT      context)
{
   /* Load a preg value from the given variable at the given offset
    * from the base-address of the variable.
    */
   const TY_IDX preg_ty = Stab_Mtype_To_Ty(preg_mtype);
   TOKEN_BUFFER tmp_tokens = New_Token_Buffer();
   STATUS       status = EMPTY_STATUS;

   /* Cast the rhs to the type of the preg, and dereference the
    * resultant address.
    */
   Append_Token_String(tmp_tokens, var_name);
   status = WN2C_based_lvalue(tmp_tokens,   /* base variable name */
			      TRUE,         /* tokens represent an lvalue */
			      Stab_Pointer_To(return_ty),
			                    /* type of base address */
			      Stab_Pointer_To(return_ty),
			                    /* expected type of base address */
			      preg_ty,      /* expected type of object */
			      var_offset);  /* offset of object from base */

   if (!STATUS_is_lvalue(status))
      Prepend_Token_Special(tmp_tokens, '*');
   
   /* Assign the variable to the preg */
   ST2C_Use_Preg(tokens, preg_ty, preg_offset, context);
   Append_Token_Special(tokens, '=');
   Append_And_Reclaim_Token_List(tokens, &tmp_tokens);

} /* WN2C_Load_Return_Reg */


static void
WN2C_Function_Call_Lhs(TOKEN_BUFFER rhs_tokens,  /* The function call */
		       BOOL         parm_return, /* Return through parameter */
		       TY_IDX       return_ty,   /* The function return type */
		       const WN    *first_arg,   /* First arg to function */
		       CONTEXT      context)
{
   /* PRECONDITION: return_ty != Void_Type.
    * Append to the rhs_tokens any assignments necessary to retrieve
    * the function-call return value into a temporary variable or 
    * into return registers, unless no use of a return register
    * fitting the return_ty is found.
    */
   TOKEN_BUFFER             lhs_tokens = New_Token_Buffer();
   STAB_OFFSET              value_offset;
   STATUS                   status;
   BOOL                     return_value_is_used = TRUE;
   RETURN_PREG              return_info = PUinfo_Get_ReturnPreg(return_ty);
   const RETURN_PREG *const return_info_ptr = &return_info;
   
   if (parm_return)
   {
      /* Return through a parameter:  Assign the call-value to
       * the dereferenced implicit argument expression (first_arg).
       */
      status = WN2C_lvalue_wn(lhs_tokens, 
			      first_arg,
			      WN_Tree_Type(first_arg),
			      return_ty,
			      0,    /* no offset */
			      context);
      if (!STATUS_is_lvalue(status))
	 Prepend_Token_Special(lhs_tokens, '*');
   }
   else /* Return through a preg; the usual case */
   {
      const ST *result_var   = CALLSITE_return_var(WN2C_Prev_CallSite);
      const WN *result_store = CALLSITE_store1(WN2C_Prev_CallSite);
      MTYPE     preg_mtype   = RETURN_PREG_mtype(return_info_ptr, 0);
      PREG_IDX  preg_offset  = RETURN_PREG_offset(return_info_ptr, 0);

      /* preg_mtype and preg_offset holds information about the first
       * return register.  Now generate code for the location into which
       * the call-value should be put.
       */
      if (preg_mtype == MTYPE_V)
      {
	 /* The resultant value from the call is not used anywhere! */
	 return_value_is_used = FALSE;
      }
      else if (result_var != NULL)
      {
	//WEI: if result of function call is void*, it's presented as UINT64,
	//and we need to cast it to the appropriate pointer type on lhs
	if (TY_kind(return_ty) == KIND_SCALAR && TY_mtype(return_ty) == Pointer_Mtype &&
	    TY_kind(ST_type(result_var)) == KIND_POINTER) {
	  WN2C_prepend_cast(rhs_tokens, ST_type(result_var), false);
	}

	 STAB_OFFSET var_offset = CALLSITE_var_offset(WN2C_Prev_CallSite);
	 TY_IDX      var_ty = ST_type(result_var);

	 Is_True(!CALLSITE_in_regs(WN2C_Prev_CallSite),
		 ("Encountered unexpected reference to a return register"));
	 
	 /* Return to a variable or to a preg, as was determined by
	  * the analysis (pattern matching) in PUinfo_init_pu().  We
	  * assign the value directly to the variable/register, without
	  * referencing the return registers.
	  */
	 if (ST_sym_class(result_var) == CLASS_PREG)
	 {
	    Is_True(RETURN_PREG_num_pregs(return_info_ptr) == 1,
		    ("Unexpected number of call-value save registers"));

	    var_ty = PUinfo_Preg_Type(var_ty, var_offset);
	    ST2C_Use_Preg(lhs_tokens, 
			  var_ty,
			  var_offset,
			  context);
	    if (!WN2C_assignment_compatible_types(var_ty, return_ty))
	    {
	       WN2C_prepend_cast(rhs_tokens, 
				 var_ty,  /* cast rhs to type of var */
				 FALSE/*pointer_to_type*/);
	    }
	 }
	 else
	 {
	    status = WN2C_lvalue_st(lhs_tokens,
				    result_var,              /* base address */
				    Stab_Pointer_To(var_ty), /* base type */
				    ST_type(result_var),
				    //return_ty, /* type object to be loaded */
				    var_offset,
				    context);
	    if (!STATUS_is_lvalue(status))
	       Prepend_Token_Special(lhs_tokens, '*');
	 }
      }
      else if (result_store != NULL)
      {
	 TY_IDX base_ty;
	 TY_IDX stored_ty;
	 
	 /* We have a store into an lvalue that is not a variable, so
	  * it must be an OPR_ISTORE node with the rhs being an LDID
	  * of the return register.  Do the exact same thing as would
	  * be done in translating the ISTORE, but substitute the rhs
	  * with the call expression.
	  */
	 Is_True(WN_operator(result_store) == OPR_ISTORE &&
		 WN_operator(WN_kid0(result_store)) == OPR_LDID, 
		 ("Unexpected store1 in WN2C_Function_Call_Lhs()"));

	 Is_True(!CALLSITE_in_regs(WN2C_Prev_CallSite),
		 ("Encountered unexpected reference to a return register"));
	 
	 /* Get the type of object being stored */
	 base_ty = WN_Tree_Type(WN_kid1(result_store));
	 if (!TY_Is_Pointer(base_ty))
	    base_ty = WN_ty(result_store);
	 stored_ty = TY_pointed(WN_ty(result_store));
	 stored_ty =
	    WN2C_MemAccess_Type(TY_pointed(base_ty),         /* base ty */
				stored_ty,                   /* preferred ty */
				WN_opc_dtype(result_store),  /* required mty */
				WN_store_offset(result_store)); /* base offs */
   
	 status = WN2C_lvalue_wn(lhs_tokens,
				 WN_kid1(result_store), /* lhs of ISTORE */
				 base_ty,               /* type of lhs */
				 stored_ty,             /* type to be stored */
				 WN_store_offset(result_store), /* lhs offs */
				 context);
	 if (!STATUS_is_lvalue(status))
	    Prepend_Token_Special(lhs_tokens, '*');

	 if (!WN2C_assignment_compatible_types(stored_ty, return_ty))
	    WN2C_prepend_cast(rhs_tokens, stored_ty, FALSE/*ptr_to_type*/);
      }
      else if (!CALLSITE_in_regs(WN2C_Prev_CallSite))
      {
	 /* The return registers are not referenced, so do not bother
	  * assigning the return value to the return registers.
	  */
	 return_value_is_used = FALSE;
      }
      else if (RETURN_PREG_num_pregs(return_info_ptr) == 1 && 
	       TY_Is_Preg_Type(return_ty))
      {
	 TY_IDX preg_ty = 
	    PUinfo_Preg_Type(Stab_Mtype_To_Ty(preg_mtype), preg_offset);

	 /* There is a single return register holding the return value,
	  * so return the rhs into this register, after casting the rhs
	  * to the appropriate type.
	  */
	 ST2C_Use_Preg(lhs_tokens, preg_ty, preg_offset, context);
	 if (!WN2C_assignment_compatible_types(preg_ty, return_ty))
	 {
	    WN2C_prepend_cast(rhs_tokens,
			      preg_ty,
			      FALSE/*pointer_to_type*/);
	 }
      }
      else /* Our most difficult case */
      {
	 /* We need to store the call-result into a temporary variable,
	  * then save the temporary variable into the return registers.
	  */
	 const UINT  tmp_idx = Stab_Lock_Tmpvar(return_ty, 
						ST2C_Declare_Tempvar);
	 const char *tmpvar_name = W2CF_Symtab_Nameof_Tempvar(tmp_idx);
	 
	 /* The lhs is simply the tmpvar */
	 Append_Token_String(lhs_tokens, tmpvar_name);

	 /* Load the first register off the tmpvar_name after assigning the
	  * rhs to the lhs.
	  */
	 Append_Token_Special(rhs_tokens, ';');
	 WN2C_Stmt_Newline(rhs_tokens, CONTEXT_srcpos(context));
	 WN2C_Load_Return_Reg(rhs_tokens,
			      return_ty, /* Type of tmpvar_name */
			      tmpvar_name, 
			      0,         /* Offset in tmpvar_name */
			      preg_mtype,
			      preg_offset,
			      context);
	    
	 if (RETURN_PREG_num_pregs(return_info_ptr) > 1)
	 {
	    /* Get the offset into the value from which the second preg
	     * needs to be loaded.
	     */
	    value_offset = TY_size(Stab_Mtype_To_Ty(preg_mtype));

	    /* Load the second register */
	    Append_Token_Special(rhs_tokens, ';');
	    WN2C_Stmt_Newline(rhs_tokens, CONTEXT_srcpos(context));
	    preg_offset = RETURN_PREG_offset(return_info_ptr, 1);
	    preg_mtype = RETURN_PREG_mtype(return_info_ptr, 1);
	    WN2C_Load_Return_Reg(rhs_tokens,
				 return_ty,   /* Type of tmpvar_name */
				 tmpvar_name, 
				 value_offset, /* Offset in tmpvar_name */
				 preg_mtype, 
				 preg_offset, 
				 context);
	 } /* if save call-value into both return pregs */

	 Stab_Unlock_Tmpvar(tmp_idx);

      } /* if return into registers */
   } /* if return into parameter */
      
   /* Assign the lhs to the rhs */
   if (return_value_is_used)
   {
      Prepend_Token_Special(rhs_tokens, '=');
      Prepend_And_Reclaim_Token_List(rhs_tokens, &lhs_tokens);
   }
   else
   {
      Reclaim_Token_Buffer(&lhs_tokens);
   }   
} /* WN2C_Function_Call_Lhs */


static void
WN2C_Function_Return_Value(TOKEN_BUFFER tokens, /* Statements before return */
			   CONTEXT context)
{
   /* PRECONDITION: return_ty != Void_Type.
    * Append any necessary assignments and a return statement for the
    * return value to the given token-list.
    */
   TOKEN_BUFFER value_tokens;
   STAB_OFFSET  value_offset;
   STATUS       status;

   value_tokens = New_Token_Buffer();
   if (PUINFO_RETURN_TO_PARAM)
   {
      /* Return through a parameter.  This paremeter will be
       * declared as a local variable in WN2C_block().
       */
      Append_Token_String(value_tokens, WN2C_Return_Value_Name);
      WN2C_Used_Return_Value = TRUE;
   }
   else /* Return through a preg; the usual case */
   {
      MTYPE     preg_mtype;
      PREG_IDX  preg_offset;
      const ST *result_var = RETURNSITE_return_var(WN2C_Next_ReturnSite);
      const WN *result_store = RETURNSITE_store1(WN2C_Next_ReturnSite);

      /* Get the offset and mtype of the first return register */
      preg_offset = RETURN_PREG_offset(PUinfo_return_preg, 0);
      preg_mtype = RETURN_PREG_mtype(PUinfo_return_preg, 0);

      /* Now that we have information about the distribution of the
       * return values in registers and how these registers are set
       * by means of STIDs (possibly from a variable), generate
       * code for the return value.
       */
      if (result_var != NULL)
      {
	 STAB_OFFSET var_offset = RETURNSITE_var_offset(WN2C_Next_ReturnSite);
	 TY_IDX      var_ty = ST_type(result_var);

	 /* Pattern matching in PUinfo_init_pu() revealed that the
	  * return value is present in a variable/non-return-register.
	  * Just return the value of this variable/register.
	  */
	 if (ST_sym_class(result_var) == CLASS_PREG)
	 {
	    Is_True(RETURN_PREG_num_pregs(PUinfo_return_preg) == 1,
		    ("Unexpected number of return-value save registers"));

	    var_ty = PUinfo_Preg_Type(var_ty, var_offset);
	    ST2C_Use_Preg(value_tokens, 
			  var_ty,
			  var_offset,
			  context);
	    if (!WN2C_assignment_compatible_types(PUINFO_RETURN_TY, var_ty))
	    {
	       WN2C_prepend_cast(value_tokens, 
				 PUINFO_RETURN_TY,
				 FALSE/*pointer_to_type*/);
	    }
	 }
	 else
	 {
	    status = WN2C_lvalue_st(value_tokens,
				    result_var,             /* base variable */
				    Stab_Pointer_To(var_ty),/* base addr ty */
				    PUINFO_RETURN_TY, /* type object loaded */
				    var_offset,
				    context);
	    if (!STATUS_is_lvalue(status))
	       Prepend_Token_Special(value_tokens, '*');
	 }
      }
      else if (result_store != NULL)
      {
	 /* We have a store into the return register, so just return
	  * the rhs of the store (should be an STID).
	  */
	 Is_True(WN_operator(result_store) == OPR_STID, 
		 ("Unexpected store1 in WN2C_Function_Return_Value()"));
	 
	 /* Get the type of object being stored */
	 CONTEXT_reset_needs_lvalue(context);
	 CONTEXT_set_top_level_expr(context);
	 status = WN2C_translate(value_tokens, WN_kid0(result_store), context);
	 if (!WN2C_assignment_compatible_types(PUINFO_RETURN_TY,
		                      WN_Tree_Type(WN_kid0(result_store))))
	 {
	    WN2C_prepend_cast(value_tokens,
			      PUINFO_RETURN_TY,
			      FALSE/*ptr_to_type*/);
	 }
      }
      else if (RETURN_PREG_num_pregs(PUinfo_return_preg) == 1 &&
	       TY_Is_Preg_Type(PUINFO_RETURN_TY))
      {
	 TY_IDX preg_ty =
	    PUinfo_Preg_Type(Stab_Mtype_To_Ty(preg_mtype), preg_offset);

	 /* There is a single return register holding the return value,
	  * so return a reference to this register casted to the 
	  * appropriate type.
	  */
	 ST2C_Use_Preg(value_tokens, preg_ty, preg_offset, context);
	 if (!WN2C_assignment_compatible_types(PUINFO_RETURN_TY, preg_ty))
	 {
	    WN2C_prepend_cast(value_tokens,
			      PUINFO_RETURN_TY, 
			      FALSE/*pointer_to_type*/);
	 }
      }
      else /* Our most difficult case */
      {
	 /* We need to store the return registers into a temporary 
	  * variable, then return the value of this variable.
	  */
	    
	 /* Store the first register */
	 WN2C_Store_Return_Reg(tokens,
			       WN2C_Return_Value_Name, 
			       0, /* offset in return value */
			       preg_mtype,
			       preg_offset, 
			       context);
	    
	 if (RETURN_PREG_num_pregs(PUinfo_return_preg) > 1)
	 {
	    /* Get the offset into the value where the second preg
	     * needs to be stored.
	     */
	    value_offset = TY_size(Stab_Mtype_To_Ty(preg_mtype));

	    /* Store the second register */
	    preg_offset = RETURN_PREG_offset(PUinfo_return_preg, 1);
	    preg_mtype = RETURN_PREG_mtype(PUinfo_return_preg, 1);
	    WN2C_Store_Return_Reg(tokens,
				  WN2C_Return_Value_Name, 
				  value_offset, /* offset in return value */
				  preg_mtype, 
				  preg_offset,
				  context);
	 } /* if return two pregs */
	 Append_Token_String(value_tokens, WN2C_Return_Value_Name);
	 WN2C_Used_Return_Value = TRUE;
      } /* if return through existing variable */
   } /* if return through parameter */

   Append_Token_String(tokens, "return");
   Append_And_Reclaim_Token_List(tokens, &value_tokens);

} /* WN2C_Function_Return_Value */


static void
WN2C_Append_Pragma_Newline(TOKEN_BUFFER tokens, SRCPOS srcpos)
{
   UINT current_indent = Current_Indentation();

   Set_Current_Indentation(0);
   WN2C_Stmt_Newline(tokens, srcpos);
   Set_Current_Indentation(current_indent);
} /* WN2C_Append_Pragma_Newline */


static void
WN2C_Callsite_Directives(TOKEN_BUFFER tokens, 
			 const WN    *call_wn,
			 const ST    *func_st,
			 CONTEXT      context)
{
   if (WN_Call_Inline(call_wn))
   {
      WN2C_Append_Pragma_Newline(tokens, CONTEXT_srcpos(context));
      Append_Token_String(tokens, "#pragma");
      Append_Token_String(tokens, "inline");
      Append_Token_Special(tokens, '(');
      ST2C_use_translate(tokens, func_st, context);
      Append_Token_Special(tokens, ')');
   }
   else if (WN_Call_Dont_Inline(call_wn))
   {
      WN2C_Append_Pragma_Newline(tokens, CONTEXT_srcpos(context));
      Append_Token_String(tokens, "#pragma");
      Append_Token_String(tokens, "noinline");
      Append_Token_Special(tokens, '(');
      ST2C_use_translate(tokens, func_st, context);
      Append_Token_Special(tokens, ')');
   }
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
} /* WN2C_Callsite_Directives */


static void
WN2C_Translate_Stmt_Sequence(TOKEN_BUFFER  tokens, 
			     const WN     *first,
			     BOOL          first_on_newline,
			     CONTEXT       context)
{
   /* Translate the sequence of statements listed at "first",
    * separating the statements with newline characters.
    */
   const WN *stmt;
   STATUS    status;

   for (stmt = first; stmt != NULL; stmt = WN_next(stmt))
   {
      if (!WN2C_Skip_Stmt(stmt))
      {
	 /* Seperate stmts by newlines */
	 CONTEXT_set_srcpos(context, WN_Get_Linenum(stmt));
	 if (first_on_newline || stmt != first)
	 {
	     if (WN_operator(stmt) == OPR_DO_LOOP ||
		 WN_operator(stmt) == OPR_WHILE_DO ||
		 WN_operator(stmt) == OPR_DO_WHILE ||
		 WN_operator(stmt) == OPR_CALL ||
		 WN_operator(stmt) == OPR_PICCALL ||
		 WN_operator(stmt) == OPR_REGION ||
		 WN_operator(stmt) == OPR_PRAGMA ||
		 WN_operator(stmt) == OPR_XPRAGMA && 
		 WN_operator(stmt) == OPR_TRAP ||
		 WN_operator(stmt) == OPR_ASSERT ||
		 WN_operator(stmt) == OPR_FORWARD_BARRIER ||
		 WN_operator(stmt) == OPR_BACKWARD_BARRIER)
             {
                /* Special newlines are added by handler routines.
                 */
             }
	     else
		WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
	 }
	 status = WN2C_translate(tokens, stmt, context);
	 if (!STATUS_is_block(status)      && 
	     WN_operator(stmt) != OPR_REGION &&
	     WN_operator(stmt) != OPR_PRAGMA &&
	     WN_operator(stmt) != OPR_XPRAGMA &&
	     WN_operator(stmt) != OPR_TRAP &&
	     WN_operator(stmt) != OPR_ASSERT &&
	     WN_operator(stmt) != OPR_FORWARD_BARRIER &&
	     WN_operator(stmt) != OPR_BACKWARD_BARRIER)
	    Append_Token_Special(tokens, ';');

	 /* Append frequency feedback info in a comment
	  */
	 if (W2C_Emit_Frequency                         && 
	     W2C_Frequency_Map != WN_MAP_UNDEFINED      &&
	     WN_MAP32_Get(W2C_Frequency_Map, stmt) >= 0 &&
	     WN_operator(stmt) != OPR_REGION              &&
	     WN_operator(stmt) != OPR_PRAGMA              &&
	     WN_operator(stmt) != OPR_XPRAGMA             &&
	     WN_operator(stmt) != OPR_TRAP                &&
	     WN_operator(stmt) != OPR_ASSERT              &&
	     WN_operator(stmt) != OPR_FORWARD_BARRIER     &&
	     WN_operator(stmt) != OPR_BACKWARD_BARRIER)
	 {
	    INT32 freq = WN_MAP32_Get(W2C_Frequency_Map, stmt);
	    Append_Token_String(tokens, "  /*FREQ=");
	    Append_Token_String(tokens, Number_as_String(freq,"%lld"));
	    Append_Token_String(tokens, "*/");
	 }
      }
   } /*for*/
} /* WN2C_Translate_Stmt_Sequence */


static void
WN2C_Translate_Comma_Sequence(TOKEN_BUFFER  tokens, 
			      const WN     *first,
			      CONTEXT       context)
{
   /* Translate the sequence of statements listed at "first",
    * separating the statements with comma characters.
    */
   const WN *stmt;

   for (stmt = first; stmt != NULL; stmt = WN_next(stmt))
   {
      if (!WN2C_Skip_Stmt(stmt))
      {
	 /* Seperate stmts by commas */
	 CONTEXT_set_srcpos(context, WN_Get_Linenum(stmt));
	 if (stmt != first)
	 {
	    Append_Token_Special(tokens, ',');
	 }
	 WN2C_translate(tokens, stmt, context);
      }
   } /*for*/
} /* WN2C_Translate_Comma_Sequence */


void
WN2C_Append_Purple_Funcinfo(TOKEN_BUFFER tokens)
{
   const char *name   = W2C_Object_Name(PUINFO_FUNC_ST);
   ST_IDX      id     = PUINFO_FUNC_ST_IDX;
   ST_SCLASS   sclass = ST_sclass(PUINFO_FUNC_ST);
   ST_EXPORT   export_class = (ST_EXPORT)ST_export(PUINFO_FUNC_ST);

   Append_Token_String(tokens, name);
   Append_Token_Special(tokens, ',');
   if (strcmp(name, WN2C_Purple_Region_Name) == 0)
   {
      /* This must match the setting in PRP_TRACE_READER::_Read_Next()
       * when a region is entered.
       */
      id = 0xffffffff;
      sclass = SCLASS_TEXT;
      export_class = EXPORT_INTERNAL;
   }
   Append_Token_String(tokens, Number_as_String(id, "%llu"));
   Append_Token_Special(tokens, ',');
   Append_Token_String(tokens, Number_as_String(sclass, "%lld"));
   Append_Token_Special(tokens, ',');
   Append_Token_String(tokens, Number_as_String(export_class, "%lld"));
   Append_Token_Special(tokens, ',');
   Append_Token_String(tokens, "0"); /* Flags */
} /* WN2C_Append_Purple_Funcinfo */


static void
Append_Cplus_Initialization(TOKEN_BUFFER tokens, CONTEXT context)
{
   Append_Token_String(tokens, "/* C++ specific initialization */");
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
   Append_Token_String(tokens,
		       "if (__cplinit.ctor != NULL) __cplinit.ctor();");
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
} /* Append_Cplus_Initialization */


/*---------------------- Prefetching Comments ------------------------*/
/*--------------------------------------------------------------------*/

static void
WN2C_Prefetch_Map(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   PF_POINTER* pfptr;
   const char *info_str;

   pfptr = (PF_POINTER*)WN_MAP_Get(WN_MAP_PREFETCH, wn);
   info_str = "/* prefetch (ptr, lrnum): ";
   if (pfptr->wn_pref_1L)
   {
      info_str = 
	 Concat2_Strings(    info_str,
          Concat2_Strings(   "1st <", 
           Concat2_Strings(  Ptr_as_String(pfptr->wn_pref_1L),
            Concat2_Strings( ", ",
             Concat2_Strings(Number_as_String(pfptr->lrnum_1L,"%lld"),
			     "> ")))));
   }
   if (pfptr->wn_pref_2L)
   {
      info_str = 
	 Concat2_Strings(    info_str,
          Concat2_Strings(   "2nd <", 
           Concat2_Strings(  Ptr_as_String(pfptr->wn_pref_2L),
            Concat2_Strings( ", ",
             Concat2_Strings(Number_as_String(pfptr->lrnum_2L,"%lld"),
			     "> ")))));
   }
   info_str = Concat2_Strings(info_str, "*/");
   Append_Indented_Newline(tokens, 1);
   Append_Token_String (tokens, info_str);
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
} /* WN2C_Prefetch_Map */


/*--------------------- prompf processing utilities -------------------*/
/*---------------------------------------------------------------------*/

static void
WN2C_Append_Prompf_Flag_Newline(TOKEN_BUFFER tokens)
{
   UINT current_indent = Current_Indentation();

   Set_Current_Indentation(0);
   Append_Indented_Newline(tokens, 1);
   Set_Current_Indentation(current_indent);
} /* WN2C_Append_Prompf_Flag_Newline */


static BOOL
WN2C_Is_Loop_Region(const WN *region, CONTEXT context)
{
   /* Return TRUE if the given region is either a DOACROSS,
    * PARALLEL_DO, or a PDO region; otherwise, return FALSE.
    */
   BOOL predicate = (WN_operator(region) == OPR_REGION);

   if (predicate)
   {
      const WN *pragma = WN_first(WN_region_pragmas(region));

      predicate = (pragma != NULL &&
		   (WN_pragma(pragma) == WN_PRAGMA_DOACROSS    ||
		    WN_pragma(pragma) == WN_PRAGMA_PARALLEL_DO ||
		    WN_pragma(pragma) == WN_PRAGMA_PDO_BEGIN) &&
		   WN_pragma_nest(pragma) <= 0 &&
		   !Ignore_Synchronized_Construct(pragma, context));
   }
   return predicate;
} /* WN2C_Is_Loop_Region */


static BOOL
WN2C_Is_Parallel_Region(const WN *region, CONTEXT context)
{
   BOOL predicate = (region != NULL && WN_operator(region) == OPR_REGION);

   if (predicate)
   {
      const WN *pragma = WN_first(WN_region_pragmas(region));

      predicate = (pragma != NULL && 
		   (WN_pragma(pragma) == WN_PRAGMA_PARALLEL_BEGIN || 
		    WN_pragma(pragma) == WN_PRAGMA_MASTER_BEGIN || 
		    WN_pragma(pragma) == WN_PRAGMA_SINGLE_PROCESS_BEGIN ||
		    WN_pragma(pragma) == WN_PRAGMA_PSECTION_BEGIN ||
		    WN_pragma(pragma) == WN_PRAGMA_PARALLEL_SECTIONS) &&
		   !Ignore_Synchronized_Construct(pragma, context));
   }   
   return predicate;
} /* WN2C_Is_Parallel_Region */


static void
WN2C_Prompf_Construct_Start(TOKEN_BUFFER tokens, const WN *construct)
{
   INT32 construct_id = WN_MAP32_Get(*W2C_Construct_Map, construct);

   if (construct_id != 0)
   {
      WN2C_Append_Prompf_Flag_Newline(tokens);
      Append_Token_String(tokens, "/*$SGI");
      Append_Token_String(tokens, "start");
      Append_Token_String(tokens, Number_as_String(construct_id, "%llu"));
      Append_Token_String(tokens, "*/");
   }
} /* WN2C_Prompf_Construct_Start */


static void
WN2C_Prompf_Construct_End(TOKEN_BUFFER tokens, const WN *construct)
{
   INT32 construct_id = WN_MAP32_Get(*W2C_Construct_Map, construct);

   if (construct_id != 0)
   {
      WN2C_Append_Prompf_Flag_Newline(tokens);
      Append_Token_String(tokens, "/*$SGI");
      Append_Token_String(tokens, "end");
      Append_Token_String(tokens, Number_as_String(construct_id, "%llu"));
      Append_Token_String(tokens, "*/");
   }
} /* WN2C_Prompf_Construct_End */


static void
WN2C_Start_Prompf_Transformed_Loop(TOKEN_BUFFER tokens,
				   const WN    *loop,
				   CONTEXT      context)
{
   /* We if this is a DOACROSS, PDO or PARALLEL DO loop, then it will
    * already have been handled by the sourrounding region, so we need
    * not emit anything here.  Otherwise, emit the prompf flags.
    */
   if (!WN2C_Is_Loop_Region(W2CF_Get_Parent(W2CF_Get_Parent(loop)), context))
      WN2C_Prompf_Construct_Start(tokens, loop);
} /* WN2C_Start_Prompf_Transformed_Loop */


static void
WN2C_End_Prompf_Transformed_Loop(TOKEN_BUFFER tokens,
				 const WN    *loop,
				 CONTEXT      context)
{
   /* We if this is a DOACROSS, PDO or PARALLEL DO loop, then it will
    * already have been handled by the sourrounding region, so we need
    * not emit anything here.  Otherwise, emit the prompf flags.
    */
   if (!WN2C_Is_Loop_Region(W2CF_Get_Parent(W2CF_Get_Parent(loop)), context))
      WN2C_Prompf_Construct_End(tokens, loop);
} /* WN2C_End_Prompf_Transformed_Loop */


static void
WN2C_Start_Prompf_Transformed_Region(TOKEN_BUFFER tokens, 
				     const WN    *region,
				     CONTEXT      context)
{
   /* Handle a PARALLEL region, or a DOACROSS, PDO or PARALLEL DO 
    * loop enclosed in a region.
    */
   if (WN2C_Is_Loop_Region(region, context) || 
       WN2C_Is_Parallel_Region(region, context))
      WN2C_Prompf_Construct_Start(tokens, region);
} /* WN2C_Start_Prompf_Transformed_Region */


static void
WN2C_End_Prompf_Transformed_Region(TOKEN_BUFFER tokens, 
				   const WN    *region,
				   CONTEXT      context)
{
   /* Handle a PARALLEL region, or a DOACROSS, PDO or PARALLEL DO 
    * loop enclosed in a region.
    */
   if (WN2C_Is_Loop_Region(region, context) ||
       WN2C_Is_Parallel_Region(region, context))
      WN2C_Prompf_Construct_End(tokens, region);
} /* WN2C_End_Prompf_Transformed_Region */


/*--------- hidden routines to handle each kind of operator -----------*/
/*---------------------------------------------------------------------*/

static STATUS
WN2C_ignore(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   return EMPTY_STATUS;
} /* WN2C_ignore */


static STATUS
WN2C_unsupported(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   fprintf(stderr, 
	   "==> wn2c cannot handle operator <%s> (%d): construct ignored!\n",
	   WN_opc_name(wn), WN_operator(wn));

   Append_Token_String(tokens, Concat3_Strings("<", WN_opc_name(wn), ">"));
   
   return EMPTY_STATUS;
} /* WN2C_unsupported */


static STATUS
WN2C_binaryop(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   STATUS status;
   
   Is_True(WN_kid_count(wn) == 2, 
     ("Expected 2 kids in WN2C_binaryop for op %s",WN_opc_name(wn)));

   if (WN2C_IS_INFIX_OP(WN_opcode(wn)))
      status = WN2C_infix_op(tokens,
			     WN_opcode(wn),
			     WN_Tree_Type(wn),
			     WN_kid0(wn), 
			     WN_kid1(wn), 
			     context);
   else if (WN2C_IS_FUNCALL_OP(WN_opcode(wn)))
      status = WN2C_funcall_op(tokens, 
			       WN_opcode(wn), 
			       WN_kid0(wn), 
			       WN_kid1(wn), 
			       context);
   else 
      Is_True(FALSE, ("Illegal operator (%s=%d) in WN2C_binaryop()",
		      WN_opc_name(wn), WN_opcode(wn)));

   return status;
} /* WN2C_binaryop */


static STATUS
WN2C_unaryop(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   Is_True(WN_kid_count(wn) == 1, ("Expected 1 kid in WN2C_unaryop()"));

   if (WN2C_IS_INFIX_OP(WN_opcode(wn)))
      WN2C_infix_op(tokens,
		    WN_opcode(wn), 
		    WN_Tree_Type(wn),
		    NULL, /* No first operand */
		    WN_kid0(wn), 
		    context);
   else if (WN2C_IS_FUNCALL_OP(WN_opcode(wn)))
      WN2C_funcall_op(tokens, WN_opcode(wn), NULL, WN_kid0(wn), context);
   else
      Is_True(FALSE, ("Illegal operator (%s=%d) in WN2C_unaryop()",
		      WN_opc_name(wn), WN_opcode(wn)));

   return EMPTY_STATUS;
} /* WN2C_unaryop */


static STATUS
WN2C_func_entry(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* Add tokens for the function header and body to "tokens".  Note
    * that all the tokens will be added to the buffer, while the task
    * of writing the tokens to file and freeing up the buffer is left
    * to the caller.
    *
    * Assume that Current_Symtab has been updated (see bedriver.c).
    * Note that Current_PU is not maintained, but we instead get to
    * it through PUinfo_current_func.
    *
    */
   ST **param_st;
   INT  param;
   
   Is_True(WN_operator(wn) == OPR_FUNC_ENTRY, 
	   ("Invalid opcode for WN2C_func_entry()"));

   /* Set the state to reflect the current PU, assuming PUinfo
    * already is up to date.
    */
   CONTEXT_set_new_func_scope(context);
   WN2C_Used_Return_Value = FALSE;
   WN2C_Next_ReturnSite = PUinfo_Get_ReturnSites();
   WN2C_Prev_CallSite = NULL;
   
   /* Emit the function pragmas before local variables */
   if (!W2C_No_Pragmas)
      WN2C_pragma_list_begin(PUinfo_pragmas, 
                             WN_first(WN_region_pragmas(wn)),
                             context);

   /* Accumulate the parameters and their ST/TY entries in the 
    * "param_st"/"param_ty" array.
    */
   param_st = (ST **)alloca((WN_num_formals(wn) + 1)*sizeof(ST *));
   for (param = 0; param < WN_num_formals(wn); param++)
   {
      Is_True(WN_operator(WN_formal(wn, param)) == OPR_IDNAME, 
	      ("Invalid opcode for parameter of OPR_FUNC_ENTRY"));
      param_st[param] = WN_st(WN_formal(wn, param));
   }
   param_st[WN_num_formals(wn)] = NULL; /* Terminates list of param STs */

   /* Write prompf information */
   if (W2C_Prompf_Emission)
      WN2C_Prompf_Construct_Start(tokens, wn);

   /* Write function header, and begin the body on a new line */
   CONTEXT_set_srcpos(context, WN_Get_Linenum(wn));
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));

   //WEI: don't output the complete type declaration at the function header
   if (Compile_Upc) {
     CONTEXT_set_incomplete_ty2c(context);
   }
   ST2C_func_header(tokens, WN_st(wn), param_st, context);
   
   /* Write out the function body */
   CONTEXT_set_srcpos(context, WN_Get_Linenum(WN_func_body(wn)));
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
   Append_Token_Special(tokens, '{');
   Increment_Indentation();

   //WEI: write the BEGIN_FUNCTION
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
   if (Compile_Upc) {
     Append_Token_String(tokens, "UPCR_BEGIN_FUNCTION();");
   }

   (void)WN2C_translate(tokens, WN_func_body(wn), context);
   if (!W2C_No_Pragmas)
      WN2C_pragma_list_end(tokens, 
                           WN_first(WN_region_pragmas(wn)),
                           context);
   Decrement_Indentation();
   Append_Indented_Newline(tokens, 1);
   Append_Token_Special(tokens, '}');

   /* Emit the function name inb a comment */
   Append_Token_String(tokens, "/*");
   ST2C_use_translate(tokens, &St_Table[WN_entry_name(wn)], context);
   Append_Token_String(tokens, "*/");

   if (W2C_Prompf_Emission)
      WN2C_Prompf_Construct_End(tokens, wn);

   /* Separate functions by two empty lines */
   Append_Indented_Newline(tokens, 2);

   /* Reset the state to reflect an exit from this PU context */
   WN2C_Prev_CallSite = NULL;
   WN2C_Next_ReturnSite = NULL;
   WN2C_Used_Return_Value = FALSE;

   return EMPTY_STATUS;
} /* WN2C_func_entry */


static STATUS
WN2C_block(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* Do the local declarations (if we have a new symtab) and the
    * list of statements.
    */
   const ST    *st;
   TOKEN_BUFFER stmt_tokens;
   const BOOL   new_func_scope = CONTEXT_new_func_scope(context);
   const BOOL   new_symtab = WN2C_new_symtab();
   UINT         current_indent;
   STATUS       status;
   ST_IDX       st_idx;
   
   Is_True(WN_operator(wn) == OPR_BLOCK,
	   ("Invalid operator for WN2C_block()"));
   
   /* Register Fortran common blocks in st2c, marking the 
    * associated types as having been declared.
    */
   if (new_symtab)
   {
      FOREACH_SYMBOL(CURRENT_SYMTAB, st, st_idx)
      {
	 if (ST_sym_class(st) == CLASS_VAR && 
	     Stab_Is_Common_Block(st)      &&
	     !ST_is_split_common(st))
	 {
	    ST2C_New_Common_Block(st);
	 }
      }
   }

   /* Reset the context so we do not interpret nested scopes as 
    * function-scopes, and save off the proper indentation for local
    * declarations in this scope.
    */
   if (new_func_scope)
   {
      /* The '{' has already been added and the indentation has been changed
       */
      CONTEXT_reset_new_func_scope(context);
      PUinfo_local_decls_indent = Current_Indentation();
   }
   else
   {
      /* Statements and local declarations for this block should be 
       * indented from the surrounding text, but the '{' character
       * should be level with the surrounding text.
       */
      Append_Token_Special(tokens, '{');
      Increment_Indentation();
   }

   /* Emit code for the statements in the block, indented and 
    * separated by a newline character.
    */
   stmt_tokens = New_Token_Buffer();
   if (new_func_scope        &&
       W2C_Cplus_Initializer && 
       (PU_is_mainpu(Pu_Table[ST_pu(PUINFO_FUNC_ST)]) || 
	strcmp(ST_name(PUINFO_FUNC_ST), "main") == 0))
   {
      Append_Cplus_Initialization(stmt_tokens, context);
   }
   CONTEXT_set_top_level_expr(context);
   if (WN_first(wn) != NULL)
      WN2C_Translate_Stmt_Sequence(
	 stmt_tokens, WN_first(wn), TRUE/*first_on_newline*/, context);

   /* Declare the identifiers local to this block, in the current
    * function scope context, when the Current_Symtab has changed.
    * We need to do this after traversing the statements to limit
    * the declarations to local variables that have been marked as
    * actually having been referenced.
    */
   if (new_symtab)
   {
      current_indent = Current_Indentation();
      Set_Current_Indentation(PUinfo_local_decls_indent);
      WN2C_Append_Symtab_Consts(NULL, /* token_buffer */ 
				FALSE, /*use const_tab*/
				1,    /* lines between decls */
				context);
      WN2C_Append_Symtab_Vars(PUinfo_local_decls, 1/*Newlines*/, context);
      Set_Current_Indentation(current_indent);

      /* Later compiler stages may use the ST_referenced flag,
       * expecting it to be FALSE by default, hence we reset
       * the flag after we have used it here.
       */
      Stab_Reset_Referenced_Flag(CURRENT_SYMTAB);
   }

   /* Append the declaration for the return variable and all the
    * local variables accumulated for this context, when this is 
    * a function body.
    */
   if (new_func_scope)
   {
      current_indent = Current_Indentation();
      Set_Current_Indentation(PUinfo_local_decls_indent);
      if (PUINFO_RETURN_TO_PARAM)
	 WN2C_Declare_Return_Parameter(PUinfo_local_decls, context);
      else if (WN2C_Used_Return_Value)
	 WN2C_Declare_Return_Variable(PUinfo_local_decls);
      Set_Current_Indentation(current_indent);

      /* Append local declarations to "tokens" */
      Append_Indented_Newline(tokens, 1);
      Append_And_Reclaim_Token_List(tokens, &PUinfo_local_decls);

      if (!Is_Empty_Token_Buffer(PUinfo_pragmas))
         Append_Indented_Newline(PUinfo_pragmas, 1);
      Append_And_Reclaim_Token_List(tokens, &PUinfo_pragmas);
      
      /* If this is a purple code-extraction, insert a placeholder
       * for purple-specific initialization.
       */
      if (W2C_Purple_Emission)
      {
	 /* <#PRP_XSYM:INIT_DECL name, id, sclass, export#>
	  */
	 Append_Indented_Newline(tokens, 1);
	 Append_Token_String(tokens, "<#PRP_XSYM:INIT_DECL");
	 WN2C_Append_Purple_Funcinfo(tokens);
	 Append_Token_String(tokens, "#>");
      }
   }
   
   /* Append the statements to the tokens */
   Append_And_Reclaim_Token_List(tokens, &stmt_tokens);

   if (new_func_scope && W2C_Purple_Emission &&
       strcmp(W2C_Object_Name(PUINFO_FUNC_ST), WN2C_Purple_Region_Name) == 0)
   {
      /* <#PRP_XSYM:TEST name, id, sclass, export#>
       */
      Append_Indented_Newline(tokens, 1);
      Append_Token_String(tokens, "<#PRP_XSYM:TEST");
      WN2C_Append_Purple_Funcinfo(tokens);
      Append_Token_String(tokens, "#>");
   }

   /* The curly brackets for a scope are not indented, so recover
    * the previous indentation before emitting the '}' token.
    */
   if (!new_func_scope)
   {
      Decrement_Indentation();
      Append_Indented_Newline(tokens, 1);
      Append_Token_Special(tokens, '}');
   }

   status = EMPTY_STATUS;
   STATUS_set_block(status);
   return status;
} /* WN2C_block */


static STATUS
WN2C_region(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* Emit region #pragma, the WN_region_pragmas, and the 
    * WN_region_body.
    */
   RID *rid;
   BOOL good_rid; 

   Is_True(WN_operator(wn) == OPR_REGION, 
	   ("Invalid operator for WN2C_region()"));

   Is_True(WN_operator(WN_region_body(wn)) == OPR_BLOCK, 
	   ("Expected OPR_BLOCK as body of OPR_REGION in WN2C_region()"));

   if (W2C_Prompf_Emission)
      WN2C_Start_Prompf_Transformed_Region(tokens, wn, context);

   good_rid = RID_map >= 0; 
   if (good_rid) 
     rid = (RID *)WN_MAP_Get(RID_map, wn);
   if (W2C_Emit_All_Regions ||
       (!W2C_No_Pragmas && good_rid && 
        (rid == NULL          ||             /* == RID_TYPE_pragma */
         RID_type(rid) == RID_TYPE_pragma))) /* User defined region */
   {
      WN2C_Append_Pragma_Newline(tokens, CONTEXT_srcpos(context));
      Append_Token_String(tokens, "#pragma");
      Append_Token_String(tokens, "region_begin");

      /* Emit the pragmas that are associated with regions and that have
       * a corresponding pragma in the source language.
       */
      if (!W2C_No_Pragmas)
         WN2C_pragma_list_begin(tokens, 
                                WN_first(WN_region_pragmas(wn)),
                                context);
      
      if (WN_first(WN_region_body(wn)) != NULL)
	 WN2C_Translate_Stmt_Sequence(tokens, 
				      WN_first(WN_region_body(wn)), 
				      TRUE/*first_on_newline*/,
				      context);

      if (!W2C_No_Pragmas)
         WN2C_pragma_list_end(tokens, 
                              WN_first(WN_region_pragmas(wn)),
                              context);
   
      WN2C_Append_Pragma_Newline(tokens, WN_Get_Linenum(wn));
      Append_Token_String(tokens, "#pragma");
      Append_Token_String(tokens, "region_end");
   }
   else
   {
      if (!W2C_No_Pragmas)
         WN2C_pragma_list_begin(tokens, 
                                WN_first(WN_region_pragmas(wn)),
                                context);

      /* Emit the body of the region, making the actual region 
       * markings completely transparent.
       */
      if (WN_first(WN_region_body(wn)) != NULL)
	 WN2C_Translate_Stmt_Sequence(tokens, 
				      WN_first(WN_region_body(wn)), 
				      TRUE/*first_on_newline*/,
				      context);

      if (!W2C_No_Pragmas)
         WN2C_pragma_list_end(tokens, 
                              WN_first(WN_region_pragmas(wn)),
                              context);
   } /* if emit pragma */

   if (W2C_Prompf_Emission)
      WN2C_End_Prompf_Transformed_Region(tokens, wn, context);

   return EMPTY_STATUS;
} /* WN2C_region */


static STATUS
WN2C_compgoto(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* The WN_compgoto_num_cases field gives the number of entries in 
    * the jump table.  Kid0 is the switch value, which is a zero-
    * based ordinal pointer into the switch cases represented by 
    * kid1.  Kid1 is a block statement with a goto statement per
    * case.  Kid2 is a lone OPR_GOTO node which gives the default 
    * jump target if the value lies outside the range of the jump
    * table.  If the kid_count is 2, then it is assumed that the 
    * jump target lies within the range of the jump-table, and we 
    * should not access kid2.
    *
    * We generate a switch statement controlled by kid0, with a case
    * for each entry in the switch table represented by kid1.  The
    * switch statement will have a default case corresponding to
    * kid2, unless the kid_count is 2.
    */
   const WN *goto_stmt;
   INT32     goto_entry;
   MTYPE     switch_mty;
   STATUS    status = EMPTY_STATUS;
   OPCODE    myopcode;
   const  WN *compgotoid;
   
   Is_True(WN_operator(wn) == OPR_COMPGOTO,
	   ("Invalid operator for WN2C_compgoto()"));

   /* Emit the switch control */
   /* FMZ-fix bug for switch based on a field of structure-type variable*/ 
  //  switch_mty = TY_mtype(WN_Tree_Type(WN_compgoto_idx(wn)));
   compgotoid = WN_compgoto_idx(wn);
   myopcode   = WN_opcode(compgotoid); 
   if (OPCODE_has_field_id(myopcode) && WN_field_id(compgotoid)) 
         switch_mty = WN_rtype(compgotoid);
   else
         switch_mty = TY_mtype(WN_Tree_Type(compgotoid));

   Append_Token_String(tokens, "switch");
   Append_Token_Special(tokens, '(');
   (void)WN2C_translate(tokens, WN_compgoto_idx(wn), context);
   Append_Token_Special(tokens, ')');
   
   /* Emit the beginning of the switch body */
   Append_Indented_Newline(tokens, 1);
   Append_Token_Special(tokens, '{');
   Append_Indented_Newline(tokens, 1);

   /* Emit each of the cases, possibly followed by a default case */
   goto_stmt = WN_first(WN_compgoto_table(wn));
   for (goto_entry = 0; goto_entry < WN_compgoto_num_cases(wn); goto_entry++)
   {
      Is_True(WN_operator(goto_stmt) == OPR_GOTO,
	      ("Expected each COMPGOTO case to be an OPR_GOTO"));
      Append_Token_String(tokens, "case");
      TCON2C_translate(tokens, Host_To_Targ(switch_mty, goto_entry));
      Append_Token_Special(tokens, ':');
      Increment_Indentation();
      Append_Indented_Newline(tokens, 1);
      (void)WN2C_translate(tokens, goto_stmt, context);
      Append_Token_Special(tokens, ';');
      Decrement_Indentation();
      Append_Indented_Newline(tokens, 1);
      goto_stmt = WN_next(goto_stmt);
   }
   if (WN_compgoto_has_default_case(wn))
   {
      goto_stmt = WN_kid(wn,2);
      Is_True(WN_operator(goto_stmt) == OPR_GOTO,
	      ("Expected COMPGOTO default case to be an OPR_GOTO"));

      Append_Token_String(tokens, "default");
      Append_Token_Special(tokens, ':');
      Increment_Indentation();
      Append_Indented_Newline(tokens, 1);
      (void)WN2C_translate(tokens, goto_stmt, context);
      Append_Token_Special(tokens, ';');
      Decrement_Indentation();
      Append_Indented_Newline(tokens, 1);
   }
      
   /* Emit the end of the switch body */
   Append_Token_Special(tokens, '}');

   STATUS_set_block(status);
   return status;
} /* WN2C_compgoto */


static STATUS
WN2C_switch(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* The WN_switch_num_cases field gives the number of entries in 
    * the case table.  Kid0 is the switch value, which is an
    * integral case-value for the switch cases in kid1.  Kid2 is 
    * a lone OPR_CASEGOTO node, which gives the default jump target
    * when the value lies outside the range of the case table.  If
    * the kid_count is 2, then it is assumed that the jump target
    * lies within the range of the jump-table, and we should not 
    * access kid2.
    *
    * We generate a switch statement controlled by kid0, with a case
    * for each entry in the switch table represented by kid1.  The
    * switch statement will have a default case corresponding to
    * kid2, unless the kid_count is 2.
    *
    * This is a little closer to a regular C switch-statement than
    * the compgoto statement, but only slightly more so.  We abandon
    * the idea of moving the code for each case in under "case" 
    * statements, since this seems a prohibitively difficult task in
    * the presence of nested switch-statements and possible code-
    * motion,inlining, etc. from optimizer phases.
    */
   const WN *goto_stmt;
   MTYPE     switch_mty;
   STATUS    status = EMPTY_STATUS;
   OPCODE    myopcode;
   const  WN *compgotoid;

   Is_True(WN_operator(wn) == OPR_SWITCH, 
	   ("Invalid operator for WN2C_switch()"));

   compgotoid = WN_compgoto_idx(wn);
   myopcode   = WN_opcode(compgotoid);

   /* Emit the switch control */
   /* FMZ-fix bug for switch based on a field of structure-type variable*/ 
//   switch_mty = TY_mtype(WN_Tree_Type(WN_compgoto_idx(wn)));

   if (OPCODE_has_field_id(myopcode) && WN_field_id(compgotoid))
         switch_mty = WN_rtype(compgotoid);
   else
         switch_mty = TY_mtype(WN_Tree_Type(compgotoid));

   Append_Token_String(tokens, "switch");
   Append_Token_Special(tokens, '(');
   (void)WN2C_translate(tokens, WN_switch_test(wn), context);
   Append_Token_Special(tokens, ')');
   
   /* Emit the beginning of the switch body */
   Append_Indented_Newline(tokens, 1);
   Append_Token_Special(tokens, '{');
   Append_Indented_Newline(tokens, 1);

   /* Emit each of the cases, possibly followed by a default case */
   for (goto_stmt = WN_first(WN_switch_table(wn));
	goto_stmt != NULL;
	goto_stmt = WN_next(goto_stmt))
   {
      Is_True(WN_operator(goto_stmt) == OPR_CASEGOTO,
	      ("Expected each SWITCH case to be an OPR_CASEGOTO"));
      Append_Token_String(tokens, "case");

      TCON2C_translate(tokens, 
		       Host_To_Targ(switch_mty, WN_const_val(goto_stmt)));

      Append_Token_Special(tokens, ':');
      Increment_Indentation();
      Append_Indented_Newline(tokens, 1);
      (void)WN2C_translate(tokens, goto_stmt, context);
      Append_Token_Special(tokens, ';');
      Decrement_Indentation();
      Append_Indented_Newline(tokens, 1);
   }
   if (WN_switch_has_default_case(wn))
   {
      goto_stmt = WN_switch_default(wn);
      Is_True(WN_operator(goto_stmt) == OPR_GOTO,
	      ("Expected SWITCH default case to be an OPR_GOTO"));

      Append_Token_String(tokens, "default");
      Append_Token_Special(tokens, ':');
      Increment_Indentation();
      Append_Indented_Newline(tokens, 1);
      (void)WN2C_translate(tokens, goto_stmt, context);
      Append_Token_Special(tokens, ';');
      Decrement_Indentation();
      Append_Indented_Newline(tokens, 1);
   }
      
   /* Emit the end of the switch body */
   Append_Token_Special(tokens, '}');
   STATUS_set_block(status);
   return status;
} /* WN2C_switch */


static STATUS
WN2C_do_loop(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* Strictly speaking, the rhs of the termination test and the rhs of
    * the induction increment expression should be evaluated only once.
    * However, we assume any side-effect expressions have been removed
    * and assigned to temporaries, so we ignore this requirement for now.
    */
   STATUS    status;
   const WN *loop_info;
   
   Is_True(WN_operator(wn) == OPR_DO_LOOP,
	   ("Invalid operator for WN2C_do_loop()"));

   if (W2C_Prompf_Emission)
   {
      WN2C_Start_Prompf_Transformed_Loop(tokens, wn, context);
   }
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
   
   loop_info = WN_do_loop_info(wn);
   if (W2C_Emit_Cgtag && loop_info != NULL)
   {
      Append_Token_String(tokens, "/* LOOPINFO #");
      Append_Token_String(tokens, Number_as_String((UINT64)loop_info, "%ull"));
      Append_Token_String(tokens, "*/");
      Append_Indented_Newline(tokens, 1);
   }

   /* Emit the loop header as a for-loop */
   Append_Token_String(tokens, "for");
   Append_Token_Special(tokens, '(');
   (void)WN2C_translate(tokens, WN_start(wn), context);
   Append_Token_Special(tokens, ';');
   (void)WN2C_translate(tokens, WN_end(wn), context);
   Append_Token_Special(tokens, ';');
   (void)WN2C_translate(tokens, WN_step(wn), context);
   Append_Token_Special(tokens, ')');

   /* Emit the loop body on the next line */
   WN2C_incr_indentation_for_stmt_body(WN_do_body(wn));
   CONTEXT_set_srcpos(context, WN_Get_Linenum(WN_do_body(wn)));
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
   status = WN2C_translate(tokens, WN_do_body(wn), context);
   WN2C_decr_indentation_for_stmt_body(WN_do_body(wn));

   if (W2C_Prompf_Emission)
      WN2C_End_Prompf_Transformed_Loop(tokens, wn, context);

   return status;
} /* WN2C_do_loop */


static STATUS
WN2C_do_while(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* A simple do-loop as in C */
   
   Is_True(WN_operator(wn) == OPR_DO_WHILE,
	   ("Invalid operator for WN2C_do_while()"));
   
   if (W2C_Prompf_Emission)
      WN2C_Start_Prompf_Transformed_Loop(tokens, wn, context);

   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));

   /* Emit the header of the do-loop */
   Append_Token_String(tokens, "do");

   /* Emit the loop body on the next line */
   WN2C_incr_indentation_for_stmt_body(WN_while_body(wn));
   CONTEXT_set_srcpos(context, WN_Get_Linenum(WN_while_body(wn)));
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
   (void)WN2C_translate(tokens, WN_while_body(wn), context);
   WN2C_decr_indentation_for_stmt_body(WN_while_body(wn));

   /* Emit the tail of the do-loop on a new line */
   WN2C_Stmt_Newline(tokens, WN_Get_Linenum(wn));
   Append_Token_String(tokens, "while");
   Append_Token_Special(tokens, '(');
   (void)WN2C_translate(tokens, WN_while_test(wn), context);
   Append_Token_Special(tokens, ')');

   if (W2C_Prompf_Emission)
      WN2C_End_Prompf_Transformed_Loop(tokens, wn, context);

   return EMPTY_STATUS;
} /* WN2C_do_while */


static STATUS 
WN2C_while_do(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* A simple while-loop as in C */
   STATUS status;
   
   Is_True(WN_operator(wn) == OPR_WHILE_DO,
	   ("Invalid operator for WN2C_while_do()"));
   
   if (W2C_Prompf_Emission)
   {
      WN2C_Start_Prompf_Transformed_Loop(tokens, wn, context);
   }
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));

   /* Emit the loop header as a while-loop */
   Append_Token_String(tokens, "while");
   Append_Token_Special(tokens, '(');
   (void)WN2C_translate(tokens, WN_while_test(wn), context);
   Append_Token_Special(tokens, ')');

   /* Emit the loop body on the next line */
   WN2C_incr_indentation_for_stmt_body(WN_while_body(wn));
   CONTEXT_set_srcpos(context, WN_Get_Linenum(WN_while_body(wn)));
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
   status = WN2C_translate(tokens, WN_while_body(wn), context);
   WN2C_decr_indentation_for_stmt_body(WN_while_body(wn));

   if (W2C_Prompf_Emission)
      WN2C_End_Prompf_Transformed_Loop(tokens, wn, context);

   return status;
} /* WN2C_while_do */


static STATUS
WN2C_if(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   STATUS status;
   
   Is_True(WN_operator(wn) == OPR_IF, ("Invalid operator for WN2C_if()"));

   /* Ignore if-guards inserted by lno, since these are redundant
    * in High WHIRL.
    */
   if (WN_Is_If_Guard(wn))
   {
      /* Emit only the THEN body, provided it is non-empty */
      if (WN_operator(WN_then(wn)) != OPR_BLOCK)
      {
	 CONTEXT_set_srcpos(context, WN_Get_Linenum(WN_then(wn)));
	 status = WN2C_translate(tokens, WN_then(wn), context);
      }
      else
      {
	 WN2C_Translate_Stmt_Sequence(
	    tokens, WN_first(WN_then(wn)), FALSE/*on_newline*/, context);
      } /* if then-block */
   }
   else /* Not a redundant guard (from whirl2c perspective) */
   {
      /* Emit the "if" header */
      Append_Token_String(tokens, "if");
      Append_Token_Special(tokens, '(');
      (void)WN2C_translate(tokens, WN_if_test(wn), context);
      Append_Token_Special(tokens, ')');

      /* Emit the THEN body on a new line */
      WN2C_incr_indentation_for_stmt_body(WN_then(wn));
      CONTEXT_set_srcpos(context, WN_Get_Linenum(WN_then(wn)));
      WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
      status = WN2C_translate(tokens, WN_then(wn), context);
      WN2C_decr_indentation_for_stmt_body(WN_then(wn));

      /* See if there is anything but an empty else-part */
      if (!WN_else_is_empty(wn))
      {
	 /* Emit the "else" keyword and the else-body on a new line */
	 Append_Indented_Newline(tokens, 1);
	 Append_Token_String(tokens, "else");
	 WN2C_incr_indentation_for_stmt_body(WN_else(wn));
	 CONTEXT_set_srcpos(context, WN_Get_Linenum(WN_else(wn)));
	 WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
	 status = WN2C_translate(tokens, WN_else(wn), context);
	 WN2C_decr_indentation_for_stmt_body(WN_else(wn));
      }
   } /* if WN_Is_If_Guard */
   
   return status;
} /* WN2C_if */


static STATUS
WN2C_goto(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   Is_True(WN_operator(wn) == OPR_GOTO ||
	   WN_operator(wn) == OPR_CASEGOTO ||
	   WN_operator(wn) == OPR_REGION_EXIT,
	   ("Invalid operator for WN2C_goto()"));
   Append_Token_String(tokens, "goto");
   WN2C_append_label_name(tokens, wn);

   return EMPTY_STATUS;
} /* WN2C_goto */


static STATUS
WN2C_altentry(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* TODO: Handle OPR_ALTENTRY in C?? */
   Is_True(WN_operator(wn) == OPR_ALTENTRY,
	   ("Invalid operator for WN2C_altentry()"));
   Append_Token_String(tokens, "__OPR_ALTENTRY");
   Append_Token_Special(tokens, '(');
   Append_Token_String(tokens, ST_name(WN_st(wn)));
   Append_Token_Special(tokens, ')');

   return EMPTY_STATUS;
} /* WN2C_altentry */


static STATUS
WN2C_condbr(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   Is_True(WN_operator(wn) == OPR_TRUEBR || WN_operator(wn) == OPR_FALSEBR,
	   ("Invalid operator for WN2C_condbr()"));

   /* Check the condition, which must be true to do the branch */
   Append_Token_String(tokens, "if");
   Append_Token_Special(tokens, '(');
   if (WN_operator(wn) == OPR_FALSEBR)
   {
      Append_Token_Special(tokens, '!');
      Append_Token_Special(tokens, '(');
      (void)WN2C_translate(tokens, WN_condbr_cond(wn), context);
      Append_Token_Special(tokens, ')');
   }
   else /* WN_operator(wn) == OPR_TRUEBR */
   {
      (void)WN2C_translate(tokens, WN_condbr_cond(wn), context);
   }
   Append_Token_Special(tokens, ')');
   
   /* Emit the branch part as a goto statement on a new line */
   Increment_Indentation();
   Append_Indented_Newline(tokens, 1);
   Append_Token_String(tokens, "goto");
   WN2C_append_label_name(tokens, wn);
   Decrement_Indentation();

   return EMPTY_STATUS;
} /* WN2C_condbr */


static STATUS
WN2C_return(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* Note that we either return through the first parameter, or
    * a preg.  This is given by the PUinfo.
    */
   Is_True(WN_operator(wn) == OPR_RETURN,
	   ("Invalid operator for WN2C_return()"));
   Is_True(RETURNSITE_return(WN2C_Next_ReturnSite) == wn,
	   ("RETURNSITE out of sequence in WN2C_return()"));

   if (PUINFO_RETURN_TY != (TY_IDX) 0 && 
       TY_kind(PUINFO_RETURN_TY) != KIND_VOID &&
       RETURN_PREG_mtype(PUinfo_return_preg, 0) != MTYPE_V)
   {
      WN2C_Function_Return_Value(tokens, context);
   }
   else /* nothing to return */
   {
      Append_Token_String(tokens, "return");
   }

   WN2C_Next_ReturnSite = RETURNSITE_next(WN2C_Next_ReturnSite);
   
   return EMPTY_STATUS;
} /* WN2C_return */

static STATUS
WN2C_return_val(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   char buf[64];
   Is_True(WN_operator(wn) == OPR_RETURN_VAL,
	   ("Invalid operator for WN2C_return_val()"));
   Append_Token_String(tokens, "return ");
   Append_Token_Special(tokens, '(');
   (void) WN2C_translate(tokens, WN_kid0(wn), context);
   Append_Token_Special(tokens, ')');
   return EMPTY_STATUS;
} /* WN2C_return_val */

static STATUS
WN2C_label(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   Is_True(WN_operator(wn) == OPR_LABEL,
	   ("Invalid operator for WN2C_label()"));

   WN2C_append_label_name(tokens, wn);
   Append_Token_Special(tokens, ':');

   return EMPTY_STATUS;
} /* WN2C_label */


static STATUS 
WN2C_exc_scope_end(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   UINT current_indent = Current_Indentation();

   Set_Current_Indentation(0);
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
   Append_Token_String(tokens, "#pragma");
   Set_Current_Indentation(current_indent);
   Append_Token_String(tokens, "EXCEPTION_SCOPE_END");
   return EMPTY_STATUS;
} /* WN2C_exc_scope_end */


static STATUS 
WN2C_exc_scope_begin(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   UINT current_indent = Current_Indentation();

   Set_Current_Indentation(0);
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
   Append_Token_String(tokens, "#pragma");
   Set_Current_Indentation(current_indent);
   Append_Token_String(tokens, "EXCEPTION_SCOPE_BEGIN");
   return EMPTY_STATUS;
} /* WN2C_exc_scope_begin */


static STATUS
WN2C_istore(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* Get the lvalue for the lhs (kid1), and the value of the rhs
    * (kid0), and assign the rhs to the lhs.
    */
   TY_IDX       stored_ty;
   TOKEN_BUFFER lhs_tokens;
   
   Is_True(WN_operator(wn) == OPR_ISTORE || 
	   (WN_operator(wn) == OPR_STID && 
	    ST_sclass(WN_st(wn)) == SCLASS_FORMAL_REF),
	   ("Invalid operator for WN2C_istore()"));
   Is_True(WN_operator(wn) != OPR_ISTORE || TY_Is_Pointer(WN_ty(wn)),
	   ("Expected WN_ty to be a pointer for WN2C_istore()"));


   /* See if there is any prefetch information with this store */
   /* Insert prefetch for stores BEFORE the store */
   if (W2C_Emit_Prefetch && WN_MAP_Get(WN_MAP_PREFETCH, wn))
     WN2C_Prefetch_Map(tokens, wn, context); /* between newlines */

   /* Get lhs of the indirect assignment
   */
   lhs_tokens = New_Token_Buffer();
   if (WN_operator(wn) == OPR_STID)
   {
      char lhs_address_area [sizeof (WN)];
      WN* lhs_address = (WN*) &lhs_address_area;;

      WN2C_create_ref_param_lda(lhs_address, wn);
      WN2C_memref_lhs(lhs_tokens, 
		      &stored_ty,
		      lhs_address,         /* lhs address */
		      WN_store_offset(wn), /* offset from this address */
		      WN_ty(lhs_address),  /* ref type for stored object */
		      WN_ty(wn),           /* type for stored object */
		      WN_opc_dtype(wn),    /* base-type for stored object */
		      context);
   }
   else
   {
     //WEI:  if lhs is a field access (e.g. f1->x), we want the type of the field 
     //instead of the struct
     TY_IDX base_ty = TY_pointed(WN_ty(wn));
     TY_IDX actual_ty = (WN_field_id(wn) > 0) ? Make_Pointer_Type(Get_Field_Type(base_ty, WN_field_id(wn))) : WN_ty(wn);

      WN2C_memref_lhs(lhs_tokens, 
		      &stored_ty,
		      WN_kid1(wn),         /* lhs address */
		      WN_store_offset(wn), /* offset from this address */
		      actual_ty,
		      //WN_ty(wn),           /* ref type for stored object */
		      TY_pointed(actual_ty),
		      //TY_pointed(WN_ty(wn)), /* type for stored object */
		      WN_opc_dtype(wn),    /* base-type for stored object */
		      context);
   }

   //
   
   /* Do the assignment */
   WN2C_Append_Assignment(tokens, 
			  &lhs_tokens, /* lhs */
			  WN_kid0(wn), /* rhs */
			  stored_ty,   /* expected type of rhs */
			  context);

   return EMPTY_STATUS;
} /* WN2C_istore */


static STATUS
WN2C_istorex(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* Both kid1 and kid2 must be LDID nodes representing pregs.
    * Load the contents of the pregs, add them together, and
    * store kid0 into the resultant address.  The assignment is
    * typed according to the WN_ty.
    */
   TY_IDX       object_ty;
   TOKEN_BUFFER lhs_tokens;

   Is_True((WN_operator(wn) == OPR_ISTOREX             &&
	    WN_operator(WN_kid1(wn)) == OPR_LDID       &&
	    WN_operator(WN_kid(wn,2)) == OPR_LDID      &&
	    ST_sym_class(WN_st(WN_kid1(wn))) == CLASS_PREG &&
	    ST_sym_class(WN_st(WN_kid(wn,2))) == CLASS_PREG),
	   ("Invalid WN tree for WN2C_istorex()"));
   Is_True(TY_Is_Pointer(WN_ty(wn)),
	   ("Expected WN_ty to be a pointer for WN2C_istorex()"));

   /* Get the type of object being stored */
   object_ty =
      WN2C_MemAccess_Type(TY_pointed(WN_ty(wn)),     /* base_type */
			  WN_Tree_Type(WN_kid0(wn)), /* preferred type */
			  WN_opc_dtype(wn),          /* required mtype */
			  WN_store_offset(wn));      /* offset from base */

   lhs_tokens = New_Token_Buffer();
   WN2C_Load_From_PregIdx(lhs_tokens, 
			  WN_st(WN_kid1(wn)),           /* preg1 */
			  WN_load_offset(WN_kid1(wn)),  /* preg_idx1 */
			  WN_st(WN_kid(wn,2)),          /* preg2 */
			  WN_load_offset(WN_kid(wn,2)), /* preg_idx2 */
			  object_ty,                    /* result type */
			  context);
   
   /* Do the assignment */
   WN2C_Append_Assignment(tokens, 
			  &lhs_tokens, /* lhs */
			  WN_kid0(wn), /* rhs */
			  object_ty,   /* expected type of rhs */
			  context);

   return EMPTY_STATUS;
} /* WN2C_istorex */


static STATUS
WN2C_mstore(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* We have (MSTORE (MLOAD ...) (...)), or, alternatively an MSTORE of
    * a scalar into successive locations.  For a store of a MLOAD,
    * translate into a call to the "__MSTORE" builtin function, unless 
    * we have a struct copy ... in which case we try to be more clever 
    * and just do a regular assignment.
    */
   TY_IDX       base_ty;
   TY_IDX       stored_ty;
   TOKEN_BUFFER lhs_tokens;
   STATUS       lhs_status;
   
   Is_True(WN_operator(wn) == OPR_MSTORE,
	   ("Invalid operator for WN2C_mstore()"));
   Is_True(TY_Is_Pointer(WN_ty(wn)),
	   ("Expected WN_ty to be a pointer for WN2C_mstore()"));

   /* Get the type of object being stored */
   base_ty = WN_Tree_Type(WN_kid1(wn));
   if (!TY_Is_Pointer(base_ty))
      base_ty = WN_ty(wn);
   stored_ty = TY_pointed(WN_ty(wn));

   if (WN_field_id(wn) != 0) {
     //use the field's type instead
     stored_ty = Get_Field_Type(stored_ty, WN_field_id(wn));
   } else {
     stored_ty = 
       WN2C_MemAccess_Type(TY_pointed(base_ty),  /* base_type */
			   stored_ty,            /* preferred type */
			   MTYPE_M,              /* required mtype */
			   WN_store_offset(wn)); /* offset from base */
   }

   //WEI: an ugly hack that should probably go in be, but I can't get it work there
   //without breaking other stuff(threadof, affinity, etc)...
   if (WN_operator(WN_kid0(wn)) == OPR_TAS) {
     WN_kid0((WN*) wn) = WN_kid0(WN_kid0(wn));
   }

   if (WN_operator(WN_kid0(wn)) == OPR_MLOAD)
   {
      /* Use a regular struct assignment, provided the object stored has
       * been found to be a struct and the size of the MLOAD is known.
       */
      if (TY_Is_Structured(stored_ty) && 
	  WN_operator(WN_kid1(WN_kid0(wn))) == OPR_INTCONST)
      {
	 /* Get the lhs, preferably as an lvalue, but possibly as an address.
	  */
	 lhs_tokens = New_Token_Buffer();
	 lhs_status = WN2C_lvalue_wn(lhs_tokens,
				     WN_kid1(wn), /* the base address */
				     base_ty,   /* the base addr_ty */
				     stored_ty, /* type of obj to be stored */
				     WN_store_offset(wn),
				     context);

	 /* Dereference the address into which we are storing, if necessary.
	  */
	 if (!STATUS_is_lvalue(lhs_status))
	    Prepend_Token_Special(lhs_tokens, '*');
   
	 /* Do the assignment */
	 WN2C_Append_Assignment(tokens, 
				&lhs_tokens, /* lhs */
				WN_kid0(wn), /* rhs */
				stored_ty,   /* expected type of rhs */
				context);
      }
      else
      {
	 /* Need to copy byte by byte, from kid0 to kid1 */
	 Append_Token_String(tokens, "__MSTORE");
	 Append_Token_Special(tokens, '(');
	 (void)WN2C_translate(tokens, WN_kid0(WN_kid0(wn)), context);
	 Append_Token_Special(tokens, ',');
	 TCON2C_translate(tokens, 
			  Host_To_Targ(MTYPE_I8, WN_load_offset(WN_kid0(wn))));
	 Append_Token_Special(tokens, ',');
	 (void)WN2C_translate(tokens, WN_kid1(wn), context);
	 Append_Token_Special(tokens, ',');
	 TCON2C_translate(tokens, Host_To_Targ(MTYPE_I8, WN_store_offset(wn)));

	 Append_Token_Special(tokens, ',');
	 (void)WN2C_translate(tokens, WN_kid(wn,2), context); /* bytes */
	 Append_Token_Special(tokens, ')');
      } /*if*/
   }
   else
   {
      TY_IDX      rhs_ty = WN_Tree_Type(WN_kid0(wn));
      TY_IDX      rhs_ptr = Stab_Pointer_To(rhs_ty);
      const INT32 rhs_size = TY_size(rhs_ty);
      const UINT  tmp_idx1 = Stab_Lock_Tmpvar(rhs_ty, ST2C_Declare_Tempvar);
      const UINT  tmp_idx2 = Stab_Lock_Tmpvar(rhs_ptr, ST2C_Declare_Tempvar);
      const char *induction_var_name = W2CF_Symtab_Nameof_Tempvar(tmp_idx1);
      const char *ptr_var_name = W2CF_Symtab_Nameof_Tempvar(tmp_idx2);

      /* Assign the address we are to store into into the "ptr_var_name",
       * taking into account the store offset.
       */
      Append_Token_String(tokens, ptr_var_name);
      Append_Token_Special(tokens, '=');
      WN2C_append_cast(tokens, rhs_ty, TRUE/*ptr_to_type*/);
      (void)WN2C_translate(tokens, WN_kid1(wn), context);
      if (WN_store_offset(wn) > 0)
      {
	 Append_Token_Special(tokens, '+');
	 TCON2C_translate(tokens, 
			  Host_To_Targ(MTYPE_I8, 
				       WN_store_offset(wn)/rhs_size));
      }
      Append_Token_Special(tokens, ';');
      Append_Indented_Newline(tokens, 1);

      /* Set up a loop header to initiate numbytes/elementsize slots.
       */
      Append_Token_String(tokens, "for");
      Append_Token_Special(tokens, '(');
      Append_Token_String(tokens, induction_var_name);
      Append_Token_Special(tokens, '=');
      TCON2C_translate(tokens, Host_To_Targ(MTYPE_I8, 0));
      Append_Token_Special(tokens, ';');
      Append_Token_String(tokens, induction_var_name);
      Append_Token_Special(tokens, '<');
      Append_Token_Special(tokens, '(');
      (void)WN2C_translate(tokens, WN_kid(wn,2), context); /* bytes */
      Append_Token_Special(tokens, '/');
      TCON2C_translate(tokens, Host_To_Targ(MTYPE_I8, rhs_size));
      Append_Token_Special(tokens, ')');
      Append_Token_Special(tokens, ';');
      Append_Token_String(tokens, Concat2_Strings(induction_var_name, "++"));
      Append_Token_Special(tokens, ')');

      /* Copy one value at a time */
      Increment_Indentation();
      Append_Indented_Newline(tokens, 1);
      Append_Token_String(tokens, ptr_var_name);
      Append_Token_Special(tokens, '[');
      Append_Token_String(tokens, induction_var_name);
      Append_Token_Special(tokens, ']');
      Append_Token_Special(tokens, '=');
      (void)WN2C_translate(tokens, WN_kid0(wn), context);
      Decrement_Indentation();

      Stab_Unlock_Tmpvar(tmp_idx1);
      Stab_Unlock_Tmpvar(tmp_idx2);
   }

   return EMPTY_STATUS;
} /* WN2C_mstore */


static STATUS
WN2C_stid(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* Get the lvalue for the lhs (kid1), and the value of the rhs
    * (kid0), and assign the rhs to the lhs.
    */
   TOKEN_BUFFER lhs_tokens;
   TY_IDX       stored_ty;    /* Type of assignment */

   Is_True(WN_operator(wn) == OPR_STID,
	   ("Invalid operator for WN2C_stid()"));

   if (ST_sym_class(WN_st(wn))==CLASS_VAR && ST_is_not_used(WN_st(wn)))
   {
      /* This is a redundant assignment statement, so determined
       * by IPA, so only generate the rhs (in case of volatiles?).
       */
      (void)WN2C_translate(tokens, WN_kid0(wn), context); /* bytes */
   }
   else if (ST_sclass(WN_st(wn)) == SCLASS_FORMAL_REF)
   {
      (void)WN2C_istore(tokens, wn, context);
   }
   else
   {
     //FIX: for field accesses, change stored type to that of the field
     TY_IDX stored_ty = WN_ty(wn);
     if (WN_field_id(wn) != 0) {
       stored_ty = Get_Field_Type(stored_ty, WN_field_id(wn));
     }
     /* Get the lhs expression */
     lhs_tokens = New_Token_Buffer();
     WN2C_stid_lhs(lhs_tokens,
		   &stored_ty,          /* Corrected stored type */
		   WN_st(wn),           /* base symbol */
		   WN_store_offset(wn), /* offset from base */
		   stored_ty,           /* stored type */
		   WN_opc_dtype(wn),    /* stored mtype */
		   context);

      /* Do the assignment */
      WN2C_Append_Assignment(tokens, 
			     &lhs_tokens, /* lhs */
			     WN_kid0(wn), /* rhs */
			     stored_ty,   /* expected type of rhs */
			     context);    /* Get the rhs expression */
   }
   return EMPTY_STATUS;
} /* WN2C_stid */


static STATUS 
WN2C_call(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* A call statement.  Generate the call and assign the return
    * value to the return registers, a return variable, or a temporary
    * return variable ... dependent on the type of value returned
    * and whether or not a return-variable was found during the PUinfo
    * analysis.
    */
   INT          arg_idx, first_arg_idx, last_arg_idx;
   TY_IDX       func_ty, return_ty;
   TYLIST_IDX   param_tylist;
   TOKEN_BUFFER call_tokens, arg_tokens;
   BOOL         return_to_param;

   /* Emit any relevant call-site directives
    */
   if (WN_operator(wn) == OPR_CALL || WN_operator(wn) == OPR_PICCALL)
   {
      WN2C_Callsite_Directives(tokens, wn, &St_Table[WN_entry_name(wn)],
                               context);
   }

   /* A buffer to hold the tokens representing the function call */
   call_tokens = New_Token_Buffer();
   
   /* Tokenize the function reference, and get the function type and
    * return type, whether or not the return is through the first 
    * parameter.  Also, get the range of kids representing the
    * actual arguments.  A NULL func_ty indicates an unknown type.
    */
   CONTEXT_reset_top_level_expr(context); /* Parenthesize func addr expr */
   if (WN_operator(wn) == OPR_CALL)
   {
      ST2C_use_translate(call_tokens, &St_Table[WN_entry_name(wn)], context);
      func_ty = ST_pu_type(&St_Table[WN_entry_name(wn)]);
      return_to_param = W2X_Unparse_Target->Func_Return_To_Param(func_ty);
      return_ty = W2X_Unparse_Target->Func_Return_Type(func_ty);
      first_arg_idx = (return_to_param? 1 : 0);
      last_arg_idx = WN_kid_count(wn) - 1;
   }
   else if (WN_operator(wn) == OPR_ICALL)
   {
      Is_True(WN_ty(wn) != (TY_IDX) 0, 
	      ("Expected non-null WN_ty for ICALL in WN_call()"));
     
      (void)WN2C_translate(call_tokens, 
			   WN_kid(wn, WN_kid_count(wn) - 1), 
			   context);

      //WEI: Need to paranthesize the address of the function
      WHIRL2C_parenthesize(call_tokens);

      /* The function type used be:
       *
       *    TY_pointed(WN_Tree_Type(WN_kid(wn,WN_kid_count(wn) - 1)));
       *
       * but is now directly available as the WN_ty attribute.
       */
      func_ty = WN_ty(wn);
      return_to_param = W2X_Unparse_Target->Func_Return_To_Param(func_ty);
      return_ty = W2X_Unparse_Target->Func_Return_Type(func_ty);
      first_arg_idx = (return_to_param? 1 : 0);
      last_arg_idx = WN_kid_count(wn) - 2;
   }
   else if (WN_operator(wn) == OPR_PICCALL)
   {
      Is_True(WN_entry_name(wn) != 0, 
	      ("Expected non-null WN_entry_name for PICCALL in WN_call()"));
      ST2C_use_translate(call_tokens, &St_Table[WN_entry_name(wn)], context);
      func_ty = ST_pu_type(&St_Table[WN_entry_name(wn)]);
      return_to_param = W2X_Unparse_Target->Func_Return_To_Param(func_ty);
      return_ty = W2X_Unparse_Target->Func_Return_Type(func_ty);
      first_arg_idx = (return_to_param? 1 : 0);
      last_arg_idx = WN_kid_count(wn) - 2;
   }
   else /* OPR_INTRINSIC_CALL */
   {
      Is_True(WN_operator(wn) == OPR_INTRINSIC_CALL,
	      ("Invalid operator for WN2C_call()"));

      Append_Token_String(call_tokens,
                          WN_intrinsic_name((INTRINSIC)WN_intrinsic(wn)));

      /* Note, we may have a void return type for implicit return through
       * argument 0.  Should be represented by a mismatch between the
       * rtype (void) and the return type in the wtable!
       */
      func_ty = (TY_IDX) 0;
      return_ty = WN_intrinsic_return_ty(WN_opcode(wn),
                                         (INTRINSIC)WN_intrinsic(wn), wn);
      return_to_param = WN_intrinsic_return_to_param(return_ty);
      first_arg_idx =  (return_to_param? 1 : 0);
      last_arg_idx = WN_kid_count(wn) - 1;
   }
   
   /* Append the argument list to the function reference.  No need to
    * reset CONTEXT_needs_lvalue(context), since this is statement and
    * CONTEXT_needs_lvalue(context) must be FALSE.
    */
   Append_Token_Special(call_tokens, '(');
   CONTEXT_set_top_level_expr(context); /* To avoid top-level parenthesis */
   if (func_ty != (TY_IDX) 0 && TY_has_prototype(func_ty))
      param_tylist = TY_parms(func_ty);
   else
      param_tylist = (TYLIST_IDX) 0;
   for (arg_idx = first_arg_idx; arg_idx <= last_arg_idx; arg_idx++)
   {
      arg_tokens = New_Token_Buffer();

      // In translating the PARM nodes, use the actual argument type rather
      // than the PARM type to do casting of argument expression types.
      //
      if (Tylist_Table[param_tylist] != TY_IDX_ZERO)
	 CONTEXT_set_given_lvalue_ty(context,
				     TYLIST_item(Tylist_Table[param_tylist]));
      else
	 CONTEXT_set_given_lvalue_ty(context,
				     WN_Tree_Type(WN_kid(wn, arg_idx)));
      
      Is_True(WN_operator(WN_kid(wn, arg_idx)) == OPR_PARM,
	      ("Expected OPR_PARM as CALL argument"));
      
      (void)WN2C_translate(arg_tokens, WN_kid(wn, arg_idx), context);

      Append_And_Reclaim_Token_List(call_tokens, &arg_tokens);
      if (Tylist_Table[param_tylist] != TY_IDX_ZERO)
	 param_tylist = TYLIST_next(param_tylist);
      if (arg_idx < last_arg_idx)
	 Append_Token_Special(call_tokens, ',');
   }
   Append_Token_Special(call_tokens, ')');

   /* Update the call site information to denote this one */
   if (WN2C_Prev_CallSite == NULL)
      WN2C_Prev_CallSite = PUinfo_Get_CallSites();
   else
      WN2C_Prev_CallSite = CALLSITE_next(WN2C_Prev_CallSite);

   Is_True(CALLSITE_call(WN2C_Prev_CallSite) == wn,
	   ("CALLSITE out of sequence in WN2C_call()"));

   /* Next, save off the function return value to a (temporary)
    * variable or a return-register, as is appropriate.
    */
   if (return_ty != (TY_IDX) 0              && 
       TY_kind(return_ty) != KIND_VOID      &&
       WN_opcode(wn) != OPC_VCALL           &&
       WN_opcode(wn) != OPC_VICALL          &&
       WN_opcode(wn) != OPC_VPICCALL        &&
       WN_opcode(wn) != OPC_VINTRINSIC_CALL)
   {
      WN2C_Function_Call_Lhs(call_tokens,
			     return_to_param,
			     return_ty,
			     WN_kid0(wn), /* First Argument */
			     context);
   }
   Append_And_Reclaim_Token_List(tokens, &call_tokens);

   return EMPTY_STATUS;
} /* WN2C_call */


static STATUS 
WN2C_eval(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* As far as C is concerned, treat this as a transparent node! */
   Is_True(WN_operator(wn) == OPR_EVAL,
	   ("Invalid operator for WN2C_eval()"));

   return WN2C_translate(tokens, WN_kid0(wn), context);
} /* WN2C_eval */


static STATUS 
WN2C_prefetch(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* Prefetch information is currently added in a comment */
   INT pflag;
   
   Is_True(WN_operator(wn) == OPR_PREFETCH || WN_operator(wn) == OPR_PREFETCHX,
	   ("Invalid operator for WN2C_prefetch()"));

   /* Get the prefetch identifier and address expression */
   if (WN_operator(wn) == OPR_PREFETCH)
   {
      Append_Token_String(tokens, 
	 Concat3_Strings("/* PREFETCH(", Ptr_as_String(wn), ")"));
      (void)WN2C_translate(tokens, WN_kid0(wn), context);
      Append_Token_String(tokens, 
	 Concat2_Strings("OFFS=", Number_as_String(WN_offset(wn), "%lld")));
   }
   else /* (WN_operator(wn) == OPR_PREFETCHX) */
   {
      Append_Token_String(tokens, 
	 Concat3_Strings("/* PREFETCHX(", Ptr_as_String(wn), ")"));
      (void)WN2C_translate(tokens, WN_kid0(wn), context);
      Append_Token_Special(tokens, '+');
      (void)WN2C_translate(tokens, WN_kid1(wn), context);
   }
      
   /* Emit the prefetch flags information (pf_cg.h) on a separate line */
   pflag = WN_prefetch_flag(wn);
   Set_Current_Indentation(Current_Indentation()+3);
   Append_Indented_Newline(tokens, 1);
   Append_Token_String(tokens, PF_GET_READ(pflag)? "read" : "write");
   Append_Token_String(tokens, 
      Concat2_Strings("strid1=", 
		      Number_as_String(PF_GET_STRIDE_1L(pflag), "%lld")));
   Append_Token_String(tokens, 
      Concat2_Strings("strid2=", 
		      Number_as_String(PF_GET_STRIDE_2L(pflag), "%lld")));
   Append_Token_String(tokens, 
      Concat2_Strings("conf=", 
		      Number_as_String(PF_GET_CONFIDENCE(pflag), "%lld"))); 
   Set_Current_Indentation(Current_Indentation()-3); 

   /* Close the comment about this prefetch */
   Append_Token_String(tokens, "*/");

   return EMPTY_STATUS;
} /* WN2C_prefetch */


static STATUS 
WN2C_comment(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   Is_True(WN_operator(wn) == OPR_COMMENT, 
	   ("Invalid operator for WN2C_comment()"));

   Append_Token_String(tokens, 
		       Concat3_Strings("/* ", Index_To_Str(WN_GetComment(wn)), " */"));

   return EMPTY_STATUS;
} /* WN2C_comment */


static STATUS 
WN2C_iload(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* Access a data object from the location (WN_kid0 + WN_load_offset),
    * where the type of data loaded is of type TY_pointed(WN_load_addr_ty(wn))
    * or of type WN_ty(wn).  When the address type denotes a struct we
    * try to find an element in the struct of type WN_ty(wn) or 
    * WN_opc_dtype(wn), and if found we load that object, otherwise we
    * load an object of type TY_pointed(WN_load_addr_ty(wn)).
    */
   TY_IDX       loaded_ty;
   TOKEN_BUFFER expr_tokens;

   Is_True(WN_operator(wn) == OPR_ILOAD || 
	   (WN_operator(wn) == OPR_LDID && 
	    ST_sclass(WN_st(wn)) == SCLASS_FORMAL_REF),
	   ("Invalid operator for WN2C_iload()"));

   /* Special case for Purple */
   if (W2C_Only_Mark_Loads && !TY_Is_Pointer(WN_ty(wn)))
   {
      char buf[64];
      sprintf(buf, "#<%p>#", wn);
      Append_Token_String(tokens, buf);
      return EMPTY_STATUS;
   }

   /* Get the type of data to be loaded from memory */
   expr_tokens = New_Token_Buffer();
   if (WN_operator(wn) == OPR_LDID)
   {
      char load_address_area [sizeof (WN)];
      WN* load_address = (WN*) &load_address_area;

      WN2C_create_ref_param_lda(load_address, wn);
      WN2C_memref_lhs(expr_tokens, 
		      &loaded_ty,
		      load_address,        /* lhs address */
		      WN_store_offset(wn), /* offset from this address */
		      WN_ty(load_address), /* ref type for stored object */
		      WN_ty(wn),           /* type for stored object */
		      WN_opc_dtype(wn),    /* base-type for stored object */
		      context);
   }
   else
   {
      WN2C_memref_lhs(expr_tokens, 
		      &loaded_ty,
		      WN_kid0(wn),         /* address */
		      WN_load_offset(wn),  /* offset from this address */
		      WN_load_addr_ty(wn), /* ref type for loaded object */
		      WN_ty(wn),           /* type for loaded object */
		      WN_opc_dtype(wn),    /* base-type for stored object */
		      context);

   }

   TY_IDX type_loaded = WN_Tree_Type(wn);
   //WEI: if we're loading a field in a struct, the type we really want is
   //the type pointed by WN_load_addr_ty(wn), which is the actual type of the field
   /*
     if (WN_operator(wn) == OPR_ILOAD && 
     WN_field_id(wn) > 0 && TY_kind(type_loaded) == KIND_STRUCT) {
     type_loaded = TY_pointed(WN_load_addr_ty(wn));
     }
   */

   /* Cast the resultant value to the expected result type if 
    * different from the type of value loaded.
    */
   if (!WN2C_arithmetic_compatible_types(loaded_ty, type_loaded))
     WN2C_prepend_cast(expr_tokens, type_loaded, FALSE/*pointer_to_type*/);

   Append_And_Reclaim_Token_List(tokens, &expr_tokens);

   /* See if there is any prefetch information with this load */
   if (W2C_Emit_Prefetch && WN_MAP_Get(WN_MAP_PREFETCH, wn))
   {
     Set_Current_Indentation(Current_Indentation()+3);
     WN2C_Prefetch_Map(tokens, wn, context); /* between newlines */
     Set_Current_Indentation(Current_Indentation()-3);
   }
   return EMPTY_STATUS;
} /* WN2C_iload */


static STATUS 
WN2C_iloadx(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   TY_IDX object_ty;

   /* Both kid0 and kid1 must be LDID nodes representing pregs.
    * Load the contents of the pregs, add them together, and
    * load an object of the given type from the resultant address.
    */
   Is_True((WN_operator(wn) == OPR_ILOADX              &&
	    WN_operator(WN_kid0(wn)) == OPR_LDID       &&
	    WN_operator(WN_kid1(wn)) == OPR_LDID       &&
	    ST_sym_class(WN_st(WN_kid0(wn))) == CLASS_PREG &&
	    ST_sym_class(WN_st(WN_kid1(wn))) == CLASS_PREG),
	   ("Invalid WN tree for for WN2C_iloadx()"));

   /* Get the type of object being loaded */
   object_ty =
      WN2C_MemAccess_Type(TY_pointed(WN_load_addr_ty(wn)), /* base_type */
			  WN_load_addr_ty(wn),             /* preferred type */
			  WN_opc_dtype(wn),           /* required mtype */
			  WN_load_offset(wn));        /* offset from base */

   WN2C_Load_From_PregIdx(tokens, 
			  WN_st(WN_kid0(wn)),          /* preg1 */
			  WN_load_offset(WN_kid0(wn)), /* preg_idx1 */
			  WN_st(WN_kid1(wn)),          /* preg2 */
			  WN_load_offset(WN_kid1(wn)), /* preg_idx2 */
			  object_ty,                   /* type to load */
			  context);
   
   /* Cast the resultant value to the result type, if it is different 
    * from the loaded type.
    */
   if (!WN2C_arithmetic_compatible_types(object_ty, WN_ty(wn)))
      WN2C_prepend_cast(tokens, WN_ty(wn), FALSE/*pointer_to_type*/);

   return EMPTY_STATUS;
} /* WN2C_iloadx */


static STATUS 
WN2C_mload(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* Load an aggregate value, presumably as a function parameter,
    * under an OPR_EVAL node, or as the rhs of an assignment.
    */
   TY_IDX       base_ty;
   TY_IDX       loaded_ty;
   STATUS       load_status;
   TOKEN_BUFFER expr_tokens;
   
   Is_True(WN_operator(wn) == OPR_MLOAD,
	   ("Invalid operator for WN2C_mload()"));
   Is_True(TY_Is_Pointer(WN_ty(wn)),
	   ("Expected WN_ty to be a pointer in WN2C_mload()"));
   Is_True(WN_operator(WN_kid1(wn)) == OPR_INTCONST,
	   ("Expected statically known size for WN2C_mload()"));

   /* Special case for Purple (cannot be a ptr, so always do it) */
   if (W2C_Only_Mark_Loads)
   {
      char buf[64];
      sprintf(buf, "#<%p>#", wn);
      Append_Token_String(tokens, buf);
      return EMPTY_STATUS;
   }

   /* Get the type of object being stored */
   base_ty = WN_Tree_Type(WN_kid0(wn));
   if (!TY_Is_Pointer(base_ty))
      base_ty = WN_ty(wn);
   if (WN_field_id(wn) != 0) {
     loaded_ty = Get_Field_Type(TY_pointed(WN_ty(wn)), WN_field_id(wn));
   } else {
     loaded_ty = TY_pointed(WN_ty(wn));
     loaded_ty = 
       WN2C_MemAccess_Type(TY_pointed(base_ty), /* base_type */
			   loaded_ty,           /* preferred type */
			   MTYPE_M,             /* required mtype */
			   WN_load_offset(wn)); /* offset from base */
   }

   /* Get the lvalue or address of the data to be loaded */
   expr_tokens = New_Token_Buffer();
   load_status = WN2C_lvalue_wn(expr_tokens, 
				WN_kid0(wn),  /* the base address */
				base_ty,      /* the base addr_ty */
				loaded_ty,    /* type of object loaded */
				WN_load_offset(wn), 
				context);

   /* Dereference the address from which we are loading, if necessary */
   if (!STATUS_is_lvalue(load_status))
      Prepend_Token_Special(expr_tokens, '*');
   
   Append_And_Reclaim_Token_List(tokens, &expr_tokens);

   return EMPTY_STATUS;
} /* WN2C_mload */


static STATUS 
WN2C_array(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* Use array addressing rules to return an address, unless
    * the context needs an lvalue and we can produce one, in which
    * case we return an lvalue and set STATUS_is_lvalue to be TRUE.
    */
   STATUS       base_addr_status;
   STATUS       return_status = EMPTY_STATUS;
   BOOL         treat_ptr_as_array;
   BOOL         set_ptr_as_array = FALSE;
   const BOOL   context_provides_base_ty = CONTEXT_array_basetype(context);
   const BOOL   context_needs_lvalue = CONTEXT_needs_lvalue(context);
   TOKEN_BUFFER tmp_tokens;
   INT32        dim;
   TY_IDX       base_ty; /* base-address for ARRAY node */
   TY_IDX       ety;     /* Type of element indexed */

   Is_True(WN_operator(wn) == OPR_ARRAY,
	   ("Invalid operator for WN2C_array()"));

   /* Specially handle the case when we have an implicit ptr_as_array
    * case, identified when the pointed type appears to be the type
    * of element indexed rather than the array indexed into.
    */
   base_ty = WN_Tree_Type(WN_kid0(wn));
   if (TY_Is_Pointer(base_ty) &&
       (!TY_Is_Array(TY_pointed(base_ty)) ||
	(TY_size(TY_AR_etype(TY_pointed(base_ty))) < WN_element_size(wn) &&
	 TY_size(TY_pointed(base_ty)) == WN_element_size(wn))))
   {
      if (!TY_ptr_as_array(Ty_Table[base_ty]))
      {
	 Set_TY_ptr_as_array(Ty_Table[base_ty]); /* Temporary side-effect */
	 set_ptr_as_array = TRUE;
      }
      treat_ptr_as_array = TRUE;
   }
   else
      treat_ptr_as_array = FALSE;

   /* Get the base address from which we are loading. Note that we
    * do handle pointers as arrays when there is no ambiguity, but
    * we still need to specially mark the case when we are indexing
    * an array of arrays.  The following should access the fifth 
    * element, each element consisting of 17 ints:
    *
    *    int (*a)[17]; .... a[5] ....
    *
    * This is presumably represented as (OPR_ARRAY (OPR_LDID a) ...),
    * but since the type of kid0 is ptr-to-arrayOfInts we erroneously
    * assume the type of wn is ptr-to-ints and erroneously generate
    * "(*a)[5]". 
    * TODO: Specially mark such array of array accesses to avoid this
    * anomaly (see also WN_Tree_Type()).  The front-end currently
    * does not mark ptrs as "ptr_as_array".  Should it?  Note that LNO
    * may explicitly mark such arrays (hence the need for the 
    * set_ptr_as_array flag).
    */
   tmp_tokens = New_Token_Buffer();
   CONTEXT_set_needs_lvalue(context);
   CONTEXT_reset_array_basetype(context);
   base_addr_status = WN2C_translate(tmp_tokens, WN_kid0(wn), context);

   if (treat_ptr_as_array)
   {
      if (set_ptr_as_array)
	 Clear_TY_ptr_as_array(Ty_Table[base_ty]);  /* Reset w2c side-effect! */
      ety = TY_pointed(base_ty);
      WHIRL2C_parenthesize(tmp_tokens);
   }
   else if (!TY_Is_Pointer(base_ty))
   {
      /* Cast the base-expression to the desired resultant type
       * and treat this ptr as a ptr-as-array type.
       */
      treat_ptr_as_array = TRUE;
      if (context_provides_base_ty)
	 base_ty = CONTEXT_given_base_ty(context);
      else
	 base_ty = WN_Tree_Type(wn);
      ety = TY_pointed(base_ty);
      WN2C_prepend_cast(tmp_tokens, base_ty, FALSE/*pointer_to_type*/);
      WHIRL2C_parenthesize(tmp_tokens);
   }
   else
   {
      treat_ptr_as_array = FALSE;
      ety = TY_AR_etype(TY_pointed(base_ty));

      /* If the base is a real address (as opposed to an lvalue or an
       * array value which according to C semantics can viewed as an address)
       * then dereference the address to get to the actual array.
       */
      if (!STATUS_is_lvalue(base_addr_status) &&
	  !STATUS_is_array_as_address(base_addr_status))
      {
	 Prepend_Token_Special(tmp_tokens, '*');
	 WHIRL2C_parenthesize(tmp_tokens);
      }
   }

   /* Generate an address or an lvalue, as requested from the 
    * context of this ARRAY node.
    */
   if (TY_Is_Array(ety))
      STATUS_set_array_as_address(return_status); /* array as address value */
   else if (context_needs_lvalue)
      STATUS_set_lvalue(return_status);           /* indexed is lvalue */
   else
      Prepend_Token_Special(tmp_tokens, '&'); /* create address value */
   Append_And_Reclaim_Token_List(tokens, &tmp_tokens);
   CONTEXT_reset_needs_lvalue(context);

   /* Append the index expressions */
   if (treat_ptr_as_array || 
       Stab_Array_Has_Dynamic_Bounds(TY_pointed(base_ty)))
   {
      /* The array has been declared as a pointer to the element types,
       * so use calculated offsets based on the element-type.
       */
      Append_Token_Special(tokens, '[');  
      CONTEXT_reset_top_level_expr(context); /* parenthesize idx exprs */
      WN2C_Normalize_Idx_To_Onedim(tokens, wn, context);
      Append_Token_Special(tokens, ']');
   }
   else
   {
      CONTEXT_set_top_level_expr(context); /* To avoid top-level parenthesis */
      for (dim = 0; dim < WN_num_dim(wn); dim++)
      {
	 Append_Token_Special(tokens, '[');
	 (void)WN2C_translate(tokens, WN_array_index(wn, dim), context);
	 Append_Token_Special(tokens, ']');
      }
   }
   
   return return_status;
} /* WN2C_array */


static STATUS 
WN2C_intrinsic_op(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* An intrinsic operator expression.  Generate the call as is,
    * regardless how the return value is returned, since we know
    * the consumer of the value is the surrounding expression.  This
    * call is not related to the call-info generated by PUinfo.
    */
   INT          arg_idx;
   TY_IDX       return_ty, result_ty;
   TOKEN_BUFFER call_tokens;

   Is_True(WN_operator(wn) == OPR_INTRINSIC_OP,
	   ("Invalid operator for WN2C_intrinsic_op()"));

   /* A buffer to hold the tokens representing the function call */
   call_tokens = New_Token_Buffer();
   Append_Token_String(call_tokens,
                       WN_intrinsic_name((INTRINSIC)WN_intrinsic(wn)));

   /* Append the argument list to the function reference.
    */
   Append_Token_Special(call_tokens, '(');
   CONTEXT_reset_needs_lvalue(context);
   CONTEXT_set_top_level_expr(context); /* To avoid top-level parenthesis */
   for (arg_idx = 0; arg_idx <= WN_kid_count(wn) - 1; arg_idx++)
   {
      (void)WN2C_translate(call_tokens, WN_kid(wn, arg_idx), context);
      if (arg_idx < WN_kid_count(wn) - 1)
	 Append_Token_Special(call_tokens, ',');
   }
   Append_Token_Special(call_tokens, ')');

   if (WN_intrinsic(wn) == INTRN_TLD_ADDR) {
     result_ty = WN_Tree_Type(wn);
     //  WN2C_prepend_cast(call_tokens, result_ty, FALSE/*pointer_to_type*/);
   } else {
     /* See if we need to cast the resultant value */
     result_ty = Stab_Mtype_To_Ty(WN_opc_rtype(wn));
   }
   return_ty = Stab_Mtype_To_Ty(TY_mtype(WN_intrinsic_return_ty(
								WN_opcode(wn),
								(INTRINSIC)WN_intrinsic(wn),
								wn)));
   
   if (!WN2C_arithmetic_compatible_types(return_ty, result_ty))
     WN2C_prepend_cast(call_tokens, result_ty, FALSE/*pointer_to_type*/);

   Append_And_Reclaim_Token_List(tokens, &call_tokens);

   return EMPTY_STATUS;
} /* WN2C_intrinsic_op */


static STATUS 
WN2C_tas(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   STATUS status;
   TOKEN_BUFFER cast_tokens;
   
   Is_True(WN_operator(wn) == OPR_TAS,
	   ("Invalid operator for WN2C_tas()"));

   if (WN2C_arithmetic_compatible_types(WN_ty(wn), WN_Tree_Type(WN_kid0(wn))))
   {
      status = WN2C_translate(tokens, WN_kid0(wn), context);
   }
   else
   {
      cast_tokens = New_Token_Buffer();
      CONTEXT_reset_needs_lvalue(context); /* Cast value, not lvalue */
      status = WN2C_translate(cast_tokens, WN_kid0(wn), context);
      WN2C_prepend_cast(cast_tokens, WN_ty(wn), FALSE/*pointer_to_type*/);
      Append_And_Reclaim_Token_List(tokens, &cast_tokens);
   }
   
   return status;
} /* WN2C_tas */


static STATUS 
WN2C_select(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* To be absolutely correct, we should evaluate both kid1 and kid2,
    * but since this would require the use of temporaries and does not
    * appear to be necessary we simply translate it into a conditional
    * expression.
    */
   Is_True(WN_operator(wn) == OPR_SELECT || 
	   WN_operator(wn) == OPR_CSELECT,
	   ("Invalid operator for WN2C_select()"));

   Append_Token_Special(tokens, '(');
   (void)WN2C_translate(tokens, WN_kid0(wn), context);
   Append_Token_Special(tokens, '?');
   (void)WN2C_translate(tokens, WN_kid1(wn), context);
   Append_Token_Special(tokens, ':');
   (void)WN2C_translate(tokens, WN_kid((wn),2), context);
   Append_Token_Special(tokens, ')');

   return EMPTY_STATUS;
} /* WN2C_select */


static STATUS 
WN2C_cvt(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   TOKEN_BUFFER expr_tokens;

   Is_True(WN_operator(wn) == OPR_CVT,
	   ("Invalid operator for WN2C_cvt()"));

   /* Cast to the desired type, assuming a cvt never is redundant.
    */
   expr_tokens = New_Token_Buffer();
   CONTEXT_reset_needs_lvalue(context); /* no need for an lvalue */
   (void)WN2C_translate(expr_tokens, WN_kid0(wn), context);
   WHIRL2C_parenthesize(expr_tokens);
   WN2C_prepend_cast(expr_tokens, 
		     Stab_Mtype_To_Ty(WN_opc_rtype(wn)), 
		     FALSE/*pointer_to_type*/);
   Append_And_Reclaim_Token_List(tokens, &expr_tokens);
   
   return EMPTY_STATUS;
} /* WN2C_cvt */


static STATUS 
WN2C_cvtl(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* Treat this simply as a regular type conversion, where the
    * resultant type is given as the rtype scaled down to the
    * given bitlength.
    */
   TY_IDX      object_ty, result_ty;
   TOKEN_BUFFER expr_tokens;
   STATUS       status;
   
   Is_True(WN_operator(wn) == OPR_CVTL,
	   ("Invalid operator for WN2C_cvtl()"));
   
   /* Get the result type and the type of value to be converted */
   result_ty = WN_Tree_Type(wn);
   object_ty = WN_Tree_Type(WN_kid0(wn));
   
   /* Translate the expression and make certain we end up with a
    * value of the expected type.
    */
   expr_tokens = New_Token_Buffer();
   if (WN2C_arithmetic_compatible_types(result_ty, object_ty))
   {
      /* This is a noop in C, since no casting is necessary */
      status = WN2C_translate(expr_tokens, WN_kid0(wn), context);
   }
   else
   {
      /* Translate and cast the resultant value to the desired result type */
      CONTEXT_reset_needs_lvalue(context); /* no need for an lvalue */
      status = WN2C_translate(expr_tokens, WN_kid0(wn), context);
      WHIRL2C_parenthesize(expr_tokens);
      WN2C_prepend_cast(expr_tokens, result_ty, FALSE/*pointer_to_type*/);
   }
   Append_And_Reclaim_Token_List(tokens, &expr_tokens);

   return status;
} /* WN2C_cvtl */


static STATUS 
WN2C_realpart(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   Is_True(WN_operator(wn) == OPR_REALPART,
	   ("Invalid operator for WN2C_realpart"));

   CONTEXT_reset_needs_lvalue(context); /* Denotes a value, not an lvalue */
   WN2C_translate(tokens, WN_kid0(wn), context);
   Append_Token_Special(tokens, '.');
   Append_Token_String(tokens, TY2C_Complex_Realpart_Name);
   
   return EMPTY_STATUS;
} /* WN2C_realpart */


static STATUS 
WN2C_imagpart(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   Is_True(WN_operator(wn) == OPR_IMAGPART,
	   ("Invalid operator for WN2C_imagpart"));

   CONTEXT_reset_needs_lvalue(context); /* Denotes a value, not an lvalue */
   WN2C_translate(tokens, WN_kid0(wn), context);
   Append_Token_Special(tokens, '.');
   Append_Token_String(tokens, TY2C_Complex_Imagpart_Name);
   
   return EMPTY_STATUS;
} /* WN2C_imagpart */


static STATUS 
WN2C_paren(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   STATUS status;

   Is_True(WN_operator(wn) == OPR_PAREN,
	   ("Invalid operator for WN2C_paren()"));

   Append_Token_Special(tokens, '(');
   CONTEXT_set_top_level_expr(context);
   status = WN2C_translate(tokens, WN_kid0(wn), context);
   Append_Token_Special(tokens, ')');

   return status;
} /* WN2C_paren */


static STATUS 
WN2C_complex(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   UINT        tmp_idx;
   const char *tmpvar_name;
   
   Is_True(WN_operator(wn) == OPR_COMPLEX,
	   ("Invalid operator for WN2C_complex()"));

   tmp_idx = Stab_Lock_Tmpvar(Stab_Mtype_To_Ty(WN_opc_rtype(wn)),
			      ST2C_Declare_Tempvar);
   tmpvar_name = W2CF_Symtab_Nameof_Tempvar(tmp_idx);

   /* Evaluate children to values, not to lvalues */
   CONTEXT_reset_needs_lvalue(context);

   Append_Token_Special(tokens, '('); /* getting complex structure */

   /* Assign real_part */
   Append_Token_Special(tokens, '('); /* getting real part */
   Append_Token_String(tokens, tmpvar_name);
   Append_Token_Special(tokens, '.');
   Append_Token_String(tokens, TY2C_Complex_Realpart_Name);
   Append_Token_Special(tokens, '=');
   (void)WN2C_translate(tokens, WN_kid0(wn), context);
   Append_Token_Special(tokens, ')'); /* gotten real part */
   Append_Token_Special(tokens, ',');

   /* Assign imaginary_part */
   Append_Token_Special(tokens, '('); /* getting imaginary part */
   Append_Token_String(tokens, tmpvar_name);
   Append_Token_Special(tokens, '.');
   Append_Token_String(tokens, TY2C_Complex_Imagpart_Name);
   Append_Token_Special(tokens, '=');
   (void)WN2C_translate(tokens, WN_kid1(wn), context);
   Append_Token_Special(tokens, ')'); /* gotten real part */
   Append_Token_Special(tokens, ',');

   /* Evaluate to complex value */
   Append_Token_String(tokens, tmpvar_name);
   Append_Token_Special(tokens, ')'); /* gotten complex number */

   Stab_Unlock_Tmpvar(tmp_idx);
   
   return EMPTY_STATUS;
} /* WN2C_complex */


static STATUS 
WN2C_bnor(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   const TY_IDX result_ty = Stab_Mtype_To_Ty(WN_opc_rtype(wn));
   TOKEN_BUFFER opnd;
   
   Is_True(WN_operator(wn) == OPR_BNOR,
	   ("Invalid operator for WN2C_bnor()"));

   /* Evaluate children to values, not to lvalues */
   CONTEXT_reset_needs_lvalue(context);
   
   CONTEXT_reset_top_level_expr(context);
   Append_Token_Special(tokens, '~');
   Append_Token_Special(tokens, '(');
   opnd = WN2C_Translate_Arithmetic_Operand(WN_kid0(wn), result_ty, context);
   Append_And_Reclaim_Token_List(tokens, &opnd);
   Append_Token_Special(tokens, '|');
   opnd = WN2C_Translate_Arithmetic_Operand(WN_kid1(wn), result_ty, context);
   Append_And_Reclaim_Token_List(tokens, &opnd);
   Append_Token_Special(tokens, ')');

   return EMPTY_STATUS;
} /* WN2C_bnor */


static STATUS 
WN2C_madd(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   const TY_IDX result_ty = Stab_Mtype_To_Ty(WN_opc_rtype(wn));
   TOKEN_BUFFER opnd;
   
   Is_True(WN_operator(wn) == OPR_MADD,
	   ("Invalid operator for WN2C_madd()"));

   /* Evaluate children to values, not to lvalues */
   CONTEXT_reset_needs_lvalue(context);
   
   CONTEXT_reset_top_level_expr(context);
   Append_Token_Special(tokens, '(');
   opnd = WN2C_Translate_Arithmetic_Operand(WN_kid1(wn), result_ty, context);
   Append_And_Reclaim_Token_List(tokens, &opnd);
   Append_Token_Special(tokens, '*');
   opnd = WN2C_Translate_Arithmetic_Operand(WN_kid(wn,2), result_ty, context);
   Append_And_Reclaim_Token_List(tokens, &opnd);
   Append_Token_Special(tokens, '+');
   opnd = WN2C_Translate_Arithmetic_Operand(WN_kid0(wn), result_ty, context);
   Append_And_Reclaim_Token_List(tokens, &opnd);
   Append_Token_Special(tokens, ')');

   return EMPTY_STATUS;
} /* WN2C_madd */


static STATUS 
WN2C_msub(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   const TY_IDX result_ty = Stab_Mtype_To_Ty(WN_opc_rtype(wn));
   TOKEN_BUFFER opnd;
   
   Is_True(WN_operator(wn) == OPR_SUB,
	   ("Invalid operator for WN2C_msub()"));

   /* Evaluate children to values, not to lvalues */
   CONTEXT_reset_needs_lvalue(context);
   
   CONTEXT_reset_top_level_expr(context);
   Append_Token_Special(tokens, '(');
   opnd = WN2C_Translate_Arithmetic_Operand(WN_kid1(wn), result_ty, context);
   Append_And_Reclaim_Token_List(tokens, &opnd);
   Append_Token_Special(tokens, '*');
   opnd = WN2C_Translate_Arithmetic_Operand(WN_kid(wn,2), result_ty, context);
   Append_And_Reclaim_Token_List(tokens, &opnd);
   Append_Token_Special(tokens, '-');
   opnd = WN2C_Translate_Arithmetic_Operand(WN_kid0(wn), result_ty, context);
   Append_And_Reclaim_Token_List(tokens, &opnd);
   Append_Token_Special(tokens, ')');

   return EMPTY_STATUS;
} /* WN2C_msub */


static STATUS 
WN2C_nmadd(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   const TY_IDX result_ty = Stab_Mtype_To_Ty(WN_opc_rtype(wn));
   TOKEN_BUFFER opnd;
   
   Is_True(WN_operator(wn) == OPR_NMADD,
	   ("Invalid operator for WN2C_nmadd()"));

   /* Evaluate children to values, not to lvalues */
   CONTEXT_reset_needs_lvalue(context);
   
   CONTEXT_reset_top_level_expr(context);
   Append_Token_Special(tokens, '-');
   Append_Token_Special(tokens, '(');
   opnd = WN2C_Translate_Arithmetic_Operand(WN_kid1(wn), result_ty, context);
   Append_And_Reclaim_Token_List(tokens, &opnd);
   Append_Token_Special(tokens, '*');
   opnd = WN2C_Translate_Arithmetic_Operand(WN_kid(wn,2), result_ty, context);
   Append_And_Reclaim_Token_List(tokens, &opnd);
   Append_Token_Special(tokens, '+');
   opnd = WN2C_Translate_Arithmetic_Operand(WN_kid0(wn), result_ty, context);
   Append_And_Reclaim_Token_List(tokens, &opnd);
   Append_Token_Special(tokens, ')');

   return EMPTY_STATUS;
} /* WN2C_nmadd */


static STATUS 
WN2C_nmsub(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   const TY_IDX result_ty = Stab_Mtype_To_Ty(WN_opc_rtype(wn));
   TOKEN_BUFFER opnd;
   
   Is_True(WN_operator(wn) == OPR_NMSUB,
	   ("Invalid operator for WN2C_nmsub()"));

   /* Evaluate children to values, not to lvalues */
   CONTEXT_reset_needs_lvalue(context);
   
   CONTEXT_reset_top_level_expr(context);
   Append_Token_Special(tokens, '-');
   Append_Token_Special(tokens, '(');
   opnd = WN2C_Translate_Arithmetic_Operand(WN_kid1(wn), result_ty, context);
   Append_And_Reclaim_Token_List(tokens, &opnd);
   Append_Token_Special(tokens, '*');
   opnd = WN2C_Translate_Arithmetic_Operand(WN_kid(wn,2), result_ty, context);
   Append_And_Reclaim_Token_List(tokens, &opnd);
   Append_Token_Special(tokens, '-');
   opnd = WN2C_Translate_Arithmetic_Operand(WN_kid0(wn), result_ty, context);
   Append_And_Reclaim_Token_List(tokens, &opnd);
   Append_Token_Special(tokens, ')');

   return EMPTY_STATUS;
} /* WN2C_nmsub */


static STATUS 
WN2C_ldid(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* This is similar to the WN2C_iload case, except that pregs must 
    * be handled specially!
    */
   TY_IDX       object_ty;    /* Type of object loaded */
   TY_IDX       base_addr_ty; /* Type of (qualified) address loaded from */
   TY_IDX       lda_st_ty;    /* Formal reference parameter type */
   STATUS       load_status;
   TOKEN_BUFFER expr_tokens;
   STAB_OFFSET  addr_offset;


   Is_True(WN_operator(wn) == OPR_LDID || 
	   (WN_operator(wn) == OPR_LDA && 
	    ST_sclass(WN_st(wn)) == SCLASS_FORMAL_REF),
	   ("Invalid operator for WN2C_ldid()"));

   /* Special case for Purple */
   if (W2C_Only_Mark_Loads && !TY_Is_Pointer(WN_ty(wn)))
   {
      char buf[64];
      sprintf(buf, "#<%p>#", wn);
      Append_Token_String(tokens, buf);
      return EMPTY_STATUS;
   }

   if (WN_operator(wn) == OPR_LDID &&
       ST_sclass(WN_st(wn)) == SCLASS_FORMAL_REF)
      return WN2C_iload(tokens, wn, context); /* Treat LDID as indirect load */
   else if (WN_operator(wn) == OPR_LDA)
   {
      lda_st_ty = ST_type(WN_st(wn));
      Set_ST_type(WN_st(wn), Stab_Pointer_To(ST_type(WN_st(wn))));
   }

   /* Get the load expression */
   addr_offset = WN_load_offset(wn);
   expr_tokens = New_Token_Buffer();   
   TY_IDX prefered_ty = WN_ty(wn);
   if (ST_sym_class(WN_st(wn)) == CLASS_PREG)
   {
      /* When loading a preg, always load it as a value of the type
       * with which it is declared.
       */
      char buffer[64];
      object_ty = PUinfo_Preg_Type(ST_type(WN_st(wn)), addr_offset);
      if (addr_offset == -1) { 
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
	    Append_Token_String(expr_tokens, buffer); 
	    break;
	 case MTYPE_F4:
	 case MTYPE_F8:
	 case MTYPE_FQ:
	 case MTYPE_C4:
	 case MTYPE_C8:
	 case MTYPE_CQ:
            sprintf(buffer, "reg%d", First_Float_Preg_Return_Offset);
	    Append_Token_String(expr_tokens, buffer); 
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
	WN2C_Append_Preg(expr_tokens, 
			 WN_st(wn),   /* preg */
			 addr_offset, /* preg index */
			 object_ty,   /* preg type */
			 context);
      }  
   }
   else
   {
      /* Get the lhs, preferably as an lvalue, but possibly as an address.
       */
     //FIX:  For field accesses, set preferred type to that of the field

     if (WN_operator(wn) == OPR_LDID &&
	 TY_Is_Structured(prefered_ty) &&
	 WN_field_id(wn) != 0 ) {
       prefered_ty = Get_Field_Type(prefered_ty, WN_field_id(wn));   
     }
     WN2C_SymAccess_Type(&base_addr_ty,
			 &object_ty,
			 ST_type(WN_st(wn)), /* base_type */
			 prefered_ty,          /* preferred type */
			 WN_opc_dtype(wn),   /* required mtype */
			 addr_offset);       /* offset from base */
     //cout << "IN LDID: OBJECT TYPE: " << object_ty << endl; 

      /* Get the lvalue or address of the data to be loaded */
      load_status = WN2C_lvalue_st(expr_tokens,
				   WN_st(wn),    /* base symbol loaded from */
				   base_addr_ty, /* address loaded from */
				   object_ty,    /* type of object loaded */
				   addr_offset,
				   context);

      /* Dereference the address from which we are loading, if necessary */
      if (!STATUS_is_lvalue(load_status))
	 Prepend_Token_Special(expr_tokens, '*');

   //need output (*type) for void pointer----fzhao
   if (TY_kind(ST_type(WN_st(wn))) == KIND_POINTER &&
          TY_kind(TY_pointed(ST_type(WN_st(wn))))==KIND_VOID &&
	  TY_kind(TY_pointed(WN_ty(wn))) != KIND_VOID) {

      Prepend_Token_String(expr_tokens,"*)");

      if (!TY_Is_Structured(TY_pointed(WN_ty(wn))))
           Prepend_Token_String(expr_tokens,
                           Scalar_C_Names[TY_mtype(TY_pointed(WN_ty(wn)))].pseudo_name);
       else {
            Prepend_Token_String(expr_tokens,TY_name(TY_pointed(WN_ty(wn))));
            Prepend_Token_String(expr_tokens,"struct ");
        }
        Prepend_Token_Special(expr_tokens, '(');
     }
  }
   
   /* Cast the resultant value to the expected result type if different 
    * from the type of value loaded.
    */

   //FIX: Compare to prefered type instead
   if (!WN2C_arithmetic_compatible_types(object_ty, prefered_ty))
   {
      
      if (!TY_Is_Structured(object_ty) && !TY_Is_Structured(WN_ty(wn)))
	 WN2C_prepend_cast(expr_tokens, WN_ty(wn), FALSE/*pointer_to_type*/);
      else
      {
	 /* Assign the expr_tokens to a temporary, cast the address of the
	  * temporary to a pointer to a WN_ty(wn), then dereference.
	  */
	 const UINT  tmp_idx = Stab_Lock_Tmpvar(object_ty, 
						ST2C_Declare_Tempvar);
	 const char *tmpvar_name = W2CF_Symtab_Nameof_Tempvar(tmp_idx);

	 /* Generate "(tmpvar = e," */
	 Append_Token_Special(tokens, '(');
	 Append_Token_String(tokens, tmpvar_name);
	 Append_Token_Special(tokens, '=');
	 Append_And_Reclaim_Token_List(tokens, &expr_tokens);
	 Append_Token_Special(tokens, ',');

	 /* Generate "*(ty *)&tmpvar)" */
	 expr_tokens = New_Token_Buffer();
	 Append_Token_Special(expr_tokens, '&');
	 Append_Token_String(expr_tokens, tmpvar_name);
	 WN2C_prepend_cast(expr_tokens, WN_ty(wn), TRUE/*pointer_to_type*/);
	 Prepend_Token_Special(expr_tokens, '*');
	 Append_Token_Special(expr_tokens, ')');
	 Stab_Unlock_Tmpvar(tmp_idx);
      }
   }
   Append_And_Reclaim_Token_List(tokens, &expr_tokens);

   if (WN_operator(wn) == OPR_LDA)
      Set_ST_type(WN_st(wn), lda_st_ty);

   return EMPTY_STATUS;
} /* WN2C_ldid */


static STATUS 
WN2C_lda(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   TY_IDX       object_ty;
   STATUS       lda_status;
   TOKEN_BUFFER expr_tokens;
   STAB_OFFSET  lda_offset;

   Is_True(WN_operator(wn) == OPR_LDA,
	   ("Invalid operator for WN2C_lda()"));
   Is_True(ST_sym_class(WN_st(wn)) != CLASS_PREG, 
	   ("Cannot take the address of a preg"));

   if (ST_sclass(WN_st(wn)) == SCLASS_FORMAL_REF)
      return WN2C_ldid(tokens, wn, context);  /* Treat LDA as a direct load */

   expr_tokens = New_Token_Buffer();
   if (ST_sym_class(WN_st(wn)) == CLASS_CONST &&
       TCON_ty(STC_val(WN_st(wn))) != MTYPE_STRING)
   {
      /* A constant (Fortran input?), so refer to it's address 
       * via a non-shared temporary variable.
       */
      UINT tmp_idx = 
	 Stab_Lock_Tmpvar(ST_type(WN_st(wn)), &ST2C_Declare_Tempvar);
      const char *tmp_name = W2CF_Symtab_Nameof_Tempvar(tmp_idx);

      Append_Token_Special(tokens, '(');
      if (TY_Is_Complex(ST_type(WN_st(wn))))
	 WN2C_Assign_Complex_Const(expr_tokens,
				   tmp_name,
				   Extract_Complex_Real(STC_val(WN_st(wn))),
				   Extract_Complex_Imag(STC_val(WN_st(wn))));
      else
      {
	 Append_Token_String(expr_tokens, tmp_name);
	 Append_Token_Special(expr_tokens, '=');
	 TCON2C_translate(expr_tokens, STC_val(WN_st(wn)));
      }
      Append_Token_Special(expr_tokens, ',');
      Append_Token_Special(expr_tokens, '&');
      Append_Token_String(expr_tokens, tmp_name);
      Append_Token_Special(expr_tokens, ')');

      if (!TY_Is_Pointer(WN_ty(wn)) ||
	  !WN2C_arithmetic_compatible_types(ST_type(WN_st(wn)),
					    TY_pointed(WN_ty(wn))))
      {
	 WN2C_prepend_cast(expr_tokens, WN_ty(wn), FALSE/*ptr_to_type*/);
      }
      lda_status = EMPTY_STATUS;
   }
   else if ((ST_sym_class(WN_st(wn)) == CLASS_FUNC ?
	     ST_pu_type(WN_st(wn)) : ST_type(WN_st(wn))) == 0)
   {
      // in case of compiler generated symbols, the type may be null
      //
      const char *tmp_name = ST_name(WN_st(wn));
      if (tmp_name == NULL)
      {
	 UINT tmp_idx = 
	    Stab_Lock_Tmpvar(MTYPE_To_TY(MTYPE_I4), &ST2C_Declare_Tempvar);
	 tmp_name = W2CF_Symtab_Nameof_Tempvar(tmp_idx);
      }
      Append_Token_String(expr_tokens, tmp_name);
      lda_status = EMPTY_STATUS;
   }
   else /* The normal case */
   {
      lda_offset = WN_lda_offset(wn);
      if (ST_sym_class(WN_st(wn)) == CLASS_CONST &&
	  TCON_ty(STC_val(WN_st(wn))) == MTYPE_STRING)
      {
	 /* front-end typing is unreliable for this case, so hard
	  * code the expected referenced type here.
	  */
	 object_ty = Stab_Mtype_To_Ty(MTYPE_U1);
      }
      else if (TY_Is_Pointer(WN_ty(wn)))
      {
         TY_IDX wn_st_type = ST_sym_class(WN_st(wn)) == CLASS_FUNC ?
                                    ST_pu_type(WN_st(wn)) : ST_type(WN_st(wn));
	 object_ty =
	    WN2C_MemAccess_Type(
               wn_st_type,                   /* base_type */
	       TY_pointed(WN_ty(wn)),        /* preferred type */
	       TY_mtype(wn_st_type),         /* required mtype */
	       lda_offset);                  /* offset from base */

        /*if the object type is "struct"(union) need to output the corresponding
          selector------fzhao
         */
         if (TY_Is_Structured(object_ty) && WN_field_id(wn) != 0 )
            object_ty = Get_Field_Type(object_ty,WN_field_id(wn));

      }
      else

      {
         object_ty = ST_sym_class(WN_st(wn)) == CLASS_FUNC ?
                                    ST_pu_type(WN_st(wn)) : ST_type(WN_st(wn));
      }
      
      /* Get the lvalue or address for the identifier */
      expr_tokens = New_Token_Buffer();
      lda_status = 
	 WN2C_lvalue_st(expr_tokens, 
			WN_st(wn),  /* base st and the addr ty */
                        Stab_Pointer_To(ST_sym_class(WN_st(wn)) == CLASS_FUNC ?
                                   ST_pu_type(WN_st(wn)) : ST_type(WN_st(wn))),
			object_ty,  /* type addressed */
			lda_offset, 
			context);

      /* Convert an lvalue into an address value, if necessary.*/
      if (!TY_Is_Pointer(WN_ty(wn)) ||
	  !WN2C_arithmetic_compatible_types(object_ty, TY_pointed(WN_ty(wn))))
      {
	 if (STATUS_is_lvalue(lda_status))
	    Prepend_Token_Special(expr_tokens, '&');
	 
//	 WN2C_prepend_cast(expr_tokens, WN_ty(wn), FALSE/*ptr_to_type*/);
// unparsed code with the type casting will 
// cause gcc/frontend issue warnings----FMZ

	 STATUS_reset_lvalue(lda_status); /* Not returning an lvalue */
      }
      else if (STATUS_is_lvalue(lda_status) && !CONTEXT_needs_lvalue(context))
      {
	 Prepend_Token_Special(expr_tokens, '&');
	 STATUS_reset_lvalue(lda_status); /* We not returning an lvalue */
      }
   }
   Append_And_Reclaim_Token_List(tokens, &expr_tokens);

   return lda_status;
} /* WN2C_lda */


static STATUS 
WN2C_const(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* This handles constants pointed to through an ST entry
    * (used for shared and non-integral constant values).
    */
   Is_True(WN_operator(wn) == OPR_CONST && 
	   ST_sym_class(WN_st(wn)) == CLASS_CONST,
	   ("Invalid operator for WN2C_const"));
   
   CONTEXT_reset_needs_lvalue(context); /* Denotes a value, not an lvalue */
   if (!TY_Is_Complex(WN_Tree_Type(wn)))
      TCON2C_translate(tokens, STC_val(WN_st(wn)));
   else
   {
      UINT tmp_idx = 
	 Stab_Lock_Tmpvar(WN_Tree_Type(wn), &ST2C_Declare_Tempvar);
      const char *tmp_name = W2CF_Symtab_Nameof_Tempvar(tmp_idx);
      
      /* A complex constant, so refer to it via a non-shared 
       * temporary variable.
       */
      Append_Token_Special(tokens, '(');
      WN2C_Assign_Complex_Const(tokens,
				tmp_name,
				Extract_Complex_Real(STC_val(WN_st(wn))),
				Extract_Complex_Imag(STC_val(WN_st(wn))));
      Append_Token_Special(tokens, ',');
      Append_Token_String(tokens, tmp_name);
      Append_Token_Special(tokens, ')'); /* gotten complex structure */
   }

   return EMPTY_STATUS;
} /* WN2C_const */


static STATUS 
WN2C_intconst(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   Is_True(WN_operator(wn) == OPR_INTCONST,
	   ("Invalid operator for WN2C_intconst()"));

   CONTEXT_reset_needs_lvalue(context); /* Denotes a value, not an lvalue */
   TCON2C_translate(tokens, Host_To_Targ(WN_opc_rtype(wn), WN_const_val(wn)));
   
   return EMPTY_STATUS;
} /* WN2C_intconst */


static STATUS 
WN2C_comma(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   Is_True(WN_operator(wn) == OPR_COMMA,
	   ("Invalid operator for WN2C_comma()"));

   CONTEXT_reset_top_level_expr(context);
   Append_Token_Special(tokens, '(');
   Append_Token_Special(tokens, '(');
   WN2C_Translate_Comma_Sequence(tokens, WN_first(WN_kid0(wn)), context);
   Append_Token_Special(tokens, ')');
   Append_Token_Special(tokens, ',');
   WN2C_translate(tokens, WN_kid1(wn), context);
   Append_Token_Special(tokens, ')');
   return EMPTY_STATUS;
} /* WN2C_comma */


static STATUS 
WN2C_rcomma(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* Assign the rhs to a temporary, do the side-effects and return
    * the temporary as the value of the expression.
    */
   UINT         tmp_idx;
   const char  *tmp_name;
	 
   Is_True(WN_operator(wn) == OPR_RCOMMA,
	   ("Invalid operator for WN2C_romma()"));

   CONTEXT_reset_top_level_expr(context);
   Append_Token_Special(tokens, '(');

   tmp_idx = Stab_Lock_Tmpvar(WN_Tree_Type(WN_kid0(wn)), 
			      &ST2C_Declare_Tempvar);
   tmp_name = W2CF_Symtab_Nameof_Tempvar(tmp_idx);
   Append_Token_String(tokens, tmp_name);
   Append_Token_Special(tokens, '=');
   WN2C_translate(tokens, WN_kid0(wn), context);
   Append_Token_Special(tokens, ',');
   Append_Token_Special(tokens, '(');
   WN2C_Translate_Comma_Sequence(tokens, WN_first(WN_kid1(wn)), context);
   Append_Token_Special(tokens, ')');
   Append_Token_Special(tokens, ',');
   Append_Token_String(tokens, tmp_name);
   Append_Token_Special(tokens, ')');
   return EMPTY_STATUS;
} /* WN2C_rcomma */


static STATUS
WN2C_parm(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* TODO: handle opcode parms properly, i.e. take some advantage
    * of the information provided in this packaging of argument 
    * expressions.  For now, just skip these nodes.
    */
   TOKEN_BUFFER parm_tokens;
   STATUS       status;
   TY_IDX       parm_ty = CONTEXT_given_lvalue_ty(context);

   CONTEXT_reset_given_lvalue_ty(context);

   Is_True(WN_operator(wn) == OPR_PARM,
	   ("Invalid operator for WN2C_parm()"));

   if (parm_ty == TY_IDX_ZERO)
   {
      // Not provided by context (e.g. for intrinsic_op).
      //
      parm_ty = WN_ty(wn);
   }
   
   if (WN2C_assignment_compatible_types(parm_ty, WN_Tree_Type(WN_kid0(wn))))
      status = WN2C_translate(tokens, WN_kid0(wn), context);
   else
   {
      parm_tokens = New_Token_Buffer();
      CONTEXT_reset_top_level_expr(context);
      WN2C_translate(parm_tokens, WN_kid0(wn), context);
      WN2C_prepend_cast(parm_tokens, parm_ty, FALSE/*pointer_to_type*/);
      Append_And_Reclaim_Token_List(tokens, &parm_tokens);
      status = EMPTY_STATUS;
   }
   return status;
} /* WN2C_parm */


static STATUS
WN2C_alloca(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
  STATUS status ;

  Append_Token_String(tokens,"__opr_alloca");
  Append_Token_Special(tokens,'(');
  status = WN2C_translate(tokens,WN_kid0(wn),context);
  Append_Token_Special(tokens,')');

  return status;
}

static STATUS
WN2C_dealloca(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{

  INT16 n,i;
  STATUS status ;

  n = WN_kid_count(wn);

  WN2C_Stmt_Newline(tokens,WN_linenum(wn));
  Append_Token_String(tokens,"__opr_dealloca");
  Append_Token_Special(tokens,'(');

  i = 0 ;
  while (i < n)
  {
    status = WN2C_translate(tokens,WN_kid(wn,i),context);
    if (++i < n)
      Append_Token_Special(tokens,',');
  }
  Append_Token_Special(tokens,')');
  return status ;
}

static STATUS
WN2C_extract_bits(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
       //output ((... >>of) & 0xff..f)
   STATUS status ;
   UINT  of = WN_bit_offset(wn);
   UINT  sz = WN_bit_size(wn)/4;
   Is_True(WN_operator(wn) == OPR_EXTRACT_BITS,
           ("Invalid operator for WN2C_extract_bits()"));

      Append_Token_Special(tokens,'(');
      Append_Token_Special(tokens,'(');
      status = WN2C_translate(tokens,WN_kid0(wn),context);
      Append_Token_Special(tokens,')');
      Append_Token_String(tokens,">>");
      TCON2C_translate(tokens,
                    Host_To_Targ(MTYPE_I4,of));
      Append_Token_Special(tokens, ')');
      Append_Token_String(tokens,"& 0x");
      while (sz) {
 	   Append_Token_Special(tokens, 'f');
           --sz;
       }      
      
   return status;
 
}

static STATUS
WN2C_compose_bits(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   // ((... & 0xff..f) << of)
   STATUS status ;
   UINT  of = WN_bit_offset(wn);
          //shift of bits (for compose_bit,it is left shift (<< of)
   UINT sz = WN_bit_size(wn)/4;
   Is_True(WN_operator(wn) == OPR_COMPOSE_BITS,
           ("Invalid operator for WN2C_compose_bits()"));

      Append_Token_Special(tokens,'(');
      Append_Token_Special(tokens,'(');
      status = WN2C_translate(tokens,WN_kid1(wn),context);
      Append_Token_Special(tokens,')');
      Append_Token_String(tokens,"& 0x");

      while (sz) {
           Append_Token_Special(tokens, 'f');
           --sz;
       }
      Append_Token_Special(tokens, ')');
      Append_Token_String(tokens,"<<");
      TCON2C_translate(tokens,
                    Host_To_Targ(MTYPE_I4,of));

   return status;
}
/*------------------------ exported routines --------------------------*/
/*---------------------------------------------------------------------*/


void 
WN2C_initialize(void)
{
   INT opr;
   INT map;

   /* Reset the WN2C_Opr_Handle array */
   for (opr = 0; opr < NUMBER_OF_OPERATORS; opr++)
      WN2C_Opr_Handler[opr] = &WN2C_unsupported;

   /* Initialize the WN2C_Opr_Handle array */
   for (map = 0; map < NUMBER_OF_OPR2HANDLER_MAPS; map++)
      WN2C_Opr_Handler[WN2C_Opr_Handler_Map[map].opr] =
	 WN2C_Opr_Handler_Map[map].handler;

   /* Reset the WN2C_Opc2cname array.  This has already been
    * implicitly done by declaring it as static:
    *
    *    OPCODE   opc;
    *    for (opc = 0; opc < NUMBER_OF_OPCODES; opc++)
    *       WN2C_Opc2cname[opc] = NULL;
    *
    * Initialize the WN2C_Opc2cname array
    */
   for (map = 0; map < NUMBER_OF_OPC2CNAME_MAPS; map++)
      WN2C_Opc2cname[WN2C_Opc2cname_Map[map].opc] = 
	 WN2C_Opc2cname_Map[map].cname;
   
} /* WN2C_initialize */


void 
WN2C_finalize(void)
{
   /* Free up information pertaining temporary whirl2c variables,
    * recover the "pointer" fields where we have created them
    * during whirl2c processing, and reclaim the Type_Buffers.
    * Also, reset the "referenced" flag for file-level symbols.
    */
   Stab_Free_Tmpvars();
} /* WN2C_finalize */


BOOL 
WN2C_new_symtab(void)
{
   /* Returns TRUE if the Current_Symtab is different from the last
    * time this function was called.  This is useful to determine if
    * the symtab has changed (in which case we should emit new 
    * declarations).  Used when processing WN block nodes.
    */
   static SYMTAB_IDX WN2C_Current_Symtab = 0;
   const BOOL new_symtab = (CURRENT_SYMTAB != WN2C_Current_Symtab);

   if (new_symtab)
      WN2C_Current_Symtab = CURRENT_SYMTAB;
   return new_symtab;
} /* WN2C_new_symtab */


STATUS 
WN2C_translate(TOKEN_BUFFER tokens, const WN *wn, CONTEXT context)
{
   /* Dispatch to the appropriate handler for this construct,
    * The handlers should all have the same prototype, just in
    * case we later decide to do this dispatch via a handler 
    * table instead of via this switch statement.
    */
   return WN2C_Opr_Handler[WN_operator(wn)](tokens, wn, context);
} /* WN2C_translate */


void
WN2C_translate_structured_types(void)
{
   Write_String(W2C_File[W2C_DOTH_FILE], NULL/* No srcpos map */,
		"/* Types */\n");
   WN2C_Append_Symtab_Types(NULL, /* token_buffer */
			    2);   /* lines between decls */
} /* WN2C_translate_structured_types */


STATUS 
WN2C_translate_file_scope_defs(CONTEXT context)
{
    /* Make the global symbol table the Current_Symtab and write 
     * the declarations to file.
     */
   CURRENT_SYMTAB = GLOBAL_SYMTAB;
   WN2C_new_symtab();

   //WEI: don't see why this needs to be called
//#if 0 WEI set "#if 0/#endif" cause some static constant
// missed output to the corresponding w2c.h file.This cause bug #19 (see bug
// track website),get back the function call----fzhao

/* we have to keep dumpout "ST_IS_CONST_VAR" symtab entry and get rid of
   "CLASS_CONST" entry dumpout. Change the function "WN2C_Append_Symtab_Consts"
    instead.----------fzhao
 */

   Write_String(W2C_File[W2C_DOTH_FILE], NULL/* No srcpos map */,
		"/* File-level symbolic constants */\n");
   WN2C_Append_Symtab_Consts(NULL, /* token_buffer */ 
			     TRUE, /* use const_tab */
			     2,    /* lines between decls */
			     context);
//#endif

   Write_String(W2C_File[W2C_DOTH_FILE], NULL/* No srcpos map */,
		"/* File-level vars and routines */\n");
   WN2C_Append_Symtab_Vars(NULL, /* token_buffer */
			   2,    /* lines between decls */
			   context);

   return EMPTY_STATUS;
} /* WN2C_translate_file_scope_defs */


STATUS 
WN2C_translate_purple_main(TOKEN_BUFFER tokens, 
			   const WN    *pu, 
			   const char  *region_name,
			   CONTEXT      context)
{
   static const char prp_return_var_name[] = "prp___return";

   TY_IDX    return_ty;
   const ST *param_st;
   INT       first_param, param;

   Is_True(WN_operator(pu) == OPR_FUNC_ENTRY, 
	   ("Invalid opcode for WN2C_translate_purple_main()"));

   /* Write function header, and begin the body on a new line
    */
   CONTEXT_set_srcpos(context, WN_Get_Linenum(pu));
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
   Append_Token_String(tokens, "int main()");

   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
   Append_Token_Special(tokens, '{');

   /* Insert parameters as local variables
    */
   Increment_Indentation();
   first_param = (PUINFO_RETURN_TO_PARAM? 1 : 0);
   for (param = first_param; param < WN_num_formals(pu); param++)
   {
      param_st = WN_st(WN_formal(pu, param));
      Append_Indented_Newline(tokens, 1);
      ST2C_decl_translate(tokens, param_st, context);
      Append_Token_Special(tokens, ';');
   }

   /* We do not really care what is returned from the purplized region,
    * but for correctness we insert a declaration for any return variable
    * here, with a default name.
    */
   return_ty = PUINFO_RETURN_TY;
   if (return_ty != (TY_IDX) 0 && TY_kind(return_ty) != KIND_VOID)
   {
      TOKEN_BUFFER return_tokens = New_Token_Buffer();

      Append_Token_String(return_tokens, prp_return_var_name);
      TY2C_translate_unqualified(return_tokens, return_ty);
      Append_Indented_Newline(tokens, 1);
      Append_And_Reclaim_Token_List(tokens, &return_tokens);
   }

   /* Insert a placeholder for initialization of parameters
    */
   Append_Indented_Newline(tokens, 1);
   Append_Token_String(tokens, "<#PRP_XSYM:INIT_PARAM ");
   WN2C_Append_Purple_Funcinfo(tokens);
   Append_Token_String(tokens, "#>");

   /* Insert call to purple region routine
    */
   Append_Indented_Newline(tokens, 1);
   Append_Token_String(tokens,
		       "/***** Call to extracted purple region ****");
   Append_Indented_Newline(tokens, 1);
   Append_Token_String(tokens, "*/");
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
   if (return_ty != (TY_IDX) 0 && TY_kind(return_ty) != KIND_VOID)
   {
      Append_Token_String(tokens, prp_return_var_name);
      Append_Token_Special(tokens, '=');
   }
   Append_Token_String(tokens, region_name);
   Append_Token_Special(tokens, '(');
   for (param = 0; param < WN_num_formals(pu); param++)
   {
      if (param > 0)
	 Append_Token_Special(tokens, ',');
      param_st = WN_st(WN_formal(pu, param));
      Append_Token_String(tokens, W2CF_Symtab_Nameof_St(param_st));
   }
   Append_Token_Special(tokens, ')');
   Append_Token_Special(tokens, ';');

   /* Insert a placeholder for final testing of parameters
    */
   Append_Indented_Newline(tokens, 1);
   Append_Token_String(tokens, "<#PRP_XSYM:TEST_PARAM ");
   WN2C_Append_Purple_Funcinfo(tokens);
   Append_Token_String(tokens, "#>");

   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
   Append_Token_String(tokens, "return 0;");

   Decrement_Indentation();
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
   Append_Token_Special(tokens, '}');
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
   WN2C_Stmt_Newline(tokens, CONTEXT_srcpos(context));
   
   return EMPTY_STATUS;
} /* WN2C_translate_purple_main */


void
WN2C_memref_lhs(TOKEN_BUFFER tokens,
		TY_IDX      *memref_typ,
		const WN    *lhs,
		STAB_OFFSET  memref_ofst,
		TY_IDX       memref_addr_ty, 
		TY_IDX       memref_object_ty, 
		MTYPE        dtype,
		CONTEXT      context)
{
   /* Get the lvalue for an indirect memory reference (iload or
    * istore), returning the type of the resultant lvalue.
    */
   TY_IDX  base_ty;
   STATUS  lhs_status;

   /* Get the type of object being stored and the base-address from
    * which the store occurs.
   */
   base_ty = WN_Tree_Type(lhs);
   if (!TY_Is_Pointer(base_ty))
      base_ty = memref_addr_ty;
   *memref_typ = TY_pointed(memref_addr_ty);
   *memref_typ = WN2C_MemAccess_Type(TY_pointed(base_ty),/* base_type */
				     *memref_typ,        /* preferred type */
				     dtype,              /* required mtype */
				     memref_ofst);       /* offset from base */
   
   /* Avoid a later cast if possible */
   //if (TY_mtype(*memref_typ) == TY_mtype(memref_object_ty))
   //   *memref_typ = memref_object_ty;

   /* Get the lhs, preferably as an lvalue, but possibly as an 
    * address.
    */
   lhs_status = WN2C_lvalue_wn(tokens,
			       lhs,         /* the base address */
			       base_ty,     /* the base addr_ty */
			       *memref_typ, /* type of object stored */
			       memref_ofst,
			       context);

   /* Dereference the address into which we are storing, if 
    * necessary.
    */
   if (!STATUS_is_lvalue(lhs_status))  
      Prepend_Token_Special(tokens, '*');
} /* WN2C_memref_lhs */


void
WN2C_stid_lhs(TOKEN_BUFFER tokens,
	      TY_IDX      *stored_typ,
	      const ST    *lhs_st,
	      STAB_OFFSET  stid_ofst,
	      TY_IDX       stid_ty, 
	      MTYPE        dtype,
	      CONTEXT      context)
{
   /* Get the lvalue for the lhs of an stid assignment.
    */
   TY_IDX  base_ty;
   STATUS  lhs_status;

   if (ST_sym_class(lhs_st) == CLASS_PREG)
   {
      /* For assignments to pregs, we never cast the lhs, instead
       * casting the rhs to fit the type of the preg.
       */
      *stored_typ = PUinfo_Preg_Type(ST_type(lhs_st), stid_ofst);
      WN2C_Append_Preg(tokens, 
		       lhs_st,    /* preg */
		       stid_ofst,   /* preg index */
		       *stored_typ,/* preg type */
		       context);
   }
   else
   {
      /* Get the lhs, preferably as an lvalue, but possibly as an 
       * address.
       */
     

      WN2C_SymAccess_Type(&base_ty,
			  stored_typ,
			  ST_type(lhs_st), /* base_type */
			  stid_ty,         /* preferred type */
			  dtype,           /* required mtype */
			  stid_ofst);      /* offset from base */
      lhs_status = WN2C_lvalue_st(tokens,
				  lhs_st,      /* base stored into */
				  base_ty,     /* base ref stored into */
				  *stored_typ, /* type of object stored */
				  stid_ofst,
				  context);

      /* Dereference the address into which we are storing, if 
       * necessary 
       */
      if (!STATUS_is_lvalue(lhs_status))
	 Prepend_Token_Special(tokens, '*');
   }
} /* WN2C_stid_lhs */


