/* $Id: x_stdlib.c,v 1.1 2003-11-04 15:59:37 eraxxon Exp $ */
/* -*-Mode: C;-*- */
/* * BeginRiceCopyright *****************************************************
 * 
 * ******************************************************* EndRiceCopyright */

/*************************** System Include Files ***************************/

#include <stdlib.h>

/**************************** User Include Files ****************************/

#include "x_stdlib.h"

/**************************** Forward Declarations **************************/

/****************************************************************************/

int ux_mkstemp(char *strtemplate)
{ return mkstemp(strtemplate); }

