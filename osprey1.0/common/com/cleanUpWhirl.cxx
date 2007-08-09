#include <map>
#include <set>
#include "cleanUpWhirl.h"
#include "wn_tree_util.h"
#include "wn_util.h"
#include "symtab.h"

#define xDEBUG(flag,code) { if (flag) {code; fflush(stdout);} }
#define DEB_CleanUpWhirl 0

// the front-end introduces temporary variables 
// for all kinds of expressions, in particular 
// control flow related and in indices, already before the 
// conversion to whirl.  These temporaries obfuscate 
// the data dependencies by introducing an extra 
// level of indirection. Replacing the temporary 
// references with the respective right-hand-side 
// expressions of their assignments is possible everywhere with 
// the one exception of dimension computations within 
// variable declarations. In the latter case there is no
// location to represent these computations in whirl other than 
// an explicit  assignment statement to a temporary. 
// Therefore we currently do not remove any temporary assignments or 
// the respective entries in the  symbol table. 
void cleanUpPUInfo(PU_Info* aPUInfo_p) { 
  typedef std::map<ST*,WN*> STPtoWNPmap;
  STPtoWNPmap tempMap; 
  bool skipKids=false;
  WN* thePU_WN_p = PU_Info_tree_ptr(aPUInfo_p);
  WN* parentWN_p=0;
  WN_TREE_CONTAINER<PRE_ORDER> aWNPtree(thePU_WN_p);
  WN_TREE_CONTAINER<PRE_ORDER>::iterator aWNPtreeIterator=aWNPtree.begin();
  while (aWNPtreeIterator != aWNPtree.end()) { 
    WN* curWN_p = aWNPtreeIterator.Wn();
    OPERATOR opr = WN_operator(curWN_p);
    if (opr==OPR_STID) {  // definitions
      if (ST_is_temp_var(WN_st(curWN_p))) {
	ST* tempST_p=WN_st(curWN_p);
	// is it not in the set? 
	if (tempMap.find(tempST_p) == tempMap.end()) { //not found
	  // add it
	  tempMap.insert(std::pair<ST*,WN*>(tempST_p,WN_kid0(curWN_p)));
	  const char* tmpName = ST_name(tempST_p); 
	  ST* puST_p = ST_ptr(PU_Info_proc_sym(aPUInfo_p));
	  const char* puName = ST_name(puST_p);
	  xDEBUG(DEB_CleanUpWhirl, printf("cleanUpWhirl: recorded temporary %s defined in %s\n",tmpName, puName););
	}
	else { // this should not happen since these are supposed to be single assignment
	  const char* tmpName = ST_name(tempST_p); 
	  ST* puST_p = ST_ptr(PU_Info_proc_sym(aPUInfo_p));
	  const char* puName = ST_name(puST_p);
	  DevWarn("cleanUpWhirl: recorded temporary %s is redefined in %s\n",tmpName, puName);
	}
      }
    }
    if (opr==OPR_LDID){ // uses
      // if we refer to a temp variable
      if (ST_is_temp_var(WN_st(curWN_p))) { 
	// that variable should have been added to the set
	// so find it: 
	ST* tempST_p=WN_st(curWN_p);
	STPtoWNPmap::iterator mapIter=tempMap.find(tempST_p);
	if (mapIter==tempMap.end()) { //not found
	  // this shouldn't happen since we expect to have all of the definitions
	  const char* tmpName = ST_name(tempST_p); 
	  ST* puST_p = ST_ptr(PU_Info_proc_sym(aPUInfo_p));
	  const char* puName = ST_name(puST_p);
	  Fatal_Error("cleanUpWhirl: no definition for temporary %s in %s\n",tmpName,puName);
	}
	// make sure the parent is set by now
	if (!aWNPtreeIterator.Get_parent_wn())
	  Fatal_Error("cleanUpWhirl: no parent set");
	// replace the current node within the parent
	WN_kid(aWNPtreeIterator.Get_parent_wn(),aWNPtreeIterator.Get_kid_index()) = WN_COPY_Tree((*mapIter).second);
	skipKids=true;
	const char* tmpName = ST_name(tempST_p); 
	ST* puST_p = ST_ptr(PU_Info_proc_sym(aPUInfo_p));
	const char* puName = ST_name(puST_p);
	xDEBUG(DEB_CleanUpWhirl, printf("cleanUpWhirl: subsituted temporary %s in %s\n",tmpName, puName););
      }
    } 
    // advance the iterator
    if (skipKids || opr==OPR_XPRAGMA){
      // XPRAGMAs may refer to temporaries before they are assigned
      aWNPtreeIterator.WN_TREE_next_skip();
      skipKids=false;
    }
    else
      ++aWNPtreeIterator;
  } // end while
}

// the C wrapper with proper traversal
extern "C" void cleanUpPUInfoTree(PU_Info* aPUInfoTree_p) { 
  if (!aPUInfoTree_p) { return; }
  cleanUpPUInfo(aPUInfoTree_p);
  for (PU_Info *aPUInfosubtree_p = PU_Info_child(aPUInfoTree_p); 
       aPUInfosubtree_p != NULL; 
       aPUInfosubtree_p = PU_Info_next(aPUInfosubtree_p)) {
    cleanUpPUInfo(aPUInfosubtree_p);
  }
}
