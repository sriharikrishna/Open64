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
 * Module: ipc_option.h
 * $Revision: 1.1.1.1 $
 * $Date: 2002-05-22 20:09:06 $
 * $Author: dsystem $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/ipa/common/ipc_option.h,v $
 *
 * Revision history:
 *  31-Jul-95 - Original Version
 *
 * Description:
 *
 * Flags and routines for processing common INLINE/IPA options.
 * See also common/com/config_ipa.[hc].
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef ipc_option_INCLUDED
#define ipc_option_INCLUDED

#ifndef strtab_INCLUDED
#include "strtab.h"             // STRTAB_TYPE
#endif

#ifndef __SGI_STL_HASH_MAP_H
#include <hash_map.h>
#endif

#ifndef mempool_allocator_INCLUDED
#include "mempool_allocator.h"
#endif

struct eqstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};

struct eqint
{
  bool operator()(const INT s1, const INT s2) const
  {
    return (s1 == s2);
  }
};

typedef  hash_map<const char*, UINT, hash<const char*>, eqstr > INLINE_PU_MAP;
typedef  hash_map<const INT, UINT, hash<INT> > INLINE_EDGE_MAP;

extern UINT User_Specified_Name_Info(char *);
extern UINT User_Specified_Edge_Info(INT);
extern BOOL Is_User_Must_Inline(UINT);
extern BOOL Is_User_No_Inline(UINT);
extern BOOL Is_User_Not_Specified(UINT);
extern BOOL Is_Skip_Not_Specified(char * );
extern BOOL Is_Skip_Equal(char * );

#ifdef _STANDALONE_INLINER
extern void Process_Non_Local_Files();
extern void Process_Non_Local_Libraries();
#endif // _STANDALONE_INLINER


extern INT number_of_partitions;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void Process_Inline_Options ( void );
extern void Process_IPA_Specfile_Options( void );

#ifdef __cplusplus
}
#endif /* __cplusplus */
    
#endif /* ipc_option_INCLUDED */
