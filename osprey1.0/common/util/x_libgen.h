/* $Id: x_libgen.h,v 1.1 2003-12-10 16:05:18 eraxxon Exp $ */
/* -*-Mode: C;-*- */
/* * BeginRiceCopyright *****************************************************
 * 
 * ******************************************************* EndRiceCopyright */

/* ====================================================================
 * ====================================================================
 *
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/common/util/x_libgen.h,v $
 * $Revision: 1.1 $
 * $Date: 2003-12-10 16:05:18 $
 *
 * Nathan Tallent.
 *
 * Description:
 *
 * Header file extensions.
 *
 * The ANSI/ISO C++ standard does not include every routine that is
 * legally available in ANSI/ISO C.  Moreover, neither C++ nor C
 * include every routine within the standard UNIX specification
 * (OpenGroup).  This file provides that extra functionality for
 * ANSI/ISO C++ and C code.
 * 
 * Routines available in C are prefixed by c_.
 * Routines available in Unix are prefixed by ux_
 *
 * ====================================================================
 * ==================================================================== */

#ifndef x_libgen_h
#define x_libgen_h

/*************************** System Include Files ***************************/

/* #include <libgen.h> */

/**************************** User Include Files ****************************/

/*************************** Forward Declarations ***************************/

/****************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

  /* NOTE: We provide our own implementations of these functions. */

  /* Unix */
  char* ux_basename(char *path);
  
  /* Unix */
  char* ux_dirname(char *path);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif
