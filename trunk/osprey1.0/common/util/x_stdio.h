/* $Id: x_stdio.h,v 1.1 2003-11-04 15:59:37 eraxxon Exp $ */
/* -*-Mode: C;-*- */
/* * BeginRiceCopyright *****************************************************
 * 
 * ******************************************************* EndRiceCopyright */

/* ====================================================================
 * ====================================================================
 *
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/common/util/x_stdio.h,v $
 * $Revision: 1.1 $
 * $Date: 2003-11-04 15:59:37 $
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
 * ====================================================================
 */

#ifndef x_stdio_h
#define x_stdio_h

/*************************** System Include Files ***************************/

#include <stdio.h>

/**************************** User Include Files ****************************/

/*************************** Forward Declarations ***************************/

/****************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

  /* Unix */
  int ux_fileno(FILE *stream);
  
  /* Unix */
  FILE *ux_fdopen(int fildes, const char *mode);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif
