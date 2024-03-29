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
 * $Revision: 1.30 $
 * $Date: 2007-06-28 20:05:09 $
 * $Author: fzhao $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/crayf90/sgi/cwh_stab.cxx,v $
 *
 * Revision history:
 *  dd-mmm-95 - Original Version
 *
 * Description: This handles symbol table conversions - types are
 *              converted in cwh_types.c.  Variables and so forth
 *              come through fei_object and use a storage class set
 *              up by fei_seg. If the storage class is a COMMON, or
 *              BASED then fei_seg returns the ST of the base. Procedures
 *              come through fei_proc_def, labels through fei_label_def_named,
 *              constants in fei_arith_con or fei_pattern_con. 
 *              In general, the object created is passed back to PDGCS
 *              in a packet with a tag. It's stored by the FE which provides
 *              it as an argument when appropriate.
 *
 *              There are various odds and ends associated with STs
 *              in an auxiliary data structure called AUXST (cwh_auxst*). 
 *              The AUXST & a few lists allows information for the FE to
 *              accumulate until it's in a suitable form for WHIRL.
 *
 *              Definitions of bit masks, PDGCS values and the like
 *              are in fef90/i_cvrt.{m,h}
 *
 * ====================================================================
 * ====================================================================
 */
/*REFERENCED*/
static char *source_file = __FILE__;

#ifdef _KEEP_RCS_ID
static char *rcs_id = "$Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/crayf90/sgi/cwh_stab.cxx,v $ $Revision: 1.30 $";
#endif /* _KEEP_RCS_ID */


/* general compiler includes */

#include "defs.h"
#include "glob.h"  
#include "stab.h"
#include "strtab.h"
#include "errors.h"
#include "targ_const.h"
#include "config_targ.h"
#include "const.h"
#include "wn.h"
#include "wn_util.h"
#include "dwarf_DST_producer.h"
#include "cxx_memory.h"
#include "cwh_stk.h"
#include <stdio.h>

/* FE includes */

#include "i_cvrt.h"

/* conversion includes */

#include "cwh_defines.h"
#include "cwh_types.h"
#include "cwh_addr.h"
#include "cwh_expr.h"
#include "cwh_block.h"
#include "cwh_stmt.h"
#include "cwh_preg.h"
#include "cwh_auxst.h"
#include "cwh_stab.h"
#include "cwh_stab.i"
#include "cwh_dst.h"
#include "cwh_mkdepend.h"
#include "sgi_cmd_line.h"

/*===================================================
 *
 * fei_next_func_idx
 *
 * Called for any function or entry point - bump 
 * a counter to serve as an idx. 
 *
 * Create the SYMTAB for a function.
 * If this is an internal procedure, then it is 
 * processed before its parent. To nest correctly,
 * the parent's symtab is allocated first. A parent
 * may have several children, so don't reallocate 
 * its SYMTAB. SYMTABs are popped in cwh_stab_end_procs.
 * ( and briefly by fei_proc_parent)
 *
 ====================================================
*/
/*ARGSUSED*/
extern INTPTR
fei_next_func_idx(INT32 Pu_arg,
                  INT32 Proc_arg,
                  INT32 altentry_idx)
{

  STB_pkt *p ;
  static INT32 i = 0 ;
  PROC_CLASS proc ;

  proc = (PROC_CLASS) Proc_arg;

  if (altentry_idx == 0) {


    if (NOT_IN_PU ) {  

      New_Scope (HOST_LEVEL, FE_Mempool, TRUE );
      cwh_auxst_register_table();
      Host_Top = -1; //should keep this?
      Has_nested_proc = FALSE ;//?
      Hosted_Equivalences = NULL;
      Alttemp_ST    = NULL;
      Altbase_ST    = NULL;
      Altaddress_ST = NULL;

    }

    if (proc == PDGCS_Proc_Intern) {

      New_Scope (INTERNAL_LEVEL, FE_Mempool, TRUE);
      cwh_auxst_register_table();
    } 

    Equivalences = NULL;
    entry_point_count = 0 ;
    STB_list = NULL ;
  }

  i++;
  p = cwh_stab_packet(cast_to_void(i), is_CONST);
  return(cast_to_int(p));  
}


/*===================================================
 *
 * fei_proc
 *
 * This routine consolidates the interface routines
 * fei_proc_def(), fei_proc_parent(), fei_proc_imp().
 *
 ====================================================
*/
/*ARGSUSED*/
INTPTR
fei_proc(char         *name_string,
	 INT32         lineno,
	 INT32         Sym_class_arg,
	 INT32         Class_arg,
	 INT32         num_dum_args,
	 INT32         parent_stx,
	 INT32         first_st_idx,
	 INT32         alt_entry_idx,
	 TYPE          result_type,
	 INT32         proc_idx,
         INT64         flags,
         INT32         in_interface,
         INT32         coarray_concurrent )
{
  INTPTR p;

  if (test_flag(flags, FEI_PROC_DEFINITION)){
     p = fei_proc_def(name_string,
                      lineno,
                      Sym_class_arg,
                      Class_arg,
                      0,
                      0,
                      num_dum_args,
                      parent_stx,
                      first_st_idx,
                      alt_entry_idx,
                      result_type,
                      0,
                      proc_idx,
                      flags, 
                      coarray_concurrent);
  }

  if (test_flag(flags, FEI_PROC_IN_INTERFACE)) {
     p = fei_proc_interface(name_string,
                      lineno,
                      Sym_class_arg,
                      Class_arg,
                      0,
                      0,
                      num_dum_args,
                      parent_stx,
                      first_st_idx,
                      alt_entry_idx,
                      result_type,
                      0,
                      proc_idx,
                      flags, 
                      coarray_concurrent);
   }


  if (test_flag(flags, FEI_PROC_PARENT)) {
     p = fei_proc_parent(name_string,
                         lineno,
                         Sym_class_arg,
                         0,
                         num_dum_args,
                         parent_stx,
                         first_st_idx,
                         alt_entry_idx,
                         result_type,
                         proc_idx,
                         flags);
  }

  if (test_flag(flags, FEI_PROC_IMPORTED)) {
     p = fei_proc_imp(lineno,
                      name_string,
                      0,
                      0,
                      Sym_class_arg,
                      Class_arg,
                      result_type,
                      flags,
		      in_interface);

  }

  return(p);
}



/*===================================================
 *
 * fei_proc_def
 *
 * Build an ST for an entry point to a procedure.
 * Establish local data structures (cwh_stab.i) to
 * record dummy arguments and alternate entry points.
 *
 * Internal and module procedures may need their 
 * names adjusting. All entry points go into the
 * global symbol table, as the BE doesn't look for 
 * TEXT STs in nested SYMTAB. If a procedure
 * was referenced earlier, an ST was created in 
 * fei_proc_imp, but without argument information,
 * so the ST is patched up here.
 *
 ====================================================
*/
/*ARGSUSED*/
INTPTR
fei_proc_def(char         *name_string,
	     INT32         lineno,
	     INT32         Sym_class_arg,
	     INT32         Class_arg,
	     INT32         unused1,
	     INT32         unused2,
	     INT32         num_dum_args,
	     INT32         parent_stx,
	     INT32         first_st_idx,
	     INT32         alt_entry_idx,
	     TYPE          result_type,
	     INT32         cmcs_node,
	     INT32         proc_idx,
             INT64         flags ,
             INT32         coarray_concurrent)
{
  ST * st    ;
  TY_IDX  ty    ;
  STB_pkt *p ;
  FUNCTION_SYM  sym_class;
  PROC_CLASS    Class;
  BOOL is_inline_func = FALSE;
  ST_EXPORT eclass;
  TY_IDX ret_ty;

  still_in_preamble = TRUE;

  sym_class = (FUNCTION_SYM) Sym_class_arg;
  Class = (PROC_CLASS) Class_arg;

  /* fn result type - void for results by formal */

  ret_ty = cast_to_TY(t_TY(result_type)) ;
  ty = cwh_types_mk_procedure_TY(ret_ty,num_dum_args,TRUE,FALSE); 

  if (Class == PDGCS_Proc_Intern) {

     eclass = EXPORT_LOCAL_INTERNAL;
     is_inline_func = TRUE;
     Has_nested_proc = TRUE;

  } else {

    eclass = EXPORT_PREEMPTIBLE;
    if (test_flag(flags,FEI_PROC_OPTIONAL_DIR)) 
      eclass = EXPORT_OPTIONAL;
    
  }

  /* Seen this symbol via a forward reference in fei_proc_imp?  */

  st = cwh_auxst_find_item(Top_Text,name_string);

  if (st == NULL) {
    
    PU_IDX idx = cwh_stab_mk_pu(ty, CURRENT_SYMTAB);

    st = New_ST(GLOBAL_SYMTAB);   
    cwh_auxst_clear(st);
    ST_Init (st, Save_Str(name_string), CLASS_FUNC, SCLASS_TEXT, eclass, (TY_IDX) idx);
    Set_ST_ofst(st,0);
    cwh_auxst_add_to_list(&Top_Text,st,FALSE);
    
    
   }
 else {
    Set_ST_sclass(st, SCLASS_TEXT);
    Set_ST_export(st, eclass);
  }

  /* if fei_proc_imp made ST, then ST has a default void return */
  /* which should be replaced with the correct return type/args */

  

  PU_IDX pu_idx = ST_pu(st);
  PU& pu = Pu_Table[pu_idx];
  pu.lexical_level =CURRENT_SYMTAB; /*"interface" declared in nested PU  gave a wrong
                                       * PU level;must reset the PU level for "later" 
                                       * defintion ---fzhao
                                       */

  Set_PU_prototype (pu, ty);
  Set_PU_f90_lang (pu);
  Set_PU_need_unparsed(pu); 

  if (is_inline_func)
     Set_PU_is_inline_function(pu);

  cwh_stab_set_linenum(st,lineno);

  /* is the MAIN anonymous? If not, create a MAIN */
  /* external for debug information               */

  if (sym_class == Main_Pgm) {

    INTPTR midx;
    Set_PU_is_mainpu(pu);
    Set_PU_no_inline(pu);

# if 0
 /*fzhao:don't generate this extra symbal table entry for main pgrogam !*/
    Main_ST = NULL;

    if (strcmp(crayf90_def_main,ST_name(st)) != 0) {

      midx = fei_proc_imp(lineno,
			  def_main,
			  0,
			  0,
			  Main_Pgm,
			  PDGCS_Proc_Imported,
			  result_type,
			  0);

      Main_ST = cast_to_ST(cast_to_STB(midx)->item);
      Set_ST_pu(Main_ST, pu_idx);
      cwh_stab_set_linenum(Main_ST,lineno);
    }
# endif
  }

#if 0
  if (sym_class == Fort_Blockdata)
    DevWarn(("TODO_NEW_SYMTAB: blockdata"));
#endif

  if (sym_class == F90_Module) {
     cwh_add_to_module_files_table(name_string);
  }

  if (Class == PDGCS_Proc_Intern) 
     Set_PU_is_nested_func(pu);

  if (Class == PDGCS_Proc_Extern) 
    if (Has_nested_proc) 
      Set_PU_uplevel(pu);

  if (test_flag(flags, FEI_PROC_RECURSE))
    Set_PU_recursive(pu);

//  if (test_flag(flags,FEI_PROC_IN_INTERFACE) &&
//       test_flag(flags,FEI_PROC_M_IMPORTED))
//       Set_ST_is_M_imported(st);

  cwh_auxst_alloc_proc_entry(st,num_dum_args, ret_ty);

  if (test_flag(flags, FEI_PROC_HASRSLT))
    Set_ST_auxst_has_rslt_tmp(st,TRUE);

  if (test_flag(flags, FEI_PROC_ELEMENTAL))
    Set_ST_auxst_is_elemental(st,TRUE);

  if (test_flag(flags, FEI_PROC_MODULE))
    Set_ST_is_in_module(st);

  if (test_flag(flags, FEI_PROC_ENTRY)) {

    Set_ST_auxst_is_altentry(st,TRUE);
    cwh_auxst_add_item(Procedure_ST,st,l_ALTENTRY);

  } else {

    Scope_tab [Current_scope].st = st;
    Procedure_ST = st  ;
    cwh_stab_pu_has_globals = FALSE;

/* Since we need use this function to get interface block information   */
/* we have to keep blocks un_initialize when we get PUs by interface    */

    if (!test_flag(flags,FEI_PROC_IN_INTERFACE))
           cwh_block_init_pu();

     if (test_flag(flags, FEI_PROC_HAS_ALT_ENTRY)) 
           Set_PU_has_altentry(pu);
  }

  if ((Class == PDGCS_Proc_Extern) || 
      (Class == PDGCS_Proc_Intern)) 
    cwh_stab_adjust_name(st); 

  // cosubroutien or cofunction ---FMZ
  if ( coarray_concurrent ) 
      Set_ST_is_coarray_concurrent(st); 


  st_for_distribute_temp=NULL;
  preg_for_distribute.preg=-1;

  entry_point_count++ ;

  p = cwh_stab_packet(st, is_ST);
  return(cast_to_int(p));
}

/**************************************************************************/
INTPTR
fei_proc_interface(char         *name_string,
	     INT32         lineno,
	     INT32         Sym_class_arg,
	     INT32         Class_arg,
	     INT32         unused1,
	     INT32         unused2,
	     INT32         num_dum_args,
	     INT32         parent_stx,
	     INT32         first_st_idx,
	     INT32         alt_entry_idx,
	     TYPE          result_type,
	     INT32         cmcs_node,
	     INT32         proc_idx,
             INT64         flags,
	     INT32         coarray_concurrent )
{
  ST * st    ;
  TY_IDX  ty    ;
  STB_pkt *p ;
  FUNCTION_SYM  sym_class;
  PROC_CLASS    Class;
  BOOL is_inline_func = FALSE;
  ST_EXPORT eclass;
  TY_IDX ret_ty;

  sym_class = (FUNCTION_SYM) Sym_class_arg;
  Class = (PROC_CLASS) Class_arg;
  eclass = EXPORT_PREEMPTIBLE;

  /* fn result type - void for results by formal */

  ret_ty = cast_to_TY(t_TY(result_type)) ;
  ty = cwh_types_mk_procedure_TY(ret_ty,num_dum_args,TRUE,FALSE); 


  st = cwh_auxst_find_item(Top_Text,name_string);

  if (st == NULL) {

    PU_IDX idx = cwh_stab_mk_pu(ty, CURRENT_SYMTAB);
    st = New_ST(GLOBAL_SYMTAB);   
    cwh_auxst_clear(st);
    ST_Init (st, Save_Str(name_string), CLASS_FUNC, SCLASS_TEXT, eclass, (TY_IDX) idx);
    Set_ST_ofst(st,0);
    cwh_auxst_add_to_list(&Top_Text,st,FALSE);
    
   }

  /* if fei_proc_imp made ST, then ST has a default void return */
  /* which should be replaced with the correct return type/args */

  

  cwh_stab_set_linenum(st,lineno);
   PU_IDX pu_idx = ST_pu(st);
  PU& pu = Pu_Table[pu_idx];

  Set_PU_need_unparsed(pu);

  if (test_flag(flags, FEI_PROC_RECURSE))
    Set_PU_recursive(pu);

  cwh_auxst_alloc_proc_entry(st,num_dum_args, ret_ty);

  if (test_flag(flags, FEI_PROC_HASRSLT))
    Set_ST_auxst_has_rslt_tmp(st,TRUE);

  if (test_flag(flags, FEI_PROC_ELEMENTAL))
    Set_ST_auxst_is_elemental(st,TRUE);

  if (test_flag(flags, FEI_PROC_MODULE))
    Set_ST_is_in_module(st);

  if (test_flag(flags, FEI_PROC_ENTRY)) {

    Set_ST_auxst_is_altentry(st,TRUE);
    cwh_auxst_add_item(Procedure_ST,st,l_ALTENTRY);

  } else {

    Procedure_ST = st  ;

     if (test_flag(flags, FEI_PROC_HAS_ALT_ENTRY)) 
           Set_PU_has_altentry(pu);
  }


  // cosubroutien or cofunction ---FMZ
  if ( coarray_concurrent ) 
      Set_ST_is_coarray_concurrent(st);  


  st_for_distribute_temp=NULL;
  preg_for_distribute.preg=-1;

  entry_point_count++ ;

  p = cwh_stab_packet(st, is_ST);
  return(cast_to_int(p));
}


/*===================================================
 *
 * fei_proc_imp
 *
 * Build an ST for an function which is 
 * referenced in the code. Sometimes this is a TEXT
 * symbol created in fei_proc_def, so we go looking
 * for these first, before creating an EXTERNAL symbol.
 *
 * If this is a forward reference, a proc_def for the 
 * symbol may be seen later, when the ST created here
 * gets details filled in.
 *
 ====================================================
*/
/*ARGSUSED*/
INTPTR
fei_proc_imp(INT32 lineno,
	     char          *name_string,
	     INT32          unused1,
	     INT32          unused2,
	     INT32          Sclass_arg,
	     INT32          Class_arg,
	     TYPE           result_type,
	     INT64          flags,
             INT32          in_interface)
{
  ST * st  ;
  ST * st_local_cp;
  STB_pkt *p  ;
  PROC_CLASS     Class;
  FUNCTION_SYM   sym_class;
  TY_IDX ret_cp_ty;
  TY_IDX ty_cp;
  PU_IDX pu_cp_idx;

  INT map = 0;
  

  sym_class = (FUNCTION_SYM) Sclass_arg;
  Class = (PROC_CLASS) Class_arg;

  st = NULL ;
  switch (Class) {
  case PDGCS_Proc_Imported:      /* external subroutine */
  case PDGCS_Proc_Intern_Ref:
  case PDGCS_Proc_SrcIntrin:  /*PU is intrinsic function*/
    
    st = cwh_auxst_find_item(Top_Text,name_string);

    if ( st == NULL ) {

      ST_EXPORT eclass = EXPORT_PREEMPTIBLE;

      if (test_flag(flags,FEI_PROC_OPTIONAL_DIR)) 
	 eclass = EXPORT_OPTIONAL;

      // create procedure TY with 0 args. Don't know how many
      // there are in a forward ref. 


      INT32  level = HOST_LEVEL ;

      if (Class == PDGCS_Proc_Intern_Ref) 
      {     
	level  = INTERNAL_LEVEL;
	eclass = EXPORT_LOCAL_INTERNAL;
        
      }

    if (Class == PDGCS_Proc_SrcIntrin)  /* FMZ add for keep intrinsic as call */
      {
	level = INTERNAL_LEVEL;
        eclass = EXPORT_INTRINSIC; 
      }

    while (map < NUM_INAMEMAP &&
               (strcmp(Iname_Map[map].oldname,name_string)))
        ++map;

   if (map < NUM_INAMEMAP )
     st = cwh_stab_mk_fn_0args(Iname_Map[map].newname,
                                eclass,
                                level,
                                cast_to_TY(t_TY(result_type)));

  else
      st = cwh_stab_mk_fn_0args(name_string,
				eclass,
				level,
				cast_to_TY(t_TY(result_type)));
     

      cwh_auxst_add_to_list(&Top_Text,st,FALSE);
   }
    break;

  default:
    break;
  }

BOOL input_form_module = (test_flag(flags,FEI_PROC_M_IMPORTED));
BOOL declared_in_model = (test_flag(flags, FEI_PROC_MODULE) && !input_form_module);
                            

if (Class ==  PDGCS_Proc_Imported &&
       !in_interface 		  &&
       !input_form_module         &&
        !(sym_class == F90_Module)) {
  st_local_cp = Copy_ST(st,CURRENT_SYMTAB);
  st_local_cp->storage_class = SCLASS_EXTERN;
  ret_cp_ty = cast_to_TY(t_TY(result_type)) ;
  ty_cp = cwh_types_mk_procedure_TY(ret_cp_ty,0,TRUE,FALSE);
  pu_cp_idx = cwh_stab_mk_pu(ty_cp, CURRENT_SYMTAB);

  Set_PU_decl_view(pu_cp_idx); /*the extra PU entry for declaration only--Oct */
  Set_PU_need_unparsed(pu_cp_idx);
 
  st_local_cp->u2.type =(TY_IDX)pu_cp_idx ;
  Set_ST_ofst(st_local_cp, 0);

  if (!declared_in_model)
      Set_ST_base(st_local_cp,st);
  else Set_ST_is_in_module(st_local_cp);
}
    
  if (sym_class == F90_Module){
    Set_ST_emit_symbol(st);
    Set_ST_is_in_module(st);
   } 

  if (test_flag(flags, FEI_PROC_HASRSLT))
    Set_ST_auxst_has_rslt_tmp(st,TRUE) ;
  
  if (test_flag(flags, FEI_PROC_ELEMENTAL))
    Set_ST_auxst_is_elemental(st,TRUE);
  
  p = cwh_stab_packet(st, is_ST);
  return(cast_to_int(p));
}

/*===================================================
 *
 * fei_arith_con
 *
 * Build an ST for a constant, unless an integral
 * constant when we pass back a WN.
 *
 ====================================================
*/
extern INTPTR
fei_arith_con(TYPE type, SLONG *start)
{
  WN    * wn;
  ST    * st;
  TY_IDX ty;
  TYPE_ID bt;
  TCON    tcon;
  QUAD_TYPE q,q1 ;
  float   * f ; 
  double  * d ;
  STB_pkt * r ;
  INT64 iconst;

  ty = cast_to_TY(t_TY(type));
  bt = TY_mtype(ty) ;

  if (MTYPE_is_integral(bt)) {

     /* May need to sign-extend constant */
     if (bt == MTYPE_I8 || bt == MTYPE_U8) {
	iconst = *(INT64 *) start;
     } else {
	iconst = (INT64) * start;
     }
     if (bt == MTYPE_I1) {
	iconst = (iconst << 56) >> 56;
     } else if (bt == MTYPE_I2) {
	iconst = (iconst << 48) >> 48;
     } else if (bt == MTYPE_I4) {
	iconst = (iconst << 32) >> 32;
     }
     
     wn = WN_CreateIntconst(Intconst_Opcode [op_form [bt]],
			    iconst) ;

    r = cwh_stab_packet(wn,is_WN);

  } else if (MTYPE_is_void(bt)) {

    wn = WN_CreateIntconst(OPC_U8INTCONST,(INT64) * (UINT32 *)start) ;
    r  = cwh_stab_packet(wn,is_WN);

  } else if (MTYPE_is_float(bt)) {
    
    switch (bt) {
    case MTYPE_F4 :  
      tcon = Host_To_Targ_Float_4(bt,(float) * (float *) start);
      break ;

    case MTYPE_F8 :  
      tcon = Host_To_Targ_Float(bt,(double) * (double *) start);
      break ;

    case MTYPE_FQ:
      /* Convert from Cray IEEE format to MIPS format */
      memcpy(&q,start,sizeof (QUAD_TYPE));
      tcon = Host_To_Targ_Quad(q);
      break ; 

    case MTYPE_C4 :  
      f  = (float *) start ;
      tcon = Host_To_Targ_Complex_4 ( bt, *f, *(f + 1));
      break ;

    case MTYPE_C8 :  
      d  = (double *) start ;
      tcon = Host_To_Targ_Complex( bt, *d, *(d + 1));
      break ;

    case MTYPE_CQ :  
      memcpy(&q,start,sizeof (QUAD_TYPE));
      memcpy(&q1,start+4,sizeof (QUAD_TYPE));
      tcon = Host_To_Targ_Complex_Quad (q,q1);
      break ;

    default:
      DevAssert((0),("Odd float constant"));
    }

    st = New_Const_Sym(Enter_tcon (tcon), ty);
    r = cwh_stab_packet(st,is_ST);
		         
  } else
    DevAssert((0),("Unimplemented constant"));

  return (cast_to_int(r)) ;

}

/*===================================================
 *
 * fei_pattern_con
 *
 * Build an ST for an untyped or string constant. 
 * Strtab is global, assumes any TY global.
 *
 ====================================================
*/
/*ARGSUSED*/
extern INTPTR
fei_pattern_con(TYPE type,char *start,INT64 bitsize)
{
  TY_IDX  ty ;
  ST * st ;

  TCON  tc;

  ty = cast_to_TY(t_TY(type));
  tc = Host_To_Targ_String (MTYPE_STRING,start,TY_size(ty));
  st = Gen_String_Sym (&tc,ty,FALSE);
 
  return(cast_to_int(st));
  
}

/*===================================================
 *
 * fei_proc_parent
 *
 * Make the current SYMTAB the parent of an
 * internal procedure so Hosted variables can
 * be inserted into the host.
 *
 * For a recursive parent called from the child, it
 * may be this is the only time the function name is
 * seen, (its fei_proc_imp), so create an ST for
 * the parent.
 * 
 ====================================================
*/
/*ARGSUSED*/
INTPTR
fei_proc_parent( char          *name_string,
		INT32          lineno,
		INT32          Sym_class_arg,
		INT32          unused,
		INT32          num_dum_args,
		INT32          parent_stx,
		INT32          first_st_idx,
		INT32          aux_idx,
		TYPE           result_type,
		INTPTR         st_idx,
                INT64	       flags )
{
  INT32 level;
  FUNCTION_SYM   sym_class;

  sym_class = (FUNCTION_SYM) Sym_class_arg;
  
  st_idx = fei_proc_imp(lineno,
			name_string,
		        0,
			0,
			sym_class,
			PDGCS_Proc_Imported,
			result_type,
			flags,
                         0);

  level = PU_lexical_level(Get_Current_PU()) - 1;

  if (level != GLOBAL_SYMTAB) {
     STB_pkt * p ;

     Current_scope = level;

     // if this is a forward ref, the scope table ST hasn't been set.
     // set it here, so can use Get_Current_PU on host procedures
     // (say after fei_proc_parent has popped symtabs).

     p = cast_to_STB(st_idx);
     Scope_tab[level].st = cast_to_ST(p->item);
   }

  if (test_flag(flags, FEI_PROC_HAS_ALT_ENTRY))
    Set_PU_has_altentry(Get_Current_PU ()); 

  return(st_idx);
}

/*===================================================
 *
 * fei_object
 *
 * Build an ST for a symbol eg: a variable. The 
 * default behaviour is to build an ST using the type
 * info and storage_idx created earlier. But there
 * are many tweaks for edge cases.
 *
 * The FE considers hosted and internal symbol tables
 * distinct. WHIRL doesn't. Nested procedures appear
 * before their host, so if a reference to a hosted
 * thing appears, it's allocated in the host's
 * symbol table. Subsequent appearances lookup the
 * host object. If the nested procedure references 
 * the host function result or dummy argument, then
 * a call to fei_proc_parent resets the current symbol
 * table and the argument list of the host is processed.
 * 
 * 
 ====================================================
*/
/*ARGSUSED*/
INTPTR
fei_object(char * name_string,
	   TYPE        type,
	   INT64       flag_bits,
	   INT32       Sym_class_arg,
	   INTPTR      storage_idx,
	   INT32       arg_num,
	   INTPTR      ptr_st_idx,
	   INT64       offset,
	   INT32       arg_intent,
	   INT64       size,
	   INT32       type_aux,
	   INT32       alignment,
	   INT32       distr_idx,
	   INT32       node_1,
	   INT32       node_2,
	   INT32       lineno,
	   INTPTR      modst_idx)
{
  TY_IDX  ty ;
  TY_IDX  tr_idx;
  ST * st ;
  ST * st1;
  ST * base_st ;

  BOOL hosted ;
  BOOL eq     ;
  BOOL in_common ;
  BOOL derived_type_or_imported_var=FALSE;
  INT64 off   ;
  SYMTAB_IDX st_level;
  
  STB_pkt *p;
  STB_pkt *o;
  STB_pkt *b;
  STB_pkt *modp;
  

  OBJECT_SYM  sym_class;

  sym_class = (OBJECT_SYM) Sym_class_arg;

  ty = cast_to_TY(t_TY(type));
  p  = cast_to_STB(storage_idx);
/* need to seperate two cases:interface & contained pu */

 if (!interface_pu) 
    hosted = (sym_class == Hosted_Dummy_Procedure) ||
           (sym_class == Hosted_Dummy_Arg ) || 
           (sym_class == Hosted_Compiler_Temp) || 
           (sym_class == Hosted_User_Variable ) ||
           (sym_class == CRI_Pointee && 
	    (test_flag(flag_bits,FEI_OBJECT_INNER_REF) ||
	     test_flag(flag_bits,FEI_OBJECT_INNER_DEF))) ;
 else 
    hosted = FALSE;


  /* ignore hosted args w/o inner ref/defs because don't    */
  /* want duplicates in symbol table for debug info (only   */
  /* do lookup if inner ref/def, for speed). However flags  */
  /* on compiler temps not always set, and Namelist lists   */
  /* are built even if the ref/def isn't set on a varbl     */


  if (hosted && 
     sym_class != Hosted_Compiler_Temp && 
     !test_flag(flag_bits,FEI_OBJECT_INNER_REF) &&
     !test_flag(flag_bits,FEI_OBJECT_INNER_DEF) &&
     !test_flag(flag_bits,FEI_OBJECT_NAMELIST_ITEM))
     return (0);

  /* ignore stmt fn dummy arg - not used */

  if (test_flag(flag_bits,FEI_OBJECT_SF_DARG))
    return(0);


  /* is this a reference to a hosted object within a nested */
  /* routine? If so just return the hosted object           */


  if ((test_flag(flag_bits,FEI_OBJECT_INNER_REF)) ||
      (test_flag(flag_bits,FEI_OBJECT_INNER_DEF)) ||
      (sym_class == Hosted_Compiler_Temp)) {

    ST * sl = cwh_stab_earlier_hosted(name_string);
    if (sl != NULL) {

       cwh_stab_adjust_base_name(sl);

      /* if hosted dummy ref appeared within nested procedure     */
      /* add to dummy list of host, ie: what we're processing now */
      /* It may be a struct-by-value so don't add to arg list,but */
      /* need correct count & TY in internal data structures      */

      if (sym_class == Dummy_Arg || sym_class == Dummy_Procedure) {

        if (ST_is_return_var(sl) && TY_kind(ST_type(sl)) != KIND_POINTER)
	  cwh_auxst_patch_proc(ST_type(sl));

	else {

	  BOOL rtmp = test_flag(flag_bits,FEI_OBJECT_RESULT_TEMP);
          ST * dmst = sl;
	  
          /* if CQ fn entry point - add return address to arg list */

	  if (rtmp && Altaddress_ST != NULL)
	    dmst = Altaddress_ST ;

	  cwh_auxst_add_dummy(dmst,rtmp);
	}
      }

      st1 = Scope_tab[CURRENT_SYMTAB].st;
      if (ST_is_in_module(st1))  //current PU is moudel
        Set_ST_base(sl,st1);
      o = cwh_stab_packet(sl,is_ST);
      return(cast_to_int(o));
    }
  }


  /* offsets are set, but ignored for host variables, for     */
  /* locals, they don't appear. For common items we need them */

  off = 0 ;
  if (test_flag(flag_bits,FEI_OBJECT_OFF_ASSIGNED)) {

    off = bit_to_byte(offset);

    if (p->form == is_SCLASS)
      if ((cast_to_SCLASS((long)p->item) != SCLASS_COMMON) &&
          (cast_to_SCLASS((long)p->item) != SCLASS_MODULE) &&
          (cast_to_SCLASS((long)p->item) != SCLASS_DGLOBAL))
	off = 0 ;
  }

  /* is this a reference to an item in a COMMON which we've already   */
  /* seen? IF so find the ST being used for the element of the common */

  in_common = ((p->form == is_ST) && (IS_COMMON(cast_to_ST(p->item)))) ||
               ((sym_class == CRI_Pointee) && IS_COMMON(cast_to_ST((cast_to_STB(ptr_st_idx))->item)));
 
  if (in_common) {

    /* if it's a pointee in COMMON, its base is on the l_COMLIST  */
    /* and the ptr/pointee are associated via the auxst           */

    if (sym_class == CRI_Pointee) {

      STB_pkt *bb = cast_to_STB(ptr_st_idx);
      DevAssert((bb->form == is_ST),("odd pointer base"));

      ST * ptr = cast_to_ST(bb->item);
      DevAssert((ptr),("odd pointee"));

      st = cwh_auxst_cri_pointee(ST_base(ptr),0);
    } else {
      st = cwh_stab_seen_common_element(cast_to_ST(p->item),off,name_string);
    }
    
    if (st) {
      if (test_flag(flag_bits,FEI_OBJECT_NOT_PT_TO_UNIQUE_MEM)) {
        Clear_ST_pt_to_unique_mem(st);
      }
      o = cwh_stab_packet(st,is_ST);

      if (decl_distribute_pragmas) 
        cwh_stab_distrib_pragmas(st) ;
      return(cast_to_int(o));
    }
  }

  /*
   * keep derived types and module variables have single 
   * global ST entries ---FMZ
   *
   */

  derived_type_or_imported_var = modst_idx ? TRUE: FALSE;

  if (derived_type_or_imported_var && !in_common) {
      modp  = cast_to_STB(modst_idx);
      st = cwh_stab_seen_derived_type_or_imported_var(cast_to_ST(modp->item)
                                                     ,name_string);
   if (st) {
      o = cwh_stab_packet(st,is_ST);
      return(cast_to_int(o));
     }
  }

  /* figure out which symbol table this object goes in           */
  /* ie: is it in COMMON somehow perhpas via CRI_Pointer as base */

  if (in_common || (sym_class == Name)||
                                (test_flag(flag_bits, FEI_OBJECT_IN_MODULE))) {

/* add test_flag(flag_bits, FEI_OBJECT_IN_MODULE) to keep the initial  *
 * variables in module still to be in global ST table --fzhao          */

     st_level = GLOBAL_SYMTAB ;

  } else {

    st_level = CURRENT_SYMTAB;
    if (hosted && IN_NESTED_PU)
       st_level = HOST_LEVEL ;
  }

 if (test_flag(flag_bits, FEI_OBJECT_IN_MODULE))
        st_level = GLOBAL_SYMTAB ;

  st = New_ST(st_level);
  cwh_auxst_clear(st);

  ST_Init(st, 
	  Save_Str(name_string), 
	  object_map[sym_class],
          cast_to_SCLASS((long)p->item), 
	  EXPORT_LOCAL, 
	  ty);
 if (test_flag(flag_bits,FEI_OBJECT_IN_COMMON))
  if (sym_class == Name) {
     Set_ST_is_not_used (st);
  }

 if (test_flag(flag_bits, FEI_OBJECT_IN_MODULE) ) {
    if (!PU_is_nested_func(Pu_Table[ST_pu(Scope_tab[CURRENT_SYMTAB].st)]))  {
        st1 = Scope_tab[CURRENT_SYMTAB].st;
        cwh_auxst_add_item(st1,st,l_TYMDLIST) ;
     } else st1 = st;

     if (hosted)
        cwh_stab_enter_hosted(st);
  Set_ST_base(st,st1);

  }
    
  Set_ST_ofst(st, off);

  cwh_stab_set_linenum(st,lineno);  

  /* general setup above, special tweaks below here */

  /* ty is the function return TY. Make it  */
  /* into ptr TY of FUNCTION returning  ty  */
     
  if ((sym_class == Dummy_Procedure) || 
      (sym_class == Hosted_Dummy_Procedure))  {

    Set_ST_is_value_parm(st);
    ty = cwh_types_mk_procedure_TY (ty,0,TRUE,hosted);

    Set_ST_type(st, cwh_types_mk_pointer_TY(ty,hosted));
  }

  /* is this a compiler-generated temp? Mark if so. The FE sets the     */
  /* flag on static temps too, but the symbol table objects to          */
  /* {F,P}STATIC, DGLOBALS etc.                                         */

  if ((sym_class == Compiler_Temp) || 
      (sym_class == Hosted_Compiler_Temp)) {
    Set_ST_auxst_is_tmp(st,TRUE);

    if (ST_sclass(st) == SCLASS_AUTO   || 
        ST_sclass(st) == SCLASS_FORMAL ||
        ST_sclass(st) == SCLASS_FORMAL_REF)
          Set_ST_is_temp_var(st);
  }
  
  if (test_flag(flag_bits,FEI_OBJECT_PARAMETER))
     Set_ST_is_parameter(st);

  /* F90 pointers and assumed-shape dummies are non-contiguous */
  if (test_flag(flag_bits,FEI_OBJECT_PRIVATE))
     Set_ST_is_private(st);

  if (test_flag(flag_bits,FEI_OBJECT_ASSUMD_SHAPE) ||
      test_flag(flag_bits,FEI_OBJECT_DV_IS_PTR))  {
     Set_ST_auxst_is_non_contiguous(st, TRUE);
     Set_TY_is_f90_assumed_shape(ST_type(st));  
  }
 
  if (test_flag(flag_bits, FEI_OBJECT_DEFERRED_SHAPE))
    Set_TY_is_f90_deferred_shape(ST_type(st));  


  if (test_flag(flag_bits, FEI_OBJECT_ASSUMED_SIZE)) {
    Set_ST_auxst_is_assumed_size(st, TRUE) ;
    Set_TY_is_f90_assumed_size(ST_type(st)) ; } 

  if (test_flag(flag_bits, FEI_OBJECT_IN_MODULE))
    Set_ST_is_in_module(st);   
 if (test_flag(flag_bits, FEI_OBJECT_EXTERNAL))
    Set_ST_is_external(st);   


  if (test_flag(flag_bits,FEI_OBJECT_READ_ONLY)) {
    Set_ST_is_const_var(st);
  }

  /* if dummy, name is the address. CQ, array, character results  */
  /* are addresses. Struct temp addresses should be values if 16B */
  /* or less and are converted here rather than FE                */

  if (ST_sclass(st) == SCLASS_FORMAL) {
    BOOL formal = TRUE;

    if (test_flag(flag_bits,FEI_OBJECT_RESULT_TEMP)) {

      /* does ABI require fn result in regs - see cwh_defines.h */

# if 0
      if (STRUCT_BY_VALUE(ty)) {

      Set_ST_sclass(st, SCLASS_AUTO);

        if (! hosted)
          cwh_auxst_patch_proc(ty);

	formal = FALSE;
	sym_class = Function_Rslt ; 
	p->form   = is_UNDEF ;

      } else 
# endif

	Set_ST_auxst_is_rslt_tmp(st, TRUE);


      if (TY_kind(ty) != KIND_STRUCT) {
 
	/* character/array result varbl address or for CQ results, st will */
        /* be made so below, Structs will be FORMAL_REFS so no pointer     */

	Set_ST_type(st, cwh_types_mk_pointer_TY(ty,hosted)); 
	Set_ST_is_value_parm(st); 
      }

      if (TY_kind(ty) != KIND_SCALAR) {

	/* seen alt entry temp already? Use it. This one is same TY_IDX    */
        /* ie: ptr to dtype, character etc. Only scalar intrinsic entries  */
        /* may differ on result type. Alttemp_ST is for results of entry   */
	/* points so applies only to host (level) procedure result varbls  */  

	if (ST_level(st) == HOST_LEVEL) { 
//	  if (Alttemp_ST != NULL) 
//	    st = Alttemp_ST ;

//	  Alttemp_ST = st ;
	}

      } else if (TY_mtype(ty) == MTYPE_CQ) {

	/* CQ scalar result. If alt entry, make local temp  */
	/* & preserve ST as result address. Maybe hosted..  */
        /* but fei_proc_parent called so in host temporarily*/

	if (PU_has_altentry(Get_Current_PU())) {

	  ST * rt = st ;

          /* create a stack temp for result var and */
          /* an equivalence group for entry pts    */
  
	  st = cwh_stab_altentry_temp(ST_name(st),hosted);

	  Set_ST_name(rt, Save_Str(".resaddr."));

	  if (Altaddress_ST  == NULL)  
	    Altaddress_ST = rt ;

          if (hosted)
             Set_ST_has_nested_ref(Altaddress_ST); 
          else
	     cwh_auxst_add_dummy(Altaddress_ST,TRUE); 


          cwh_auxst_add_item(ST_base(st),st,l_EQVLIST);
          Set_ST_is_equivalenced(st);

	  sym_class = Function_Rslt ; 
	  p->form   = is_UNDEF ;
	  formal    = FALSE;
	} 
      } 

    } else {
        if (test_flag(flag_bits,FEI_OBJECT_OPTIONAL)) 
           Set_ST_is_optional_argument(st);

        switch (arg_intent) {
           case 1:
              Set_ST_is_intent_in_argument(st);
               break;

           case 2:
              Set_ST_is_intent_out_argument(st);
              break;
           default:
              break;

        } /*switch*/
     } 

    if (formal)
      cwh_stab_formal_ref(st,hosted);

 }

  /* allocatable & assumed shape cannot be aliases, unless a pointer TARGET */
  
  if (test_flag(flag_bits,FEI_OBJECT_ALLOCATE) || 
      test_flag(flag_bits,FEI_OBJECT_ASSUMD_SHAPE)) {

    if (!test_flag(flag_bits,FEI_OBJECT_TARGET) &&
        !test_flag(flag_bits,FEI_OBJECT_NOT_PT_TO_UNIQUE_MEM)) {
      Set_ST_pt_to_unique_mem(st);
    }
  }
  
  
  /* If automatic, create symbol, and pointer as        */
  /* base. Offset is ST of address temp (base).         */
  /* For Cray pointers, the base comes from ptr_st_idx. */
  /* If this is a Host Pointee, the only way to figure  */
  /* it is to look at the base and use its SYMTAB       */

  
  if (p->form == is_SCLASS && (cast_to_SCLASS((long)p->item) == SCLASS_BASED)) {

    if (sym_class == CRI_Pointee) {
      b = cast_to_STB(ptr_st_idx);
      base_st = cast_to_ST(b->item);
      cwh_auxst_cri_pointee(base_st, st);

    } else {
      b = cast_to_STB((UINTPS) offset);
      base_st = cast_to_ST(b->item);
    }

    Set_ST_base(st, base_st);
    Set_ST_ofst(st, 0);
    Set_ST_sclass(st, ST_sclass(base_st));

    Set_ST_auxst_is_auto_or_cpointer(st, TRUE);

    if (test_flag(flag_bits, FEI_OBJECT_TARGET)) 
      Set_ST_is_f90_target(base_st) ;
    else if (sym_class != CRI_Pointee &&
             !test_flag(flag_bits,FEI_OBJECT_NOT_PT_TO_UNIQUE_MEM))
      Set_ST_pt_to_unique_mem(base_st);

    Set_ST_type(base_st, cwh_types_mk_pointer_TY(ty,hosted));

    /* make base name into p_<based_varbl> for intelligibility & w2f output*/

    if (!hosted)
      cwh_stab_adjust_base_name(st);
  } 



  /* Is part of some earlier base, eg: common or equivalence */

  eq = test_flag(flag_bits,FEI_OBJECT_EQUIV) ;
  
  if (p->form == is_ST) {
    Set_ST_sclass(st, ST_sclass(cast_to_ST(p->item)));

    if (!test_flag(flag_bits,FEI_OBJECT_IN_COMMON)&& (
           ST_sclass(cast_to_ST(p->item))==SCLASS_COMMON ||
           ST_sclass(cast_to_ST(p->item))==SCLASS_MODULE ))
         Set_ST_sclass(st,SCLASS_AUTO);  

    Set_ST_base(st, cast_to_ST(p->item));

// this above stmt set base_idx for variables in common block 


    /* adding rename of use'd varbl in later PU to */
    /* module data initalized earlier?             */

    if (ST_sclass(st) == SCLASS_DGLOBAL)
      Set_ST_is_initialized(st);
      
    if (eq) 
      Set_ST_is_equivalenced(st);
  }

  /* record the hosted object, so other routines use same ST */

  if (hosted) {
      cwh_stab_enter_hosted(st);

      if (IS_AUTO_OR_FORMAL(st))
	Set_ST_has_nested_ref(st);

  }

  /* Set function result flags - May be function result shared */
  /* between entry points, when it has an EQUIV base           */
  /* If there is an integer result the TY size is at least I8  */

  if ((sym_class == Function_Rslt) || 
      (hosted && test_flag(flag_bits,FEI_OBJECT_RESULT_TEMP))) {

    if (Has_Base_Block(st)) {

      TY_IDX temp_ty_idx = ST_type (ST_base(st));
      Set_TY_align (temp_ty_idx, 8);
      Set_ST_type (ST_base(st), temp_ty_idx);
      Set_ST_is_return_var(ST_base(st));
      cwh_stab_altres_offset(st,hosted);

    } else if (ST_sclass(st) != SCLASS_FORMAL_REF)
      Set_ST_is_return_var(st);
  }

  /* Non hosted, formal to be added to fn's list of dummies. */  
  /* Maybe was found in 'earlier hosted' list though if it   */
  /* was a Host dummy used within internal routine           */
  
  if (IS_FORMAL(st)) {

    if (! hosted )
       cwh_auxst_add_dummy(st,test_flag(flag_bits,FEI_OBJECT_RESULT_TEMP));
  } 

  /* Add COMMON or EQUIVALENCEd item to internal lists */

  if (Has_Base_Block(st)) {

    if (IS_COMMON(ST_base(st))) {
      if (sym_class != CRI_Pointee)
	cwh_auxst_add_item(ST_base(st),st,l_COMLIST) ;

    } else if (eq) {
      cwh_auxst_add_item(ST_base(st),st,l_EQVLIST);
    }
  }

  /* if a dope vector for a pointer, set flag & check type is f90 pointer */

  if (test_flag(flag_bits, FEI_OBJECT_DV_IS_PTR)) {
    Set_ST_auxst_is_f90_pointer(st, TRUE);
    tr_idx = Make_F90_Pointer_Type(ty);
    Set_TY_is_f90_pointer(tr_idx);
    Set_ST_type(st,tr_idx);
    Set_ST_is_my_pointer(st) ;

    if (ST_sclass(st) == SCLASS_FORMAL) {
       DevAssert(TY_is_f90_pointer(TY_pointed(ST_type(st))),(" missing pf90p"));
    } else {
       DevAssert(TY_is_f90_pointer(ST_type(st)),(" missing f90p"));
    }       
  }

  if (test_flag(flag_bits, FEI_OBJECT_ALLOCATE)) {
    Set_ST_auxst_is_allocatable(st, TRUE) ;
    Set_ST_is_allocatable(st) ; } 

  if (test_flag(flag_bits,FEI_OBJECT_RESULT_TEMP))
     Set_ST_is_return_var(st); 

  if (test_flag(flag_bits, FEI_OBJECT_ASSUMD_SHAPE)) {
    Set_ST_auxst_is_assumed_shape(st, TRUE) ;
    Set_TY_is_f90_assumed_shape(ST_type(st));  } 

  if (test_flag(flag_bits, FEI_OBJECT_DEFERRED_SHAPE)) 
    Set_TY_is_f90_deferred_shape(ST_type(st));  
   
  if (test_flag(flag_bits, FEI_OBJECT_IN_MODULE))
    Set_ST_is_in_module(st);   
  if (test_flag(flag_bits, FEI_OBJECT_EXTERNAL))
    Set_ST_is_external(st);  

  if (modst_idx) { /* this variable imported  by use stmt */
      modp  = cast_to_STB(modst_idx);
      Set_ST_base(st, cast_to_ST(modp->item));
      cwh_auxst_add_item(ST_base(st),st,l_TYMDLIST) ;
   }

  if (test_flag(flag_bits, FEI_OBJECT_ASSUMED_SIZE)) {
    Set_ST_auxst_is_assumed_size(st, TRUE) ;
    Set_TY_is_f90_assumed_size(ST_type(st)) ; } 
 
 
  if (test_flag(flag_bits, FEI_OBJECT_TARGET)) 
    Set_ST_is_f90_target(st) ;
  
  if (test_flag(flag_bits, FEI_OBJECT_ACTUAL_ARG))
    cwh_expr_set_flags(st,f_T_PASSED);
      
  /* for distributed arrays, put the ST into the   */
  /* distribute pragmas and write out the pragmas. */

  if (decl_distribute_pragmas) 
    cwh_stab_distrib_pragmas(st) ;

  if (!Has_Base_Block(st))
     DevAssert((ST_ofst(st) == 0),("Offset?"));

  o = cwh_stab_packet(st,is_ST);
  return(cast_to_int(o));
}

/*==================================================
 * Use stmt rename-only list
 *
 *=================================================
 */

void
fei_rename_list(char * name_string)
{
  ST *st;
  st = New_ST(CURRENT_SYMTAB);
    ST_Init(st,
          Save_Str(name_string),
          CLASS_NAME,
          SCLASS_UNKNOWN,
          EXPORT_LOCAL,
          (TY_IDX)0);
   cwh_stk_push(st,ST_item);

}

/*===================================================
 *
 * fei_seg
 *
 * Given a description of a storage block, look 
 * at the segment and generate an SCLASS to return. 
 * The SCLASS will be handed to fei_new_object and the like.
 *
 * If a COMMON name, make an ST for it and return 
 * that. Elements of the Common will be added to the 
 * Common's AUXST. FE_Partial_Split is default, if 
 * Full Split required it's done later.
 * 
 * If an equivalence base, then make the base ST here
 * as we lose the segment information. It's used 
 * when a based object appears in fei_object.
 *
 ====================================================
*/
/*ARGSUSED*/
INTPTR
fei_seg (char        * name_string,
	 INT32        Seg_type_arg,
	 INT32        owner,
	 INT32        parent,
	 INT32        aux_index,
	 INT32        flag_bits,
	 INT32        nest_level,
	 INT64        block_length )
{
  INT32 rt   ;
  ST   *st   ;
  ST  *st1;
  STB_pkt *p ;
  SEGMENT_TYPE seg_type;
  TY_IDX  ty;

  seg_type = (SEGMENT_TYPE) Seg_type_arg;

  if ((seg_type == Seg_Common ) ) {

    BOOL is_duplicate = test_flag(flag_bits,FEI_SEG_DUPLICATE);

     st = cwh_stab_common_ST(name_string, block_length,0);

     if (test_flag(flag_bits,FEI_SEG_THREADPRIVATE)) {
	Set_ST_is_thread_private(st);
        Set_ST_not_gprel(st);
     }

     if (test_flag(flag_bits,FEI_SEG_MODULE)) 
	Set_ST_auxst_is_module_data(st,TRUE);

     if (test_flag(flag_bits,FEI_SEG_EXTERNAL))
        Set_ST_is_external(st);   

      cwh_auxst_add_to_list(&Commons_Already_Seen,st,FALSE); 

      ty = ST_type(st);

      if (test_flag(flag_bits,FEI_SEG_VOLATILE))
       Set_TY_is_volatile(ty);

#if 0
    else {  /* found common from earlier PU. Check?/set flags */

      if (test_flag(flag_bits,FEI_SEG_THREADPRIVATE)) {
	Set_ST_is_thread_private(st);
        Set_ST_not_gprel(st);
      }
    }
#endif

    /* add to list of COMMONs requiring DST info */

    cwh_auxst_add_item(Procedure_ST,st,l_DST_COMLIST);

    p = cwh_stab_packet(st,is_ST);

    } else if (test_flag(flag_bits,FEI_SEG_EQUIVALENCED)) { 

    /* if saw hosted equiv from internal procedure, use that */
      st = cwh_stab_earlier_hosted(name_string);
      if (st == NULL) {
         SYMTAB_IDX level = CURRENT_SYMTAB;

         if (seg_type == Seg_Non_Local_Stack)
	    level = HOST_LEVEL ;

         st = New_ST(level);  
         cwh_auxst_clear(st);
         ST_Init(st, Save_Str(name_string), CLASS_VAR, SCLASS_AUTO, EXPORT_LOCAL,0);

         if (test_flag(flag_bits,FEI_SEG_MODULE)) //June
                 st1 = Scope_tab[CURRENT_SYMTAB].st;
         else st1 = st;

         Set_ST_base(st, st1);

         Set_ST_ofst(st, 0);

         if (test_flag(flag_bits,FEI_SEG_SAVED) || (seg_type == Seg_Static_Local)) 
	      Set_ST_sclass(st, SCLASS_PSTATIC);
         else
	      Set_ST_is_temp_var(st);
      
         if (seg_type == Seg_Non_Local_Stack) {
	   cwh_stab_enter_hosted(st);
	   Set_ST_has_nested_ref(st);
          } 

         Set_ST_type(st, cwh_types_mk_equiv_TY(block_length));

         if (test_flag(flag_bits,FEI_SEG_MODULE)){
              Set_ST_auxst_is_module_data(st,TRUE);
              Set_ST_is_in_module(st);
         }  

         if (test_flag(flag_bits,FEI_SEG_EXTERNAL))
             Set_ST_is_external(st);   
             cwh_stab_to_list_of_equivs(st,seg_type == Seg_Non_Local_Stack);
          }
         if (test_flag(flag_bits,FEI_SEG_EXTERNAL)){
             Set_ST_is_external(st);   
          }

         p = cwh_stab_packet(st,is_ST);

     } else {  /* get SCLASS */
       rt = cast_to_int(segment_map[seg_type]);
       p = cwh_stab_packet(cast_to_void(rt),is_SCLASS);
    }

    return (cast_to_int(p));
}


/*===================================================
 *
 * fei_name
 *
 * Introduces a new name, but often an alternative
 * for one we have seen already. So far, only dummies
 * in entry points that are the same name as a dummy
 * in the procedure header are of interest. They have 
 * not been through fei_object for this entry point
 * so didn't get stuck onto the dummy list...
 *
 * Lists of Namelist items are built up here, then
 * associated with a Namelist name in fei_namelist.
 *
 ====================================================
 */
/*ARGSUSED*/
INTPTR
fei_name (char *name_string,
	  INT32  st_grp,
	  INTPTR  st_idx,
	  INT32   prev_idx,
	  INT32   idx )
{
  ST * st;
  STB_pkt *p;  
  STB_pkt *r;

  r = NULL ;

  switch ((SYM_GROUP)st_grp) {
  case Sym_Namelist:

    if (prev_idx == 0) 
      Namelist = NULL;

    p = cast_to_STB(st_idx);
    DevAssert((p->form == is_ST),(" name item??")); 

    st = cast_to_ST(p->item);  
    (void) cwh_auxst_add_to_list(&Namelist,st,FALSE) ;
    r  = cwh_stab_packet(cast_to_void(Namelist),is_LIST);
    break ;

  case Sym_Object:

    if (st_idx != 0){

     if (entry_point_count > 1 ) {  

	p = cast_to_STB(st_idx);

	if (p->form == is_ST) {
	  st = cast_to_ST(p->item) ;

	  if (IS_FORMAL(st) ) {  
	    if (!cwh_auxst_find_dummy(st)) 
	      cwh_auxst_add_dummy(st,FALSE);
	  } 
	}
      } 
    } else {
       /* Just return a pointer to a duplicate of the name string */
       cwh_mkdepend_add_name(idx, name_string);
    }
    break;

   case Sym_Null:
      cwh_mkdepend_add_name(idx, name_string);
      break;

  default:
    break ;
  }
  return(cast_to_int(r));
}
/*===================================================
 *
 * fei_namelist
 *
 * Introduces a namelist name, and the associated
 * list of components (idx). Put them in the
 * symbol table.
 * 
 ====================================================
 */
/*ARGSUSED*/
INTPTR
fei_namelist(char  * name_string,
	     INT32   nitems,
	     INTPTR  idx,
             INT32   in_model )
{
  ST * st;
  TY_IDX  ty;
  STB_pkt *p;
  STB_pkt *l;
  WN * wn;
  WN * wn1;
  OPCODE  opc;
  WN * block;
  ITEM *element;
  int i = 0;
  
  ty = cwh_types_mk_namelist_TY(nitems);
 if (in_model){
  st = New_ST(GLOBAL_SYMTAB);  
  Set_ST_is_in_module(st);
   } 
 else 
  st = New_ST(CURRENT_SYMTAB);// here,if in module then should be 
                              //GLOBAL_SYMTAB
  cwh_auxst_clear(st);
  ST_Init(st, Save_Str(name_string), CLASS_VAR, SCLASS_AUTO, EXPORT_LOCAL, ty);
  Set_ST_ofst(st, 0);

  p = cwh_stab_packet(cast_to_void(st),is_ST) ;

  if (in_model >2)
    Set_ST_is_external(st);

  l = cast_to_STB(idx);
  DevAssert((l->form == is_LIST),("Nm list??"));
  cwh_auxst_add_list(st, (LIST *) l->item, l_NAMELIST);

  opc = OPCODE_make_op(OPR_NAMELIST,MTYPE_V,MTYPE_V);     
  wn  =  WN_Create(opc,nitems);
  WN_st_idx(wn) = ST_st_idx(st);
  element = NULL;
// add kids
    while ((element = cwh_auxst_next_element(
              st,element,l_NAMELIST)) != NULL ) {
      wn1  =  WN_Create(OPC_IDNAME,0);

      st = I_element(element);
      WN_st_idx(wn1) = ST_st_idx(st);
      WN_kid(wn,i) = wn1;
            i++;
//    printf("namelist %s\n",ST_name(st)); 
                   
                 }
  cwh_block_append_given_id(wn,First_Block,FALSE);
  
  return (cast_to_int(p));
}

/*===================================================
 *
 * fei_label
 *
 * Introduces a new label. Give it an ST and return.
 * Internal labels are named in the FFE, but the
 * name is ignored here.
 *
 * Symtab_last label is incremented for internal labels
 * in Gen_Label, but not for others. Numbers should be
 * unique for WN, so bump here.
 *
 ====================================================
 */
/*ARGSUSED*/
INT32
fei_label(char       *name_string,
	  INT32       flags,
	  INT32       Class,
	  char        *fmt_string,
	  INT32       debug)
{
  LABEL_IDX l_idx;
  
  switch ((LABEL_SYM)Class) {

  case 	PDGCS_Lbl_User :
  case	PDGCS_Lbl_Format:
    {
      LABEL& lbl = New_LABEL (CURRENT_SYMTAB, l_idx);
      LABEL_Init(lbl, Save_Str(name_string), LKIND_DEFAULT);
    }
    break ;
    
  case PDGCS_Lbl_Internal:
    {
      LABEL& int_lbl = New_LABEL (CURRENT_SYMTAB, l_idx);
      LABEL_Init(int_lbl, 0, LKIND_DEFAULT);
    }
    break ;
    
  default:
    DevAssert((0),(" Unexpected Label"));
    
  }  
  return(cast_to_int(l_idx));
}

/*===================================================
 *
 * cwh_stab_set_symtab
 *
 * Set the current SYMTAB correctly. If this is
 * an internal procedure, be may have been processing
 * Host dummies - see fei_proc_parent - and now
 * need to go back to the child.
 *
 * symtab == scope : there can be only 1 pair of
 * internal/external routines being processed at once.
 *                     
 ====================================================
*/
extern void
cwh_stab_set_symtab(ST *st)
{
  Current_scope = PU_lexical_level(st);
}

/*===================================================
 *
 * cwh_stab_const_ST
 *
 * Make(find) an ST from the INTCONST(CONST) in this WN.
 *
 ====================================================
*/
extern ST *
cwh_stab_const_ST(WN *wn)
{
  TCON    tcon;
  ST     *st  ; 

  if (WNOPR(wn) == OPR_CONST) 
    st = WN_st(wn);

  else if (WNOPR(wn) == OPR_INTCONST) {
    tcon = Host_To_Targ (WNRTY(wn),WN_const_val(wn));
    st = New_Const_Sym(Enter_tcon (tcon), Be_Type_Tbl(WNRTY(wn)));

  } else {
    DevAssert((0),("unexpected WN"));
  }
  return st;
}

/*===================================================
 *
 * cwh_stab_const
 *
 * Make a WN from the ST for this const,
 *
 ====================================================
*/
extern WN *
cwh_stab_const(ST *st)
{
  WN *wn  ;
  TYPE_ID bt;

  bt = TY_mtype(ST_type(st));
  wn = WN_CreateConst (Const_Opcode [bt],st);

  return(wn);
}

/*===================================================
 *
 * cwh_stab_address_temp_ST
 *
 * Make an ST for a local (AUTO) address temp. Sets
 * 
 * ST_is_temp_var       - avoids DST info.
 * 
 * If uniq is TRUE sets
 *
 * ST_pt_to_unique_mem  - not target of ptr.
 *
 ====================================================
*/
extern ST *
cwh_stab_address_temp_ST(char * name, TY_IDX  ty , BOOL uniq)
{
  ST * st ;

  st = New_ST(CURRENT_SYMTAB);
  cwh_auxst_clear(st);
  ST_Init (st, 
	   Save_Str(cwh_types_mk_anon_name(name)), 
	   CLASS_VAR, 
	   SCLASS_AUTO, 
	   EXPORT_LOCAL, 
	   ty);

  Set_ST_is_temp_var(st);

  if (uniq) 
   Set_ST_pt_to_unique_mem(st);

  cwh_expr_temp_set_pragma(st);
  return st ;
}

/*================================================================
 *
 * cwh_stab_temp_ST
 *
 * Makes an ST for a temp, marks it LOCAL if in PDO 
 *
 * ================================================================
 */
extern ST *
cwh_stab_temp_ST(TY_IDX ty,char * name)
{
  ST * st; 

  st = Gen_Temp_Symbol(ty,name);
  cwh_auxst_clear(st);
  cwh_expr_temp_set_pragma(st) ;

  return st;
}

/*===================================================
 *
 * cwh_stab_add_pragma
 *
 * Set given flag in ST's pragma. Only the 
 * ACCESSED_ID pragma for host variables referenced
 * within internal procedures are handled. If a NULL
 * was returned from the preamble routine, then we 
 * were probably in a declaration & there was no block
 * to add the pragma too. It'll be done when the code
 * is executed.
 *
 ====================================================
*/
extern void
cwh_stab_add_pragma(ST *st, WN_PRAGMA_ACCESSED_FLAGS flag )
{
  WN   * wn ;
  enum site block = block_ca ; 

  wn = cwh_auxst_pragma(st);

  if (wn == NULL) {

    wn = WN_CreatePragma (WN_PRAGMA_ACCESSED_ID,st,0,flag);

    if (cwh_stmt_add_to_preamble(wn, block)) 
      (void) cwh_auxst_pragma(st,wn);
    else
      WN_DELETE_Tree(wn);

  } else
    WN_pragma_arg2(wn) = WN_pragma_arg2(wn) | flag ; 
}

/*===================================================
 *
 * cwh_stab_packet
 *
 * Sometimes we return either a WN, an ST, or constant.
 * to the PDGCS layer. eg: for an array bound or character
 * len. To distinguish they are tagged. 
 *
 ====================================================
*/
extern STB_pkt * 
cwh_stab_packet(void * thing, enum is_form fm)
{
  STB_pkt *p ;

  p = cwh_stab_packet_typed(thing,fm, 0) ;
  return (p) ;
}

/*===================================================
 *
 * cwh_stab_packet_typed
 *
 * TYped version of above. In the case of
 * a logical constant, we have to type the WN to
 * distinguish it from an integer. Other instances
 * could use the mechanism, but don't. (no need);
 *
 ====================================================
*/
extern STB_pkt * 
cwh_stab_packet_typed(void * thing, enum is_form fm, TY_IDX  ty)
{
  STB_pkt *p ;

  p = (STB_pkt *) malloc(sizeof(STB_pkt)) ;
  
  p->item = thing ;
  p->form = fm    ;
  p->ty   = ty    ;
  p->next = STB_list;
  
  STB_list = p ;

  return (p) ;
}

/*===================================================
 *
 * cwh_stab_free_packet
 *
 * Free the STB packet list
 *
 ====================================================
*/
static void
cwh_stab_free_packet(void)
{

  STB_pkt *p ;
  STB_pkt *q ;

  p = STB_list ;

  while (p != NULL) {
    q = p->next ;
    free(p);
    p = q ;
  }

  STB_list = NULL ;

}
/*===============================================
 *
 * cwh_stab_end_procs
 *
 * Clean up at the end of a procedure. Get rid of
 * packets and auxst info created for this PU.
 * 
 * Set Scope to host, or global symtab. 
 * fei_next_func_idx will adjust to whatever's next.
 *
 *===============================================
 */ 
extern void 
cwh_stab_end_procs(void)
{
  cwh_stab_free_packet(); 
  cwh_auxst_free() ; 

  if (! IN_NESTED_PU) 
    Has_nested_proc = FALSE ;

  cwh_auxst_un_register_table() ;
  Delete_Scope(CURRENT_SYMTAB);

  Current_scope -= 1;
  cwh_auxst_clear_per_PU();
  entry_point_count = 0 ;
}

/*===============================================
 *
 * cwh_stab_earlier_hosted
 *
 * Internal procedures are entered before hosts,
 * so to reference the host variable within the
 * inner procedure, the 'internal' version was 
 * placed in the host symbol table. Now, processing
 * the host symbols, have been given an ST with a
 * inner def/ref, so want to find the one used earlier
 * and return that. It may be a reference to a host
 * symbol from another internal proc of course.
 *
 *===============================================
 */ 
static ST * 
cwh_stab_earlier_hosted(const char * name)
{
  ST * sl ;
  INT32 i ;

  for(i = 0 ; i <= Host_Top ; i ++) {
    sl = Host_STs[i];
    if (ST_class(sl) == CLASS_VAR) 
      if (strcmp(name,ST_name(sl)) == 0) 
	return (sl);
  }
  return (NULL);
}

/*===============================================
 *
 * cwh_stab_enter_hosted
 *
 * Save this ST on the list of hosted varbls
 * while processing the inner procedure. When 
 * processing the host then this ST is the one
 * to look for & use.
 *
 *===============================================
 */ 
static void
cwh_stab_enter_hosted(ST * st)
{
  Host_Top ++ ;

  if (Host_Top >= Host_Current_Size) {
     Host_Current_Size += HOST_ST_SIZE_CHANGE;
     Host_STs = (ST **) realloc(Host_STs,sizeof(ST *)*Host_Current_Size);
  }

  Host_STs[Host_Top] = st;
}

/*===============================================
 *
 * cwh_stab_adjust_name
 *
 * Internal and module procedures are named
 * <proc>.in.<host>, but for DST information the
 * additional information should be stripped off
 * and the stem used. DW_AT_linkage strings 
 * preserve the original, so the linker can find it.
 *
 * The MAIN program is an exception - we want MAIN,
 * as the ST for ld to resolve the executable from 
 * main_/crt0 but require the program name as a 
 * debuggable name.
 * 
 * clovis@par.univie.ac.at -> removed the name change
 * in main program
 *
 * This builds the stem, and tacks it into the 
 * ST's AUXST. 
 *
 *===============================================
 */ 
static void
cwh_stab_adjust_name(ST * st)
{
  char *p;
  char *s;
  char  c;
  INT32 n;

  s = ST_name(st);

  PU& pu = Pu_Table[ST_pu(st)];
  if (PU_is_mainpu(pu)) {

    //Set_ST_name(st, Save_Str(def_main_u));

    //if (!strcmp(crayf90_def_main,s)) 
    //  s = def_main ;

    n = strlen(s);
    p = (char *) malloc(n+1);
    (void) cwh_auxst_stem_name(st,strcpy(p,s));
    p[n-1] = '\0';

  } else {

    c = '.' ;
    p = strchr(s,c);
    
    if (p != NULL) {

      n = p-s+1;
      p = (char *) malloc(n);
      p = strncpy(p,s,n-1);
      p[n-1] = '\0';

      cwh_auxst_stem_name(st,p);
    }
  }
}

/*===============================================
 *
 * cwh_stab_adjust_base_name.
 *
 * The FE gives temps names t$<n>. To make w2f
 * output and IR a little more intelligible, the
 * name of a base (address) temp is altered to be
 * p_<object>. 
 *
 * For a hosted ST, this must happen only in the
 * host. eg: several internal procedures may use 
 * the same t$3 from the host, so match them all,
 * then alter the ST.
 *
 *===============================================
 */ 
static void
cwh_stab_adjust_base_name(ST * st)
{

  if (Has_Base_Block(st)) {
    ST * base = ST_base(st);
    if (ST_is_temp_var(base))
      if (ST_sclass(base) == SCLASS_AUTO)
	if (!ST_is_return_var(base))
	  if (!ST_has_nested_ref(st) || 
	      (ST_has_nested_ref(st) && CURRENT_SYMTAB == HOST_LEVEL))
	    Set_ST_name(base,Save_Str2("p_",ST_name(st)));      
  }
}

/*===============================================
 *
 * cwh_stab_main_ST
 *
 * Returns the ST * of an CLASS_EXTERNAL ST used
 * to put out DST info for named programs.
 *
 *===============================================
 */ 
extern ST *
cwh_stab_main_ST(void)
{
  return Main_ST;
}

/*===============================================
 *
 * cwh_stab_set_linenum
 *
 * Set the line number where the ST was declared
 * in the AUXST
 * 
 *===============================================
 */ 
extern void
cwh_stab_set_linenum(ST *st, INT32 lineno)
{
  USRCPOS *pos;
  char *file_name;
  static char *last_file_name = NULL;
  static INT32 last_file_num  = 0 ;
  INT32 local_line_num;
  
  pos = cwh_auxst_srcpos_addr(st);
  file_name = global_to_local_file(lineno);
  local_line_num = global_to_local_line_number(lineno);
  if (last_file_name != file_name) 
    last_file_num = cwh_dst_enter_path(file_name); 

  USRCPOS_filenum(*pos) = last_file_num ;
  USRCPOS_linenum(*pos) = local_line_num;

  last_file_name = file_name ;
}

/*===============================================
 *
 * cwh_stab_formal_ref
 *
 * Given an ST of SCLASS FORMAL, decide if
 * it should be a SCLASS_FORMAL_REF.
 * ie: it's scalar and by address.
 *
 *===============================================
 */ 
static void
cwh_stab_formal_ref(ST * st, BOOL host) 
{

  TY_IDX ty ;

  if (!ST_is_value_parm(st)) {

    ty = ST_type(st);

    if (TY_kind(ty) == KIND_SCALAR || TY_kind(ty) == KIND_STRUCT)
       Set_ST_sclass(st, SCLASS_FORMAL_REF);
    else
       Set_ST_type(st, cwh_types_mk_pointer_TY(ty, host));
  }
}

/*===============================================
 *
 * cwh_stab_full_split
 *
 * Given an ST of a common block, with
 * elements of the COMMON ordered by offset within
 * the AUXST, split the common fully.
 * 
 * This is lifted from mfef77 as the split should
 * be consistent with f77 .o files which contain 
 * similar common definitions.
 *
 *===============================================
 */ 
static void
cwh_stab_full_split(ST *c, enum list_name list)
{
  ITEM  * el;
  INT32   nf;
  INT32   i;
  LIST   *l;
  FIELDS fp_table ;

  l  = cwh_auxst_get_list(c,l_COMLIST);
  if ( l == NULL)
    return;

  nf = L_num(l);
  if (nf == 0)
    return ;

  if (ST_is_initialized(c) || TY_is_volatile(ST_type(c))) {
    cwh_stab_mk_flds(c,list);
    return ;
  }

  fp_table = (FIELDS) malloc ( sizeof(FIELD_ITEM) * nf) ;

  i  = 0 ;
  el = NULL ;

  while ((el = cwh_auxst_next_element(c,el,list)) != NULL ) {

    ST *st = I_element(el);
    FIELDS_fp(i) = st;
    FIELDS_first_offset(i) = ST_ofst(st);
    FIELDS_last_offset(i)  = ST_ofst(st) + TY_size(ST_type(st)) - 1;
    i ++ ;
  }

  DevAssert((i==nf),(" cant count"));

//  cwh_stab_dump_FIELDS(fp_table,0,nf-1);

  cwh_stab_find_overlaps(fp_table,nf);

  /* if the COMMON was split, issue the elements of each partition
   * then the list of partitions which make up the COMMON. If not
   * split just issue all the elements of the COMMON.
   */

  if (cwh_stab_split_common(c,fp_table,nf)) {

    el = NULL ;
    while ((el = cwh_auxst_next_element(c,el,l_SPLITLIST)) != NULL ) {

      cwh_stab_mk_flds(I_element(el),l_COMLIST);
    }

    cwh_stab_mk_flds(c,l_SPLITLIST);

    L_num(l)   = 0 ;
    L_first(l) = NULL ;
    L_last(l)  = NULL ;

  } else 
    cwh_stab_mk_flds(c,list);

  free(fp_table);

}

/*===============================================
 *
 * cwh_stab_find_overlaps
 *
 * Utility function for Full_Split_Common.
 * Given an array of FIELDS ordered by first
 * offset, find any overlaps cause by equivalence,
 * and make all corresponding first & last offsets
 * reflect the size of the equivalence block
 *
 *===============================================
 */ 
static void
cwh_stab_find_overlaps(FIELDS fp_table, INT32 nf)
{
  INT32 i,j,first;
  INT64 last_offset;
  INT64 first_offset;
  
  first = 0;
  first_offset = FIELDS_first_offset(0);
  last_offset  = FIELDS_last_offset(0);
  
  for ( i = 1; i < nf; i++ ) {

    if ( FIELDS_first_offset(i) > last_offset ) {

      for ( j = first; j < i; j++ ) {

	FIELDS_first_offset(j) = first_offset;   
	FIELDS_last_offset(j)  = last_offset;
      }

      first = i;
      first_offset = FIELDS_first_offset(i);
      last_offset  = FIELDS_last_offset(i);
      
    } else if ( FIELDS_last_offset(i) > last_offset )
      last_offset = FIELDS_last_offset(i);
  }
  
  for ( j = first; j < i; j++ ) {

    FIELDS_first_offset(j) = first_offset;
    FIELDS_last_offset(j)  = last_offset;
  }
}


/*===============================================
 *
 * cwh_stab_split_common
 *
 * Utility function for Full_Split_Common.
 * Given an array of FIELDS ordered by first
 * offset, and separated into non-overalapping
 * groups, split the common. 
 *
 * All fields within an equivalence group have 
 * the same first_offset and the last_offset
 * & hence extent of group.
 *
 * If the common was split return TRUE.
 *
 *===============================================
 */ 
static BOOL
cwh_stab_split_common(ST * c, FIELDS fp_table, INT32 nf)
{
  ST     * e  ;
  ST     * nc ;
  TY_IDX ty ;
  TY_IDX tc ;

  INT32  i,j,k ;
  INT32  first ;
  INT32  full_split_last_array = -1;
  INT64  first_offset;
  INT64  last_offset;
  BOOL   seen_a_split = FALSE ;


  tc = ST_type(c);
  first = 0;
  first_offset = FIELDS_first_offset(0);
  last_offset  = FIELDS_last_offset(0);
  full_split_last_array = -1;


  for ( i = 1; i < nf; i++ ) {

    if ( FIELDS_last_offset(i) > last_offset ) {

      e  = FIELDS_fp(i);
      ty = ST_type(e);

      if ((TY_kind(ty) == KIND_ARRAY) &&
	  (FIELDS_first_offset(i) % TY_align(tc) == 0)) {

	if ( TY_size(ty) >= FE_Full_Split_Array_Limit ) {

	  BOOL split = FALSE;

	  for ( j = 0; j < FE_Full_Split_Limits_Count; j++ ) {

	    if (   FIELDS_first_offset(i) - first_offset
		<   FE_Full_Split_Limits [j].rel_offset
		- FE_Full_Split_Limits [j].delta )
	      break;

	    if ( need_to_split ( FIELDS_first_offset(i),
				first_offset,
				FE_Full_Split_Limits [j].rel_offset,
				FE_Full_Split_Limits [j].delta ) ) {
 	      split = TRUE;
 	      seen_a_split = TRUE;
	      break;
	    }

	    for (k  = full_split_last_array;
		 k >= 0;
		 k  = FIELDS_prev_array_index(k) ) {

	      if ( need_to_split (FIELDS_first_offset(i),
				  FIELDS_first_offset(k),
				  FE_Full_Split_Limits [j].rel_offset,
				  FE_Full_Split_Limits [j].delta ) ) {
		split = TRUE;
		seen_a_split = TRUE;
		break;
	      }
	    } 
	    if ( split )
	      break;
	  }

	  if ( split ) {

	    nc = cwh_stab_split_ST(c,
				   FIELDS_first_offset(first),
	                           FIELDS_last_offset(i-1)); 
	    cwh_stab_emit_split(nc,fp_table,first, i-1);
	    cwh_auxst_add_item(c,nc, l_SPLITLIST);
	    if (ST_is_thread_private(c)) Set_ST_is_thread_private(nc);
	    first = i;
	    first_offset = FIELDS_first_offset(i);
	    full_split_last_array = -1;
	  }

	  FIELDS_prev_array_index(i) = full_split_last_array;
	  full_split_last_array = i;
	}
      }
      last_offset  = FIELDS_last_offset(i);
    }
  }

  if (seen_a_split) {
    nc = cwh_stab_split_ST(c,
			   FIELDS_first_offset(first),
			   FIELDS_last_offset(i-1)); 
    cwh_stab_emit_split(nc,fp_table,first, i-1);
    cwh_auxst_add_item(c,nc, l_SPLITLIST);
  }

  return seen_a_split ;
}

/*===============================================
 *
 * need_to_split
 *
 * Utility function for cwh_stab_split_common
 * Given an current position and  offset decide
 * if the block has to be split.
 *
 *===============================================
 */ 
static BOOL
need_to_split (INT64 cur_offset,
	       INT64 base_offset, 
	       INT64 rel_offset,
	       int    delta )
{
  BOOL    split;
  INT64   offset;

  offset = cur_offset - base_offset;
  offset = offset % rel_offset;

  split  = ( offset < delta ) || ( offset > ( rel_offset - delta ) );

  return split;
}

/*===============================================
 *
 * cwh_stab_dump_FIELDS
 *
 * Dumps n items of a FIELDS array. the indexes
 * are inclusive.
 *
 *===============================================
 */ 
static void
cwh_stab_dump_FIELDS(FIELDS fp_table, INT32 from, INT32 to)
{
  ST    *st;
  INT32  i ;

  for ( i = from; i <= to; i++ ) {

    st = FIELDS_fp(i);

    printf (" %d - ",i);

    printf (" f_off: %16llx, l_off: %16llx, prev %4d,",
	    FIELDS_first_offset(i),
	    FIELDS_last_offset(i),
	    FIELDS_prev_array_index(i));
    if (st)
      printf (" ST: %x (%s)\n",st,ST_name(st));
    else
      printf (" ST: <none>\n");

  }
}

/*===============================================
 *
 * cwh_stab_emit_split
 *
 * Emits a split COMMON ST, given a fields table,
 * and the first & last (inclusive) FIELDS of the split.
 * 
 * Each element of the common has its base and offset
 * adjusted to a slot in the new common. The common
 * is ordered by offset in the FIELDS.
 *
 *===============================================
 */ 
static void
cwh_stab_emit_split(ST * c, FIELDS fp_table, INT32 from, INT32 to)
{

  INT32 i  ;
  ST  * e  ;
  INT64 off;

  off = FIELDS_first_offset(from);

  for (i = from ; i <= to; i ++) {
    e = FIELDS_fp(i);
    Set_ST_ofst(e, (ST_ofst(e) - off));
    Set_ST_base(e, c);
    cwh_auxst_add_item(c,e,l_COMLIST) ;
  }
}

/*===============================================
 *
 * cwh_stab_split_ST
 *
 * Create a new ST for the part of the common
 * that has been split. The name is derived
 * from the name of the original common and
 * the 'offset' of the first field in the 
 * split section. The name should match mfef77's.
 *
 *===============================================
 */ 
static ST *
cwh_stab_split_ST(ST * c, INT64 low_off, INT64 high_off)
{
  INT32 l  ;
  INT64 off;
  char *name;
  ST * st;

  l = strlen(ST_name(c));

  name = (char *) malloc(l + 128);

  name[0] = '_';  
  name[1] = '_';

  (void) strcpy(&name[2],ST_name(c));
  sprintf(&name[l+2], ".%lld", low_off );

  off = high_off-low_off+1;
  st  = cwh_stab_common_ST(name,byte_to_bit(off),TY_align(ST_type(c)));

  Set_ST_ofst(st, 0);
  Set_ST_base(st, c);

  Set_ST_is_split_common(st) ;

  if (ST_is_thread_private(c)) 
    Set_ST_is_thread_private(st);

  Set_TY_split(Ty_Table[ST_type(st)]);

  free (name);
  return st ;
}

/*===============================================
 *
 * cwh_stab_common_ST
 *
 * Create a new ST for a common, given a name,
 * a size & alignment (or 0).
 *
 *===============================================
 */ 
static ST *
cwh_stab_common_ST(char *name,INT64 size, mUINT16 al)
{

  ST * st ;
  ST * st1;
  SYMTAB_IDX s=CURRENT_SYMTAB;
  st1 = Scope_tab[s].st;

   st = New_ST(GLOBAL_SYMTAB);
  cwh_auxst_clear(st);
  ST_Init(st, Save_Str(name), CLASS_VAR, SCLASS_COMMON, EXPORT_PREEMPTIBLE,
	  cwh_types_mk_common_TY(size,al));

  Set_ST_base(st, st1);
  Set_ST_ofst(st, 0);

  if (CURRENT_SYMTAB != GLOBAL_SYMTAB) {
     cwh_stab_pu_has_globals = TRUE;
   ;
  }

  return st;
}
/************************************************
 *
 * cwh_stab_module_ST
 *
 * Follow cwh_stab_common_ST 
 * Only difference is the type change from
 * SCLASS_COMMON to SCLASS_MODULE
*************************************************/

static ST *
cwh_stab_module_ST(char *name,INT64 size, mUINT16 al)
{

  ST * st ;

  st = New_ST(GLOBAL_SYMTAB);
  cwh_auxst_clear(st);
  ST_Init(st, Save_Str(name), CLASS_VAR, SCLASS_MODULE, EXPORT_PREEMPTIBLE,
          cwh_types_mk_module_TY(size,al));

  Set_ST_base(st, st);
  Set_ST_ofst(st, 0);

  if (CURRENT_SYMTAB != GLOBAL_SYMTAB) {
     cwh_stab_pu_has_globals = TRUE;
       ;
  }

  return st;
}



/*===============================================
 *
 * cwh_stab_altres_offset
 *
 * Given an ST which represents part of a
 * result variable for an alternate entry
 * point, figure out what the ST_ofst
 * should be. 
 *
 * The offsets may have to be revamped, if as 
 * the full size of the equivalence class isn't
 * known until all return temps are processed.
 *
 * characters and arrays don't get here, becuase
 * they are passed by address, so there isn't a
 * shared variable.
 *
 *===============================================
 */ 
static void
cwh_stab_altres_offset(ST *st, BOOL hosted)
{
  ITEM * et;
  
  BOOL change  ;
  BOOL same    ;
  BOOL allF4C4 ;
  BOOL isF8    ;
  BOOL isC4    ;
  TY_IDX ty    ;

  if (ST_has_nested_ref(st) && ! hosted)
    return;

  ty = ST_type(st);

  if (TY_kind(ty) == KIND_STRUCT) /* struct < 16B? */
    return ;

  DevAssert((TY_kind(ty) == KIND_SCALAR),("Only scalars"));

  /* was a base introduced because CQ entry appeared first? */
  /* if so, make all bases consistent - use CQ one          */

  if (Altbase_ST == NULL) 
    Altbase_ST = ST_base(st); 
  else if (Altbase_ST != ST_base(st)) 
    Set_ST_base(st, Altbase_ST);


  /* are all entry points same TY? or all C4s and F4s? */

  allF4C4 = (TY_mtype(ty) == MTYPE_C4) || 
            (TY_mtype(ty) == MTYPE_F4) ;

  isF8    = (TY_mtype(ty) == MTYPE_F8);
  isC4    = (TY_mtype(ty) == MTYPE_C4);


  /* look through the list of return types & decide if they */
  /* are all the same, or consistent in an interesting way  */ 

  et = NULL;
  same = TRUE ;

  while ((et = cwh_auxst_next_element(ST_base(st),et,l_RETURN_TEMPS)) != NULL ) {

    TY_IDX tyi = ST_type(I_element(et));

    allF4C4 = allF4C4 &&
      ((TY_mtype(tyi) == MTYPE_C4) || 
       (TY_mtype(tyi) == MTYPE_F4)) ;

    isF8    = isF8 ||
      (TY_mtype(tyi) == MTYPE_F8) ;

    isC4    = isC4 ||
      (TY_mtype(tyi) == MTYPE_C4) ;

    same = same && (ty == tyi);
  }

  Set_ST_auxst_altentry_shareTY(ST_base(st),same);


  /* ints always I8, C4s require 16 bytes, if not all C4 or C4 & F4 */

  change = FALSE ;
  
  TYPE_ID  bt = TY_mtype(ty);
  TY_IDX   tb = ST_type(ST_base(st));
  TY&       t = Ty_Table[tb];

  if (MTYPE_is_integral(bt)) {
    if (TY_size(tb) < TY_size(Be_Type_Tbl(MTYPE_I8))) {

      Set_TY_size(t, TY_size(Be_Type_Tbl(MTYPE_I8)));
      change = TRUE;
    }

  } else if (!same) {
    if (!allF4C4) {
      if (isC4 && isF8) {
	if (TY_size(tb) < TY_size(Be_Type_Tbl(MTYPE_C8))) {

	  Set_TY_size(t, TY_size(Be_Type_Tbl(MTYPE_C8)));
	  change = TRUE;
	}
      }
    }
  }

  /* is equiv size, enough (FE has different understanding) */

  if (TY_size(tb) <= TY_size(ty)) {

    Set_TY_size(t, TY_size(ty));
    change = TRUE;
  }

  cwh_stab_altres_offset_comp(st,allF4C4);
  cwh_auxst_add_item(ST_base(st),st,l_RETURN_TEMPS);

  /* equivalence TY changed? recompute offsets of previous items */

  if (change) {

    et = NULL ;
    while ((et = cwh_auxst_next_element(ST_base(st),et,l_RETURN_TEMPS)) != NULL ) {
      cwh_stab_altres_offset_comp(I_element(et),allF4C4);
    }
  }
}

/*===============================================
 *
 * cwh_stab_altres_offset_comp
 *
 * Utility for cwh_stab_altres_offset
 * 
 * sets the offset for the given ST.
 * 
 * The flag says all altreturn values are
 * either F4, or C4s
 *
 *===============================================
 */ 
static void
cwh_stab_altres_offset_comp(ST *st, BOOL allF4C4)
{
  TY_IDX  ty;
  TY_IDX  tb;
  TYPE_ID bt ;

  ty = ST_type(st);
  bt = TY_mtype(ty);
  tb = ST_type(ST_base(st));

  if (MTYPE_is_complex(bt)) {
    if (bt == MTYPE_C4) 
      if (TY_size(tb) > 8)
	Set_ST_ofst(st, 8);

  } else if (MTYPE_is_float(bt)) {
    if (bt == MTYPE_F4) 
      if (TY_size(tb) > 4 && !allF4C4)
	Set_ST_ofst(st, 4);

  } else 
    Set_ST_ofst(st, TY_size(Be_Type_Tbl(MTYPE_I8)) - TY_size(ty));
}

/*===============================================
 *
 * cwh_stab_altentry_TY
 *
 * Given a ST, find the size of altentry_temp
 * associated with its TY. The types of a result 
 * variable reflects the register used for the results
 * in a composite TY:
 * 
 * integers, logicals : I8
 * floats:  F8
 * complex: C8
 *
 * if all entries have the same result type, then the
 * result varbl is of that type, except integers are
 * always I8. 
 *
 * In an expression however, for floats we need to 
 * store the same type as the ST really is, so
 * the 'expr' flag controls this.
 *
 *===============================================
 */ 
extern TY_IDX 
cwh_stab_altentry_TY(ST *st, BOOL expr)
{
  TY_IDX tr;
  TY_IDX ty;
  TY_IDX base;

  TYPE_ID max;
  TYPE_ID bt ;

  ty = ST_type(st);

  DevAssert((TY_kind(ty) == KIND_SCALAR),("Only scalars"));

  base = ST_type(ST_base(st));
  bt   = TY_mtype(ty);
  max  = bt ;

  if (MTYPE_is_complex(bt)) {
    if (!expr) {
      if (TY_size(base) == 8)
	max = MTYPE_C4;
      else
	max = MTYPE_FQ;
    } 

  } else if (MTYPE_is_float(bt)) {
      if (TY_size(base) == 4)
	max = MTYPE_F4;
      else if (TY_size(base) == 8) {
	max = MTYPE_F8;
	if (ST_ofst(st) == 0 && bt == MTYPE_F4)
	  max = MTYPE_C4;
      } else
	max = MTYPE_FQ;
  } else 
    max = MTYPE_I8;

  tr = Be_Type_Tbl(max);

  return tr;
}

/*===============================================
 *
 * cwh_stab_distrib_pragmas
 *
 * if an ST in fei_object is the subject of
 * distribute directives the ST may have to 
 * be tacked on to the list of declaration pragmas.
 *
 * Set_ST_is_reshaped if a distribute_reshape 
 *    
 *===============================================
 */
static void
cwh_stab_distrib_pragmas(ST *st)
{
  TY_IDX  ty;
  WN_ITER  *stmt_iter;
  WN *stmt, *wn;
  PREG_det preg;

  ty = ST_type(st);

  if (ST_sclass(st) == SCLASS_FORMAL)
    ty = TY_pointed(ty);

  DevAssert((TY_kind(ty)==KIND_ARRAY),("distribute of non-array"));

  stmt_iter = WN_WALK_StmtIter(decl_distribute_pragmas);
  while(stmt_iter != NULL) { 
    stmt_iter = WN_WALK_StmtNext(stmt_iter);
    if (stmt_iter) {
      stmt= WN_ITER_wn(stmt_iter);
      if (stmt!=NULL) {
	switch(WN_opcode(stmt)) {
	case OPC_XPRAGMA:
	case OPC_PRAGMA:
	  WN_st_idx(stmt) = ST_st_idx(st);
	  if (WN_pragma(stmt)==WN_PRAGMA_DISTRIBUTE_RESHAPE)
	    Set_ST_is_reshaped(st);
	  break;
	default:
	  DevAssert((0),("unexpected distribute pragma"));
	}
      }
    }
  }

  /* attach the pragmas to the decl statement list */

  cwh_block_append_given_id(decl_distribute_pragmas,First_Block,FALSE);
  decl_distribute_pragmas = NULL;

  /* associate a PREG with the distributed array and write to it */

  preg = cwh_auxst_distr_preg(st);
  wn = cwh_load_distribute_temp();
  wn = WN_CreateStid( OPC_I4STID, preg.preg, preg.preg_st, preg.preg_ty, wn);
  cwh_block_append_given_id(wn,First_Block,FALSE);

  /* create another write to the global preg for all distributed arrays */

  if (preg_for_distribute.preg==-1) {
    preg_for_distribute=cwh_preg_next_preg(MTYPE_I4, NULL, NULL);
  }
  wn = cwh_load_distribute_temp();
  wn = WN_CreateStid( OPC_I4STID, preg_for_distribute.preg,
		     preg_for_distribute.preg_st, preg_for_distribute.preg_ty, wn);
  cwh_block_append_given_id(wn,First_Block,FALSE);

  /* set the needs LNO bits */

  Set_PU_mp_needs_lno (Get_Current_PU ());
  Set_FILE_INFO_needs_lno (File_info);
}
/*===================================================
 *
 * cwh_load_distribute_temp
 *
 * Creates a LDID of the temp allocated to store to
 * the PREGs associated with the distributed arrays
 *
 ====================================================
*/
extern WN *
cwh_load_distribute_temp(void)
{
  TY_IDX ty;
  WN *rtrn;

  ty = Be_Type_Tbl(MTYPE_I4);

  if (st_for_distribute_temp == NULL) {
    st_for_distribute_temp = Gen_Temp_Symbol(ty,TY_name(ty));
    cwh_auxst_clear(st_for_distribute_temp);
  }
  rtrn = WN_CreateLdid(OPC_I4I4LDID, 0, st_for_distribute_temp, ty);
  return rtrn;
} 


/*===============================================
 *
 * cwh_stab_altentry_temp
 *
 * Found a CQ entry point, without having seen
 * an ST for the shared result temp. Make the
 * shared result_temp's base, if it doesn't exist
 * and the temp itself. Don't enter the temp, but
 * leave it up to fei_object.
 *
 *===============================================
 */
static ST *
cwh_stab_altentry_temp(char * name, BOOL hosted)
{
  ST * st;
  TY_IDX  ty;

  TYPE t ;
  INT32 size ;

  size = byte_to_bit(TY_size(Be_Type_Tbl(MTYPE_CQ)));

  if (Altbase_ST == NULL) {

    ty = cwh_types_mk_equiv_TY(size);
    st = cwh_stab_address_temp_ST(".cq_base.",ty , FALSE);
    Set_ST_base(st, st);
    cwh_stab_to_list_of_equivs(st, hosted) ;
    Altbase_ST  = st;
  }

  t  = fei_descriptor(0,Basic,size,C_omplex,0,0);
  st = New_ST(CURRENT_SYMTAB);
  cwh_auxst_clear(st);
  ST_Init (st, Save_Str(name), CLASS_VAR, SCLASS_AUTO, EXPORT_LOCAL, cast_to_TY(t_TY(t)));
  Set_ST_base(st, Altbase_ST);
  Set_ST_ofst(st, 0);

  return st;
}

/*===============================================
 *
 * cwh_stab_to_list_equivs
 *
 * Add this equivalence group base to a list 
 * of equivalences for this PU.
 *
 *===============================================
 */
static void
cwh_stab_to_list_of_equivs(ST *st, BOOL hosted)
{
  LIST ** l = &Equivalences ;

  if (hosted) 
    l = &Hosted_Equivalences ;

  cwh_auxst_add_to_list(l,st,FALSE);
}

/*===================================================
 *
 * cwh_stab_set_tylist_for_entries
 *
 * Create tylists for the procedure and all its
 * entry points.
 *
 ====================================================
*/
void
cwh_stab_set_tylist_for_entries(ST *proc)
{

 ITEM *en = NULL;

 cwh_auxst_set_tylist(proc);
 while ((en = cwh_auxst_next_element(proc,en,l_ALTENTRY)) != NULL) {
    cwh_auxst_set_tylist(I_element(en));
 }

}


/*===================================================
 *
 * cwh_stab_emit_commons_and_equivalences
 *
 * Make fld's for the members of all the common blocks
 * equivalences and entry points in this PU. Couldn't
 * do it earlier, because didn't know sizes of blocks
 * to emit.
 *
 ====================================================
*/
extern void
cwh_stab_emit_commons_and_equivalences(SYMTAB_IDX level)
{

  void (*fp) (ST *, enum list_name) = &cwh_stab_mk_flds;

  if (FE_Full_Split)
    fp = &cwh_stab_full_split ;

  if (level == GLOBAL_SYMTAB) 
    cwh_stab_emit_list(&Commons_Already_Seen,l_COMLIST,fp);

  else {
    
    cwh_stab_emit_list(&Equivalences,l_EQVLIST,&cwh_stab_mk_flds);

  /* Emit any equivalence blocks for alternate entry points */
  /* or equivalence'd host variables                        */
  /* Entry points can't appear in internal routines, and    */
  /* internal routines see just host results they reference */

    if (level == HOST_LEVEL)
      cwh_stab_emit_list(&Hosted_Equivalences,l_EQVLIST,&cwh_stab_mk_flds);

  }
}

/*===================================================
 *
 * cwh_stab_emit_list
 *
 * walk over one of the lists of STs that want FLDS
 * generating from item (fld ST) associated with each
 * eg: a common and its elements.
 *
 ====================================================
*/
static void
cwh_stab_emit_list(LIST ** lp, enum list_name list, void (*fp) (ST *, enum list_name))
{
  ITEM * i;

  if (*lp != NULL ) {
    i = L_first(*lp);

    while (i != NULL) {
      fp (I_element(i),list) ;
      i = I_next(i);
    }

    cwh_auxst_free_list(lp);
  }
}

/*===================================================
 *
 * cwh_stab_mk_flds
 *
 * Make fld's for all the members of common or
 * equivalence block passed in. 
 *
 ====================================================
*/
static void
cwh_stab_mk_flds(ST * blk, enum list_name list)
{
  ITEM * el;
  INT32   nf;
  INT32   i;
  LIST   *l;

  l  = cwh_auxst_get_list(blk, list);
  if (l == NULL)
    return ;

  nf = L_num(l);

  if (nf == 0)
    return ;

  //  cwh_stab_dump_list(l,FALSE); 

  i  = 0 ;
  el = NULL ;

  while ((el = cwh_auxst_next_element(blk,el,list)) != NULL ) {
    cwh_types_mk_element(blk,I_element(el));
    i ++ ;
  }

  DevAssert((i == nf), (" can't count"));
}

/*===================================================
 *
 * cwh_stab_earlier_common
 * 
 * Has this common been seen already? If so, use the
 * old ST. Module data can always share the same 
 * COMMON st, as the definition is consistent between
 * PUs. For user commons the is_duplicate flag is set
 * by the FE if name and types match. Equivalences
 * cause the flag to be false.
 *
 ====================================================
*/ 
static ST*
cwh_stab_earlier_common(char *name_string, BOOL is_duplicate)
{
  ITEM * i;

  if (Commons_Already_Seen!= NULL ) {
    i = L_first(Commons_Already_Seen);

    while (i != NULL) {
      ST *st = I_element(i) ;
      if (ST_auxst_is_module_data(st) || is_duplicate)
	if (strcmp(ST_name(st),name_string) == 0) {
	  return st ;
	}
      i = I_next(i);
    }
  }

  return NULL;
}

/*===============================================
 *
 * cwh_stab_seen_common_element
 *
 * Is this item an element of a Common that
 * we've already seen? If it's equivalenced
 * at the same offset, lookup on name too.
 * (there may be more than 1...)
 *
 * TODO make efficient...
 *
 *===============================================
 */
static ST *
cwh_stab_seen_common_element(ST *c, INT64 offset, char* name)
{
  ITEM * el = NULL;
  ST *   st ;

  while ((el = cwh_auxst_next_element(c,el,l_COMLIST)) != NULL ) {
    st = I_element(el);
    if (ST_ofst(st) == offset)
      if (strcmp(ST_name(st),name) == 0) 
	return st ;

  }
  return NULL ;
}

/*===================================================
 *===================================================*/
ST *
cwh_stab_seen_derived_type_or_imported_var(ST *c, char* name)
 {
  ITEM * el = NULL;
  ST *   st ;

  while ((el = cwh_auxst_next_element(c,el,l_TYMDLIST)) != NULL ) {
    st = I_element(el);
    if (ST_pu(c) == ST_pu(ST_base(st)))
      if (strcmp(ST_name(st),name) == 0)
        return st ;
  }
  return NULL ;
 }
/*===================================================
 *
 * cwh_stab_mk_fn_0args
 * 
 * create a new extern function, with 0 args.
 * This does not assign a scope array so if any
 * tables are needed, fei_proc_def or fei_proc_parent
 * will need to associate the ST with a scope
 *
 ====================================================
*/
extern ST *
cwh_stab_mk_fn_0args(char *name, ST_EXPORT eclass,SYMTAB_IDX level,TY_IDX rty)
{
  ST    * st ;
  PU_IDX  pu ;
  TY_IDX  ty ;

  ty = cwh_types_mk_procedure_TY(rty,					
				 0,
				 TRUE,
				 FALSE);

  pu = cwh_stab_mk_pu(ty, level);
  st = New_ST(GLOBAL_SYMTAB);
  cwh_auxst_clear(st);
  Set_PU_need_unparsed(pu);

  ST_Init (st, 
	   Save_Str(name), 
	   CLASS_FUNC, 
	   SCLASS_EXTERN,
	   eclass,
	   (TY_IDX)pu);

  Set_ST_ofst(st, 0);
  return(st);
}

/*===================================================
 *
 * cwh_stab_mk_pu
 * 
 * create a new PU for the given procedure TY at
 * level L.
 *
 ====================================================
*/
static PU_IDX
cwh_stab_mk_pu(TY_IDX pty, SYMTAB_IDX level)
{
  PU_IDX pu_idx;
  PU&    pu = New_PU (pu_idx); 

  PU_Init(pu, pty, level);   

  return (pu_idx);
}

/*===================================================
 *
 * fei_smt_parameter
 *
 * If debug symbol tables are being built, this sends
 * information for adding parameters (named constants)
 * to the DST.  It adds the name and line number for 
 * a specific named constant.  The constant was sent 
 * earlier with fei_arith_con or fei_pattern_con.
 *
 ====================================================
*/

INTPTR
fei_smt_parameter(char * name_string,
   	   	  TYPE   type,
                  INTPTR con_idx,
		  INT32	 Class,
		  INT32	 lineno)

{
   INT32 len;
   char * name;
   char * name1;
   STB_pkt *p;
   ST *  st;
   TY_IDX ty;
   WN *  wn;


   ty = cast_to_TY(t_TY(type));

   if (TY_is_character(ty)) { /* Character */
     st = cast_to_ST(con_idx);
   }
   else {
     p = cast_to_STB(con_idx);

     if (p->form == is_ST) {
       st = cast_to_ST(p->item);
     }
     else if (p->form == is_WN) {
        wn = cast_to_WN(p->item);
        st = cwh_stab_const_ST(wn);
      }
   }

   /* Store the name in the auxiliary name table for the symbol. */

   /* WN's share const entries, but the same constant value may have */
   /* multiple names, so the names are concatenated with blank       */
   /* separation and held in stem name until cwh_dst_process_var is  */
   /* called.  Then they are separated and an entry is made for each */
   /* parameter in the DST.                                          */

   name = NULL;
   name = cwh_auxst_stem_name(st, name);

   if (name == NULL) {  /* this is the first name for this ST */
      len = strlen(name_string);
      name1 = (char *) malloc(len+1);
      strcpy(name1, name_string);
      cwh_auxst_stem_name(st, name1);
      cwh_auxst_add_item(Procedure_ST,st,l_DST_PARMLIST);
   }
   else {
      len = strlen(name_string);
      len += strlen(name);
      ++len;
      name1 = (char *) malloc(len+1);
      strcpy(name1, name_string);
      strcat(name1, " ");
      strcat(name1, name);
      free(name);
      cwh_auxst_stem_name(st, name1);
   }
//made a local symtab entry for parameter ---fzhao
   ST * parast = New_ST(CURRENT_SYMTAB);
   ST_Init(parast,
	      Save_Str(name_string), 
        	CLASS_PARAMETER,
        	SCLASS_UNKNOWN,
        	EXPORT_LOCAL,
        	ty);
   Set_ST_base(parast,st);
   Set_ST_sclass(parast,ST_sclass(st));
 
   cwh_stab_set_linenum(st,lineno);

   return(cast_to_int(st));
}

/*===================================================
 *
 * fei_interface
 *
 * Introduces an interface block, and the associated
 * list of components (pu's). Put it in the
 * First_Block.
 *
 ====================================================
 */
/*ARGSUSED*/
INTPTR
fei_interface(char  * name_string,
             INT32   nitems,
             INT32   kind_interface,
             INT32 is_imported)
{
  ST * st;
  TY_IDX  ty;
  STB_pkt *p;
  WN * wn;
  WN * wn1;
  OPCODE  opc;
  WN * block;
  int i = 0;
  int k;


  st = New_ST(CURRENT_SYMTAB);
                             
  cwh_auxst_clear(st);

  ty = 0;

  ST_Init(st, Save_Str(name_string), CLASS_VAR, SCLASS_AUTO, EXPORT_LOCAL, ty);
  Set_ST_ofst(st, 0);

  if (is_imported)
     Set_ST_is_external(st);

  if (kind_interface == 1)
    Set_ST_is_assign_interface(st);
  else if (kind_interface == 2)
         Set_ST_is_operator_interface(st);
       else if (kind_interface == 3)
             Set_ST_is_u_operator_interface(st);

  p = cwh_stab_packet(cast_to_void(st),is_ST) ;


  opc = OPCODE_make_op(OPR_INTERFACE,MTYPE_V,MTYPE_V);
  wn  =  WN_Create(opc,nitems);
  WN_st_idx(wn) = ST_st_idx(st);

 if (nitems !=0)
  for (k = nitems -1 ; k >= 0  ; k --) {
     wn1 = cwh_stk_pop_WN();
     WN_kid(wn,k) = wn1;
  }

  cwh_block_append_given_id(wn,First_Block,FALSE);

  return (cast_to_int(p));
}


void fei_set_in_interface_processing()
  {
     interface_pu = 1;
  }


void fei_reset_in_interface_processing()
  {
    interface_pu = 0;
  }
