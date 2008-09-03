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

#ifdef USE_PCH
# include "common_com_pch.h"
#endif /* USE_PCH */
#pragma hdrstop
#include <unistd.h>		    /* for unlink() */
#include <fcntl.h>		    /* for open() */
#include <sys/mman.h>		    /* for mmap() */
#include <alloca.h>		    /* for alloca() */
#include <signal.h>		    /* for signal() */
#include <string.h>                 /* for strerror() */
#include <errno.h>		    /* for system error code */
#include <elf.h>		    /* Open64 version */
#include <sys/elf_whirl.h>	    /* for WHIRL sections' sh_info */
#include <cmplrs/rcodes.h>

#include "x_stdlib.h" // for mkstemp()

#ifndef USE_STANDARD_TYPES
# define USE_STANDARD_TYPES	    /* override unwanted defines in "defs.h" */
#endif

#include "defs.h"		    /* for wn_core.h */
#include "alignof.h"                /* for ALIGNOF() */
#ifdef OWN_ERROR_PACKAGE
/* Turn off assertion in the opcode handling routines, we assume the tree
 * is clean.  Also, this removes the dependency on "errors.h", which is not
 * used by all programs.
 */
#define Is_True(x,y) (0)
#define ERRORS_INCLUDED
#else
#include "erglob.h"
#endif
#include "errors.h"
#include "opcode.h"		    /* for wn_core.h */
#include "mempool.h"		    /* for MEM_POOL (for maps) */
#include "strtab.h"		    /* for strtab */
#include "symtab.h"		    /* for symtab */
#include "const.h"		    /* for constab */
#include "targ_const.h"		    /* for tcon */
#include "irbdata.h"		    /* for inito */
#include "config_targ.h"	    /* for Target_ABI */
#include "config_elf_targ.h"
#include "wn_core.h"		    /* for WN */
#include "wn.h"		    	    /* for max_region_id */
#include "wn_map.h"		    /* for WN maps */
#include "dwarf_DST_mem.h"	    /* for dst */
#include "pu_info.h"
#include "ir_elf.h"
#include "ir_bwrite.h"
#include "ir_bcom.h"
#include "ir_bread.h"
#include "tracing.h"                /* TEMPORARY FOR ROBERT'S DEBUGGING */

#ifdef BACK_END
#include "glob.h"
#include "pf_cg.h"
#include "instr_reader.h"
#include "fb_whirl.h"			// for feedback stuff

BOOL Write_BE_Maps = FALSE;
BOOL Write_AC_INTERNAL_Map = FALSE;
BOOL Write_ALIAS_CLASS_Map = FALSE;

extern WN **prefetch_ldsts;
extern INT num_prefetch_ldsts;
extern INT max_num_prefetch_ldsts;

extern WN **alias_classes;
extern INT num_alias_class_nodes;
extern INT max_alias_class_nodes;

extern WN **ac_internals;
extern INT num_ac_internal_nodes;
extern INT max_ac_internal_nodes;

extern "C" {
extern void *C_Dep_Graph(void);
extern void Depgraph_Write (void *depgraph, Output_File *fl, WN_MAP off_map);
}
#endif /* BACK_END */


#define MMAP(addr, len, prot, flags, fd, off)				\
    mmap((void *)(addr), (size_t)(len), (int)(prot), (int)(flags),	\
	 (int)(fd), (off_t)(off))

#define MUNMAP(addr, len)						\
    munmap((char *)(addr), (size_t)(len))

#define OPEN(path, flag, mode)						\
    open((const char *)(path), (int)(flag), (mode_t)(mode))



extern "C" typedef void (*SIG_HANDLER)(int);

static SIG_HANDLER old_sigsegv;   /* the previous signal handler */
static SIG_HANDLER old_sigbus;    /* the previous signal handler */

Output_File *Current_Output = 0;

#if !defined(__sgi)
# define MAPPED_SIZE 0x400000
#endif

static void
cleanup (Output_File *fl)
{
    fl->output_fd = -1;
    if (fl->num_of_section > 0)
	free (fl->section_list);
    fl->num_of_section = 0;
    fl->section_list = NULL;

    // eraxxon: must have already called munmap() to support cygwin.
    // (See comments below.)
    //MUNMAP((char *)(fl->map_addr), (size_t)(fl->mapped_size));
    fl->map_addr = NULL;
    fl->file_size = 0;
} /* cleanup */


/* Low-level I/O routines */


/* we steal the signal handler for SIGSEGV/SIGBUS when we create the mmap
 * because SIGSEGV/SIGBUS caused by operations on the mapped regions needs
 * to be handled differently. We keep the old signal handler in
 * "old_sigsegv/old_sigbus", to which we pass the non-mmap related SIGSEGV.
 */
extern "C" void
ir_bwrite_signal_handler (int sig, int err_num)
{
    SIG_HANDLER old_handler = 0;

    if (Doing_mmapped_io && err_num > 0) {
	Fatal_Error ("I/O error in %s: %s", Current_Output ?
		     Current_Output->file_name : "mmapped object",
		     strerror(err_num));
    }
    
    switch (sig) {
    case SIGBUS:
	old_handler = old_sigbus;
	break;
    case SIGSEGV:
	old_handler = old_sigsegv;
	break;
    }
    
    if (old_handler == SIG_DFL) {
      /* resignal - will get default handler */
      kill(getpid(), sig);
    } else if (old_handler != SIG_IGN) {
      /* call old handler */
      (*old_handler)(sig);
    }
    return;

} /* ir_bwrite_signal_handler */




/* Elf-related routines */
#define DEFAULT_NUM_OF_SECTIONS 8	

static Section *
get_section (Elf64_Word sh_info, char *name, Output_File *fl)
{
    register INT i;

    for (i = 0; i < fl->num_of_section; i++) {
	if ((fl->section_list[i].shdr.sh_info == sh_info) &&
	    (strcmp (fl->section_list[i].name, name) == 0)) {
	    fl->cur_section = fl->section_list + i;
	    return fl->cur_section;
	}
    }

    if (fl->num_of_section == 0) {
	fl->max_num_of_section = DEFAULT_NUM_OF_SECTIONS;
	fl->section_list =
	    (Section *)malloc(fl->max_num_of_section * sizeof(Section));
	FmtAssert (fl->section_list, ("No more memory"));
    } else if (fl->max_num_of_section == fl->num_of_section) {
	fl->max_num_of_section *= 2;
	fl->section_list = (Section *) realloc
	    (fl->section_list,
	     fl->max_num_of_section * sizeof(Section));
	FmtAssert (fl->section_list, ("No more memory"));
    }

    fl->cur_section = fl->section_list + fl->num_of_section;
    fl->num_of_section += 1;

    memset (fl->cur_section, 0, sizeof(Section));
    fl->cur_section->name = name;
    fl->cur_section->shdr.sh_info = sh_info;
    fl->cur_section->shdr.sh_type = SHT_MIPS_WHIRL;

    return fl->cur_section;
} /* get_section */


template <class Shdr>
UINT64
layout_sections (Shdr& strtab_sec, Output_File *fl)
{
    UINT64 e_shoff;
    Elf64_Word strtab_size = 1;

    for (INT i = 0; i < fl->num_of_section; i++) {
	Section& sec = fl->section_list[i];
	sec.shdr.sh_name = strtab_size;
	strtab_size += strlen(sec.name) + 1;
    }

    memset (&strtab_sec, 0, sizeof(Shdr));
    strtab_sec.sh_name = strtab_size;
    strtab_size += strlen (ELF_SHSTRTAB) + 1;
    strtab_sec.sh_type = SHT_STRTAB;
    strtab_sec.sh_size = strtab_size;
    strtab_sec.sh_offset = fl->file_size;
    fl->file_size += strtab_size;
    strtab_sec.sh_addralign = 1;
    strtab_sec.sh_entsize = 1;

    fl->file_size = ir_b_align (fl->file_size, ALIGNOF(Shdr),	0);
    e_shoff = fl->file_size;

    /* There are 2 extra sections: the null and the section string table */
    fl->file_size += sizeof(Shdr) * (fl->num_of_section + 2);

    /* make sure the entire file is mapped */
    if (fl->file_size >= fl->mapped_size)
	ir_b_grow_map (0, fl);

    return e_shoff;
} /* layout_sections */


// Solaris CC workaround the template function "write_output()" causes
// "Assertion error" with CC compiler when -g is present, to solve
// this problem, I removed the template definition and replaced it
// with explicit overloaded definition. The following two definitions
// are for "ELF32" and "ELF64", respectively.
//
#if !defined(__GNUC__) && defined(_SOLARIS_SOLARIS)

#ifdef ELF
#undef ELF
#endif
#define ELF ELF64
static void
write_output (UINT64 e_shoff, const typename ELF::Elf_Shdr& strtab_sec,
	      Output_File *fl, const ELF& tag)
{
    INT i;
    char *base_addr = fl->map_addr;

    /* write Elf header */
    typename ELF::Elf_Ehdr* ehdr = (typename ELF::Elf_Ehdr *) fl->map_addr;
    strcpy ((char *) ehdr->e_ident, ELFMAG);
    ehdr->e_ident[EI_CLASS] = tag.Elf_class ();
#if (defined(__sgi) || defined(__sun) || defined(__MACH__))
    ehdr->e_ident[EI_DATA] = ELFDATA2MSB; /* assume MSB for now */
#else
    ehdr->e_ident[EI_DATA] = ELFDATA2LSB; /* assume LSB for now */
#endif
    ehdr->e_ident[EI_VERSION] = EV_CURRENT;
    ehdr->e_type = ET_IR;
    ehdr->e_machine = Get_Elf_Target_Machine();
    ehdr->e_version = EV_CURRENT;
    ehdr->e_shoff = e_shoff;
    ehdr->e_flags = Config_ELF_From_Target (! Use_32_Bit_Pointers, FALSE,
					    (INT) Target_ISA);  
    ehdr->e_ehsize = sizeof(ELF::Elf_Ehdr);
    ehdr->e_shentsize = sizeof(ELF::Elf_Shdr);
    ehdr->e_shnum = fl->num_of_section + 2;
    ehdr->e_shstrndx = fl->num_of_section + 1;

    /* the section string table */
    char* str = base_addr + strtab_sec.sh_offset;
    str[0] = '\0';
    str++;
    for (i = 0; i < fl->num_of_section; i++) {
	strcpy (str, fl->section_list[i].name);
	str += strlen (str) + 1;
    }
    strcpy (str, ELF_SHSTRTAB);

    /* simple check to see if layout has been done right */
    if ((INTPTR)str + strlen(ELF_SHSTRTAB) + 1 >
	e_shoff + (INTPTR) base_addr)
	ErrMsg (EC_IR_Scn_Write, "Section Header String Table", fl->file_name);
	
    /* Finally, the section header table */
    typename ELF::Elf_Shdr* shdr =
	(typename ELF::Elf_Shdr *) (base_addr + e_shoff);
    memset (shdr, 0, sizeof(ELF::Elf_Shdr)); /* First null entry */
    shdr++;
    if (tag.Elf_class() == ELFCLASS64) {
	for (i = 0; i < fl->num_of_section; i++, shdr++)
	    memcpy (shdr, &(fl->section_list[i].shdr), sizeof(ELF::Elf_Shdr));
    } else {
	// convert Elf64_Shdr to Elf32_Shdr
	for (i = 0; i < fl->num_of_section; i++, shdr++) {
	    shdr->sh_name = fl->section_list[i].shdr.sh_name;
	    shdr->sh_type = fl->section_list[i].shdr.sh_type;
	    shdr->sh_flags = fl->section_list[i].shdr.sh_flags;
	    shdr->sh_addr = fl->section_list[i].shdr.sh_addr;
	    shdr->sh_offset = fl->section_list[i].shdr.sh_offset;
	    shdr->sh_size = fl->section_list[i].shdr.sh_size;
	    shdr->sh_link = fl->section_list[i].shdr.sh_link;
	    shdr->sh_info = fl->section_list[i].shdr.sh_info;
	    shdr->sh_addralign = fl->section_list[i].shdr.sh_addralign;
	    shdr->sh_entsize = fl->section_list[i].shdr.sh_entsize;
	}
    }
    memcpy (shdr, &strtab_sec, sizeof(ELF::Elf_Shdr));
} /* write_output */

// Solaris CC workaround
// as mentioned earlier, following is the definition on "ELF32"

#ifdef ELF
#undef ELF
#endif
#define ELF ELF32
static void
write_output (UINT64 e_shoff, const typename ELF::Elf_Shdr& strtab_sec,
	      Output_File *fl, const ELF& tag)
{
    INT i;
    char *base_addr = fl->map_addr;

    /* write Elf header */
    typename ELF::Elf_Ehdr* ehdr = (typename ELF::Elf_Ehdr *) fl->map_addr;
    strcpy ((char *) ehdr->e_ident, ELFMAG);
    ehdr->e_ident[EI_CLASS] = tag.Elf_class ();
#if (defined(__sgi) || defined(__sun) || defined(__MACH__))
    ehdr->e_ident[EI_DATA] = ELFDATA2MSB; /* assume MSB for now */
#else
    ehdr->e_ident[EI_DATA] = ELFDATA2LSB; /* assume LSB for now */
#endif
    ehdr->e_ident[EI_VERSION] = EV_CURRENT;
    ehdr->e_type = ET_IR;
    ehdr->e_machine = Get_Elf_Target_Machine();
    ehdr->e_version = EV_CURRENT;
    ehdr->e_shoff = e_shoff;
    ehdr->e_flags = Config_ELF_From_Target (! Use_32_Bit_Pointers, FALSE,
					    (INT) Target_ISA);  
    ehdr->e_ehsize = sizeof(ELF::Elf_Ehdr);
    ehdr->e_shentsize = sizeof(ELF::Elf_Shdr);
    ehdr->e_shnum = fl->num_of_section + 2;
    ehdr->e_shstrndx = fl->num_of_section + 1;

    /* the section string table */
    char* str = base_addr + strtab_sec.sh_offset;
    str[0] = '\0';
    str++;
    for (i = 0; i < fl->num_of_section; i++) {
	strcpy (str, fl->section_list[i].name);
	str += strlen (str) + 1;
    }
    strcpy (str, ELF_SHSTRTAB);

    /* simple check to see if layout has been done right */
    if ((INTPTR)str + strlen(ELF_SHSTRTAB) + 1 >
	e_shoff + (INTPTR) base_addr)
	ErrMsg (EC_IR_Scn_Write, "Section Header String Table", fl->file_name);
	
    /* Finally, the section header table */
    typename ELF::Elf_Shdr* shdr =
	(typename ELF::Elf_Shdr *) (base_addr + e_shoff);
    memset (shdr, 0, sizeof(ELF::Elf_Shdr)); /* First null entry */
    shdr++;
    if (tag.Elf_class() == ELFCLASS64) {
	for (i = 0; i < fl->num_of_section; i++, shdr++)
	    memcpy (shdr, &(fl->section_list[i].shdr), sizeof(ELF::Elf_Shdr));
    } else {
	// convert Elf64_Shdr to Elf32_Shdr
	for (i = 0; i < fl->num_of_section; i++, shdr++) {
	    shdr->sh_name = fl->section_list[i].shdr.sh_name;
	    shdr->sh_type = fl->section_list[i].shdr.sh_type;
	    shdr->sh_flags = fl->section_list[i].shdr.sh_flags;
	    shdr->sh_addr = fl->section_list[i].shdr.sh_addr;
	    shdr->sh_offset = fl->section_list[i].shdr.sh_offset;
	    shdr->sh_size = fl->section_list[i].shdr.sh_size;
	    shdr->sh_link = fl->section_list[i].shdr.sh_link;
	    shdr->sh_info = fl->section_list[i].shdr.sh_info;
	    shdr->sh_addralign = fl->section_list[i].shdr.sh_addralign;
	    shdr->sh_entsize = fl->section_list[i].shdr.sh_entsize;
	}
    }
    memcpy (shdr, &strtab_sec, sizeof(ELF::Elf_Shdr));
} /* write_output */

#undef ELF 
// don't forget to undefine ELF, just for safety. 

#else   // __GNUC__ _SOLARIS_SOLARIS 
// remove the template definition, see the comments above for details.

template <class ELF>
static void
write_output (UINT64 e_shoff, const typename ELF::Elf_Shdr& strtab_sec,
	      Output_File *fl, const ELF& tag)
{
    INT i;
    char *base_addr = fl->map_addr;

    /* write Elf header */
    typename ELF::Elf_Ehdr* ehdr = (typename ELF::Elf_Ehdr *) fl->map_addr;
    strcpy ((char *) ehdr->e_ident, ELFMAG);
    ehdr->e_ident[EI_CLASS] = tag.Elf_class ();
#if (defined(__sgi) || defined(__sun) || defined(__MACH__))
    ehdr->e_ident[EI_DATA] = ELFDATA2MSB; /* assume MSB for now */
#else
    ehdr->e_ident[EI_DATA] = ELFDATA2LSB; /* assume LSB for now */
#endif
    ehdr->e_ident[EI_VERSION] = EV_CURRENT;
    ehdr->e_type = ET_IR;
    ehdr->e_machine = Get_Elf_Target_Machine();
    ehdr->e_version = EV_CURRENT;
    ehdr->e_shoff = e_shoff;
    ehdr->e_flags = Config_ELF_From_Target (! Use_32_Bit_Pointers, FALSE,
					    (INT) Target_ISA);  
    ehdr->e_ehsize = sizeof(typename ELF::Elf_Ehdr);
    ehdr->e_shentsize = sizeof(typename ELF::Elf_Shdr);
    ehdr->e_shnum = fl->num_of_section + 2;
    ehdr->e_shstrndx = fl->num_of_section + 1;

    /* the section string table */
    char* str = base_addr + strtab_sec.sh_offset;
    str[0] = '\0';
    str++;
    for (i = 0; i < fl->num_of_section; i++) {
	strcpy (str, fl->section_list[i].name);
	str += strlen (str) + 1;
    }
    strcpy (str, ELF_SHSTRTAB);

    /* simple check to see if layout has been done right */
    if ((INTPTR)str + strlen(ELF_SHSTRTAB) + 1 >
	e_shoff + (INTPTR) base_addr)
	ErrMsg (EC_IR_Scn_Write, "Section Header String Table", fl->file_name);
	
    /* Finally, the section header table */
    typename ELF::Elf_Shdr* shdr =
	(typename ELF::Elf_Shdr *) (base_addr + e_shoff);
    memset (shdr, 0, sizeof(typename ELF::Elf_Shdr)); /* First null entry */
    shdr++;
    if (tag.Elf_class() == ELFCLASS64) {
	for (i = 0; i < fl->num_of_section; i++, shdr++)
	    memcpy (shdr, &(fl->section_list[i].shdr), 
		    sizeof(typename ELF::Elf_Shdr));
    } else {
	// convert Elf64_Shdr to Elf32_Shdr
	for (i = 0; i < fl->num_of_section; i++, shdr++) {
	    shdr->sh_name = fl->section_list[i].shdr.sh_name;
	    shdr->sh_type = fl->section_list[i].shdr.sh_type;
	    shdr->sh_flags = fl->section_list[i].shdr.sh_flags;
	    shdr->sh_addr = fl->section_list[i].shdr.sh_addr;
	    shdr->sh_offset = fl->section_list[i].shdr.sh_offset;
	    shdr->sh_size = fl->section_list[i].shdr.sh_size;
	    shdr->sh_link = fl->section_list[i].shdr.sh_link;
	    shdr->sh_info = fl->section_list[i].shdr.sh_info;
	    shdr->sh_addralign = fl->section_list[i].shdr.sh_addralign;
	    shdr->sh_entsize = fl->section_list[i].shdr.sh_entsize;
	}
    }
    memcpy (shdr, &strtab_sec, sizeof(typename ELF::Elf_Shdr));
} /* write_output */

#endif  // __GNUC__ _SOLARIX_SOLARIS


/*
 * open and create a temporary file, then unlink it right away so that the
 * file will be gone as soon as it is closed.
 */  

#define DEFAULT_TMPDIR		"/usr/tmp"
#define DEFAULT_TEMPLATE	"/elf_wnXXXXXX"

static int
create_temp_file (Output_File *fl)
{
    register char *tmpdir;
    register char *path;
    register int fd;

    if ((tmpdir = getenv("TMPDIR")) == 0)
	tmpdir = DEFAULT_TMPDIR;
    path = (char *) malloc (strlen(tmpdir) + strlen(DEFAULT_TEMPLATE) + 1);
    if (path == 0)
	return -1;
    strcpy (path, tmpdir);
    strcat (path, DEFAULT_TEMPLATE);
    fd = ux_mkstemp (path);
    if (fd != -1)
	unlink (path);

    fl->file_name = path;
    return fd;
} /* create_temp_file */


/* The followings are exported functions. */

/*
 * Open an output file for writing and return an Output_File structure
 * to record the current state of that file.  If no file name is given,
 * a temporary file will be created.  More than one file may be
 * open at once.  Returns NULL if the file cannot be opened.
 */

Output_File *
WN_open_output (const char *file_name)
{
    Output_File *fl;
    Section *cur_section;

// Solaris CC error
// compiler complains that "Could not find a match for std::signal(int, 
// void(*)(int))." Not sure how to solve this problem yet! 
// okay, just add std:: before signal and it will compile

    if (old_sigsegv == 0)
#ifdef _SOLARIS_SOLARIS
	old_sigsegv = std::signal (SIGSEGV, reinterpret_cast<SIG_HANDLER>
			      (ir_bwrite_signal_handler));
#else
        old_sigsegv = signal (SIGSEGV, reinterpret_cast<SIG_HANDLER>
                              (ir_bwrite_signal_handler));
#endif

    if (old_sigbus == 0)
#ifdef _SOLARIS_SOLARIS
	old_sigbus = std::signal (SIGBUS, reinterpret_cast<SIG_HANDLER>
			     (ir_bwrite_signal_handler)); 
#else
        old_sigbus = signal (SIGBUS, reinterpret_cast<SIG_HANDLER>
                             (ir_bwrite_signal_handler));
#endif

    fl = (Output_File *)malloc(sizeof(Output_File));
    if (!fl) return NULL;

    if (file_name == 0) {
	fl->output_fd = create_temp_file (fl);
    } else {
	fl->file_name = file_name;
	// set mode to rw for all; users umask will AND with that.
	fl->output_fd = OPEN (file_name, O_RDWR|O_CREAT|O_TRUNC, 0666);
    }
    if (fl->output_fd < 0)
	return NULL;

    // mmap() normally cannot automatically increase file size, so we
    // allocate some space using ftruncate().  SGI's mmap() can avoid
    // this. cf. use of ftruncate() in ir_bcom.cxx.
#if !defined(__sgi)
    ftruncate(fl->output_fd, MAPPED_SIZE);
#endif

    fl->section_list = NULL;
    fl->cur_section = NULL;
    fl->num_of_section = 0;
    
    /* initialize the output map */
    if (ir_b_create_map (fl) == (char *) (-1))
	return NULL;

    /* leave space for the Elf header */
#ifdef __ALWAYS_USE_64BIT_ELF__
    fl->file_size = sizeof(Elf64_Ehdr);
#else
    fl->file_size = Use_32_Bit_Pointers ?
	sizeof(Elf32_Ehdr) : sizeof(Elf64_Ehdr);
#endif

    /* go to the PU-specific section */
    cur_section = get_section (WT_PU_SECTION, MIPS_WHIRL_PU_SECTION, fl);

    fl->file_size = ir_b_align (fl->file_size, sizeof(mUINT64), 0);
    cur_section->shdr.sh_offset = fl->file_size;

    /* leave space for a pointer to the PU-section header */
    fl->file_size += sizeof(Elf64_Word);

    return fl;
} /* WN_open_output */


/*
 * Write out the PU-section header.  This routine is called after all the
 * PUs have been written out.
 */

void
WN_write_PU_Infos (PU_Info *pu_list, Output_File *fl)
{
    INT32 hdr_sz;
    Section *cur_section = fl->cur_section;

    /* make sure we're in the right section */
    if (strcmp(cur_section->name, MIPS_WHIRL_PU_SECTION) != 0)
	ErrMsg (EC_IR_Scn_Write, "PU headers", fl->file_name);

    fl->file_size = ir_b_align (fl->file_size, sizeof(mINT32), 0);

    /* record the header offset at the beginning of the section */
    *(Elf64_Word *)(fl->map_addr + cur_section->shdr.sh_offset) =
	(Elf64_Word)(fl->file_size - cur_section->shdr.sh_offset);

    hdr_sz = Sizeof_PU_Infos (pu_list);

    /* allocate space in the output file */
    if (fl->file_size + hdr_sz >= fl->mapped_size)
	ir_b_grow_map (hdr_sz, fl);
    
    if (Write_PU_Infos (pu_list, fl->map_addr + fl->file_size) == -1)
	ErrMsg (EC_IR_Scn_Write, "PU headers", fl->file_name);
    fl->file_size += hdr_sz;

    cur_section->shdr.sh_size = fl->file_size - cur_section->shdr.sh_offset;
    /* some data structures placed in the WHIRL_PU_SECTION needs to be
       64bit aligned, even though the pu_info structure itself is not */
    cur_section->shdr.sh_addralign = sizeof(mINT64);
} /* WN_write_PU_Infos */


/*
 * Write out a tree.  This routine traverse the specified tree and writes
 * out every node in that tree.  The tree nodes are assumed valid and no
 * validation is performed.  The size and offset of the output subsection
 * are stored in the PU_Info structure.  The subsection offsets of the WNs
 * are stored in a map specified by the "off_map" parameter, so that they
 * can be used when writing out information associated with the tree
 * (e.g. dependence graphs).  Memory occupied by the tree can be freed
 * after it is written out.
 */

void
WN_write_tree (PU_Info *pu, WN_MAP off_map, Output_File *fl)
{
    static char *scn_name = "tree";
    UINT padding;
    Elf64_Word this_tree;
    WN *tree;
    off_t tree_base;
    Section *cur_section = fl->cur_section;

    /* make sure we're in the right section */
    if (strcmp(cur_section->name, MIPS_WHIRL_PU_SECTION) != 0 ||
	PU_Info_state(pu, WT_TREE) != Subsect_InMem)
	ErrMsg (EC_IR_Scn_Write, scn_name, fl->file_name);

    tree = PU_Info_tree_ptr(pu);

#ifdef BACK_END
    if (Write_BE_Maps) {
	/* initialize globals used to record nodes with prefetch pointers */
	prefetch_ldsts = NULL;
	num_prefetch_ldsts = 0;
	max_num_prefetch_ldsts = 0;
    }

    if (Write_ALIAS_CLASS_Map) {
      /* globals used to record nodes with alias classification
       * information
       */
      alias_classes = NULL;
      num_alias_class_nodes = 0;
      max_alias_class_nodes = 0;
    }

    if (Write_AC_INTERNAL_Map) {
      /* globals used to record nodes with alias classification's
       * internal information for indirect memops
       */
      ac_internals = NULL;
      num_ac_internal_nodes = 0;
      max_ac_internal_nodes = 0;
    }
#endif

    padding = ((char *)tree) - (char *)WN_StartAddress(tree);
    if (padding == 0) {
	/* root of tree must have prev/next pointers  */
	ErrMsg (EC_IR_Scn_Write, scn_name, fl->file_name);
    }
    /* add room for offset to the first node */
    padding += sizeof(Elf64_Word);
    fl->file_size = ir_b_align (fl->file_size, ALIGNOF(WN), padding);
    tree_base = fl->file_size;

    this_tree = ir_b_write_tree (tree, tree_base, fl, off_map);

    /* save the offset to the first node */
    *(Elf64_Word *)(fl->map_addr + tree_base) = this_tree;

#ifdef BACK_END
    /* stash the array of nodes with prefetches in the PU_Info */
    if (Write_BE_Maps && num_prefetch_ldsts > 0) {
	/* mark the end (must have already allocated space for the marker) */
	prefetch_ldsts[num_prefetch_ldsts] = NULL;
	PU_Info_subsect_ptr(pu, WT_PREFETCH) = (void *)prefetch_ldsts;
	Set_PU_Info_state(pu, WT_PREFETCH, Subsect_InMem);
    }

    if (Write_ALIAS_CLASS_Map && num_alias_class_nodes > 0) {
      alias_classes[num_alias_class_nodes] = NULL;
      Set_PU_Info_alias_class_ptr(pu, alias_classes);
      Set_PU_Info_state(pu, WT_ALIAS_CLASS, Subsect_InMem);
    }

    if (Write_AC_INTERNAL_Map && num_ac_internal_nodes > 0) {
      ac_internals[num_ac_internal_nodes] = NULL;
      Set_PU_Info_ac_internal_ptr(pu, ac_internals);
      Set_PU_Info_state(pu, WT_AC_INTERNAL, Subsect_InMem);
    }
#endif

    Set_PU_Info_state(pu, WT_TREE, Subsect_Written);
    PU_Info_subsect_size(pu, WT_TREE) = fl->file_size - tree_base;
    PU_Info_subsect_offset(pu, WT_TREE) =
	tree_base - cur_section->shdr.sh_offset;

    Set_Max_Region_Id(0);	/* reset max region id for pu */

} /* WN_write_tree */


/*
 * Write out the command and arguments needed to compile this IR.  The
 * arguments are in the standard argc, argv formats.
 */

void
WN_write_flags (INT argc, char **argv, Output_File *fl)
{
    INT i, argv_size;
    off_t offset;
    Elf64_Word argc_buf = argc;
    Section *cur_section;

    FmtAssert (argc > 0 && argv != 0, ("invalid argument to WN_write_flags()"));

    cur_section = get_section (WT_COMP_FLAGS, MIPS_WHIRL_COMP_FLAGS, fl);

    fl->file_size = ir_b_align (fl->file_size, sizeof(Elf64_Word), 0);
    cur_section->shdr.sh_offset = fl->file_size;

    /* First, write out argc */
    ir_b_save_buf (&argc_buf, sizeof(Elf64_Word), sizeof(Elf64_Word), 0, fl);

    /* Second, write out argv.  Because we need to convert the argv array
       from pointers to Elf64_Words, we just increment the file_size by
       the appropriate amount instead of using save_buf.  We then go back
       and fill in the values when writing out the actual argv strings. */

    fl->file_size = ir_b_align (fl->file_size, sizeof(Elf64_Word), 0);
    argv_size = sizeof(Elf64_Word) * argc;
    if (fl->file_size + argv_size >= fl->mapped_size)
	ir_b_grow_map (argv_size, fl);

    offset = fl->file_size;
    fl->file_size += argv_size;
    
    for (i = 0; i < argc; i++) {
	INT len = strlen(argv[i]) + 1;
	Elf64_Word string_offset, *argv_ptr;

	string_offset = ir_b_save_buf (argv[i], len, 0, 0, fl);

	/* the following statement is NOT loop-invariant since the file
	   may be remapped at a different address when writing the strings */
	argv_ptr = (Elf64_Word *) (fl->map_addr + offset);

	argv_ptr[i] = string_offset - cur_section->shdr.sh_offset;
    }

    cur_section->shdr.sh_size = fl->file_size - cur_section->shdr.sh_offset;
    cur_section->shdr.sh_addralign = sizeof(Elf64_Word);

} /* WN_write_flags */


/*
 * Write out the global symbol table.  Unlike the local symtabs, the
 * global table gets its own Elf section and can only be written out
 * once.
 */


void
WN_write_globals (Output_File *fl)
{
    Section *cur_section = get_section (WT_GLOBALS, MIPS_WHIRL_GLOBALS, fl);

    fl->file_size = ir_b_align (fl->file_size, sizeof(mINT64), 0);
    cur_section->shdr.sh_offset = fl->file_size;

    (void) ir_b_write_global_symtab (fl->file_size, fl);

    cur_section->shdr.sh_size = fl->file_size - cur_section->shdr.sh_offset;
    cur_section->shdr.sh_addralign = sizeof(mINT64);

} // WN_write_globals


void
WN_write_symtab (PU_Info *pu, Output_File *fl)
{
    Section *cur_section = fl->cur_section;

    /* make sure we're in the right section */
    if (strcmp(cur_section->name, MIPS_WHIRL_PU_SECTION) != 0 ||
	PU_Info_state(pu, WT_SYMTAB) != Subsect_InMem)
	ErrMsg (EC_IR_Scn_Write, "local symtab", fl->file_name);

    const SCOPE& scope =
	Scope_tab[PU_lexical_level (&St_Table[PU_Info_proc_sym(pu)])];

    fl->file_size = ir_b_align (fl->file_size, sizeof(mINT64), 0);
    off_t symtab_base = fl->file_size;

    (void)ir_b_write_local_symtab (scope, symtab_base, fl);

    Set_PU_Info_state(pu, WT_SYMTAB, Subsect_Written);
    PU_Info_subsect_size(pu, WT_SYMTAB) = fl->file_size - symtab_base;
    PU_Info_subsect_offset(pu, WT_SYMTAB) =
	symtab_base - cur_section->shdr.sh_offset;
} // WN_write_symtab

extern "C" void
WN_write_strtab (const void* strtab, UINT64 size, Output_File *fl)
{
    Section *cur_section = get_section (WT_STRTAB, MIPS_WHIRL_STRTAB, fl);

    fl->file_size = ir_b_align (fl->file_size, sizeof(char), 0);
    cur_section->shdr.sh_offset = fl->file_size;

    (void) ir_b_save_buf (strtab, size, sizeof(char), 0, fl);

    cur_section->shdr.sh_size = fl->file_size - cur_section->shdr.sh_offset;
    cur_section->shdr.sh_addralign = sizeof(char);
    
} // WN_write_strtab


/*
 * Write out the debug symbol table (dst).  The DST gets its own Elf
 * section and should only be written once per file.
 */

void
WN_write_dst (DST_TYPE dst, Output_File *fl)
{
    off_t dst_base;
    Section *cur_section;

    cur_section = get_section (WT_DST, MIPS_WHIRL_DST, fl);
    
    fl->file_size = ir_b_align (fl->file_size, sizeof(mINT32), 0);
    cur_section->shdr.sh_offset = fl->file_size;
    dst_base = fl->file_size;

    (void) ir_b_write_dst (dst, dst_base, fl);

    cur_section->shdr.sh_size = fl->file_size - cur_section->shdr.sh_offset;
    cur_section->shdr.sh_addralign = sizeof(mINT32);

} /* WN_write_dst */


#ifdef BACK_END

namespace
{
    // local templates used only by WN_write_feedback

    template <class T>
    void
    write_profile (off_t base, const T& data, Output_File* fl, mUINT32& num,
		   mUINT32& offset) {
	offset =
	    ir_b_save_buf (&(data.front()),
			   data.size () * sizeof(typename T::value_type), 
			   ALIGNOF(typename T::value_type), 0, fl) - base;
	num = data.size ();
    }

    template <class T>
    void
    write_target_profile (off_t base, T& data, Output_File* fl,
			  mUINT32& num, mUINT32& offset,
			  mUINT32& target_offset) {
	num = data.size ();
	fl->file_size = ir_b_align (fl->file_size, sizeof(mINT64), 0);
	offset = fl->file_size - base;

	typename T::iterator first (data.begin ());
	while (first != data.end ()) {
	    const vector<FB_FREQ>& freq = first->freq_targets;
	    ir_b_save_buf (&(freq.front ()), freq.size () * sizeof(FB_FREQ),
			   ALIGNOF(FB_FREQ), 0, fl);
	    ++first;
	}

	target_offset = fl->file_size - base;
	first = data.begin ();
	while (first != data.end ()) {
	    INT32 num = first->freq_targets.size ();
	    ir_b_save_buf (&num, sizeof(INT32), sizeof(INT32), 0, fl);
	    ++first;
	}
    }
} 



/*
 *  Write out the IPA summary information.
 */

void
IPA_write_summary (void (*IPA_irb_write_summary) (Output_File*),
		   Output_File *fl)
{
    Section *cur_section;

    cur_section = get_section(WT_IPA_SUMMARY, MIPS_WHIRL_SUMMARY, fl);

    fl->file_size = ir_b_align(fl->file_size, sizeof(mINT64), 0);
    cur_section->shdr.sh_offset = fl->file_size;

    (*IPA_irb_write_summary) (fl);

    cur_section->shdr.sh_size = fl->file_size - cur_section->shdr.sh_offset;
    cur_section->shdr.sh_addralign = sizeof(mINT64);

}



/*
 * Write out the prefetch pointer mapping. The prefetch mappings are written
 * to PU subsections.  The size and offset of the output subsection are
 * stored in the PU_Info structure.  The off_map mapping must contain the
 * subsection offsets for the WN nodes referenced from the prefetch nodes.
 * To avoid making a second pass through the WHIRL tree, we rely on the
 * WN_write_tree routine to provide an array of the WNs with prefetch
 * pointers in the PU_Info.
 */

void
WN_write_prefetch (PU_Info *pu, WN_MAP off_map, Output_File *fl)
{
    WN **pf_ldsts;
    off_t prefetch_base;
    Section *cur_section = fl->cur_section;
    INT i;
    WN *node;
    PF_POINTER *pf_ptr;
    INT32 cur_offset;
    Elf64_Word node_offset;

#define PF_PTR_ADDR(offset) ((PF_POINTER *)(fl->map_addr + (offset)))

    if (PU_Info_state(pu, WT_PREFETCH) == Subsect_Missing)
	return;

    /* make sure we're in the right section */
    if (strcmp(cur_section->name, MIPS_WHIRL_PU_SECTION) != 0 ||
	PU_Info_state(pu, WT_PREFETCH) != Subsect_InMem)
	ErrMsg (EC_IR_Scn_Write, "prefetch map", fl->file_name);

    pf_ldsts = (WN **)PU_Info_subsect_ptr(pu, WT_PREFETCH);
    if (!pf_ldsts) {
	/* this should only happen if the prefetches were read in and then
	   removed but we might as well check for it */
	Set_PU_Info_state(pu, WT_PREFETCH, Subsect_Missing);
	return;
    }

    fl->file_size = ir_b_align(fl->file_size, sizeof(mINT32), 0);
    prefetch_base = fl->file_size;

    for (i = 0; pf_ldsts[i]; i++) {

	node = pf_ldsts[i];
	pf_ptr = (PF_POINTER *) WN_MAP_Get(WN_MAP_PREFETCH, node);

	node_offset = (Elf64_Word)WN_MAP32_Get(off_map, node);
	cur_offset = ir_b_save_buf((void *)&node_offset, sizeof(Elf64_Word),
				   sizeof(Elf64_Word), 0, fl);

	cur_offset = ir_b_save_buf((void *)pf_ptr, sizeof(PF_POINTER),
				   ALIGNOF(PF_POINTER), 0, fl);

	/* change the WN pointers to offsets. Store a -1 if NULL pointer */
        PF_PTR_ADDR(cur_offset)->wn_pref_1L =
          ((pf_ptr->wn_pref_1L) ?
           (WN *) WN_MAP32_Get(off_map, pf_ptr->wn_pref_1L) :
           (WN *) -1);
	PF_PTR_ADDR(cur_offset)->wn_pref_2L =
          ((pf_ptr->wn_pref_2L) ?
           (WN *) WN_MAP32_Get(off_map, pf_ptr->wn_pref_2L) :
           (WN *) -1);
    }

    /* deallocate the array created by write_tree */
    PU_Info_subsect_ptr(pu, WT_PREFETCH) = NULL;
    free(pf_ldsts);

    /* write out a -1 to mark the end */
    node_offset = (Elf64_Word)-1;
    ir_b_save_buf(&node_offset, sizeof(Elf64_Word), sizeof(Elf64_Word), 0, fl);

    Set_PU_Info_state(pu, WT_PREFETCH, Subsect_Written);
    PU_Info_subsect_size(pu, WT_PREFETCH) = fl->file_size - prefetch_base;
    PU_Info_subsect_offset(pu, WT_PREFETCH) =
	prefetch_base - cur_section->shdr.sh_offset;

} /* WN_write_prefetch */


template<class MAP_ENTRY_TYPE>
static inline MAP_ENTRY_TYPE
WN_MAP_retrieve(WN_MAP, WN *, MAP_ENTRY_TYPE);

static inline INT32
WN_MAP_retrieve(WN_MAP wn_map, WN *wn, INT32)
{
  return WN_MAP32_Get(wn_map, wn);
}

static inline INT64
WN_MAP_retrieve(WN_MAP wn_map, WN *wn, INT64)
{
  return WN_MAP64_Get(wn_map, wn);
}

static inline void *
WN_MAP_retrieve(WN_MAP wn_map, WN *wn, void *)
{
  return WN_MAP_Get(wn_map, wn);
}

// Write out an WHIRL mapping, such as an alias classification map, or
// a frequency map.
template<class MAP_ENTRY_TYPE>
static inline void
WN_write_generic_map(PU_Info        *pu,
		     WN_MAP          off_map,
		     Output_File    *fl,
		     INT32           subsection_type,
		     WN_MAP          value_map,
		     const char     *subsection_name,
		     MAP_ENTRY_TYPE)
{
  WN **nodes_represented;	// WHIRL nodes that need map values recorded
  off_t mapping_base;		// File offset of the mapping info
  Elf64_Word node_offset;	// File offset of the current WN
  INT i;

  if (PU_Info_state (pu, subsection_type) == Subsect_Missing)
    return;

  if (strcmp (fl->cur_section->name, MIPS_WHIRL_PU_SECTION) != 0 ||
      PU_Info_state(pu, subsection_type) != Subsect_InMem) {
    ErrMsg (EC_IR_Scn_Write, subsection_name, fl->file_name);
  }

  nodes_represented = (WN **) PU_Info_subsect_ptr (pu, subsection_type);
  if (!nodes_represented) {
    Set_PU_Info_state(pu, subsection_type, Subsect_Missing);
    return;
  }

  fl->file_size = ir_b_align (fl->file_size, sizeof(mINT64), 0);
  mapping_base = fl->file_size;

  for (i = 0; nodes_represented[i]; i++) {
    WN *node = nodes_represented[i];
    MAP_ENTRY_TYPE map_value = WN_MAP_retrieve(value_map,
					       node,
					       (MAP_ENTRY_TYPE) 0);
    if (map_value != (MAP_ENTRY_TYPE) 0) {
      node_offset = WN_MAP32_Get(off_map, node);
      (void) ir_b_save_buf ((void*) &node_offset, sizeof(Elf64_Word),
			    sizeof(Elf64_Word), 0, fl);
      (void) ir_b_save_buf ((void *) &map_value,	// data
			    sizeof(MAP_ENTRY_TYPE),	// size
			    sizeof(MAP_ENTRY_TYPE),	// alignment
			    0,				// padding
			    fl);			// file
    }
  }

  PU_Info_subsect_ptr(pu, subsection_type) = NULL;

  free(nodes_represented);
    
  /* write out a -1 to mark the end */
  node_offset = (Elf64_Word)-1;
  ir_b_save_buf(&node_offset, sizeof(Elf64_Word), sizeof(Elf64_Word), 0, fl);

  Set_PU_Info_state(pu, subsection_type, Subsect_Written);
  PU_Info_subsect_size(pu, subsection_type) = fl->file_size - mapping_base;
  PU_Info_subsect_offset(pu, subsection_type) =
    mapping_base - fl->cur_section->shdr.sh_offset;
} /* WN_write_generic_map */


void
WN_write_INT32_map(PU_Info        *pu,
		   WN_MAP          off_map,
		   Output_File    *fl,
		   INT32           subsection_type,
		   WN_MAP          value_map,
		   const char     *subsection_name)
{
  WN_write_generic_map(pu, off_map, fl, subsection_type, value_map,
		       subsection_name, (INT32) 0);
}

void
WN_write_voidptr_map(PU_Info        *pu,
		     WN_MAP          off_map,
		     Output_File    *fl,
		     INT32           subsection_type,
		     WN_MAP          value_map,
		     const char     *subsection_name)
{
  WN_write_generic_map(pu, off_map, fl, subsection_type, value_map,
		       subsection_name, (void *) 0);
}

void
IPA_copy_PU (PU_Info *pu, char *section_base, Output_File *outfile)
{
    char *subsect;
    off_t base;
    char buffer [sizeof (WN)];
    WN *dummy = (WN *) &buffer [0];;
    INT padding;
    Subsect_State state;

    /* make sure we're in the right section */
    if (strcmp(outfile->cur_section->name, MIPS_WHIRL_PU_SECTION) != 0)
	ErrMsg (EC_IR_Scn_Write, "PU", outfile->file_name);

    /* copy the SYMTAB section */
    state = PU_Info_state(pu, WT_SYMTAB);
    if (state == Subsect_Exists)
	Set_PU_Info_state(pu, WT_SYMTAB, Subsect_Written);
    else if (state != Subsect_Written)
	ErrMsg (EC_IR_Scn_Write, "symtab", outfile->file_name);
    subsect = section_base + PU_Info_subsect_offset(pu, WT_SYMTAB);
    outfile->file_size = ir_b_align(outfile->file_size, sizeof(mINT64), 0);
    base = outfile->file_size;
    ir_b_save_buf(subsect, PU_Info_subsect_size(pu, WT_SYMTAB),
		  sizeof(mINT64), 0, outfile);

    PU_Info_subsect_offset(pu, WT_SYMTAB) = 
	base - outfile->cur_section->shdr.sh_offset;

    /* calculate the padding for the TREE section */
    WN_set_opcode(dummy, OPC_FUNC_ENTRY);
    padding = (char *)dummy - (char *)WN_StartAddress(dummy) +
	sizeof(Elf64_Word);

    /* copy the TREE section */
    state = PU_Info_state(pu, WT_TREE);
    if (state == Subsect_Exists)
	Set_PU_Info_state(pu, WT_TREE, Subsect_Written);
    else if (state != Subsect_Written)
	ErrMsg (EC_IR_Scn_Write, "tree", outfile->file_name);
    subsect = section_base + PU_Info_subsect_offset(pu, WT_TREE);
    outfile->file_size = ir_b_align(outfile->file_size, ALIGNOF(WN),
				    padding);
    base = outfile->file_size;
    ir_b_save_buf(subsect, PU_Info_subsect_size(pu, WT_TREE),
		  ALIGNOF(WN), padding, outfile);
    PU_Info_subsect_offset(pu, WT_TREE) =
	base - outfile->cur_section->shdr.sh_offset;

    /* copy the DEPGRAPH section (optional) */
    state = PU_Info_state(pu, WT_DEPGRAPH);
    if (state != Subsect_Missing) {
	if (state == Subsect_Exists)
	    Set_PU_Info_state(pu, WT_DEPGRAPH, Subsect_Written);
	else if (state != Subsect_Written)
	    ErrMsg (EC_IR_Scn_Write, "depgraph", outfile->file_name);
	subsect = section_base + PU_Info_subsect_offset(pu, WT_DEPGRAPH);
	outfile->file_size = ir_b_align(outfile->file_size, sizeof(mINT32), 0);
	base = outfile->file_size;
	ir_b_save_buf(subsect, PU_Info_subsect_size(pu, WT_DEPGRAPH),
		      sizeof(mINT32), 0, outfile);
	PU_Info_subsect_offset(pu, WT_DEPGRAPH) =
	    base - outfile->cur_section->shdr.sh_offset;
    }

    /* copy the PREFETCH section (optional) */
    state = PU_Info_state(pu, WT_PREFETCH);
    if (state != Subsect_Missing) {
	if (state == Subsect_Exists)
	    Set_PU_Info_state(pu, WT_PREFETCH, Subsect_Written);
	else if (state != Subsect_Written)
	    ErrMsg (EC_IR_Scn_Write, "prefetch", outfile->file_name);
	subsect = section_base + PU_Info_subsect_offset(pu, WT_PREFETCH);
	outfile->file_size = ir_b_align(outfile->file_size, sizeof(mINT32), 0);
	base = outfile->file_size;
	ir_b_save_buf(subsect, PU_Info_subsect_size(pu, WT_PREFETCH),
		      sizeof(mINT32), 0, outfile);
	PU_Info_subsect_offset(pu, WT_PREFETCH) =
	    base - outfile->cur_section->shdr.sh_offset;
    }

}

#endif /* BACK_END */


void
WN_write_revision (Output_File *fl)
{
    Section *cur_section;
    int length = strlen (Whirl_Revision);

    cur_section = get_section (0, ELF_COMMENT, fl);

    cur_section->shdr.sh_offset = fl->file_size;

    ir_b_save_buf (Whirl_Revision, length+1, 0, 0, fl);
    
    cur_section->shdr.sh_size = fl->file_size - cur_section->shdr.sh_offset;
    cur_section->shdr.sh_addralign = 1;
    /* override the default section type */
    cur_section->shdr.sh_type = SHT_PROGBITS;

} /* write_revision */


void
WN_close_output (Output_File *fl)
{
    if (fl->output_fd < 0)
	ErrMsg (EC_IR_Close, fl->file_name, EBADF);
    
#ifndef __ALWAYS_USE_64BIT_ELF__
    if (Use_32_Bit_Pointers) {
	Elf32_Shdr strtab_sec;	    /* for section string table */
	UINT64 offset = layout_sections (strtab_sec, fl);

	// Solaris CC workaround
	// write_output() was defined as a template function, now it is not.
	write_output (offset, strtab_sec, fl, ELF32());
    } else 
#endif
    {
	Elf64_Shdr strtab_sec;
	UINT64 e_shoff = layout_sections (strtab_sec, fl);
	write_output (e_shoff, strtab_sec, fl, ELF64());
    }
    
    // eraxxon: unmap output file now for cygwin.  cygwin is
    // necessarily built upon Win32 which requires all other accesses
    // to the file to be closed before it can be changed.
    MUNMAP(fl->map_addr, fl->mapped_size);
    if (ftruncate(fl->output_fd, fl->file_size) != 0) {
         ErrMsg (EC_IR_Close, fl->file_name, errno);
    }
    
    close (fl->output_fd);
    cleanup (fl);

} /* WN_close_output */

void 
WN_close_file (void *this_fl)
{
    Output_File *fl = (Output_File *)this_fl;
    if (fl->output_fd < 0)
	ErrMsg (EC_IR_Close, fl->file_name, EBADF);
    
    // eraxxon: unmap output file now for cygwin.  cygwin is
    // necessarily built upon Win32 which requires all other accesses
    // to the file to be closed before it can be changed.
    MUNMAP(fl->map_addr, fl->mapped_size);
    if (ftruncate(fl->output_fd, fl->file_size) != 0) {
	ErrMsg (EC_IR_Close, fl->file_name, errno);
    }

    close (fl->output_fd);
    cleanup (fl);
}


#ifndef OWN_ERROR_PACKAGE

/*
 * Define common routines for writing all the whirl sections.
 * These routines use the standard compiler error reporting mechanism.
 */

static Output_File *ir_output;

void
Write_PU_Info (PU_Info *pu)
{
    Temporary_Error_Phase ephase("Writing WHIRL file");

    WN_MAP off_map = WN_MAP_UNDEFINED;

    WN_write_symtab (pu, ir_output);

    /* create a map to save the file offsets for certain WN nodes */
#ifdef BACK_END
    if (Write_BE_Maps || Write_ALIAS_CLASS_Map) {
	Current_Map_Tab = PU_Info_maptab(pu);
	MEM_POOL_Push(MEM_local_nz_pool_ptr);
	off_map = WN_MAP32_Create(MEM_local_nz_pool_ptr);
    }
#endif

    WN_write_tree (pu, off_map, ir_output);

#ifdef BACK_END
    if (Write_BE_Maps || Write_ALIAS_CLASS_Map) {

	if (Write_ALIAS_CLASS_Map) {
	  WN_write_INT32_map(pu, off_map, ir_output, WT_ALIAS_CLASS,
			     WN_MAP_ALIAS_CLASS, "alias class map");
	}

	WN_MAP_Delete(off_map);
	MEM_POOL_Pop(MEM_local_nz_pool_ptr);
    }

#endif // BACK_END

}


Output_File *
Open_Output_Info (const char *output_file)
{
    Set_Error_Phase ("Writing WHIRL file" );
    ir_output = WN_open_output (output_file);

    if (!ir_output) {
	ErrMsg (EC_IR_Create, output_file, errno);
    }
    return ir_output;
}

void
Write_Global_Info (PU_Info *pu_tree)
{
    Set_Error_Phase ("Writing WHIRL file" );
    WN_write_PU_Infos (pu_tree, ir_output);


    WN_write_globals (ir_output);

    WN_write_dst(Current_DST, ir_output);

    WN_write_strtab(Index_To_Str (0), STR_Table_Size (), ir_output);

}

void
Close_Output_Info (void)
{
    WN_write_revision (ir_output);

    WN_close_output(ir_output);
}


#if (defined(linux) || defined(__CYGWIN__))
extern "C" void
WN_write_elf_symtab (const void* symtab, UINT64 size, UINT64 entsize,
		     UINT align, Output_File* fl)
{
    // create an ELF_STRTAB section that points to MIPS_WHIRL_STRTAB
    Section* elf_strtab = get_section (0, ELF_STRTAB, fl);
    Section* whirl_strtab = get_section (WT_STRTAB, MIPS_WHIRL_STRTAB, fl);
    elf_strtab->shdr = whirl_strtab->shdr;
    elf_strtab->shdr.sh_type = SHT_STRTAB;
    elf_strtab->shdr.sh_info = 0;

    UINT strtab_idx = elf_strtab - fl->section_list + 1; // shdr[0] is always zero

    Section* cur_section = get_section (0, ELF_SYMTAB, fl);

    fl->file_size = ir_b_align (fl->file_size, align, 0);
    cur_section->shdr.sh_offset = fl->file_size;

    (void) ir_b_save_buf (symtab, size, align, 0, fl);

    cur_section->shdr.sh_type = SHT_SYMTAB;
    cur_section->shdr.sh_size = fl->file_size - cur_section->shdr.sh_offset;
    cur_section->shdr.sh_addralign = align;
    cur_section->shdr.sh_link = strtab_idx;
    cur_section->shdr.sh_entsize = entsize;
} // WN_write_elf_symtab
#endif // linux
#endif // OWN_ERROR_PACKAGE




