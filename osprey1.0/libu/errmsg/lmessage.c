/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

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


#pragma ident "@(#) libu/errmsg/lmessage.c	92.4	10/14/99 17:05:30"
#include <liberrno.h>
#include <errno.h>
#include <fcntl.h>

#if !defined(_ABSOFT)
# include <nl_types.h>
#else
# include "ac_msg_type.h"
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if defined(_ABSOFT) && defined(TARGET_NT)
# include <siohdr.h>
#endif

#if defined(_LITTLE_ENDIAN)
# include <cray/nlcatmsg.h>
#endif

#ifdef	_UNICOS

# include <tapereq.h>

 extern	char	**_argv;
# define CMDNAME (*_argv)

#else

# include <cray/portdefs.h>
# if defined(_LITTLE_ENDIAN)
#  include <cray/nlcatmsg.h>
# endif

# define CMDNAME ""

#endif

/*
 *	_lmessage - run-time library message processor
 *
 *		errn		Message number
 *		severity	Pointer to message severity; NULL if
 *				unrecoverable error message
 */

void
_lmessage(int errn, char *severity, va_list args)
{
	register int	flmn;		/* library message number 	*/
	char		mbuf[MAXMLN+1];	/* Message buffer		*/
	char		tbuf[MAXMLN+1];	/* Temporary message buffer	*/
	char		*mcnm;		/* Pointer to msg. catalog name	*/
	char		*bmsg;		/* Pointer to back-up message	*/
	char		*smsg;		/* Pointer to severity text	*/
	nl_catd		mcfd;		/* Message catalog descriptor	*/
#ifdef	__mips
	static nl_catd	lib_mcfd;	/* lib msg cat file descriptor	*/
	static int	lib_mcfd_opnd = 0; /* lib msg cat already open flag */
	int		lib_mcat = 0;	/* lib msg catalog indicator	*/
#endif	/* __mips */

#ifdef	_UNICOS
	if (_numargs() == 0)
		(void) abort();
#endif

	mbuf[0]	= '\n';
	mbuf[1]	= '\0';
	bmsg	= NULL;		/* Assume no back-up message */
	smsg	= "";		/* Assume unset severity */
	flmn	= errn;

	/*
	 * Map -1 errors to FERDPEOF (temporarily, we hope)
	 * Map all negative errors to positive errors.
	 */

	if (flmn < 0)
		if (flmn == -1)
			flmn	= FERDPEOF;
		else
			flmn	= -flmn;

	if (flmn < BASE) {
		mcnm	= "sys";
		smsg	= "UNRECOVERABLE error on system request";
#ifdef	_UNICOS
		/*
		 * Provide a back-up error message for those errors
		 * where we expect that the message system will fail
		 * to retrieve a message.
		 */

		if (flmn == ENOMEM)
			bmsg	= "Unable to allocate memory.";

		if (flmn == EMFILE)
			bmsg	= "Too many open files.";
#else
		/*
		 * Use strerror() to provide a back-up error message
		 * for 'sys' errors on non-UNICOS systems.  We don't
		 * want to load strerror() on UNICOS systems, however.
		 */

		bmsg	= strerror(flmn);
#endif
	}
	else
		if (flmn < (BASE+999) ||
		    (flmn >= FDC_ERRB && flmn < (FDC_ERRB+999))) {
			mcnm	= FEMCN;	/* Library catalog name */
#ifdef	__mips
			lib_mcat	= 1;
#endif	/* __mips */
			smsg	= "UNRECOVERABLE library error";

			/*
			 * Provide a minimal back-up message for
			 * out-of-memory errors.
			 */

			if (flmn == FENOMEMY || flmn == FEFMTMEM ||
			    flmn == FDC_ERR_NOMEM)
				bmsg	= "Unable to allocate memory.";
		}
#ifdef	_UNICOS
		else
			if (flmn >= ETFIRST && flmn <= ETLAST) {
				mcnm	= "tape";
				smsg	= "UNRECOVERABLE tape error";
				bmsg	= "Consult Appendix C, Tape Subsystem User's Guide, SG-2051";
			}
#endif
			else {
				mcnm	= "unknown";
				smsg	= "UNRECOVERABLE";
				bmsg	= "Unrecognized error number";
			}

	if (bmsg != NULL)	/* Set back-up message, if extant */
		(void) strcpy(&mbuf[1], bmsg);

	if (severity != NULL)
		smsg	= severity;

	/* Open the message catalog */

#if defined(__sgi)
	/* if this is the library message catalog, then... */ 
	if (lib_mcat) {
		if (lib_mcfd_opnd) {
			/* set lib msg cat fd to opened lib msg cat fd */
			mcfd	= lib_mcfd;
		} else {
			/* message catalog not opened yet, do the following:
			 * 1. open it
			 * 2. save the lib msg cat fd
			 * 3. set flag indicating it was opened
			 */
			mcfd	= catopen(mcnm, 0);
			lib_mcfd	= mcfd;
			lib_mcfd_opnd	= 1;
		}
	} else {
		/* not the lib message catalog, open catalog */
		mcfd	= catopen(mcnm, 0);
	}
#else	/* __sgi */
	mcfd	= catopen(mcnm, 0);
#endif	/* __sgi */

	/* Retrieve the raw message text */

	(void) catgetmsg(mcfd, NL_MSGSET, flmn, &mbuf[1], MAXMLN);

	/* Edit the message */

	(void) vsprintf(tbuf, &mbuf[1], args);

	/* Format the message */

	(void) catmsgfmt(CMDNAME, mcnm, flmn, smsg, tbuf, &mbuf[1],
			 MAXMLN, NULL, NULL);

	/* Print the formatted message */
	fwrite(mbuf, strlen(mbuf), 1, stderr);

	return;
}
