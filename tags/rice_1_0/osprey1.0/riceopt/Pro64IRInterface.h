
#ifndef Pro64IRInterface_h
#define Pro64IRInterface_h

//-----------------------------------------------------------------------------
//
// A Pro64-specific derivation of the IR interface.
//
// FIXME: Just started, work in progress.
//-----------------------------------------------------------------------------

#include <cassert>
#include <list>
#include <string>

// OpenAnalysis headers.
// FIXME: I just noticed that Name and DGraph need to be included in
// IRInterface.h.  For now I just include them manually here. 
#include "Utils/DGraph.h"
#include "Interface/IRInterface.h"

// Pro64 headers.
#include "wn.h"
#include "ir_reader.h"		// For dump_wn().
#include "wutil.h"             // For intrinsic info used in dump_wn_subtree.


class Pro64IRStmtIterator: public IRStmtIterator {
public:
  Pro64IRStmtIterator (WN *wn) { curr_wn = wn; }
  ~Pro64IRStmtIterator () {}

  StmtHandle Current () { return (StmtHandle) curr_wn; }
  bool IsValid () { return (curr_wn != 0); }
  void operator++ () { curr_wn = WN_next (curr_wn) ? WN_next (curr_wn) : 0; }

private:
  WN *curr_wn;   
};


class Pro64IRUseDefIterator: public IRUseDefIterator {
// FIXME: New, wip, and untested.
public:
  Pro64IRUseDefIterator () { assert (0); }
  Pro64IRUseDefIterator (WN *n, int uses_or_defs);
  ~Pro64IRUseDefIterator () {}

  LeafHandle Current () { return (LeafHandle) (*node_list_iter); }
  bool IsValid () { return (node_list_iter != node_list_end); }
  void operator++ () { ++node_list_iter; };
	
private:
  std::list<WN *> uses_node_list;
  std::list<WN *> defs_node_list;
  std::list<WN *>::iterator node_list_iter;
  std::list<WN *>::iterator node_list_end;
  void build_use_def_lists (WN *, int);
};


class Pro64IRInterface : public IRInterface {
public:
  ~Pro64IRInterface () {}
  IRStmtType GetStmtType (StmtHandle h);
  StmtLabel GetLabel (StmtHandle h);

  //------------------------------
  // for compound statement. 
  //------------------------------
  IRStmtIterator *GetFirstInCompound (StmtHandle h);

  //------------------------------
  // for procedure, loop
  //------------------------------
  IRStmtIterator *Body (StmtHandle h);

  //------------------------------
  // loops
  //------------------------------
  StmtHandle LoopHeader (StmtHandle h);
  bool LoopIterationsDefinedAtEntry (StmtHandle h);
  ExprHandle GetLoopCondition (StmtHandle h); 
  StmtHandle GetLoopIncrement (StmtHandle h);

  //------------------------------
  // invariant: a two-way conditional or a multi-way conditional MUST provide
  // provided either a target, or a target label
  //------------------------------

  //------------------------------
  // unstructured two-way conditionals: 
  //------------------------------
  // two-way branch, loop continue
  StmtLabel  GetTargetLabel (StmtHandle h, int n);

  ExprHandle GetCondition (StmtHandle h);

  //------------------------------
  // unstructured multi-way conditionals
  //------------------------------
  int NumUMultiTargets (StmtHandle h);
  StmtLabel GetUMultiTargetLabel (StmtHandle h, int targetIndex);
  StmtLabel GetUMultiCatchallLabel (StmtHandle h);
  ExprHandle GetUMultiCondition (StmtHandle h, int targetIndex);

  //------------------------------
  // structured multiway conditionals
  //------------------------------
  int NumMultiCases (StmtHandle h);
  // condition for multi body 
  ExprHandle GetSMultiCondition (StmtHandle h, int bodyIndex);
  // multi-way beginning expression
  ExprHandle GetMultiExpr (StmtHandle h);
  IRStmtIterator *MultiBody (StmtHandle h, int bodyIndex);
  bool IsBreakImplied (StmtHandle multicond);
  IRStmtIterator *GetMultiCatchall (StmtHandle h);

  //------------------------------
  // structured conditionals
  //------------------------------
  IRStmtIterator *TrueBody (StmtHandle h);
  IRStmtIterator *ElseBody (StmtHandle h);

  //------------------------------
  // obtain uses and defs
  //------------------------------
  IRUseDefIterator *GetUses (StmtHandle h);
  IRUseDefIterator *GetDefs (StmtHandle h);

  SymHandle GetSymHandle (LeafHandle vh) {
    WN *wn = (WN *) vh;
    assert (OPERATOR_has_sym (WN_operator (wn)));
    return (SymHandle) WN_st (wn); 
  }

  // Given a SymHandle, return the textual name.
  char *GetSymNameFromSymHandle (SymHandle sh) {
    ST *st = (ST *) sh;
    return (char *) ST_name (st); 
  }

  void PrintLeaf (LeafHandle vh, ostream & os) {
    WN *wn = (WN *) vh;
    if (OPERATOR_has_sym (WN_operator (wn)) && WN_st (wn)) {
      os << ST_name (WN_st (wn));
    }
  }

  void Dump (StmtHandle stmt, ostream& os) {
    WN *wn = (WN *) stmt;
    dump_wn_subtree (wn, os);
    os << "\n";
  }

private:
  // Dump Whirl subtree in a compact, friendly format.
  void dump_wn_subtree (WN *, ostream &);
};



#endif // Pro64IRInterface_h
