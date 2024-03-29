/*
32.newehdr.c - implementation of the elf{32,64}_newehdr(3) functions.
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
static const char rcsid[] = "@(#) $Id: 32.newehdr.c,v 1.1.1.1 2002-05-22 20:08:56 dsystem Exp $";
#endif /* lint */

static char*
_elf_newehdr(Elf *elf, unsigned cls) {
    size_t size;

    if (!elf) {
	return NULL;
    }
    elf_assert(elf->e_magic == ELF_MAGIC);
    if (elf->e_readable) {
	return _elf_getehdr(elf, cls);
    }
    else if (!elf->e_ehdr) {
	size = _msize(cls, _elf_version, ELF_T_EHDR);
	elf_assert(size);
	if ((elf->e_ehdr = (char*)malloc(size))) {
	    memset(elf->e_ehdr, 0, size);
	    elf->e_free_ehdr = 1;
	    elf->e_ehdr_flags |= ELF_F_DIRTY;
	    elf->e_kind = ELF_K_ELF;
	    elf->e_class = cls;
	    return elf->e_ehdr;
	}
	seterr(ERROR_MEM_EHDR);
    }
    else if (elf->e_class != cls) {
	seterr(ERROR_CLASSMISMATCH);
    }
    else {
	elf_assert(elf->e_kind == ELF_K_ELF);
	return elf->e_ehdr;
    }
    return NULL;
}

Elf32_Ehdr*
elf32_newehdr(Elf *elf) {
    return (Elf32_Ehdr*)_elf_newehdr(elf, ELFCLASS32);
}

#if __LIBELF64

Elf64_Ehdr*
elf64_newehdr(Elf *elf) {
    return (Elf64_Ehdr*)_elf_newehdr(elf, ELFCLASS64);
}

#endif /* __LIBELF64 */
