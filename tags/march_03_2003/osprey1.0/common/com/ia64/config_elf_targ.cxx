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
 * Module: config_elf_targ.cxx
 * $Revision: 1.1.1.1 $
 * $Date: 2002-05-22 20:07:03 $
 *
 * Description:
 *
 * ELF configuration specific to the target machine/system.
 *
 * ====================================================================
 * ====================================================================
 */

/* Solaris workaround
 * define EF_IRIX_ABI64 as in IRIX 's <elf.h> to allow compilation on 
 * Solaris.
 */
#ifdef _SOLARIS_SOLARIS
#define   EF_IRIX_ABI64           0x00000010
#endif

/* IRIX workaround
 * when compiling on irix, couldn't resolve EM_IA_64 (didn't have this
 * problem before before <elf.h> should use osprey1.0/linux/include/
 * elf.h which has the definition
 */

#ifdef _SGI_SGI
#define EM_IA_64 50     /* intel IA64 */
#endif


#include <elf.h>
#include <sys/elf_whirl.h>
#include "defs.h"
#include "erglob.h"
#include "tracing.h"
#include "config_elf_targ.h"

/* ====================================================================
 *
 * Config_Target_From_ELF
 *
 * Based on the flags in the ELF header <ehdr>,
 * return whether is 64bit and the isa level.
 *
 * ====================================================================
 */
void Config_Target_From_ELF (Elf64_Word e_flags, BOOL *is_64bit, INT *isa)
{
  *is_64bit = (e_flags & EF_IRIX_ABI64);

  *isa = 1;
}


/* ====================================================================
 *
 * Config_ELF_From_Target
 *
 * Return the ELF specific flags for the current target ABI and ISA subset.
 *
 * ====================================================================
 */
Elf32_Word Config_ELF_From_Target(BOOL is_64bit, BOOL old_abi, INT isa)
{
  Elf32_Word e_flags = 0;

  if (is_64bit) e_flags |= EF_IRIX_ABI64;

  return e_flags;
}

Elf32_Half Get_Elf_Target_Machine (void)
{
        return EM_IA_64;
}

