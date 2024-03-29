#!/bin/csh -f
#
#
#  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.
#
#  This program is free software; you can redistribute it and/or modify it
#  under the terms of version 2 of the GNU General Public License as
#  published by the Free Software Foundation.
#
#  This program is distributed in the hope that it would be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
#
#  Further, this software is distributed without any warranty that it is
#  free of the rightful claim of any third person regarding infringement 
#  or the like.  Any license provided herein, whether implied or 
#  otherwise, applies only to this software file.  Patent licenses, if 
#  any, provided herein do not apply to combinations of this program with 
#  other software, or any other product whatsoever.  
#
#  You should have received a copy of the GNU General Public License along
#  with this program; if not, write the Free Software Foundation, Inc., 59
#  Temple Place - Suite 330, Boston MA 02111-1307, USA.
#
#  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
#  Mountain View, CA 94043, or:
#
#  http://www.sgi.com
#
#  For further information regarding this notice, see:
#
#  http://oss.sgi.com/projects/GenInfo/NoticeExplan
#
#

### ====================================================================
### ====================================================================
### Module: gen_swp_incr_list.csh
### $Revision: 1.1.1.1 $
### $Date: 2002-05-22 20:06:25 $
### $Author: dsystem $
### $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/cg/gen_swp_incr_list.csh,v $
### Revision history:
###   dd-mm-yy
###
### Usage:      gen_swp_incr_list MTP_BIN
###
###     Generate the swp_incr_list.[ch] module.  The argument is the
###     MTP_BIN directory.  We do this in a file so the make rule can
###     depend on and it can be rebuilt when the procedure changes
###
### ====================================================================
### ====================================================================



csh -f $1/gen_x_list.csh    'struct swp_loh_incr *'                    \
                            'SWP_LOH_INCR'                             \
                            'defs.h'                                   \
                            'mempool.h'                                 \
                            'swp_loh_incr_list.h'


