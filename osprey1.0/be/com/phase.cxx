/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if 
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston MA 02111-1307, USA.

  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/


/* ====================================================================
 * ====================================================================
 *
 * Module: phase.c
 * $Revision: 1.1.1.1 $
 * $Date: 2002-05-22 20:06:32 $
 * $Author: dsystem $
 * $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/com/phase.cxx,v $
 *
 * Revision history:
 *  17-Feb -95 - Original Version
 *
 * Description:
 *  Phase-specific functions and data structures that the common driver
 *  need to know.
 *
 * ====================================================================
 * ====================================================================
 */

#include "defs.h"
#include "phase.h"

PHASE_SPECIFIC_OPTION_GROUP phase_ogroup_table[] = {
    {"SWP", 3, PHASE_CG},
    {"GRA", 3, PHASE_CG},
    {"CG", 2, PHASE_CG},
    {"GCM", 3, PHASE_CG},
    {"IPL", 3, PHASE_IPL},
    {NULL}			    /* list terminator */
};
