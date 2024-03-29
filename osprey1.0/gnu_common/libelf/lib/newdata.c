/*
newdata.c - implementation of the elf_newdata(3) function.
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
static const char rcsid[] = "@(#) $Id: newdata.c,v 1.1.1.1 2002-05-22 20:08:57 dsystem Exp $";
#endif /* lint */

Elf_Data*
elf_newdata(Elf_Scn *scn) {
    Scn_Data *sd;

    if (!scn) {
	return NULL;
    }
    elf_assert(scn->s_magic == SCN_MAGIC);
    if (scn->s_index == SHN_UNDEF) {
	seterr(ERROR_NULLSCN);
    }
    else if (!(sd = (Scn_Data*)malloc(sizeof(*sd)))) {
	seterr(ERROR_MEM_SCNDATA);
    }
    else {
	*sd = _elf_data_init;
	sd->sd_data_flags = ELF_F_DIRTY;
	sd->sd_freeme = 1;
	sd->sd_data.d_version = _elf_version;
	if (scn->s_data_n) {
	    scn->s_data_n->sd_link = sd;
	}
	else {
	    scn->s_data_1 = sd;
	}
	scn->s_data_n = sd;
	return &sd->sd_data;
    }
    return NULL;
}
