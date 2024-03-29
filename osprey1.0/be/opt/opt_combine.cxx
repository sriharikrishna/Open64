//-*-c++-*-
// ====================================================================
// ====================================================================
//
// Module: opt_combine.cxx
// $Revision: 1.1.1.1 $
// $Date: 2002-05-22 20:06:47 $
// $Author: dsystem $
// $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/be/opt/opt_combine.cxx,v $
//
// Revision history:
//  12-SEP-94 shin - Original Version
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
//
// Description:
//
// Combines (or changes) expessions to enable other optimizations
//
// ====================================================================
// ====================================================================

#ifdef USE_PCH
#include "opt_pch.h"
#endif // USE_PCH
#pragma hdrstop


#ifdef _KEEP_RCS_ID
#define opt_combine_CXX	"opt_combine.cxx"
static char *rcs_id = 	opt_combine_CXX"$Revision: 1.1.1.1 $";
#endif /* _KEEP_RCS_ID */

#include <sys/types.h>
#include "defs.h"
#include "config.h"
#include "config_opt.h"
#include "mempool.h"
#include "tracing.h"
#include "opt_base.h"

#include "wn.h"
#include "wutil.h"
#include "opt_config.h"
#include "opt_defs.h"

#include "opt_combine.h"


// ====================================================================
// handle the DIV operator
// ====================================================================

static BOOL
Combine_div_operator( WN *old_wn, WN **new_wn, OPCODE old_wn_opc )
{
  Is_True( OPCODE_operator(old_wn_opc) == OPR_DIV,
    ("Combine_div_operator: bad op: %s", OPCODE_name(old_wn_opc)) );

  const MTYPE rtype = OPCODE_rtype(old_wn_opc);
  const MTYPE desc  = OPCODE_desc(old_wn_opc);

  if ( MTYPE_is_float(rtype) || MTYPE_is_complex(rtype) ) {
    // Transform:  DIV        MPY
    //             a b  =>   a  RECIP
    //                            b
    // TODO:  The case where a=1 can be done as:
    // Transform:  DIV        RECIP
    //             1 b  =>      b
    // That case is valid on Recip_Allowed, which is less harmful.
    if ( Div_Split_Allowed ) {
      const OPCODE recip_opc = OPCODE_make_op(OPR_RECIP,rtype,desc);
      WN *recip = WN_CreateExp1(recip_opc, WN_kid(old_wn,1));
      const OPCODE mpy_opc = OPCODE_make_op(OPR_MPY,rtype,desc);
      WN *mpy = WN_CreateExp2(mpy_opc, WN_kid0(old_wn), recip);
      *new_wn = mpy;
      return TRUE;
    }
  }
  else if ( WOPT_Enable_DIVREM && MTYPE_is_integral(rtype) ) {
    // Transform:  DIV        DIVPART
    //             a b  =>    DIVREM
    //                        a    b
    const OPCODE divrem_opc = OPCODE_make_op(OPR_DIVREM,rtype,desc);
    WN *divrem = WN_CreateExp2(divrem_opc, WN_kid0(old_wn),WN_kid1(old_wn));
    const OPCODE divpart_opc = OPCODE_make_op(OPR_DIVPART,rtype,desc);
    WN *divpart = WN_CreateExp1(divpart_opc, divrem);
    *new_wn = divpart;
    return TRUE;
  }

  // if make it here, must not have done anything
  return FALSE;
}

// ====================================================================
// handle the INTRINSIC_OP operator
// ====================================================================

static BOOL
Combine_intrinsic_operator( WN *old_wn, WN **new_wn, OPCODE old_wn_opc )
{
  Is_True( OPCODE_operator(old_wn_opc) == OPR_INTRINSIC_OP,
    ("Combine_intrinsic_operator: bad op: %s", OPCODE_name(old_wn_opc)) );

  const MTYPE rtype = OPCODE_rtype(old_wn_opc);
  const MTYPE desc  = OPCODE_desc(old_wn_opc);
  const INTRINSIC intr = (INTRINSIC)WN_intrinsic(old_wn);
  OPCODE new_intr_opc;
  INTRINSIC new_intr;
  OPCODE  new_complex;

  switch ( intr ) {

    //
    // transform  sin(x) ==> imagpart(cis(x))
    //
    case INTRN_F4SIN:
      new_intr_opc = OPC_C4INTRINSIC_OP;
      new_intr = INTRN_F4CIS; new_complex = OPC_F4IMAGPART;
      goto handle_sin;
    case INTRN_F8SIN:
      new_intr_opc = OPC_C8INTRINSIC_OP;
      new_intr = INTRN_F8CIS; new_complex = OPC_F8IMAGPART;
      goto handle_sin;
    case INTRN_FQSIN:
      new_intr_opc = OPC_CQINTRINSIC_OP;
      new_intr = INTRN_FQCIS; new_complex = OPC_FQIMAGPART;
      {
	handle_sin:
	if (CIS_Allowed) {
	  WN *intr_wn = WN_Create_Intrinsic( new_intr_opc, new_intr, 1,
					     &WN_kid0(old_wn) );
	  WN *imag = WN_CreateExp1( new_complex, intr_wn );
	  *new_wn = imag;
	  return TRUE;
	} else
	  return FALSE;
      }

    //
    // transform  cos(x) ==> realpart(cis(x))
    //
    case INTRN_F4COS:
      new_intr_opc = OPC_C4INTRINSIC_OP;
      new_intr = INTRN_F4CIS; new_complex = OPC_F4REALPART;
      goto handle_cos;
    case INTRN_F8COS:
      new_intr_opc = OPC_C8INTRINSIC_OP;
      new_intr = INTRN_F8CIS; new_complex = OPC_F8REALPART;
      goto handle_cos;
    case INTRN_FQCOS:
      new_intr_opc = OPC_CQINTRINSIC_OP;
      new_intr = INTRN_FQCIS; new_complex = OPC_FQREALPART;
      {
	handle_cos:
	if (CIS_Allowed) {
	  WN *intr_wn = WN_Create_Intrinsic( new_intr_opc, new_intr, 1,
					     &WN_kid0(old_wn) );
	  WN *real = WN_CreateExp1( new_complex, intr_wn );
	  *new_wn = real;
	  return TRUE;
	} else
	  return FALSE;
      }

  }

  // if make it here, must not have done anything
  return FALSE;
}

// ====================================================================
// handle the MIN/MAX operators
//
// Convert MIN(x) ==> MINPART(MINMAX(x))
//         MAX(x) ==> MAXPART(MINMAX(x))
// ====================================================================

static BOOL
Combine_minmax_operator( WN *old_wn, WN **new_wn, OPCODE old_wn_opc )
{
  OPCODE minmax_part;

  if ( ! WOPT_Enable_MINMAX )
    return FALSE;

  const MTYPE rtype = OPCODE_rtype(old_wn_opc);
  const MTYPE desc  = OPCODE_desc(old_wn_opc);

  // we do not currently support this with the QUAD type
  if ( rtype == MTYPE_FQ )
    return FALSE;

  switch ( OPCODE_operator(old_wn_opc) ) {
    case OPR_MIN:
      minmax_part = OPCODE_make_op(OPR_MINPART,rtype,desc);
      break;
    case OPR_MAX:
      minmax_part = OPCODE_make_op(OPR_MAXPART,rtype,desc);
      break;
    default:
      FmtAssert( FALSE, 
	("Combine_minmax_operator: bad operator") );
      return FALSE;
  }

  const OPCODE minmax_opc = OPCODE_make_op(OPR_MINMAX,rtype,desc);
  WN *minmax = WN_CreateExp2(minmax_opc, WN_kid0(old_wn),WN_kid1(old_wn));
  WN *part = WN_CreateExp1(minmax_part, minmax);
  *new_wn = part;
  return TRUE;
}

// ====================================================================
// handle the REM operator
// ====================================================================

static BOOL
Combine_rem_operator( WN *old_wn, WN **new_wn, OPCODE old_wn_opc )
{
  Is_True( OPCODE_operator(old_wn_opc) == OPR_REM,
    ("Combine_rem_operator: bad op: %s", OPCODE_name(old_wn_opc)) );

  const MTYPE rtype = OPCODE_rtype(old_wn_opc);
  const MTYPE desc  = OPCODE_desc(old_wn_opc);

  if ( WOPT_Enable_DIVREM && MTYPE_is_integral(rtype) ) {
    // Transform:  REM        REMPART
    //             a b  =>    DIVREM
    //                        a    b
    const OPCODE divrem_opc = OPCODE_make_op(OPR_DIVREM,rtype,desc);
    WN *divrem = WN_CreateExp2(divrem_opc, WN_kid0(old_wn),WN_kid1(old_wn));
    const OPCODE rempart_opc = OPCODE_make_op(OPR_REMPART,rtype,desc);
    WN *rempart = WN_CreateExp1(rempart_opc, divrem);
    *new_wn = rempart;
    return TRUE;
  }

  // if make it here, must not have done anything
  return FALSE;
}


// ====================================================================
// ====================================================================
// main driver for combining/transforming operations
// ====================================================================
// ====================================================================

extern BOOL
Combine_Operations( WN *old_wn, WN **new_wn )
{
  const OPCODE old_wn_opc = WN_opcode(old_wn);
  const OPERATOR old_wn_opr = OPCODE_operator(old_wn_opc);

  switch ( old_wn_opr ) {

    case OPR_DIV:
      return Combine_div_operator( old_wn, new_wn, old_wn_opc );

    case OPR_INTRINSIC_OP:
      return Combine_intrinsic_operator( old_wn, new_wn, old_wn_opc );

    case OPR_MAX:
    case OPR_MIN:
      return Combine_minmax_operator( old_wn, new_wn, old_wn_opc );

    case OPR_REM:
      return Combine_rem_operator( old_wn, new_wn, old_wn_opc );

  } // end switch

  return FALSE;
}

// ====================================================================
// handle the MPY operator
// ====================================================================

static BOOL
Uncombine_mpy_operator( WN *old_wn, WN **new_wn, OPCODE old_wn_opc )
{
  Is_True( OPCODE_operator(old_wn_opc) == OPR_MPY,
    ("Uncombine_mpy_operator: bad op: %s", OPCODE_name(old_wn_opc)) );

  WN *kid1 = WN_kid1(old_wn);
  const OPCODE   kid1_opc = WN_opcode(kid1);
  const OPERATOR kid1_opr = OPCODE_operator(kid1_opc);

  if ( kid1_opr == OPR_RECIP ) {
    // Transform:  MPY       into  DIV
    //            a  RECIP         a b
    //                 b
    const OPCODE div_opc = OPCODE_make_op(OPR_DIV,
			OPCODE_rtype(kid1_opc),OPCODE_desc(kid1_opc) );
    WN *div = WN_CreateExp2(div_opc, WN_kid0(old_wn), WN_kid0(kid1));
    *new_wn = div;
    return TRUE;
  }

  WN *kid0 = WN_kid0(old_wn);
  const OPCODE   kid0_opc = WN_opcode(kid0);
  const OPERATOR kid0_opr = OPCODE_operator(kid0_opc);

  if ( kid0_opr == OPR_RECIP ) {
    // Transform:  MPY       into  DIV
    //            RECIP b          b a
    //              a   
    const OPCODE div_opc = OPCODE_make_op(OPR_DIV,
			OPCODE_rtype(kid0_opc),OPCODE_desc(kid0_opc) );
    WN *div = WN_CreateExp2(div_opc, WN_kid1(old_wn), WN_kid0(kid0));
    *new_wn = div;
    return TRUE;
  }

  // if make it here, must not have done anything
  return FALSE;
}

// ====================================================================
// handle the DIVPART and REMPART operators
// ====================================================================

static BOOL
Uncombine_divrempart_operator(WN *old_wn,WN **new_wn,OPCODE old_wn_opc)
{
  const OPERATOR old_wn_opr = OPCODE_operator(old_wn_opc);

  Is_True( old_wn_opr == OPR_DIVPART || old_wn_opr == OPR_REMPART,
    ("Uncombine_divrempart_operator: bad op: %s", OPCODE_name(old_wn_opc)) );

  WN *kid0 = WN_kid0(old_wn);
  const OPCODE   kid0_opc = WN_opcode(kid0);
  const OPERATOR kid0_opr = OPCODE_operator(kid0_opc);

  if ( kid0_opr == OPR_DIVREM ) {
    // Transform: DIVPART    into  DIV (or REM)
    //            DIVREM           a b
    //            a    b
    const OPCODE divrem_opc = old_wn_opr == OPR_DIVPART ?
	 OPCODE_make_op(OPR_DIV,
			OPCODE_rtype(kid0_opc),OPCODE_desc(kid0_opc) ):
	 OPCODE_make_op(OPR_REM,
			OPCODE_rtype(kid0_opc),OPCODE_desc(kid0_opc) );
    WN *divrem = WN_CreateExp2(divrem_opc, WN_kid0(kid0),WN_kid1(kid0));
    *new_wn = divrem;
    return TRUE;
  }

  // if make it here, must not have done anything
  return FALSE;
}

// ====================================================================
// handle the IMAGPART and REALPART operators
// ====================================================================

static BOOL
Uncombine_imagrealpart_operator(WN *old_wn,WN **new_wn,OPCODE old_wn_opc)
{
  const OPERATOR old_wn_opr = OPCODE_operator(old_wn_opc);

  Is_True( old_wn_opr == OPR_IMAGPART || old_wn_opr == OPR_REALPART,
    ("Uncombine_imagrealpart_operator: bad op: %s", OPCODE_name(old_wn_opc)) );

  WN *kid0 = WN_kid0(old_wn);
  const OPCODE   kid0_opc = WN_opcode(kid0);
  const OPERATOR kid0_opr = OPCODE_operator(kid0_opc);

  if ( kid0_opr == OPR_INTRINSIC_OP ) {
    INTRINSIC new_intr;
    OPCODE new_intr_opc;
    switch ( (INTRINSIC)WN_intrinsic(kid0) ) {
      case INTRN_F4CIS: 
	new_intr = old_wn_opr == OPR_IMAGPART ? INTRN_F4SIN : INTRN_F4COS;
	new_intr_opc = OPC_F4INTRINSIC_OP;
	goto handle_sincos;

      case INTRN_F8CIS:
	new_intr = old_wn_opr == OPR_IMAGPART ? INTRN_F8SIN : INTRN_F8COS;
	new_intr_opc = OPC_F8INTRINSIC_OP;
	goto handle_sincos;

      case INTRN_FQCIS:
	new_intr = old_wn_opr == OPR_IMAGPART ? INTRN_FQSIN : INTRN_FQCOS;
	new_intr_opc = OPC_FQINTRINSIC_OP;
	// fall-thru to handle_sincos
	{
	  handle_sincos:
	  WN *intr_wn = WN_Create_Intrinsic( new_intr_opc, new_intr, 1,
					     &WN_kid0(kid0) );
	  *new_wn = intr_wn;
	  return TRUE;
	}

    }
  }

  // if make it here, must not have done anything
  return FALSE;
}

// ====================================================================
// handle the MINPART and MAXPART operators
//
// Convert  MINPART(MINMAX(x)) ==> MIN(x)
//          MAXPART(MINMAX(x)) ==> MAX(x)
// ====================================================================

static BOOL
Uncombine_minmaxpart_operator(WN *old_wn,WN **new_wn,OPCODE old_wn_opc)
{
  const OPERATOR old_wn_opr = OPCODE_operator(old_wn_opc);

  Is_True( old_wn_opr == OPR_MINPART || old_wn_opr == OPR_MAXPART,
    ("Uncombine_minmaxpart_operator: bad op: %s", OPCODE_name(old_wn_opc)) );

  WN *kid0 = WN_kid0(old_wn);
  const OPCODE   kid0_opc = WN_opcode(kid0);
  const OPERATOR kid0_opr = OPCODE_operator(kid0_opc);

  if ( kid0_opr == OPR_MINMAX ) {
    const OPCODE min_or_max_opc = old_wn_opr == OPR_MINPART ?
      OPCODE_make_op(OPR_MIN, OPCODE_rtype(kid0_opc),OPCODE_desc(kid0_opc) ) :
      OPCODE_make_op(OPR_MAX, OPCODE_rtype(kid0_opc),OPCODE_desc(kid0_opc) );
    
    WN *min_or_max = WN_CreateExp2(min_or_max_opc, WN_kid0(kid0),WN_kid1(kid0));
    *new_wn = min_or_max;
    return TRUE;
  }

  // if make it here, must not have done anything
  return FALSE;
}

// ====================================================================
// ====================================================================
// main driver for un-combining/transforming operations
// ====================================================================
// ====================================================================

extern BOOL
Uncombine_Operations( WN *old_wn, WN **new_wn )
{
  const OPCODE old_wn_opc = WN_opcode(old_wn);
  const OPERATOR old_wn_opr = OPCODE_operator(old_wn_opc);

  switch ( old_wn_opr ) {

    case OPR_MPY:
      return Uncombine_mpy_operator( old_wn, new_wn, old_wn_opc );

    case OPR_DIVPART:
    case OPR_REMPART:
      return Uncombine_divrempart_operator( old_wn, new_wn, old_wn_opc);

    case OPR_IMAGPART:
    case OPR_REALPART:
      return Uncombine_imagrealpart_operator(old_wn,new_wn,old_wn_opc);

    case OPR_MAXPART:
    case OPR_MINPART:
      return Uncombine_minmaxpart_operator(old_wn,new_wn,old_wn_opc);

  } // end switch

  return FALSE;
}

