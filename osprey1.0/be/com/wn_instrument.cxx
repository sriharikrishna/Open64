//-*-c++-*-
// ====================================================================
// ====================================================================
//
// Module: wn_instrument.cxx
// $Revision: 1.1.1.1 $
// $Date: 2002-05-22 20:06:34 $
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
// The procedures WN_Instrument and WN_Annotate, declared in
// wn_instrument.h, invoke the instrumentation and annotation phases of
// feedback collection.
//
// The class WN_INSTRUMENT_WALKER implements the instrumentation and
// annotation phases of feedback collection.  Only WN_Instrument and
// WN_Annotate should need to reference the WN_INSTRUMENT_WALKER class.
//
// Interface of WN_INSTRUMENT_WALKER to WN_Instrument and WN_Annotate:
//
// WN_INSTRUMENT_WALKER( BOOL instrumenting, PROFILE_PHASE phase );
//
//   If instrumenting is TRUE, insert instrumentation.
//   If instrumenting is FALSE, annotate the WHIRL with feedback data.
//   In either case, set phase number that the instrumenter is in.
//
// ~WN_INSTRUMENT_WALKER();
//
// void Tree_Walk( WN *wn );
//
//   Walk the tree and perform instrumentation or annotate feedback.
//
// ====================================================================
// ====================================================================


#ifdef USE_PCH
#include "be_com_pch.h"
#endif /* USE_PCH */

#pragma hdrstop
#define USE_STANDARD_TYPES

#include <algorithm>
#include <vector>
#include <stack> 
#include <list>
#include <set>

using namespace std;

// #include "stab.h"
#include "mempool.h"
#include "cxx_memory.h"
#include "tracing.h"
#include "config_opt.h"         // for Instrumentation_Enabled
#include "defs.h"
#include "wn.h"
#include "wn_map.h"
#include "wn_util.h"
#include "profile_com.h"
#include "instr_reader.h"
#include "targ_sim.h"
#include "wn_pragmas.h"
#include "ir_reader.h"		// fdump_tree
#include "glob.h"
#include "errors.h"
#include "be_symtab.h"
#include "vho_lower.h"
#include "fb_whirl.h"
#include "wn_instrument.h"

// Use this switch to turn on/off additional debugging messages

#define Instrumenter_DEBUG 0


// ALSO, SEE:
//   common/com/profile_com.h
//   common/com/instr_reader.h
//   common/instrument/instr_reader.cxx

// Invokes instrumentation:
//   be/be/driver.cxx (through WN_Instrument and wiw_wopt.Tree_Walk)


// ====================================================================
//
// Instrumentation_File_Name is the prefix for the names of the
// feedback data files.
//
// ====================================================================


static char * Instrumentation_File_Name = "";


// ====================================================================
//
// WN_INSTRUMENT_WALKER class declaration
//
// ====================================================================


class WN_INSTRUMENT_WALKER {
private:

  // ------------------------------------------------------------------
  // Private members of WN_INSTRUMENT_WALKER should be accessed only
  // through the private methods provided further below.
  // ------------------------------------------------------------------

  // _mempool is the local memory pool

  MEM_POOL *         _mempool;

  // Phase at which instrumentation/annotation is occuring
  //  (PROFILE_PHASE declared in common/com/profile_com.h)

  PROFILE_PHASE      _phase;

  // _instrumenting is TRUE  if inserting instrumentation
  // _instrumenting is FALSE if annotating WHIRL with feedback data
  // _vho_lower is TRUE iff introduced instrumentation code requires
  //   VHO Lowering after tree walk finishes (to lower commas)
  // _in_preamble is TRUE iff the tree walker is currently within a
  //   PU's preamble (after FUNCT_ENTRY or ALTENTRY and before the
  //   WN_PRAGMA_PREAMBLE_END pragma.  The tree walker should skip all
  //   WHIRL nodes within a preamble.

  BOOL               _instrumenting;
  BOOL               _vho_lower;
  BOOL               _in_preamble;

  // Counter for each type of instrumentation

  UINT32             _count_invoke;
  UINT32             _count_branch;
  UINT32             _count_loop;
  UINT32             _count_circuit;
  UINT32             _count_call;
  UINT32             _count_switch;
  UINT32             _count_compgoto;

  // _instrument_count is total number of WHIRL nodes that have been
  //   instrumented/annotated; used as a checksum

  UINT32             _instrument_count;

  // _pu_handle is the preg acting as a Handle for all profile calls in PU
  // _fb_handle is a list of handles for feedback info for this PU

  PREG_NUM           _pu_handle;
  PU_PROFILE_HANDLES _fb_handle;

  // Instrumentation initialization code will be inserted just before
  // the WN_PRAGMA_PREAMBLE_END pragma that occurs after each entry's
  // preamble.  During the tree walk:
  // _entry_pragma_stmt and _entry_pragma_block are NULL if no
  //   WN_PRAGMA_PREAMBLE_END pragma have been encountered yet.
  //   Otherwise _entry_pragma_stmt holds one WN_PRAGMA_PREAMBLE_END
  //   pragma, and _entry_pragma_block is the BLOCK in which it appears.
  // _other_entry_pragmas contains (stmt, block) pairs for all other
  //   WN_PRAGMA_PREAMBLE_END pragmas that have been encountered.
  // These three members should only be accessed through the private
  // methods provided further below.


// Solaris CC porting
  typedef mempool_allocator<WN *>  ALLOC_TYPE;
  typedef deque<WN *, ALLOC_TYPE > DEQUE_TYPE;
  typedef stack<WN *, DEQUE_TYPE > STACK_TYPE;

  WN *               _entry_pragma_stmt;
  WN *               _entry_pragma_block;
  STACK_TYPE         _other_entry_pragmas;

  // _instrumentation_nodes contains all instrumentation WHIRL nodes
  //   that were inserted into the WHIRL tree but have not yet been
  //   passed by the tree walker.  Instrumentation nodes should not
  //   be instrumented or annotated.
  // This vector should only be accessed through the private methods
  //   provided further below.

  vector<WN *, mempool_allocator<WN *> >   _instrumentation_nodes;

  // SWITCH and COMPGOTO case information

  vector<INT32, mempool_allocator<INT32> > _switch_num_targets;
  vector<INT64, mempool_allocator<INT64> > _switch_case_values;
  vector<INT32, mempool_allocator<INT32> > _compgoto_num_targets;

  // ------------------------------------------------------------------
  // Undefined default methods made private to detect errors
  // ------------------------------------------------------------------

  WN_INSTRUMENT_WALKER(void);
  WN_INSTRUMENT_WALKER(const WN_INSTRUMENT_WALKER&);
  WN_INSTRUMENT_WALKER& operator=(const WN_INSTRUMENT_WALKER&);

  // ------------------------------------------------------------------

  // Get the PU handle.
  WN *PU_Handle() const {
    return WN_LdidPreg( Pointer_type, _pu_handle );
  }

  // Get the feedback handle.
  PU_PROFILE_HANDLES& FB_Handle() { return _fb_handle; }

  // Is the feedback handle empty?
public:
  BOOL FB_Handle_Empty() { return _fb_handle.empty(); }
private:

  // Perform VHO_Lower on the tree after finishing tree walk
  void Set_VHO_Lower_Tree() { _vho_lower = TRUE; }

  // ------------------------------------------------------------------
  // List of WN_PRAGMA_PREAMBLE_END pragma WHIRL nodes and BLOCKs
  // ------------------------------------------------------------------

  // During the instrumention tree walk, the WN_PRAGMA_PREAMBLE_END
  // pragma WHIRL node for each PU entry (along with the BLOCK that
  // contains it) is stored in a list.  After the tree walk,
  // instrumentation initialization code is inserted at the end of each
  // entry's preamble.
  //
  // Implementation: The top pragma statement and block are kept in
  // _entry_pragma_stmt and _entry_pragma_block.  Any other pragmas are
  // stored in _other_entry_pragmas.  Since most PUs only have one
  // entry point, _other_entry_pragmas is usually not used.
  //
  // The methods below maintain the list of pragma nodes and blocks:
  //
  // Entry_List_Empty returns TRUE iff the list is empty.
  // Entry_Pragma returns the pragma at the top of the list.
  // Entry_Block returns the block which contains the Entry_Pragma.
  //   (If the list is empty, Entry_Pragma and Entry_Block return NULL)
  // Pop_Entry_Pragma discards the top pragma and block from the list.
  // Push_Entry_Pragma inserts a pragma and its block into the list.

  BOOL Entry_List_Empty() const { return _entry_pragma_stmt == NULL; }
  WN * Entry_Pragma()     const { return _entry_pragma_stmt; }
  WN * Entry_Block()      const { return _entry_pragma_block; }

  void Pop_Entry_Pragma() {
    if ( _other_entry_pragmas.empty() )
      _entry_pragma_stmt  = _entry_pragma_block = NULL;
    else {
      _entry_pragma_stmt  = _other_entry_pragmas.top();
      _other_entry_pragmas.pop();
      _entry_pragma_block = _other_entry_pragmas.top();
      _other_entry_pragmas.pop();
    }
  }

  void Push_Entry_Pragma( WN *stmt, WN *block ) {
    Is_True( stmt != NULL, ( "WN_INSTRUMENT_WALKER::Push_Entry_Pragma"
			     " stmt is NULL" ) );
    if ( _entry_pragma_stmt == NULL ) {
      _entry_pragma_stmt  = stmt;
      _entry_pragma_block = block;
    } else {
      _other_entry_pragmas.push( block );
      _other_entry_pragmas.push( stmt );
    }
  }

  // ------------------------------------------------------------------
  // List of instrumentation nodes that have not yet been passed
  // ------------------------------------------------------------------

  // _instrumentation_nodes contains all instrumentation WHIRL nodes
  // that were inserted into the WHIRL tree but have not yet been
  // passed by the tree walker.  Instrumentation nodes should not be
  // instrumented or annotated.  The following methods search and
  // update that list:

  // Record_Instrument_Node appends wn to the list of not-yet-passed
  //   instrumentation nodes.  It should be invoked on any node
  //   inserted _after_ the tree walker's current position.
  // Test_Instrument_Node returns TRUE iff wn is in the list of WHIRL
  //   not-yet-passed instrumentation nodes.  If Test_Instrument_Node
  //   does find a match, it removes that node from the list, so it
  //   should only be called once per node.

  void Record_Instrument_Node( WN *wn ) {
    _instrumentation_nodes.push_back( wn );
  }

  BOOL Test_Instrument_Node( WN *wn ) {
    INT t, last = _instrumentation_nodes.size() - 1;
    // Search from back to front, since next match is usually at end
    // reverse order
    for ( t = last; t >= 0; --t )
      if ( _instrumentation_nodes[t] == wn ) {
	_instrumentation_nodes[t] = _instrumentation_nodes[last];
	_instrumentation_nodes.pop_back();
	return TRUE;
      }
    return FALSE;
  }

  // ------------------------------------------------------------------
  // Methods to insert the instrumentation code into the WHIRL tree
  // ------------------------------------------------------------------

  // Is_Return_Store_Stmt returns TRUE iff the statement wn (after a
  //   call) is saving the return value to a pseudo-register.
  //   Instrumentation cannot be placed between a call and such a
  //   statement.
  // Is_Return_Store_Comma returns TRUE iff the statement wn (after a
  //   call) is a comma returning the return value of the last call in
  //   the comma's block.  Instrumentation cannot be placed after the
  //   call in the block.
  // Test_Dedicated_Reg returns TRUE iff the tree rooted at wn refers
  //   to a hardware register that instrumentation may overwrite.
  // Instrument_Before inserts an instrumentation WHIRL node in the
  //   current block before the current statement.
  // Instrument_After inserts an instrumentation WHIRL node in the
  //   current block before the current statement.
  // Instrument_Entry inserts an instrumentation WHIRL node at the end
  //   of the preamble of each of the program unit's entry points.
  // Create_Comma_Kid ensures that WN_kid( wn, kid_idx ) is a COMMA
  //   WHIRL node, so that instrumentation code can be inserted into
  //   it.  Create_Comma_Kid returns a pointer to the COMMA node.

  BOOL Is_Return_Store_Stmt( WN *wn );
  BOOL Is_Return_Store_Comma( WN *wn );
  BOOL Test_Dedicated_Reg( WN *wn );

  void Instrument_Before( WN *call, WN *current_stmt, WN *block );
  void Instrument_After( WN *call, WN *current_stmt, WN *block );
  void Instrument_Entry( WN *call );

  WN *Create_Comma_Kid( WN *wn, INT kid_idx );

  // ------------------------------------------------------------------
  // Instrumentation and Annotation of each type of WHIRL node
  // ------------------------------------------------------------------

  void Instrument_Invoke( WN *wn, INT32 id, WN *block );
  void Initialize_Instrumenter_Invoke( INT32 count );
  void Annotate_Invoke( WN *wn, INT32 id );
  void Instrument_Branch( WN *wn, INT32 id, WN *block );
  void Instrument_Cselect( WN *wn, INT32 id );
  void Initialize_Instrumenter_Branch( INT32 count );
  void Annotate_Branch( WN *wn, INT32 id );
  void Instrument_Loop( WN *wn, INT32 id, WN *block );
  void Initialize_Instrumenter_Loop( INT32 count );
  void Annotate_Loop( WN *wn, INT32 id );
  void Instrument_Circuit( WN *wn, INT32 id );
  void Initialize_Instrumenter_Circuit( INT32 count );
  void Annotate_Circuit( WN *wn, INT32 id );
  void Instrument_Call( WN *wn, INT32 id, WN *block );
  void Initialize_Instrumenter_Call( INT32 count );
  void Annotate_Call( WN *wn, INT32 id );
  void Instrument_Switch( WN *wn, INT32 id, WN *block );
  void Initialize_Instrumenter_Switch( INT32 count );
  void Annotate_Switch( WN *wn, INT32 id );
  void Instrument_Compgoto( WN *wn, INT32 id, WN *block );
  void Initialize_Instrumenter_Compgoto( INT32 count );
  void Annotate_Compgoto( WN *wn, INT32 id );

  // ------------------------------------------------------------------

  // Walk the tree and perform instrumentation or annotate feedback
  void Tree_Walk_Node( WN *wn, WN *stmt, WN *block );

  // ------------------------------------------------------------------
  // Public interface to WN_Instrument and WN_Annotate
  // ------------------------------------------------------------------

public:

  // If instrumenting is TRUE, insert instrumentation.
  // If instrumenting is FALSE, annotate the WHIRL with feedbackd data.
  // In either case, set phase number that the instrumenter is in.

  WN_INSTRUMENT_WALKER( BOOL instrumenting, PROFILE_PHASE phase,
			MEM_POOL *local_mempool,
			PU_PROFILE_HANDLES fb_handles );
  ~WN_INSTRUMENT_WALKER() {}

  // Walk the tree and perform instrumentation or annotate feedback.

  void Tree_Walk( WN *wn );
};


// ====================================================================
//
// Utility functions to generate calls to instrumentation functions.
//
// ====================================================================


WN *
Gen_Call_Shell( char *name, TYPE_ID rtype, INT32 argc )
{
  TY_IDX  ty = Make_Function_Type( MTYPE_To_TY( rtype ) );
  ST     *st = Gen_Intrinsic_Function( ty, name );

  Clear_PU_no_side_effects( Pu_Table[ST_pu( st )] );
  Clear_PU_is_pure( Pu_Table[ST_pu( st )] );
  Set_PU_no_delete( Pu_Table[ST_pu( st )] );

  WN *wn_call = WN_Call( rtype, MTYPE_V, argc, st );

  WN_Set_Call_Default_Flags(  wn_call );
  // WN_Reset_Call_Non_Parm_Mod( wn_call );
  // WN_Reset_Call_Non_Parm_Ref( wn_call );
  
  return wn_call;
}  


WN *
Gen_Call( char *name, TYPE_ID rtype = MTYPE_V )
{
  WN *call = Gen_Call_Shell( name, rtype, 0 );
  return call;
}


inline WN *
Gen_Param( WN *arg, UINT32 flag )
{
  return WN_CreateParm( WN_rtype( arg ), arg,
			MTYPE_To_TY( WN_rtype( arg ) ), flag );
}


WN *
Gen_Call( char *name, WN *arg1, TYPE_ID rtype = MTYPE_V )
{
  WN *call = Gen_Call_Shell( name, rtype, 1 );
  WN_actual( call, 0 ) = Gen_Param( arg1, WN_PARM_BY_VALUE );
  return call;
}


WN *
Gen_Call( char *name, WN *arg1, WN *arg2, TYPE_ID rtype = MTYPE_V )
{
  WN *call = Gen_Call_Shell( name, rtype, 2 );
  WN_actual( call, 0 ) = Gen_Param( arg1, WN_PARM_BY_VALUE );
  WN_actual( call, 1 ) = Gen_Param( arg2, WN_PARM_BY_VALUE );
  return call;
}


WN *
Gen_Call( char *name, WN *arg1, WN *arg2, WN *arg3, TYPE_ID rtype = MTYPE_V )
{
  WN *call = Gen_Call_Shell( name, rtype, 3 );
  WN_actual( call, 0 ) = Gen_Param( arg1, WN_PARM_BY_VALUE );
  WN_actual( call, 1 ) = Gen_Param( arg2, WN_PARM_BY_VALUE );
  WN_actual( call, 2 ) = Gen_Param( arg3, WN_PARM_BY_VALUE );
  return call;
}


WN *
Gen_Call( char *name, WN *arg1, WN *arg2, WN *arg3, WN *arg4,
	  TYPE_ID rtype = MTYPE_V )
{
  WN *call = Gen_Call_Shell( name, rtype, 4 );
  WN_actual( call, 0 ) = Gen_Param( arg1, WN_PARM_BY_VALUE );
  WN_actual( call, 1 ) = Gen_Param( arg2, WN_PARM_BY_VALUE );
  WN_actual( call, 2 ) = Gen_Param( arg3, WN_PARM_BY_VALUE );
  WN_actual( call, 3 ) = Gen_Param( arg4, WN_PARM_BY_VALUE );
  return call;
}


WN *
Gen_Call( char *name, WN *arg1, WN *arg2, WN *arg3, WN *arg4,
	  WN *arg5, TYPE_ID rtype = MTYPE_V )
{
  WN *call = Gen_Call_Shell( name, rtype, 5 );
  WN_actual( call, 0 ) = Gen_Param( arg1, WN_PARM_BY_VALUE );
  WN_actual( call, 1 ) = Gen_Param( arg2, WN_PARM_BY_VALUE );
  WN_actual( call, 2 ) = Gen_Param( arg3, WN_PARM_BY_VALUE );
  WN_actual( call, 3 ) = Gen_Param( arg4, WN_PARM_BY_VALUE );
  WN_actual( call, 4 ) = Gen_Param( arg5, WN_PARM_BY_VALUE );
  return call;
}


WN *
Gen_Call( char *name, WN *arg1, WN *arg2, WN *arg3, WN *arg4,
	  WN *arg5, WN *arg6, TYPE_ID rtype = MTYPE_V )
{
  WN *call = Gen_Call_Shell( name, rtype, 6 );
  WN_actual( call, 0 ) = Gen_Param( arg1, WN_PARM_BY_VALUE );
  WN_actual( call, 1 ) = Gen_Param( arg2, WN_PARM_BY_VALUE );
  WN_actual( call, 2 ) = Gen_Param( arg3, WN_PARM_BY_VALUE );
  WN_actual( call, 3 ) = Gen_Param( arg4, WN_PARM_BY_VALUE );
  WN_actual( call, 4 ) = Gen_Param( arg5, WN_PARM_BY_VALUE );
  WN_actual( call, 5 ) = Gen_Param( arg6, WN_PARM_BY_VALUE );
  return call;
}


// Some parameters are by reference, not by value


WN *
Gen_Call_ref3( char *name, WN *arg1, WN *arg2, WN *arg3, 
	       TYPE_ID rtype = MTYPE_V )
{
  WN *call = Gen_Call_Shell( name, rtype, 3 );
  WN_actual( call, 0 ) = Gen_Param( arg1, WN_PARM_BY_VALUE );
  WN_actual( call, 1 ) = Gen_Param( arg2, WN_PARM_BY_VALUE );
  WN_actual( call, 2 ) = Gen_Param( arg3, WN_PARM_BY_REFERENCE );
  return call;
}


WN *
Gen_Call_ref35( char *name, WN *arg1, WN *arg2, WN *arg3, WN *arg4,
		WN *arg5, TYPE_ID rtype = MTYPE_V )
{
  WN *call = Gen_Call_Shell( name, rtype, 5 );
  WN_actual( call, 0 ) = Gen_Param( arg1, WN_PARM_BY_VALUE );
  WN_actual( call, 1 ) = Gen_Param( arg2, WN_PARM_BY_VALUE );
  WN_actual( call, 2 ) = Gen_Param( arg3, WN_PARM_BY_REFERENCE );
  WN_actual( call, 3 ) = Gen_Param( arg4, WN_PARM_BY_VALUE );
  WN_actual( call, 4 ) = Gen_Param( arg5, WN_PARM_BY_REFERENCE );
  return call;
}

// ====================================================================


WN_INSTRUMENT_WALKER::WN_INSTRUMENT_WALKER( BOOL instrumenting,
					    PROFILE_PHASE phase,
					    MEM_POOL *local_mempool,
					    PU_PROFILE_HANDLES fb_handles )
  : _mempool( local_mempool ),
    _phase( phase ),
    _instrumenting( instrumenting ),
    _vho_lower( FALSE ),
    _in_preamble( FALSE ),
    _count_invoke( 0 ),
    _count_branch( 0 ),
    _count_loop( 0 ),
    _count_circuit( 0 ),
    _count_call( 0 ),
    _count_switch( 0 ),
    _count_compgoto( 0 ),
    _instrument_count( 0 ),
    _pu_handle( 0 ),
    _fb_handle( fb_handles ),
    _entry_pragma_stmt( NULL ),
    _entry_pragma_block( NULL ),
    _other_entry_pragmas( DEQUE_TYPE( ALLOC_TYPE( local_mempool ) ) ),
    _instrumentation_nodes( local_mempool ),
    _switch_num_targets( local_mempool ),
    _switch_case_values( local_mempool ),
    _compgoto_num_targets( local_mempool )
{
  if ( _instrumenting )
    _pu_handle = Create_Preg( Pointer_type, "pu_instrument_handle" );
}

// ====================================================================
//
// Procedures to insert the instrumentation code into the WHIRL tree
//
// ====================================================================


BOOL 
WN_INSTRUMENT_WALKER::Is_Return_Store_Stmt( WN *wn )
{
  if ( wn && WN_operator( wn ) == OPR_STID ) {
    WN *val = WN_kid( wn, 0 );
    if ( WN_operator( val ) == OPR_LDID ) {
      ST *st = WN_st( val );
      if ( ST_sym_class( st ) == CLASS_PREG
	   && ( Is_Return_Preg( WN_offset( val ) )
		|| st == Return_Val_Preg ) )
	return TRUE;
    }
  }
  
  return FALSE;
}


BOOL 
WN_INSTRUMENT_WALKER::Is_Return_Store_Comma( WN *wn )
{
  if ( wn && WN_operator( wn ) == OPR_COMMA ) {
    WN *val = WN_kid( wn, 1 );
    if ( WN_operator( val ) == OPR_LDID ) {
      ST *st = WN_st( val );
      if ( ST_sym_class( st ) == CLASS_PREG
	   && ( Is_Return_Preg( WN_offset( val ) )
		|| st == Return_Val_Preg ) )
	return TRUE;
    }
  }
  
  return FALSE;
}


BOOL
WN_INSTRUMENT_WALKER::Test_Dedicated_Reg( WN *wn )
{
  if ( wn == NULL )
    return FALSE;

  OPERATOR opr = WN_operator( wn );

  if ( opr == OPR_LDID ) {
    ST *st = WN_st( wn );
    if ( ST_sym_class( st ) == CLASS_PREG
	 && Preg_Is_Dedicated( WN_offset( wn ) ) )
      return TRUE;
  }
  
  // traverse the tree starting with this node.
  if ( opr == OPR_BLOCK ) {
    // Special traversal case for BLOCK structure
    WN *node;
    for ( node = WN_first( wn ); node; node = WN_next( node ) )
      if ( Test_Dedicated_Reg( node ) )
	return TRUE;
  }
  else { // Traverse the kids
    for ( INT32 i = 0; i < WN_kid_count( wn ); i++ )
      if ( Test_Dedicated_Reg( WN_kid( wn, i ) ) )
	return TRUE;
  }
  return FALSE;
}


void
WN_INSTRUMENT_WALKER::Instrument_Before( WN *wn, WN *current_stmt,
					 WN *block )
{
  if ( Test_Dedicated_Reg( current_stmt ) ) {
    DevWarn( "Instrumenter Warning: Hardware registers used in "
	     "instrumented node - program may behave differently!" );
    // fdump_tree( TFile, current_stmt );
  }

  WN *stmt_prev = WN_prev( current_stmt );
  WN_INSERT_BlockAfter( block, stmt_prev, wn );
}


void
WN_INSTRUMENT_WALKER::Instrument_After( WN *wn, WN *current_stmt,
					WN *block )
{
  WN *stmt = WN_next( current_stmt );

  // If at a call, place instrumentation after return args saved.
  // if ( _phase != PROFILE_PHASE_BEFORE_VHO )
  int i = 0;
  if ( OPCODE_is_call( WN_opcode( current_stmt ) ) )
    while ( stmt && Is_Return_Store_Stmt( stmt ) ) {
      if ( WN_rtype( current_stmt ) == MTYPE_V ) {
	DevWarn( "Instrumenter Warning: Should NOT have skipped!" );
	// fdump_tree( TFile, current_stmt );
	// fdump_tree( TFile, stmt );
      }
      i++;
      stmt = WN_next( stmt );
    }

  if ( WN_rtype( current_stmt ) != MTYPE_V && i == 0 ) {
    DevWarn( "Instrumenter Warning: Should have skipped!" );
    // fdump_tree( TFile, current_stmt );
    // fdump_tree( TFile, stmt );
  }

  WN_INSERT_BlockBefore( block, stmt, wn );
  
  Record_Instrument_Node( wn );
}


void
WN_INSTRUMENT_WALKER::Instrument_Entry( WN *wn )
{
  WN_INSERT_BlockBefore( Entry_Block(), Entry_Pragma(), wn );
}


WN *
WN_INSTRUMENT_WALKER::Create_Comma_Kid( WN *wn, INT kid_idx ) {
  WN *wn_comma;
  WN *wn_kid = WN_kid( wn, kid_idx );
  OPERATOR opr_kid = OPCODE_operator( WN_opcode( wn_kid ) );
  if ( opr_kid == OPR_COMMA ) {
    wn_comma = wn_kid;
  } else {
    wn_comma = WN_Create( OPR_COMMA, WN_rtype( wn_kid ), MTYPE_V, 2 );
    WN_kid( wn_comma, 0 ) = WN_CreateBlock();
    WN_kid( wn_comma, 1 ) = wn_kid;
    WN_kid( wn, kid_idx ) = wn_comma;
  }
  _vho_lower = TRUE;
  return wn_comma;
}


// ====================================================================
//
// Below are instrumentation and annotation procedures for each type
// of instrumentation:
// -> Invoke
// -> Branch
// -> Loop
// -> Circuit
// -> Call
// -> Switch
// -> Compgoto
//
// Three procedures are defined for each type or subtype:
//
// Instrument_*
// Initialize_Instrumenter_*
// Annotate_*
//
// ====================================================================


void
WN_INSTRUMENT_WALKER::Instrument_Invoke( WN *wn, INT32 id, WN *block )
{
  WN *instr = Gen_Call( INVOKE_INSTRUMENT_NAME,
			PU_Handle(), WN_Intconst( MTYPE_I4, id ) );
  Instrument_After( instr, wn, block );
}


void
WN_INSTRUMENT_WALKER::Initialize_Instrumenter_Invoke( INT32 count )
{
  if ( count == 0 ) return;

  WN *total_invokes = WN_Intconst( MTYPE_I4, count );
  // __profile_invoke_init( handle, total_invokes )
  Instrument_Entry( Gen_Call( INVOKE_INIT_NAME,
			      PU_Handle(), total_invokes ) );
}


void
WN_INSTRUMENT_WALKER::Annotate_Invoke( WN *wn, INT32 id )
{
  // Sum profile frequency counts
  PU_PROFILE_HANDLES& handles = FB_Handle();
  FB_Info_Invoke info_invoke( FB_FREQ_ZERO );
  for ( PU_PROFILE_ITERATOR i( handles.begin() );
	i != handles.end (); ++i ) {
    FB_Info_Invoke& info = Get_Invoke_Profile( *i, id );
    info_invoke.freq_invoke += info.freq_invoke;
  }

  // Attach profile information to node.
  Cur_PU_Feedback->Annot_invoke( wn, info_invoke );
}


// ====================================================================


void
WN_INSTRUMENT_WALKER::Instrument_Branch( WN *wn, INT32 id, WN *block )
{
  // Compute condition once and save to preg.
  TYPE_ID cond_type = WN_rtype( WN_kid0( wn ) );
  PREG_NUM cond = Create_Preg( cond_type, "__branch_cond" );
  Instrument_Before( WN_StidIntoPreg( cond_type, cond,
				      MTYPE_To_PREG( cond_type ),
				      WN_kid0( wn ) ),
		     wn, block );

  // Replace condition by preg
  WN_kid0( wn ) = WN_LdidPreg( cond_type, cond );


  // profile_branch( handle, b_id, condition, taken_if_true );
  OPERATOR opr = OPCODE_operator( WN_opcode( wn ) );
  WN *taken = WN_Relational( ( opr == OPR_FALSEBR ) ? OPR_EQ : OPR_NE,
			     MTYPE_I4, WN_LdidPreg( cond_type, cond ),
			     WN_Intconst( MTYPE_I4, 0 ) );
  WN *instr = Gen_Call( BRANCH_INSTRUMENT_NAME, PU_Handle(),
			WN_Intconst( MTYPE_I4, id ), taken );
  Instrument_Before( instr, wn, block );
}


void
WN_INSTRUMENT_WALKER::Instrument_Cselect( WN *wn, INT32 id )
{
  // Create comma for kid0
  WN *comma = Create_Comma_Kid( wn, 0 );

  // Compute condition once and save to preg.
  TYPE_ID cond_type = WN_rtype( WN_kid( comma, 1 ) );
  PREG_NUM cond = Create_Preg( cond_type, "__cselect_cond" );
  WN *stid = WN_StidIntoPreg( cond_type, cond, MTYPE_To_PREG( cond_type ),
			      WN_kid( comma, 1 ) );
  WN_INSERT_BlockLast( WN_kid( comma, 0 ), stid );

  // Replace condition by preg
  WN_kid( comma, 1 ) = WN_LdidPreg( cond_type, cond );

  // Insert instrumentation call
  WN *taken = WN_Relational( OPR_NE, MTYPE_I4,
			     WN_LdidPreg( cond_type, cond ),
			     WN_Intconst( MTYPE_I4, 0 ) );
  WN *instr = Gen_Call( BRANCH_INSTRUMENT_NAME, PU_Handle(),
			WN_Intconst( MTYPE_I4, id ), taken );
  WN_INSERT_BlockLast( WN_kid( comma, 0 ), instr );
}


void
WN_INSTRUMENT_WALKER::Initialize_Instrumenter_Branch( INT32 count )
{
  if ( count == 0 ) return;

  // __profile_branch_init(handle, total_branches)
  WN *total_branches = WN_Intconst( MTYPE_I4, count );
  Instrument_Entry( Gen_Call( BRANCH_INIT_NAME,
			      PU_Handle(), total_branches ) );
}


void
WN_INSTRUMENT_WALKER::Annotate_Branch(WN *wn, INT32 id)
{
  PU_PROFILE_HANDLES& handles = FB_Handle();
  FB_Info_Branch info_branch( FB_FREQ_ZERO, FB_FREQ_ZERO );
  for (PU_PROFILE_ITERATOR i( handles.begin() ); i != handles.end(); ++i ) {
    FB_Info_Branch& info = Get_Branch_Profile( *i, id );
    info_branch.freq_taken += info.freq_taken;
    info_branch.freq_not_taken += info.freq_not_taken;
  }
  // Attach profile information to node.
  Cur_PU_Feedback->Annot_branch( wn, info_branch );
}


// ====================================================================


void
WN_INSTRUMENT_WALKER::Instrument_Loop( WN *wn, INT32 id, WN *block )
{
  OPERATOR opr = OPCODE_operator( WN_opcode( wn ) );

  // profile_loop( handle, id ) before loop.
  Instrument_Before( Gen_Call( LOOP_INSTRUMENT_NAME,
			       PU_Handle(),
			       WN_Intconst( MTYPE_I4, id ) ),
		     wn, block );
  
  // profile_loop_iter( handle, id ) at beginning of loop iter.
  WN *body = ( opr == OPR_DO_LOOP
	       ? WN_do_body( wn ) : WN_while_body( wn ) );
  WN *iter_call = Gen_Call( LOOP_INST_ITER_NAME,
			    PU_Handle(),
			    WN_Intconst( MTYPE_I4, id ) );
  WN_INSERT_BlockFirst( body, iter_call );
  Record_Instrument_Node( iter_call );
}


void
WN_INSTRUMENT_WALKER::Initialize_Instrumenter_Loop( INT32 count )
{
  if ( count == 0 ) return;

  WN *total_loops = WN_Intconst( MTYPE_I4, count );
  // __profile_loop_init( handle, total_loops )
  Instrument_Entry( Gen_Call( LOOP_INIT_NAME,
				   PU_Handle(), total_loops ) );
}


void
WN_INSTRUMENT_WALKER::Annotate_Loop( WN *wn, INT32 id )
{
  PU_PROFILE_HANDLES& handles = FB_Handle();
  FB_Info_Loop info_loop( FB_FREQ_ZERO, FB_FREQ_ZERO, FB_FREQ_ZERO,
			  FB_FREQ_ZERO, FB_FREQ_ZERO, FB_FREQ_ZERO );
  for ( PU_PROFILE_ITERATOR i( handles.begin() ); i != handles.end(); ++i ) {
    FB_Info_Loop& info = Get_Loop_Profile ( *i, id ); 
    info_loop.freq_zero += info.freq_zero;
    info_loop.freq_positive += info.freq_positive;
    info_loop.freq_out += info.freq_out;
    info_loop.freq_back += info.freq_back;
    info_loop.freq_exit += info.freq_exit;
    info_loop.freq_iterate += info.freq_iterate;
  }
  // Attach profile information to node.
  Cur_PU_Feedback->Annot_loop( wn, info_loop );
}


// ====================================================================


// WN * 
// SHORT_CIRCUIT_INSTRUMENTER::Instrument_Clause(WN *clause, WN *call)
// {
//   OPERATOR opr = OPCODE_operator(WN_opcode(clause));
//   WN *comma;
//
//   if (opr == OPR_COMMA) {
//     comma = clause;
//   } else {
//     comma = WN_Create(OPR_COMMA, WN_rtype(clause), MTYPE_V, 2);
//     WN_kid(comma, 0) = WN_CreateBlock();
//     WN_kid(comma, 1) = clause;
//   }
//
//   WN_INSERT_BlockFirst(WN_kid(comma, 0), call);
//   return comma;
// }


void
WN_INSTRUMENT_WALKER::Instrument_Circuit( WN *wn, INT32 id )
{
  // No need to instrument left branch (kid 0)

  // Create comma for right branch (kid 1)
  WN *comma = Create_Comma_Kid( wn, 1 );

  // Compute condition once and save to preg
  TYPE_ID cond_type = WN_rtype( WN_kid( comma, 1 ) );
  PREG_NUM cond = Create_Preg( cond_type, "__circuit_cond" );
  WN *stid = WN_StidIntoPreg( cond_type, cond, MTYPE_To_PREG( cond_type ),
			      WN_kid( comma, 1 ) );
  WN_INSERT_BlockLast( WN_kid( comma, 0 ), stid );

  // Replace condition in left branch by preg
  WN_kid( comma, 1 ) = WN_LdidPreg( cond_type, cond );

  // Insert instrumentation call
  //   IDEA: Use BRANCH instead of SHORT_CIRCUIT
  OPERATOR opr = OPCODE_operator( WN_opcode( wn ) );
  WN *taken = WN_Relational( opr == OPR_CAND ? OPR_EQ : OPR_NE,
			     MTYPE_I4, WN_LdidPreg( cond_type, cond ),
			     WN_Intconst( MTYPE_I4, 0 ) );
  WN *instr = Gen_Call( SHORT_CIRCUIT_INST_NAME, PU_Handle(),
			WN_Intconst( MTYPE_I4, id ), taken );
  WN_INSERT_BlockLast( WN_kid( comma, 0 ), instr );
}


void
WN_INSTRUMENT_WALKER::Initialize_Instrumenter_Circuit( INT32 count )
{
  if ( count == 0 ) return;

  WN *total_short_circuits = WN_Intconst( MTYPE_I4, count );
  // __profile_short_circuit_init( handle, total_short_circuits )
  Instrument_Entry( Gen_Call( SHORT_CIRCUIT_INIT_NAME,
				   PU_Handle(), total_short_circuits ) );
}


void
WN_INSTRUMENT_WALKER::Annotate_Circuit( WN *wn, INT32 id )
{
  PU_PROFILE_HANDLES& handles = FB_Handle();
  FB_Info_Circuit info_circuit( FB_FREQ_ZERO, FB_FREQ_ZERO, FB_FREQ_ZERO );
  for (PU_PROFILE_ITERATOR i( handles.begin () ); i != handles.end(); ++i ) {
    FB_Info_Circuit& info = Get_Short_Circuit_Profile( *i, id );
    info_circuit.freq_left += info.freq_left;
    info_circuit.freq_right += info.freq_right;
    info_circuit.freq_neither += info.freq_neither;
  }
  // Attach profile information to node.
  Cur_PU_Feedback->Annot_circuit( wn, info_circuit );
}


// ====================================================================


void
WN_INSTRUMENT_WALKER::Instrument_Call( WN *wn, INT32 id, WN *block )
{
  // Get the name of the called function.
  WN *called_func_name;
  if ( WN_has_sym( wn ) ) {
    char *name = ST_name( WN_st( wn ) );
    called_func_name = WN_LdaString( name, 0, strlen( name ) + 1 );
  } else
    called_func_name = WN_Zerocon( Pointer_type );

  // profile_call_invoke( handle, call_id, called_func_name )
  Instrument_Before( Gen_Call( CALL_INST_ENTRY_NAME,
			       PU_Handle(),
			       WN_Intconst( MTYPE_I4, id ),
			       called_func_name ),
		     wn, block );

  // profile_call_exit( handle, call_id, called_func_name )
  Instrument_After( Gen_Call( CALL_INST_EXIT_NAME,
			      PU_Handle(),
			      WN_Intconst( MTYPE_I4, id ),
			      WN_COPY_Tree( called_func_name ) ),
		    wn, block );
}


void
WN_INSTRUMENT_WALKER::Initialize_Instrumenter_Call( INT32 count )
{
  if ( count == 0 ) return;

  WN *total_calls = WN_Intconst( MTYPE_I4, count );
  // __profile_call_init( handle, total_calls )
  Instrument_Entry( Gen_Call( CALL_INIT_NAME,
			      PU_Handle(), total_calls ) );
}


void
WN_INSTRUMENT_WALKER::Annotate_Call( WN *wn, INT32 id )
{
  PU_PROFILE_HANDLES& handles = FB_Handle();
  FB_Info_Call info_call( FB_FREQ_ZERO );
  for (PU_PROFILE_ITERATOR i( handles.begin() ); i != handles.end (); ++i) {
    FB_Info_Call& info = Get_Call_Profile( *i, id );
    info_call.freq_entry += info.freq_entry;
    info_call.freq_exit += info.freq_exit;
  }

  info_call.in_out_same = ( info_call.freq_entry == info_call.freq_exit );

  // Attach profile information to node.
  Cur_PU_Feedback->Annot_call( wn, info_call );
}


// ====================================================================


void
WN_INSTRUMENT_WALKER::Instrument_Switch( WN *wn, INT32 id, WN *block )
{
  // Record number of targets
  _switch_num_targets.push_back( WN_num_entries( wn ) );

  // Record case values (one for each target)
  for ( WN *wn_casegoto = WN_first( WN_kid1( wn ) );
	wn_casegoto != NULL;
	wn_casegoto = WN_next( wn_casegoto ) ) {
    _switch_case_values.push_back( WN_const_val( wn_casegoto ) );
  }

  // Compute target once and save to preg.
  TYPE_ID cond_type = WN_rtype( WN_kid0( wn ) );
  PREG_NUM cond = Create_Preg( cond_type, "__switch_cond" );

  Instrument_Before( WN_StidIntoPreg( cond_type, cond,
				      MTYPE_To_PREG( cond_type ),
				      WN_kid0( wn ) ),
		     wn, block );
  WN_kid0( wn ) = WN_LdidPreg( cond_type, cond );

  // profile_switch(handle, switch_id, target, num_targets)
  WN *instr = Gen_Call( SWITCH_INSTRUMENT_NAME, PU_Handle(),
			WN_Intconst( MTYPE_I4, id ),
			WN_LdidPreg( cond_type, cond ),
			WN_Intconst( MTYPE_I4,
				     WN_num_entries( wn ) ) );
  Instrument_Before( instr, wn, block );
}


void
WN_INSTRUMENT_WALKER::Initialize_Instrumenter_Switch( INT32 count )
{
  if ( count == 0 ) return;

  // Build switch length array from vector.
  INT32 num_switches = count;
      
  TY_IDX arrayTY = Make_Array_Type( MTYPE_I4, 1, num_switches );
  ST *arrayST = New_ST( CURRENT_SYMTAB );
  ST_Init( arrayST, Save_Str( "switch_num_targets" ),
	   CLASS_VAR, SCLASS_PSTATIC, EXPORT_LOCAL, arrayTY );
      
  // This should be only initialized once - instead of every time.
  // How to do this?
  for ( INT32 i = 0; i < num_switches; i++ ) {
    WN *st = WN_Stid(MTYPE_I4, i * MTYPE_RegisterSize( MTYPE_I4 ),
		     arrayST, arrayTY,
		     WN_Intconst( MTYPE_I4, _switch_num_targets[i] ) );
    Instrument_Entry( st );
  }

  WN *total_switches = WN_Intconst( MTYPE_I4, num_switches );
  WN *switch_num_targets = WN_Lda( Pointer_type, 0, arrayST );


  // Build case value array from vector.
  INT32 num_case_values = _switch_case_values.size();

  arrayTY = Make_Array_Type( MTYPE_I8, 1, num_case_values );
  arrayST = New_ST( CURRENT_SYMTAB );
  ST_Init( arrayST, Save_Str( "switch_case_values" ),
	   CLASS_VAR, SCLASS_PSTATIC, EXPORT_LOCAL, arrayTY );
      
  // This should be only initialized once - instead of every time.
  // How to do this?
  for ( INT32 j = 0; j < num_case_values; j++ ) {
    WN *st = WN_Stid( MTYPE_I8, j * MTYPE_RegisterSize( MTYPE_I8 ),
		      arrayST, arrayTY,
		      WN_Intconst( MTYPE_I8, _switch_case_values[j] ) );
    Instrument_Entry( st );
  }

  WN *total_case_values = WN_Intconst( MTYPE_I4, num_case_values );
  WN *switch_case_values = WN_Lda( Pointer_type, 0, arrayST );

  // __profile_switch_init(handle, total_switches, target_count_array)
  WN *instr = Gen_Call_ref35( SWITCH_INIT_NAME, PU_Handle(),
			      total_switches, switch_num_targets,
			      total_case_values, switch_case_values );
  Instrument_Entry( instr );
}


static inline void
Handle_Switch_Profile( PU_PROFILE_HANDLES& handles, WN* wn, INT32 id,
		       FB_Info_Switch& (*get_profile) ( PU_PROFILE_HANDLE,
							INT32 ) )
{
  FB_Info_Switch& info = (*get_profile) ( handles[0], id );
  if ( handles.size() == 1 ) {
    // Attach profile information to node.
    Cur_PU_Feedback->Annot_switch( wn, info );
  } else {
    FB_Info_Switch info_switch;
    info_switch.freq_targets.insert( info_switch.freq_targets.begin(),
				     info.freq_targets.begin(),
				     info.freq_targets.end() );
    PU_PROFILE_ITERATOR i (handles.begin ());
    for (++i; i != handles.end (); ++i) {
      FB_Info_Switch& info = (*get_profile) (*i, id);
      FmtAssert( info.size () == info_switch.size (),
		 ("Inconsistent profile data from different files"));
      transform( info.freq_targets.begin(),
		 info.freq_targets.end(),
		 info_switch.freq_targets.begin(),
		 info_switch.freq_targets.begin(),
		 plus<FB_FREQ>() );
    }
    Cur_PU_Feedback->Annot_switch( wn, info_switch );
  }
}


void
WN_INSTRUMENT_WALKER::Annotate_Switch( WN *wn, INT32 id )
{
  Handle_Switch_Profile( FB_Handle(), wn, id, Get_Switch_Profile );
}


// ====================================================================


void
WN_INSTRUMENT_WALKER::Instrument_Compgoto( WN *wn, INT32 id, WN *block )
{
  _compgoto_num_targets.push_back( WN_num_entries( wn ) );
  
  // Compute target once and save to preg.
  TYPE_ID cond_type = WN_rtype( WN_kid0( wn ) );
  PREG_NUM cond = Create_Preg( cond_type, "__compgoto_cond" );

  WN *target = WN_StidIntoPreg( cond_type, cond,
				MTYPE_To_PREG( cond_type ),
				WN_kid0( wn ) );
  Instrument_Before( target, wn, block );
  WN_kid0( wn ) = WN_LdidPreg( cond_type, cond );

  // profile_compgoto( handle, compgoto_id, target, num_targets )
  WN *instr = Gen_Call( COMPGOTO_INSTRUMENT_NAME, PU_Handle(),
			WN_Intconst( MTYPE_I4, id ),
			WN_LdidPreg( cond_type, cond ),
			WN_Intconst( MTYPE_I4,
				     WN_num_entries( wn ) ) );
  Instrument_Before( instr, wn, block );
}


void
WN_INSTRUMENT_WALKER::Initialize_Instrumenter_Compgoto( INT32 count )
{
  if ( count == 0 ) return;

  // Build compgoto length array from vector.
  INT32 num_compgotos = count;

  TY_IDX arrayTY = Make_Array_Type( MTYPE_I4, 1, num_compgotos );
  ST *arrayST = New_ST( CURRENT_SYMTAB );
  ST_Init( arrayST, Save_Str( "compgoto_num_targets" ),
	   CLASS_VAR, SCLASS_PSTATIC, EXPORT_LOCAL, arrayTY );

  // This should be only initialized once - instead of every time.
  // How to do this?
  for ( INT32 i = 0; i < num_compgotos; i++ ) {
    WN *st = WN_Stid( MTYPE_I4, i * MTYPE_RegisterSize( MTYPE_I4 ),
		      arrayST, arrayTY, 
		      WN_Intconst( MTYPE_I4, _compgoto_num_targets[i] ) );
    Instrument_Entry( st );
  }

  WN *total_compgotos = WN_Intconst( MTYPE_I4, num_compgotos );
  WN *compgoto_num_targets = WN_Lda( Pointer_type, 0, arrayST );

  // __profile_compgoto_init(handle, total_compgotos, target_count_array)
  Instrument_Entry( Gen_Call_ref3( COMPGOTO_INIT_NAME, PU_Handle(),
				   total_compgotos,
				   compgoto_num_targets ) );
}


void
WN_INSTRUMENT_WALKER::Annotate_Compgoto( WN *wn, INT32 id )
{
  Handle_Switch_Profile( FB_Handle(), wn, id, Get_Compgoto_Profile );
}


// ====================================================================
//
// This is the main driver of the instrumenter. It traverses the tree
// calling appropriate instrumentation procedures when needed.
//
// Tree_Walk_Node must traverse the WHIRL tree in the same order
// during annotation as during instrumentation.
//
// Tree_Walk_Node and Tree_Walk must proceed FORWARD through the
// statements within a BLOCK (so that _instrumentation_nodes only need
// record upcoming WHIRL nodes).
//
// ====================================================================


void
WN_INSTRUMENT_WALKER::Tree_Walk_Node( WN *wn, WN *stmt, WN *block )
{
  OPERATOR opr = WN_operator( wn );

  // Pay special attention to ALT_ENTRY, PRAGMA, and REGION

  // ALT_ENTRY indicates we are entering preamble
  if ( opr == OPR_ALTENTRY ) {
    Is_True( ! _in_preamble, ( "WN_INSTRUMENT_WALKER::Tree_Walk_Node found"
			       " no WN_PRAGMA_PREAMBLE_END pragma" ) );
    _in_preamble = TRUE;
  }

  // WN_PRAGMA_PREAMBLE_END pragma indicates the end of the preamble;
  // Record the PRAGMA for later insertion of instrumentation
  //   initialization code.
  else if ( opr == OPR_PRAGMA
	    && WN_pragma( wn ) == WN_PRAGMA_PREAMBLE_END ) {
    Is_True( _in_preamble, ( "WN_INSTRUMENT_WALKER::Tree_Walk_Node found"
			     " extra WN_PRAGMA_PREAMBLE_END pragma" ) );
    _in_preamble = FALSE;
    Push_Entry_Pragma( wn, block );
  }

  else if ( opr == OPR_REGION ) {

    // PREG for _pu_handle must be scoped SHARED within PARALLEL regions
    WN *regn_prag = WN_first( WN_region_pragmas( wn ) );
    if ( regn_prag ) {
      switch ( WN_pragma( regn_prag ) ) {
      case WN_PRAGMA_PARALLEL_BEGIN:
      case WN_PRAGMA_PARALLEL_SECTIONS:
      case WN_PRAGMA_PARALLEL_DO:
      case WN_PRAGMA_DOACROSS:
	{
	  WN *prag = WN_CreatePragma( WN_PRAGMA_SHARED,
				      MTYPE_To_PREG( Pointer_type ),
				      _pu_handle, 0 );
	  WN_set_pragma_compiler_generated( prag );
	  WN_INSERT_BlockLast( WN_region_pragmas ( wn ), prag );
	}
	break;
      default:
	break;  // not a PARALLEL region
      }
    }
  }

  // Do not instrument or annotate code within the preamble
  // Skip over any nodes added by instrumentation
  if ( _in_preamble || Test_Instrument_Node( wn ) )
    return;

  // Traverse WHIRL subtree.
  if ( opr == OPR_BLOCK ) {

    // Special traversal case for BLOCK structure
    WN *node;
    for ( node = WN_first( wn ); node; node = WN_next( node ) )
      Tree_Walk_Node( node, node, wn );

    Is_True( ! _in_preamble, ( "WN_INSTRUMENT_WALKER::Tree_Walk_Node found"
			       " no WN_PRAGMA_PREAMBLE_END pragma" ) );

  } else if ( OPERATOR_is_expression( opr ) ) {

    // Watch out for special case:
    if ( Is_Return_Store_Comma( wn ) ) {

      // Handle COMMA holding CALL with return value --- convert:
      //   BLOCK                   --->      BLOCK
      //    ... PARAMs ...         --->       ... PARAMs ...
      //   CALL                    --->      CALL
      //                           --->       LDID preg_return_val
      //                           --->      STID temp
      //   END_BLOCK               --->      END_BLOCK
      //   LDID preg_return_val    --->     LDID temp
      //  COMMA                    --->     COMMA
      // This convertion allow the CALL to be instrumented correctly.

      // Store return value into preg
      TYPE_ID val_type = WN_rtype( WN_kid( wn, 1 ) );
      PREG_NUM val = Create_Preg( val_type, "__call_comma" );
      WN *stid = WN_StidIntoPreg( val_type, val, MTYPE_To_PREG( val_type ),
				  WN_kid( wn, 1 ) );
      WN_INSERT_BlockLast( WN_kid( wn, 0 ), stid );
      
      // Comma now returns value of preg
      WN_kid( wn, 1 ) = WN_LdidPreg( val_type, val );
    }

    // Traverse the kids of the current expression
    for ( INT32 i = 0; i < WN_kid_count( wn ); i++ )
      Tree_Walk_Node( WN_kid( wn, i ), stmt, block );

  } else {

    // Traverse the kids of the current statement
    for ( INT32 i = 0; i < WN_kid_count( wn ); i++ )
      Tree_Walk_Node( WN_kid( wn, i ), wn, block );
  }

  // Perform the instrumentation or annotation of the current node
  switch ( opr ) {

  case OPR_PRAGMA:
    if ( WN_pragma( wn ) != WN_PRAGMA_PREAMBLE_END )
      break;
    {
      _instrument_count++;
      INT32 id = _count_invoke++;
      if ( _instrumenting )
	Instrument_Invoke( wn, id, block );
      else
	Annotate_Invoke( wn, id );
    }
    break;

  case OPR_TRUEBR:
  case OPR_FALSEBR:
  case OPR_IF:
    {
      _instrument_count++;
      INT32 id = _count_branch++;
      if ( _instrumenting )
	Instrument_Branch( wn, id, block );
      else
	Annotate_Branch( wn, id );
    }
    break;

  case OPR_CSELECT:
    {
      _instrument_count++;
      INT32 id = _count_branch++;
      if ( _instrumenting )
	Instrument_Cselect( wn, id );
      else
	Annotate_Branch( wn, id );
    }
    break;

  case OPR_DO_LOOP:
  case OPR_WHILE_DO:
  case OPR_DO_WHILE:
    {
      _instrument_count++;
      INT32 id = _count_loop++;
      if ( _instrumenting )
	Instrument_Loop( wn, id, block );
      else
	Annotate_Loop( wn, id );
    }
    break;

  case OPR_CAND:
  case OPR_CIOR:
    {
      _instrument_count++;
      INT32 id = _count_circuit++;
      if ( _instrumenting )
	Instrument_Circuit( wn, id );
      else
	Annotate_Circuit( wn, id );
    }
    break;

  case OPR_PICCALL:
  case OPR_CALL:
  case OPR_ICALL:
  case OPR_INTRINSIC_CALL:
  case OPR_IO:
    {
      _instrument_count++;
      INT32 id = _count_call++;
      if ( _instrumenting )
	Instrument_Call( wn, id, block );
      else
	Annotate_Call( wn, id );
    }
    break;

  case OPR_SWITCH:
    {
      _instrument_count++;
      INT32 id = _count_switch++;
      if ( _instrumenting )
	Instrument_Switch( wn, id, block );
      else
	Annotate_Switch( wn, id );
    }
    break;

  case OPR_COMPGOTO:
  case OPR_XGOTO:
    {
      _instrument_count++;
      INT32 id = _count_compgoto++;
      if ( _instrumenting )
	Instrument_Compgoto( wn, id, block );
      else
	Annotate_Compgoto( wn, id );
    }
    break;
  }

  // This shouldn't be necessary, but....
  if ( opr == OPR_REGION ) {
    if ( _vho_lower ) {
      WN_region_body( wn ) = VHO_Lower( WN_region_body( wn ) );
      _vho_lower = FALSE;
    }
  }
}


void
WN_INSTRUMENT_WALKER::Tree_Walk( WN *root ) 
{
  // Root must be a func entry!!
  Is_True( WN_operator( root ) == OPR_FUNC_ENTRY,
	   ( "WN_INSTRUMENT_WALKER::Tree_Walk:"
	     " OPR_FUNC_ENTRY expected at top of PU tree." ) );

  // Cur_PU_Feedback should not be NULL if annotating
  Is_True( _instrumenting || Cur_PU_Feedback,
	   ( "WN_INSTRUMENT_WALKER::Tree_Walk:"
	     " NULL Cur_PU_Feedbackduring annotation" ) );

  // Tree_Walk should not be called if annotating with empty _fb_handle
  Is_True( _instrumenting || ! _fb_handle.empty (),
	   ( "WN_INSTRUMENT_WALKER::Tree_Walk: No feedback info for"
	     " program unit %s in file %s.", Cur_PU_Name, Src_File_Name ) );

  // Don't instrument twice!
  // Can we come up with an Is_True to check for multiple instrumentation?
  // Is_True( ! _entry_is_instrumented,
  //	   ( "Instrumenter Error: Whirl tree already instrumented "
  //	     "(initialization done)!" ) );

#if Instrumenter_DEBUG
  fdump_tree( TFile, root );
#endif

  // Instrument all statements after (and including) the preamble end;
  // Do not instrument statements in the preamble
  _in_preamble = TRUE;  // will be set FALSE at WN_PRAGMA_PREAMBLE_END
  WN* body = WN_func_body( root );
  WN* stmt;
  for ( stmt = WN_first( body ); stmt; stmt = WN_next( stmt ) )
    Tree_Walk_Node( stmt, stmt, body );
  Is_True( ! _in_preamble, ( "WN_INSTRUMENT_WALKER::Tree_Walk found"
			     " no WN_PRAGMA_PREAMBLE_END pragma" ) );

  // Is there any instrumentation to be initialized?
  if ( _instrumenting && _instrument_count > 0 ) {
      
    // Add initialization to each entry point.
    while ( ! Entry_List_Empty() ) {
      // Initialize the instrumentation.
      // Note: this only needs to be done in the entire program.
      // For now, I'm calling this at every PU.
      WN *output_file_name
	= WN_LdaString( Instrumentation_File_Name, 0,
			strlen( Instrumentation_File_Name ) + 1 );
      WN *phasenum = WN_Intconst( MTYPE_I4, _phase );

      WN *unique_output = WN_Intconst( MTYPE_I4,
				       Instrumentation_Unique_Output );

      // __profile_init( outfile )
      Instrument_Entry( Gen_Call( INST_INIT_NAME, output_file_name,
				  phasenum, unique_output ) );
	  
      // Initialize the PU instrumentation.
      WN *src_file_name = WN_LdaString ( Src_File_Name, 0,
					 strlen( Src_File_Name ) + 1 );
      WN *pu_name = WN_LdaString ( Cur_PU_Name, 0, strlen( Cur_PU_Name ) + 1 );
      WN *pc = WN_Lda( Pointer_type, 0, WN_st( root ) );
      WN *checksum = WN_Intconst( MTYPE_I4, _instrument_count );
      // r2 = __profile_pu_init( src_file_name, pu_name, pc, checksum )
      Instrument_Entry( Gen_Call( PU_INIT_NAME, src_file_name,
				  pu_name, pc, checksum, Pointer_type ) );
	  
      // Get current handle.
      PREG_NUM rreg1, rreg2;

      if ( WHIRL_Return_Info_On ) {

	RETURN_INFO return_info
	  = Get_Return_Info( Be_Type_Tbl( Pointer_type ), Use_Simulated );

	if ( RETURN_INFO_count( return_info ) <= 2 ) {
	  rreg1 = RETURN_INFO_preg( return_info, 0 );
	  rreg2 = RETURN_INFO_preg( return_info, 1 );
	} else
	  Fail_FmtAssertion( "WN_Instrumenter::WN_tree_init:"
			     " more than 2 return registers");
      } else {
	Get_Return_Pregs( Pointer_type, MTYPE_UNKNOWN, &rreg1, &rreg2 );
      }
	  
      // handle = r2;
      Instrument_Entry( WN_StidIntoPreg( Pointer_type, _pu_handle,
					 MTYPE_To_PREG( Pointer_type ),
					 WN_LdidPreg( Pointer_type,
						      rreg1 ) ) );

      // Initialize specific instrumentation.
      Initialize_Instrumenter_Invoke(   _count_invoke   );
      Initialize_Instrumenter_Branch(   _count_branch   );
      Initialize_Instrumenter_Loop(     _count_loop     );
      Initialize_Instrumenter_Circuit(  _count_circuit  );
      Initialize_Instrumenter_Call(     _count_call     );
      Initialize_Instrumenter_Switch(   _count_switch   );
      Initialize_Instrumenter_Compgoto( _count_compgoto );

      Pop_Entry_Pragma();
    }

    // Perform additional lowering - if necessary.
    if ( _vho_lower ) {
      WN_func_body( root ) = VHO_Lower( WN_func_body( root ) );
      _vho_lower = FALSE;
    }

  } else if ( ! _instrumenting ) { // feedback

    // Compare checksums!
    for ( PU_PROFILE_ITERATOR i( _fb_handle.begin() );
	  i != _fb_handle.end(); ++i ) {
	  
      UINT32 checksum = Get_PU_Checksum( *i );
	
      Is_True( _instrument_count == checksum,
	       ( "Instrumenter Error: (Phase %d) Feedback file has invalid "
		 " checksum for program unit %s in file %s. "
		 " Computed = %d, In file = %d.",
		 _phase, Cur_PU_Name, Src_File_Name, _instrument_count, 
		 checksum ) );
    }
  }

#if Instrumenter_DEBUG
  fdump_tree(TFile, root);
#endif
}


// ====================================================================
//
// Interface to the rest of the compiler backend -- see fuller
// descriptions in the header file wn_instrument.h.
//
// WN_Instrument performs feedback instrumentation on the WHIRL tree
// rooted at wn.
//
// WN_Annotate reads frequency counts from one or more previously
// generated feedback data files and stores the feedback data into
// the FEEDBACK object Cur_PU_Feedback.
//
// Set_Instrumentation_File_Name records the prefix for the names of
// the feedback data files.
//
// ====================================================================


void
WN_Instrument( WN *wn, PROFILE_PHASE phase )
{
  Set_Error_Phase( "WN_Instrument" );
  if ( Instrumenter_DEBUG )
    DevWarn( "WN_Instrument, phase == %d", phase );

  // Create and initialize local memory pool
  MEM_POOL local_mempool;
  MEM_POOL_Initialize( &local_mempool, "WN_INSTRUMENT_WALKER_Pool", FALSE );
  MEM_POOL_Push( &local_mempool );
  {
    // Walk the WHIRL tree -- instrument
    PU_PROFILE_HANDLES fb_handles; // empty for instrumentation
    WN_INSTRUMENT_WALKER wiw( TRUE, phase, &local_mempool, fb_handles );
    wiw.Tree_Walk( wn );
  }
  // Dispose of local memory pool
  MEM_POOL_Pop( &local_mempool );
  MEM_POOL_Delete( &local_mempool );
}


void
WN_Annotate( WN *wn, PROFILE_PHASE phase, MEM_POOL *MEM_pu_pool )
{
  Set_Error_Phase( "WN_Annotate" );
  if ( Instrumenter_DEBUG )
    DevWarn( "WN_Annotate, phase == %d", phase );

  // Prepare Cur_PU_Feedback, allocating a new FEEDBACK object if
  // necessary.  Note that feedback info might not be always available
  // (e.g., function never called)
  PU_PROFILE_HANDLES fb_handles
    = Get_PU_Profile( Cur_PU_Name, Src_File_Name,
		      Feedback_File_Info[phase] );
  if ( fb_handles.empty() ) {
    DevWarn( "Cannot find expected feedback data - function not called?" );
    return;
  } else {
    // Use CXX_DELETE( Cur_PU_Feedback, MEM_pu_pool ); first if not NULL??
    if ( Cur_PU_Feedback == NULL )
      Cur_PU_Feedback = CXX_NEW( FEEDBACK( wn, MEM_pu_pool ), MEM_pu_pool );
  }

  // Create and initialize local memory pool
  MEM_POOL local_mempool;
  MEM_POOL_Initialize( &local_mempool, "WN_INSTRUMENT_WALKER_Pool", FALSE );
  MEM_POOL_Push( &local_mempool );
  {
    // Walk the WHIRL tree -- annotate
    WN_INSTRUMENT_WALKER wiw( FALSE, phase, &local_mempool, fb_handles );
    wiw.Tree_Walk( wn );
  }
  // Dispose of local memory pool
  MEM_POOL_Pop( &local_mempool );
  MEM_POOL_Delete( &local_mempool );

  Cur_PU_Feedback->Verify("after annotation");
}


void
Set_Instrumentation_File_Name( char *fname ) 
{
  if ( fname ) {
    Instrumentation_File_Name
      = (char *) malloc( sizeof( char ) *
			 ( strlen(fname)
			   + Instrumentation_Phase_Num / 10 + 2 ) );
    sprintf( Instrumentation_File_Name, "%s%d", fname,
	     Instrumentation_Phase_Num );
    // Instrumentation_File_Name = fname;
  } else {
    DevWarn( "Instrumenter Warning: Invalid instrumentation file name." );
    Instrumentation_File_Name = "";
  }
}


// ====================================================================

