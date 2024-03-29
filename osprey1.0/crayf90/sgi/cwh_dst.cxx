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
 * Module: cwh_dst.c
 * $Revision: 1.5 $
 * $Date: 2007-01-08 21:48:42 $
 * $Author: fzhao $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/crayf90/sgi/cwh_dst.cxx,v $
 *
 * Revision history:
 *  dd-mmm-95 - Original Version
 *
 * Description: Create and put out the DST information. An attempt is
 *              made to use only ST information (& files), but there
 *              are inquiries to other cwh* routines for dope vectors, 
 *              commmon elements and line numbers(?). These are indicated
 *              by macros in cwh_dst.i. The support is enough for f90,
 *              no effort was made for C or C++.
 * 
 *              The entry points are
 *                1) cwh_dst_init_file - initialize file level stuff.
 *                2) cwh_dst_write - write file's worth of DST to IRB file.
 *                3) cwh_dst_enter_pu - build the DST info for a PU.
 *                4) cwh_dst_enter_path - enter path of  source file.
 *
 * ====================================================================
 * ====================================================================
 */

static char *source_file = __FILE__;
#ifdef _KEEP_RCS_ID
static char *rcs_id = "$Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/crayf90/sgi/cwh_dst.cxx,v $ $Revision: 1.5 $";
#endif /* _KEEP_RCS_ID */

/* sgi includes */

#include <limits.h>
#include <sys/stat.h>  
#include <unistd.h>    

#include "x_stdio.h" // for fileno()
#include "x_string.h" // for strdup()

#include "defs.h"
#include "glob.h"
#include "errors.h"
#include "wn.h"
#include "dwarf_DST_producer.h"
#include "dwarf_DST_dump.h"
#include "config_targ.h"
#include "file_util.h"

/* conversion includes */

#include "cwh_defines.h"
#include "cwh_dst.h"
#include "cwh_dst.i"
#include "cwh_preg.h"
#include "cwh_stab.h"
#include "cwh_auxst.h"
#include "cwh_types.h"
#include "sgi_cmd_line.h"

char *FE_command_line = NULL;

/*===================================================
 *
 * cwh_dst_init_file
 *
 * DST initialization for a file. Enter the path
 * as the first file, and the filename as the
 * compile_name.
 *
 * set the cwd as part of the include paths.
 *
 ====================================================
*/
extern void
cwh_dst_init_file(char *src_path)
{
  char         *comp_info = NULL;
  char         *file ;

  DST_Init(NULL,0) ;

  file = strrchr(src_path,'/');

  comp_info = cwh_dst_get_command_line_options(); 


  comp_unit_idx = DST_mk_compile_unit(++(file),
				      current_host_dir,
				      comp_info,
				      DW_LANG_Fortran90,
				      DW_ID_case_insensitive);
  (void) cwh_dst_enter_path(src_path);
  free (comp_info);
}

/*===================================================
 *
 * cwh_dst_write
 *
 * Write out DST information for a file. Each DST
 * fe_ptr has to be changed to a back end ST index
 * via DST_set_assoc_idx.
 *
 ====================================================
*/
extern void
cwh_dst_write(void)
{

   file_name_idx = DST_write_files();
   incl_dir_idx  = DST_write_directories();

   if (!DST_IS_NULL(comp_unit_idx))
     (void) DST_preorder_visit(comp_unit_idx, 0, &DST_set_assoc_idx); 

   if (DSTdump_File_Name != NULL) {

      DST_set_dump_filename(DSTdump_File_Name);
      DST_dump(incl_dir_idx, file_name_idx, comp_unit_idx);
   }

}

static void 
cwh_dst_process_var (UINT32, ST* st)
{
   switch(ST_class(st)) {

   case CLASS_VAR:
     cwh_dst_mk_var(st,current_scope_idx);
     break;

   case CLASS_CONST:
     cwh_dst_mk_const(st,current_scope_idx);
     break;
   }
}

/*===================================================
 *
 * cwh_dst_mk_const
 *
 * Write out DST information for a constant.
 *
 ====================================================
*/
static void 
cwh_dst_mk_const(ST * st,DST_INFO_IDX  parent)
{
   DST_CONST_VALUE	cval;
   USRCPOS		s;
   int			exit	= 0;
   DST_INFO_IDX		i,t ;
   char			*ptr;
   TY_IDX		ty;
   TYPE_ID		type ;
   char		       *name;
   char			*str;


   /* DST_mk_constant_def(USRCPOS   decl,         Source location   */
   /*               char           *name,         Name of constant  */
   /*               DST_INFO_IDX    type,         Type of constant  */
   /*               DST_CONST_VALUE cval,         Value of constant */
   /*               BOOL            is_external)  External?         */

   s = GET_ST_LINENUM(st);

   ty = ST_type(st);
   type = TY_mtype(ty);
   t = cwh_dst_mk_type(ty);

   switch(TY_mtype(ty)) {

   case MTYPE_I1:
   case MTYPE_U1:
      DST_CONST_VALUE_form(cval) = DST_FORM_DATA1;
      DST_CONST_VALUE_form_data1(cval) = TCON_i0(Tcon_Table[ST_tcon(st)]);
      break;

   case MTYPE_I2:
   case MTYPE_U2:
      DST_CONST_VALUE_form(cval) = DST_FORM_DATA2;
      DST_CONST_VALUE_form_data2(cval) = TCON_i0(Tcon_Table[ST_tcon(st)]);
      break;

   case MTYPE_I4:
   case MTYPE_U4:
      DST_CONST_VALUE_form(cval) = DST_FORM_DATA4;
      DST_CONST_VALUE_form_data4(cval) = TCON_i0(Tcon_Table[ST_tcon(st)]);
      break;

   case MTYPE_I8:
   case MTYPE_U8:
      DST_CONST_VALUE_form(cval) = DST_FORM_DATA8;
      DST_CONST_VALUE_form_data8(cval) = TCON_i0(Tcon_Table[ST_tcon(st)]);
      break;

   case MTYPE_F4:
      DST_CONST_VALUE_form(cval) = DST_FORM_DATA4;
      DST_CONST_VALUE_form_data4(cval) = TCON_ival(Tcon_Table[ST_tcon(st)]);
      break;

   case MTYPE_F8:
      DST_CONST_VALUE_form(cval) = DST_FORM_DATA8;
      DST_CONST_VALUE_form_data8(cval) = TCON_i0(Tcon_Table[ST_tcon(st)]);
      break;

   case MTYPE_F16:
   case MTYPE_FQ:
      exit = 1;
      break;

   case MTYPE_STR:   /* Not generally used. */
      exit = 1;
      break;

# if 0
      DST_CONST_VALUE_form(cval) = DST_FORM_STRING;
      DST_CONST_VALUE_form_string(cval) =
           DST_mk_string (Index_to_char_array (TCON_str_idx (ST_tcon_val(st))));
# endif

   case MTYPE_UNKNOWN: 

      if (TY_kind(ty) == KIND_ARRAY && TY_is_character(ty)) {
         DST_CONST_VALUE_form(cval) = DST_FORM_STRING;
         DST_CONST_VALUE_form_string(cval) =
           DST_mk_string (Index_to_char_array (TCON_str_idx (ST_tcon_val(st))));
      }
      break; 
   }

   if (exit == 1) return;  /* Type is not implemented. */

   name = NULL;
   name = cwh_auxst_stem_name(st, name);
      
   ptr = strtok(name, " ");

   while (ptr != NULL) {
      i = DST_mk_constant_def(s,
                              ptr,
                              t,
                              cval,
                              FALSE);
      DST_append_child(current_scope_idx,i);
      ptr = strtok(NULL, " ");
   }
   return;
}

/*===================================================
 *
 * cwh_dst_enter_pu
 *
 * Enter DST information for this PU. The entry point
 * is required in the PU_info, and alternate entry points
 * are always generated, but the local symbols
 * are only generated with -g.
 *
 ====================================================
*/
extern DST_IDX
cwh_dst_enter_pu(ST *en)
{

  ITEM * al;
  ITEM * com;
  ITEM * parm;
  ST * st;
  DST_INFO_IDX i;
  PU& pu = Pu_Table[ST_pu(en)];


  DST_begin_PU();
  cwh_dst_struct_clear_DSTs(); 

  current_scope_idx = cwh_dst_mk_func(en);

  if (PU_is_mainpu(pu)) 
    cwh_dst_mk_MAIN(GET_MAIN_ST(),current_scope_idx);

  /* nested? so is dwarf. Save idx until parent appears */

  if (PU_is_nested_func(pu))
    cwh_dst_inner_add_DST(current_scope_idx);

  else {

    cwh_dst_inner_read_DSTs(current_scope_idx);
    cwh_dst_inner_clear_DSTs();
    DST_append_child(comp_unit_idx,current_scope_idx);
  }

  al = NULL ;
  while ((al = GET_NEXT_ALTENTRY(en,al)) != NULL) {
    i = cwh_dst_mk_func(I_element(al));
    DST_append_child(comp_unit_idx,i);
  }


  if (Debug_Level > 0) {

    /* Set up integer DSTs so MP lowerer can make DSTs for  */
    /* loop varbls and so forth (wn_mp.c Add_DST_variable)  */

    (void) cwh_dst_basetype(Be_Type_Tbl(MTYPE_I4));
    (void) cwh_dst_basetype(Be_Type_Tbl(MTYPE_I8));

    For_all (St_Table, CURRENT_SYMTAB, &cwh_dst_process_var);

    /* look for commons or module data referenced within this */
    /* PU, but promoted to the global symbol table            */

    ITEM * com = NULL;
    ITEM * parm = NULL;
    
    if (PU_lexical_level(pu) == 2)
      while ((com = GET_NEXT_COMMON(en,com)) != NULL) 
	cwh_dst_mk_var(I_element(com),current_scope_idx);

    if (PU_lexical_level(pu) == 2)
      while ((parm = GET_NEXT_PARAMETER(en,parm)) != NULL) 
	cwh_dst_process_var(1, I_element(parm));

  }

  DST_end_PU();

  return(current_scope_idx);
}

/*===================================================
 *
 * cwh_dst_mk_func
 *
 * Enter DST information for a CLASS_FUNC symbol,
 * ie: a procedure entry point.
 *
 * If it was an internal or module procedure the
 * name might not be the same as seen by the linker.
 *
 * Strip off any trailing underscores..
 *
 *===================================================
*/
static DST_IDX
cwh_dst_mk_func(ST * st)
{

  DST_INFO_IDX t;
  DST_INFO_IDX i;

  USRCPOS s;
  char *p ;
  char *r ;
  char *l ;
  INT32 n ;
  TY_IDX ty;
  PU& pu = Pu_Table[ST_pu(st)];

  s = GET_ST_LINENUM(st);

  l = NULL;  
  p = GET_MODIFIED_NAME(st);
  if (p != NULL)
    r = p ;

  else {
    r = ST_name(st);
    n = strlen(r);

    if (r[n-1] == '_') {
      l = ux_strdup(r);
      l[n-1] = '\0';
      r = l ;
    }
  }

  ty = PU_prototype(Pu_Table[ST_pu(st)]);
  t  = cwh_dst_mk_subroutine_type(ty);
  
  if (IS_ALTENTRY(st)) 
    i = DST_mk_entry_point(s,r,t,(void *)ST_st_idx(st));

  else {
    i = DST_mk_subprogram(s,
			  r,
			  t,
			  DST_INVALID_IDX,
			  (void*)ST_st_idx(st),
			  DW_INL_not_inlined,
			  DW_VIRTUALITY_none,
			  0,	       
			  FALSE, 
			  FALSE, 
                          FALSE,
			  TRUE); 

    if (p != NULL && !PU_is_mainpu(pu)) 
      DST_add_linkage_name_to_subprogram(i,ST_name(st));
  }
    
  if (l != NULL)
    free(l);

  return i;
}

/*===================================================
 *
 * cwh_dst_mk_MAIN
 *
 * If this is a named program a DST for MAIN_ is
 * also required, so the debugger can find which
 * file contains the program stmt. If anonymous,
 * then a DST entry for MAIN has already been created
 * and mn == NULL. It's distinct from mk_func_entry
 * because the arguments to mk_subprogram are for
 * a declaration & weak symbol
 *
 *===================================================
*/
static void
cwh_dst_mk_MAIN(ST *mn, DST_INFO_IDX en_idx)
{
  DST_INFO_IDX t;
  DST_INFO_IDX i;
  USRCPOS s;
  TY_IDX ty;

  if (mn != NULL) {

    s  = GET_ST_LINENUM(mn);
    ty = PU_prototype(Pu_Table[ST_pu(mn)]);
    t  = cwh_dst_mk_subroutine_type(ty);
    i  = DST_mk_subprogram(s,
			   ST_name(mn),
			   t,
			   en_idx,
			   (void*) ST_st_idx(mn),
			   DW_INL_not_inlined,
			   DW_VIRTUALITY_none,
			   0,	       
			   TRUE,
                           FALSE,
			   FALSE, 
			   TRUE); 

    DST_append_child(comp_unit_idx,i);
  }
}

/*===================================================
 *
 * cwh_dst_mk_var
 *
 * Enter DST information for a CLASS_VAR symbol,
 * ie: a variable. Variables in COMMON are ignored here,
 * but processed when the COMMON symbol is seen. Ditto
 * temps when required as bounds, etc. We assume a ST 
 * name beginning with '@' is not required (it's a base) 
 * as it can't be printed in the debugger, but module
 * dats is a COMMON starting with @..
 *
 * The DST is appended to the parent.
 *
 *===================================================
*/
static void
cwh_dst_mk_var(ST * st,DST_INFO_IDX  parent)
{

  DST_INFO_IDX i ; 
  DST_INFO_IDX j ;

  Top_ST = st ;
  Making_FLD_DST = FALSE;

  switch(ST_sclass(st)) {

  case SCLASS_FORMAL:
  case SCLASS_FORMAL_REF:
    if (!ST_is_temp_var(st)) {
      Top_ST_has_dope = cwh_dst_has_dope(ST_type(st));
      i = cwh_dst_mk_formal(st) ;
      DST_append_child(parent,i);
    }
    break;

  case SCLASS_COMMON:
  case SCLASS_DGLOBAL:
    i = cwh_dst_mk_common(st);
    if (!DST_IS_NULL(i)) {
      j = cwh_dst_mk_common_inclusion(st,i);

      DST_append_child(parent,j);
      DST_append_child(parent,i);
    }
    break;

  default:
    if (Has_Base_Block(st)) {
      if ((ST_sclass(ST_base(st)) != SCLASS_COMMON) && 
  	(ST_sclass(ST_base(st)) != SCLASS_DGLOBAL)) {
        Top_ST_has_dope = cwh_dst_has_dope(ST_type(st));
        i = cwh_dst_mk_variable(st);
        DST_append_child(parent,i);
      }
    } else if  (!ST_is_temp_var(st)) {
      if (* ST_name(st) != '@') {  
	Top_ST_has_dope = cwh_dst_has_dope(ST_type(st));
	i = cwh_dst_mk_variable(st);
	DST_append_child(parent,i);
      }
    }
    break;
  }
}

/*===================================================
 *
 * cwh_dst_mk_variable
 *
 * Enter DST information for an ST of CLASS_VAR and
 * and any of the static/auto sclasses. For BASED
 * variables, the ST of the base provides the ST 
 * which later generates location information, but
 * cgdwarf.c figures this out. COMMON block elements
 * aren't processed here.
 * 
 *===================================================
*/
static DST_INFO_IDX
cwh_dst_mk_variable(ST * st)
{
  TY_IDX         d;
  DST_VARIABLE	*def_attr;
  DST_ATTR_IDX	 def_attr_idx;
  DST_INFO	*def_info;
  BOOL           dr;
  DST_INFO_IDX   dope_ty;
  DST_INFO_IDX   i;
  USRCPOS        s;
  DST_INFO_IDX   t;
  

  s  = GET_ST_LINENUM(st);
  d  = ST_type(st) ;

  dr = (Has_Base_Block(st)) && ST_auxst_is_auto_or_cpointer(st) ;

  if (IS_DOPE_TY(d)) {
     t  = cwh_dst_dope_type(ST_type(st),
                            st,
                            ST_ofst(st),
                            current_scope_idx,
                            FALSE,
                            &dope_ty);
     dr = TRUE ;
  } else 
    t = cwh_dst_mk_type(d);

  i = DST_mk_variable(s,
		      ST_name(st),
		      t,
		      0,
		      (void *) ST_st_idx(st),
		      DST_INVALID_IDX,
		      FALSE,
		      ST_sclass(st) == SCLASS_AUTO,
		      FALSE, 
		      ST_auxst_is_tmp(st));

  if (ST_auxst_is_assumed_size(st)) {
     DST_SET_assumed_size(DST_INFO_flag(DST_INFO_IDX_TO_PTR(i)));
  }

  if (IS_DOPE_TY(d)) {
     def_info     = DST_INFO_IDX_TO_PTR(i);
     def_attr_idx = DST_INFO_attributes(def_info);
     def_attr     = DST_ATTR_IDX_TO_PTR(def_attr_idx, DST_VARIABLE);

     DST_VARIABLE_def_dopetype(def_attr) = dope_ty;

     if (ST_auxst_is_assumed_shape(st)) {
        DST_SET_assumed_shape(DST_INFO_flag(def_info));
     }
     else if (ST_auxst_is_allocatable(st)) {
        DST_SET_allocatable(DST_INFO_flag(def_info));
     }
     else if (ST_auxst_is_f90_pointer(st)) {
        DST_SET_f90_pointer(DST_INFO_flag(def_info));
     }
  }

  if (dr)
    DST_SET_deref(DST_INFO_flag(DST_INFO_IDX_TO_PTR(i)));

  return i ;

}
/*===================================================
 *
 * cwh_dst_mk_formal
 *
 * Enter DST information for an ST of CLASS_VAR and
 * SCLASS_FORMAL. Assumes a formal parameter which
 * has a pointer TY is a fortran parameter by reference.
 *
 *===================================================
*/
static DST_INFO_IDX
cwh_dst_mk_formal(ST * st)
{
  ST_IDX         ba;
  DST_FORMAL_PARAMETER	*def_attr;
  DST_ATTR_IDX	 def_attr_idx;
  DST_INFO	*def_info;
  DST_INFO_IDX   dope_ty;
  BOOL           dr ;
  DST_INFO_IDX   t;
  TY_IDX         ta;
  TY_IDX         ty;

  BOOL           c_pointee = FALSE;
  BOOL           generated = FALSE ;
  DST_INFO_IDX   i = DST_INVALID_IDX ;

  USRCPOS s;


  s = GET_ST_LINENUM(st);

  ty = ST_type(st);
  ta = ty ;
  dr = FALSE ;
  ba = ST_st_idx(st) ;

  /* cray pointee? */

  if (Has_Base_Block(st)) {
    ba = ST_st_idx(ST_base(st)) ;
    c_pointee = TRUE;
  }

  /* If FORMAL is result temp address, it's by_value */
  /* if array/struct temp, it's by ref               */
  /* other by-values are scalars eg: char len temps  */

  if (ST_sclass(st) == SCLASS_FORMAL)
    if (!ST_is_value_parm(st))
      ta = TY_pointed(ty);
    else if (TY_kind(ty) == KIND_POINTER)
      ta = TY_pointed(ty);


  if (IS_DOPE_TY(ta)) {
     t  = cwh_dst_dope_type(ST_type(st),
                            st,
                            ST_ofst(st),
                            current_scope_idx,
                            FALSE,
                            &dope_ty);
     dr = TRUE ;
  } else 
    t = cwh_dst_mk_type(ta);

  i = DST_mk_formal_parameter(s,
			      ST_name(st),
			      t,
			      (void *) ba,
			      DST_INVALID_IDX,
			      DST_INVALID_IDX,
			      FALSE, /* FIX optional */
			      FALSE,
			      generated,
			      FALSE);          /* is_declaration_only */



  if (IS_DOPE_TY(ta)) {
     def_info     = DST_INFO_IDX_TO_PTR(i);
     def_attr_idx = DST_INFO_attributes(def_info);
     def_attr     = DST_ATTR_IDX_TO_PTR(def_attr_idx, DST_FORMAL_PARAMETER);

     DST_FORMAL_PARAMETER_dopetype(def_attr) = dope_ty;

     if (ST_auxst_is_assumed_shape(st)) {
        DST_SET_assumed_shape(DST_INFO_flag(def_info));
     }
     else if (ST_auxst_is_f90_pointer(st)) {
        DST_SET_f90_pointer(DST_INFO_flag(def_info));
     }
  }

  if (ST_auxst_is_assumed_size(st)) {
     DST_SET_assumed_size(DST_INFO_flag(DST_INFO_IDX_TO_PTR(i)));
  }

  if ( dr ||
      (TY_kind(ty) == KIND_POINTER) ||
      (ST_sclass(st) == SCLASS_FORMAL_REF))
    DST_SET_deref(DST_INFO_flag( DST_INFO_IDX_TO_PTR(i)));

  if (dr || c_pointee)
    DST_SET_base_deref(DST_INFO_flag( DST_INFO_IDX_TO_PTR(i)));

  return i;
}

/*===================================================
 *
 * cwh_dst_mk_common_inclusion
 *
 * Adds a common_inclusion to the DST. Sets up
 * the line number of the common.
 *
 ====================================================
*/
static DST_INFO_IDX
cwh_dst_mk_common_inclusion(ST * com, DST_INFO_IDX c)
{
  DST_INFO_IDX i;

  USRCPOS s;

  s = GET_ST_LINENUM(com);

  i = DST_mk_common_incl(s,c);

  return i;
}

/*===================================================
 *
 * cwh_dst_mk_common
 *
 * Enter DST information for a COMMON block and
 * all of its members. It assumes the COMMON TY
 * is a struct, and its members are all on the TY.
 *
 *===================================================
*/
static DST_INFO_IDX
cwh_dst_mk_common(ST * st)
{
  BOOL           dr;
  DST_VARIABLE	*def_attr;
  DST_ATTR_IDX	 def_attr_idx;
  DST_INFO	*def_info;
  DST_INFO_IDX   dope_ty;
  ITEM          *e;
  ST            *el;
  DST_INFO_IDX   i;
  DST_INFO_IDX   m;
  DST_INFO_IDX   t;
  USRCPOS        s;
  TY_IDX         te;
  TY_IDX         ty;


  ty = ST_type(st);

  DevAssert((TY_kind(ty) == KIND_STRUCT),("DST complains about common"));

  i = DST_mk_common_block(ST_name(st),(void*) ST_st_idx(st)); 
   
  e = NULL ;

  while ((e = GET_NEXT_ELEMENT_ST(st,e)) != NULL) {

    el = I_element(e);
    s  = GET_ST_LINENUM(st);
    te = ST_type(el);

    Top_ST = el;
    Top_ST_has_dope = cwh_dst_has_dope(te);

    dr = IS_DOPE_TY(te);

    if (dr) {
      t  = cwh_dst_dope_type(ST_type(el),
                             el,
                             ST_ofst(el),
                             i,
                             FALSE,
                             &dope_ty);
    } else
      t = cwh_dst_mk_type(te);

    m = DST_mk_variable_comm(s,
			     ST_name(el),
			     t,
			     (void *) ST_st_idx(st),
			     ST_ofst(el)) ;

    if (dr) {
       def_info     = DST_INFO_IDX_TO_PTR(m);
       def_attr_idx = DST_INFO_attributes(def_info);
       def_attr     = DST_ATTR_IDX_TO_PTR(def_attr_idx, DST_VARIABLE);

       DST_VARIABLE_comm_dopetype(def_attr) = dope_ty;

       if (ST_auxst_is_assumed_shape(el)) {
          DST_SET_assumed_shape(DST_INFO_flag(def_info));
       }
       else if (ST_auxst_is_allocatable(el)) {
          DST_SET_allocatable(DST_INFO_flag(def_info));
       }
       else if (ST_auxst_is_f90_pointer(el)) {
          DST_SET_f90_pointer(DST_INFO_flag(def_info));
       }
    }

    if (ST_auxst_is_assumed_size(el)) {
       DST_SET_assumed_size(DST_INFO_flag(DST_INFO_IDX_TO_PTR(m)));
    }

    if (dr)
      DST_SET_deref(DST_INFO_flag( DST_INFO_IDX_TO_PTR(m)));

    DST_append_child(i,m);
  }			

  return i;
}

/*===================================================
 *
 * cwh_dst_mk_type
 *
 * Make or find the DST info of this TY. 
 *
 *===================================================
*/
static DST_INFO_IDX 
cwh_dst_mk_type(TY_IDX  ty)
{
  DST_INFO_IDX i;

  switch (TY_kind(ty)) {
  case KIND_VOID:
    i = DST_INVALID_IDX;
    break;

  case KIND_SCALAR:
    i = cwh_dst_basetype(ty);
    break ;

  case KIND_ARRAY:
    i = cwh_dst_array_type(ty);
    break ;

  case KIND_STRUCT:
      i = cwh_dst_struct_type(ty);
    break;

  case KIND_POINTER:
    i = cwh_dst_pointer_type(ty);
    break;

  case KIND_FUNCTION:
    i = cwh_dst_mk_subroutine_type(ty);
    break ;
    
  default:
    DevAssert((0),("DST TY"));
  }

  return i;
}

/*===================================================
 *
 * cwh_dst_basetype
 *
 * Given a SCALAR ty, returns the corresponding DST
 * basetype for its typeid. Appends it to compilation 
 * unit to avoid duplication.
 *
 *===================================================
*/
static DST_INFO_IDX
cwh_dst_basetype(TY_IDX ty)
{
  TYPE_ID bt ;
  DST_INFO_IDX i ;

  bt = TY_mtype(ty);

  if (bt == MTYPE_V) return(DST_INVALID_IDX);

  if (TY_is_logical(Ty_Table[ty]))
    bt = bt -MTYPE_I1 + MTYPE_V + 1 ;

  if (!DST_IS_NULL(base_types[bt]))
    return base_types[bt];

  i = DST_mk_basetype(ate_types[bt].name,
		      ate_types[bt].encoding, 
		      ate_types[bt].size);

  base_types[bt] = i;
  DST_append_child(comp_unit_idx,i);
  return i;
}

/*===================================================
 *
 * cwh_dst_pointer_type
 *
 * Given a pointer TY, return an IDX.
 * Appends it to the current scope.
 *
 *===================================================
*/
static DST_INFO_IDX
cwh_dst_pointer_type(TY_IDX ty)
{
  DST_INFO_IDX i;
  DST_INFO_IDX t;

  t = cwh_dst_mk_type(TY_pointed(ty));
  i = DST_mk_pointer_type(t,
			  DW_ADDR_none,
			  TY_size(ty));

  DST_append_child(current_scope_idx,i);
  return i ;

}

/*===================================================
 *
 * cwh_dst_mk_subroutine_type
 *
 * Make the type DST info for a subroutine. 
 *
 *===================================================
*/
static DST_INFO_IDX 
cwh_dst_mk_subroutine_type(TY_IDX  ty)
{
  DST_INFO_IDX t ;

/* TEMPORARY TO DO FIX */  
  t = cwh_dst_basetype(Be_Type_Tbl(MTYPE_V));
/*   t = cwh_dst_mk_type(TY_ret_type(ty)); TODO fix with scope */

#if 0
  if (!DST_IS_NULL(t)) {

    USRCPOS_clear(s);
    
    i = DST_mk_subroutine_type(s,
			       NULL,
			       t,
			       DST_INVALID_IDX,
			       FALSE);

  }
  DST_append_child(current_scope_idx,i);
  return i;
#endif
  return t ;
}

/*===================================================
 *
 * cwh_dst_array_type
 *
 * Given a ARRAY ty, returns a DST_IDX for the
 * TY. Appends it to the current scope.
 *
 *===================================================
*/
static DST_INFO_IDX
cwh_dst_array_type(TY_IDX ty)
{

  DST_INFO_IDX i ;
  DST_INFO_IDX t ;
  DST_INFO_IDX d ;

  USRCPOS s;
  INT32 j;
  INT idx;

  USRCPOS_clear(s);

  if (cwh_dst_is_character_TY(ty)) {
    i = cwh_dst_substring_type(ty);

  } else {

    t = cwh_dst_mk_type(TY_AR_etype(ty));
    i = DST_mk_array_type(s, 
			  TY_name(ty),
			  t,
			  0,
			  DST_INVALID_IDX,
			  TRUE);
    
    TY& tt = Ty_Table[ty];
    ARB_HANDLE arb = TY_arb(ty);
    for (idx =  TY_AR_ndims(ty) - 1; idx >=0 ; idx--) {
      d = cwh_dst_subrange(arb[idx]) ;
      DST_append_child(i,d);
    }
  }
 DST_append_child(current_scope_idx,i);
  return i;
}

/*===================================================
 *
 * cwh_dst_struct_type
 *
 * Given a STRUCT TY, returns a DST_IDX for the
 * TY. Appends it to the current scope.
 *
 * A list of DSTs associated with STRUCT TYs is
 * kept for recursive types.
 *
 *===================================================
*/
static DST_INFO_IDX
cwh_dst_struct_type(TY_IDX ty)
{
  DST_INFO_IDX i ;

  USRCPOS s;

  USRCPOS_clear(s);

  i = cwh_dst_struct_has_DST(ty);
  
  if (DST_IS_NULL(i) || Top_ST_has_dope) {

    i = DST_mk_structure_type(s,
			      TY_name(ty),
			      TY_size(ty),
			      DST_INVALID_IDX, 
			      FALSE);

    Top_ST_has_dope = FALSE;
    cwh_dst_struct_set_DST(ty,i) ;

    FLD_HANDLE f = TY_fld(Ty_Table[ty]);

    while (!f.Is_Null ()) {
      (void) cwh_dst_member(f,i);
      f = FLD_next(f);
    }			

    DST_append_child(current_scope_idx, i);
  }

  return i;
}

/*===================================================
 *
 * cwh_dst_substring_type
 *
 * Given a character TY with a 1D KIND_ARRAY of
 * scalars, make the substring IDX.
 *
 *===================================================
*/
static DST_INFO_IDX
cwh_dst_substring_type(TY_IDX ty)
{

  DST_INFO_IDX i    ;
  DST_cval_ref len  ;
  DST_flag     const_len ;
  USRCPOS s;

  USRCPOS_clear(s);

  ARB_HANDLE arb = TY_arb(ty);

  const_len = ARB_const_ubnd(arb);

  if (const_len)
    len.cval = ARB_ubnd_val(arb);
  else {
    len.ref =  cwh_dst_mk_variable(&St_Table[ARB_ubnd_var(arb)]);
    DST_append_child(current_scope_idx,len.ref);
  }

  i = DST_mk_string_type(s,
			 TY_name(ty),
			 const_len,
			 len);

  DST_append_child(current_scope_idx, i);
  return i;
}

/*===================================================
 *
 * cwh_dst_is_character_TY
 *
 * Given TY, returns T if this is the TY that 
 * represents a character substring. ie: a
 * 1D KIND_ARRAY of scalar characters. Assumed
 * to be called with an KIND_ARRAY.
 *
 *===================================================
*/
static BOOL
cwh_dst_is_character_TY(TY_IDX ty)
{
  TY_IDX  ts  ;
  BOOL rs  ;

  DevAssert((TY_kind(ty) == KIND_ARRAY),("bad char ty"));

  rs = FALSE;
  ts = TY_AR_etype(ty);

  if (TY_is_character(Ty_Table[ts])) 
    if (TY_kind(ts) == KIND_SCALAR) 
      rs = TRUE;

  return rs ;
}

/*===================================================
 *
 * cwh_dst_dope_type
 *
 * Given a TY which which uses a dope vector, make
 * the type IDX for the dope and pass it back. The
 * ST is a convenience to form adresses in the subrange
 * IDX (bounds expressions) for assumed shape dummies.
 * (to get the correct dereferencing). if the ST is NULL
 * then it's a type component and the derefs are constant
 * offsets into the type.
 *
 * The address in the dope is at location 0, so there
 * isn't any fiddling with the offset in the parent
 * routines, just the offset of a derived type entry
 * or common.
 *
 * comp == component of derived type.
 *
 *===================================================
*/
static DST_INFO_IDX
cwh_dst_dope_type(TY_IDX  td , ST * st, mINT64 off, DST_INFO_IDX parent, BOOL comp, DST_INFO_IDX *dope_ty)
{
  DST_INFO_IDX i ;
  DST_INFO_IDX t ;

  USRCPOS s;
  TY_IDX ty;

  char *n  = '\0';

  USRCPOS_clear(s);

  /* Create a type for the dope vector itself.  This is attached to the */
  /* object to be used by the debugger for cracking the dope vector.    */

  *dope_ty = cwh_dst_mk_type(td);

  ty = GET_DOPE_BASE_TY(td);

  if (TY_kind(ty) == KIND_ARRAY) {
    ty = TY_AR_etype(ty);

    t  = cwh_dst_mk_type(ty);
    i  = DST_mk_array_type(s,n,t,0,DST_INVALID_IDX,TRUE);

    cwh_dst_dope_bounds(td,st,off,i,parent, comp);
    DST_append_child(parent,i);

  } else {

    i = cwh_dst_mk_type(ty);

  }

  if (comp) {
    i = DST_mk_pointer_type(i,
			    DW_ADDR_none,
			    Pointer_Size);
    DST_append_child(parent,i);
  }

  return i ;
}

/*===================================================
 *
 * cwh_dst_dope_bounds
 *
 * Get the bounds associated with the dope and
 * put them into subrange types. There may not be
 * any in a scalar pointer. This assumes the bounds
 * are an array of STRUCTs, each - lb,ub,str - and the
 * bounds are all the same size.
 *
 * td   - dope vector TY.
 * st   - NULL if in dtype, or symbol ST.
 * off  - offset of dope into derived type, or
 * arr    idx of array type DST
 * p    - parent idx (symbol or current scope)
 * comp - TRUE if component of derived type
 *===================================================
*/
static void
cwh_dst_dope_bounds(TY_IDX  td, ST * st, mINT64 off, DST_INFO_IDX arr, DST_INFO_IDX p, BOOL comp)
{
  TY_IDX tf;

  DST_cval_ref u  ;
  DST_cval_ref l  ;

  DST_INFO_IDX i ;
  DST_INFO_IDX t ;
  DST_INFO_IDX s ;
  DST_INFO_IDX x ;

  INT32 rnk,k,sz;
  BOOL  str = FALSE;
  enum  str_knd kind;

  FLD_HANDLE fld = GET_DOPE_BOUNDS(td);

  if (st != NULL) 
    str = (ST_sclass(st) == SCLASS_FORMAL) || (ST_sclass(st) == SCLASS_FORMAL_REF) ;

  str  = TY_is_f90_pointer(Ty_Table[td]) || str;
  kind = cwh_dst_stride_kind(GET_DOPE_BASE_TY(td));
    

  /* axis bounds - 1d array of structs, lb,ub,str per struct */

  if (!fld.Is_Null ()) {

    off = FLD_ofst(fld) + off;
    tf  = FLD_type(fld);
    rnk = TY_AR_ubnd_val(tf,0);
    FLD_HANDLE bnd_fld = TY_fld(Ty_Table[TY_AR_etype(tf)]);
    t   = cwh_dst_mk_type(FLD_type(bnd_fld));
    sz  = FLD_ofst(FLD_next(bnd_fld))- FLD_ofst(bnd_fld);

    for (k = 0 ; k <= rnk ; k ++) {

      l.ref = cwh_dst_mk_dope_bound(st,off,t,p,comp);
      off += sz ;
      u.ref = cwh_dst_mk_dope_bound(st,off,t,p,comp);
      off += sz ;

      i = DST_mk_subrange_type(FALSE,l,FALSE,u);
      DST_SET_count(DST_INFO_flag(DST_INFO_IDX_TO_PTR(i))) ;

      if (str) {
        s = cwh_dst_mk_dope_bound(st,off,t,p,comp);
	x = DST_INFO_attributes(DST_INFO_IDX_TO_PTR(i)) ;
	DST_SUBRANGE_TYPE_stride_ref(DST_ATTR_IDX_TO_PTR(x,DST_SUBRANGE_TYPE)) = s ; 

	if (kind == s_TWO_BYTE)
	  DST_SET_stride_2byte(DST_INFO_flag(DST_INFO_IDX_TO_PTR(i))) ;
	else if ((kind == s_BYTE) || (kind == s_CHAR))
	  DST_SET_stride_1byte(DST_INFO_flag(DST_INFO_IDX_TO_PTR(i))) ;
      }

      off += sz ;

      DST_append_child(arr,i);
    }
  }
}

/*===================================================
 *
 * cwh_dst_stride_kind
 *
 * Given the base TY of a dope vector, figure 
 * out what sort of stride multiplier it has,
 * consequently, what DST_stride_<info> to put
 * out. The stride is a word, unless integer*1,*2 or
 * logical*1,2 or a derived type with only character
 * components or subtypes.
 *
 *===================================================
*/
static enum str_knd
cwh_dst_stride_kind(TY_IDX  ty)
{
  enum str_knd rt = s_NONE;
  enum str_knd at ;
  
  switch (TY_kind(ty)) {
  case KIND_ARRAY:
    rt = cwh_dst_stride_kind(TY_AR_etype(ty));
    break;
    
  case KIND_STRUCT:
    if (IS_DOPE_TY(ty)) 
      rt = s_WORD ;
    else {

      FLD_HANDLE f = TY_fld(Ty_Table[ty]);
      while ((!f.Is_Null ()) && ((rt == s_CHAR) || (rt == s_NONE))) {

	at = cwh_dst_stride_kind(FLD_type(f)) ;

	if (at == s_CHAR) 
	  rt = s_CHAR ;
	else
	  rt = s_WORD;

	f = FLD_next(f);
      }
    }
    break;
    

  case KIND_SCALAR:
    if (cwh_types_is_character(ty))
      rt = s_CHAR;
    else if ((TY_mtype(ty) == MTYPE_I1) || (TY_mtype(ty) == MTYPE_U1))
      rt = s_BYTE ;
    else if ((TY_mtype(ty) == MTYPE_I2)  || (TY_mtype(ty) == MTYPE_U2))
      rt = s_TWO_BYTE ;
    else
      rt = s_WORD;
    break ;

  case KIND_POINTER:
    rt = cwh_dst_stride_kind(TY_pointed(ty));
    break ;

  default:
    DevAssert((0),(" dope type"));
    
  }

  return rt ;
}

/*===================================================
 *
 * cwh_dst_member
 *
 * Given an FLD make a member IDX. For fortran we
 * assume there are no bitfields or static members.
 *
 * Result tacked on to parent
 *
 *===================================================
*/

static DST_INFO_IDX
cwh_dst_member(FLD_HANDLE fld, DST_INFO_IDX parent)
{
  DST_MEMBER	*def_attr;
  DST_ATTR_IDX	 def_attr_idx;
  DST_INFO	*def_info;
  DST_INFO_IDX   dope_ty;
  BOOL           dope ;
  DST_INFO_IDX   i ;
  DST_INFO_IDX   t ;
  TY_IDX         ty;

  USRCPOS s;


  USRCPOS_clear(s);

  Making_FLD_DST=TRUE ;

  ty = FLD_type(fld);
  dope = IS_DOPE_TY(ty);

  if (dope) 
    t = cwh_dst_dope_type(ty,
                          Top_ST,
                          FLD_ofst(fld),
                          parent,
                          TRUE,
                          &dope_ty);
   else
    t = cwh_dst_mk_type(ty);

  i = DST_mk_member(s,
		    FLD_name(fld),
		    t,
		    FLD_ofst(fld),
		    0, 
		    FLD_bofst(fld),
		    FLD_bsize(fld),
		    FLD_is_bit_field(fld),
		    FALSE, 
		    FALSE,
		    FALSE);

  if (dope) {
     def_info     = DST_INFO_IDX_TO_PTR(i);
     def_attr_idx = DST_INFO_attributes(def_info);
     def_attr     = DST_ATTR_IDX_TO_PTR(def_attr_idx, DST_MEMBER);

     DST_MEMBER_dopetype(def_attr) = dope_ty;
     DST_SET_f90_pointer(DST_INFO_flag(def_info));
  }

  DST_append_child(parent,i);

  Making_FLD_DST=FALSE ;
  return i;

}

/*===================================================
 *
 * cwh_dst_struct_has_DST
 *
 * Given an STRUCT TY, search the list of
 * Struct TYs for a DST. If found, return it.
 *
 * Dwarf won't handle addressing operations in 
 * structures, but the bounds of a pointer component
 * (dope) are in a structure. A new DST is emitted
 * for each variable of a type that includes a pointer
 * array component, so the bounds in the array
 * subrange DST can get DST location entries. eg:
 * 
 *   type t1  
 *     integer, pointer, dimension :: a(:)
 *   end type
 *   type (t1) var1, var2
 * 
 * gets a distinct type die for var1 & var2. Hence
 * can't enter such TYs here.
 *
 *===================================================
*/
static DST_INFO_IDX
cwh_dst_struct_has_DST(TY_IDX ty)
{
  INT32 i   ;
  TY_IDX ts  ;
  FLD_HANDLE fld ;
  BOOL has_ptr_array_dope = FALSE;

  if (!IS_DOPE_TY(ty)) {

    ts = ty ;

    fld = TY_fld(Ty_Table[ts]);

    while (!fld.Is_Null () && !has_ptr_array_dope) {

      ts = FLD_type(fld) ;

      TY& t = Ty_Table[ts];

      if (IS_DOPE_TY(ts)) 
	if (TY_is_f90_pointer(t)) {
	  ts = GET_DOPE_BASE_TY(ts);
	  if (TY_kind(ts) == KIND_ARRAY)
	    if (TY_kind(TY_AR_etype(Ty_Table[ts])) == KIND_STRUCT) /* allow recursive types */
	      if (!Making_FLD_DST)                       /* as ptr components but */
		has_ptr_array_dope = TRUE;               /* not at top level      */
	}

      fld = FLD_next(fld);
    }
  }

  if (! has_ptr_array_dope) {

    /* look for most recent, in case of duplicates for ptrs */

    for(i = Struct_Top ; i >= 0 ; i --) 
      if (ty == Struct_DSTs[i].ty)
	return Struct_DSTs[i].idx;

  }

  return (DST_INVALID_IDX);
}

/*===================================================
*
* cwh_dst_struct_set_DST
*
* Associate a STRUCT TY and a DST_INFO_IDX. A slot
* for a visited flag in the TY would be better.
*
* There are 2 reasons for this routine
*  a) to economize on DST entries.
*  b) to handle recursive types.
*
*===================================================
*/
static void
cwh_dst_struct_set_DST(TY_IDX ty, DST_INFO_IDX i)
{
  Struct_Top ++ ;
  if (Struct_Top >= Struct_Current_Size) {
    Struct_Current_Size += STRUCT_DST_SIZE_CHANGE;
    Struct_DSTs = (TYIDX *) realloc(Struct_DSTs,sizeof(TYIDX)*Struct_Current_Size);
  }

  Struct_DSTs[Struct_Top].ty  = ty;
  Struct_DSTs[Struct_Top].idx = i;
}

/*===================================================
 *
 * cwh_dst_struct_clear_DSTs
 *
 * Clean up the list of STRUCT<->DST entries.
 *
 *===================================================
*/
static void 
cwh_dst_struct_clear_DSTs(void)
{
  Struct_Top = -1 ;
}

/*===================================================
 *
 * cwh_dst_add_inner
 *
 * Remember the DST_IDX of an inner procedure
 * until the parent comes along.
 *
 *===================================================
*/
static void
cwh_dst_inner_add_DST(DST_INFO_IDX i)
{

  Inner_Top ++ ;

  if (Inner_Top >= Inner_Current_Size) {
     Inner_Current_Size += INNER_DST_SIZE_CHANGE;
     Inner_DSTs = (DST_INFO_IDX *) realloc(Inner_DSTs,sizeof(DST_INFO_IDX)*Inner_Current_Size);
  }

  Inner_DSTs[Inner_Top] = i;
}

/*===================================================
 *
 * cwh_dst_inner_clear_DSTs
 *
 * Clean up the array of inner procedure DST's
 *
 *===================================================
*/
static void 
cwh_dst_inner_clear_DSTs(void)
{
  Inner_Top = -1 ;
}

/*===================================================
 *
 * cwh_dst_inner_read_DSTs
 *
 * Given a DST of a parent procedure, tack on all
 * its inner procedures.
 *
 *===================================================
*/
static void 
cwh_dst_inner_read_DSTs(DST_INFO_IDX parent)
{
  INT32 i ;

  for(i = 0 ; i <= Inner_Top ; i ++) 
    DST_append_child(parent,Inner_DSTs[i]);
}

/*===================================================
 *
 * cwh_dst_subrange
 *
 * Given an ARB make a subrange type. 
 *
 *===================================================
*/
static DST_INFO_IDX
cwh_dst_subrange(ARB_HANDLE ar) 
{
  DST_INFO_IDX i ;
  DST_cval_ref lb,ub;
  DST_flag     const_lb,const_ub ;
  BOOL         extent = FALSE ;
  const_lb = ARB_const_lbnd(ar) ;
  const_ub = ARB_const_ubnd(ar) ;

  if (const_lb)
    lb.cval = ARB_lbnd_val(ar) ;
  else {
    lb.ref = cwh_dst_mk_variable(&St_Table[ARB_lbnd_var(ar)]);
    DST_append_child(current_scope_idx,lb.ref);
  } 

  if (const_ub)
    ub.cval = ARB_ubnd_val(ar) ;
  else {
    ub.ref  = cwh_dst_mk_variable(&St_Table[ARB_ubnd_var(ar)]);
    DST_append_child(current_scope_idx,ub.ref);
  }

  i = DST_mk_subrange_type(const_lb,
			   lb, 
			   const_ub,
			   ub);

  if (extent) 
    DST_SET_count(DST_INFO_flag(DST_INFO_IDX_TO_PTR(i))) ;

  return i;
}
/*===================================================
 *
 * cwh_dst_mk_dope_bound
 *
 * Make a DST dope bound. This is an anonymous variable
 * whose type is the type of the FLD and whose address
 * is derived from the ST of the dope. If the ST is NULL,
 * then the bound is in a dtype component and is an offset.
 * But dbx doesn't like this dwarf, so we make a new DST struct
 * for each dope vector and and set the bounds as DST locations.
 *
 * The dwarf location is 
 *     non-based:     <ST location> <offset>
 *     based/formal:  <ST location> <deref> <offset>
 *     component:     <offset>
 *
 * dp        - dope vector
 * offset    - into common or struct of this bound.
 * t         - idx of bound type
 * p         - idx of parent scope (common/current scope)
 * component - is ptr component of derived type? 
 *
 *===================================================
*/
static DST_INFO_IDX
cwh_dst_mk_dope_bound(ST *dp, mINT64 offset, DST_INFO_IDX t, DST_INFO_IDX p, BOOL component)
{
  DST_INFO_IDX i ;

  TY_IDX ty ;  
  BOOL dr  = FALSE;             /* deref reqd */
  BOOL ce  = FALSE;             /* common element (eg:module data) */
  BOOL dapc= FALSE;             /* dope array with array valued ptr component */
  char *n  = '\0';
  USRCPOS s;

  USRCPOS_clear(s);

  DevAssert((dp != NULL),(" missing dope ST "));

  BOOL class_based = (ST_base_idx(dp) != ST_st_idx(dp));
  ce = (class_based &&
	 ((ST_sclass(ST_base(dp)) == SCLASS_COMMON) ||
	  (ST_sclass(ST_base(dp)) == SCLASS_DGLOBAL))) ;

  dr  = (ST_sclass(dp) == SCLASS_FORMAL) || (ST_sclass(dp) == SCLASS_FORMAL_REF);

  /* If a pointer component & f90 pointer, then the bounds are */
  /* indirectly accesssed via the address in the ptr's dope.   */
  /* If in common, it's an offset into the common              */

  dr |= (class_based && !ce) || 
         ((component && ST_auxst_is_f90_pointer(dp))) ;

  /* if a dtype array with an array valued ptr component, then   */
  /* can't represent ptr bounds with location (ie: need to index */ 
  /* into dope vector array) so use a field, as we'd like to do  */
  /* for all bounds                                              */

  ty = ST_type(dp);
  dapc = TY_kind(ty) == KIND_ARRAY && Making_FLD_DST ;

  /* but since dbx falls over immediately with array location of  */
  /* DW_AT_member the locations of the array bounds of the 1st    */
  /* pointer compenent are generated.                             */

  dapc = FALSE;
  
  if (!dapc) {

    if (ce) {

      i = DST_mk_variable_comm(s,
			       NULL,
			       t,
			       (void *) ST_st_idx(ST_base(dp)),
			       offset);

    } else {

      i = DST_mk_variable(s,
			  n,
			  t,
			  offset,
			  (void *) ST_st_idx(dp),
			  DST_INVALID_IDX,
			  FALSE,
			  ST_sclass(dp) == SCLASS_AUTO,
			  FALSE, 
			  TRUE);
    }

    if (dr) 
      DST_SET_base_deref(DST_INFO_flag(DST_INFO_IDX_TO_PTR(i)));

  } else {

    i = DST_mk_member(s,
		      n,
		      t,
		      offset,
		      Pointer_Size, /*FIX */
		      0,FALSE,FALSE,FALSE,FALSE,FALSE); 

  }
  DST_append_child(p,i);  

  return i ;
}

/*===================================================
 *
 * cwh_dst_has_dope
 *
 * Does this derived type TY contain any dope information.
 * If so TRUE. A dope TY at the top level is ignored
 * because it belongs to a variable.
 *
 * Assumes STRUCTS have been flattened.
 *
 *===================================================
*/
static BOOL
cwh_dst_has_dope(TY_IDX  ty)
{
  while(TY_kind(ty) == KIND_POINTER)
    ty = TY_pointed(ty);

  if (!IS_DOPE_TY(ty)) {

    if (TY_kind(ty) == KIND_STRUCT) {

      FLD_HANDLE fld = TY_fld(Ty_Table[ty]);

      while (!fld.Is_Null ()) {
	if (IS_DOPE_TY(FLD_type(fld)))
	  if (!GET_DOPE_BOUNDS(FLD_type(fld)).Is_Null ())
	    return(TRUE);

	fld = FLD_next(fld);
      }
    }
  }

  return (FALSE);
}

/*===================================================
 *
 * DST_set_assoc_idx
 *
 * Set up the DST/ST be association. The ST was stuffed 
 * into the fe_ptr of the DST_ASSOC_INFO when the DST 
 * was made. Now the ST's have been written, convert the
 * ASSOC info into the BE ST indexes.
 *
 *===================================================
*/
static INT32
DST_set_assoc_idx(INT32 dummy, 
		  DST_DW_tag tag, 
		  DST_flag flag, 
		  DST_ATTR_IDX iattr, 
		  DST_INFO_IDX inode)
{
   DST_INFO       *node;
   DST_ASSOC_INFO *assoc;
   mINT32	  level, index;
   ST_IDX         st;
#if 0 // buggy code--FMZ
   if (DST_IS_assoc_fe(flag))
   {  
      /* Set the ASSOC_INFO_st_level && ASSOC_INFO_st_index fields 
       * pointed to by the assoc_info field in the 
       * source_correspondence.  We need to dispatch on the tag value
       * to determine the form of assoc_info.
      */
      switch (tag)
      {
      case DW_TAG_subprogram:
	 if (DST_IS_memdef(flag))
	 {
	    assoc = &DST_SUBPROGRAM_memdef_st(
		        DST_ATTR_IDX_TO_PTR(iattr, DST_SUBPROGRAM));
	 }
         else if (!DST_IS_declaration(flag))
	 {
	    assoc = &DST_SUBPROGRAM_def_st(
		        DST_ATTR_IDX_TO_PTR(iattr, DST_SUBPROGRAM));
	 }
	 else
	 {
	    DevAssert((FALSE), ("Illegal subprogram DST_ASSOC_INFO")); 
	 }
	 st = (ST_IDX) pDST_ASSOC_INFO_fe_ptr(assoc);
	 Get_ST_Id( st,  &level, &index );
	 pDST_ASSOC_INFO_st_idx(assoc) = st;
	 break;
	 
      case DW_TAG_entry_point:
	 assoc = &DST_ENTRY_POINT_st(
			DST_ATTR_IDX_TO_PTR(iattr, DST_ENTRY_POINT));
	 st = (ST_IDX) pDST_ASSOC_INFO_fe_ptr(assoc);
	 Get_ST_Id( st,  &level, &index );
	 pDST_ASSOC_INFO_st_idx(assoc) = st;
	 break;

      case DW_TAG_formal_parameter:
	 assoc = &DST_FORMAL_PARAMETER_st(
		     DST_ATTR_IDX_TO_PTR(iattr, DST_FORMAL_PARAMETER));
	 st = (ST_IDX) pDST_ASSOC_INFO_fe_ptr(assoc);
	 Get_ST_Id( st,  &level, &index );
	 pDST_ASSOC_INFO_st_idx(assoc) = st;
	 break;

      case DW_TAG_common_block:
	 assoc = &DST_COMMON_BLOCK_st( 
			DST_ATTR_IDX_TO_PTR(iattr, DST_COMMON_BLOCK ) );
	 st = (ST_IDX) pDST_ASSOC_INFO_fe_ptr(assoc);
	 Get_ST_Id( st,  &level, &index );
	 pDST_ASSOC_INFO_st_idx(assoc) = st;
	 break;

      case DW_TAG_variable:
	 if (DST_IS_comm(flag)) {
	    assoc = &DST_VARIABLE_comm_st(
			DST_ATTR_IDX_TO_PTR(iattr, DST_VARIABLE)); 
	 }
	 else if (DST_IS_memdef(flag))
	 {
	    assoc = &DST_VARIABLE_memdef_st(
			DST_ATTR_IDX_TO_PTR(iattr, DST_VARIABLE));
	 }
         else if (!DST_IS_declaration(flag))
	 {
	    assoc = &DST_VARIABLE_def_st(
			DST_ATTR_IDX_TO_PTR(iattr, DST_VARIABLE));
	 }
	 else
	 {
	    DevAssert((FALSE), ("Illegal DST variable assoc ptr"));
	 }


	 st = (ST_IDX) pDST_ASSOC_INFO_fe_ptr(assoc);

	 Get_ST_Id( st,  &level, &index );
	 pDST_ASSOC_INFO_st_idx(assoc) = st;
	 break;
	 
      case DW_TAG_label:
	 assoc = &DST_LABEL_low_pc(DST_ATTR_IDX_TO_PTR(iattr, DST_LABEL));
         DevAssert((0),("NEW_SYMTAB: DW_TAG_label"));
	 pDST_ASSOC_INFO_st_idx(assoc) = make_ST_IDX(index,level);
	 break;
	 
      case DW_TAG_lexical_block:
	 assoc = &DST_LEXICAL_BLOCK_low_pc(
		     DST_ATTR_IDX_TO_PTR(iattr, DST_LEXICAL_BLOCK));
         DevAssert((0),("NEW_SYMTAB: DW_TAG_lexical_block"));
	 pDST_ASSOC_INFO_st_idx(assoc) = make_ST_IDX(index,level);
	 assoc = &DST_LEXICAL_BLOCK_high_pc(
		     DST_ATTR_IDX_TO_PTR(iattr, DST_LEXICAL_BLOCK));
         DevAssert((0),("NEW_SYMTAB: DW_TAG_lexical_block"));
	 pDST_ASSOC_INFO_st_idx(assoc) = make_ST_IDX(index,level);
	 break;

      case DW_TAG_inlined_subroutine:
	 assoc = &DST_INLINED_SUBROUTINE_low_pc(
		     DST_ATTR_IDX_TO_PTR(iattr, DST_INLINED_SUBROUTINE));
	 st = (ST_IDX) pDST_ASSOC_INFO_fe_ptr(assoc);
	 Get_ST_Id( st,  &level, &index );
	 pDST_ASSOC_INFO_st_idx(assoc) = st;
	 assoc = &DST_INLINED_SUBROUTINE_high_pc(
		     DST_ATTR_IDX_TO_PTR(iattr, DST_INLINED_SUBROUTINE));
	 st = (ST_IDX) pDST_ASSOC_INFO_fe_ptr(assoc);
	 Get_ST_Id( st,  &level, &index );
	 pDST_ASSOC_INFO_st_idx(assoc) = st;
	 break;

       default:
	 DevAssert((FALSE),("Invalid DST_ASSOC_INFO field access"));
	 break;
      }
      node = DST_INFO_IDX_TO_PTR(inode);
      DST_SET_assoc_idx(DST_INFO_flag(node));
      DST_RESET_assoc_fe(DST_INFO_flag(node));
   }
#endif

   return dummy;
}


/*===================================================
 *
 * DST_get_ordinal_num
 *
 * get the DST number of this file. Looks in the
 * list of files, and returns an index if exists,
 * otherwise creates an entry.
 *
 *===================================================
*/

static mUINT16
DST_get_ordinal_num(char    *the_name, 
		    char  ***the_list, 
		    mUINT16 *the_next, 
		    mUINT16 *the_size)
{
  mUINT16 idx, next = *the_next, size = *the_size;
  char  **list = *the_list;
   
  /* See if it exists */

  if ((the_name == NULL) || (the_name[0] == '\0')) 
    idx = 0;
  else {
    for (idx = 0; (idx < next) && (strcmp(the_name, list[idx]) != 0); idx += 1);
	  	   
    /* does not exist so create it */

    if (idx == next)   {
      if (next >= size) {
	size += DST_NAME_TABLE_SIZE;
	*the_size = size;
	if (next == 0)
	  list = (char **)malloc(size*sizeof(char *)); 
	else
	  list = (char **)realloc((char *)list,size*sizeof(char *)); 
				  
	*the_list = list;
      }
      list[next] = the_name;
      *the_next += 1;
    }
    idx += 1;	/* ordinal number is one larger than dir_list idx */
  }
  return idx;
}

/*===================================================
 *
 * DST_write_files
 *
 * Write out DST filelist - these are written out
 * as filename string, pathname #. The list was 
 * created with cwh_dst_enter_file.
 *
 ====================================================
*/
static DST_FILE_IDX
DST_write_files(void)
{
   struct stat       fstat;
   char             *dir_name, *file_name;
   UINT64            file_size ;
   UINT64            fmod_time ;
   DST_FILE_IDX      file_idx, first_file_idx = DST_INVALID_INIT;
   UINT32            dir_length;
   INT32             name_idx;
   
   for (name_idx = 0; name_idx < next_file_idx; name_idx += 1)
   {
      file_name = file_list[name_idx];

      if (stat(file_name, &fstat) == 0) {
	 file_size = (UINT64)fstat.st_size;
	 fmod_time = (UINT64)fstat.st_mtime;

      } else {
	file_size = 0ll;  
	fmod_time = 0ll;  
      }
      DST_directory_of(file_name, &dir_name, &dir_length);
      file_idx = DST_mk_file_name(
		    &file_name[dir_length],        /* name */
		    DST_get_ordinal_num(dir_name,  /* path */
				        &dir_list,
				        &next_dir_idx,
				        &dir_list_size),
		    file_size,
                    fmod_time);
      if (name_idx == 0)
	 first_file_idx = file_idx;
   }
   return first_file_idx;
}

/*===================================================
 *
 * DST_directory_of
 *
 * Copy file_path into dir_name then remove the 
 * filename by placing a '\0' at the last '/'
 * can be used separately.
 *
 ====================================================
*/

static void
DST_directory_of(char *file_path, char **dir_name, UINT32 *dir_length)
{
   char *dir;

   *dir_name = ux_strdup(file_path);
   dir= strrchr(*dir_name,'/')  ;

   *dir = '\0';
   *dir_length = dir - *dir_name + 1  ;
}

/*===================================================
 *
 * DST_write_directories
 *
 * Write out all directories, and return the idx
 * of the first.
 *
 ====================================================
*/
static DST_DIR_IDX
DST_write_directories(void)
{
   mUINT16     name_idx;
   DST_DIR_IDX dir_idx, first_idx = DST_INVALID_INIT;
   
   for (name_idx = 0; name_idx < next_dir_idx; name_idx += 1) {

      dir_idx = DST_mk_include_dir(dir_list[name_idx]);
      if (name_idx == 0)
	 first_idx = dir_idx;
   }

   return first_idx;
}

/*===================================================
 *
 * cwh_dst_enter_path
 *
 * Enter a new pathname into the file_list and get 
 * back its index.
 *
 ====================================================
*/
extern mUINT16
cwh_dst_enter_path(char * fname)
{
   mUINT16  idx;
   mUINT16  old;
   char *file_name;
   
   file_name = Make_Absolute_Path(fname);

   old = next_file_idx;
   idx = DST_get_ordinal_num(file_name,
			     &file_list,
			     &next_file_idx,
			     &file_list_size);

   if (next_file_idx == old)
     free(file_name);

   return idx ;
}

/*===================================================
 *
 * cwh_dst_get_command_line_options
 *
 * Given the set of options passed into the front-end, string
 * together the ones of interest for debugging and return
 * the resultant string.  The options of interest depends on 
 * the level of debugging.  The caller should free the malloced
 * string once it is no longer needed.
 *
 *
 ====================================================
*/

static char *
cwh_dst_get_command_line_options(void) 
{
  INT32	    i, 
            strlength = 0;
  INT32     num_opts = 0;
  char    **selected_opt;
  INT32    *opt_size;
  char     *rtrn, *cp;
  char      ch;
  BOOL	record_option;

  if (FE_command_line != NULL) {
      /*
       * driver passed in a command-line file, whose contents are (currently)
       *
       *    command-line
       *    current working directory
       *
       * As it turns out, we need only the first line. We read it in, and use
       * it in place of the arguments passed in to the frontend, which are
       * generally useless for the purposes of tools that need to
       * automatically rebuild these objects..
       *
       * If no command line was passed in, we just return the contents of
       * argv[] as usual.
       */

      struct stat statb;
      FILE *cmdfile;

      if ((cmdfile = fopen(FE_command_line, "r")) != NULL) {
	  if (fstat(ux_fileno(cmdfile), &statb) == 0) {
	      char *endcp;
	  
	      /* allocate a buffer as big as the file: this is safe and fast */
	      rtrn = (char *) malloc(statb.st_size+1);

	      /* we need only the first line */
	      fgets(rtrn, statb.st_size, cmdfile);

	      /* scan for newline or end of buffer, and stick in a
	       * terminating NULL */
	      for (cp = rtrn, endcp = rtrn+statb.st_size;
		   *cp != 0 && *cp != '\n' && cp < endcp;
		   cp++);
	      *cp = '\0';
	      fclose(cmdfile);
	      return rtrn;
	  }
	  /* If we got here, we managed to fopen the file, but not fstat it.
	   * This is technically an internal error, but what the heck, we
	   * will just ignore it and fall through to the "default" code..
	   */
	  fclose(cmdfile);
      }
  }
  /* else fall back to returning argv[] formatted into string */

  selected_opt = (char **)malloc(sizeof(char*) * save_argc);
  opt_size = (INT32 *)malloc(sizeof(INT32) * save_argc);
  
  for (i = 1; i < save_argc; i++)
  {
     if (save_argv[i] != NULL && save_argv[i][0] == '-')
     {
	ch = save_argv[i][1];  /* Next flag character */
	if (Debug_Level <= 0)
	   /* No debugging */
	   record_option = (ch == 'g' || /* Debugging option */
			    ch == 'O');  /* Optimization level */
	else
	   /* Full debugging */
	   record_option = (ch == 'D' || /* Macro symbol definition */
			    ch == 'g' || /* Debugging option */
			    ch == 'I' || /* Search path for #include files */
			    ch == 'O' || /* Optimization level */
			    ch == 'U');  /* Macro symbol undefined */
	if (record_option)
	{
	   opt_size[num_opts] = strlen(save_argv[i]) + 1; /* Arg + space/null */
	   selected_opt[num_opts] = save_argv[i];
	   strlength += opt_size[num_opts];
	   num_opts += 1;
	}
     }
  }
  
  if (strlength == 0)
  {
     rtrn = (char *)calloc(1, 1); /* An empty string */
  }
  else
  {
     rtrn = (char *)malloc(strlength);
     cp = rtrn;

     /* Append the selected options to the string (rtrn) */
     for (i = 0; i < num_opts; i++)
	if (opt_size[i] > 0)
	{
	   cp = strcpy(cp, selected_opt[i]) + opt_size[i];
	   cp[-1] = ' '; /* Space character */
	}
     cp[-1] = '\0'; /* Terminating null character */
  }
  
  free(selected_opt);
  free(opt_size);
  return rtrn;
}

static char
Get_ST_Id (ST_IDX st_idx, INT *level, INT *index)
{
  if (st_idx) {

    *level = ST_IDX_level(st_idx);
    *index = ST_IDX_index(st_idx);
  }

  else {

    *level = 0;
    *index = 0;
  }

  return NULL;
}

/*================================================================
 *
 * char * cwh_dst_filename_from_filenum(SRCPOS s)
 *
 * given a file number, return a pointer to the associated file name. 
 *
 *================================================================
*/
extern char *
cwh_dst_filename_from_filenum(INT idx)
{
  Is_True((idx > 0 && idx <= next_file_idx),("Bad file number (%d)\n",idx));
  return file_list[idx-1];
}
