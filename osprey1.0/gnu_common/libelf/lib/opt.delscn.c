/*
opt.delscn.c - implementation of the elf_delscn(3) function.
Copyright (C) 1995 - 1998 Michael Riepe <michael@stud.uni-hannover.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <private.h>

#ifndef lint
static const char rcsid[] = "@(#) $Id: opt.delscn.c,v 1.1.1.1 2002-05-22 20:08:57 dsystem Exp $";
#endif /* lint */

static size_t
_newindex(size_t old, size_t index) {
    return old == index ? SHN_UNDEF : (old > index ? old - 1 : old);
}

static void
_elf32_update_shdr(Elf *elf, size_t index) {
    Elf32_Shdr *shdr;
    Elf_Scn *scn;

    ((Elf32_Ehdr*)elf->e_ehdr)->e_shnum = elf->e_scn_n->s_index + 1;
    for (scn = elf->e_scn_1; scn; scn = scn->s_link) {
	shdr = &scn->s_shdr32;
	switch (shdr->sh_type) {
	    case SHT_REL:
	    case SHT_RELA:
		shdr->sh_info = _newindex(shdr->sh_info, index);
		/* fall through */
	    case SHT_DYNSYM:
	    case SHT_DYNAMIC:
	    case SHT_HASH:
	    case SHT_SYMTAB:
		shdr->sh_link = _newindex(shdr->sh_link, index);
		/* fall through */
	    default:
		break;
	}
    }
}

#if __LIBELF64

static void
_elf64_update_shdr(Elf *elf, size_t index) {
    Elf64_Shdr *shdr;
    Elf_Scn *scn;

    ((Elf64_Ehdr*)elf->e_ehdr)->e_shnum = elf->e_scn_n->s_index + 1;
    for (scn = elf->e_scn_1; scn; scn = scn->s_link) {
	shdr = &scn->s_shdr64;
	switch (shdr->sh_type) {
	    case SHT_REL:
	    case SHT_RELA:
		shdr->sh_info = _newindex(shdr->sh_info, index);
		/* fall through */
	    case SHT_DYNSYM:
	    case SHT_DYNAMIC:
	    case SHT_HASH:
	    case SHT_SYMTAB:
		shdr->sh_link = _newindex(shdr->sh_link, index);
		/* fall through */
	    default:
		break;
	}
    }
}

#endif /* __LIBELF64 */

size_t
elf_delscn(Elf *elf, Elf_Scn *scn) {
    Elf_Scn *pscn;
    Scn_Data *sd;
    Scn_Data *tmp;
    size_t index;

    if (!elf || !scn) {
	return SHN_UNDEF;
    }
    elf_assert(elf->e_magic == ELF_MAGIC);
    elf_assert(scn->s_magic == SCN_MAGIC);
    elf_assert(elf->e_ehdr);
    if (scn->s_elf != elf) {
	seterr(ERROR_ELFSCNMISMATCH);
	return SHN_UNDEF;
    }
    elf_assert(elf->e_scn_1);
    if (scn == elf->e_scn_1) {
	seterr(ERROR_NULLSCN);
	return SHN_UNDEF;
    }

    /*
     * Find previous section.
     */
    for (pscn = elf->e_scn_1; pscn->s_link; pscn = pscn->s_link) {
	if (pscn->s_link == scn) {
	    break;
	}
    }
    if (pscn->s_link != scn) {
	seterr(ERROR_ELFSCNMISMATCH);
	return SHN_UNDEF;
    }
    /*
     * Unlink section.
     */
    if (elf->e_scn_n == scn) {
	elf->e_scn_n = pscn;
    }
    pscn->s_link = scn->s_link;
    index = scn->s_index;
    /*
     * Free section descriptor and data.
     */
    for (sd = scn->s_data_1; sd; sd = tmp) {
	tmp = sd->sd_link;
	if (sd->sd_free_data && sd->sd_memdata) {
	    free(sd->sd_memdata);
	}
	if (sd->sd_freeme) {
	    free(sd);
	}
    }
    if ((sd = scn->s_rawdata)) {
	if (sd->sd_free_data && sd->sd_memdata) {
	    free(sd->sd_memdata);
	}
	if (sd->sd_freeme) {
	    free(sd);
	}
    }
    if (scn->s_freeme) {
	elf_assert(scn->s_index > 0);
	free(scn);
    }
    /*
     * Adjust section indices.
     */
    for (scn = pscn->s_link; scn; scn = scn->s_link) {
	elf_assert(scn->s_index > index);
	scn->s_index--;
    }
    /*
     * Adjust ELF header and well-known section headers.
     */
    if (elf->e_class == ELFCLASS32) {
	_elf32_update_shdr(elf, index);
	return index;
    }
#if __LIBELF64
    else if (elf->e_class == ELFCLASS64) {
	_elf64_update_shdr(elf, index);
	return index;
    }
#endif /* __LIBELF64 */
    else if (valid_class(elf->e_class)) {
	seterr(ERROR_UNIMPLEMENTED);
    }
    else {
	seterr(ERROR_UNKNOWN_CLASS);
    }
    return SHN_UNDEF;
}
