#include <elf.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <search.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "wn.h"
#include "stab.h"
#include "irbdata.h"
#include "wintrinsic.h"
#include "glob.h"
#include "pu_info.h"
#include "ir_bread.h"
#include "ir_bwrite.h"
#include "file_util.h"

/*
INT8 Debug_Level = 0;
void Signal_Cleanup (INT sig) { }
char * Host_Format_Parm (INT kind, MEM_PTR parm) { return NULL; }
*/

extern "C" void DoSomething() {
  Initialize_Symbol_Tables(TRUE);
  ST *st = New_ST(1);
}
