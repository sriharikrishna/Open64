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
 * Module: config_opt.cxx
 * $Revision: 1.2 $
 * $Date: 2002-07-12 16:48:28 $
 * $Author: fzhao $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/common/com/config_opt.cxx,v $
 *
 * Revision history:
 *  08-Sep-94 - Original Version (wodriver.c)
 *  15-Aug-96 - config_opt.c extracted from config.c
 *
 * Description:
 *
 * Configure the -OPT group (included in config.cxx).
 * Used from front ends, ipl, inline, ipa (ld), and be.
 *
 * ====================================================================
 * ====================================================================
 */

#ifdef _KEEP_RCS_ID
/*REFERENCED*/
static char *config_opt_rcs_id = "$Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/common/com/config_opt.cxx,v $ $Revision: 1.2 $";
#endif /* _KEEP_RCS_ID */

/* This file is included in config.c, so it doesn't need its own set of
 * standard includes -- only the following:
 */
#include "config_opt.h"
extern BOOL WOPT_Enable_Goto;	/* In case config_wopt.h is later */

/* ====================================================================
 *
 * Global flag variables which reflect the -OPT group options.
 *
 * ====================================================================
 */

/***** Optimization Warning Messages *****/
BOOL Show_OPT_Warnings = TRUE;          /* Display OPT warning messages */

/***** Aliasing control *****/
OPTION_LIST *Alias_Option = NULL;
BOOL Alias_Pointer_Parms = TRUE;        /* Parms ptr indep? */
BOOL Alias_Pointer_Cray = FALSE;        /* Cray pointer semantics? */
BOOL Alias_Pointer_Types = FALSE;	/* Ptrs to distinct basic types indep? */
BOOL Alias_Not_In_Union  = FALSE;	/* Ptrs point to non-union types */
BOOL Alias_Pointer_Strongly_Typed = FALSE; /* Ptrs to distinct types indep? */
BOOL Alias_Pointer_Named_Data = FALSE;	/* No pointers to named data? */
BOOL Alias_Pointer_Restricted = FALSE;	/* *p and *q not aliased? */
BOOL Alias_Pointer_Disjoint   = FALSE;	/* **p and **q not aliased? */
BOOL Alias_Common_Scalar = FALSE;       /* Distinguish scalar from array */
/* We will normally default Alias_Pointer_Types to TRUE, but can't
 * for K&R C.  This option is set by the driver for K&R C compilations
 * for use in overriding the default -- not intended for user use.
 */
static BOOL Alias_Pointer_Cckr = FALSE;	/* -cckr default rules? */
static BOOL Alias_Pointer_Types_Set = FALSE;	/* alias=typed set? */
static BOOL Alias_Not_In_Union_Set  = FALSE;	/* alias=nounion set? */
BOOL  Alias_F90_Pointer_Unaliased = FALSE;  /* Are F90 pointers unaliased? */

/***** Alignment control *****/
BOOL Align_Object = TRUE;	/* Try to improve alignment of objects */
BOOL Align_Padding = FALSE;	/* Pad objects to natural alignment */

/* These alignment options are only relevant in the back end: */
#ifdef BACK_END
# define ALIGN_INSTS	  &Align_Instructions
#else
# define ALIGN_INSTS	  &Ignore_Int
#endif

/***** Constant folding (simplifier) options *****/
BOOL Enable_Cfold_Aggressive = FALSE;	/* Complex constant folding? */
static BOOL Cfold_Aggr_Set = FALSE;	/* ... option seen? */
BOOL Enable_Cfold_Reassociate = FALSE;	/* Re-association allowed? */
static BOOL Cfold_Reassoc_Set = FALSE;	/* ... option seen? */
BOOL Enable_Cfold_Intrinsics = FALSE;	/* Intrinsic constant folding? */
BOOL Cfold_Intrinsics_Set = FALSE;	/* ... option seen? */
BOOL CIS_Allowed = TRUE;	/* combine sin(x) and cos(x) to cis(x) */
static BOOL CIS_Set = FALSE;	/* ... option seen? */
BOOL Enable_CVT_Opt= FALSE;	/* Remove useless convert operators */
BOOL Enable_CVT_Opt_Set= FALSE;	/* ... option seen? */
BOOL Optimize_CVTL_Exp = FALSE;	/* Optimize expansion of CVTL operators */
BOOL Div_Split_Allowed = FALSE;		/* change a/b --> a*1/b ? */
static BOOL Div_Split_Set = FALSE;	/* ... option seen? */
BOOL Fast_Exp_Allowed = FALSE;		/* Avoid exp() calls? */
static BOOL Fast_Exp_Set = FALSE;	/* ... option seen? */
BOOL Fast_IO_Allowed = FALSE;		/* Fast printf/scanf/printw ? */
static BOOL Fast_IO_Set = FALSE;	/* ... option seen? */
BOOL Fast_Sqrt_Allowed = FALSE;		/* sqrt(x) --> x * rsqrt(x) ? */
static BOOL Fast_Sqrt_Set = FALSE;	/* ... option seen? */
BOOL Rsqrt_Allowed = FALSE;		/* generate RSQRT instruction? */
static BOOL Rsqrt_Set = FALSE;		/* ... option seen? */
BOOL Recip_Allowed;		        /* generate RECIP instruction? */
static BOOL Recip_Set = FALSE;		/* ... option seen? */
BOOL Simp_Fold_Unsigned_Relops = FALSE; /* Constant fold unsigned relops */
static BOOL Simp_Fold_Unsigned_Relops_Set = FALSE;
/* Allow folding which might cause error if overflow occurs? */
BOOL Simp_Unsafe_Relops = FALSE;
static BOOL Simp_Unsafe_Relops_Set = FALSE;
BOOL Simp_Canonicalize = TRUE;          /* Simplifier canonicalization */
BOOL Enable_WN_Simp = TRUE;             /* Use the WHIRL simplifier */
static BOOL Enable_WN_Simp_Set=FALSE;   /* ... option seen? */
BOOL GCM_Eager_Null_Ptr_Deref = FALSE; /* allow speculation past NULL ptr
					  test. assumes page zero as
					  readable */
BOOL GCM_Eager_Null_Ptr_Deref_Set=FALSE;   /* ... option seen? */
BOOL GCM_Speculative_Ptr_Deref= TRUE;  /* allow load speculation of a memory
					  reference that differs by a small
					  offset from some reference location */
BOOL GCM_Speculative_Ptr_Deref_Set=FALSE;   /* ... option seen? */

/***** Limits on optimization *****/
#define DEFAULT_OLIMIT		3000
#define DEFAULT_O3_OLIMIT	4000	/* allow more time for -O3 compiles */
#define MAX_OLIMIT		INT32_MAX
INT32 Olimit = DEFAULT_OLIMIT;
static BOOL Olimit_Set = FALSE;
BOOL Olimit_opt = FALSE;	/* use regions? */
static BOOL Olimit_opt_Set = FALSE;

/* Debugging Flags for All Optimizations */
static OPTION_LIST *Opt_Skip = NULL;		/* Raw list */
SKIPLIST *Optimization_Skip_List = NULL;	/* Processed list */
static OPTION_LIST *Region_Skip = NULL;		/* Raw list */
SKIPLIST *Region_Skip_List = NULL;		/* Processed list */

/***** Miscellaneous -OPT: group options *****/
char *Ofast = NULL;		/* -OPT:Ofast platform name */
BOOL OPT_Pad_Common = FALSE;	/* Do internal common block padding? */
BOOL OPT_Reorg_Common = FALSE;	/* Do common block reorganization (split)? */
BOOL OPT_Reorg_Common_Set = FALSE;	/* ... option seen? */
BOOL OPT_Unroll_Analysis = TRUE;	/* Enable unroll limitations? */
BOOL OPT_Unroll_Analysis_Set = FALSE;	/* ... option seen? */
BOOL OPT_Lower_Speculate = FALSE;	/* speculate CAND/CIOR */
BOOL OPT_Lower_Treeheight = FALSE;	/* reassociate commutative ops */
static BOOL OPT_Lower_Treeheight_Set = FALSE;
BOOL OPT_Inline_Divide = TRUE;		/* inline divide sequences */
static BOOL OPT_Inline_Divide_Set = FALSE;
BOOL OPT_Space = FALSE;			/* various text space optimizations */
BOOL Early_MP_Processing = FALSE; /* Do mp lowerering before lno/preopt */
BOOL Implied_Do_Io_Opt = TRUE;	/* Do implied-do loop opt for I/O */
BOOL Cray_Ivdep=FALSE;		/* Use Cray meaning for Ivdep */
BOOL Liberal_Ivdep=FALSE;	/* Use liberal meaning for Ivdep */
BOOL Inhibit_EH_opt=FALSE;	/* Don't remove EH regions without calls */
BOOL Allow_wrap_around_opt = TRUE;
static BOOL Allow_wrap_around_opt_Set = FALSE;	/* ... option seen? */
BOOL Enable_GOT_Call_Conversion = FALSE;	/* %call16 -> %got_disp */
static BOOL Enable_GOT_Call_overridden = FALSE;	/* ... option seen? */
BOOL OPT_recompute_addr_flags = FALSE; /* recompute addr saved */
BOOL OPT_IPA_addr_analysis = TRUE; /* enable the use of IPA addr analysis result */
BOOL Delay_U64_Lowering = TRUE;/* Delay unsigned 64-bit lowering to after wopt */
BOOL OPT_shared_memory = TRUE;	// assume use of shared memory
/* Put each function in its own text section */
BOOL Section_For_Each_Function = FALSE;
BOOL Inline_Intrinsics_Early=FALSE;    /* Inline intrinsics just after VHO */
BOOL Enable_extract_compose=TRUE;     /* This is also forced off for MIPS and IA32 in
					  config_targ.cxx */
#ifdef __linux__
BOOL Enable_WFE_DFE = FALSE;
#endif /* __linux __ */


/***** Instrummentation Related Options *****/
INT32 Instrumentation_Phase_Num = 0;
BOOL Instrumentation_Enabled = FALSE;
UINT32 Instrumentation_Actions = 0;
BOOL Instrumentation_Unique_Output = FALSE; // always create unique output
OPTION_LIST *Feedback_Option = NULL;

/***** Obsolete options *****/
static BOOL Fprop_Limit_Set = FALSE;

/* ====================================================================
 *
 * Descriptor for the -OPT option group.
 *
 * ====================================================================
 */

/* Optimization options: */
static OPTION_DESC Options_OPT[] = {
  { OVK_BOOL, OV_VISIBLE,	TRUE,	"warning",		"warn",
    0, 0, 0,  &Show_OPT_Warnings,     NULL,
    "Control interpretation of possible variable aliasing" },

  { OVK_LIST,	OV_VISIBLE,	TRUE, 	"alias",		"alia",
    0, 0, 0,	&Alias_Option,	NULL,
    "Control interpretation of possible variable aliasing" },

  { OVK_INT32,	OV_SHY,		TRUE, "align_instructions",	"align_i",
    16, 0, 1024, ALIGN_INSTS,	NULL,
    "Align subprogram entries and loops by given byte count" },

  { OVK_BOOL,	OV_INTERNAL,	TRUE, "align_object",		"align_o",
    0, 0, 0,	&Align_Object,	NULL,
    "Allow realignment of objects to improve memory accesses" },

  { OVK_BOOL,	OV_INTERNAL,	TRUE, "align_padding",		"align_p",
    0, 0, 0,	&Align_Padding,	NULL,
    "Allow padding of objects to improve memory alignment" },

  { OVK_INT32,  OV_INTERNAL,	TRUE,	"bblength",		"bb",
    DEF_BBLENGTH, MIN_BBLENGTH, MAX_BBLENGTH, &Split_BB_Length, NULL,
    "Restrict BB length by splitting longer BBs" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE,	"cis",			"cis",
    0, 0, 0,	&CIS_Allowed,	 &CIS_Set,
    "Convert sin/cos pairs into a single call" },

  { OVK_INT32,  OV_VISIBLE,	TRUE,	"const_copy_limit",	"const",
    DEF_CONST_COPY_TN_CNT, 0, INT32_MAX,&Const_Copy_TN_CNT, NULL,
    "Avoid constant/copy propagation if there are more than n expressions" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "cray_ivdep",		"cray_ivdep",
    0, 0, 0,    &Cray_Ivdep,	NULL,
    "IVDEP pragma/directive break parallelism-inhibiting dependencies" },

  { OVK_BOOL,	OV_INTERNAL,	TRUE, "cvt",			"cvt",
    0, 0, 0,	&Enable_CVT_Opt,	&Enable_CVT_Opt_Set,
    "Optimize conversion operators" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "div_split",		"div_split",
    0, 0, 0,	&Div_Split_Allowed, &Div_Split_Set ,
    "Allow splitting of a/b into a*recip(b)" },

  { OVK_BOOL,	OV_INTERNAL,	TRUE, "early_mp",		"early_mp",
    0, 0, 0,	&Early_MP_Processing, NULL,
    "Lower before LNO" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "fast_bit_intrinsics",	"fast_bit",
    0, 0, 0,	&Fast_Bit_Allowed,	&Fast_Bit_Set,
    "Don't check bit count range for Fortran bit intrinsics" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "fast_complex",		"fast_co",
    0, 0, 0,	&Fast_Complex_Allowed,	&Fast_Complex_Set,
    "Use fast algorithms with limited domains for complex norm and divide" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "fast_exp",		"fast_ex",
    0, 0, 0,	&Fast_Exp_Allowed,	&Fast_Exp_Set,
    "Use multiplication and square root for exp() where faster" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "fast_io",		"fast_io",
    0, 0, 0,	&Fast_IO_Allowed,	&Fast_IO_Set,
    "Inline some C I/O routines if __INLINE_INTRINSICS is defined" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "fast_nint",		"fast_nint",
    0, 0, 0,	&Fast_NINT_Allowed,	&Fast_NINT_Set,
    "Use IEEE rounding instead of Fortran rounding for NINT intrinsics" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "fast_sqrt",		"fast_sq",
    0, 0, 0,	&Fast_Sqrt_Allowed,	&Fast_Sqrt_Set,
    "May use x*rsqrt(x) for sqrt(x) on machines where faster" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "fast_trunc",		"fast_trunc",
    0, 0, 0,	&Fast_trunc_Allowed,	&Fast_trunc_Set,
    "Inline NINT and related intrinsics with limited-domain algorithm" },

  { OVK_BOOL,	OV_SHY,		TRUE, "fold_aggressive",	"fold_ag",
    0, 0, 0,	&Enable_Cfold_Aggressive, &Cfold_Aggr_Set,
    "Allow aggressive expression folding optimizations" },

  { OVK_BOOL,	OV_SHY,		TRUE, "fold_intrinsics",	"fold_i",
    0, 0, 0,	&Enable_Cfold_Intrinsics, &Cfold_Intrinsics_Set,
    "Allow expression folding of Fortran intrinsic calls" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "fold_reassociate",	"fold_r",
    0, 0, 0,	&Enable_Cfold_Reassociate, &Cfold_Reassoc_Set,
    "Allow optimizations which reassociate floating point operators" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "fold_unsafe_relops",	"fold_unsafe_relops",
    0, 0, 0,	&Simp_Unsafe_Relops, &Simp_Unsafe_Relops_Set,
    "Allow relational operator folding which may cause integer overflow" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "fold_unsigned_relops",	"fold_unsigned",
    0, 0, 0,	&Simp_Fold_Unsigned_Relops, &Simp_Fold_Unsigned_Relops_Set,
    "Allow relop folding which may cause unsigned integer overflow" },

  { OVK_BOOL,   OV_VISIBLE,	TRUE, "got_call_conversion",  "got_call",
    0, 0, 0,    &Enable_GOT_Call_Conversion, &Enable_GOT_Call_overridden,
    "Allow function address loads to be moved out of loops" },

  { OVK_BOOL,	OV_INTERNAL,	TRUE, "goto_conversion",	"goto",
    0, 0, 0,	&WOPT_Enable_Goto, NULL,
    "Enable conversion of GOTO to more structured constructs" },

  { OVK_INT32,	OV_VISIBLE,	TRUE, "IEEE_arithmetic",	"IEEE_a",
    1, 1, 3,	&IEEE_Arithmetic, &IEEE_Arith_Set,
    "Level of conformance to IEEE-754 arithmetic rules" },

  { OVK_BOOL,	OV_SHY,		TRUE, "IEEE_comparisons",	"IEEE_c",
    0, 0, 0,	&Force_IEEE_Comparisons, NULL,
    "Force conforming operations on IEEE-754 NaN and Inf values" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "IEEE_NaN_Inf",		"IEEE_N",
    0, 0, 0,	&Force_IEEE_Comparisons, NULL,
    "Force conforming operations on IEEE-754 NaN and Inf values" },

  { OVK_BOOL,	OV_SHY,		TRUE, "implied_do_io_opt",	NULL,
    1, 0, 0,	&Implied_Do_Io_Opt, NULL,
    "Optimize implied DO I/O to minimize calls in Fortran" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "inline_intrinsics",	"inline_intr",
    0, 0, 0,	&Inline_Intrinsics_Allowed, &Inline_Intrinsics_Set,
    "Allow inlining of Fortran intrinsic functions" },

  { OVK_BOOL,	OV_SHY,		TRUE, "ldx",			"ldx",
    0, 0, 0,	&Indexed_Loads_Allowed,	NULL,
    "Allow generation of indexed load/store operations" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "liberal_ivdep",	"liberal_ivdep",
    0, 0, 0,    &Liberal_Ivdep, NULL,
    "IVDEP pragmas/directives break all dependencies within an array" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "Inhibit_EH_opt",	"Inhibit_EH_opt",
    0, 0, 0,    &Inhibit_EH_opt, NULL,
    "Don't remove EH regions without calls" },

  { OVK_BOOL,	OV_INTERNAL,	TRUE, "nary",			"nary",
    0, 0, 0,	&Enable_NaryExpr,	&Enable_NaryExpr_Set,
    "Allow N-ary tree height reduction of MADDs" },

  { OVK_NAME,	OV_SHY,		FALSE, "Ofast",			"Ofast",
    0, 0, 0,	&Ofast,		NULL,
    "Tailor options for performance on current target" },

  { OVK_INT32,	OV_VISIBLE,	FALSE, "Olimit",		"Ol",
    DEFAULT_OLIMIT, 0, MAX_OLIMIT,	&Olimit,	&Olimit_Set,
    "Limit size of subprograms which will be optimized" },

  { OVK_BOOL,	OV_INTERNAL,	FALSE, "Olimit_opt",		"Olimit_o",
    0, 0, 0,	&Olimit_opt,	&Olimit_opt_Set,
    "Use regions if Olimit exceeded" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "pad_common",		"pad",
    0, 0, 0,	&OPT_Pad_Common, NULL,
    "Force padding of COMMON blocks to improve cache behavior" },

  { OVK_BOOL,	OV_INTERNAL,	TRUE, "ptr_opt",		"ptr_o",
    0, 0, 0,	&Ptr_Opt_Allowed, NULL,
    "Allow treatment of pointers as arrays when possible in C" },

  { OVK_BOOL,	OV_INTERNAL,	FALSE, "rail",			"rail",
    0, 0, 0,	&Regions_Around_Inner_Loops, NULL,
    "Insert regions around inner loops" },

  { OVK_BOOL,	OV_INTERNAL,	FALSE, "rbi",			"rbi",
    0, 0, 0,	&Region_Boundary_Info, NULL,
    "Create region boundary information" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "recip",		"recip",
    0, 0, 0,	&Recip_Allowed, &Recip_Set,
    "Allow use of recip instruction" },

  { OVK_LIST,	OV_SHY,		FALSE, "region_skip_equal",	"region_skip_e",
    0, 0, 4096,	&Region_Skip,	NULL,
    "Skip optimization of this region" },

  { OVK_LIST,	OV_SHY,		FALSE, "region_skip_before",	"region_skip_b",
    0, 0, 4096,	&Region_Skip,	NULL,
    "Skip optimization of regions before this one" },

  { OVK_LIST,	OV_SHY,		FALSE, "region_skip_after",	"region_skip_a",
    0, 0, 4096,	&Region_Skip,	NULL,
    "Skip optimization of regions after this one" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "reorg_common",		"reorg",
    0, 0, 0,	&OPT_Reorg_Common, &OPT_Reorg_Common_Set,
    "Allow splitting of COMMON blocks to improve cache behavior" },

  { OVK_BOOL,	OV_VISIBLE,	FALSE, "procedure_reorder", "procedure_reorder",
    0, 0, 0,	&Section_For_Each_Function, NULL,
    "Place each function in its own .text section" },

  { OVK_INT32,  OV_VISIBLE,	TRUE, "roundoff",		"ro",
    2, 0, 3,	&Roundoff_Level, &Roundoff_Set,
    "Level of acceptable departure from source roundoff semantics" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "rsqrt",		"rsqrt",
    0, 0, 0,	&Rsqrt_Allowed, &Rsqrt_Set,
    "Allow use of rsqrt instruction" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "shared_memory",	"shared_mem",
    0, 0, 0,	&OPT_shared_memory, NULL,
    "Assume use of shared memory" },

  { OVK_LIST,	OV_SHY,		FALSE, "skip_equal",		"skip_e",
    0, 0, 4096,	&Opt_Skip,	NULL,
    "Skip optimization of this subprogram" },

  { OVK_LIST,	OV_SHY,		FALSE, "skip_before",		"skip_b",
    0, 0, 4096,	&Opt_Skip,	NULL,
    "Skip optimization of subprograms before this one" },

  { OVK_LIST,	OV_SHY,		FALSE, "skip_after",		"skip_a",
    0, 0, 4096,	&Opt_Skip,	NULL,
    "Skip optimization of subprograms after this one" },

  { OVK_BOOL,	OV_VISIBLE,	FALSE, "space",		"sp",
    0, 0, 0,    &OPT_Space,	NULL,
    "Bias optimizations to minimize code space" },

  { OVK_BOOL,	OV_INTERNAL,	TRUE, "speculate",		"",
    0, 0, 0,	&OPT_Lower_Speculate, NULL,
    "Allow speculation for CAND/COR operators" },

  { OVK_BOOL,   OV_INTERNAL,    TRUE, "speculative_null_ptr_deref","",
    0, 0, 0,    &GCM_Eager_Null_Ptr_Deref, &GCM_Eager_Null_Ptr_Deref_Set,
    "Allow speculation of loads above NULL pointer tests" },

  { OVK_BOOL,   OV_INTERNAL,    TRUE, "speculative_ptr_deref","",
    0, 0, 0,    &GCM_Speculative_Ptr_Deref, &GCM_Speculative_Ptr_Deref_Set,
    "Allow speculative loads of memory locations that differ by a small offset from some referenced memory location" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "swp",			"swp",
    0, 0, 0,    &Enable_SWP,	&Enable_SWP_overridden,
    "Enable software pipelining" },

  { OVK_BOOL,	OV_INTERNAL,	TRUE, "treeheight",		"",
    0, 0, 0,	&OPT_Lower_Treeheight, &OPT_Lower_Treeheight_Set,
    "Allow tree height reduction" },

  { OVK_BOOL,	OV_INTERNAL,	TRUE, "inline_divide",		"",
    0, 0, 0,	&OPT_Inline_Divide, &OPT_Inline_Divide_Set,
    "Inline divide and remainder operations if possible" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "unroll_analysis",	"unroll_analysis",
    0, 0, 0,	&OPT_Unroll_Analysis,	&OPT_Unroll_Analysis_Set,
    "Analyze inner loop requirements before unrolling" },

  { OVK_INT32,	OV_VISIBLE,	TRUE, "unroll_size",		"unroll_s",
    0, 0, INT32_MAX, &OPT_unroll_size, &OPT_unroll_size_overridden,
    "Maximum size of loops to be unrolled" },

  { OVK_INT32,	OV_VISIBLE,	TRUE, "unroll_times_max",	"unroll",
    0, 0, INT32_MAX, &OPT_unroll_times, &OPT_unroll_times_overridden,
    "Maximum number of times to unroll loops" },

  { OVK_BOOL,	OV_INTERNAL,	TRUE, "wn_simplify",		"wn_simp",
    0, 0, 0,	&Enable_WN_Simp, &Enable_WN_Simp_Set,
    "Enable simplifier" },

  { OVK_BOOL,	OV_VISIBLE,	TRUE, "wrap_around_unsafe_opt", "wrap_around_unsafe",
    0, 0, 0,	&Allow_wrap_around_opt,	&Allow_wrap_around_opt_Set,
    "Allow LFTR which may wrap around MAX_INT" },

  /* intrinsic expansion for bzero/blkclr/bcopy/memset/memcpy/memmove */
  {OVK_BOOL,    OV_VISIBLE,	TRUE,	"mem_intrinsics",       "",
    0, 0, 0,    &CG_mem_intrinsics, NULL,
    "Enable inline expansion of memory intrinsics (bzero, blkclr, bcopy, memset, memcpy, memmove)" },
  {OVK_INT32,   OV_VISIBLE,	TRUE,     "memmove_count",        "memmove",
    16, 0, INT32_MAX,   &CG_memmove_inst_count,&CG_memmove_inst_count_overridden,
    "Maximum size of inline expansion of memory intrinsics" },
  {OVK_BOOL,    OV_VISIBLE,	TRUE,     "bcopy_cannot_overlap",         "",
    0, 0, 0,    &CG_bcopy_cannot_overlap, NULL,
    "Assume that source and target of bcopy cannot overlap" },
  {OVK_BOOL,    OV_VISIBLE,	TRUE,     "memcpy_cannot_overlap",        "",
    0, 0, 0,    &CG_memcpy_cannot_overlap,      NULL,
    "Assume that source and target of memcpy cannot overlap" },
  {OVK_BOOL,    OV_VISIBLE,	TRUE,     "memmove_cannot_overlap", "",
    0, 0, 0,    &CG_memmove_cannot_overlap,     NULL,
    "Assume that source and target of memmove cannot overlap" },
  {OVK_BOOL,    OV_VISIBLE,	TRUE,	"memmove_nonconst",       "",
    0, 0, 0,    &CG_memmove_nonconst, NULL,
    "Enable inline expansion of memory intrinsics (bzero, blkclr, bcopy, memset, memcpy, memmove) whose size is not a constant" },

  { OVK_BOOL,	OV_INTERNAL,	TRUE, "ipa_addr_analysis", "ipa_addr",
    0, 0, 0,	&OPT_IPA_addr_analysis, NULL,
    "Enable the use of IPA address analysis result in the backend"},

  { OVK_BOOL,	OV_INTERNAL,	TRUE, "recompute_addr_flags", "",
    0, 0, 0,	&OPT_recompute_addr_flags, NULL,
    "Recompute address flags in the backend (for debugging)"},

  { OVK_INT32,  OV_VISIBLE,	TRUE, "instrument",		"instr",
    0, 0, 3,	&Instrumentation_Phase_Num, &Instrumentation_Enabled,
    "Phases in the compiler where instrumentation needs to be done" },

  { OVK_UINT32,  OV_INTERNAL,	TRUE, "instrument_action",		"",
    0, 0, UINT32_MAX,	&Instrumentation_Actions, &Instrumentation_Enabled,
    "Phases in the compiler where instrumentation needs to be done" },

  { OVK_BOOL,	OV_INTERNAL,	FALSE,	"instr_unique_output",	"",
    0, 0, 0,	&Instrumentation_Unique_Output,	NULL,
    "Always create a unique name for the profile data file" },

  { OVK_LIST,	OV_VISIBLE,	TRUE, 	"feedback",		"feed",
    0, 0, 0,	&Feedback_Option,	NULL,
    "Phases in the compiler where feedback needs to be done" },

  { OVK_BOOL,	OV_INTERNAL,	TRUE, "early_intrinsics",		"",
    0, 0, 0,	&Inline_Intrinsics_Early, NULL,
    "Lower intrinsics early" },

  { OVK_BOOL,	OV_INTERNAL,	TRUE, "delay_u64_lowering",	"delay_u64",
    0, 0, 0,	&Delay_U64_Lowering, NULL,
    "Delay unsigned 64-bit lowering to after wopt" },

  { OVK_BOOL,	OV_INTERNAL,	TRUE, "extract_deposit",	"extr",
    0, 0, 0,	&Enable_extract_compose,	NULL,
    "Enable use of extract/compose opcodes" },

  { OVK_BOOL, OV_VISIBLE,     FALSE, "ansi_setjmp",           "ansi_setjmp",
    0, 0, 0,  &LANG_Ansi_Setjmp_On,   &LANG_Ansi_Setjmp_Set,
    "C/C++: enable optimization of functions with calls to setjmp" },

#ifdef __linux__
  { OVK_BOOL,	OV_INTERNAL,	TRUE, "wfe_dfe",	"wfe_dfe",
    0, 0, 0,	&Enable_WFE_DFE,	NULL,
    "Enable dead function elimination in the frontend" },
#endif /* __linux__ */

  /* Obsolete options: */

  { OVK_OBSOLETE,	OV_INTERNAL,	FALSE, "global_limit",		NULL,
    0, 0, 0,		NULL,	NULL,	"" },

  { OVK_OBSOLETE,	OV_INTERNAL,	FALSE, "feopt",		NULL,
    0, 0, 0,		NULL,	NULL,
    "Enable special optimizations in front ends" },

  { OVK_OBSOLETE,	OV_VISIBLE,	FALSE, "fold_arith_limit",	NULL,
    0, 0, 0,		NULL,	 NULL,
    "Limit size of subexpressions to be folded" },

  { OVK_OBSOLETE,	OV_INTERNAL,	FALSE, "fold_float",		NULL,
    0, 0, 0,		NULL,	NULL,
    "Allow expression folding optimizations of floating point" },

  { OVK_OBSOLETE,	OV_INTERNAL,	FALSE, "fprop_limit",		NULL,
    0, 0, INT32_MAX,	NULL,	NULL,	"" },

  { OVK_COUNT }		/* List terminator -- must be last */
};
