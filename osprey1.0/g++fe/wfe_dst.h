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
 * Module: wfe_dst.h
 * $Revision: 1.1.1.1 $
 * $Date: 2002-05-22 20:08:08 $
 * $Author: dsystem $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/g++fe/wfe_dst.h,v $
 *
 * Revision history:
 *  11-May-93 - Original Version
 *
 * Description:
 *  Extracts from the EDG front-end intermediate representation the
 *  information necessary for the back-end to generate the dwarf 
 *  debugging sections.  Currently, we only supply search-path 
 *  directories, file-descriptors, and scope and declarative information.
 *  In the future we may also supply preprocessing infoamtion.
 *
 *  The subroutines exported through this interface have the following
 *  intended usage and functionality:
 *
 *
 *  DST_build():   ---- wfe_dst.c
 *     Creates DST entries for all declared variables, subroutines,
 *     labels, and types.  This call will also enter file-descriptors
 *     and include directories.  This procedure should only be called
 *     after the EDG front-end processing is complete.  When not in
 *     debugging mode, only file/directory descriptors are entered.
 *     
 *  DST_get_ordinal_file_num():    ---- wfe_dst.c
 *     Should always be called through the macro DST_FILE_NUMBER!  Enters
 *     the file-name into a file-list and returns its ordinal number in 
 *     that list.  If the (full) path name for the file has been seen 
 *     before, then the previous ordinal number is returned and the name 
 *     is not entered into the list a second time.
 *
 *  DST_enter_weak_subroutine():   ---- wfe_dst.c
 *  DST_enter_weak_variable():     ---- wfe_dst.c
 *     Assumes the secondary is a DST_SUBPR_DECL/DST_VAR_DECL and sets 
 *     its "origin" field to the DST_IDX of the primary.
 *
 *  DST_enter_inlined_subroutine(); ---- wfe_dst.c
 *     Called from inline_call_nd in edexpr.c, this generates the DST entries
 *     for an inlined subroutine and its formal parameters and local
 *     variables.
 *
 *  DST_write_to_dotB():      ---- wfe_dst.c
 *     Writes the DST structure out to the .B file, assuming the .B
 *     header information already has already been written out
 *     (through a call to Write_Dot_B_Header).  Also dumps the
 *     DST to file, provided DSTdump_File_Name!=NULL (see "glob.h").
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef wfe_dst_INCLUDED
#define wfe_dst_INCLUDED

#ifdef _KEEP_RCS_ID
static char *wfe_dst_rcs_id = "$Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/g++fe/wfe_dst.h,v $ $Revision: 1.1.1.1 $";
#endif /* _KEEP_RCS_ID */

#ifdef __cplusplus

#include "dwarf_DST.h"

extern void 
DST_build(int num_copts, /* Number of options passed to fec(c) */
	  char *copts[]); /* The array of option passed to fec(c) */

extern void DST_write_to_dotB(void);

extern DST_INFO_IDX DST_Create_Subprogram (ST *func_st,tree fndecl);

extern DST_INFO_IDX DST_Get_Comp_Unit (void);
extern DST_INFO_IDX Create_DST_type_For_Tree(
	tree type_tree, TY_IDX ttidx  , TY_IDX idx);
extern DST_INFO_IDX Create_DST_decl_For_Tree(
	tree decl_node, ST* var_st);

#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif

/* set current line number and current file */
extern void WFE_Set_Line_And_File (unsigned int line, const char* file);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* wfe_dst_INCLUDED */
