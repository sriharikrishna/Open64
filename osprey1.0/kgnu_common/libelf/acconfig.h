/*
acconfig.h - Special definitions for libelf, processed by autoheader.
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

/* @(#) $Id: acconfig.h,v 1.1 2004-05-24 17:11:33 fzhao Exp $ */

/* Define if you want to include extra debugging code */
#undef ENABLE_DEBUG

/* Define if memmove() does not copy overlapping arrays correctly */
#undef HAVE_BROKEN_MEMMOVE

/* Define if you have the catgets function. */
#undef HAVE_CATGETS

/* Define if you have the gettext function. */
#undef HAVE_GETTEXT

/* Define if you have the memset function.  */
#undef HAVE_MEMSET

/* Define if struct nlist is declared in <elf.h> or <sys/elf.h> */
#undef HAVE_STRUCT_NLIST_DECLARATION

/* Define if Elf32_Dyn is declared in <link.h> */
#undef NEED_LINK_H

/* Define to `<elf.h>' or `<sys/elf.h>' if one of them is present */
#undef __LIBELF_HEADER_ELF_H

/* Define if you want 64-bit support (and your system supports it) */
#undef __LIBELF64

/* Define if you want 64-bit support, and are running IRIX */
#undef __LIBELF64_IRIX

/* Define if you want 64-bit support, and are running Linux */
#undef __LIBELF64_LINUX

/* Define to a 64-bit signed integer type if one exists */
#undef __libelf_i64_t

/* Define to a 64-bit unsigned integer type if one exists */
#undef __libelf_u64_t

/* Define to a 32-bit signed integer type if one exists */
#undef __libelf_i32_t

/* Define to a 32-bit unsigned integer type if one exists */
#undef __libelf_u32_t

/* Define to a 16-bit signed integer type if one exists */
#undef __libelf_i16_t

/* Define to a 16-bit unsigned integer type if one exists */
#undef __libelf_u16_t
