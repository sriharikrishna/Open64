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


#ifndef defs_INCLUDED
#define defs_INCLUDED
/* ====================================================================
 * ====================================================================
 *
 * Module: defs.h
 * $Revision: 1.9 $
 * $Date: 2004-01-13 20:36:53 $
 *
 * Revision history:
 *  21-Aug-89 - Original Version
 *  24-Jan-91 - Copied for TP/Muse
 *
 * Description:
 *
 * This header file contains definitions of ubiquitous configuration
 * parameters used in the microcode compiler, in particular mappings
 * of the host-independent type names used to the host-supported basic
 * types.
 *
 * ====================================================================
 * ====================================================================
 */



#define PROTOTYPES 1
/* We always want Insist error messages: */
#define Insist_On 1

/* ====================================================================
 *
 * System libraries to be made generally available
 *
 * ====================================================================
 */

/* Make stdio, and string support generally available: */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <inttypes.h> /* portable definition of basic types */

#ifdef __cplusplus
extern "C" {
#endif


/* Since <signal.h> doesn't prototype signal, upsetting ccom: */
/* extern void (*signal(int, void (*) (int, ...)))(int, ...); */


/* ====================================================================
 *
 * Characterize the host machine
 *
 * ====================================================================
 */


/***** Note the size of a word (in malloc units): *****/
#ifndef HOST_WORD_SIZE
# define HOST_WORD_SIZE	4
#endif
#define WORDSIZE HOST_WORD_SIZE	/* OBSOLETE */
#if HOST_WORD_SIZE == 2
# define TWO_BYTE_WORDS
#endif
#if HOST_WORD_SIZE == 4
# define FOUR_BYTE_WORDS
#endif
#if HOST_WORD_SIZE == 8
# define EIGHT_BYTE_WORDS
#endif

/* Map low indices to low-order bits in the bit vector package: */
#define BV_LITTLE_ENDIAN_BIT_NUMBERING	1

/* Should bit vector packages use table lookup instead of shifts? */
#if 0
#define BV_MEMORY_BIT_MASKS	/* when shifting is slow */
#endif

/* ====================================================================
 *
 * The compiler process being compiled
 *
 * The command line must specify one of -DDRIVER, -DFRONT_END_xxx, or
 * -DBACK_END.  The following sets related defines based on those.
 *
 * NOTE: The original Josie sources sometimes used aliases C_FRONT_END
 * for FRONT_END_C, FETYPE_CH for F77_FRONT_END, COMMON_CORE for
 * BACK_END, and ONE_PROC for SINGLE_PROCESS.  These have all been
 * eliminated except for uses of FETYPE_CH specifically to refer to
 * the FORTRAN front end's CHARACTER type.
 *
 * ====================================================================
 */

#define	COMPILER_DRIVER	1
#define	COMPILER_FE_C	2
#define	COMPILER_FE_CC	3
#define	COMPILER_FE_F77	4
#define	COMPILER_FE_F90	5
#define COMPILER_BE	6
#define COMPILER_TDGEN	7

#ifdef DRIVER
# define COMPILER_PROCESS COMPILER_DRIVER
# define COMPILER 1
#endif
#if defined(FRONT_END_C)
# define COMPILER_PROCESS COMPILER_FE_C
# define FRONT_END 1
# define COMPILER 1
#endif
#if defined(FRONT_END_CPLUSPLUS)
# define COMPILER_PROCESS COMPILER_FE_CC
# define FRONT_END 1
# define COMPILER 1
#endif
#ifdef FRONT_END_F77
# define COMPILER_PROCESS COMPILER_FE_F77
# define FRONT_END_FORTRAN 1
# define FRONT_END 1
# define COMPILER 1
#endif
#ifdef FRONT_END_F90
# define COMPILER_PROCESS COMPILER_FE_F90
# define FRONT_END_FORTRAN 1
# define FRONT_END 1
# define COMPILER 1
#endif
#ifdef BACK_END
# define COMPILER_PROCESS COMPILER_BE
# define COMPILER 1
#endif
#ifdef TDGEN
# define COMPILER_PROCESS COMPILER_TDGEN
#endif

/* Are we compiling front end and back end as a single process? */
#if defined(FRONT_END) && defined(BACK_END)
# define SINGLE_PROCESS 1
#endif


/* Allow inline keyword, making it 'static' for debugging if Is_True_On */
#ifdef _LANGUAGE_C
#ifndef inline
#ifdef Is_True_On
#define inline static
#else
/*
 * Solaris CC workaround
 * Solaris doesn't define __inline
 */
/*** #define inline static __inline ***/
#define inline static
#endif
#endif
#endif


/* ====================================================================
 *
 * Type mapping
 *
 * The following type names are to be used in general to avoid host
 * dependencies.  Two sets are provided.  The first, without a prefix,
 * specifies a minimum bit length for the object being defined of 8,
 * 16, or 32 bits.  It is to be interpreted as a host-efficient type
 * of at least that size.  The second, with a "m" prefix, also gives
 * a minimum bit length, but that bit length is preferred (minimizing
 * memory usage is the priority) if remotely reasonable on the host.
 * The latter types should be used only for objects which will be
 * replicated extensively.
 *
 * ====================================================================
 */

typedef signed int	INT;	/* The natural integer on the host */
typedef signed int	INT8;	/* Use the natural integer */
typedef signed int	INT16;	/* Use the natural integer */
typedef signed int	INT32;	/* The natural integer matches */
typedef signed long long INT64;	
typedef unsigned long	INTPTR;	/* Integer the same size as pointer*/
typedef unsigned int	UINT;	/* The natural integer on the host */
typedef unsigned int	UINT8;	/* Use the natural integer */
typedef unsigned int	UINT16;	/* Use the natural integer */
typedef unsigned int	UINT32;	/* The natural integer matches */
typedef unsigned long long UINT64;
typedef int		BOOL;	/* Natural size Boolean value */
typedef signed char	mINT8;	/* Avoid - often very inefficient */
typedef signed short	mINT16;	/* Use a 16-bit integer */
typedef signed int	mINT32;	/* The natural integer matches */
typedef signed long long mINT64;
typedef unsigned char	mUINT8;	/* Use the natural integer */
typedef unsigned short	mUINT16;/* Use a 16-bit integer */
typedef unsigned int	mUINT32;/* The natural integer matches */
typedef unsigned long long mUINT64;
typedef unsigned char	mBOOL;	/* Minimal size Boolean value */

/* Define largest efficient integers for the host machine: */
typedef signed long	INTSC;	/* Scaled integer */
typedef unsigned long	UINTSC;	/* Scaled integer */

/* Define pointer-sized integers for the host machine: */
typedef signed long	INTPS;	/* Pointer-sized integer */
typedef unsigned long	UINTPS;	/* Pointer-sized integer */

/* Provide some limits that match the above types */

/* The following are defined in <inttypes.h> but may not be visible in C++ */

#if defined(__CYGWIN__) /* FIXME: remove eventually */
# undef INT64_MAX /* force redefinition because system headers erroneously */
# undef INT64_MIN /*   do not include appropriate 'll' suffix */
#endif

#ifndef INT8_MAX
# define INT8_MAX	127		/* Max 8-bit int */
#endif
#ifndef INT8_MIN
# define INT8_MIN	(-127)		/* Min 8-bit int */
#endif
#ifndef UINT8_MAX
# define UINT8_MAX	255u		/* Max 8-bit unsigned int */
#endif
#ifndef INT16_MAX
# define INT16_MAX	32767		/* Max 16-bit int */
#endif
#ifndef INT16_MIN
# define INT16_MIN	(-32768)	/* Min 16-bit int */
#endif
#ifndef UINT16_MAX
# define UINT16_MAX	65535u		/* Max 16-bit unsigned int */
#endif
#ifndef INT32_MAX
# define INT32_MAX	2147483647	/* Max 32-bit int */
#endif
#ifndef INT32_MIN
# define INT32_MIN	(-2147483647-1)	/* Min 32-bit int */
#endif
#ifndef UINT32_MAX
# define UINT32_MAX	4294967295u	/* Max 32-bit unsigned int */
#endif
#ifndef INT64_MAX
# define INT64_MAX	0x7fffffffffffffffll	/* Max 64-bit int */
#endif
#ifndef INT64_MIN
# define INT64_MIN	0x8000000000000000ll	/* Min 64-bit int */
#endif
#ifndef UINT64_MAX
# define UINT64_MAX	0xffffffffffffffffull	/* Max 64-bit unsigned int */
#endif


#define	INTSC_MAX	INT32_MAX	/* Max scaled int */
#define	INTSC_MIN	INT32_MIN	/* Min scaled int */
#define	UINTSC_MAX	UINT32_MAX	/* Max scaled uint */
#define	INTPS_MAX	INT32_MAX	/* Max pointer-sized int */
#define	INTPS_MIN	INT32_MIN	/* Min pointer-sized int */
#define	UINTPS_MAX	UINT32_MAX	/* Max pointer-sized uint */



/* Define quad-precision floating point for the host machine.
 * WARNING: Depending on the host, this type need not be usable.
 * Instead, see QUAD_TYPE in targ_const.h and its reference to
 * HOST_SUPPORTS_QUAD_FLOAT in config_host.h.  We do this this way to
 * avoid needing to allow the standard type names whenever
 * targ_const.h is included.
 */

/* Solaris CC workaround (FIXME)
 * if not define HOST_SUPPORTS_QUAD_FLOAT, later in be/com/emulate.cxx, 
 * the CC compiler will complain that 'long double' cannot be converted
 * into QUAD_TYPE. So I added defined(_SOLARIS_SOLARIS)
 */
#if (defined(_COMPILER_VERSION) && (_COMPILER_VERSION >= 400) && _SGIAPI) \
    || defined(__GNUC__) || defined(_SOLARIS_SOLARIS) || defined(__alpha)
# define HOST_SUPPORTS_QUAD_FLOAT 1
#else
# define HOST_SUPPORTS_QUAD_FLOAT 0
#endif

#if HOST_SUPPORTS_QUAD_FLOAT
  /* Temporarily remove this to get rid of warnings: */
  typedef long double	QUADFP;		/* 128-bit floating point */
#else 
  typedef double	QUADFP;		/* 128-bit floating point */
#endif


/* ==================================================================== */

/* A generic memory pointer type, e.g. for use in the memory
 * allocation routines. */
typedef void *MEM_PTR;

/* Short hand for those who don't like "char *" */
typedef char *STRING;

/* Define the target's basic INT type: */
/* WARNING:  This isn't quite accurate.  A single compiler may
 * support multiple targets with multiple possibilities for these
 * types.  They should be viewed as maximal for the supported
 * targets.
 * TODO:  They aren't there yet (e.g. the 64-bit targets).
 * Determine how to deal with this -- maybe a completely different
 * mechanism is required.
 */
typedef INT64	TARG_INT;	/* Individual objects */
typedef mINT64	mTARG_INT;	/* Table components */
typedef UINT64  TARG_UINT;
typedef mUINT64 mTARG_UINT;

/* Define standard values: */
#ifndef TRUE
# define TRUE	((BOOL) 1)
#endif
#ifndef FALSE
# define FALSE	((BOOL) 0)
#endif

/* Something to use to represent undefined positive integers.  Perahps we
 * could generalize this somehow, but it is useful the way it is.
 */
#ifndef UNDEFINED
# define UNDEFINED -1
#endif

/* Finally, eliminate the standard type names to prevent their
 * accidental use.  We must, however, allow this to be overridden for
 * files which need them, e.g. to use /usr/include expansions which
 * require them.
 */
#if 0 /* !defined(USE_STANDARD_TYPES) && !defined(_NEW_SYMTAB) */
# define short	SYNTAX_ERROR_short
# define int	SYNTAX_ERROR_int
# define long	SYNTAX_ERROR_long
#endif /* USE_STANDARD_TYPES */



/* ====================================================================
 *
 * Ubiquitous compiler types and macros
 *
 * We want several compiler standard types to be universally available
 * to allow their use without including their natural defining header
 * files.
 *
 * ====================================================================
 */

/* Define the general TDT table index type: */
typedef UINT16	CLASS_INDEX;	/* Individual objects */
typedef mUINT16	mCLASS_INDEX;	/* Table components */

/* Define the PREG offset type: */
typedef INT32 PREG_NUM;		/* Individual objects */
typedef mINT32 mPREG_NUM;	/* Table components */

/* Define the IDTYPE used by wopt */
typedef mUINT32 IDTYPE;

/* Define standard functions: */
#ifdef MAX
# undef MAX
#endif
#ifdef MIN
# undef MIN
#endif
#define MAX(a,b)	((a>=b)?a:b)
#define MIN(a,b)	((a<=b)?a:b)

inline INT Max(INT i, INT j)
{
  return MAX(i,j);
}
inline INT Min(INT i, INT j)
{
  return MIN(i,j);
}


/* --------------------------------------------------------------------
 * VERY_BAD_PTR
 * a memory address that is almost certainly going to cause 
 * a SIG. Many pointer values are initialized to this value to
 * catch any uninitialized uses of that pointer.
 *
 * Actually this could be host dependent, but the value below 
 * is good enough for all current hosts
 * --------------------------------------------------------------------
 */
#define VERY_BAD_PTR (0xfffffff)



#ifdef __cplusplus
}
#endif

#endif /* defs_INCLUDED */
