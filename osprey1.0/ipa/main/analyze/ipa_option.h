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
 * Module: ipa_option.h
 * $Revision: 1.1.1.1 $
 * $Date: 2002-05-22 20:09:09 $
 * $Author: dsystem $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/ipa/main/analyze/ipa_option.h,v $
 *
 * Revision history:
 *  08-Jun-95 - Original Version
 *  11-Apr-96 - Removed most option variables to common/com/config_ipa.
 *
 * Description:
 *  List of global variables set by the -IPA option group.
 *  See also common/com/config_ipa.h.
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef ipa_option_INCLUDED
#define ipa_option_INCLUDED

#include "config_ipa.h"		/* For -INLINE/-IPA group options */

#ifndef ipa_trace_INCLUDED
#include "ipa_trace.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _LIGHTWEIGHT_INLINER
#ifndef __DEM_H__
#include "ld_dem.h"		    /* for C++ demangler */
#endif
#endif // _LIGHTWEIGHT_INLINER

#ifndef ipc_weak_INCLUDED
#include "ipc_weak.h"		    /* make demangle() weak extern */
#endif

struct skiplist;

extern BOOL IPA_Enable_Memtrace;    /* Memory trace */

extern struct skiplist *IPA_Skip_List;	/* List of skip options */

extern BOOL Trace_IPA;		    /* Main IPA progress trace */
extern BOOL Trace_Perf;		    /* performance tracing */
extern BOOL Trace_IPALNO;         /* trace IPA to LNO writing */ 

extern BOOL Verbose;
extern BOOL Demangle;		    /* demangle C++ identifiers in error msg */

extern BOOL ProMP_Listing;

#undef DEMANGLE
#define DEMANGLE(name) (name)
#if 0
#if (defined(_STANDALONE_INLINER) || defined(_LIGHTWEIGHT_INLINER))
#define DEMANGLE(name) (name)
#else
#define DEMANGLE(name) (always_demangle (name, (char) Demangle))
#endif /* __STANDALONE_INLINER */
#endif
    
extern void Process_IPA_Options ( INT argc, char **argv );

#ifdef __cplusplus
}
#endif

#endif /* ipa_option_INCLUDED */
