
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>

#include <values.h>
#include "defs.h"
#include "errors.h"
#include "gnu_config.h"
#include "gnu/flags.h"
extern "C" {
#include "gnu/system.h"
#include "gnu/tree.h"
#include "gnu/toplev.h"
#include "gnu/upc-act.h"
}

#include "glob.h"
#include "symtab.h"
#include "strtab.h"
#include "tree_symtab.h"
#include "wn.h"
#include "wfe_expr.h"
#include "wfe_misc.h"
#include "wfe_dst.h"
#include "ir_reader.h"
#include <cmplrs/rcodes.h>

#include "cxx_memory.h"

#define NAME_LEN 256
#define INIT_LEN 1000
#define TYPE_LEN 64
#define LINE_LEN 2048

#define min(a,b) (a > b) ? b : a

class Decl {
public:
  bool is_static;
  string name;
  string orig_type; // original type of the decl
  UINT64 size;
  int kind;
  UINT64 esize;
  //a blkSize of 0 means indefinite blk size
  UINT blkSize; 
  string initExp;
  string dimExp;
  int* initDim;

  Decl(string name, UINT64 size, UINT blkSize, UINT64 esize, char kind, string init, string orig_type, string dim);
  void print();
  bool is_pshared();
  bool hasInit();
  UINT64 get_bsize();
  UINT64 get_num_blk();
  unsigned int get_num_dim();
  string get_next_dimlen();

private:
  int curPos; // current position in the dimension expression
  void get_init_dim();
};

vector<Decl*> file_vars;

string decls = "";
string static_alloc = "";
string static_init = "";
string TLD_init = "";

//list of upc runtime functions
const string SHARED = "upcr_shared_ptr_t ";
const string PSHARED = "upcr_pshared_ptr_t ";
const string ISNULL = "upcr_isnull_shared";
const string ISNULL_P = "upcr_isnull_pshared";
const string S_TO_P = "upcr_shared_to_pshared";
const string P_TO_S = "upcr_pshared_to_shared";
const string S_TO_L = "upcr_shared_to_local";
const string P_TO_L = "upcr_pshared_to_local";
const string ALL_ALLOC = "upcr_all_alloc";
const string MEMSET = "upcr_memset";
const string MEMPUT = "upcr_memput";
const string PUT_P = "upcr_put_pshared";
const string PUT_S = "upcr_put_shared";
const string INIT_ARRAY = "upcr_startup_initarray";
const string PINIT_ARRAY = "upcr_startup_initparray";
const string SHALLOC = "upcr_startup_shalloc";
const string PSHALLOC = "upcr_startup_pshalloc";
const string SHALLOC_T = "upcr_startup_shalloc_t";
const string PSHALLOC_T = "upcr_startup_pshalloc_t";
const string MY = "upcr_mythread";
const string INIT = "UPCR_INITIALIZED_SHARED";
const string INIT_P = "UPCR_INITIALIZED_PSHARED";
const string TLD_DECL = "UPCR_TLD_DEFINE";
const string TLD_ADDR = "UPCR_TLD_ADDR";
const string THREADS = "upcr_threads";


enum kinds {SCALAR, ARRAY, POINTER, STRUCT};

//soem helper functions
string utoa(UINT64 i) {

  string val = "";
  while (i >= 10) {
    val = (char) ((i % 10) + '0') + val;  
    i = i / 10;
  }
  return ((char) (i + '0')) + val;
}
 
string itoa(INT64 i) {
  
  if (i < 0) {
    return "-" + utoa(-i);
  } else {
    return utoa(i);
  }
}

//Convert name to a legal C identifier,
//by stripping off .c in end of filename, as well as any leading /
//If id == true, this also converts the string into a legal C identifier(letters+digits)
string strip(string name, bool id = false) {

  string ans;
  int index = name.find_last_of(".");
  ans = (index > 0) ? name.substr(0, index) : name;
  index = ans.find_last_of("/");
  ans = (index > 0) ? ans.substr(index+1, ans.size() - index - 1) : ans;
  if (id) {
    if (!isalpha(ans[0])) {
      //convert first character to a letter
      ans[0] = '_';
    }
    for (int i = 1; i < ans.size(); i++) {
      if (!isalnum(ans[i])) {
	ans[i] = '_';
      }
    }
  }
  return ans;
}

static string remove_addrof(string s) {
  if (s[0] == '&') {
    return s.substr(1, s.size() - 1);
  } 
  return s;
}

/* output the shared pointer declarations */
static string output_decl(Decl* decl) {
  
  string result = "";

  if (decl->is_static) {
    result += "static ";
  }

  result += decl->is_pshared() ? PSHARED : SHARED;
  result += decl->name;
  if (decl->hasInit()) {
    if (decl->kind == POINTER && decl->initExp == "0") {
      result += " = UPCR_NULL_SHARED";
    } else {
      result += " = " + (decl->is_pshared() ? INIT_P : INIT);
    }
  }
  result += ";\n";
  return result;
}

static string cast_init_exp(bool to_pshared, string init_exp) {

  for (vector<Decl*>::const_iterator iter=file_vars.begin(); iter!=file_vars.end(); iter++) {
    Decl* d = *iter;
    if (d->name == init_exp) {
      if (to_pshared && !d->is_pshared()) {
	return S_TO_P + "(" + init_exp + ")";
      }
      if (!to_pshared && d->is_pshared()) {
	return P_TO_S + "(" + init_exp + ")";
      }
      break;
    }
  }
  return init_exp;
}

void process_shared(const char* file_name) {

  char kind, stat;
  unsigned long size, esize;
  UINT bsize;
  char name[NAME_LEN], init[INIT_LEN], orig_type[TYPE_LEN], dim[TYPE_LEN];
  char line[LINE_LEN];

  //declarations the allocation and initialization function for shared data
  static_alloc += "void UPCRI_ALLOC_" + strip(Orig_Src_File_Name, true) + "_" + utoa(hash_val(Orig_Src_File_Name)) + "() { \n";
  static_alloc += "UPCR_BEGIN_FUNCTION();\n";
  static_init += "void UPCRI_INIT_" + strip(Orig_Src_File_Name, true) + "_" + utoa(hash_val(Orig_Src_File_Name)) + "() { \n";
  static_init += "UPCR_BEGIN_FUNCTION();\n";

  //output the internally used symbol "upc_forall_control"
  decls += "extern int upcr_forall_control;\n";

  if (strlen(file_name) == 0) {
    return; //no shared global variable in the program
  }

  FILE* in = fopen(file_name, "r");
  
  //temporray buffer to store the absolute path of the file
  char hash_name[MAX_PATH];

  while (fgets(line, LINE_LEN, in) != NULL) {
    /* format of the symbol file:
       Name  size(bytes)  block size(number of elements)  element size(bytes)  kind(one of array(A), scalar(S), struct(M), and pointer(P))  initial expression
    */

    if (sscanf(line, "%s\t%ul\t%u\t%ul\t%c\t%s\t%s\t%s\n", name, &size, &bsize, &esize, &kind, init, orig_type, dim) != 8) {
      cerr << "invaliad line format:<" << line << ">" << endl;
    } else {
      Decl* decl = CXX_NEW(Decl(name, (UINT64) size, bsize, (UINT64) esize, kind, init, orig_type, dim), &MEM_src_pool);
      file_vars.push_back(decl);
    }
  }

  for (vector<Decl*>::const_iterator iter=file_vars.begin(); iter!=file_vars.end(); iter++) {
    decls += output_decl(*iter);
  }
  
  string shared = "" , pshared = "";

  //create a variable to store each initialization var (something like <orig_type> foo_val = foo_init;)
  for (vector<Decl*>::const_iterator iter=file_vars.begin(); iter!=file_vars.end(); iter++) {
    Decl* d = *iter;
    string flag = "0";
    if (d->kind == ARRAY && threads_int == 0 &&
	d->blkSize != 0) {
      flag = "1";
    }

    if (d->is_pshared()) {
      pshared += "{&" + d->name + ", ";
      pshared += utoa(d->get_bsize());
      pshared +=  ", " + utoa(d->get_num_blk()) + ", ";
      pshared += flag + "}, \n";
    } else {
      shared += "{&" + d->name + ", " + utoa(d->get_bsize()) + ", " + utoa(d->get_num_blk()) + ", ";
      shared += flag + "}, \n";
    }
  }

  if (shared != "") {
    static_alloc += SHALLOC_T + " info[] = { \n";
    static_alloc += shared;
    static_alloc += " };\n";
  }
  if (pshared != "") {
    static_alloc += PSHALLOC_T + " pinfo[] = { \n";
    static_alloc += pshared;
    static_alloc += " };\n";
  }

  static_alloc += "\n";;

  if (shared != "") {
    static_alloc += SHALLOC + "(info, ";
    static_alloc += "sizeof(info) / sizeof(" + SHALLOC_T + "));\n";
  }
  if (pshared != "") {
    static_alloc += PSHALLOC + "(pinfo, ";
    static_alloc += "sizeof(pinfo) / sizeof(" + PSHALLOC_T + "));\n";
  }

  //now do init function
  for (vector<Decl*>::const_iterator iter=file_vars.begin(); iter!=file_vars.end(); iter++) {
    Decl* d = *iter;
    if (!d->hasInit()) {
      continue;
    }
    switch (d->kind) {
    case STRUCT:
    case SCALAR:
      static_init += d->orig_type + " " + d->name + "_val = " + d->initExp + ";\n";
      break;
    case POINTER:
      break;
    case ARRAY:
      static_init += d->orig_type + " " + d->name + "_val";
      for (int i = 0; i < d->get_num_dim(); i++) {
	static_init += "[" + utoa(d->initDim[i]) + "]";
      }
      static_init += " = " + d->initExp + ";\n"; 
      static_init += "upcr_startup_arrayinit_diminfo_t ";
      static_init += d->name + "_diminfos[] = {\n";
      for (int i = 0; i < d->get_num_dim(); i++) {
	string dim_len = d->get_next_dimlen();
	bool has_thread = false;
	if (dim_len[0] == 'T') {
	  dim_len = dim_len.substr(1, dim_len.size() - 1);
	  has_thread = true;
	}
	static_init += "{" + utoa(d->initDim[i]) + ", " + dim_len + ", " + (has_thread ? "1" : "0") + "},\n";
      }
      static_init += "};\n";
      break;
    }
  }

  for (vector<Decl*>::const_iterator iter=file_vars.begin(); iter!=file_vars.end(); iter++) {
    Decl* d = *iter;
    Decl* rhs;

    if (!d->hasInit()) {
      continue;
    }
    switch (d->kind) {
    case POINTER:
      if (d->initExp == "0") {
	break; //Pointer is already initialized to null during declaration
      }
      //fall through
    case SCALAR:
    case STRUCT:
      //ouput something like upcr_put_shared(ptr, 0, ptr_val, size)
      static_init += "if (" + MY + "()== 0) { \n";
      static_init += (d->is_pshared() ? PUT_P : PUT_S) + "(" + d->name;
      static_init += ", 0"; 
      if (d->kind == POINTER) {
	static_init += ", &" + cast_init_exp(d->is_pshared(), d->initExp);
      } else {
	static_init += ", &" + d->name + "_val"; 
      }
      static_init += ", " + utoa(d->size) + ");\n";
      static_init += "}\n";
      break;
    case ARRAY: 
      static_init += (d->is_pshared() ? PINIT_ARRAY : INIT_ARRAY) + "(";
      static_init += d->name + ", ";
      static_init += d->name + "_val, ";      
      static_init += d->name + "_diminfos, ";
      static_init += utoa(d->get_num_dim()) + ", ";
      static_init += utoa(d->esize) + ", ";
      static_init += utoa(d->blkSize) + ");\n";
      break;
    }
    static_init += "\n";
  }
  decls += "\n";
}


/**
 *
 * Handles TLD variables
 */
void process_nonshared(ST_IDX st, tld_pair_p info) {

  string type = info->type;
  TY_IDX ty = ST_type(st);
  int kind = TY_kind(ty);
  string init = info->init_exp;
  //used for array types
  int idx = type.find_last_of("]");
  //used for function pointer types
  int p_idx = type.find("(*");
  string name = ST_name(st); 

  bool is_function = kind == KIND_POINTER && p_idx > 0;

  /*
   *  There are two special cases where we can't use the given type string directly
   *  for arrays:  []<elt_type>
   *  for function pointers: ret_type (*)(arg_type_list)
   */
  if (ST_sclass(st) == SCLASS_EXTERN) {
    decls += "extern ";
    if (kind == KIND_ARRAY) {
      decls += type.substr(idx+1, type.size() - idx - 1) + " ";
    } else if (is_function) {
      decls += type.substr(0, p_idx + 2);
    } else {
      if (TY_is_shared(ST_type(st))) {
	decls += TY_is_pshared(ST_type(st)) ? PSHARED : SHARED;
      } else {
	decls += type + " ";
      }
    }
    decls += name;
    
    if (kind == KIND_ARRAY) {
      decls += type.substr(0, idx+1);
    } else if (is_function) {
      decls += type.substr(p_idx+2, type.size() - p_idx - 2);
    }
  } else {
    //Test if we need to output typedefs for the variable
    if (kind == KIND_ARRAY) {
      string elt_type = type.substr(idx+1, type.size() - idx - 1);
      if (p_idx > 0) {
	//Well, we have an array of function pointers, need two levels of indirection
	p_idx -= idx+1;
	decls += "typedef " + elt_type.substr(0, p_idx + 2);
	decls += "_type_" + name + "_fun";
	decls += elt_type.substr(p_idx+2, elt_type.size() - (p_idx+2));
	decls += ";\n"; 
	elt_type = "_type_" + name + "_fun";
      }
      decls += "typedef ";
      decls += elt_type + " ";
      decls += "_type_" + name;
      decls += type.substr(0, idx+1);
      decls += ";\n"; 
    } else if (is_function) {
      //we have a function pointer
      decls += "typedef ";
      decls += type.substr(0, p_idx + 2);
      decls += "_type_" + name;
      decls += type.substr(p_idx+2, type.size() - p_idx - 2);
      decls += ";\n"; 
    }
    
    if (ST_sclass(st) == SCLASS_FSTATIC || ST_sclass(st) == SCLASS_PSTATIC) {
      decls += "static ";
    }
    if (TY_is_volatile(ST_type(st))) {
      decls += "volatile ";
    }
    if (TY_is_const(ST_type(st))) {
      decls += "const ";
    }

    
    if (kind == KIND_ARRAY || (kind == KIND_POINTER && p_idx > 0)) {
      decls += "_type_" + name + " ";
    } else {
      TY_IDX ty_idx = ST_type(st);
      if (TY_kind(ty_idx) == KIND_POINTER && TY_is_shared(TY_pointed(ty_idx))) {
	if (TY_is_pshared(TY_pointed(ty_idx))) {
	  decls += PSHARED;
	} else {
	  decls += SHARED;
	}
      } else {
	decls += type + " ";
      }
    }
    decls += (init != "NONE" ? TLD_DECL : TLD_DECL + "_TENTATIVE") + "(";
    decls += name;
    decls += ", " + utoa(info->size) + ")";
    if (init != "NONE") {
      //go ahead and initialize 
      decls += " = ";
      TY_IDX ty_idx = ST_type(st);
      if (TY_kind(ty_idx) == KIND_POINTER && TY_is_shared(TY_pointed(ty_idx))) {
	int block = TY_block_size(TY_pointed(ty_idx));
	decls += (block == 0 || block == 1 ? "UPCR_INITIALIZED_PSHARED" : "UPCR_INITIALIZED_SHARED");
      } else {
	decls += init;
      }
    }
  }
  decls += ";\n"; 

  //if it's a pointer that's initialized to the address of another global variable, need some extra work to initialize it
  if (TY_kind(ST_type(st)) == KIND_POINTER && init != "NONE") {
    init = remove_addrof(init); //get rid of the leading '&', if any
    TY_IDX ty_idx = TY_pointed(ST_type(st));

    TLD_init += "*((";
    if (TY_is_shared(ty_idx)) {
       TLD_init += TY_is_pshared(ty_idx) ? PSHARED : SHARED;
       TLD_init += "*) " + TLD_ADDR;
       TLD_init += "(" + name + ")) = ";
       TLD_init += init;
    } else {
      TLD_init += type;
      TLD_init += "*) " + TLD_ADDR;
      TLD_init += "(" + name + ")) = ";
      TLD_init += "(" + type + ")";
      TLD_init += TLD_ADDR + "(";
      TLD_init += init + ")";
    }
    TLD_init += ";\n";
  }
}

void output_file() {

  string out_file = strip(Orig_Src_File_Name) + ".global_data.c";  
  FILE* out = fopen(out_file.c_str(), "w");
  fputs(decls.c_str(), out); 
  fputs(static_alloc.c_str(), out);
  fputs("}\n\n", out);
  fputs(static_init.c_str(), out);
  fputs(TLD_init.c_str(), out);
  fputs("}\n", out);
}

/**************************************
 *
 * Implementations of the Decl Class
 */
  
Decl::Decl(string name, UINT64 size, UINT blkSize, UINT64 esize, char kind, string initExp, string orig_type, string dimExp) {

  this->is_static = false;
  this->initExp = initExp;
  this->name = name;
  this->orig_type = orig_type;
  this->size = size;
  this->esize = esize;
  this->blkSize = blkSize;
  this->dimExp = dimExp;

  switch (kind) {
  case 'S': 
    this->kind = SCALAR;
    break;
  case 'P':
    this->kind = POINTER;
    break;
  case 'A':
    this->kind = ARRAY;
    get_init_dim();
    break;
  case 'M':
    this->kind = STRUCT;
    this->orig_type.replace(this->orig_type.find(","), 1, " ");
  }

  this->curPos = 0;
}

//initialize the dimension array by parsing the array initializer of this decl
void Decl::get_init_dim() {

  if (!hasInit()) {
    return;
  }

  int dim = get_num_dim();
  initDim = CXX_NEW_ARRAY(int, dim, &MEM_src_pool);
  int tmp[dim];
  for (int i = 0; i < dim; initDim[i] = 0, tmp[i] = 0, i++);


  int curLevel = -1;
  for (int i = 0; i < initExp.size(); i++) {
    switch (initExp[i]) {
    case '{':
      curLevel++;
      break;
    case ',':
      tmp[curLevel]++;
      break;
    case '}':
      if (initDim[curLevel] < tmp[curLevel]) {
	initDim[curLevel] = tmp[curLevel];
      }
      tmp[curLevel--] = 0;
      break;
    default:
      ;
      //do nothing here
    }
  }
}

bool Decl::hasInit() {
  return initExp != "NONE";
}

bool Decl::is_pshared() {
  
  if (kind == POINTER && orig_type == "VOID*") {
    return false;
  }
  return blkSize == 0 || blkSize == 1;
}

unsigned int Decl::get_num_dim() {

  if (kind != ARRAY || !hasInit()) {
    return 0;
  }
  int i;
  for (i = 0; initExp[i] == '{'; i++);
  return i;
}

UINT64 Decl::get_bsize() {

  if (kind != ARRAY || blkSize == 0) {
    //all non-array variables only reside on node 0
    return size; 
  }
  return min(blkSize * esize, size);
}

UINT64 Decl::get_num_blk() {

  UINT64 blkBytes = get_bsize();
  return (size % blkBytes == 0) ? (size / blkBytes) : size / blkBytes + 1; 
}

//DimExp is of the form n1,n2,....
string Decl::get_next_dimlen() {

  string ans;
  int comma = dimExp.find(",", curPos);
  if (comma == -1) {
    return dimExp.substr(curPos, dimExp.size() - curPos);
  } else {
    ans = dimExp.substr(curPos, comma - curPos);
    curPos = comma + 1;
    return ans;
  }
}

void Decl::print() {

  cout << "Declaration: " << endl;
  cout << "\t name: " << name << endl;
  cout << "\t kind: " << kind << endl;
  cout << "\t total size: " << size << endl;
  cout << "\t block size(# elts): " << blkSize << endl;
  cout << "\t elt size: " << esize << endl;
  if (hasInit()) {
    cout << "\t initial value: " << initExp << endl;
  }
}
