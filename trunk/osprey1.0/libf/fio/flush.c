/*

  Copyright (C) 2000, 2001, Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2.1 of the GNU Lesser General Public License 
  as published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU Lesser General Public 
  License along with this program; if not, write the Free Software 
  Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307, 
  USA.

  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/



static char USMID[] = "@(#) libf/fio/flush.c	92.3	11/16/99 15:43:33";

#include <stdio.h>
#include <errno.h>
#include <liberrno.h>
#include "fio.h"
#ifdef	_ABSOFT
#include "ac_sysdep.h"
#else

#if	defined(_LITTLE_ENDIAN)
#ifndef	FILE_FLAG
#define	FILE_FLAG(__f) (__f)->_flag
#endif

#ifndef	IOWRT
#define	IOWRT = _IO_CURRENTLY_PUTTING
#endif

#else		/* LITTLE_ENDIAN */

#ifndef FILE_FLAG
#define FILE_FLAG(__f) (__f)->_flag
#endif

#endif		/* LITTLE_ENDIAN */

#endif

#define FLUSH_ERROR(_ERROR_) {			\
		if (statp)			\
			*rstat	= _ERROR_;	\
		else				\
			_ferr(&cfs, _ERROR_);	\
	}

#define FLUSH_ERROR1(_ERROR_, _DATA_) {		\
		if (statp) {			\
			*rstat	= _ERROR_;	\
			goto flush_done;	\
		}				\
		else				\
			_ferr(&cfs, _ERROR_, _DATA_);\
	}


/*
 * FLUSH return statuses.
 */

#define FLUSH_OK	0	/* successful flush */
#define NOT_SUPPORTED	(-1)	/* this unit does not support FLUSH */

/*
 *	FLUSH	This routine writes to the underlying file any buffered
 *		data for a Fortran file.  FLUSH does not alter the file
 *		position or state of the unit in any other way.
 *
 *	FLUSH is an intrinsic, and the Fortran compiler will always pass
 *	a NULL argument for the istat argument if it is not provided on
 *	the CALL to FLUSH.
 */
void
#ifdef	_UNICOS
FLUSH(
#elif	defined(__mips) || defined(_LITTLE_ENDIAN)
__flush_f90(
#else
flush_(
#endif
	_f_int	*unump,		/* Fortran unit number */
	_f_int	*istat		/* Optional error status parameter */
)
{
	register short	statp;		/* 1 if istat parameter passed */
	int		*rstat;		/* Pointer to return status word */
	int		errn;		/* Error status */
	register unum_t	unum;		/* unit number */
	unit		*cup;
	struct fiostate	cfs;

	unum	= *unump;
	statp	=
#ifdef	_UNICOS
		(_numargs() >= 2)
#else
		(istat != NULL)
#endif
			? 1 : 0;

	rstat	= statp ? istat : &errn;
	*rstat	= FLUSH_OK;	/* Assume FLUSH works */

	STMT_BEGIN(unum, 0, T_FLUSH, NULL, &cfs, cup);	/* lock the unit */

	if (cup == NULL) {
		if (!GOOD_UNUM(unum)) 
			errn	= FEIVUNIT;
		else {
			/*
			 * Ignore FLUSH on unopened reserved unit.
			 */
			if (RSVD_UNUM(unum))
				goto flush_done;

			errn	= FENOTOPN;
		}

		FLUSH_ERROR1(errn, unum);
	}

	if (cup->useq == 0) {	/* If file opened for direct access */
		*rstat	= NOT_SUPPORTED;
		goto flush_done;
	} 

	if ( ! cup->uwrt)	/* If not writing, do nothing */
		goto flush_done;

	switch (cup->ufs) {
		struct ffsw	fstat;	/* ffflush() status */

		case FS_FDC:
			if (__ffflush(cup->ufp.fdc, &fstat) < 0)
				FLUSH_ERROR(fstat.sw_error);
			break;

		case FS_TEXT:
		case STD:
#if	!defined(_LITTLE_ENDIAN)
			if (FILE_FLAG(cup->ufp.std) & _IOWRT)
				if (fflush(cup->ufp.std) == EOF)
					FLUSH_ERROR(errno);
#endif
			break;

		default:
			*rstat	= NOT_SUPPORTED;
	} /* switch */

flush_done:
	STMT_END(cup, T_FLUSH, NULL, &cfs);	/* unlock the unit */

	return;
}

#if	defined(_LITTLE_ENDIAN)
void
flush_( _f_int	*unump)
{
	_f_int	istt;		/* Optional error status is present */
	__flush_f90(unump, &istt);
	return;
}
#endif

#if	defined(__mips) || defined(_LITTLE_ENDIAN)
/* istat receives:
 * 	 0 - on normal return,
 *	-1 - if flush is not supported for this unit,
 *	+n - a positive error number if an error is encountered.
 * If istat is not provided, any error results in program abort.
 */
void
flush_stat_8_(
	_f_int8	*unump,		/* Fortran unit number */
	_f_int8	*istat)		/* Optional error status present */
{
	_f_int	unum;	/* Fortran unit number */
	_f_int	istt;	/* Optional error status is present*/

	unum	= *unump;
	__flush_f90(&unum, &istt);
	*istat	= (_f_int8) istt;

	return;
}

void
flush_stat_4_(
	_f_int	*unump,		/* Fortran unit number */
	_f_int	*istat)		/* Optional error status present */
{
	__flush_f90(unump, istat);

	return;
}

void
flush_stat_4_8_(
	_f_int	*unump,		/* Fortran unit number */
	_f_int8	*istat)		/* Optional error status present */
{
	_f_int	istt;	/* Optional error status is present*/

	__flush_f90(unump, &istt);
	*istat	= (_f_int8) istt;
	return;
}

void
flush_stat_8_4_(
	_f_int8	*unump,		/* Fortran unit number */
	_f_int	*istat)		/* Optional error status present */
{
	_f_int	unum;	/* Fortran unit number */

	unum	= *unump;
	__flush_f90(&unum, istat);
	return;
}

void
flush_f90_4_( _f_int	*unump)		/* Fortran unit number */
{
	_f_int	istat;	/* status word */

	__flush_f90(unump, &istat);
	return;
}

void
flush_f90_8_( _f_int8	*unump)		/* Fortran unit number */
{
	_f_int	istat;		/* status word */
	_f_int	unum;		/* Fortran unit number */

	unum	= *unump;
	__flush_f90(&unum, &istat);
	return;
}

#endif	/* __mips  or _LITTLE_ENDIAN */
