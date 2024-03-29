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


/* =======================================================================
 * =======================================================================
 *
 *  Module: cg_flags.h
 *  $Revision: 1.1.1.1 $
 *  $Date: 2002-05-22 20:06:22 $
 *  $Author: dsystem $
 *  $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/cg/cg_flags.h,v $
 *
 *  Description:
 *  ============
 *
 *  Exports variables global to all of CG.
 *
 *  Exported variables:
 *  ==================-
 *
 *  BOOL CG_tail_call
 *	Enable tail call generation.
 *
 *  BOOL CG_unique_exit
 *	Generate unique exit blocks.
 *
 *  BOOL CG_warn_bad_freqs
 *	Whenever a phase notices that freq related data is wrong or
 *      inconsistent, it can warn when this flag is true.
 *
 *  BOOL CG_enable_loop_optimizations
 *	Enable the innermost loop optimizations (CGPREP). This includes
 *	loop unrolling, r/w elimination, recurrence breaking, SWP.
 *
 *  INT32 CG_skip_after
 *  INT32 CG_skip_before
 *  INT32 CG_skip_equal
 *	Controls which PUs we skip optimizing, i.e. we set CG_opt_level=0
 *	for.
 *
 *  INT32 CG_local_skip_after
 *  INT32 CG_local_skip_before
 *  INT32 CG_local_skip_equal
 *	Options to control the optimization done by any phase in CG. The
 *    	interpretation of the numbers to skip is left totally to the 
 *	local phase. For example, the local scheduler uses these options
 *	to control which basic blocks to skip scheduling for.
 *
 *  BOOL CG_skip_local_sched
 *	Enable skipping of scheduling of basic blocks based on the 
 *	-CG:skip_local_[after,before,equal] options.
 *
 *  BOOL CG_skip_local_swp
 *	Enable skipping of pipelining of inner loops based on the 
 *	-CG:skip_local_[after,before,equal] options.
 *
 *  INT CG_opt_level
 *      Gives the optimization level to use throughout CG.
 *
 *  BOOL CG_localize_tns
 *      Make sure we have no global TNs (and thus don't need GRA).
 *  
 *  CG_cond_defs_allowed
 *	Allow generation of conditional definitions (predicated insts)
 *
 *  BOOL CG_enable_reverse_if_conversion
 *  BOOL CG_enable_reverse_if_conversion_overridden
 *      Enable reverse if conversion.
 *  
 *  BOOL CG_enable_thr
 *      Enable tree-height reduction phase in CG.
 * 
 *  BOOL CG_enable_spec_imul
 *      allow loops with speculated integer mul 
 *      to be if converted.
 * 
 *  BOOL CG_enable_spec_idiv
 *      allow loops with speculated integer div 
 *      to be if converted.
 * 
 *  BOOL CG_enable_spec_fdiv
 *      allow loops with speculated fdiv 
 *      to be if converted. recips are also included
 *      with this flag.
 *
 *  BOOL CG_enable_spec_fsqrt
 *      allow loops with speculated fsqrt 
 *      to be if converted.
 *
 *  INT32 CG_maxinss
 *      Maximum number of ops in a loop to be if converted.
 *	This is calculated as: CG_maxinss_default * CG_opt_level
 *
 *  INT32 CG_maxblocks
 *      Maximum number of bbs in a loop to be if converted.
 *
 *  BOOL GRA_LIVE_Phase_Invoked
 *	Flag to detect if global liveness (GRA_LIVE) has been invoked.
 *
 *  BOOL CFLOW_Enable
 *	Master switch.
 *
 *  BOOL CFLOW_opt_before_cgprep
 *	Perform cflow pass before cgprep.
 *
 *  BOOL CFLOW_opt_after_cgprep
 *	Perform cflow pass after cgprep. This includes reordering.
 *
 *  BOOL CFLOW_Enable_Unreachable
 *	Enable unreachable BB removal.
 *
 *  BOOL CFLOW_Enable_Branch
 *	Enable various branch opts.
 *
 *  BOOL CFLOW_Enable_Merge
 *	Enable BB merging.
 *
 *  BOOL CFLOW_Enable_Reorder
 *	Enable BB reordering.
 *
 *  BOOL CFLOW_Enable_Freq_Order
 *	Enable freq-guided BB reordering.
 *
 *  BOOL CFLOW_Enable_Clone
 *	Enable BB cloning.
 *
 *  BOOL CFLOW_opt_all_br_to_bcond
 *	Optimize all branches to conditional branches, ignoring
 *	code bloat effects.
 *
 *  const char *CFLOW_heuristic_tolerance
 *	A floating point number between 0 and 1 inclusive, that specifies
 *	the tolerance in considering the probabilities in a conditional
 *	branch as equally likely. The tolerance is the percentage of
 *      the average probability that the actual probability may vary
 *	above or below the average and still be considered equally likely.
 *	This tolerance only applies to heuristically determined probablities.
 *
 *  const char *CFLOW_feedback_tolerance
 *	Same as CFLOW_heuristic_tolerance except it applies only to
 *	feedback determined probabilities.
 *
 *  UINT32 CFLOW_clone_incr
 *  UINT32 CFLOW_clone_max_incr
 *  UINT32 CFLOW_clone_min_incr
 *	These parameters control the amount of growth that can result
 *	from cloning. clone_incr is the main control and limits
 *	growth to a percentage of the PU size. clone_max_incr and
 *	clone_min_incr set minimum and maximum increments (in instructions).
 *
 *  BOOL FREQ_enable
 *	Enable BB freqency estimates.
 *
 *  BOOL FREQ_view_cfg
 *	Indicates if daVanci should be invoked to view the frequency
 *	annotated CFG at appropriate times.
 *
 *  const char *FREQ_frequent_never_ratio
 *	The ratio in the probabilities of "frequent" to "never" successors
 *	tagged with frequency hint pragmas.
 *
 *  const char *FREQ_eh_freq
 *	The frequency (relative to the entry point) that an exception
 *	handler is executed.
 *
 *  CGSPILL_Rematerialize_Constants
 *	Enable rematerialization of constants instead of spilling them.
 *
 *  CGSPILL_Enable_Force_Rematerialization
 *	Debugging aid -- force all constants to be rematerialized at
 *	each point of use.
 *
 *  LOCS_PRE_Enable_Scheduling
 *	Enable the local scheduler phase before register allocation.
 *
 *  LOCS_POST_Enable_Scheduling
 *      Enable the local scheduler phase after register allocation.
 *
 *  LOCS_Enable_Scheduling
 *      Enables the HyperBlock Scheduler (for single BBs).
 *
 *  LOCS_Enable_Bundle_Formation
 *	Enable the bundle formation phase in LOCS (default TRUE for IA64).
 *
 *  IGLS_Enable_PRE_HB_Scheduling
 *    Enable the Hyperblock Scheduler phase before register allocation.
 *
 *  IGLS_Enable_POST_HB_Scheduling
 *    Enable the Hyperblock Scheduler phase after register allocation.
 *
 *  IGLS_Enable_HB_Scheduling
 *    Enable the Hyperblock Scheduling phase.
 *
 *  GCM_PRE_Enable_Scheduling
 *	Enable global code motion before register allocation (GRA/LRA)
 *
 *  GCM_POST_Enable_Scheduling
 *	Enable global code motion after register allocation (GRA/LRA)
 *
 *  GCM_Enable_Scheduling
 *	Enable global code motion. sets both GCM_PRE_Enable_Scheduling and
 *	GCM_POST_Enable_Scheduling
 *
 *  GCM_Motion_Across_Calls
 *	Enable code motion across procedure calls. 
 *
 *  GCM_Eager_Ptr_Deref
 *	Allow speculation among two pointer references varied by a constant
 *	offset.
 *
 *  GCM_Speculative_Loads
 *	Convert loads to safe speculative form and allow control speculation. 
 *
 *  GCM_Predicated_Loads
 *	Convert loads to safe speculative form by guarding it using a 
 *      qualifying predicate.
 *
 *  GCM_Min_Reg_Usage
 *	Allow code movement with emphasis on minimizing register usages.
 *
 *  GCM_Pointer_Spec
 *	Controls both GCM_Eager_Ptr_Deref and GCM_Null_Ptr_Deref	
 *
 *  IGLS_Enable_All_Scheduling
 *    Enables all the scheduling phases in CG (i.e LOCS, HBS and GCM).
 *
 *  CGTARG_Enable_Brlikely
 *	Enable the generation of branch-likely instructions.. 
 *
 *  Enable_Fill_Delay_Slots
 *      Enable filling of branch delay slots with an instruction from
 *	either one of the successor blocks (GCM) or reorder 
 *	instructions within a basic block to fill the delay slot (LOCS)
 *
 *  GCM_Enable_Fill_Delay_Slots
 *      Enable filling of branch delay slots with an instruction from
 *	either one of the successor blocks. is controlled by 
 *	Enable_Fill_Delay_Slots
 *
 *  CGEXP_use_copyfcc
 *	Use copyfcc pseudo-op in EXP_copy rather than generating a sequence.
 *
 *  CGEXP_expandconstant
 *	Specify maximum number of instruction a constant can generate
 *	before puting it in .rodata
 *
 *  CGEXP_normalize_logical
 *	Normalize the input of logical or/and/not to 0/1 (default true)
 *
 *  CGEXP_gp_prolog_call_shared
 *	Generate a shorter gp-prolog sequence for a CPIC compile. Emit
 *	a lui/addiu sequence instead of lui/addiu/addu (default false)
 *
 *  CGEXP_cvrt_int_div_to_mult
 *	Generate a multiply-upper sequence of operations in place of
 *	integer divide when the divisor is a compile time constant.
 *
 *  CGEXP_cvrt_int_div_to_fdiv
 *	Generate a floating divide operation in place of a 32 bit
 *	integer divide.
 *
 *  CGEXP_fast_imul
 *	Generate an alternative sequence for integer multiplies
 *	where possible instead of the straight forward translation.
 *
 *  CGEXP_float_consts_from_ints
 *	If an architecture has adequate integer load immediate instructions,
 *	use them to construct floating point constants and move to the
 *	floating point unit. Otherwise use the default method, e.g. load
 *	from memory.
 *
 *  EMIT_pjump_all
 *	Generate a PJUMP relocation for all calls, not just the 
 *	non-preemptible ones. This allows ld to change jalr to jal
 *	for calls that it can determine are non-preemptible, even
 *	though CG cannot tell. This change can be enabled by default
 *	only when the corresponding ld change is done. Defer making
 *	it default till 7.3 to prevent problems with 7.2 objects
 *	being linked with an earlier ld (default false)
 *
 *  LRA_do_reorder
 *	Allow lra to reorder instructions in order to minimize register
 *	lifetimes (default false)
 *
 *  INT32 CG_branch_mispredict_penalty;
 *	The average number of cycles lost when a branch is mispredicted
 *
 *  INT32 CG_branch_mispredict_factor;
 *	The misprediction rate may not correlate to the branch probability.
 *	A factor (0-100) will scale the CG_branch_mispredict_penalty;
 *
 *  EMIT_use_cold_section
 *	Put code region BBs into .text.cold. Turn off for debugging...
 *	(default true)
 *
 *  HB_formation
 *	Perform hyperblock formation.
 *
 *  HB_max_blocks
 *	Maximum number of blocks allowable in a hyperblock (default setting
 *	is architecturally dependent).
 *
 *  HB_max_sched_growth
 *	Multiplier used to determine the maximum increase in schedule height
 *	over that of the priority path that a path can cause when added to
 *	a hyperblock.
 *
 *  HB_min_path_priority_ratio
 *	The minimum ratio a path can have relative to the last path added 
 *	and still be added to the hyperblock.
 *
 *  HB_min_priority
 *	Minimum priority that a hyperblock can have.
 *
 *  HB_call_hazard_multiplier
 *      The factor by which to reduce the priority of a path that contains
 *	a call.
 *
 *  HB_memory_hazard_multiplier
 *      The factor by which to reduce the priority of a path that contains
 *	an unresolvable memory store.
 *
 *  HB_base_probability_contribution
 *	Factor to ensure base contribution of path probability to priority.
 *
 *  HB_require_alias
 *	Require that alias information be present for complex hyperblock
 *	formation.
 *
 *  HB_loops
 *	Perform aggressive hyperblock formation for loops.
 *
 *  HB_simple_ifc, HB_simple_ifc_set
 *	Perform simple, always profitable full if-conversion.
 *
 *  HB_complex_non_loop
 *	Perform hyperblock formation for complex control flow structures
 *	outside of loops.
 *
 *  HB_loops_with_exits
 *	Perform aggressive hyperblock formation for loops with exits.
 *
 *  HB_general_use_pq 
 *	Use priority queue to order processing of side paths when forming
 *	general candidate regions during hyperblock formation.
 *
 *  HB_general_from_top
 *	Prefer side paths near the top of the main path when forming
 *	general candidate regions during hyperblock formation.
 *
 *  HB_min_blocks
 *	Minimum number of blocks considered for a hyperblock.
 *
 *  HB_allow_tail_duplication
 *      Allows tail-duplication to be performed on side-entrances
 *      creating more aggressive hyperblock formation opportunities.
 *
 *  HB_exclude_calls
 *      Flag to disallow blocks with calls as part of hyperblock
 *      formation.
 *
 *  HB_exclude_pgtns
 *      Flag to disallow hyperblocks if it contains any global predicate TNs
 *      (PGTNS). The PQS framework relies on TNs having a unique definition, 
 *      not live-into and live-out of the hyperblock. However, some global 
 *	PGTNS do exist which satisfy this property. Allow forming HBs under
 *      this condition. The flag, if turned ON, will diasallow the formation.
 *
 *  GRA_LIVE_Predicate_Aware
 *      Allow GRA_LIVE computation to take advantage of predicate query data if available
 *
 *  EMIT_interface_section
 *	Emit interface section (default true).
 *
 * =======================================================================
 * =======================================================================
 */

#ifndef cg_flags_INCLUDED
#define cg_flags_INCLUDED


extern BOOL CG_warn_bad_freqs;
extern BOOL CG_enable_loop_optimizations;
extern INT32 CG_skip_after;
extern INT32 CG_skip_before;
extern INT32 CG_skip_equal;
extern INT32 CG_local_skip_after;
extern INT32 CG_local_skip_before;
extern INT32 CG_local_skip_equal;
extern BOOL CG_skip_local_hbf;
extern BOOL CG_skip_local_loop;
extern BOOL CG_skip_local_sched;
extern BOOL CG_skip_local_swp;
extern INT CG_opt_level;
extern BOOL CG_localize_tns;
extern BOOL CG_localize_tns_Set;
extern BOOL LOCALIZE_using_stacked_regs;
extern BOOL CG_tail_call;
extern BOOL CG_unique_exit;
extern BOOL CG_cond_defs_allowed;
extern BOOL CG_enable_feedback;
extern BOOL CG_enable_reverse_if_conversion;
extern BOOL CG_enable_reverse_if_conversion_overridden;
extern BOOL CG_enable_thr;
extern BOOL CG_enable_spec_imul;
extern BOOL CG_enable_spec_idiv;
extern BOOL CG_enable_spec_fdiv;
extern BOOL CG_enable_spec_fsqrt;
extern BOOL CG_enable_spec_imul_overridden;
extern BOOL CG_enable_spec_idiv_overridden;
extern BOOL CG_enable_spec_fdiv_overridden;
extern BOOL CG_enable_spec_fsqrt_overridden;
extern BOOL CG_create_madds;

#define CG_maxinss_default 100
extern INT32 CG_maxinss;
extern INT32 CG_maxblocks;
extern BOOL GRA_LIVE_Phase_Invoked;

/* CFLOW: 
 */
extern BOOL CFLOW_Enable;
extern BOOL CFLOW_opt_before_cgprep;
extern BOOL CFLOW_opt_after_cgprep;
extern BOOL CFLOW_Enable_Unreachable;
extern BOOL CFLOW_Enable_Branch;
extern BOOL CFLOW_Enable_Merge;
extern BOOL CFLOW_Enable_Reorder;
extern BOOL CFLOW_Enable_Freq_Order;
extern BOOL CFLOW_Enable_Clone;
extern BOOL CFLOW_opt_all_br_to_bcond;
extern const char *CFLOW_heuristic_tolerance;
extern const char *CFLOW_feedback_tolerance;
extern UINT32 CFLOW_clone_incr;
extern UINT32 CFLOW_clone_max_incr;
extern UINT32 CFLOW_clone_min_incr;
extern const char *CFLOW_cold_threshold;

/* FREQ:
 */
extern BOOL FREQ_enable;
extern BOOL FREQ_view_cfg;
extern const char *FREQ_frequent_never_ratio;
extern const char *FREQ_eh_freq;

extern BOOL CG_enable_rename;

/* Prefetch and load latency */

extern BOOL CG_enable_prefetch;
extern BOOL CG_enable_z_conf_prefetch;
extern BOOL CG_enable_nz_conf_prefetch;
extern BOOL CG_enable_pf_L1_ld;
extern BOOL CG_enable_pf_L1_st;
extern BOOL CG_enable_pf_L2_ld;
extern BOOL CG_enable_pf_L2_st;
extern BOOL CG_exclusive_prefetch;

extern INT32 CG_L1_ld_latency;
extern INT32 CG_L2_ld_latency;
extern INT32 CG_z_conf_L1_ld_latency;
extern INT32 CG_z_conf_L2_ld_latency;
extern INT32 CG_ld_latency;
extern INT32 CG_L1_pf_latency;
extern INT32 CG_L2_pf_latency;

extern BOOL CGSPILL_Rematerialize_Constants;
extern BOOL CGSPILL_Enable_Force_Rematerialization;

/* GCM, LOCS and IGLS */

extern BOOL LOCS_PRE_Enable_Scheduling;
extern BOOL LOCS_POST_Enable_Scheduling;
extern BOOL LOCS_Enable_Bundle_Formation;
extern BOOL LOCS_Enable_Scheduling;
extern BOOL GCM_Enable_Scheduling;
extern BOOL GCM_PRE_Enable_Scheduling;
extern BOOL GCM_POST_Enable_Scheduling;
extern BOOL GCM_Motion_Across_Calls;
extern BOOL GCM_Min_Reg_Usage;
extern BOOL GCM_Pointer_Spec;
extern BOOL GCM_Eager_Ptr_Deref;
extern BOOL GCM_Speculative_Loads;
extern BOOL GCM_Predicated_Loads;
extern BOOL GCM_Test;
extern BOOL CGTARG_Enable_Brlikely;
extern BOOL Enable_Fill_Delay_Slots;
extern BOOL GCM_Enable_Fill_Delay_Slots;
extern BOOL GCM_Enable_Cflow;
extern const char *CGTARG_Branch_Taken_Prob;
extern double CGTARG_Branch_Taken_Probability;
extern BOOL CGTARG_Branch_Taken_Prob_overridden;
extern BOOL IGLS_Enable_PRE_HB_Scheduling;
extern BOOL IGLS_Enable_POST_HB_Scheduling;
extern BOOL IGLS_Enable_HB_Scheduling;
extern BOOL IGLS_Enable_All_Scheduling;

extern BOOL EMIT_pjump_all;
extern BOOL EMIT_use_cold_section;
extern BOOL EMIT_interface_section;

extern INT32 EMIT_Long_Branch_Limit;	/* max distance (in bytes) for branches */
extern BOOL EMIT_stop_bits_for_asm;
extern BOOL EMIT_stop_bits_for_volatile_asm;
extern BOOL EMIT_explicit_bundles;

extern INT32 CGEXP_expandconstant;	/* maximum # instructions to expand constants */
#define DEFAULT_CGEXP_CONSTANT	3

extern BOOL CGEXP_use_copyfcc;
extern BOOL CGEXP_normalize_logical;
extern BOOL CGEXP_gp_prolog_call_shared;
extern BOOL CGEXP_fast_imul;
extern BOOL CGEXP_float_consts_from_ints;
extern BOOL CGEXP_cvrt_int_div_to_mult;
extern BOOL CGEXP_cvrt_int_div_to_fdiv;
extern BOOL CGEXP_opt_float_div_by_const;

// 10/17/00: these are temporary for osprey tuning -- remove later
extern const char *CGEXP_lfhint_L1;
extern const char *CGEXP_lfhint_L2;
extern const char *CGEXP_ldhint_L1;
extern const char *CGEXP_ldhint_L2;
extern const char *CGEXP_sthint_L1;
extern const char *CGEXP_sthint_L2;

extern BOOL LRA_do_reorder;

extern BOOL GRA_use_old_conflict;
extern BOOL GRA_shrink_wrap;
extern BOOL GRA_loop_splitting;
extern BOOL GRA_home;
extern BOOL GRA_remove_spills;
extern BOOL GRA_preference_globals;
extern BOOL GRA_preference_dedicated;
extern BOOL GRA_preference_glue;
extern BOOL GRA_preference_all;
extern BOOL GRA_ensure_spill_proximity;
extern BOOL GRA_choose_best_split;
extern BOOL GRA_use_stacked_regs;
extern BOOL GRA_redo_liveness;
extern BOOL GRA_recalc_liveness;
extern INT32 GRA_non_home_hi;
extern INT32 GRA_non_home_lo;
extern const char* GRA_call_split_freq_string;
extern const char* GRA_spill_count_factor_string;

extern BOOL  HB_formation;
extern BOOL  HB_static_freq_heuristics;
extern const char* HB_call_hazard_multiplier;
extern const char* HB_memory_hazard_multiplier;
extern const char* HB_min_path_priority_ratio;
extern const char* HB_base_probability_contribution;
extern INT   HB_max_blocks;
extern const char* HB_min_priority;
extern const char* HB_max_sched_growth;
extern BOOL  HB_require_alias;
extern BOOL  HB_loops;
extern BOOL  HB_loops_with_exits;
extern BOOL  HB_complex_non_loop;
extern BOOL  HB_simple_ifc;
extern BOOL  HB_simple_ifc_set;
extern BOOL  HB_general_use_pq;
extern BOOL  HB_general_from_top;
extern INT   HB_min_blocks;
extern BOOL  HB_allow_tail_duplication;
extern BOOL  HB_exclude_calls;
extern BOOL  HB_exclude_pgtns;
extern BOOL  HB_skip_hammocks;
extern BOOL  GRA_LIVE_Predicate_Aware;

extern BOOL Use_Page_Zero;  /* set bit in object to allow use of page 0 */

extern INT32 CG_branch_mispredict_penalty;
extern INT32 CG_branch_mispredict_factor;

//
//  Architecturally dependent flags.
//
extern BOOL HB_convert_loops_with_exits;
extern BOOL HB_complex_non_loop_if_conversion;


/* Recurrence breaking flags */
extern BOOL CG_LOOP_fix_recurrences;
extern BOOL CG_LOOP_fix_recurrences_specified;
extern BOOL CG_LOOP_back_substitution;
extern BOOL CG_LOOP_back_substitution_specified;
extern BOOL CG_LOOP_back_substitution_variant;
extern BOOL CG_LOOP_back_substitution_variant_specified;
extern BOOL CG_LOOP_interleave_reductions;
extern BOOL CG_LOOP_interleave_reductions_specified;
extern BOOL CG_LOOP_interleave_posti;
extern BOOL CG_LOOP_interleave_posti_specified;
extern BOOL CG_LOOP_reassociate;
extern BOOL CG_LOOP_reassociate_specified;
extern INT32 CG_LOOP_recurrence_min_omega;

// temporary flags for controlling algorithm selection for fdiv, sqrt, etc
extern const char *CGEXP_fdiv_algorithm;
extern const char *CGEXP_sqrt_algorithm;

#endif /* cg_flags_INCLUDED */
