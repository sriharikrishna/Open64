/* $Id: x_string.c,v 1.2 2003-11-28 15:44:47 eraxxon Exp $ */
/* -*-Mode: C;-*- */
/* * BeginRiceCopyright *****************************************************
 * 
 * ******************************************************* EndRiceCopyright */

/*************************** System Include Files ***************************/

#include <string.h>

/**************************** User Include Files ****************************/

#include "x_string.h"

/**************************** Forward Declarations **************************/

/****************************************************************************/

int ux_strcasecmp(const char *s1, const char *s2) 
{ return strcasecmp(s1, s2); }

int ux_strncasecmp(const char *s1, const char *s2, size_t n)
{ return strncasecmp(s1, s2, n); }


char *ux_strdup(const char *s1)
{ return strdup(s1); }


void *ux_memccpy(void * /*restrict*/ s1, const void * /*restrict*/ s2, 
		int c, size_t n)
{ return memccpy(s1, s2, c, n); }
