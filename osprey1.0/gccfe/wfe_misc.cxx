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

#include <iostream>
#include <values.h>
#include <sys/types.h>
#include <elf.h>
#include "defs.h"
#include "config.h"
#include "config_debug.h"
#include "config_list.h"
#include "config_targ2.h"
#include "controls.h"
#include "erglob.h"
#include "erlib.h"
#include "file_util.h"
#include "flags.h"
#include "glob.h"
#include "mempool.h"
#include "tracing.h"
#include "util.h"
#include "errors.h"
// #include "cmd_line.h"
#include "err_host.tab"
#include <stdarg.h>
#include "gnu_config.h"
extern "C" {
#include "gnu/system.h"
#include "gnu/tree.h"
}
#include "wn.h"
#include "wn_util.h"
#include "wn_simp.h"
#include "symtab.h"
#include "pu_info.h"
#include "ir_reader.h"
#include "ir_bwrite.h"
#include "wfe_decl.h"
#include "wfe_expr.h"
#include "wfe_dst.h"
#include "wfe_misc.h"
#include "wfe_stmt.h"
#include "c_int_model.h"
#include "tree_symtab.h"

int WFE_Keep_Zero_Length_Structs = FALSE;

extern int optimize;

PU_Info *PU_Tree_Root = NULL;
int      wfe_invoke_inliner = FALSE;

extern void Initialize_IRB (void);	/* In lieu of irbutil.h */
extern char *asm_file_name;		/* from toplev.c */

//extern int real_pointer_size;           /* from toplev.h */
//extern int real_bits_per_word;          /* ditto */
//extern int real_units_per_word;         /* ditto */  
//extern char* real_size_type;
//extern char* real_ptrdiff_type;

int trace_verbose = FALSE;
// an_error_severity error_threshold = es_warning;

static BOOL Prepare_Source (void);
static void WFE_Stmt_Stack_Init (void);
static void WFE_Stmt_Stack_Free (void);

// The following taken from gnu/flags.h
// our #include of flags.h gets common/util/flags.h instead
enum debug_info_level
{
  DINFO_LEVEL_NONE,     /* Write no debugging info.  */
  DINFO_LEVEL_TERSE,    /* Write minimal info to support tracebacks only.  */
  DINFO_LEVEL_NORMAL,   /* Write info for all declarations (and line table). */
  DINFO_LEVEL_VERBOSE   /* Write normal info plus #define/#undef info.  */
};

/* Specify how much debugging info to generate.  */
extern enum debug_info_level debug_info_level;
// End gnu/flags.h data decl

extern void process_shared(const char* filename);
extern void process_nonshared(ST_IDX st, tld_pair_p info);
extern void output_file();

//defined in shared-alloc.cxx
extern string strip(string name, bool id = false);


/* ====================================================================
 *
 * Local data.
 *
 * ====================================================================
 */

/*       MAX_DEBUG_LEVEL	2  :: Defined in flags.h */
# define DEF_DEBUG_LEVEL	0
INT8 Debug_Level = DEF_DEBUG_LEVEL;	/* -gn:	debug level */
# define MAX_MSG_LEVEL 2
# define DEF_MSG_LEVEL 2

#ifdef MONGOOSE_CIF
mUINT32 Cif_Level = 0;       	/* CIF level */
#define MAX_CIF_LEVEL 3 
#define DEF_CIF_LEVEL 2 
#endif /* MONGOOSE_CIF */

/* Default file	extensions: */
#define	IRB_FILE_EXTENSION ".B"	/* ACIR file */
#define	IRD_FILE_EXTENSION ".D"	/* Intermediate data file */
#define	ERR_FILE_EXTENSION ".e"	/* Error file */
#define	LST_FILE_EXTENSION ".l"	/* Listing file */
#define	TRC_FILE_EXTENSION ".t"	/* Trace file */
#define DSTDUMP_FILE_EXTENSION ".fe.dst" /* DST dump-file extension */

/* Static data:	command	line information: */
static INT32 Argc;		/* Copy of argc */
static char **Argv;		/* Copy of argv */
static INT32 Source_Arg;	/* Number of current source arg */
static INT32 Src_Count;		/* Number of source files seen */
static char Dash [] = "-";

/* Internal flags: */
static BOOL Echo_Flag =	FALSE;	/* Echo command	lines */
static BOOL Delete_IR_File = FALSE;	/* Delete SGIR file when done */

static int pshared_size = 16, shared_size = 16;


/* ====================================================================
 *
 * Cleanup_Files
 *
 * Close all per-source	files involved in a compilation	and prepare the
 * global variables for	the next source.  This routine is externalized
 * for signal cleanup; the report parameter allows suppressing of error
 * reporting during such cleanup.
 *
 * ====================================================================
 */

void
Cleanup_Files (	BOOL report, BOOL delete_dotofile )
{
  /* No	current	line number for	errors:	*/
  Set_Error_Line (ERROR_LINE_UNKNOWN);

  /* Close source file:	*/
  if ( Src_File	!= NULL	&& Src_File != stdin &&	fclose (Src_File) ) {
    if ( report	) ErrMsg ( EC_Src_Close, Src_File_Name,	errno );
  }
  Src_File = NULL;

  /* Close and delete SGIR file: */
  if ( IR_File != NULL && fclose (IR_File) ) {
    if ( report	) ErrMsg ( EC_IR_Close,	IR_File_Name, errno );
  }
  IR_File = NULL;
  if ( Delete_IR_File && unlink	(IR_File_Name) ) {
    if ( report	) ErrMsg ( EC_IR_Delete, IR_File_Name, errno );
  }

  /* Close listing file: */
  if ( Lst_File	!= NULL	&& Lst_File != stdout && fclose	(Lst_File) ) {
    if ( report	) ErrMsg ( EC_Lst_Close, Lst_File_Name,	errno );
  }
  Lst_File = NULL;

  /* Close trace file: */
  Set_Trace_File ( NULL	);

  /* Disable timing file: */
  Tim_File = NULL;

  /* Finally close error file: */
  Set_Error_File ( NULL	);
  Set_Error_Source ( NULL );
}

/* ====================================================================
 *
 * Terminate
 *
 * Do any necessary cleanup and	terminate the program with the given
 * status.
 *
 * ====================================================================
 */

void
Terminate ( INT status )
{
  /* Close and delete files as necessary: */
  Cleanup_Files	( FALSE, FALSE);

  exit (status);
}

/* ====================================================================
 *
 * Prepare_Source
 *
 * Process the next source argument and	associated file	control	flags
 * from	the command line.  Pre-process the source file unless
 * suppressed, and initialize output files as required.	 Return	TRUE
 * iff we have a successfully pre-processed source file	left to
 * compile.
 *
 * ====================================================================
 */

static BOOL
Prepare_Source ( void )
{
  INT16	i;
  char *cp;
  char *fname;
  INT16 len;
  BOOL  dashdash_flag = FALSE;

  /* Initialize error handler: */
  Init_Error_Handler ( 100 );
  Set_Error_Line ( ERROR_LINE_UNKNOWN );
  Set_Error_File ( NULL );
  Set_Error_Phase ( "Front End Driver" );

  /* Clear file names: */
  Src_File_Name = NULL;	/* Source file */
  IR_File_Name = NULL;	/* SGIR file */
  Irb_File_Name = NULL;	/* ACIR file */
  Err_File_Name = Dash;	/* Error file */
  Lst_File_Name = NULL;	/* Listing file */
  Trc_File_Name = NULL;	/* Trace file */
  DSTdump_File_Name = NULL; /* DST dump */

  Delete_IR_File = FALSE;
  
  /* Check the command line flags for -f? and source file names: */
  while ( ++Source_Arg <= Argc ) {
    i = Source_Arg;

    /* Null argument => end of list: */
    if ( Argv[i] == NULL ) return FALSE;

    if ( !dashdash_flag && (*(Argv[i]) == '-' )) {
      cp = Argv[i]+1;	/* Pointer to next flag character */

      /* -oname or -o name are passed to the linker: */
      if ( *cp == 'o' ) {
	++cp;
	if ( *cp == 0 ) {
	  /* Link file name is next command line argument: */
	  ++Source_Arg;
	}
	continue;
      }

      /* process as command-line option group */
      if (strncmp(cp, "OPT:", 4) == 0) { 
	Process_Command_Line_Group (cp, Common_Option_Groups);
    	continue;
      }
    } 
    else {
      Src_Count++;
      dashdash_flag = FALSE;

      /* Copy the given source name: */
      len = strlen ( Argv[i] );
      Src_File_Name = (char *) malloc (len+5);
      strcpy ( Src_File_Name, Argv[i] );

      /* We've got a source file name -- open other files.
       * We want them to be created in the current directory, so we
       * strip off the filename only from Src_File_Name for use:
       */
      fname = Last_Pathname_Component ( Src_File_Name );

      /* Error file first to get error reports: */
      if ( Err_File_Name == NULL ) {
	/* Replace source file extension to get error file: */
	Err_File_Name = New_Extension
			    ( fname, ERR_FILE_EXTENSION	);
      } else if ( *Err_File_Name == '-' ) {
	/* Disable separate error file: */
	Err_File_Name = NULL;
      }
      Set_Error_File ( Err_File_Name );

      /* Trace file next: */
      if ( Trc_File_Name == NULL ) {
	if ( Tracing_Enabled ) {
	  /* Replace source file extension to get trace file: */
	  Trc_File_Name = New_Extension
			    ( fname, TRC_FILE_EXTENSION	);
	}
      } else if ( *Trc_File_Name == '-' ) {
	/* Leave trace file on stdout: */
	Trc_File_Name = NULL;
      }
      Set_Trace_File ( Trc_File_Name );
      if ( Get_Trace (TKIND_INFO, TINFO_TIME) ) Tim_File = TFile;

      /* We're ready to pre-process: */
      IR_File_Name = Src_File_Name;

      /* Open the IR file for compilation: */
      if ( Irb_File_Name == NULL ) {
	if (asm_file_name == NULL) {
		/* Replace source file extension to get listing file: */
		Irb_File_Name = New_Extension (	fname, IRB_FILE_EXTENSION );
	}
	else {
		Irb_File_Name = asm_file_name;
	}
      }

	if ( (Irb_File = fopen ( Irb_File_Name, "w" )) == NULL ) {
	  ErrMsg ( EC_IR_Open, IR_File_Name, errno );
	  Cleanup_Files ( TRUE, FALSE );	/* close opened files */
	  return Prepare_Source ();
	} else {
	  if ( Get_Trace ( TP_MISC, 1) ) {
	    fprintf ( TFile, 
	      "\n%sControl Values: Open_Dot_B_File\n%s\n", DBar, DBar );
	    Print_Controls ( TFile, "", TRUE );
	  }
	}

      /* Configure internal options for this source file */
      Configure_Source ( Src_File_Name );

      return TRUE;
    }
  }

  return FALSE;
}

static void Read_Config_File(int argc, char** argv) {

  for (int i = 0; i < argc; i++) {
    char* arg = argv[i];
    if (strncmp(arg, "-fconfig-", 9) == 0) {
      char* Config_File_Name = arg + 9;
      FILE* config_file = fopen(Config_File_Name, "r");
      if (config_file == NULL) {
	fprintf(stderr, "CANNOT OPEN CONFIGURATION FILE: %s\n", Config_File_Name);
	exit(1);
      }
      char line[100];
      int size;
      char param[100];
      while (fgets(line, 100, config_file) != NULL) {
	if (sscanf(line, "%s\t%d", param, &size) != 2) {
	  cerr << "Malformed Line in config file: " << line << endl;
	  continue;
	}
	if (strcmp(param, "shared_ptr") == 0) {
	  shared_size = size;
	} else if (strcmp(param, "pshared_ptr") == 0) {
	  pshared_size = size;
	}
      }
      return;
    }
  }
}

extern int is_ia32; /* from toplev.h */

/**
 *  hack to get the front end to target ia32 
 *  All changes that are necessary to get 32-bit backend to work needs to be placed here
 */
static void Test_ia32(int argc, char** argv) {

  //real_size_type = (char*) malloc(32);
  //real_ptrdiff_type = (char*) malloc(32);
  for (int i = 0; i < argc; i++) {
    char* arg = argv[i];
    if (strcmp(arg, "-ia32") == 0) {
      ABI_Name = "ia32";
      is_ia32 = 1;
      //real_pointer_size = 32;
      /* FIXME: currently the following two are not used, 
	 because the code assumes that BITS_PER_WORD and UNITS_PER_WORD are constants and use them in static arrays
	 may need to change the code if this becomes a problem for 32-bit platforms...
      */
      //real_bits_per_word = 32;
      //real_units_per_word = 4;
      //strcpy(real_size_type, "long unsigned int");
      //strcpy(real_ptrdiff_type, "long int");      
      return;
    }
  }
  //real_pointer_size = 64;
  //strcpy(real_size_type, "long unsigned int");
  //strcpy(real_ptrdiff_type, "long int");      
}

void
WFE_Init (INT argc, char **argv, char **envp )
{
  Set_Error_Tables ( Phases, host_errlist );
  MEM_Initialize();
  Handle_Signals();

  /* Perform preliminary command line processing: */
  Set_Error_Line ( ERROR_LINE_UNKNOWN );
  Set_Error_Phase ( "Front End Driver" );
  Preconfigure ();

#ifdef TARG_MIPS
  ABI_Name = (char *) mips_abi_string;
#endif
#ifdef TARG_IA64
  ABI_Name = "i64";
#endif
#ifdef TARG_IA32
  ABI_Name = "ia32";
#endif
  Init_Controls_Tbl();
  Argc = argc;
  Argv = argv;

  //WEI: code for supporting ia32 backend
  Test_ia32(argc, argv);
  Read_Config_File(argc, argv);
  
  Configure ();
  Initialize_C_Int_Model();
  IR_reader_init();
  Initialize_Symbol_Tables (TRUE);
  if (compiling_upc)
     Create_Special_Shared_Global_Symbols() ;

  WFE_Stmt_Stack_Init ();
  WFE_Stmt_Init ();
  WFE_Expr_Init ();
  WHIRL_Mldid_Mstid_On = TRUE;
  WN_Simp_Fold_LDA = TRUE;  // fold (LDA offset) + const to LDA (offset+const)
			    // since the static initialization code relies on it
  WHIRL_Keep_Cvt_On = TRUE; // so simplifier won't I8I4CVT
  Opt_Level = optimize;

  // This is not right: we should match what gnu does
  // and this is only an approximation.
  Debug_Level = (debug_info_level >= DINFO_LEVEL_NORMAL)? 2:0;
} /* WFE_Init */




void
WFE_File_Init (INT argc, char **argv)
{
  /* Process each source file: */
  Prepare_Source();
  MEM_POOL_Push (&MEM_src_pool);
  Restore_Cmd_Line_Ctrls();

  /* If the user forgot to specify sources, complain: */
  if ( Src_Count == 0 ) {
    ErrMsg ( EC_No_Sources );
  }

  Open_Output_Info ( Irb_File_Name );
  DST_build(argc, argv);	// do initial setup of dst
}



// function object for writing out various symbol tables
template <class SYMBOL_TABLE>
struct WRITE_UPC_INIT
{
  Output_File *fl;
  
 const  void operator () (UINT x, ST *&t) const {
   t->Print(stdout);
  }
  
  WRITE_UPC_INIT (Output_File *_fl) : fl (_fl) {}
}; 

char* get_type(int mtype) {
  
  switch (mtype) {
  case MTYPE_I1:
    return "char";
  case MTYPE_I2:
    return "short";
  case MTYPE_I4:
    return "int";
  case MTYPE_U1:
    return "unsigned char";
  case MTYPE_U2:
    return "unsigned short";
  case MTYPE_U4:
    return "unsigned int";
  case MTYPE_I8:
    return "long";
  case MTYPE_U8:
    return "unsigned long";
  case MTYPE_F4:
    return "float";
  case MTYPE_F8:
    return "double";
  case MTYPE_FQ:
    return "long double";
  default:
    cerr << "COULDN'T FIND CORRECT TYPE IN get_type(): " << mtype << endl;
    return "NONE";
  }
}

static void output_Init_Val(char* s, INITV_IDX idx, TY_IDX ty = 0) {

  TCON tc; //used for INITVKIND_VAL
  ST_IDX rhs_st; //used for INITVKIND_SYMOFF
  int mtype;

  switch (INITV_kind(idx)) {
  case INITVKIND_ZERO:
    sprintf(s, "0");
    break;
  case INITVKIND_ONE:
    sprintf(s, "1");
    break;
  case INITVKIND_VAL:
    tc = INITV_tc_val(idx);
    mtype = TCON_ty(tc);
    switch (mtype) {
    case MTYPE_I1:
    case MTYPE_I2:
    case MTYPE_I4:
      sprintf(s, "%d", TCON_ival(tc));
      break;
    case MTYPE_U1:
    case MTYPE_U2:
    case MTYPE_U4:
      sprintf(s, "%u", TCON_uval(tc));
      break;
    case MTYPE_I8:
      sprintf(s, "%ld", (long) TCON_i0(tc));
      break;
    case MTYPE_U8:
      sprintf(s, "%lu", (unsigned long) TCON_k0(tc));
      break;
    case MTYPE_F4:
      sprintf(s, "%f", TCON_fval(tc));
      break;
    case MTYPE_F8:
      sprintf(s, "%f", TCON_dval(tc));
      break;
    case MTYPE_FQ:
      sprintf(s, "%Lf", TCON_qval(tc));
      break;
    default:
      //don't think other types matter
      cerr << "Unexpected type for init value: " << TCON_ty(tc) << endl;
      sprintf(s, "0\n");
     } //inner switch
    break;
  case INITVKIND_SYMOFF:
    rhs_st = INITV_st(idx);
    if (ty != 0 && Type_Is_Shared_Ptr(ty)) {
      TY_IDX rhs_ty = ST_type(rhs_st);
      if (!TY_is_shared(rhs_ty)) {
	//should be caught elsewhere, just in case...
	cerr << "ASSIGNING a non-shared value to a pointer to shared data: " << ST_name(rhs_st) << endl;
      }
      bool lhs_pshared = TY_is_shared(ty) ? TY_is_pshared(ty) : TY_is_pshared(TY_pointed(ty));
      if (lhs_pshared && !TY_is_pshared(rhs_ty)) {
	sprintf(s, "upcr_shared_to_pshared(%s)", ST_name(rhs_st));
      } else if (!lhs_pshared && TY_is_pshared(rhs_ty)) {
	sprintf(s, "upcr_pshared_to_shared(%s)", ST_name(rhs_st));
      } else {
	sprintf(s, "%s", ST_name(rhs_st));
      }
    } else {
      //initialization for regular pointers, need a "&"
      sprintf(s, "&%s", ST_name(rhs_st));
    }
    break;
  default:
    sprintf(s, "NONE");
  }
}

static unsigned int get_dim(TY_IDX idx) {

  switch (TY_kind(idx)) {
  case KIND_STRUCT:
    return 1;
  case KIND_ARRAY: {
    int dim = 0;
    for (;TY_kind(idx) == KIND_ARRAY; idx = TY_etype(idx), dim++);
    return dim;
  }
  default:
    return 0;
  }
}

/**
 *
 * Get the correct init exp for the given INITV_IDX idx
 * dim is the number of dimensions for arrays
 */
static string get_init_exp(ST_IDX st) {

  string init_exp = "";
  char buf[100];
  INITV_IDX idx = INITV_index(ST_IDX_level(st), st);
  TY_IDX ty_idx = ST_type(st);
  int dim = get_dim(ty_idx);
  int kind = TY_kind(ty_idx);
  if (idx == INITV_Table_Size() + 1) {
    return "NONE";
  }  

  if (kind == KIND_ARRAY) {
    char dimlen[dim];
    for (int i = 0; i < dim; i++) {
      init_exp += "{";
      dimlen[i] = 0;
    }
    int len = 0;
    for (int next_idx = idx, prev = -1; next_idx != 0; next_idx = INITV_next(next_idx)) {
      if (INITV_kind(next_idx) == INITVKIND_PAD) {
	init_exp += "}";
	if (INITV_next(next_idx) != 0) {
	  init_exp += ",";
	}
      } else {		
	if (prev != -1 && INITV_kind(prev) == INITVKIND_PAD) {
	  //we're at a new dimension, print {
	  init_exp += "{";
	}
	//cout << " val ";
	output_Init_Val(buf, next_idx);
	init_exp += buf;
	init_exp += ",";
      }
      prev = next_idx;
    }
    //cout << endl;
  } else if (kind == KIND_STRUCT) {
    init_exp += "{";
    for (int next_idx = idx; next_idx != 0; next_idx = INITV_next(next_idx)) {
      if (INITV_kind(next_idx) != INITVKIND_PAD) {
	output_Init_Val(buf, next_idx);
	init_exp += buf;
	init_exp += ",";
      }
    }
    init_exp += "}";
  } else {
    output_Init_Val(buf, idx, ty_idx);
    init_exp += buf;
  }
  return init_exp;
}


//Print the type string for the given ty entry
//For arrays, the base(non-array) type is printed

static void print_type(TY_IDX ty, FILE* out) {

  switch (TY_kind(ty)) {
  case KIND_SCALAR:
    fprintf(out, "\t%s\t", get_type(TY_mtype(ty)));
    break;
  case KIND_ARRAY: {
    print_type(TY_etype(ty), out);
    break;
  } 
  case KIND_STRUCT: 
    //use "," since scanf doesn't like space (change back later to shared-alloc) 
    fprintf(out, "\t%s,", TY_is_union(ty) ? "union" : "struct");   
    fprintf(out, "%s\t", TY_name(ty));
    break;
  case KIND_POINTER:
    if (TY_kind(TY_pointed(ty)) == KIND_VOID) {
      //treat "void *" as a special case
      fprintf(out, "\tVOID*\t");
    } else {
      fprintf(out, "\tNONE\t"); //we don't care about the type of a pointer
    }
    break;
  default:
    Is_True(false, ("Invalid kind of types in WFE_File_Finish"));
  }
}

static string file_name = ""; 
static FILE* out_file = NULL;

extern int compiling_upc;

static UINT64 get_real_size(TY_IDX t_idx) {

  switch (TY_kind(t_idx)) {
  case KIND_SCALAR:
    return TY_size(t_idx);
  case KIND_POINTER: 
    if (TY_is_shared(t_idx)) {
      return TY_is_pshared(t_idx) ? pshared_size : shared_size;
    } else if (TY_is_shared(TY_pointed(t_idx))) {
      return TY_is_pshared(TY_pointed(t_idx)) ? pshared_size : shared_size;
    }
    return TY_size(t_idx);
  case KIND_ARRAY: {
    UINT64 orig_size = Get_Type_Inner_Size(t_idx);
    return TY_size(t_idx) * (get_real_size(Get_Inner_Array_Type(t_idx)) / orig_size);
  }
  case KIND_STRUCT: {
    //FIXME:  not done with fixing the size of structs yet
    /*
      int size = 0;
      FLD_HANDLE fh = TY_fld(t_idx);
      while (true) {
      TY_IDX fld_ty = FLD_type(fh);
      size += get_real_size(fld_ty);
      if (FLD_last_field(fh)) {
      break;
      }
      fh = FLD_next(fh);
      } */
    return TY_size(t_idx);
  }
  default:
    Is_True(false, ("Invalid kind of types in WFE_File_Finish"));
  }  
}

void add_shared_symbol(ST_IDX st, TY_IDX t_idx, int thread_dim) {

  if (file_name == "") {
    file_name = strip(Orig_Src_File_Name) + "_symbols";
    out_file = fopen(file_name.c_str(), "w");
    Is_True(out_file != NULL, ("", "Cannot open a temporary shared symbol file for writing"));
  }
  if (compiling_upc) {
    if(TY_is_shared(ST_type(st))){
      char *tmp = ST_name(st);
      int kind = TY_kind(t_idx);
      int bsize = Get_Type_Block_Size(t_idx); 
      UINT64 size  = get_real_size(t_idx);
      UINT64 eltSize = (TY_kind(t_idx) == KIND_ARRAY) ? get_real_size(Get_Inner_Array_Type(t_idx)) : size;
      fprintf(out_file,"%s\t%ul\t%d\t%ul\t", tmp, (unsigned long) size, bsize, (unsigned long) eltSize);
      if(kind == KIND_SCALAR) 
	fprintf(out_file,"S\t");
      else if(kind == KIND_POINTER)
	fprintf(out_file,"P\t");
      if(kind == KIND_ARRAY)
	fprintf(out_file,"A\t");
      if(kind == KIND_STRUCT)
	fprintf(out_file,"M\t");
      
      //now try to get init value
      string init_exp = get_init_exp(st);
      fprintf(out_file, init_exp.c_str());
      
      //print the C type string for the ty entry
      print_type(t_idx, out_file);
      
      //Finally, for array we also need dimension information
      if (kind == KIND_ARRAY) {
	int dim = 1;
	for (TY_IDX ty = t_idx; TY_kind(ty) == KIND_ARRAY; ty = TY_etype(ty), dim++) {
	  if (thread_dim == dim) {
	    fprintf(out_file, "T"); //Indicating the dimension refers to THREADS
	  }
	  fprintf(out_file, "%d,", (int) (TY_size(ty) / TY_size(TY_etype(ty))));
	}
	fprintf(out_file, "\n");
      } else {
	fprintf(out_file, "NONE\n");
      }
    }
  }
}

void add_TLD_symbol(ST_IDX st, tld_pair_p info) {

  info->init_exp = get_init_exp(st); 
  process_nonshared(st, info);
}

/* from wfe_decl.cxx */
extern void add_symbols();

void
WFE_File_Finish (void)
{
  Verify_SYMTAB (GLOBAL_SYMTAB);

  /* in the unlikely case that the file contains no functions... */
  add_symbols();

  if (out_file > 0) { 
    fclose(out_file);
    process_shared(file_name.c_str());
  } else {
    process_shared("");
  }

  Write_Global_Info (PU_Tree_Root);
  Close_Output_Info ();
  IR_reader_finish ();
  MEM_POOL_Pop (&MEM_src_pool);
  
  output_file();

}

void
WFE_Finish ()
{
  WFE_Stmt_Stack_Free ();
}

void
WFE_Check_Errors (int *error_count, int *warning_count, BOOL *need_inliner)
{
  
  /* If we've seen errors, note them and terminate: */
  Get_Error_Count ( error_count, warning_count);
  *need_inliner = wfe_invoke_inliner;
}

#define ENLARGE(x) (x + (x >> 1))
#define WN_STMT_STACK_SIZE 32

typedef struct wn_stmt {
  WN            *wn;
  WFE_STMT_KIND  kind;
} WN_STMT;

static WN_STMT *wn_stmt_stack;
static WN_STMT *wn_stmt_sp;
static WN_STMT *wn_stmt_stack_last;
static INT      wn_stmt_stack_size;

char * WFE_Stmt_Kind_Name [wfe_stmk_last+1] = {
  "'unknown'",
  "'function entry'",
  "'function pragma'",
  "'function body'",
  "'region pragmas'",
  "'scope'",
  "'if condition'",
  "'if then clause'",
  "'if else clause'",
  "'while condition'",
  "'while body'",
  "'dowhile condition'",
  "'dowhile body'",
  "'for condition'",
  "'for body'",
  "'switch'",
  "'comma'",
  "'rcomma'",
  "'last'"
};

static void
WFE_Stmt_Stack_Init (void)
{
  wn_stmt_stack_size = WN_STMT_STACK_SIZE;
  wn_stmt_stack      = (WN_STMT *) malloc (sizeof (WN_STMT) *
                                           wn_stmt_stack_size );
  wn_stmt_sp         = wn_stmt_stack - 1;
  wn_stmt_stack_last = wn_stmt_stack + wn_stmt_stack_size - 1;
} /* WFE_Stmt_Stack_Init */

static void
WFE_Stmt_Stack_Free (void)
{
  free (wn_stmt_stack);
  wn_stmt_stack = NULL;
} /* WFE_Stmt_stack_free */

void
WFE_Stmt_Push (WN* wn, WFE_STMT_KIND kind, SRCPOS srcpos)
{
  INT new_stack_size;

  if (wn_stmt_sp == wn_stmt_stack_last) {
    new_stack_size = ENLARGE(wn_stmt_stack_size);
    wn_stmt_stack =
      (WN_STMT *) realloc (wn_stmt_stack, new_stack_size * sizeof (WN_STMT));
    wn_stmt_sp = wn_stmt_stack + wn_stmt_stack_size - 1;
    wn_stmt_stack_size = new_stack_size;
    wn_stmt_stack_last = wn_stmt_stack + wn_stmt_stack_size - 1;
  }
  ++wn_stmt_sp;
  wn_stmt_sp->wn   = wn;
  wn_stmt_sp->kind = kind;

  if (srcpos)
    WN_Set_Linenum ( wn, srcpos );
} /* WFE_Stmt_Push */

WN*
WFE_Stmt_Top (void)
{
  FmtAssert (wn_stmt_sp >= wn_stmt_stack,
             ("no more entries on stack in function WFE_Stmt_Top"));

  return (wn_stmt_sp->wn);
} /* WFE_Stmt_Top */

void
WFE_Stmt_Append (WN* wn, SRCPOS srcpos)
{
  WN * body;
  WN * last;

  if (srcpos) {
    WN_Set_Linenum ( wn, srcpos );
    if (WN_operator(wn) == OPR_BLOCK && WN_first(wn) != NULL)
    	WN_Set_Linenum ( WN_first(wn), srcpos );
  }

  body = WFE_Stmt_Top ();

  if (body) {

    last = WN_last(body);
    WN_INSERT_BlockAfter (body, last, wn);
  }
} /* WFE_Stmt_Append */


WN*
WFE_Stmt_Last (void)
{
  WN * body;

  body = WFE_Stmt_Top ();
  return (WN_last(body));
} /* WFE_Stmt_Last */


WN *
WFE_Stmt_Delete ()
{
  WN * body;
  WN * last;
  WN * prev;

  body = WFE_Stmt_Top ();
  last = WN_last(body);
  prev = WN_prev(last);
  if (prev)
    WN_next(prev)  = NULL;
  else
    WN_first(body) = NULL;
  WN_last(body) = prev;
  WN_prev(last) = NULL;

  return last;
} /* WFE_Stmt_Delete */


WN*
WFE_Stmt_Pop (WFE_STMT_KIND kind)
{
  WN * wn;

  FmtAssert (wn_stmt_sp >= wn_stmt_stack,
             ("no more entries on stack in function WFE_Stmt_Pop"));

  FmtAssert (wn_stmt_sp->kind == kind,
             ("mismatch in statements: expected %s, got %s\n",
              WFE_Stmt_Kind_Name [kind],
              WFE_Stmt_Kind_Name [wn_stmt_sp->kind]));

  wn = wn_stmt_sp->wn;
  wn_stmt_sp--;

  return (wn);
} /* WFE_Stmt_Pop */

/*
void process_diag_override_option(an_option_kind kind,
                                  char          *opt_arg)
{
}
*/
