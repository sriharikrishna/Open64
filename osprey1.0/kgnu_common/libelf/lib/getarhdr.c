/*
getarhdr.c - implementation of the elf_getarhdr(3) function.
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
static const char rcsid[] = "@(#) $Id: getarhdr.c,v 1.1 2004-05-24 17:12:14 fzhao Exp $";
#endif /* lint */

Elf_Arhdr*
elf_getarhdr(Elf *elf) {
    if (!elf) {
	return NULL;
    }
    elf_assert(elf->e_magic == ELF_MAGIC);
    if (elf->e_arhdr) {
	return elf->e_arhdr;
    }
    seterr(ERROR_NOTARCHIVE);
    return NULL;
}
