/* $Id: x_stdio.c,v 1.1 2003-11-04 15:59:37 eraxxon Exp $ */
/* -*-Mode: C;-*- */
/* * BeginRiceCopyright *****************************************************
 * 
 * ******************************************************* EndRiceCopyright */

/*************************** System Include Files ***************************/

#include <stdio.h>

/**************************** User Include Files ****************************/

#include "x_stdio.h"

/**************************** Forward Declarations **************************/

/****************************************************************************/

int ux_fileno(FILE *stream)
{ return fileno(stream); }

FILE *ux_fdopen(int fildes, const char *mode)
{ return fdopen(fildes, mode); }
