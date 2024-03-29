//-*-c++-*-
// ====================================================================
// ====================================================================
//
// Module: opt_pch.h
// $Revision: 1.1.1.1 $
// $Date: 2002-05-22 20:06:50 $
// $Author: dsystem $
// $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/opt/opt_pch.h,v $
//
// ====================================================================
//
// Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of version 2 of the GNU General Public License as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it would be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// Further, this software is distributed without any warranty that it
// is free of the rightful claim of any third person regarding
// infringement  or the like.  Any license provided herein, whether
// implied or otherwise, applies only to this software file.  Patent
// licenses, if any, provided herein do not apply to combinations of
// this program with other software, or any other product whatsoever.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
//
// Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
// Mountain View, CA 94043, or:
//
// http://www.sgi.com
//
// For further information regarding this notice, see:
//
// http://oss.sgi.com/projects/GenInfo/NoticeExplan
//
// ====================================================================
// ====================================================================


// All system headers included directly from OPT

#include <alloca.h>

#include "defs.h"
#include "config_targ.h"
#include "errors.h"
#include "erglob.h"
#include "region_util.h"
#include "symtab.h"
#include "tracing.h"
#include "wn.h"

#include "cxx_base.h"
#include "cxx_memory.h"
#include "cxx_template.h"

#include "opt_defs.h"
#include "opt_array.h"
#include "opt_base.h"
#include "opt_config.h"
#include "opt_htable.h"
#include "opt_sym.h"
#include "opt_util.h"
#include "opt_wn.h"

#include "opt_bb.h"
#include "opt_cfg.h"
#include "opt_points_to.h"
#include "opt_ssa.h"
#include "opt_cvtl_rule.h"
