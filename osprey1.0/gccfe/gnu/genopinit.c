/* Generate code to initialize optabs from machine description.
   Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998,
   1999, 2000 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */


#include "hconfig.h"
#include "system.h"
#include "rtl.h"
#include "errors.h"
#include "gensupport.h"


/* Many parts of GCC use arrays that are indexed by machine mode and
   contain the insn codes for pattern in the MD file that perform a given
   operation on operands of that mode.

   These patterns are present in the MD file with names that contain
   the mode(s) used and the name of the operation.  This program
   writes a function `init_all_optabs' that initializes the optabs with
   all the insn codes of the relevant patterns present in the MD file.

   This array contains a list of optabs that need to be initialized.  Within
   each string, the name of the pattern to be matched against is delimited
   with $( and $).  In the string, $a and $b are used to match a short mode
   name (the part of the mode name not including `mode' and converted to
   lower-case).  When writing out the initializer, the entire string is
   used.  $A and $B are replaced with the full name of the mode; $a and $b
   are replaced with the short form of the name, as above.

   If $N is present in the pattern, it means the two modes must be consecutive
   widths in the same mode class (e.g, QImode and HImode).  $I means that
   only integer modes should be considered for the next mode, and $F means
   that only float modes should be considered.

   For some optabs, we store the operation by RTL codes.  These are only
   used for comparisons.  In that case, $c and $C are the lower-case and
   upper-case forms of the comparison, respectively.  */

const char * const optabs[] =
{ "extendtab[$B][$A][0] = CODE_FOR_$(extend$a$b2$)",
  "extendtab[$B][$A][1] = CODE_FOR_$(zero_extend$a$b2$)",
  "fixtab[$A][$B][0] = CODE_FOR_$(fix$F$a$I$b2$)",
  "fixtab[$A][$B][1] = CODE_FOR_$(fixuns$F$a$b2$)",
  "fixtrunctab[$A][$B][0] = CODE_FOR_$(fix_trunc$F$a$I$b2$)",
  "fixtrunctab[$A][$B][1] = CODE_FOR_$(fixuns_trunc$F$a$I$b2$)",
  "floattab[$B][$A][0] = CODE_FOR_$(float$I$a$F$b2$)",
  "floattab[$B][$A][1] = CODE_FOR_$(floatuns$I$a$F$b2$)",
  "add_optab->handlers[$A].insn_code = CODE_FOR_$(add$a3$)",
  "sub_optab->handlers[$A].insn_code = CODE_FOR_$(sub$a3$)",
  "smul_optab->handlers[$A].insn_code = CODE_FOR_$(mul$a3$)",
  "umul_highpart_optab->handlers[$A].insn_code = CODE_FOR_$(umul$a3_highpart$)",
  "smul_highpart_optab->handlers[$A].insn_code = CODE_FOR_$(smul$a3_highpart$)",
  "smul_widen_optab->handlers[$B].insn_code = CODE_FOR_$(mul$a$b3$)$N",
  "umul_widen_optab->handlers[$B].insn_code = CODE_FOR_$(umul$a$b3$)$N",
  "sdiv_optab->handlers[$A].insn_code = CODE_FOR_$(div$I$a3$)",
  "udiv_optab->handlers[$A].insn_code = CODE_FOR_$(udiv$I$a3$)",
  "sdivmod_optab->handlers[$A].insn_code = CODE_FOR_$(divmod$a4$)",
  "udivmod_optab->handlers[$A].insn_code = CODE_FOR_$(udivmod$a4$)",
  "smod_optab->handlers[$A].insn_code = CODE_FOR_$(mod$a3$)",
  "umod_optab->handlers[$A].insn_code = CODE_FOR_$(umod$a3$)",
  "flodiv_optab->handlers[$A].insn_code = CODE_FOR_$(div$F$a3$)",
  "ftrunc_optab->handlers[$A].insn_code = CODE_FOR_$(ftrunc$F$a2$)",
  "and_optab->handlers[$A].insn_code = CODE_FOR_$(and$a3$)",
  "ior_optab->handlers[$A].insn_code = CODE_FOR_$(ior$a3$)",
  "xor_optab->handlers[$A].insn_code = CODE_FOR_$(xor$a3$)",
  "ashl_optab->handlers[$A].insn_code = CODE_FOR_$(ashl$a3$)",
  "ashr_optab->handlers[$A].insn_code = CODE_FOR_$(ashr$a3$)",
  "lshr_optab->handlers[$A].insn_code = CODE_FOR_$(lshr$a3$)",
  "rotl_optab->handlers[$A].insn_code = CODE_FOR_$(rotl$a3$)",
  "rotr_optab->handlers[$A].insn_code = CODE_FOR_$(rotr$a3$)",
  "smin_optab->handlers[$A].insn_code = CODE_FOR_$(smin$I$a3$)",
  "smin_optab->handlers[$A].insn_code = CODE_FOR_$(min$F$a3$)",
  "smax_optab->handlers[$A].insn_code = CODE_FOR_$(smax$I$a3$)",
  "smax_optab->handlers[$A].insn_code = CODE_FOR_$(max$F$a3$)",
  "umin_optab->handlers[$A].insn_code = CODE_FOR_$(umin$I$a3$)",
  "umax_optab->handlers[$A].insn_code = CODE_FOR_$(umax$I$a3$)",
  "neg_optab->handlers[$A].insn_code = CODE_FOR_$(neg$a2$)",
  "abs_optab->handlers[$A].insn_code = CODE_FOR_$(abs$a2$)",
  "sqrt_optab->handlers[$A].insn_code = CODE_FOR_$(sqrt$a2$)",
  "sin_optab->handlers[$A].insn_code = CODE_FOR_$(sin$a2$)",
  "cos_optab->handlers[$A].insn_code = CODE_FOR_$(cos$a2$)",
  "strlen_optab->handlers[$A].insn_code = CODE_FOR_$(strlen$a$)",
  "one_cmpl_optab->handlers[$A].insn_code = CODE_FOR_$(one_cmpl$a2$)",
  "ffs_optab->handlers[$A].insn_code = CODE_FOR_$(ffs$a2$)",
  "mov_optab->handlers[$A].insn_code = CODE_FOR_$(mov$a$)",
  "movstrict_optab->handlers[$A].insn_code = CODE_FOR_$(movstrict$a$)",
  "cmp_optab->handlers[$A].insn_code = CODE_FOR_$(cmp$a$)",
  "tst_optab->handlers[$A].insn_code = CODE_FOR_$(tst$a$)",
  "bcc_gen_fctn[$C] = gen_$(b$c$)",
  "setcc_gen_code[$C] = CODE_FOR_$(s$c$)",
  "movcc_gen_code[$A] = CODE_FOR_$(mov$acc$)",
  "cbranch_optab->handlers[$A].insn_code = CODE_FOR_$(cbranch$a4$)",
  "cmov_optab->handlers[$A].insn_code = CODE_FOR_$(cmov$a6$)",
  "cstore_optab->handlers[$A].insn_code = CODE_FOR_$(cstore$a4$)",
  "reload_in_optab[$A] = CODE_FOR_$(reload_in$a$)",
  "reload_out_optab[$A] = CODE_FOR_$(reload_out$a$)",
  "movstr_optab[$A] = CODE_FOR_$(movstr$a$)",
  "clrstr_optab[$A] = CODE_FOR_$(clrstr$a$)" };

static void gen_insn PARAMS ((rtx));

static void
gen_insn (insn)
     rtx insn;
{
  const char *name = XSTR (insn, 0);
  int m1 = 0, m2 = 0, op = 0;
  size_t pindex;
  int i;
  const char *np, *pp, *p, *q;

  /* Don't mention instructions whose names are the null string.
     They are in the machine description just to be recognized.  */
  if (*name == 0)
    return;

  /* See if NAME matches one of the patterns we have for the optabs we know
     about.  */

  for (pindex = 0; pindex < sizeof optabs / sizeof optabs[0]; pindex++)
    {
      int force_float = 0, force_int = 0;
      int force_consec = 0;
      int matches = 1;

      for (pp = optabs[pindex]; pp[0] != '$' || pp[1] != '('; pp++)
	;

      for (pp += 2, np = name; matches && ! (pp[0] == '$' && pp[1] == ')');
	   pp++)
	{
	  if (*pp != '$')
	    {
	      if (*pp != *np++)
		break;
	    }
	  else
	    switch (*++pp)
	      {
	      case 'N':
		force_consec = 1;
		break;
	      case 'I':
		force_int = 1;
		break;
	      case 'F':
		force_float = 1;
		break;
	      case 'c':
		for (op = 0; op < NUM_RTX_CODE; op++)
		  {
		    for (p = GET_RTX_NAME(op), q = np; *p; p++, q++)
		      if (*p != *q)
			break;

		    /* We have to be concerned about matching "gt" and
		       missing "gtu", e.g., so verify we have reached the
		       end of thing we are to match.  */
		    if (*p == 0 && *q == 0 && GET_RTX_CLASS(op) == '<')
		      break;
		  }

		if (op == NUM_RTX_CODE)
		  matches = 0;
		else
		  np += strlen (GET_RTX_NAME(op));
		break;
	      case 'a':
	      case 'b':
		/* This loop will stop at the first prefix match, so
                   look through the modes in reverse order, in case
                   EXTRA_CC_MODES was used and CC is a prefix of the
                   CC modes (as it should be).  */
		for (i = ((int) MAX_MACHINE_MODE) - 1; i >= 0; i--)
		  {
		    for (p = GET_MODE_NAME(i), q = np; *p; p++, q++)
		      if (TOLOWER (*p) != *q)
			break;

		    if (*p == 0
			&& (! force_int || mode_class[i] == MODE_INT)
			&& (! force_float || mode_class[i] == MODE_FLOAT))
		      break;
		  }

		if (i < 0)
		  matches = 0;
		else if (*pp == 'a')
		  m1 = i, np += strlen (GET_MODE_NAME(i));
		else
		  m2 = i, np += strlen (GET_MODE_NAME(i));

		force_int = force_float = 0;
		break;

	      default:
		abort ();
	      }
	}

      if (matches && pp[0] == '$' && pp[1] == ')'
	  && *np == 0
	  && (! force_consec || (int) GET_MODE_WIDER_MODE(m1) == m2))
	break;
    }

  if (pindex == sizeof optabs / sizeof optabs[0])
    return;

  /* We found a match.  If this pattern is only conditionally present,
     write out the "if" and two extra blanks.  */

  if (*XSTR (insn, 2) != 0)
    printf ("  if (HAVE_%s)\n  ", name);

  printf ("  ");

  /* Now write out the initialization, making all required substitutions.  */
  for (pp = optabs[pindex]; *pp; pp++)
    {
      if (*pp != '$')
	putchar (*pp);
      else
	switch (*++pp)
	  {
	  case '(':  case ')':
	  case 'I':  case 'F':  case 'N':
	    break;
	  case 'a':
	    for (np = GET_MODE_NAME(m1); *np; np++)
	      putchar (TOLOWER (*np));
	    break;
	  case 'b':
	    for (np = GET_MODE_NAME(m2); *np; np++)
	      putchar (TOLOWER (*np));
	    break;
	  case 'A':
	    printf ("(int) %smode", GET_MODE_NAME(m1));
	    break;
	  case 'B':
	    printf ("(int) %smode", GET_MODE_NAME(m2));
	    break;
	  case 'c':
	    printf ("%s", GET_RTX_NAME(op));
	    break;
	  case 'C':
	    printf ("(int) ");
	    for (np = GET_RTX_NAME(op); *np; np++)
	      putchar (TOUPPER (*np));
	    break;
	  }
    }

  printf (";\n");
}

extern int main PARAMS ((int, char **));

int
main (argc, argv)
     int argc;
     char **argv;
{
  rtx desc;

  progname = "genopinit";

  if (argc <= 1)
    fatal ("No input file name.");

  if (init_md_reader (argv[1]) != SUCCESS_EXIT_CODE)
    return (FATAL_EXIT_CODE);

  printf ("/* Generated automatically by the program `genopinit'\n\
from the machine description file `md'.  */\n\n");

  printf ("#include \"config.h\"\n");
  printf ("#include \"system.h\"\n");
  printf ("#include \"rtl.h\"\n");
  printf ("#include \"flags.h\"\n");
  printf ("#include \"insn-flags.h\"\n");
  printf ("#include \"insn-codes.h\"\n");
  printf ("#include \"insn-config.h\"\n");
  printf ("#include \"recog.h\"\n");
  printf ("#include \"expr.h\"\n");
  printf ("#include \"reload.h\"\n\n");

  printf ("void\ninit_all_optabs ()\n{\n");

  /* Read the machine description.  */

  while (1)
    {
      int line_no, insn_code_number = 0;

      desc = read_md_rtx (&line_no, &insn_code_number);
      if (desc == NULL)
	break;

      if (GET_CODE (desc) == DEFINE_INSN || GET_CODE (desc) == DEFINE_EXPAND)
	gen_insn (desc);
    }

  printf ("}\n");

  fflush (stdout);
  return (ferror (stdout) != 0 ? FATAL_EXIT_CODE : SUCCESS_EXIT_CODE);
}

/* Define this so we can link with print-rtl.o to get debug_rtx function.  */
const char *
get_insn_name (code)
     int code ATTRIBUTE_UNUSED;
{
  return NULL;
}
