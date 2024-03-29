/* Support for the generic parts of PE/PEI; common header information.
   Copyright 1995, 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
   Written by Cygnus Solutions.

This file is part of BFD, the Binary File Descriptor library.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/*
Most of this hacked by  Steve Chamberlain,
			sac@cygnus.com

PE/PEI rearrangement (and code added): Donn Terry
				       Softway Systems, Inc.
*/

/* Hey look, some documentation [and in a place you expect to find it]!

   The main reference for the pei format is "Microsoft Portable Executable
   and Common Object File Format Specification 4.1".  Get it if you need to
   do some serious hacking on this code.

   Another reference:
   "Peering Inside the PE: A Tour of the Win32 Portable Executable
   File Format", MSJ 1994, Volume 9.

   The *sole* difference between the pe format and the pei format is that the
   latter has an MSDOS 2.0 .exe header on the front that prints the message
   "This app must be run under Windows." (or some such).
   (FIXME: Whether that statement is *really* true or not is unknown.
   Are there more subtle differences between pe and pei formats?
   For now assume there aren't.  If you find one, then for God sakes
   document it here!)

   The Microsoft docs use the word "image" instead of "executable" because
   the former can also refer to a DLL (shared library).  Confusion can arise
   because the `i' in `pei' also refers to "image".  The `pe' format can
   also create images (i.e. executables), it's just that to run on a win32
   system you need to use the pei format.

   FIXME: Please add more docs here so the next poor fool that has to hack
   on this code has a chance of getting something accomplished without
   wasting too much time.
*/

#ifndef GET_FCN_LNNOPTR
#define GET_FCN_LNNOPTR(abfd, ext) \
     bfd_h_get_32(abfd, (bfd_byte *) ext->x_sym.x_fcnary.x_fcn.x_lnnoptr)
#endif

#ifndef GET_FCN_ENDNDX
#define GET_FCN_ENDNDX(abfd, ext)  \
	bfd_h_get_32(abfd, (bfd_byte *) ext->x_sym.x_fcnary.x_fcn.x_endndx)
#endif

#ifndef PUT_FCN_LNNOPTR
#define PUT_FCN_LNNOPTR(abfd, in, ext)  bfd_h_put_32(abfd,  in, (bfd_byte *) ext->x_sym.x_fcnary.x_fcn.x_lnnoptr)
#endif
#ifndef PUT_FCN_ENDNDX
#define PUT_FCN_ENDNDX(abfd, in, ext) bfd_h_put_32(abfd, in, (bfd_byte *) ext->x_sym.x_fcnary.x_fcn.x_endndx)
#endif
#ifndef GET_LNSZ_LNNO
#define GET_LNSZ_LNNO(abfd, ext) bfd_h_get_16(abfd, (bfd_byte *) ext->x_sym.x_misc.x_lnsz.x_lnno)
#endif
#ifndef GET_LNSZ_SIZE
#define GET_LNSZ_SIZE(abfd, ext) bfd_h_get_16(abfd, (bfd_byte *) ext->x_sym.x_misc.x_lnsz.x_size)
#endif
#ifndef PUT_LNSZ_LNNO
#define PUT_LNSZ_LNNO(abfd, in, ext) bfd_h_put_16(abfd, in, (bfd_byte *)ext->x_sym.x_misc.x_lnsz.x_lnno)
#endif
#ifndef PUT_LNSZ_SIZE
#define PUT_LNSZ_SIZE(abfd, in, ext) bfd_h_put_16(abfd, in, (bfd_byte*) ext->x_sym.x_misc.x_lnsz.x_size)
#endif
#ifndef GET_SCN_SCNLEN
#define GET_SCN_SCNLEN(abfd,  ext) bfd_h_get_32(abfd, (bfd_byte *) ext->x_scn.x_scnlen)
#endif
#ifndef GET_SCN_NRELOC
#define GET_SCN_NRELOC(abfd,  ext) bfd_h_get_16(abfd, (bfd_byte *)ext->x_scn.x_nreloc)
#endif
#ifndef GET_SCN_NLINNO
#define GET_SCN_NLINNO(abfd, ext)  bfd_h_get_16(abfd, (bfd_byte *)ext->x_scn.x_nlinno)
#endif
#ifndef PUT_SCN_SCNLEN
#define PUT_SCN_SCNLEN(abfd,in, ext) bfd_h_put_32(abfd, in, (bfd_byte *) ext->x_scn.x_scnlen)
#endif
#ifndef PUT_SCN_NRELOC
#define PUT_SCN_NRELOC(abfd,in, ext) bfd_h_put_16(abfd, in, (bfd_byte *)ext->x_scn.x_nreloc)
#endif
#ifndef PUT_SCN_NLINNO
#define PUT_SCN_NLINNO(abfd,in, ext)  bfd_h_put_16(abfd,in, (bfd_byte  *) ext->x_scn.x_nlinno)
#endif
#ifndef GET_LINENO_LNNO
#define GET_LINENO_LNNO(abfd, ext) bfd_h_get_16(abfd, (bfd_byte *) (ext->l_lnno));
#endif
#ifndef PUT_LINENO_LNNO
#define PUT_LINENO_LNNO(abfd,val, ext) bfd_h_put_16(abfd,val,  (bfd_byte *) (ext->l_lnno));
#endif

/* The f_symptr field in the filehdr is sometimes 64 bits.  */
#ifndef GET_FILEHDR_SYMPTR
#define GET_FILEHDR_SYMPTR bfd_h_get_32
#endif
#ifndef PUT_FILEHDR_SYMPTR
#define PUT_FILEHDR_SYMPTR bfd_h_put_32
#endif

/* Some fields in the aouthdr are sometimes 64 bits.  */
#ifndef GET_AOUTHDR_TSIZE
#define GET_AOUTHDR_TSIZE bfd_h_get_32
#endif
#ifndef PUT_AOUTHDR_TSIZE
#define PUT_AOUTHDR_TSIZE bfd_h_put_32
#endif
#ifndef GET_AOUTHDR_DSIZE
#define GET_AOUTHDR_DSIZE bfd_h_get_32
#endif
#ifndef PUT_AOUTHDR_DSIZE
#define PUT_AOUTHDR_DSIZE bfd_h_put_32
#endif
#ifndef GET_AOUTHDR_BSIZE
#define GET_AOUTHDR_BSIZE bfd_h_get_32
#endif
#ifndef PUT_AOUTHDR_BSIZE
#define PUT_AOUTHDR_BSIZE bfd_h_put_32
#endif
#ifndef GET_AOUTHDR_ENTRY
#define GET_AOUTHDR_ENTRY bfd_h_get_32
#endif
#ifndef PUT_AOUTHDR_ENTRY
#define PUT_AOUTHDR_ENTRY bfd_h_put_32
#endif
#ifndef GET_AOUTHDR_TEXT_START
#define GET_AOUTHDR_TEXT_START bfd_h_get_32
#endif
#ifndef PUT_AOUTHDR_TEXT_START
#define PUT_AOUTHDR_TEXT_START bfd_h_put_32
#endif
#ifndef GET_AOUTHDR_DATA_START
#define GET_AOUTHDR_DATA_START bfd_h_get_32
#endif
#ifndef PUT_AOUTHDR_DATA_START
#define PUT_AOUTHDR_DATA_START bfd_h_put_32
#endif

/* Some fields in the scnhdr are sometimes 64 bits.  */
#ifndef GET_SCNHDR_PADDR
#define GET_SCNHDR_PADDR bfd_h_get_32
#endif
#ifndef PUT_SCNHDR_PADDR
#define PUT_SCNHDR_PADDR bfd_h_put_32
#endif
#ifndef GET_SCNHDR_VADDR
#define GET_SCNHDR_VADDR bfd_h_get_32
#endif
#ifndef PUT_SCNHDR_VADDR
#define PUT_SCNHDR_VADDR bfd_h_put_32
#endif
#ifndef GET_SCNHDR_SIZE
#define GET_SCNHDR_SIZE bfd_h_get_32
#endif
#ifndef PUT_SCNHDR_SIZE
#define PUT_SCNHDR_SIZE bfd_h_put_32
#endif
#ifndef GET_SCNHDR_SCNPTR
#define GET_SCNHDR_SCNPTR bfd_h_get_32
#endif
#ifndef PUT_SCNHDR_SCNPTR
#define PUT_SCNHDR_SCNPTR bfd_h_put_32
#endif
#ifndef GET_SCNHDR_RELPTR
#define GET_SCNHDR_RELPTR bfd_h_get_32
#endif
#ifndef PUT_SCNHDR_RELPTR
#define PUT_SCNHDR_RELPTR bfd_h_put_32
#endif
#ifndef GET_SCNHDR_LNNOPTR
#define GET_SCNHDR_LNNOPTR bfd_h_get_32
#endif
#ifndef PUT_SCNHDR_LNNOPTR
#define PUT_SCNHDR_LNNOPTR bfd_h_put_32
#endif

#ifdef COFF_WITH_PEP64

#define GET_OPTHDR_IMAGE_BASE bfd_h_get_64
#define PUT_OPTHDR_IMAGE_BASE bfd_h_put_64
#define GET_OPTHDR_SIZE_OF_STACK_RESERVE bfd_h_get_64
#define PUT_OPTHDR_SIZE_OF_STACK_RESERVE bfd_h_put_64
#define GET_OPTHDR_SIZE_OF_STACK_COMMIT bfd_h_get_64
#define PUT_OPTHDR_SIZE_OF_STACK_COMMIT bfd_h_put_64
#define GET_OPTHDR_SIZE_OF_HEAP_RESERVE bfd_h_get_64
#define PUT_OPTHDR_SIZE_OF_HEAP_RESERVE bfd_h_put_64
#define GET_OPTHDR_SIZE_OF_HEAP_COMMIT bfd_h_get_64
#define PUT_OPTHDR_SIZE_OF_HEAP_COMMIT bfd_h_put_64
#define GET_PDATA_ENTRY bfd_get_64

#else /* !COFF_WITH_PEP64 */

#define GET_OPTHDR_IMAGE_BASE bfd_h_get_32
#define PUT_OPTHDR_IMAGE_BASE bfd_h_put_32
#define GET_OPTHDR_SIZE_OF_STACK_RESERVE bfd_h_get_32
#define PUT_OPTHDR_SIZE_OF_STACK_RESERVE bfd_h_put_32
#define GET_OPTHDR_SIZE_OF_STACK_COMMIT bfd_h_get_32
#define PUT_OPTHDR_SIZE_OF_STACK_COMMIT bfd_h_put_32
#define GET_OPTHDR_SIZE_OF_HEAP_RESERVE bfd_h_get_32
#define PUT_OPTHDR_SIZE_OF_HEAP_RESERVE bfd_h_put_32
#define GET_OPTHDR_SIZE_OF_HEAP_COMMIT bfd_h_get_32
#define PUT_OPTHDR_SIZE_OF_HEAP_COMMIT bfd_h_put_32
#define GET_PDATA_ENTRY bfd_get_32

#endif /* !COFF_WITH_PEP64 */

/* These functions are architecture dependent, and are in peicode.h:
   coff_swap_reloc_in
   int coff_swap_reloc_out
   coff_swap_filehdr_in
   coff_swap_scnhdr_in
   pe_mkobject
   pe_mkobject_hook  */

/* The functions described below are common across all PE/PEI
   implementations architecture types, and actually appear in
   peigen.c.  */

void _bfd_pei_swap_sym_in PARAMS ((bfd*, PTR, PTR));
#define coff_swap_sym_in _bfd_pei_swap_sym_in

unsigned int _bfd_pei_swap_sym_out PARAMS ((bfd*, PTR, PTR));
#define coff_swap_sym_out _bfd_pei_swap_sym_out

void _bfd_pei_swap_aux_in PARAMS ((bfd *, PTR, int, int, int, int, PTR));
#define coff_swap_aux_in _bfd_pei_swap_aux_in

unsigned int _bfd_pei_swap_aux_out \
  PARAMS ((bfd *, PTR, int, int, int, int, PTR));
#define coff_swap_aux_out _bfd_pei_swap_aux_out

void _bfd_pei_swap_lineno_in PARAMS ((bfd*, PTR, PTR));
#define coff_swap_lineno_in _bfd_pei_swap_lineno_in

unsigned int _bfd_pei_swap_lineno_out PARAMS ((bfd*, PTR, PTR));
#define coff_swap_lineno_out _bfd_pei_swap_lineno_out

void _bfd_pei_swap_aouthdr_in PARAMS ((bfd*, PTR, PTR));
#define coff_swap_aouthdr_in _bfd_pei_swap_aouthdr_in

unsigned int _bfd_pei_swap_aouthdr_out PARAMS ((bfd *, PTR, PTR));
#define coff_swap_aouthdr_out _bfd_pei_swap_aouthdr_out

unsigned int _bfd_pei_swap_scnhdr_out PARAMS ((bfd *, PTR, PTR));
#define coff_swap_scnhdr_out _bfd_pei_swap_scnhdr_out

boolean _bfd_pe_print_private_bfd_data_common PARAMS ((bfd *, PTR));

boolean _bfd_pe_bfd_copy_private_bfd_data_common PARAMS ((bfd *, bfd *));

void _bfd_pe_get_symbol_info PARAMS ((bfd *, asymbol *, symbol_info *));

boolean _bfd_pei_final_link_postscript
  PARAMS ((bfd *, struct coff_final_link_info *));

#ifndef coff_final_link_postscript
#define coff_final_link_postscript _bfd_pei_final_link_postscript
#endif
/* The following are needed only for ONE of pe or pei, but don't
   otherwise vary; peicode.h fixes up ifdefs but we provide the
   prototype.  */

unsigned int _bfd_pe_only_swap_filehdr_out PARAMS ((bfd*, PTR, PTR));
unsigned int _bfd_pei_only_swap_filehdr_out PARAMS ((bfd*, PTR, PTR));
boolean _bfd_pe_bfd_copy_private_section_data
  PARAMS ((bfd *, asection *, bfd *, asection *));
