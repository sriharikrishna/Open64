/* $Id: x_math.c,v 1.1 2003-11-04 15:59:37 eraxxon Exp $ */
/* -*-Mode: C;-*- */
/* * BeginRiceCopyright *****************************************************
 * 
 * ******************************************************* EndRiceCopyright */

/*************************** System Include Files ***************************/

#include <math.h>

/**************************** User Include Files ****************************/

#include "x_math.h"

/**************************** Forward Declarations **************************/

/* Solaris workaround: The Solaris standard C library does not have
 * "float" version of mathematic functions even though they are
 * now in C99.
 * cf. common/com/ia64/targ_const.cxx 
 */
#ifdef _SOLARIS_SOLARIS
inline float hypotf(float x, float y) { return (float)hypot(x, y); }
inline long double hypotl(long double x, long double y) 
  { return hypot((double)x, (double)y); }
#endif

/****************************************************************************/

double c_hypot(double x, double y) 
{ return hypot(x,y); }

float c_hypotf(float x, float y) 
{ return hypotf(x,y); }

long double c_hypotl(long double x, long double y) 
{ return hypotl(x,y); }

