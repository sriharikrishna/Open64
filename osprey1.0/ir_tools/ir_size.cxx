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

// Solaris workaround
// define it as 1 to keep it the same as in ir_reader.cxx, etc
#ifdef _SOLARIS_SOLARIS
#define SHT_MIPS_WHIRL        1
#endif

#include <elf.h>
#include <errno.h>		    /* for sys_errlist */
#include <sys/stat.h>
#include <sys/elf_whirl.h>

#if !defined(__CYGWIN__)
  // Cygwin doesn't yet have libgen.h or basename(). Sigh.
# include <libgen.h>		    /* for basename() */
#endif

// Solaris CC workaround
#if defined(_SOLARIS_SOLARIS) && !defined(__GNUC__)
#include <algorithm>
using namespace std;
#else
#include <algo.h>
#endif

#include "defs.h"
#include "pu_info.h"
#include "opcode.h"
#include "wn.h"
#include "ir_bread.h"		    /* for WN_open_input(), etc. */
#include "err_host.tab"


static void *handle;   /* file handle */

extern BOOL 
file_exists (char *path)
{
        INT st;
        struct stat sbuf;
        st = stat(path, &sbuf);
        if (st == -1 && (errno == ENOENT || errno == ENOTDIR))
                return FALSE;
        else
                return TRUE;
}

static INT sym_size = 0;
static INT wn_size = 0;
static INT dg_size = 0;
static INT pref_size = 0;
static INT fb_size = 0;

static void
size_of_each_pu (PU_Info *pu_tree, BOOL verbose)
{
    PU_Info *pu;
    INT i = 0;

    for (pu = pu_tree; pu != NULL; pu = PU_Info_next(pu)) {
	if (verbose)
	    printf("%d\t%d\t%d\t%d\t%d\t%d\n",
		i,
		PU_Info_subsect_size(pu,WT_SYMTAB),
		PU_Info_subsect_size(pu,WT_TREE),
		PU_Info_subsect_size(pu,WT_DEPGRAPH),
		PU_Info_subsect_size(pu,WT_PREFETCH),
		PU_Info_subsect_size(pu,WT_FEEDBACK)
	    );
	sym_size += PU_Info_subsect_size(pu,WT_SYMTAB);
	wn_size += PU_Info_subsect_size(pu,WT_TREE);
	dg_size += PU_Info_subsect_size(pu,WT_DEPGRAPH);
	pref_size += PU_Info_subsect_size(pu,WT_PREFETCH);
	fb_size += PU_Info_subsect_size(pu,WT_FEEDBACK);
	if (PU_Info_child(pu)) {
	    size_of_each_pu (PU_Info_child(pu), verbose);
	}
	i++;
    }
}

static void 
print_size (char *name, INT size)
{
	if (size == 0) return;
	printf("%7s:\t%7d\n", name, size);
}

static void
ir_size (char *input_file, BOOL verbose)
{
    PU_Info *pu_tree;
    INT gsym_size = 0;
    INT const_size = 0;
    INT dst_size = 0;
    INT str_size = 0;
    INT ipa_size = 0;
    INT info_size = 0;
    handle = Open_Input_Info (input_file);

    gsym_size = Get_Elf_Section_Size (handle, SHT_MIPS_WHIRL, WT_GLOBALS);
    const_size = Get_Elf_Section_Size (handle, SHT_MIPS_WHIRL, WT_CONSTAB);
    dst_size = Get_Elf_Section_Size (handle, SHT_MIPS_WHIRL, WT_DST);
    str_size = Get_Elf_Section_Size (handle, SHT_MIPS_WHIRL, WT_STRTAB);
    ipa_size = Get_Elf_Section_Size (handle, SHT_MIPS_WHIRL, WT_IPA_SUMMARY);

    pu_tree = WN_get_PU_Infos (handle, NULL);
    if (pu_tree == (PU_Info *)-1) {
        ErrMsg ( EC_IR_Scn_Read, "PU headers", input_file);
    }
    info_size = Sizeof_PU_Infos(pu_tree);

    print_size("GLOBALS", gsym_size);
    print_size("CONSTS", const_size);
    print_size("DST", dst_size);
    print_size("STRTAB", str_size);
    print_size("IPA", ipa_size);
    print_size("PU_INFO", info_size);
    if (verbose) {
	printf("--------------------------------------------------------\n");
	printf("PU#\tSYMTAB\tTREE\tDEPGRF\tPREF\tFEEDBACK\n");
    }
    size_of_each_pu (pu_tree, verbose);
    if (verbose)
    	printf("--------------------------------------------------------\n");
    print_size("PU_SYMS", sym_size);
    print_size("WHIRL", wn_size);
    print_size("DEPGRF", dg_size);
    print_size("PREF", pref_size);
    print_size("FEEDBACK", fb_size);

    Free_Input_Info ();
}

static void
usage (char *progname)
{
  fprintf (stderr, "Usage: %s [-v] <Binary IR>\n", progname);
  fprintf (stderr, "\t-v option will print out verbose info\n");
  fprintf (stderr, "\tall sizes are in bytes\n");
  exit (1);
}

main (INT argc, char *argv[])
{
    register char *progname;
    INT binarg = 1;
    BOOL verbose = FALSE;

    MEM_Initialize();
    Set_Error_Tables (Phases, host_errlist);
    Init_Error_Handler (10);
    Set_Error_File(NULL);
    Set_Error_Line(ERROR_LINE_UNKNOWN);

#if defined(__CYGWIN__) 
    progname = argv[0]; // FIXME: see <libgen.h> above
#else
    progname = basename (argv[0]);
#endif

    if (argc < 2)
	usage(progname);
    while (*argv[binarg] == '-') {
	if (strcmp(argv[binarg], "-v") == 0) {
		verbose = TRUE;
	} else {
	      usage(progname);
	}
	++binarg;
    }
	
    if (argc < binarg+1)
	usage(progname);
    if (!file_exists(argv[binarg]))
	usage(progname);

    ir_size (argv[binarg], verbose);

    exit (0);
} /* main */


/* Dummy definitions to satisify references from routines that got pulled
 * in by the header files but are never called
 */
void Signal_Cleanup (INT sig) { }

char * Host_Format_Parm (INT kind, MEM_PTR parm) { return NULL; }

INT8 Debug_Level = 0;
