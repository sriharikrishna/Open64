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
 * Module: config_lno.h
 * $Revision: 1.2 $
 * $Date: 2002-07-12 16:48:28 $
 * $Author: fzhao $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/common/com/config_lno.h,v $
 *
 * Revision history:
 *  02-Nov-96 - Original Version
 *
 * Description:
 *
 * Define the external interface to the internal flags representing the
 * -LNO group options.  It is a single struct, so that addition of
 * flags for new options does not require additions to the be Exported
 * file, and so that push/pop operations are simplified.
 *
 * NOTE:  Only the standard group option reader, and routines in the
 * associated file config_lno.c, should modify the structs declared
 * here.  By following this discipline, leaving a few undefined flags
 * at the end of the struct, and adding new flags there, we can avoid 
 * serious version incompatibilities between be.so and its clients.
 *
 * ====================================================================
 *
 * To add a new option:
 *
 * (On conversion from the old LNO implementation, I tried to use
 * naming which was mostly like what had been used before, but
 * consistent.  The instructions below reflect the results.)
 *
 *   1) In the LNO_FLAGS options struct defined below, add a field to
 *	receive the new option value.  If you need a flag indicating
 *	whether the option was set explicitly on the command line, add
 *	a BOOL for that as well, with an appended "_set" in its name.
 *	(You might also need another field if the option will be used
 *	in a different form after configuration, i.e. the option value
 *	is a string that is converted to a number.  If so, add another
 *	field.)
 *
 *	The fields are starting out in alphabetical order by option
 *	name.  When adding new ones, keep in mind that adding them in
 *	the middle will create a required correspondence between the
 *	new be.so and lno.so (for purposes of using the later options).
 *	That may be alright, but if you want to avoid it, add the new
 *	fields just before the buffer at the end (and you can move
 *	them into place later when it doesn't matter, if you care).
 *
 *   2) Below the LNO_FLAGS definition are #defines for the
 *	"LNO_Option_Name" pseudo-variables that everyone will use to
 *	reference them.  Add #defines for your new ones.  Note that
 *	they all have LNO_ prefixes and are capitalized like global
 *	variables (which they were before this implementation).
 *
 *   3) There are two static instances of LNO_FLAGS in config_lno.c.
 *	Default_LNO contains default values to be used when
 *	initializing new structs (when we implement pushing/popping
 *	for regions), and Initial_LNO contains the initial defaults.
 *	Add the correct default values for your options there.
 *
 *   4) The option group descriptor is also in config_lno.c.  Add your
 *	new option there.  Note that the option descriptors are
 *	specified using a small set of macros defined above the
 *	descriptor.
 *
 *   5) If any configuration is required after reading them in, add the
 *	required code to LNO_Configure in config_lno.c.
 *
 * For memory hierarchy options, treatment is similar based on the MHD
 * class defined in config_cache.h.  Note that in that case, there is
 * a main MHD class for single-instance parameters, and a MHD_LEVEL
 * class for parameters with a value for each memory hierarchy level.
 * The above instructions are modified as follows:
 *
 *   1)	Modify MHD/MHD_LEVEL instead of LNO_FLAGS.
 *
 *   2)	No corresponding access #defines have been done for these.
 *	Perhaps they should be.
 *
 *   3)	Instead of changing static instances, change the constructors
 *	in config_cache.cxx.
 *
 *   4)	Same, with different option specification macros.
 *
 *   5) Same.
 *
 * NOTE:  It is NOT necessary to add anything to the be Exported list.
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef config_lno_INCLUDED
#define config_lno_INCLUDED

#ifdef _KEEP_RCS_ID
/*REFERENCED*/
static char *config_lno_h_rcs_id = "$Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/common/com/config_lno.h,v $ $Revision: 1.2 $";
#endif /* _KEEP_RCS_ID */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ====================================================================
 *
 * -LNO: option group
 *
 * Define the global structure containing -LNO option group flags.
 *
 * WARNING:  Most of the fields in this struct must be addressable by
 * an option group descriptor -- hence BOOL instead of mBOOL.
 *
 * ====================================================================
 */

extern BOOL Run_autopar;

/* We reference a memory hierarchy descriptor from config_cache.* */
struct MHD;

typedef struct lno_flags {
  /* Support a stack of structs, e.g. for region support.
   * Each stack element points to its predecessor; the bottom to NIL.
   */
  struct lno_flags *next;	/* Next copy on stack */
  struct MHD *_mhd;		/* Memory hierarchy descriptor */

  BOOL	Aequiv;
  BOOL	Autodist;
  UINT32 Run_AP;
  BOOL	Ill_xform_directives;
  BOOL	Backward_substitution;
  BOOL	Blind_loop_reversal;
  BOOL	Blocking;
  UINT32 Blocking_Size;
  BOOL	Cache_model_edge_effects;
  BOOL	Coupled_opts;
  BOOL	Cse;
  BOOL	Fancy_tile;
  BOOL	Run_fiz_fuse;
  UINT32 Fission;
  UINT32 Fission_inner_register_limit;
  BOOL	Forward_substitution;
  UINT32 Fusion;
  UINT32 Fusion_peeling_limit;
  UINT32 Gather_Scatter;
  UINT32 Graph_capacity;
  BOOL	Hoist_messy_bounds;
  BOOL	Ignore_pragmas;
  BOOL	Interchange;
  BOOL	Run_lego;
  BOOL	Run_lego_set;
  BOOL	Run_lego_localizer;
  BOOL	Loop_finalization;
  UINT32 Max_do_loop_depth_strict;
  BOOL	Mem_sim;
  BOOL	Minvar;
  UINT32 Opt;
  UINT32 Cache_model;
  BOOL	Run_outer;
  UINT32 Outer_unroll;
  BOOL	Outer_unroll_deep;
  UINT32 Outer_unroll_min_for_further_unroll;
  UINT32 Outer_unroll_max;
  BOOL	Outer_unroll_model_only;
  UINT32 Outer_unroll_prod_max;
  BOOL	Outer_unroll_unity;
  UINT32 Outer_unroll_aggre;
  UINT32 Run_p3;
  BOOL	Pseudo_lower;
  UINT32 Run_prefetch;
  BOOL	Run_prefetch_set;
  UINT32 Prefetch_ahead;
  UINT32 Prefetch_iters_ahead;
  UINT32 Prefetch_cache_factor;
  BOOL	Prefetch_indirect;
  BOOL	Run_prefetch_manual;
  BOOL	Run_prefetch_manual_set;
  BOOL	Power_of_two_hack;
  BOOL	Sclrze;
  UINT32 SE_tile_size;
  UINT32 Split_tiles;
  UINT32 Split_tiles_size;
  BOOL	Run_test;
  BOOL	Test_dump;
  BOOL	Trapezoidal_outer_unroll;
  BOOL	Use_malloc;
  BOOL	Use_parm;
  BOOL	Verbose;
  BOOL	Version_mp_loops;
  BOOL	Run_vintr;
  BOOL	Run_oinvar;
  UINT32 Run_doacross;
  UINT32 Preferred_doacross_tile_size;
  UINT32 Parallel_overhead; 
  BOOL Prompl; 
  BOOL IfMinMax; 
  BOOL Run_call_info; 
  BOOL Shackle; 
  BOOL Cross_loop; 
  BOOL IPA_Enabled;
  UINT32 Num_Iters;
  UINT32 Pure_Level;
  UINT32 Small_trip_count;
  UINT32 Local_pad_size;
  UINT32 Full_unrolling;  
  /* This buffer area allows references to new fields to be added in
   * later revisions, from other DSOs, without requiring a new be.so
   * or running the risk of referencing illegal data.  Assuming that
   * the buffer is initialized to zeroes, any such references will
   * simply pick up FALSE values (for the Booleans):
   */
  INT32 buffer[16];	/* Buffer space -- initialize to FALSE */
} LNO_FLAGS;

#define LNO_FLAGS_next(f)	(f->next)
#define LNO_FLAGS_mhd(f)	(f->_mhd)

/* ====================================================================
 *
 * -LNO: option group
 *
 * Global data "objects" and manipulation functions.
 *
 * ====================================================================
 */

/* This is always the current top of stack: */
extern LNO_FLAGS *Current_LNO;
extern struct MHD *Current_MHD;

/* And this is always the invariant bottom of stack: */
extern LNO_FLAGS Initial_LNO;

/* Define pseudo-global-variables for general usage: */
#define LNO_Aequiv			Current_LNO->Aequiv
#define LNO_Autodist			Current_LNO->Autodist
#define LNO_Run_AP			Current_LNO->Run_AP
#define LNO_Apply_Illegal_Transformation_Directives	\
	Current_LNO->Ill_xform_directives
#define LNO_Backward_Substitution	Current_LNO->Backward_substitution
#define LNO_Blind_Loop_Reversal		Current_LNO->Blind_loop_reversal
#define LNO_Blocking			Current_LNO->Blocking
#define LNO_Blocking_Size		Current_LNO->Blocking_Size
#define LNO_Cache_Model_Edge_Effects	Current_LNO->Cache_model_edge_effects
#define LNO_Coupled_Opts		Current_LNO->Coupled_opts
#define LNO_Cse				Current_LNO->Cse
#define LNO_Fancy_Tile			Current_LNO->Fancy_tile
#define LNO_Run_Fiz_Fuse		Current_LNO->Run_fiz_fuse
#define LNO_Fission			Current_LNO->Fission
#define LNO_Fission_Inner_Register_Limit	\
	Current_LNO->Fission_inner_register_limit
#define LNO_Forward_Substitution	Current_LNO->Forward_substitution
#define LNO_Fusion			Current_LNO->Fusion
#define LNO_Fusion_Peeling_Limit	Current_LNO->Fusion_peeling_limit
#define LNO_Gather_Scatter		Current_LNO->Gather_Scatter
#define LNO_Graph_Capacity		Current_LNO->Graph_capacity
#define LNO_Hoist_Messy_Bounds		Current_LNO->Hoist_messy_bounds
#define LNO_Ignore_Pragmas		Current_LNO->Ignore_pragmas
#define LNO_Interchange			Current_LNO->Interchange
#define LNO_Run_Lego			Current_LNO->Run_lego
#define LNO_Run_Lego_Set		Current_LNO->Run_lego_set
#define LNO_Run_Lego_Localizer		Current_LNO->Run_lego_localizer
#define LNO_Loop_Finalization		Current_LNO->Loop_finalization
#define LNO_Max_Do_Loop_Depth_Strict	Current_LNO->Max_do_loop_depth_strict
#define LNO_Mem_Sim			Current_LNO->Mem_sim
#define LNO_Minvar			Current_LNO->Minvar
#define LNO_Opt				Current_LNO->Opt
#define LNO_Cache_Model			Current_LNO->Cache_model
#define LNO_Run_Outer			Current_LNO->Run_outer
#define LNO_Outer_Unroll		Current_LNO->Outer_unroll
#define LNO_Outer_Unroll_Deep		Current_LNO->Outer_unroll_deep
#define LNO_Outer_Unroll_Min_For_Further_Unroll	\
	Current_LNO->Outer_unroll_min_for_further_unroll
#define LNO_Outer_Unroll_Max		Current_LNO->Outer_unroll_max
#define LNO_Outer_Unroll_Model_Only	Current_LNO->Outer_unroll_model_only
#define LNO_Outer_Unroll_Prod_Max	Current_LNO->Outer_unroll_prod_max
#define LNO_Outer_Unroll_Unity		Current_LNO->Outer_unroll_unity
#define LNO_Outer_Unroll_Aggre		Current_LNO->Outer_unroll_aggre
#define LNO_Run_P3			Current_LNO->Run_p3
#define LNO_Pseudo_Lower		Current_LNO->Pseudo_lower
#define LNO_Run_Prefetch		Current_LNO->Run_prefetch
#define LNO_Run_Prefetch_Set		Current_LNO->Run_prefetch_set
#define LNO_Prefetch_Ahead		Current_LNO->Prefetch_ahead
#define LNO_Prefetch_Iters_Ahead	Current_LNO->Prefetch_iters_ahead
#define LNO_Prefetch_Cache_Factor	Current_LNO->Prefetch_cache_factor
#define LNO_Prefetch_Indirect		Current_LNO->Prefetch_indirect
#define LNO_Run_Prefetch_Manual		Current_LNO->Run_prefetch_manual
#define LNO_Run_Prefetch_Manual_Set	Current_LNO->Run_prefetch_manual_set
#define LNO_Power_Of_Two_Hack		Current_LNO->Power_of_two_hack
#define LNO_Sclrze			Current_LNO->Sclrze
#define LNO_SE_Tile_Size		Current_LNO->SE_tile_size
#define LNO_Split_Tiles			Current_LNO->Split_tiles
#define LNO_Split_Tiles_Size		Current_LNO->Split_tiles_size
#define LNO_Run_Test			Current_LNO->Run_test
#define LNO_Test_Dump			Current_LNO->Test_dump
#define LNO_Trapezoidal_Outer_Unroll	Current_LNO->Trapezoidal_outer_unroll
#define LNO_Use_Malloc			Current_LNO->Use_malloc
#define LNO_Use_Parm			Current_LNO->Use_parm
#define LNO_Verbose			Current_LNO->Verbose
#define LNO_Version_Mp_Loops		Current_LNO->Version_mp_loops
#define LNO_Run_Vintr			Current_LNO->Run_vintr
#define LNO_Run_Oinvar			Current_LNO->Run_oinvar
#define LNO_Run_Doacross		Current_LNO->Run_doacross
#define LNO_Preferred_doacross_tile_size	\
			Current_LNO->Preferred_doacross_tile_size
#define LNO_Parallel_Overhead		Current_LNO->Parallel_overhead
#define LNO_Prompl			Current_LNO->Prompl
#define LNO_IfMinMax			Current_LNO->IfMinMax
#define LNO_Run_call_info		Current_LNO->Run_call_info
#define LNO_Shackle 			Current_LNO->Shackle 
#define LNO_Cross_Loop 			Current_LNO->Cross_loop
/* Access to the current TOS struct is via pseudo-global variables: */
/* bounds_check same as subscript_check */
#define LNO_Const_Mod_Warning		(Current_LNO->cmod_warn)

// ipa information available for LNO
#define LNO_IPA_Enabled                     Current_LNO->IPA_Enabled

// estimated number of iters to use for num-symb-iters in LNO
#define LNO_Num_Iters                     Current_LNO->Num_Iters

// 0 => don't create CALL_INFOs for pure functions 
// 1 => (default) create CALL_INFOs for pure functions 
// 2 => 1 + treat "no side effects" functions as pure, too
#define LNO_Pure_Level			  Current_LNO->Pure_Level

// Largest inner loop trip count for which we'll try full unrolling
#define LNO_Small_Trip_Count		Current_LNO->Small_trip_count

// The amount by which to pad local array dimensions
#define LNO_Local_Pad_Size		Current_LNO->Local_pad_size

// The amount by which to pad local array dimensions
#define LNO_Full_Unrolling_Limit	Current_LNO->Full_unrolling

/* Initialize the current top of stack to defaults: */
extern void LNO_Init_Config ( void );

/* Push a new struct on top of stack, either a copy of the current
 * TOS, or the defaults:
 */
extern void LNO_Push_Config ( BOOL use_default );

/* Pop a struct from top of stack and return TRUE if the old TOS was
 * not the original TOS, or do nothing and return FALSE:
 */
extern BOOL LNO_Pop_Config ( void );

/* Configure the current top of stack struct: */
extern void LNO_Configure ( void );


#ifdef __cplusplus
}
#endif /* __cplusplus */
    
#endif /* config_lno_INCLUDED */
