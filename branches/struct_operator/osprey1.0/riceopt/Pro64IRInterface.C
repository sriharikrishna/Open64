
//-----------------------------------------------------------------------------
//
// A Pro64-specific derivation of the IR interface.
//
// See Interface/IRInterface.h for more documentation on these functions.
//
// FIXME: a work in progress, untested, etc.
//-----------------------------------------------------------------------------

#include "Pro64IRInterface.h"

// Translate a whirl statement type into a IRStmtType.
IRStmtType
Pro64IRInterface::GetStmtType (StmtHandle h) 
{
  IRStmtType ty;
  WN *wn = (WN *) h;

  //
  // There are currently three IRStmtTypes that are not returned by this
  // function-- BREAK, LOOP_CONTINUE and STRUCT_MULTIWAY_CONDITIONAL.
  // That is, Whirl does not appear to have a structured switch statement or
  // high-level forms of break or loop continue.
  //

  switch (WN_operator (wn)) {
  //
  // Return statements.
  //
  case OPR_RETURN:
  case OPR_RETURN_VAL:
    ty = RETURN;
    break;
  //
  // Top-tested loops.
  //
  case OPR_DO_LOOP:
  case OPR_WHILE_DO:
    ty = LOOP;
    break;
  //
  // End-tested loop.
  //
  case OPR_DO_WHILE:
    ty = END_TESTED_LOOP;
    break;
  //
  // Structured IF-statement.
  //
  case OPR_IF:
    ty = STRUCT_TWOWAY_CONDITIONAL;
    break;
  //
  // Unconditional jump.
  //
  case OPR_GOTO:
    ty = UNCONDITIONAL_JUMP;
    break;
  //
  // Unconditional jump (indirect).
  //
  case OPR_AGOTO:
    ty = UNCONDITIONAL_JUMP_I;
    break;
  //
  // Unstructured two-way branches.
  //
  case OPR_TRUEBR:
    ty = USTRUCT_TWOWAY_CONDITIONAL_T;
    break;

  case OPR_FALSEBR:
    ty = USTRUCT_TWOWAY_CONDITIONAL_F;
    break;
  //
  // Unstructured multi-way branch.
  //
  case OPR_COMPGOTO:  // FIXME: Also, OPR_XGOTO?
  case OPR_SWITCH:
    ty = USTRUCT_MULTIWAY_CONDITIONAL;
    break;
  //
  // Alternate entry point.
  //
  case OPR_ALTENTRY:
    ty = ALTERNATE_PROC_ENTRY;
    break;
  //
  // A block of statements.
  //
  case OPR_BLOCK:
    ty = COMPOUND;
    break;
  //
  // Simple statements.
  // FIXME: Is this all of them?
  //
  case OPR_CALL:
  case OPR_ICALL:
  case OPR_PICCALL:
  case OPR_VFCALL:
  case OPR_INTRINSIC_CALL:
  case OPR_PRAGMA:
  case OPR_XPRAGMA:
  case OPR_ASM_STMT:
  case OPR_EVAL:
  case OPR_PREFETCH:
  case OPR_PREFETCHX:
  case OPR_COMMENT:
  case OPR_AFFIRM:
  case OPR_FORWARD_BARRIER:	// FIXME: ???
  case OPR_BACKWARD_BARRIER:	// FIXME: ???
  case OPR_LABEL:
  case OPR_WHERE:
  case OPR_IO:			// FIXME: Internal control flow possible?
  case OPR_LDID:
  case OPR_STID:
  case OPR_ILOAD:
  case OPR_ISTORE:
  case OPR_ILOADX:
  case OPR_ISTOREX:
  case OPR_MLOAD:
  case OPR_MSTORE:
  case OPR_LDBITS:
  case OPR_STBITS:
  case OPR_ILDBITS:
  case OPR_ISTBITS:
    ty = SIMPLE;
    break;
  //
  // Bother.
  //
  default:
    // FIXME:
    //        OPR_GOTO_OUTER_BLOCK,
    //        OPR_TRAP, OPR_ASSERT
    //        OPR_REGION, OPR_REGION_EXIT
    //
    dump_wn (wn);
    assert (0);
    break;
  }
  return ty;
}


// Given a compound statement, return an IRStmtIterator* for the statements.
IRStmtIterator*
Pro64IRInterface::GetFirstInCompound (StmtHandle h)
{
  WN *wn = (WN *) h;
  return new Pro64IRStmtIterator (WN_first (wn));
}


// Given a statement, return the label associated with it (or NULL if none).
StmtLabel
Pro64IRInterface::GetLabel (StmtHandle h)
{
  WN *wn = (WN *) h;

  if (WN_operator (wn) == OPR_LABEL) {
    return (StmtLabel) WN_label_number (wn);
  } else {
    return 0;
  }
}


IRStmtIterator*
Pro64IRInterface::Body (StmtHandle h)
{
  WN *wn = (WN *) h;
  WN *body_wn = 0;

  switch (WN_operator (wn)) {
  case OPR_DO_LOOP:
    body_wn = WN_do_body (wn);
    break;
  case OPR_WHILE_DO:
  case OPR_DO_WHILE:
    body_wn = WN_while_body (wn);
    break;
  default:
    // FIXME: Any other statement types here?
    assert (0);
    break;
  }

  return new Pro64IRStmtIterator (body_wn);
}


//-----------------------------------------------------------------------------
// Loops
//-----------------------------------------------------------------------------

bool
Pro64IRInterface::LoopIterationsDefinedAtEntry (StmtHandle h)
{
  WN *wn = (WN *) h;

  // In Whirl, only an OPR_DO_LOOP is specified to have Fortran semantics,
  // which means the increment is a loop-invariant expression.  However,
  // Pro64 already enforces the restrictions, so we don't have to do
  // anything special here (i.e., always return false).
  return false;
}


// The loop header is the initialization block.
StmtHandle
Pro64IRInterface::LoopHeader (StmtHandle h)
{
  WN *wn = (WN *) h;

  // This is called for all top-tested loops, but only OPR_DO_LOOP has
  // an initialization statement.
  if (WN_operator (wn) == OPR_DO_LOOP) {
    return (StmtHandle) WN_start (wn);
  } else if (WN_operator (wn) == OPR_WHILE_DO) {
    return 0;
  } else {
    assert (0);
  }
}


ExprHandle
Pro64IRInterface::GetLoopCondition (StmtHandle h)
{
  WN *wn = (WN *) h;
  WN *expr_wn = 0;

  switch (WN_operator (wn)) {
  case OPR_DO_LOOP:
    expr_wn = WN_end (wn);
    break;
  case OPR_WHILE_DO:
  case OPR_DO_WHILE:
    expr_wn = WN_while_test (wn);
    break;
  default:
    assert (0);
    break;
  }

  return (ExprHandle) expr_wn;
}


StmtHandle
Pro64IRInterface::GetLoopIncrement (StmtHandle h)
{
  WN *wn = (WN *) h;

  // This is called for all top-tested loops, but only OPR_DO_LOOP has
  // an initialization statement.
  if (WN_operator (wn) == OPR_DO_LOOP) {
    return (StmtHandle) WN_step (wn);
  } else if (WN_operator (wn) == OPR_WHILE_DO) {
    return 0;
  } else {
    assert (0);
  }
}


//-----------------------------------------------------------------------------
// Structured multiway conditionals
//-----------------------------------------------------------------------------

int
Pro64IRInterface::NumMultiCases (StmtHandle h)
{
  // Whirl does not appear to have a structured switch statement.
  assert (0);
  return 0;
}


ExprHandle
Pro64IRInterface::GetSMultiCondition (StmtHandle h, int bodyIndex)
{
  // Whirl does not appear to have a structured switch statement.
  assert (0);
  return (ExprHandle) 0;
}


ExprHandle
Pro64IRInterface::GetMultiExpr (StmtHandle h)
{
  // Whirl does not appear to have a structured switch statement.
  assert (0);
  return (ExprHandle) 0;
}


// I'm assuming bodyIndex is 0..n-1.
IRStmtIterator*
Pro64IRInterface::MultiBody (StmtHandle h, int bodyIndex)
{
  // Whirl does not appear to have a structured switch statement.
  assert (0);
  return (IRStmtIterator *) 0;
}


IRStmtIterator*
Pro64IRInterface::GetMultiCatchall (StmtHandle h)
{
  // Whirl does not appear to have a structured switch statement.
  assert (0);
  return (IRStmtIterator *) 0;
}


bool
Pro64IRInterface::IsBreakImplied (StmtHandle multicond)
{
  // Whirl does not appear to have a structured switch statement.
  assert (0);
  return false;
}


//-----------------------------------------------------------------------------
// Unstructured multiway conditionals
//-----------------------------------------------------------------------------

// Given an unstructured multi-way branch, return the number of targets.
// The count does not include the optional default/catchall target.
int
Pro64IRInterface::NumUMultiTargets (StmtHandle h)
{ 
  WN *wn = (WN *) h;

  // FIXME: Support also OPR_XGOTO?
  if (WN_operator (wn) == OPR_COMPGOTO
      || WN_operator (wn) == OPR_SWITCH) {
    return WN_num_entries (wn);
  } else {
    assert (0);
  }
}


// Given an unstructured multi-way branch, return the label of the target
// statement at 'targetIndex'. The n targets are indexed [0..n-1]. 
StmtLabel
Pro64IRInterface::GetUMultiTargetLabel (StmtHandle h, int targetIndex)
{
  WN *wn = (WN *) h;
  StmtLabel target_label = 0;
  WN *curr_goto;

  //
  // Whirl has a number of multiway branches--  OPR_SWITCH, OPR_COMPGOTO,
  // and OPR_XGOTO. SWITCH and COMPGOTO are redundant in the sense that
  // SWITCH could be used for any COMPGOTO. Nevertheless, we have to handle
  // them all.
  // FIXME: XGOTO is a lowered form of COMPGOTO which isn't handled here yet. 
  //
  assert (WN_operator (wn) == OPR_COMPGOTO || WN_operator (wn) == OPR_SWITCH);
  if (WN_operator (wn) == OPR_COMPGOTO) {
    assert (WN_operator (WN_kid1 (wn)) == OPR_BLOCK);
    curr_goto = WN_first (WN_kid1 (wn));
  } else {
    assert (WN_operator (WN_switch_table (wn)) == OPR_BLOCK);
    curr_goto = WN_first (WN_switch_table (wn));
  }

  //
  // For COMPGOTO, kid 1 is an OPR_BLOCK which contains the dispatch table.
  // It is a list of OPR_GOTOs to the corresponding targets. SWITCH is
  // similar, except its table is a list of OPR_CASEGOTOs.
  //
  // Below is somewhat inefficient, but the method wants random access to the
  // targets, while Whirl blocks have to be traversed sequentially.
  //
  int curr_idx = 0;
  while (curr_goto) {
    if (curr_idx == targetIndex) {
      assert (WN_operator (curr_goto) == OPR_GOTO
              || WN_operator (curr_goto) == OPR_CASEGOTO);
      target_label = (StmtLabel) WN_label_number (curr_goto);
      break;
    }
    curr_goto = WN_next (curr_goto);
    ++curr_idx;
  }
    
  if (curr_idx != targetIndex) {
    // Target not found, error.
    assert (0);
  }

  return target_label;
}


// Given an unstructured multi-way branch, return the label of the optional
// default/catchall target. Return 0 if no default target.
StmtLabel
Pro64IRInterface::GetUMultiCatchallLabel (StmtHandle h)
{ 
  WN *wn = (WN *) h;
  WN *target = 0;

  // FIXME: Support also OPR_XGOTO?
  if (WN_operator (wn) == OPR_COMPGOTO) {
    target = WN_kid2 (wn);
  } else if (WN_operator (wn) == OPR_SWITCH) {
    target = WN_switch_default (wn);
  } else {
    assert (0);
  }

  if (target) {
    assert (WN_operator (target) == OPR_GOTO);
    return (StmtLabel) WN_label_number (target);
  } else {
    return 0;
  }
}


// Given an unstructured multi-way branch, return the condition expression
// corresponding to target 'targetIndex'. The n targets are indexed [0..n-1].
ExprHandle
Pro64IRInterface::GetUMultiCondition (StmtHandle h, int targetIndex)
{
  // FIXME: It isn't yet decided whether or not this function is needed in
  // the IR interface.
  assert (0);
  return 0;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// Unstructured two-way branch (future loop continue?)
StmtLabel
Pro64IRInterface::GetTargetLabel (StmtHandle h, int n)
{
  WN *wn = (WN *) h;

  if (WN_operator (wn) == OPR_GOTO
      || WN_operator (wn) == OPR_TRUEBR
      || WN_operator (wn) == OPR_FALSEBR) {
    return (StmtLabel) WN_label_number (wn);
  } else {
    assert (0);
  }
}


//-----------------------------------------------------------------------------
// Structured conditionals
//
// FIXME: Is GetCondition for unstructured conditionals also?  It is currently
// implemented that way.
//-----------------------------------------------------------------------------
ExprHandle
Pro64IRInterface::GetCondition (StmtHandle h)
{
  WN *wn = (WN *) h;
  WN *expr_wn = 0;

  if (WN_operator (wn) == OPR_IF) {
    expr_wn = WN_if_test (wn);
  } else if (WN_operator (wn) == OPR_TRUEBR
             || WN_operator (wn) == OPR_FALSEBR) {
    expr_wn = WN_kid0 (wn);
  } else {
    assert (0);
  }
  return (ExprHandle) expr_wn;
}


IRStmtIterator*
Pro64IRInterface::TrueBody (StmtHandle h)
{
  WN *wn = (WN *) h;

  if (WN_operator (wn) == OPR_IF) {
    return new Pro64IRStmtIterator (WN_then (wn));
  } else {
    assert (0);
  }
}


IRStmtIterator*
Pro64IRInterface::ElseBody (StmtHandle h)
{
  WN *wn = (WN *) h;

  if (WN_operator (wn) == OPR_IF) {
    return new Pro64IRStmtIterator (WN_else (wn));
  } else {
    assert (0);
  }
}


//-----------------------------------------------------------------------------
// Obtain uses and defs
//-----------------------------------------------------------------------------
IRUseDefIterator*
Pro64IRInterface::GetUses (StmtHandle h)
{
  WN *wn = (WN *) h;
  return new Pro64IRUseDefIterator (wn, IRUseDefIterator::Uses);
}


IRUseDefIterator*
Pro64IRInterface::GetDefs (StmtHandle h)
{
  WN *wn = (WN *) h;
  return new Pro64IRUseDefIterator (wn, IRUseDefIterator::Defs);
}


// FIXME: Hack.  Only used from CFG::ltnode.
char *
IRGetSymNameFromLeafHandle(LeafHandle vh)
{
  WN *wn = (WN *) vh;

  assert (OPERATOR_has_sym (WN_operator (wn)));
  if (WN_st (wn)) {
    return (char *) ST_name (WN_st (wn));
  } else {
    return "<ERROR? st == 0>";
  }
}


//-----------------------------------------------------------------------------
// Creation of use and def lists.
//-----------------------------------------------------------------------------

// FIXME: All of this is new, wip and completely untested.

// The Pro64IRUseDefIterator constructor.
// Build the list of uses and defs for this statement. The node list iterator
// will be initialized to either the list of uses or the list of defs.
Pro64IRUseDefIterator::Pro64IRUseDefIterator (WN *subtree, int uses_or_defs)
{
  if (subtree && ! OPERATOR_is_not_executable (WN_operator (subtree))) {
    build_use_def_lists (subtree, 0);
  }

  if (uses_or_defs == IRUseDefIterator::Uses) {
    node_list_iter = uses_node_list.begin ();
    node_list_end = uses_node_list.end ();
  } else {
    node_list_iter = defs_node_list.begin ();
    node_list_end = defs_node_list.end ();
  }
}


// Determine the nodes corresponding to the uses (r-values) of 't' or to
// the definitions (l-values) of 't'.
//
// These are stored in the given private members uses_node_list and
// defs_node_lists.
//
// Parameter lhs_of_store indicates that the LHS of a store is being
// processed, so that an LDA can be correctly determined to be a use or
// definition.
void
Pro64IRUseDefIterator::build_use_def_lists (WN *t, int flags)
{
  enum { OuterMost_OPR_ARRAY = 1 };
  OPERATOR opr = WN_operator (t);
  switch (opr) {
  // FIXME: ISTOREBITS, ISTOREX, MSTORE?
  case OPR_LDID:
  case OPR_LDBITS:
  case OPR_IDNAME:
    assert (OPERATOR_has_sym (opr));
    uses_node_list.push_back (t);
    return;
  case OPR_STID:
  case OPR_STBITS:
    assert (OPERATOR_has_sym (opr));
    defs_node_list.push_back (t);
    break;
  case OPR_LDA:
    assert (OPERATOR_has_sym (opr));
    // Perhaps strange, but Whirl has the notion of taking the address of a
    // constant.  The FORTRAN call foo(2) in Whirl is foo(OPR_LDA(2)). We
    // don't want a constant noted as a use or def.
    if (ST_sym_class (WN_st (t)) != CLASS_CONST) {
      uses_node_list.push_back (t);
    }
    return;
  case OPR_LDMA:
    assert (OPERATOR_has_sym (opr));
    uses_node_list.push_back (t);
    return;
  case OPR_ISTORE:
    // Process kid0, the RHS of the store.
    build_use_def_lists (WN_kid0 (t), 0);
    // Process kid1, the LHS of the store.
    build_use_def_lists (WN_kid1 (t), OuterMost_OPR_ARRAY);
    return;
  case OPR_ARRAY:
  case OPR_ARRSECTION:
    // Kid 0 is an LDA or LDID which represents the base of the array being
    // referenced or defined. Only an outermost OPR_ARRAY of a subtree will
    // ever represent a definition of an array section or element (e.g., In
    // reference A(G(I)) = X, variable A is a definition, while G is a use).
    WN *base = WN_kid0 (t);
    assert (WN_operator (base) == OPR_LDA || WN_operator (base) == OPR_LDID);
    if ((flags & OuterMost_OPR_ARRAY)) {
      defs_node_list.push_back (base);
    } else {
      uses_node_list.push_back (base);
    }
    flags &= ~OuterMost_OPR_ARRAY;
    // Kids 1..n are dimensions, which we need to ignore. Kids n+1..2n
    // are the index expressions.
    int ndims = WN_kid_count (t) >> 1;
    for (int kid = 0; kid < ndims; kid++) {
      if (! OPERATOR_is_stmt (WN_operator (WN_kid (t, kid+ndims+1)))) {
        build_use_def_lists (WN_kid (t, kid+ndims+1), flags);
      }
    } // for kids
    return;
  } // switch

  // Recursively visit all kids (that aren't statements) adding their
  // uses and defs.
  for (int kid = 0; kid < WN_kid_count (t); kid++) {
    if (! OPERATOR_is_stmt (WN_operator (WN_kid (t, kid)))) {
      build_use_def_lists (WN_kid (t, kid), flags);
    }
  } // for kids
}


//-----------------------------------------------------------------------------
// A private method to visualize Whirl expressions in a compact way.
//
// This is strictly for debugging. It does not attempt, for example, to
// properly parenthesize expressions, etc.
//-----------------------------------------------------------------------------
void
Pro64IRInterface::dump_wn_subtree (WN *wn, ostream &os)
{
  if (wn == 0)
    return;

  OPERATOR opr = WN_operator (wn);
  string op;
  switch (opr) {
  // Leaf operations and memory operations.
  case OPR_STID:
    PrintLeaf ((LeafHandle) wn, os);
    os << " = ";
    dump_wn_subtree (WN_kid0 (wn), os);
    break;
  case OPR_ISTORE:
    dump_wn_subtree (WN_kid1 (wn), os);
    os << " = ";
    dump_wn_subtree (WN_kid0 (wn), os);
    break;
  case OPR_ISTOREX:
    os << "<ISTOREX> FIXME";
    break;
  case OPR_ILOAD:
    dump_wn_subtree (WN_kid0 (wn), os);
    break;
  case OPR_LDA:
    // Perhaps strange, but Whirl has the notion of the address of a
    // constant.  The FORTRAN call foo(2) in Whirl is foo(OPR_LDA(2)).
    if (ST_sym_class (WN_st (wn)) == CLASS_CONST) {
      os << Targ_Print (0, WN_val (wn));
    } else {
      (*this).PrintLeaf ((LeafHandle) wn, os);
    }
    break;
  case OPR_LDMA:
  case OPR_LDID:
  case OPR_IDNAME:
    (*this).PrintLeaf ((LeafHandle) wn, os);
    break;
  case OPR_CONST:
    os << Targ_Print (0, WN_val (wn));
    break;
  case OPR_INTCONST:
    os << WN_const_val (wn);
    break;
  // Unary expression operations.
  case OPR_CVT:
  case OPR_CVTL:
  case OPR_TAS:
    os << OPCODE_name (WN_opcode (wn)) << "(";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << ")";
    break;
  case OPR_PARM:
    if (WN_flag (wn) & WN_PARM_BY_REFERENCE)
      os << "&"; 
    dump_wn_subtree (WN_kid0 (wn), os);
    break;
  case OPR_ABS:
    os << "ABS(";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << ")";
    break;
  case OPR_SQRT:
    os << "SQRT(";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << ")";
    break;
  case OPR_RSQRT:
    os << "RSQRT(";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << ")";
    break;
  case OPR_RECIP:
    os << "RECIP(";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << ")";
    break;
  case OPR_PAREN:
    os << "(";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << ")";
    break;
  case OPR_RND:
    os << "RND(";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << ")";
    break;
  case OPR_TRUNC:
    os << "TRUNC(";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << ")";
    break;
  case OPR_CEIL:
    os << "CEIL(";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << ")";
    break;
  case OPR_FLOOR:
    os << "FLOOR(";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << ")";
    break;
  case OPR_NEG:
    op = "-";
    goto print_generic_unary;
  case OPR_BNOT:
    op = "~";
    goto print_generic_unary;
  case OPR_LNOT:
    op = "!";
    goto print_generic_unary;
print_generic_unary:
    os << op;
    dump_wn_subtree (WN_kid0 (wn), os);
    break;
  // Binary expression operations.
  case OPR_ADD:
    op = "+";
    goto print_generic_binary;
  case OPR_SUB:
    op = "-";
    goto print_generic_binary;
  case OPR_MPY:
    op = "*";
    goto print_generic_binary;
  case OPR_DIV:
    op = "/";
    goto print_generic_binary;
  case OPR_MOD:
    os << "MOD(";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << ",";
    dump_wn_subtree (WN_kid1 (wn), os);
    os << ")";
    break;
  case OPR_REM:
    op = "%";
    goto print_generic_binary;
  case OPR_EQ:
    op = "==";
    goto print_generic_binary;
  case OPR_NE:
    op = "!=";
    goto print_generic_binary;
  case OPR_GE:
    op = ">=";
    goto print_generic_binary;
  case OPR_LE:
    op = "<=";
    goto print_generic_binary;
  case OPR_GT:
    op = '>';
    goto print_generic_binary;
  case OPR_LT:
    op = '<';
    goto print_generic_binary;
  case OPR_MAX:
    os << "MAX(";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << ",";
    dump_wn_subtree (WN_kid1 (wn), os);
    os << ")";
    break;
  case OPR_MIN:
    os << "MIN(";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << ",";
    dump_wn_subtree (WN_kid1 (wn), os);
    os << ")";
    break;
  case OPR_SHL:
    op = "<<";
    goto print_generic_binary;
  case OPR_ASHR:
  case OPR_LSHR:
    op = ">>";
    goto print_generic_binary;
  case OPR_LAND:
    op = "&&";
    goto print_generic_binary;
  case OPR_LIOR:
    op = "||";
    goto print_generic_binary;
  case OPR_BAND:
    op = "&";
    goto print_generic_binary;
  case OPR_BIOR:
    op = "|";
    goto print_generic_binary;
  case OPR_BXOR:
    op = "^";
print_generic_binary:
    dump_wn_subtree (WN_kid0 (wn), os);
    os << op;
    dump_wn_subtree (WN_kid1 (wn), os);
    break;
  // Ternary operations.
  case OPR_SELECT:
    dump_wn_subtree (WN_kid0 (wn), os);
    os << " ? "; 
    dump_wn_subtree (WN_kid1 (wn), os);
    os << " : "; 
    dump_wn_subtree (WN_kid2 (wn), os);
    break;
  case OPR_MADD:
    os << "(";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << "*";
    dump_wn_subtree (WN_kid1 (wn), os);
    os << ")+";
    dump_wn_subtree (WN_kid2 (wn), os);
    break;
  case OPR_MSUB:
    os << "(";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << "*";
    dump_wn_subtree (WN_kid1 (wn), os);
    os << ")-";
    dump_wn_subtree (WN_kid2 (wn), os);
    break;
  case OPR_NMADD:
    os << "-((";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << "*";
    dump_wn_subtree (WN_kid1 (wn), os);
    os << ")+";
    dump_wn_subtree (WN_kid2 (wn), os);
    os << ")";
    break;
  case OPR_NMSUB:
    os << "-((";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << "*";
    dump_wn_subtree (WN_kid1 (wn), os);
    os << ")-";
    dump_wn_subtree (WN_kid2 (wn), os);
    os << ")";
    break;
  // N-ary operations.
  case OPR_ARRAY: {
    int ndims = WN_kid_count (wn) >> 1;
    dump_wn_subtree (WN_kid0 (wn), os);
    os << "(";
    for (int i = 0; i < ndims; i++) {
      dump_wn_subtree (WN_kid (wn, i+ndims+1), os);
      if (i < ndims-1) 
        cout << ",";
    }
    os << ")";
    break;
  }
  case OPR_TRIPLET: // Output as LB:UB:STRIDE
    dump_wn_subtree (WN_kid0 (wn), os);
    os << ":";
    dump_wn_subtree (WN_kid2 (wn), os);
    os << ":";
    dump_wn_subtree (WN_kid1 (wn), os);
    break; 
  case OPR_ARRAYEXP:
    dump_wn_subtree (WN_kid0 (wn), os);
    break; 
  case OPR_ARRSECTION: {
    int ndims = WN_kid_count (wn) >> 1;
    dump_wn_subtree (WN_kid0 (wn), os);
    os << "(";
    for (int i = 0; i < ndims; i++) {
      dump_wn_subtree (WN_kid (wn, i+ndims+1), os);
      if (i < ndims-1) 
        cout << ",";
    }
    os << ")";
    break;
  }
  // Control flow constructs
  case OPR_DO_LOOP:
    // In OA, the DO_LOOP node represents the loop condition.
    os << "do_loop (";
    dump_wn_subtree (WN_end (wn), os);
    os << ")"; 
    break;
  case OPR_DO_WHILE:
    // In OA, the DO_WHILE node represents the loop condition.
    os << "do_while (";
    dump_wn_subtree (WN_while_test (wn), os);
    os << ")"; 
    break;
  case OPR_WHILE_DO:
    // In OA, the WHILE_DO node represents the loop condition.
    os << "while_do ("; 
    dump_wn_subtree (WN_while_test (wn), os);
    os << ")";
    break;
  case OPR_IF:
    // In OA, the IF node represents the condition.
    os << "if ("; 
    dump_wn_subtree (WN_if_test (wn), os);
    os << ")"; 
    break;
  case OPR_TRUEBR:
    // In OA, the TRUEBR node represents the condition.
    os << "truebr ("; 
    dump_wn_subtree (WN_kid0 (wn), os);
    os << "), L" << WN_label_number (wn); 
    break;
  case OPR_FALSEBR:
    // In OA, the FALSEBR node represents the condition.
    os << "falsebr (";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << "), L" << WN_label_number (wn); 
    break;
  case OPR_RETURN:
    os << "return";
    break;
  case OPR_RETURN_VAL:
    os << "return (";
    dump_wn_subtree (WN_kid0 (wn), os);
    os << ")";
    break;
  case OPR_CALL:
    os << ST_name (WN_st (wn)) << "(";
    for (int kid = 0; kid < WN_kid_count (wn); kid++) {
      dump_wn_subtree (WN_kid (wn, kid), os);
      if (kid < WN_kid_count (wn)-1)
        os << ", ";
    } // for kids
    os << ")";
    break;
  case OPR_INTRINSIC_CALL:
  case OPR_INTRINSIC_OP:
    os << INTRINSIC_name ((INTRINSIC) WN_intrinsic (wn)) << "(";
    for (int kid = 0; kid < WN_kid_count (wn); kid++) {
      dump_wn_subtree (WN_kid (wn, kid), os);
      if (kid < WN_kid_count (wn)-1)
        os << ", ";
    } // for kids
    os << ")";
    break;
  default:
    dump_wn (wn);
    break;
  }
}

