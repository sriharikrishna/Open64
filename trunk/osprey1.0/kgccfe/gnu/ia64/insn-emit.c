/* Generated automatically by the program `genemit'
from the machine description file `md'.  */

#include "config.h"
#include "system.h"
#include "rtl.h"
#include "tm_p.h"
#include "function.h"
#include "expr.h"
#include "optabs.h"
#include "real.h"
#include "flags.h"
#include "output.h"
#include "insn-config.h"
#include "hard-reg-set.h"
#include "recog.h"
#include "resource.h"
#include "reload.h"
#include "toplev.h"
#include "ggc.h"

#define FAIL return (end_sequence (), _val)
#define DONE return (_val = get_insns (), end_sequence (), _val)

rtx
gen_movbi (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	operand1);
}

rtx
gen_movsi_symbolic (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (3,
		gen_rtx_SET (VOIDmode,
	operand0,
	operand1),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (DImode)),
		gen_rtx_USE (VOIDmode,
	gen_rtx_REG (DImode,
	1))));
}

rtx
gen_movdi_symbolic (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (3,
		gen_rtx_SET (VOIDmode,
	operand0,
	operand1),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (DImode)),
		gen_rtx_USE (VOIDmode,
	gen_rtx_REG (DImode,
	1))));
}

rtx
gen_load_gprel (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_PLUS (DImode,
	gen_rtx_REG (DImode,
	1),
	operand1));
}

rtx
gen_gprel64_offset (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_MINUS (DImode,
	operand1,
	gen_rtx_REG (DImode,
	1)));
}

rtx
gen_load_ltoff_dtpmod (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_PLUS (DImode,
	gen_rtx_REG (DImode,
	1),
	gen_rtx_UNSPEC (DImode,
	gen_rtvec (1,
		operand1),
	0)));
}

rtx
gen_load_ltoff_dtprel (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_PLUS (DImode,
	gen_rtx_REG (DImode,
	1),
	gen_rtx_UNSPEC (DImode,
	gen_rtvec (1,
		operand1),
	1)));
}

rtx
gen_load_ltoff_tprel (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_PLUS (DImode,
	gen_rtx_REG (DImode,
	1),
	gen_rtx_UNSPEC (DImode,
	gen_rtvec (1,
		operand1),
	3)));
}

rtx
gen_extendqidi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_SIGN_EXTEND (DImode,
	operand1));
}

rtx
gen_extendhidi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_SIGN_EXTEND (DImode,
	operand1));
}

rtx
gen_extendsidi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_SIGN_EXTEND (DImode,
	operand1));
}

rtx
gen_zero_extendqidi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_ZERO_EXTEND (DImode,
	operand1));
}

rtx
gen_zero_extendhidi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_ZERO_EXTEND (DImode,
	operand1));
}

rtx
gen_zero_extendsidi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_ZERO_EXTEND (DImode,
	operand1));
}

rtx
gen_extendsfdf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_FLOAT_EXTEND (DFmode,
	operand1));
}

rtx
gen_extendsftf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_FLOAT_EXTEND (TFmode,
	operand1));
}

rtx
gen_extenddftf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_FLOAT_EXTEND (TFmode,
	operand1));
}

rtx
gen_truncdfsf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_FLOAT_TRUNCATE (SFmode,
	operand1));
}

rtx
gen_trunctfsf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_FLOAT_TRUNCATE (SFmode,
	operand1));
}

rtx
gen_trunctfdf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_FLOAT_TRUNCATE (DFmode,
	operand1));
}

rtx
gen_floatditf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_FLOAT (TFmode,
	operand1));
}

rtx
gen_fix_truncsfdi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_FIX (DImode,
	operand1));
}

rtx
gen_fix_truncdfdi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_FIX (DImode,
	operand1));
}

rtx
gen_fix_trunctfdi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_FIX (DImode,
	operand1));
}

rtx
gen_fix_trunctfdi2_alts (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_FIX (DImode,
	operand1)),
		gen_rtx_USE (VOIDmode,
	operand2)));
}

rtx
gen_floatunsdisf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSIGNED_FLOAT (SFmode,
	operand1));
}

rtx
gen_floatunsdidf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSIGNED_FLOAT (DFmode,
	operand1));
}

rtx
gen_floatunsditf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSIGNED_FLOAT (TFmode,
	operand1));
}

rtx
gen_fixuns_truncsfdi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSIGNED_FIX (DImode,
	operand1));
}

rtx
gen_fixuns_truncdfdi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSIGNED_FIX (DImode,
	operand1));
}

rtx
gen_fixuns_trunctfdi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSIGNED_FIX (DImode,
	operand1));
}

rtx
gen_fixuns_trunctfdi2_alts (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSIGNED_FIX (DImode,
	operand1)),
		gen_rtx_USE (VOIDmode,
	operand2)));
}

rtx
gen_extv (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_SIGN_EXTRACT (DImode,
	operand1,
	operand2,
	operand3));
}

rtx
gen_extzv (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_ZERO_EXTRACT (DImode,
	operand1,
	operand2,
	operand3));
}

rtx
gen_shift_mix4left (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	gen_rtx_ZERO_EXTRACT (DImode,
	operand0,
	GEN_INT (32L),
	const0_rtx),
	operand1),
		gen_rtx_CLOBBER (VOIDmode,
	operand2)));
}

rtx
gen_mix4right (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	gen_rtx_ZERO_EXTRACT (DImode,
	operand0,
	GEN_INT (32L),
	GEN_INT (32L)),
	operand1);
}

rtx
gen_andbi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_AND (BImode,
	operand1,
	operand2));
}

rtx
gen_iorbi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_IOR (BImode,
	operand1,
	operand2));
}

rtx
gen_one_cmplbi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_NOT (BImode,
	operand1)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (BImode))));
}

rtx
gen_mulhi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_MULT (HImode,
	operand1,
	operand2));
}

rtx
gen_addsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_PLUS (SImode,
	operand1,
	operand2));
}

rtx
gen_subsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_MINUS (SImode,
	operand1,
	operand2));
}

rtx
gen_mulsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_MULT (SImode,
	operand1,
	operand2));
}

rtx
gen_maddsi4 (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_PLUS (SImode,
	gen_rtx_MULT (SImode,
	operand1,
	operand2),
	operand3));
}

rtx
gen_negsi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_NEG (SImode,
	operand1));
}

rtx
gen_divsi3_internal (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (5,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_FLOAT (TFmode,
	gen_rtx_DIV (SImode,
	operand1,
	operand2))),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (BImode)),
		gen_rtx_USE (VOIDmode,
	operand3)));
}

rtx
gen_adddi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_PLUS (DImode,
	operand1,
	operand2));
}

rtx
gen_subdi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_MINUS (DImode,
	operand1,
	operand2));
}

rtx
gen_muldi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_MULT (DImode,
	operand1,
	operand2));
}

rtx
gen_madddi4 (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_PLUS (DImode,
	gen_rtx_MULT (DImode,
	operand1,
	operand2),
	operand3)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (DImode))));
}

rtx
gen_smuldi3_highpart (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_TRUNCATE (DImode,
	gen_rtx_LSHIFTRT (TImode,
	gen_rtx_MULT (TImode,
	gen_rtx_SIGN_EXTEND (TImode,
	operand1),
	gen_rtx_SIGN_EXTEND (TImode,
	operand2)),
	GEN_INT (64L))));
}

rtx
gen_umuldi3_highpart (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_TRUNCATE (DImode,
	gen_rtx_LSHIFTRT (TImode,
	gen_rtx_MULT (TImode,
	gen_rtx_ZERO_EXTEND (TImode,
	operand1),
	gen_rtx_ZERO_EXTEND (TImode,
	operand2)),
	GEN_INT (64L))));
}

rtx
gen_negdi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_NEG (DImode,
	operand1));
}

rtx
gen_divdi3_internal_lat (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (5,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_FLOAT (TFmode,
	gen_rtx_DIV (SImode,
	operand1,
	operand2))),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (BImode))));
}

rtx
gen_divdi3_internal_thr (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (4,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_FLOAT (TFmode,
	gen_rtx_DIV (SImode,
	operand1,
	operand2))),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (BImode))));
}

rtx
gen_addsf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_PLUS (SFmode,
	operand1,
	operand2));
}

rtx
gen_subsf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_MINUS (SFmode,
	operand1,
	operand2));
}

rtx
gen_mulsf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_MULT (SFmode,
	operand1,
	operand2));
}

rtx
gen_abssf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_ABS (SFmode,
	operand1));
}

rtx
gen_negsf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_NEG (SFmode,
	operand1));
}

rtx
gen_minsf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_SMIN (SFmode,
	operand1,
	operand2));
}

rtx
gen_maxsf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_SMAX (SFmode,
	operand1,
	operand2));
}

rtx
gen_divsf3_internal_lat (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (4,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_DIV (SFmode,
	operand1,
	operand2)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (BImode))));
}

rtx
gen_divsf3_internal_thr (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (4,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_DIV (SFmode,
	operand1,
	operand2)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (BImode))));
}

rtx
gen_adddf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_PLUS (DFmode,
	operand1,
	operand2));
}

rtx
gen_subdf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_MINUS (DFmode,
	operand1,
	operand2));
}

rtx
gen_muldf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_MULT (DFmode,
	operand1,
	operand2));
}

rtx
gen_absdf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_ABS (DFmode,
	operand1));
}

rtx
gen_negdf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_NEG (DFmode,
	operand1));
}

rtx
gen_mindf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_SMIN (DFmode,
	operand1,
	operand2));
}

rtx
gen_maxdf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_SMAX (DFmode,
	operand1,
	operand2));
}

rtx
gen_divdf3_internal_lat (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (5,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_DIV (DFmode,
	operand1,
	operand2)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (BImode))));
}

rtx
gen_divdf3_internal_thr (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (4,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_DIV (DFmode,
	operand1,
	operand2)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (DFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (BImode))));
}

rtx
gen_addtf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_PLUS (TFmode,
	operand1,
	operand2));
}

rtx
gen_subtf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_MINUS (TFmode,
	operand1,
	operand2));
}

rtx
gen_multf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_MULT (TFmode,
	operand1,
	operand2));
}

rtx
gen_abstf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_ABS (TFmode,
	operand1));
}

rtx
gen_negtf2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_NEG (TFmode,
	operand1));
}

rtx
gen_mintf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_SMIN (TFmode,
	operand1,
	operand2));
}

rtx
gen_maxtf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_SMAX (TFmode,
	operand1,
	operand2));
}

rtx
gen_divtf3_internal_lat (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (6,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_DIV (TFmode,
	operand1,
	operand2)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (BImode))));
}

rtx
gen_divtf3_internal_thr (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (4,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_DIV (TFmode,
	operand1,
	operand2)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (BImode))));
}

rtx
gen_ashldi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_ASHIFT (DImode,
	operand1,
	operand2));
}

rtx
gen_ashrdi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_ASHIFTRT (DImode,
	operand1,
	operand2));
}

rtx
gen_lshrdi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_LSHIFTRT (DImode,
	operand1,
	operand2));
}

rtx
gen_one_cmplsi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_NOT (SImode,
	operand1));
}

rtx
gen_anddi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_AND (DImode,
	operand1,
	operand2));
}

rtx
gen_iordi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_IOR (DImode,
	operand1,
	operand2));
}

rtx
gen_xordi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_XOR (DImode,
	operand1,
	operand2));
}

rtx
gen_one_cmpldi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_NOT (DImode,
	operand1));
}

rtx
gen_doloop_end_internal (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	pc_rtx,
	gen_rtx_IF_THEN_ELSE (VOIDmode,
	gen_rtx_NE (VOIDmode,
	operand0,
	const0_rtx),
	gen_rtx_LABEL_REF (VOIDmode,
	operand1),
	pc_rtx)),
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_IF_THEN_ELSE (DImode,
	gen_rtx_NE (VOIDmode,
	operand0,
	const0_rtx),
	gen_rtx_PLUS (DImode,
	operand0,
	constm1_rtx),
	operand0))));
}

rtx
gen_call_nogp (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_CALL (VOIDmode,
	gen_rtx_MEM (DImode,
	operand0),
	const0_rtx),
		gen_rtx_CLOBBER (VOIDmode,
	operand1)));
}

rtx
gen_call_value_nogp (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_CALL (VOIDmode,
	gen_rtx_MEM (DImode,
	operand1),
	const0_rtx)),
		gen_rtx_CLOBBER (VOIDmode,
	operand2)));
}

rtx
gen_sibcall_nogp (operand0)
     rtx operand0;
{
  return gen_rtx_CALL (VOIDmode,
	gen_rtx_MEM (DImode,
	operand0),
	const0_rtx);
}

rtx
gen_call_gp (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (4,
		gen_rtx_CALL (VOIDmode,
	gen_rtx_MEM (VOIDmode,
	operand0),
	const1_rtx),
		gen_rtx_CLOBBER (VOIDmode,
	operand1),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (DImode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (DImode))));
}

rtx
gen_call_value_gp (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (4,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_CALL (VOIDmode,
	gen_rtx_MEM (DImode,
	operand1),
	const1_rtx)),
		gen_rtx_CLOBBER (VOIDmode,
	operand2),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (DImode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (DImode))));
}

rtx
gen_sibcall_gp (operand0)
     rtx operand0;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (3,
		gen_rtx_CALL (VOIDmode,
	gen_rtx_MEM (DImode,
	operand0),
	const1_rtx),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (DImode)),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (DImode))));
}

rtx
gen_return_internal (operand0)
     rtx operand0;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_RETURN (VOIDmode),
		gen_rtx_USE (VOIDmode,
	operand0)));
}

rtx
gen_return ()
{
  return gen_rtx_RETURN (VOIDmode);
}

rtx
gen_jump (operand0)
     rtx operand0;
{
  return gen_rtx_SET (VOIDmode,
	pc_rtx,
	gen_rtx_LABEL_REF (VOIDmode,
	operand0));
}

rtx
gen_indirect_jump (operand0)
     rtx operand0;
{
  return gen_rtx_SET (VOIDmode,
	pc_rtx,
	operand0);
}

rtx
gen_prologue_allocate_stack (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_PLUS (DImode,
	operand1,
	operand2)),
		gen_rtx_SET (VOIDmode,
	operand3,
	operand3)));
}

rtx
gen_epilogue_deallocate_stack (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	operand1),
		gen_rtx_SET (VOIDmode,
	operand1,
	operand1)));
}

rtx
gen_prologue_use (operand0)
     rtx operand0;
{
  return gen_rtx_UNSPEC (DImode,
	gen_rtvec (1,
		operand0),
	25);
}

rtx
gen_alloc (operand0, operand1, operand2, operand3, operand4)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
     rtx operand4;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (5,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSPEC_VOLATILE (DImode,
	gen_rtvec (1,
		const0_rtx),
	0)),
		gen_rtx_USE (VOIDmode,
	operand1),
		gen_rtx_USE (VOIDmode,
	operand2),
		gen_rtx_USE (VOIDmode,
	operand3),
		gen_rtx_USE (VOIDmode,
	operand4)));
}

rtx
gen_gr_spill_internal (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSPEC (DImode,
	gen_rtvec (2,
		operand1,
		operand2),
	10)),
		gen_rtx_CLOBBER (VOIDmode,
	operand3)));
}

rtx
gen_gr_restore_internal (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSPEC (DImode,
	gen_rtvec (2,
		operand1,
		operand2),
	11)),
		gen_rtx_USE (VOIDmode,
	operand3)));
}

rtx
gen_fr_spill (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSPEC (TFmode,
	gen_rtvec (1,
		operand1),
	12));
}

rtx
gen_fr_restore (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSPEC (TFmode,
	gen_rtvec (1,
		operand1),
	13));
}

rtx
gen_bsp_value (operand0)
     rtx operand0;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSPEC (DImode,
	gen_rtvec (1,
		const0_rtx),
	21));
}

rtx
gen_set_bsp (operand0)
     rtx operand0;
{
  return gen_rtx_UNSPEC_VOLATILE (VOIDmode,
	gen_rtvec (1,
		operand0),
	4);
}

rtx
gen_flushrs ()
{
  return gen_rtx_UNSPEC (VOIDmode,
	gen_rtvec (1,
		const0_rtx),
	22);
}

rtx
gen_nop ()
{
  return const0_rtx;
}

rtx
gen_nop_m ()
{
  return const1_rtx;
}

rtx
gen_nop_i ()
{
  return GEN_INT (2L);
}

rtx
gen_nop_f ()
{
  return GEN_INT (3L);
}

rtx
gen_nop_b ()
{
  return GEN_INT (4L);
}

rtx
gen_nop_x ()
{
  return GEN_INT (5L);
}

rtx
gen_bundle_selector (operand0)
     rtx operand0;
{
  return gen_rtx_UNSPEC (VOIDmode,
	gen_rtvec (1,
		operand0),
	23);
}

rtx
gen_blockage ()
{
  return gen_rtx_UNSPEC_VOLATILE (VOIDmode,
	gen_rtvec (1,
		const0_rtx),
	1);
}

rtx
gen_insn_group_barrier (operand0)
     rtx operand0;
{
  return gen_rtx_UNSPEC_VOLATILE (VOIDmode,
	gen_rtvec (1,
		operand0),
	2);
}

rtx
gen_break_f ()
{
  return gen_rtx_UNSPEC_VOLATILE (VOIDmode,
	gen_rtvec (1,
		const0_rtx),
	3);
}

rtx
gen_prefetch (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_PREFETCH (VOIDmode,
	operand0,
	operand1,
	operand2);
}

rtx
gen_builtin_setjmp_receiver (operand0)
     rtx operand0;
{
  return gen_rtx_UNSPEC_VOLATILE (VOIDmode,
	gen_rtvec (1,
		operand0),
	7);
}

rtx
gen_fetchadd_acq_si (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	operand1),
		gen_rtx_SET (VOIDmode,
	operand1,
	gen_rtx_UNSPEC (SImode,
	gen_rtvec (2,
		operand1,
		operand2),
	20))));
}

rtx
gen_fetchadd_acq_di (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	operand1),
		gen_rtx_SET (VOIDmode,
	operand1,
	gen_rtx_UNSPEC (DImode,
	gen_rtvec (2,
		operand1,
		operand2),
	20))));
}

rtx
gen_cmpxchg_acq_si (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	operand1),
		gen_rtx_SET (VOIDmode,
	operand1,
	gen_rtx_UNSPEC (SImode,
	gen_rtvec (3,
		operand1,
		operand2,
		operand3),
	19))));
}

rtx
gen_cmpxchg_acq_di (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	operand1),
		gen_rtx_SET (VOIDmode,
	operand1,
	gen_rtx_UNSPEC (DImode,
	gen_rtvec (3,
		operand1,
		operand2,
		operand3),
	19))));
}

rtx
gen_xchgsi (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	operand1),
		gen_rtx_SET (VOIDmode,
	operand1,
	operand2)));
}

rtx
gen_xchgdi (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_PARALLEL (VOIDmode, gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	operand1),
		gen_rtx_SET (VOIDmode,
	operand1,
	operand2)));
}

rtx
gen_pred_rel_mutex (operand0)
     rtx operand0;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSPEC (BImode,
	gen_rtvec (1,
		operand0),
	15));
}

rtx
gen_safe_across_calls_all ()
{
  return gen_rtx_UNSPEC_VOLATILE (VOIDmode,
	gen_rtvec (1,
		const0_rtx),
	5);
}

rtx
gen_safe_across_calls_normal ()
{
  return gen_rtx_UNSPEC_VOLATILE (VOIDmode,
	gen_rtvec (1,
		const0_rtx),
	6);
}

rtx
gen_ptr_extend (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSPEC (DImode,
	gen_rtvec (1,
		operand1),
	24));
}

extern rtx gen_split_274 PARAMS ((rtx *));
rtx
gen_split_274 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx _val = 0;
  start_sequence ();

  operand0 = operands[0];
  operand1 = operands[1];
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	operand1,
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	operand0,
	const1_rtx)));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_EQ (VOIDmode,
	copy_rtx (operand1),
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	const0_rtx)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_275 PARAMS ((rtx *));
rtx
gen_split_275 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx operand5;
  rtx _val = 0;
  start_sequence ();
operands[2] = gen_rtx_REG (CCImode, REGNO (operands[0]));
   operands[3] = gen_rtx_REG (CCImode, REGNO (operands[0]) + 1);
   operands[4] = gen_rtx_REG (CCImode, REGNO (operands[1]));
   operands[5] = gen_rtx_REG (CCImode, REGNO (operands[1]) + 1);
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  operand5 = operands[5];
  emit_insn (gen_rtx_SET (VOIDmode,
	operand2,
	operand4));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand3,
	operand5));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSPEC (BImode,
	gen_rtvec (1,
		copy_rtx (operand0)),
	15)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_movqi (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
    operands[1] = operand1;
{
  rtx op1 = ia64_expand_move (operands[0], operands[1]);
  if (!op1)
    DONE;
  operands[1] = op1;
}
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_movhi (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
    operands[1] = operand1;
{
  rtx op1 = ia64_expand_move (operands[0], operands[1]);
  if (!op1)
    DONE;
  operands[1] = op1;
}
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_movsi (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
    operands[1] = operand1;
{
  rtx op1 = ia64_expand_move (operands[0], operands[1]);
  if (!op1)
    DONE;
  operands[1] = op1;
}
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_279 PARAMS ((rtx *));
rtx
gen_split_279 (operands)
      rtx *operands ATTRIBUTE_UNUSED;
{
  rtx _val = 0;
  start_sequence ();
{
  rtx scratch = operands[2];
  if (!reload_completed)
    scratch = gen_reg_rtx (Pmode);
  ia64_expand_load_address (operands[0], operands[1], scratch); 
  DONE;
}
  emit_insn (const0_rtx);
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_movdi (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
    operands[1] = operand1;
{
  rtx op1 = ia64_expand_move (operands[0], operands[1]);
  if (!op1)
    DONE;
  operands[1] = op1;
}
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_281 PARAMS ((rtx *));
rtx
gen_split_281 (operands)
      rtx *operands ATTRIBUTE_UNUSED;
{
  rtx _val = 0;
  start_sequence ();
{
  rtx scratch = operands[2];
  if (!reload_completed)
    scratch = gen_reg_rtx (Pmode);
  ia64_expand_load_address (operands[0], operands[1], scratch); 
  DONE;
}
  emit_insn (const0_rtx);
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_282 PARAMS ((rtx *));
rtx
gen_split_282 (operands)
      rtx *operands ATTRIBUTE_UNUSED;
{
  rtx _val = 0;
  start_sequence ();
{
  ia64_expand_load_address (operands[0], operands[1], NULL_RTX);
  DONE;
}
  emit_insn (const0_rtx);
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_load_fptr (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx operand2;
  rtx operand3;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[4];
    operands[0] = operand0;
    operands[1] = operand1;
{
  operands[2] = no_new_pseudos ? operands[0] : gen_reg_rtx (DImode);
  operands[3] = gen_rtx_MEM (DImode, operands[2]);
  RTX_UNCHANGING_P (operands[3]) = 1;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand2,
	gen_rtx_PLUS (DImode,
	gen_rtx_REG (DImode,
	1),
	operand1)));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand3));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_load_gprel64 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx operand2;
  rtx operand3;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[4];
    operands[0] = operand0;
    operands[1] = operand1;
{
  operands[2] = no_new_pseudos ? operands[0] : gen_reg_rtx (DImode);
  operands[3] = pic_offset_table_rtx;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand2,
	gen_rtx_MINUS (DImode,
	operand1,
	operand3)));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_PLUS (DImode,
	operand3,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_load_symptr (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx operand3;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[4];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  operands[3] = pic_offset_table_rtx;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand2,
	gen_rtx_PLUS (DImode,
	gen_rtx_HIGH (DImode,
	operand1),
	operand3)));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_LO_SUM (DImode,
	operand2,
	operand1)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_load_dtprel (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSPEC (DImode,
	gen_rtvec (1,
		operand1),
	2));
}

rtx
gen_add_dtprel (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_PLUS (DImode,
	operand1,
	gen_rtx_UNSPEC (DImode,
	gen_rtvec (1,
		operand2),
	2)));
}

rtx
gen_load_tprel (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSPEC (DImode,
	gen_rtvec (1,
		operand1),
	4));
}

rtx
gen_add_tprel (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  return gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_PLUS (DImode,
	operand1,
	gen_rtx_UNSPEC (DImode,
	gen_rtvec (1,
		operand2),
	4)));
}

rtx
gen_movti (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx operand2 ATTRIBUTE_UNUSED;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
{
  rtx op1 = ia64_expand_move (operands[0], operands[1]);
  if (!op1)
    DONE;
  operands[1] = op1;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	operand1),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (DImode)))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_291 PARAMS ((rtx *));
rtx
gen_split_291 (operands)
      rtx *operands ATTRIBUTE_UNUSED;
{
  rtx _val = 0;
  start_sequence ();
{
  rtx adj1, adj2, in[2], out[2], insn;
  int first;

  adj1 = ia64_split_timode (in, operands[1], operands[2]);
  adj2 = ia64_split_timode (out, operands[0], operands[2]);

  first = 0;
  if (reg_overlap_mentioned_p (out[0], in[1]))
    {
      if (reg_overlap_mentioned_p (out[1], in[0]))
	abort ();
      first = 1;
    }

  if (adj1 && adj2)
    abort ();
  if (adj1)
    emit_insn (adj1);
  if (adj2)
    emit_insn (adj2);
  insn = emit_insn (gen_rtx_SET (VOIDmode, out[first], in[first]));
  if (GET_CODE (out[first]) == MEM
      && GET_CODE (XEXP (out[first], 0)) == POST_MODIFY)
    REG_NOTES (insn) = gen_rtx_EXPR_LIST (REG_INC,
					  XEXP (XEXP (out[first], 0), 0),
					  REG_NOTES (insn));
  insn = emit_insn (gen_rtx_SET (VOIDmode, out[!first], in[!first]));
  if (GET_CODE (out[!first]) == MEM
      && GET_CODE (XEXP (out[!first], 0)) == POST_MODIFY)
    REG_NOTES (insn) = gen_rtx_EXPR_LIST (REG_INC,
					  XEXP (XEXP (out[!first], 0), 0),
					  REG_NOTES (insn));
  DONE;
}
  emit_insn (const0_rtx);
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_292 PARAMS ((rtx *));
rtx
gen_split_292 (operands)
      rtx *operands ATTRIBUTE_UNUSED;
{
  rtx _val = 0;
  start_sequence ();
{
  rtx in[2], out[2];
  int first;

  ia64_split_timode (in, operands[1], NULL_RTX);
  ia64_split_timode (out, operands[0], NULL_RTX);

  first = 0;
  if (reg_overlap_mentioned_p (out[0], in[1]))
    {
      if (reg_overlap_mentioned_p (out[1], in[0]))
	abort ();
      first = 1;
    }

  emit_insn (gen_rtx_SET (VOIDmode, out[first], in[first]));
  emit_insn (gen_rtx_SET (VOIDmode, out[!first], in[!first]));
  DONE;
}
  emit_insn (const0_rtx);
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_reload_inti (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  unsigned int s_regno = REGNO (operands[2]);
  if (s_regno == REGNO (operands[0]))
    s_regno += 1;
  operands[2] = gen_rtx_REG (DImode, s_regno);
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	operand1),
		gen_rtx_CLOBBER (VOIDmode,
	operand2))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_reload_outti (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  unsigned int s_regno = REGNO (operands[2]);
  if (s_regno == REGNO (operands[1]))
    s_regno += 1;
  operands[2] = gen_rtx_REG (DImode, s_regno);
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	operand1),
		gen_rtx_CLOBBER (VOIDmode,
	operand2))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_movsf (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
    operands[1] = operand1;
{
  rtx op1 = ia64_expand_move (operands[0], operands[1]);
  if (!op1)
    DONE;
  operands[1] = op1;
}
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_movdf (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
    operands[1] = operand1;
{
  rtx op1 = ia64_expand_move (operands[0], operands[1]);
  if (!op1)
    DONE;
  operands[1] = op1;
}
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_movtf (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
    operands[1] = operand1;
{
  /* We must support TFmode loads into general registers for stdarg/vararg
     and unprototyped calls.  We split them into DImode loads for convenience.
     We don't need TFmode stores from general regs, because a stdarg/vararg
     routine does a block store to memory of unnamed arguments.  */
  if (GET_CODE (operands[0]) == REG
      && GR_REGNO_P (REGNO (operands[0])))
    {
      /* We're hoping to transform everything that deals with TFmode
	 quantities and GR registers early in the compiler.  */
      if (no_new_pseudos)
	abort ();

      /* Struct to register can just use TImode instead.  */
      if ((GET_CODE (operands[1]) == SUBREG
	   && GET_MODE (SUBREG_REG (operands[1])) == TImode)
	  || (GET_CODE (operands[1]) == REG
	      && GR_REGNO_P (REGNO (operands[1]))))
	{
	  emit_move_insn (gen_rtx_REG (TImode, REGNO (operands[0])),
			  SUBREG_REG (operands[1]));
	  DONE;
	}

      if (GET_CODE (operands[1]) == CONST_DOUBLE)
	{
	  emit_move_insn (gen_rtx_REG (DImode, REGNO (operands[0])),
			  operand_subword (operands[1], 0, 0, TFmode));
	  emit_move_insn (gen_rtx_REG (DImode, REGNO (operands[0]) + 1),
			  operand_subword (operands[1], 1, 0, TFmode));
	  DONE;
	}

      /* If the quantity is in a register not known to be GR, spill it.  */
      if (register_operand (operands[1], TFmode))
	operands[1] = spill_tfmode_operand (operands[1], 1);

      if (GET_CODE (operands[1]) == MEM)
	{
	  rtx out[2];

	  out[WORDS_BIG_ENDIAN] = gen_rtx_REG (DImode, REGNO (operands[0]));
	  out[!WORDS_BIG_ENDIAN] = gen_rtx_REG (DImode, REGNO (operands[0])+1);

	  emit_move_insn (out[0], adjust_address (operands[1], DImode, 0));
	  emit_move_insn (out[1], adjust_address (operands[1], DImode, 8));
	  DONE;
	}

      abort ();
    }

  if (! reload_in_progress && ! reload_completed)
    {
      operands[0] = spill_tfmode_operand (operands[0], 0);
      operands[1] = spill_tfmode_operand (operands[1], 0);

      if (! ia64_move_ok (operands[0], operands[1]))
	operands[1] = force_reg (TFmode, operands[1]);
    }
}
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_insv (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[4];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
    operands[3] = operand3;
{
  int width = INTVAL (operands[1]);
  int shift = INTVAL (operands[2]);

  /* If operand[3] is a constant, and isn't 0 or -1, then load it into a
     pseudo.  */
  if (! register_operand (operands[3], DImode)
      && operands[3] != const0_rtx && operands[3] != constm1_rtx)
    operands[3] = force_reg (DImode, operands[3]);

  /* If this is a single dep instruction, we have nothing to do.  */
  if (! ((register_operand (operands[3], DImode) && width <= 16)
	 || operands[3] == const0_rtx || operands[3] == constm1_rtx))
    {
      /* Check for cases that can be implemented with a mix instruction.  */
      if (width == 32 && shift == 0)
	{
	  /* Directly generating the mix4left instruction confuses
	     optimize_bit_field in function.c.  Since this is performing
	     a useful optimization, we defer generation of the complicated
	     mix4left RTL to the first splitting phase.  */
	  rtx tmp = gen_reg_rtx (DImode);
	  emit_insn (gen_shift_mix4left (operands[0], operands[3], tmp));
	  DONE;
	}
      else if (width == 32 && shift == 32)
	{
	  emit_insn (gen_mix4right (operands[0], operands[3]));
	  DONE;
	}

      /* We could handle remaining cases by emitting multiple dep
	 instructions.

	 If we need more than two dep instructions then we lose.  A 6
	 insn sequence mov mask1,mov mask2,shl;;and,and;;or is better than
	 mov;;dep,shr;;dep,shr;;dep.  The former can be executed in 3 cycles,
	 the latter is 6 cycles on an Itanium (TM) processor, because there is
	 only one function unit that can execute dep and shr immed.

	 If we only need two dep instruction, then we still lose.
	 mov;;dep,shr;;dep is still 4 cycles.  Even if we optimize away
	 the unnecessary mov, this is still undesirable because it will be
	 hard to optimize, and it creates unnecessary pressure on the I0
	 function unit.  */

      FAIL;

#if 0
      /* This code may be useful for other IA-64 processors, so we leave it in
	 for now.  */
      while (width > 16)
	{
	  rtx tmp;

	  emit_insn (gen_insv (operands[0], GEN_INT (16), GEN_INT (shift),
			       operands[3]));
	  shift += 16;
	  width -= 16;
	  tmp = gen_reg_rtx (DImode);
	  emit_insn (gen_lshrdi3 (tmp, operands[3], GEN_INT (16)));
	  operands[3] = tmp;
	}
      operands[1] = GEN_INT (width);
      operands[2] = GEN_INT (shift);
#endif
    }
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	gen_rtx_ZERO_EXTRACT (DImode,
	operand0,
	operand1,
	operand2),
	operand3));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_299 PARAMS ((rtx *));
rtx
gen_split_299 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx _val = 0;
  start_sequence ();
operands[3] = operands[2];
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  emit_insn (gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_ASHIFT (DImode,
	operand1,
	GEN_INT (32L))));
  emit_insn (gen_rtx_SET (VOIDmode,
	gen_rtx_ZERO_EXTRACT (DImode,
	operand0,
	GEN_INT (32L),
	const0_rtx),
	gen_rtx_LSHIFTRT (DImode,
	copy_rtx (operand3),
	GEN_INT (32L))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_300 PARAMS ((rtx *));
rtx
gen_split_300 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx _val = 0;
  start_sequence ();
operands[3] = operands[2];
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  emit_insn (gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_ASHIFT (DImode,
	operand1,
	GEN_INT (32L))));
  emit_insn (gen_rtx_SET (VOIDmode,
	gen_rtx_ZERO_EXTRACT (DImode,
	operand0,
	GEN_INT (32L),
	const0_rtx),
	gen_rtx_LSHIFTRT (DImode,
	copy_rtx (operand3),
	GEN_INT (32L))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_301 PARAMS ((rtx *));
rtx
gen_split_301 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx _val = 0;
  start_sequence ();

  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_EQ (VOIDmode,
	operand2,
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_AND (BImode,
	gen_rtx_NE (BImode,
	const0_rtx,
	const0_rtx),
	copy_rtx (operand0)))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_302 PARAMS ((rtx *));
rtx
gen_split_302 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx _val = 0;
  start_sequence ();

  operand0 = operands[0];
  operand1 = operands[1];
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	operand1,
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_AND (BImode,
	gen_rtx_NE (BImode,
	const0_rtx,
	const0_rtx),
	copy_rtx (operand0)))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_303 PARAMS ((rtx *));
rtx
gen_split_303 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx _val = 0;
  start_sequence ();

  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	operand2,
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_IOR (BImode,
	gen_rtx_EQ (BImode,
	const0_rtx,
	const0_rtx),
	copy_rtx (operand0)))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_304 PARAMS ((rtx *));
rtx
gen_split_304 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx _val = 0;
  start_sequence ();

  operand0 = operands[0];
  operand1 = operands[1];
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_EQ (VOIDmode,
	operand1,
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_IOR (BImode,
	gen_rtx_EQ (BImode,
	const0_rtx,
	const0_rtx),
	copy_rtx (operand0)))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_305 PARAMS ((rtx *));
rtx
gen_split_305 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx _val = 0;
  start_sequence ();
operands[3] = gen_rtx_REG (CCImode, REGNO (operands[1]));
   operands[4] = gen_rtx_REG (CCImode, REGNO (operands[2]));
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  emit_insn (gen_rtx_SET (VOIDmode,
	operand4,
	operand3));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	const1_rtx));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	operand2,
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	const0_rtx)));
  emit_insn (gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_UNSPEC (BImode,
	gen_rtvec (1,
		copy_rtx (operand0)),
	15)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_306 PARAMS ((rtx *));
rtx
gen_split_306 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx _val = 0;
  start_sequence ();

  operand0 = operands[0];
  operand1 = operands[1];
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	operand1,
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	operand0,
	const0_rtx)));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_EQ (VOIDmode,
	copy_rtx (operand1),
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	const1_rtx)));
  emit_insn (gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_UNSPEC (BImode,
	gen_rtvec (1,
		copy_rtx (operand0)),
	15)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_307 PARAMS ((rtx *));
rtx
gen_split_307 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx _val = 0;
  start_sequence ();

  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_AND (BImode,
	gen_rtx_NE (BImode,
	gen_rtx_AND (DImode,
	operand3,
	const1_rtx),
	const0_rtx),
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_308 PARAMS ((rtx *));
rtx
gen_split_308 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx _val = 0;
  start_sequence ();

  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_AND (BImode,
	gen_rtx_NE (BImode,
	gen_rtx_AND (DImode,
	operand3,
	const1_rtx),
	const0_rtx),
	operand2)));
  emit (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_NOT (BImode,
	copy_rtx (operand0))),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (VOIDmode)))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_309 PARAMS ((rtx *));
rtx
gen_split_309 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx _val = 0;
  start_sequence ();

  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_IOR (BImode,
	gen_rtx_NE (BImode,
	operand3,
	const0_rtx),
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_310 PARAMS ((rtx *));
rtx
gen_split_310 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx _val = 0;
  start_sequence ();

  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_IOR (BImode,
	gen_rtx_NE (BImode,
	operand3,
	const0_rtx),
	operand2)));
  emit (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_NOT (BImode,
	copy_rtx (operand0))),
		gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (VOIDmode)))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_peephole2_311 PARAMS ((rtx, rtx *));
rtx
gen_peephole2_311 (curr_insn, operands)
     rtx curr_insn ATTRIBUTE_UNUSED;
     rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx operand5;
  rtx operand6;
  rtx operand7;
  rtx _val = 0;
  HARD_REG_SET _regs_allocated;
  CLEAR_HARD_REG_SET (_regs_allocated);
  start_sequence ();
operands[7] = copy_rtx (operands[1]);
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  operand5 = operands[5];
  operand6 = operands[6];
  operand7 = operands[7];
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand6,
	operand7));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_abssi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx operand2;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
{ operands[2] = gen_reg_rtx (BImode); }
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand2,
	gen_rtx_GE (BImode,
	operand1,
	const0_rtx)));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_IF_THEN_ELSE (SImode,
	gen_rtx_EQ (VOIDmode,
	operand2,
	const0_rtx),
	gen_rtx_NEG (SImode,
	operand1),
	operand1)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_sminsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx operand3;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[4];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{ operands[3] = gen_reg_rtx (BImode); }
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_GE (BImode,
	operand1,
	operand2)));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_IF_THEN_ELSE (SImode,
	gen_rtx_NE (VOIDmode,
	operand3,
	const0_rtx),
	operand2,
	operand1)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_smaxsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx operand3;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[4];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{ operands[3] = gen_reg_rtx (BImode); }
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_GE (BImode,
	operand1,
	operand2)));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_IF_THEN_ELSE (SImode,
	gen_rtx_NE (VOIDmode,
	operand3,
	const0_rtx),
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_uminsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx operand3;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[4];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{ operands[3] = gen_reg_rtx (BImode); }
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_GEU (BImode,
	operand1,
	operand2)));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_IF_THEN_ELSE (SImode,
	gen_rtx_NE (VOIDmode,
	operand3,
	const0_rtx),
	operand2,
	operand1)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_umaxsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx operand3;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[4];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{ operands[3] = gen_reg_rtx (BImode); }
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_GEU (BImode,
	operand1,
	operand2)));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_IF_THEN_ELSE (SImode,
	gen_rtx_NE (VOIDmode,
	operand3,
	const0_rtx),
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_divsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  rtx op1_tf, op2_tf, op0_tf, op0_di, twon34;
  REAL_VALUE_TYPE twon34_r;

  op0_tf = gen_reg_rtx (TFmode);
  op0_di = gen_reg_rtx (DImode);

  if (CONSTANT_P (operands[1]))
    operands[1] = force_reg (SImode, operands[1]);
  op1_tf = gen_reg_rtx (TFmode);
  expand_float (op1_tf, operands[1], 0);

  if (CONSTANT_P (operands[2]))
    operands[2] = force_reg (SImode, operands[2]);
  op2_tf = gen_reg_rtx (TFmode);
  expand_float (op2_tf, operands[2], 0);

  /* 2^-34 */
  real_2expN (&twon34_r, -34);
  twon34 = CONST_DOUBLE_FROM_REAL_VALUE (twon34_r, TFmode);
  twon34 = force_reg (TFmode, twon34);

  emit_insn (gen_divsi3_internal (op0_tf, op1_tf, op2_tf, twon34));

  emit_insn (gen_fix_trunctfdi2_alts (op0_di, op0_tf, const1_rtx));
  emit_move_insn (operands[0], gen_lowpart (SImode, op0_di));
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_DIV (SImode,
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_modsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  rtx op2_neg, op1_di, div;

  div = gen_reg_rtx (SImode);
  emit_insn (gen_divsi3 (div, operands[1], operands[2]));

  op2_neg = expand_unop (SImode, neg_optab, operands[2], NULL_RTX, 0);

  /* This is a trick to get us to reuse the value that we're sure to
     have already copied to the FP regs.  */
  op1_di = gen_reg_rtx (DImode);
  convert_move (op1_di, operands[1], 0);

  emit_insn (gen_maddsi4 (operands[0], div, op2_neg,
			  gen_lowpart (SImode, op1_di)));
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_MOD (SImode,
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_udivsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  rtx op1_tf, op2_tf, op0_tf, op0_di, twon34;
  REAL_VALUE_TYPE twon34_r;

  op0_tf = gen_reg_rtx (TFmode);
  op0_di = gen_reg_rtx (DImode);

  if (CONSTANT_P (operands[1]))
    operands[1] = force_reg (SImode, operands[1]);
  op1_tf = gen_reg_rtx (TFmode);
  expand_float (op1_tf, operands[1], 1);

  if (CONSTANT_P (operands[2]))
    operands[2] = force_reg (SImode, operands[2]);
  op2_tf = gen_reg_rtx (TFmode);
  expand_float (op2_tf, operands[2], 1);

  /* 2^-34 */
  real_2expN (&twon34_r, -34);
  twon34 = CONST_DOUBLE_FROM_REAL_VALUE (twon34_r, TFmode);
  twon34 = force_reg (TFmode, twon34);

  emit_insn (gen_divsi3_internal (op0_tf, op1_tf, op2_tf, twon34));

  emit_insn (gen_fixuns_trunctfdi2_alts (op0_di, op0_tf, const1_rtx));
  emit_move_insn (operands[0], gen_lowpart (SImode, op0_di));
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UDIV (SImode,
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_umodsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  rtx op2_neg, op1_di, div;

  div = gen_reg_rtx (SImode);
  emit_insn (gen_udivsi3 (div, operands[1], operands[2]));

  op2_neg = expand_unop (SImode, neg_optab, operands[2], NULL_RTX, 0);

  /* This is a trick to get us to reuse the value that we're sure to
     have already copied to the FP regs.  */
  op1_di = gen_reg_rtx (DImode);
  convert_move (op1_di, operands[1], 1);

  emit_insn (gen_maddsi4 (operands[0], div, op2_neg,
			  gen_lowpart (SImode, op1_di)));
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UMOD (SImode,
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_321 PARAMS ((rtx *));
rtx
gen_split_321 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx operand5;
  rtx operand6;
  rtx operand7;
  rtx _val = 0;
  start_sequence ();
operands[7] = CONST1_RTX (TFmode);
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  operand5 = operands[5];
  operand6 = operands[6];
  operand7 = operands[7];
  emit (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (3,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_DIV (TFmode,
	const1_rtx,
	operand2)),
		gen_rtx_SET (VOIDmode,
	operand6,
	gen_rtx_UNSPEC (BImode,
	gen_rtvec (2,
		operand1,
		copy_rtx (operand2)),
	14)),
		gen_rtx_USE (VOIDmode,
	const1_rtx))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand4,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand1),
	copy_rtx (operand0))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand5,
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand2),
	copy_rtx (operand0))),
	operand7)),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand4),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand5),
	copy_rtx (operand4)),
	copy_rtx (operand4))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand5),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand5),
	copy_rtx (operand5)),
	operand3)),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand5),
	copy_rtx (operand4)),
	copy_rtx (operand4))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_322 PARAMS ((rtx *));
rtx
gen_split_322 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx operand5;
  rtx _val = 0;
  start_sequence ();

  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  operand5 = operands[5];
  emit (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand5,
	gen_rtx_PLUS (DImode,
	gen_rtx_MULT (DImode,
	operand1,
	operand2),
	operand3)),
		gen_rtx_CLOBBER (VOIDmode,
	operand0))));
  emit_insn (gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	copy_rtx (operand5)));
  emit_insn (gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_PLUS (DImode,
	copy_rtx (operand0),
	operand4)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_absdi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx operand2;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
{ operands[2] = gen_reg_rtx (BImode); }
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand2,
	gen_rtx_GE (BImode,
	operand1,
	const0_rtx)));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_IF_THEN_ELSE (DImode,
	gen_rtx_EQ (VOIDmode,
	operand2,
	const0_rtx),
	gen_rtx_NEG (DImode,
	operand1),
	operand1)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_smindi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx operand3;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[4];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{ operands[3] = gen_reg_rtx (BImode); }
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_GE (BImode,
	operand1,
	operand2)));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_IF_THEN_ELSE (DImode,
	gen_rtx_NE (VOIDmode,
	operand3,
	const0_rtx),
	operand2,
	operand1)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_smaxdi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx operand3;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[4];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{ operands[3] = gen_reg_rtx (BImode); }
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_GE (BImode,
	operand1,
	operand2)));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_IF_THEN_ELSE (DImode,
	gen_rtx_NE (VOIDmode,
	operand3,
	const0_rtx),
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_umindi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx operand3;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[4];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{ operands[3] = gen_reg_rtx (BImode); }
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_GEU (BImode,
	operand1,
	operand2)));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_IF_THEN_ELSE (DImode,
	gen_rtx_NE (VOIDmode,
	operand3,
	const0_rtx),
	operand2,
	operand1)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_umaxdi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx operand3;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[4];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{ operands[3] = gen_reg_rtx (BImode); }
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_GEU (BImode,
	operand1,
	operand2)));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_IF_THEN_ELSE (DImode,
	gen_rtx_NE (VOIDmode,
	operand3,
	const0_rtx),
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_ffsdi2 (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx operand5;
  rtx operand6;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[7];
    operands[0] = operand0;
    operands[1] = operand1;
{
  operands[2] = gen_reg_rtx (DImode);
  operands[3] = gen_reg_rtx (DImode);
  operands[4] = gen_reg_rtx (DImode);
  operands[5] = gen_reg_rtx (DImode);
  operands[6] = gen_reg_rtx (BImode);
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
    operand4 = operands[4];
    operand5 = operands[5];
    operand6 = operands[6];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand6,
	gen_rtx_EQ (BImode,
	operand1,
	const0_rtx)));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand2,
	gen_rtx_PLUS (DImode,
	operand1,
	constm1_rtx)));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand5,
	const0_rtx));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_XOR (DImode,
	operand1,
	operand2)));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand4,
	gen_rtx_UNSPEC (DImode,
	gen_rtvec (1,
		operand3),
	16)));
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_IF_THEN_ELSE (DImode,
	gen_rtx_NE (VOIDmode,
	operand6,
	const0_rtx),
	operand5,
	operand4)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_divdi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  rtx op1_tf, op2_tf, op0_tf;

  op0_tf = gen_reg_rtx (TFmode);

  if (CONSTANT_P (operands[1]))
    operands[1] = force_reg (DImode, operands[1]);
  op1_tf = gen_reg_rtx (TFmode);
  expand_float (op1_tf, operands[1], 0);

  if (CONSTANT_P (operands[2]))
    operands[2] = force_reg (DImode, operands[2]);
  op2_tf = gen_reg_rtx (TFmode);
  expand_float (op2_tf, operands[2], 0);

  if (TARGET_INLINE_INT_DIV_LAT)
    emit_insn (gen_divdi3_internal_lat (op0_tf, op1_tf, op2_tf));
  else
    emit_insn (gen_divdi3_internal_thr (op0_tf, op1_tf, op2_tf));

  emit_insn (gen_fix_trunctfdi2_alts (operands[0], op0_tf, const1_rtx));
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_DIV (DImode,
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_moddi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  rtx op2_neg, div;

  div = gen_reg_rtx (DImode);
  emit_insn (gen_divdi3 (div, operands[1], operands[2]));

  op2_neg = expand_unop (DImode, neg_optab, operands[2], NULL_RTX, 0);

  emit_insn (gen_madddi4 (operands[0], div, op2_neg, operands[1]));
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_MOD (SImode,
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_udivdi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  rtx op1_tf, op2_tf, op0_tf;

  op0_tf = gen_reg_rtx (TFmode);

  if (CONSTANT_P (operands[1]))
    operands[1] = force_reg (DImode, operands[1]);
  op1_tf = gen_reg_rtx (TFmode);
  expand_float (op1_tf, operands[1], 1);

  if (CONSTANT_P (operands[2]))
    operands[2] = force_reg (DImode, operands[2]);
  op2_tf = gen_reg_rtx (TFmode);
  expand_float (op2_tf, operands[2], 1);

  if (TARGET_INLINE_INT_DIV_LAT)
    emit_insn (gen_divdi3_internal_lat (op0_tf, op1_tf, op2_tf));
  else
    emit_insn (gen_divdi3_internal_thr (op0_tf, op1_tf, op2_tf));

  emit_insn (gen_fixuns_trunctfdi2_alts (operands[0], op0_tf, const1_rtx));
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UDIV (DImode,
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_umoddi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  rtx op2_neg, div;

  div = gen_reg_rtx (DImode);
  emit_insn (gen_udivdi3 (div, operands[1], operands[2]));

  op2_neg = expand_unop (DImode, neg_optab, operands[2], NULL_RTX, 0);

  emit_insn (gen_madddi4 (operands[0], div, op2_neg, operands[1]));
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UMOD (DImode,
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_333 PARAMS ((rtx *));
rtx
gen_split_333 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx operand5;
  rtx operand6;
  rtx operand7;
  rtx _val = 0;
  start_sequence ();
operands[7] = CONST1_RTX (TFmode);
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  operand5 = operands[5];
  operand6 = operands[6];
  operand7 = operands[7];
  emit (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (3,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_DIV (TFmode,
	const1_rtx,
	operand2)),
		gen_rtx_SET (VOIDmode,
	operand6,
	gen_rtx_UNSPEC (BImode,
	gen_rtvec (2,
		operand1,
		copy_rtx (operand2)),
	14)),
		gen_rtx_USE (VOIDmode,
	const1_rtx))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand2),
	copy_rtx (operand0))),
	operand7)),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand4,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand1),
	copy_rtx (operand0))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand5,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand3))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand4),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand4)),
	copy_rtx (operand4))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand0)),
	copy_rtx (operand0))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand3),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand5),
	copy_rtx (operand4)),
	copy_rtx (operand4))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand5),
	copy_rtx (operand0)),
	copy_rtx (operand0))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand4),
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand2),
	copy_rtx (operand3))),
	copy_rtx (operand1))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand4),
	copy_rtx (operand0)),
	copy_rtx (operand3))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_334 PARAMS ((rtx *));
rtx
gen_split_334 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx operand5;
  rtx operand6;
  rtx _val = 0;
  start_sequence ();
operands[6] = CONST1_RTX (TFmode);
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  operand5 = operands[5];
  operand6 = operands[6];
  emit (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (3,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_DIV (TFmode,
	const1_rtx,
	operand2)),
		gen_rtx_SET (VOIDmode,
	operand5,
	gen_rtx_UNSPEC (BImode,
	gen_rtvec (2,
		operand1,
		copy_rtx (operand2)),
	14)),
		gen_rtx_USE (VOIDmode,
	const1_rtx))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand2),
	copy_rtx (operand0))),
	operand6)),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand0)),
	copy_rtx (operand0))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand3),
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand3))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand0)),
	copy_rtx (operand0))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand3),
	gen_rtx_MULT (TFmode,
	copy_rtx (operand0),
	copy_rtx (operand1))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand4,
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand2),
	copy_rtx (operand3))),
	copy_rtx (operand1))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand4),
	copy_rtx (operand0)),
	copy_rtx (operand3))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_divsf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  rtx insn;
  if (TARGET_INLINE_FLOAT_DIV_LAT)
    insn = gen_divsf3_internal_lat (operands[0], operands[1], operands[2]);
  else
    insn = gen_divsf3_internal_thr (operands[0], operands[1], operands[2]);
  emit_insn (insn);
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_DIV (SFmode,
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_336 PARAMS ((rtx *));
rtx
gen_split_336 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx operand5;
  rtx operand6;
  rtx operand7;
  rtx operand8;
  rtx operand9;
  rtx operand10;
  rtx _val = 0;
  start_sequence ();
{
  operands[6] = gen_rtx_REG (TFmode, REGNO (operands[0]));
  operands[7] = gen_rtx_REG (TFmode, REGNO (operands[1]));
  operands[8] = gen_rtx_REG (TFmode, REGNO (operands[2]));
  operands[9] = gen_rtx_REG (DFmode, REGNO (operands[0]));
  operands[10] = CONST1_RTX (TFmode);
}
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  operand5 = operands[5];
  operand6 = operands[6];
  operand7 = operands[7];
  operand8 = operands[8];
  operand9 = operands[9];
  operand10 = operands[10];
  emit (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (3,
		gen_rtx_SET (VOIDmode,
	operand6,
	gen_rtx_DIV (TFmode,
	const1_rtx,
	operand8)),
		gen_rtx_SET (VOIDmode,
	operand5,
	gen_rtx_UNSPEC (BImode,
	gen_rtvec (2,
		operand7,
		copy_rtx (operand8)),
	14)),
		gen_rtx_USE (VOIDmode,
	const1_rtx))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand7),
	copy_rtx (operand6))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand4,
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand8),
	copy_rtx (operand6))),
	operand10)),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand3),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand4),
	copy_rtx (operand3)),
	copy_rtx (operand3))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand4),
	gen_rtx_MULT (TFmode,
	copy_rtx (operand4),
	copy_rtx (operand4))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand3),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand4),
	copy_rtx (operand3)),
	copy_rtx (operand3))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand4),
	gen_rtx_MULT (TFmode,
	copy_rtx (operand4),
	copy_rtx (operand4))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand9,
	gen_rtx_FLOAT_TRUNCATE (DFmode,
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand4),
	copy_rtx (operand3)),
	copy_rtx (operand3)))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_FLOAT_TRUNCATE (SFmode,
	copy_rtx (operand6)))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_337 PARAMS ((rtx *));
rtx
gen_split_337 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx operand5;
  rtx operand6;
  rtx operand7;
  rtx operand8;
  rtx operand9;
  rtx operand10;
  rtx _val = 0;
  start_sequence ();
{
  operands[6] = gen_rtx_REG (TFmode, REGNO (operands[0]));
  operands[7] = gen_rtx_REG (TFmode, REGNO (operands[1]));
  operands[8] = gen_rtx_REG (TFmode, REGNO (operands[2]));
  operands[9] = gen_rtx_REG (SFmode, REGNO (operands[3]));
  operands[10] = CONST1_RTX (TFmode);
}
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  operand5 = operands[5];
  operand6 = operands[6];
  operand7 = operands[7];
  operand8 = operands[8];
  operand9 = operands[9];
  operand10 = operands[10];
  emit (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (3,
		gen_rtx_SET (VOIDmode,
	operand6,
	gen_rtx_DIV (TFmode,
	const1_rtx,
	operand8)),
		gen_rtx_SET (VOIDmode,
	operand5,
	gen_rtx_UNSPEC (BImode,
	gen_rtvec (2,
		operand7,
		copy_rtx (operand8)),
	14)),
		gen_rtx_USE (VOIDmode,
	const1_rtx))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand8),
	copy_rtx (operand6))),
	operand10)),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand3),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand3)),
	copy_rtx (operand3))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand6),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand6)),
	copy_rtx (operand6))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand9,
	gen_rtx_FLOAT_TRUNCATE (SFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand7),
	copy_rtx (operand6)))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand4,
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand8),
	copy_rtx (operand3))),
	copy_rtx (operand7))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_FLOAT_TRUNCATE (SFmode,
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand4),
	copy_rtx (operand6)),
	copy_rtx (operand3))))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_divdf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  rtx insn;
  if (TARGET_INLINE_FLOAT_DIV_LAT)
    insn = gen_divdf3_internal_lat (operands[0], operands[1], operands[2]);
  else
    insn = gen_divdf3_internal_thr (operands[0], operands[1], operands[2]);
  emit_insn (insn);
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_DIV (DFmode,
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_339 PARAMS ((rtx *));
rtx
gen_split_339 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx operand5;
  rtx operand6;
  rtx operand7;
  rtx operand8;
  rtx operand9;
  rtx operand10;
  rtx operand11;
  rtx operand12;
  rtx _val = 0;
  start_sequence ();
{
  operands[7] = gen_rtx_REG (TFmode, REGNO (operands[0]));
  operands[8] = gen_rtx_REG (TFmode, REGNO (operands[1]));
  operands[9] = gen_rtx_REG (TFmode, REGNO (operands[2]));
  operands[10] = gen_rtx_REG (DFmode, REGNO (operands[3]));
  operands[11] = gen_rtx_REG (DFmode, REGNO (operands[5]));
  operands[12] = CONST1_RTX (TFmode);
}
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  operand5 = operands[5];
  operand6 = operands[6];
  operand7 = operands[7];
  operand8 = operands[8];
  operand9 = operands[9];
  operand10 = operands[10];
  operand11 = operands[11];
  operand12 = operands[12];
  emit (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (3,
		gen_rtx_SET (VOIDmode,
	operand7,
	gen_rtx_DIV (TFmode,
	const1_rtx,
	operand9)),
		gen_rtx_SET (VOIDmode,
	operand6,
	gen_rtx_UNSPEC (BImode,
	gen_rtvec (2,
		operand8,
		copy_rtx (operand9)),
	14)),
		gen_rtx_USE (VOIDmode,
	const1_rtx))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand8),
	copy_rtx (operand7))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand4,
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand9),
	copy_rtx (operand7))),
	operand12)),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand3),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand4),
	copy_rtx (operand3)),
	copy_rtx (operand3))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand5,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand4),
	copy_rtx (operand4))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand7),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand4),
	copy_rtx (operand7)),
	copy_rtx (operand7))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand3),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand5),
	copy_rtx (operand3)),
	copy_rtx (operand3))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand4),
	gen_rtx_MULT (TFmode,
	copy_rtx (operand5),
	copy_rtx (operand5))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand7),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand5),
	copy_rtx (operand7)),
	copy_rtx (operand7))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand10,
	gen_rtx_FLOAT_TRUNCATE (DFmode,
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand4),
	copy_rtx (operand3)),
	copy_rtx (operand3)))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand7),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand4),
	copy_rtx (operand7)),
	copy_rtx (operand7))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand11,
	gen_rtx_FLOAT_TRUNCATE (DFmode,
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand9),
	copy_rtx (operand3))),
	copy_rtx (operand8)))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand6),
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_FLOAT_TRUNCATE (DFmode,
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand5),
	copy_rtx (operand7)),
	copy_rtx (operand3))))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_340 PARAMS ((rtx *));
rtx
gen_split_340 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx operand5;
  rtx operand6;
  rtx operand7;
  rtx operand8;
  rtx operand9;
  rtx operand10;
  rtx _val = 0;
  start_sequence ();
{
  operands[6] = gen_rtx_REG (TFmode, REGNO (operands[0]));
  operands[7] = gen_rtx_REG (TFmode, REGNO (operands[1]));
  operands[8] = gen_rtx_REG (TFmode, REGNO (operands[2]));
  operands[9] = gen_rtx_REG (DFmode, REGNO (operands[3]));
  operands[10] = CONST1_RTX (TFmode);
}
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  operand5 = operands[5];
  operand6 = operands[6];
  operand7 = operands[7];
  operand8 = operands[8];
  operand9 = operands[9];
  operand10 = operands[10];
  emit (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (3,
		gen_rtx_SET (VOIDmode,
	operand6,
	gen_rtx_DIV (TFmode,
	const1_rtx,
	operand8)),
		gen_rtx_SET (VOIDmode,
	operand5,
	gen_rtx_UNSPEC (BImode,
	gen_rtvec (2,
		operand7,
		copy_rtx (operand8)),
	14)),
		gen_rtx_USE (VOIDmode,
	const1_rtx))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand8),
	copy_rtx (operand6))),
	operand10)),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand6),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand6)),
	copy_rtx (operand6))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand3),
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand3))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand6),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand6)),
	copy_rtx (operand6))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand3),
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand3))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand6),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand6)),
	copy_rtx (operand6))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand9,
	gen_rtx_FLOAT_TRUNCATE (DFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand7),
	copy_rtx (operand3)))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand4,
	gen_rtx_PLUS (DFmode,
	gen_rtx_NEG (DFmode,
	gen_rtx_MULT (DFmode,
	operand2,
	copy_rtx (operand9))),
	operand1)),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_PLUS (DFmode,
	gen_rtx_MULT (DFmode,
	copy_rtx (operand4),
	copy_rtx (operand0)),
	copy_rtx (operand9)))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_divtf3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  rtx insn;
  if (TARGET_INLINE_FLOAT_DIV_LAT)
    insn = gen_divtf3_internal_lat (operands[0], operands[1], operands[2]);
  else
    insn = gen_divtf3_internal_thr (operands[0], operands[1], operands[2]);
  emit_insn (insn);
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_DIV (TFmode,
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_342 PARAMS ((rtx *));
rtx
gen_split_342 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx operand5;
  rtx operand6;
  rtx operand7;
  rtx operand8;
  rtx _val = 0;
  start_sequence ();
operands[8] = CONST1_RTX (TFmode);
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  operand5 = operands[5];
  operand6 = operands[6];
  operand7 = operands[7];
  operand8 = operands[8];
  emit (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (3,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_DIV (TFmode,
	const1_rtx,
	operand2)),
		gen_rtx_SET (VOIDmode,
	operand7,
	gen_rtx_UNSPEC (BImode,
	gen_rtvec (2,
		operand1,
		copy_rtx (operand2)),
	14)),
		gen_rtx_USE (VOIDmode,
	const1_rtx))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand7),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand2),
	copy_rtx (operand0))),
	operand8)),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand7),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand4,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand1),
	copy_rtx (operand0))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand7),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand5,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand3))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand7),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand6,
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand3)),
	copy_rtx (operand3))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand7),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand3),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand5),
	copy_rtx (operand5)),
	copy_rtx (operand3))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand7),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand5),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand6),
	copy_rtx (operand0)),
	copy_rtx (operand0))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand7),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand5),
	copy_rtx (operand3)),
	copy_rtx (operand0))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand7),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand4),
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand2),
	copy_rtx (operand4))),
	copy_rtx (operand1))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand7),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand3),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand0)),
	copy_rtx (operand4))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand7),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand5),
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand2),
	copy_rtx (operand0))),
	copy_rtx (operand8))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand7),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand4),
	copy_rtx (operand0)),
	copy_rtx (operand0))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand7),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand4),
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand2),
	copy_rtx (operand3))),
	copy_rtx (operand1))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand7),
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand4),
	copy_rtx (operand0)),
	copy_rtx (operand3)))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_343 PARAMS ((rtx *));
rtx
gen_split_343 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx operand5;
  rtx operand6;
  rtx _val = 0;
  start_sequence ();
operands[6] = CONST1_RTX (TFmode);
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  operand5 = operands[5];
  operand6 = operands[6];
  emit (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (3,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_DIV (TFmode,
	const1_rtx,
	operand2)),
		gen_rtx_SET (VOIDmode,
	operand5,
	gen_rtx_UNSPEC (BImode,
	gen_rtvec (2,
		operand1,
		copy_rtx (operand2)),
	14)),
		gen_rtx_USE (VOIDmode,
	const1_rtx))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand2),
	copy_rtx (operand0))),
	operand6)),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand4,
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand0)),
	copy_rtx (operand0))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand3),
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand3))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand3),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand4)),
	copy_rtx (operand4))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand4),
	gen_rtx_MULT (TFmode,
	copy_rtx (operand1),
	copy_rtx (operand0))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand2),
	copy_rtx (operand3))),
	copy_rtx (operand6))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand0),
	copy_rtx (operand3)),
	copy_rtx (operand3))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand3),
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand2),
	copy_rtx (operand4))),
	copy_rtx (operand1))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand3),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand3),
	copy_rtx (operand0)),
	copy_rtx (operand4))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand4),
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand2),
	copy_rtx (operand0))),
	copy_rtx (operand6))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand4),
	copy_rtx (operand0)),
	copy_rtx (operand0))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	copy_rtx (operand4),
	gen_rtx_PLUS (TFmode,
	gen_rtx_NEG (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand2),
	copy_rtx (operand3))),
	copy_rtx (operand1))),
		gen_rtx_USE (VOIDmode,
	const1_rtx)))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	copy_rtx (operand5),
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_PLUS (TFmode,
	gen_rtx_MULT (TFmode,
	copy_rtx (operand4),
	copy_rtx (operand0)),
	copy_rtx (operand3)))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_ashlsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  if (GET_CODE (operands[2]) != CONST_INT)
    {
      /* Why oh why didn't Intel arrange for SHIFT_COUNT_TRUNCATED?  Now
	 we've got to get rid of stray bits outside the SImode register.  */
      rtx subshift = gen_reg_rtx (DImode);
      emit_insn (gen_zero_extendsidi2 (subshift, operands[2]));
      operands[2] = subshift;
    }
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_ASHIFT (SImode,
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_ashrsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
/*FMZH  rtx subtarget = gen_reg_rtx (DImode);*/
  rtx subtarget = gen_reg_rtx (SImode);
  if (GET_CODE (operands[2]) == CONST_INT)
/*FMZH    emit_insn (gen_extv (subtarget, gen_lowpart (DImode, operands[1]),*/
    emit_insn (gen_extv (subtarget, gen_lowpart (SImode, operands[1]),
			 GEN_INT (32 - INTVAL (operands[2])), operands[2]));
  else
    {
/*FMZH      rtx subshift = gen_reg_rtx (DImode);*/
      rtx subshift = gen_reg_rtx (SImode);
      emit_insn (gen_extendsidi2 (subtarget, operands[1]));
      emit_insn (gen_zero_extendsidi2 (subshift, operands[2]));
      emit_insn (gen_ashrdi3 (subtarget, subtarget, subshift));
    }
/*FMZH  emit_move_insn (gen_lowpart (DImode, operands[0]), subtarget);*/
  emit_move_insn (gen_lowpart (SImode, operands[0]), subtarget);
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_ASHIFTRT (SImode,
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_lshrsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
/* FMZH  rtx subtarget = gen_reg_rtx (DImode); */
  rtx subtarget = gen_reg_rtx (SImode);
  if (GET_CODE (operands[2]) == CONST_INT)
/*    emit_insn (gen_extzv (subtarget, gen_lowpart (DImode, operands[1]),*/
    emit_insn (gen_extzv (subtarget, gen_lowpart (SImode, operands[1]),
			  GEN_INT (32 - INTVAL (operands[2])), operands[2]));
  else
    {
/*      rtx subshift = gen_reg_rtx (DImode); */
      rtx subshift = gen_reg_rtx (SImode);
      emit_insn (gen_zero_extendsidi2 (subtarget, operands[1]));
      emit_insn (gen_zero_extendsidi2 (subshift, operands[2]));
      emit_insn (gen_lshrdi3 (subtarget, subtarget, subshift));
    }
/*  emit_move_insn (gen_lowpart (DImode, operands[0]), subtarget); */
  emit_move_insn (gen_lowpart (SImode, operands[0]), subtarget);
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_LSHIFTRT (SImode,
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_rotrsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  if (GET_MODE (operands[2]) != VOIDmode)
    {
      rtx tmp = gen_reg_rtx (DImode);
      emit_insn (gen_zero_extendsidi2 (tmp, operands[2]));
      operands[2] = tmp;
    }
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_ROTATERT (SImode,
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_348 PARAMS ((rtx *));
rtx
gen_split_348 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx _val = 0;
  start_sequence ();
operands[3] = gen_rtx_REG (DImode, REGNO (operands[0]));
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  emit_insn (gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_IOR (DImode,
	gen_rtx_ZERO_EXTEND (DImode,
	operand1),
	gen_rtx_ASHIFT (DImode,
	gen_rtx_ZERO_EXTEND (DImode,
	copy_rtx (operand1)),
	GEN_INT (32L)))));
  emit_insn (gen_rtx_SET (VOIDmode,
	copy_rtx (operand3),
	gen_rtx_LSHIFTRT (DImode,
	copy_rtx (operand3),
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_rotlsi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  if (! shift_32bit_count_operand (operands[2], SImode))
    {
      rtx tmp = gen_reg_rtx (SImode);
      emit_insn (gen_subsi3 (tmp, GEN_INT (32), operands[2]));
      emit_insn (gen_rotrsi3 (operands[0], operands[1], tmp));
      DONE;
    }
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_ROTATE (SImode,
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_350 PARAMS ((rtx *));
rtx
gen_split_350 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx _val = 0;
  start_sequence ();
{
  operands[3] = gen_rtx_REG (DImode, REGNO (operands[0]));
  operands[2] = GEN_INT (32 - INTVAL (operands[2]));
}
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  emit_insn (gen_rtx_SET (VOIDmode,
	operand3,
	gen_rtx_IOR (DImode,
	gen_rtx_ZERO_EXTEND (DImode,
	operand1),
	gen_rtx_ASHIFT (DImode,
	gen_rtx_ZERO_EXTEND (DImode,
	copy_rtx (operand1)),
	GEN_INT (32L)))));
  emit_insn (gen_rtx_SET (VOIDmode,
	copy_rtx (operand3),
	gen_rtx_LSHIFTRT (DImode,
	copy_rtx (operand3),
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_351 PARAMS ((rtx *));
rtx
gen_split_351 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx _val = 0;
  start_sequence ();

  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_PLUS (DImode,
	gen_rtx_MULT (DImode,
	operand1,
	operand2),
	operand3)));
  emit_insn (gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx_PLUS (DImode,
	copy_rtx (operand0),
	operand4)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_rotrdi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  if (! shift_count_operand (operands[2], DImode))
    FAIL;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_ROTATERT (DImode,
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_rotldi3 (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  if (! shift_count_operand (operands[2], DImode))
    FAIL;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_ROTATE (DImode,
	operand1,
	operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_cmpbi (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
    operands[1] = operand1;
{
  ia64_compare_op0 = operands[0];
  ia64_compare_op1 = operands[1];
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	cc0_rtx,
	gen_rtx_COMPARE (VOIDmode,
	operand0,
	operand1)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_cmpsi (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
    operands[1] = operand1;
{
  ia64_compare_op0 = operands[0];
  ia64_compare_op1 = operands[1];
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	cc0_rtx,
	gen_rtx_COMPARE (VOIDmode,
	operand0,
	operand1)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_cmpdi (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
    operands[1] = operand1;
{
  ia64_compare_op0 = operands[0];
  ia64_compare_op1 = operands[1];
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	cc0_rtx,
	gen_rtx_COMPARE (VOIDmode,
	operand0,
	operand1)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_cmpsf (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
    operands[1] = operand1;
{
  ia64_compare_op0 = operands[0];
  ia64_compare_op1 = operands[1];
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	cc0_rtx,
	gen_rtx_COMPARE (VOIDmode,
	operand0,
	operand1)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_cmpdf (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
    operands[1] = operand1;
{
  ia64_compare_op0 = operands[0];
  ia64_compare_op1 = operands[1];
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	cc0_rtx,
	gen_rtx_COMPARE (VOIDmode,
	operand0,
	operand1)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_cmptf (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
    operands[1] = operand1;
{
  ia64_compare_op0 = operands[0];
  ia64_compare_op1 = operands[1];
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	cc0_rtx,
	gen_rtx_COMPARE (VOIDmode,
	operand0,
	operand1)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_beq (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (EQ, VOIDmode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_jump_insn (gen_rtx_SET (VOIDmode,
	pc_rtx,
	gen_rtx_IF_THEN_ELSE (VOIDmode,
	operand1,
	gen_rtx_LABEL_REF (VOIDmode,
	operand0),
	pc_rtx)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_bne (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (NE, VOIDmode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_jump_insn (gen_rtx_SET (VOIDmode,
	pc_rtx,
	gen_rtx_IF_THEN_ELSE (VOIDmode,
	operand1,
	gen_rtx_LABEL_REF (VOIDmode,
	operand0),
	pc_rtx)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_blt (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (LT, VOIDmode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_jump_insn (gen_rtx_SET (VOIDmode,
	pc_rtx,
	gen_rtx_IF_THEN_ELSE (VOIDmode,
	operand1,
	gen_rtx_LABEL_REF (VOIDmode,
	operand0),
	pc_rtx)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_ble (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (LE, VOIDmode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_jump_insn (gen_rtx_SET (VOIDmode,
	pc_rtx,
	gen_rtx_IF_THEN_ELSE (VOIDmode,
	operand1,
	gen_rtx_LABEL_REF (VOIDmode,
	operand0),
	pc_rtx)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_bgt (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (GT, VOIDmode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_jump_insn (gen_rtx_SET (VOIDmode,
	pc_rtx,
	gen_rtx_IF_THEN_ELSE (VOIDmode,
	operand1,
	gen_rtx_LABEL_REF (VOIDmode,
	operand0),
	pc_rtx)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_bge (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (GE, VOIDmode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_jump_insn (gen_rtx_SET (VOIDmode,
	pc_rtx,
	gen_rtx_IF_THEN_ELSE (VOIDmode,
	operand1,
	gen_rtx_LABEL_REF (VOIDmode,
	operand0),
	pc_rtx)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_bltu (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (LTU, VOIDmode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_jump_insn (gen_rtx_SET (VOIDmode,
	pc_rtx,
	gen_rtx_IF_THEN_ELSE (VOIDmode,
	operand1,
	gen_rtx_LABEL_REF (VOIDmode,
	operand0),
	pc_rtx)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_bleu (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (LEU, VOIDmode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_jump_insn (gen_rtx_SET (VOIDmode,
	pc_rtx,
	gen_rtx_IF_THEN_ELSE (VOIDmode,
	operand1,
	gen_rtx_LABEL_REF (VOIDmode,
	operand0),
	pc_rtx)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_bgtu (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (GTU, VOIDmode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_jump_insn (gen_rtx_SET (VOIDmode,
	pc_rtx,
	gen_rtx_IF_THEN_ELSE (VOIDmode,
	operand1,
	gen_rtx_LABEL_REF (VOIDmode,
	operand0),
	pc_rtx)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_bgeu (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (GEU, VOIDmode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_jump_insn (gen_rtx_SET (VOIDmode,
	pc_rtx,
	gen_rtx_IF_THEN_ELSE (VOIDmode,
	operand1,
	gen_rtx_LABEL_REF (VOIDmode,
	operand0),
	pc_rtx)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_bunordered (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (UNORDERED, VOIDmode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_jump_insn (gen_rtx_SET (VOIDmode,
	pc_rtx,
	gen_rtx_IF_THEN_ELSE (VOIDmode,
	operand1,
	gen_rtx_LABEL_REF (VOIDmode,
	operand0),
	pc_rtx)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_bordered (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (ORDERED, VOIDmode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_jump_insn (gen_rtx_SET (VOIDmode,
	pc_rtx,
	gen_rtx_IF_THEN_ELSE (VOIDmode,
	operand1,
	gen_rtx_LABEL_REF (VOIDmode,
	operand0),
	pc_rtx)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_doloop_end (operand0, operand1, operand2, operand3, operand4)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
     rtx operand4;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[5];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
    operands[3] = operand3;
    operands[4] = operand4;
{
  /* Only use cloop on innermost loops.  */
  if (INTVAL (operands[3]) > 1)
    FAIL;
  emit_jump_insn (gen_doloop_end_internal (gen_rtx_REG (DImode, AR_LC_REGNUM),
					   operands[4]));
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
    operand4 = operands[4];
  }
  emit_insn (gen_rtx_USE (VOIDmode,
	operand0));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand1));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand2));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand3));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand4));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_seq (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (EQ, DImode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_sne (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (NE, DImode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_slt (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (LT, DImode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_sle (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (LE, DImode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_sgt (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (GT, DImode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_sge (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (GE, DImode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_sltu (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (LTU, DImode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_sleu (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (LEU, DImode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_sgtu (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (GTU, DImode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_sgeu (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (GEU, DImode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_sunordered (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (UNORDERED, DImode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_sordered (operand0)
     rtx operand0;
{
  rtx operand1;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
operands[1] = ia64_expand_compare (ORDERED, DImode);
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_385 PARAMS ((rtx *));
rtx
gen_split_385 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx _val = 0;
  start_sequence ();

  operand0 = operands[0];
  operand1 = operands[1];
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	operand1,
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	operand0,
	const1_rtx)));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_EQ (VOIDmode,
	copy_rtx (operand1),
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	const0_rtx)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_386 PARAMS ((rtx *));
rtx
gen_split_386 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx _val = 0;
  start_sequence ();

  operand0 = operands[0];
  operand1 = operands[1];
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_NE (VOIDmode,
	operand1,
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	operand0,
	const0_rtx)));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	gen_rtx_EQ (VOIDmode,
	copy_rtx (operand1),
	const0_rtx),
	gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	const1_rtx)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_387 PARAMS ((rtx *));
rtx
gen_split_387 (operands)
      rtx *operands ATTRIBUTE_UNUSED;
{
  rtx _val = 0;
  start_sequence ();
{
  rtx tmp;
  int emitted_something;

  emitted_something = 0;
  if (! rtx_equal_p (operands[0], operands[2]))
    {
      tmp = gen_rtx_SET (VOIDmode, operands[0], operands[2]);
      tmp = gen_rtx_COND_EXEC (VOIDmode, operands[4], tmp);
      emit_insn (tmp);
      emitted_something = 1;
    }
  if (! rtx_equal_p (operands[0], operands[3]))
    {
      tmp = gen_rtx_fmt_ee (GET_CODE (operands[4]) == NE ? EQ : NE,
			    VOIDmode, operands[1], const0_rtx);
      tmp = gen_rtx_COND_EXEC (VOIDmode, tmp,
			       gen_rtx_SET (VOIDmode, operands[0],
					    operands[3]));
      emit_insn (tmp);
      emitted_something = 1;
    }
  if (! emitted_something)
    emit_note (NULL, NOTE_INSN_DELETED);
  DONE;
}
  emit_insn (const0_rtx);
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_388 PARAMS ((rtx *));
rtx
gen_split_388 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx _val = 0;
  start_sequence ();

  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	operand4,
	gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_NEG (DImode,
	operand2))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_389 PARAMS ((rtx *));
rtx
gen_split_389 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx operand5;
  rtx _val = 0;
  start_sequence ();
{
  operands[5] = gen_rtx_fmt_ee (GET_CODE (operands[4]) == NE ? EQ : NE,
				VOIDmode, operands[1], const0_rtx);
}
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  operand5 = operands[5];
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	operand4,
	gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_NEG (DImode,
	operand2))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	operand5,
	gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	operand3)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_390 PARAMS ((rtx *));
rtx
gen_split_390 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx _val = 0;
  start_sequence ();

  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	operand4,
	gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_NEG (SImode,
	operand2))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_391 PARAMS ((rtx *));
rtx
gen_split_391 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx operand5;
  rtx _val = 0;
  start_sequence ();
{
  operands[5] = gen_rtx_fmt_ee (GET_CODE (operands[4]) == NE ? EQ : NE,
				VOIDmode, operands[1], const0_rtx);
}
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  operand5 = operands[5];
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	operand4,
	gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_NEG (SImode,
	operand2))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	operand5,
	gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	operand3)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_392 PARAMS ((rtx *));
rtx
gen_split_392 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx operand5;
  rtx operand6;
  rtx operand7;
  rtx _val = 0;
  start_sequence ();
{
  operands[7] = gen_rtx_fmt_ee (GET_CODE (operands[6]) == NE ? EQ : NE,
				VOIDmode, operands[1], const0_rtx);
}
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  operand5 = operands[5];
  operand6 = operands[6];
  operand7 = operands[7];
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	operand6,
	gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx (GET_CODE (operand5), SImode,
		operand2,
		operand4))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	operand7,
	gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx (GET_CODE (operand5), SImode,
		operand3,
		copy_rtx (operand4)))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_393 PARAMS ((rtx *));
rtx
gen_split_393 (operands)
      rtx *operands;
{
  rtx operand0;
  rtx operand1;
  rtx operand2;
  rtx operand3;
  rtx operand4;
  rtx operand5;
  rtx operand6;
  rtx operand7;
  rtx _val = 0;
  start_sequence ();
{
  operands[7] = gen_rtx_fmt_ee (GET_CODE (operands[6]) == NE ? EQ : NE,
				VOIDmode, operands[1], const0_rtx);
}
  operand0 = operands[0];
  operand1 = operands[1];
  operand2 = operands[2];
  operand3 = operands[3];
  operand4 = operands[4];
  operand5 = operands[5];
  operand6 = operands[6];
  operand7 = operands[7];
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	operand6,
	gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx (GET_CODE (operand5), SImode,
		operand4,
		operand2))));
  emit_insn (gen_rtx_COND_EXEC (VOIDmode,
	operand7,
	gen_rtx_SET (VOIDmode,
	copy_rtx (operand0),
	gen_rtx (GET_CODE (operand5), SImode,
		copy_rtx (operand4),
		operand3))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_call (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[4];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
    operands[3] = operand3;
{
  ia64_expand_call (NULL_RTX, operands[0], operands[2], false);
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
  }
  emit_insn (gen_rtx_USE (VOIDmode,
	operand0));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand1));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand2));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand3));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_sibcall (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[4];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
    operands[3] = operand3;
{
  ia64_expand_call (NULL_RTX, operands[0], operands[2], true);
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
  }
  emit_insn (gen_rtx_USE (VOIDmode,
	operand0));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand1));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand2));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand3));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_call_value (operand0, operand1, operand2, operand3, operand4)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
     rtx operand4;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[5];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
    operands[3] = operand3;
    operands[4] = operand4;
{
  ia64_expand_call (operands[0], operands[1], operands[3], false);
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
    operand4 = operands[4];
  }
  emit_insn (gen_rtx_USE (VOIDmode,
	operand0));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand1));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand2));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand3));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand4));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_sibcall_value (operand0, operand1, operand2, operand3, operand4)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
     rtx operand4;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[5];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
    operands[3] = operand3;
    operands[4] = operand4;
{
  ia64_expand_call (operands[0], operands[1], operands[3], true);
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
    operand4 = operands[4];
  }
  emit_insn (gen_rtx_USE (VOIDmode,
	operand0));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand1));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand2));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand3));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand4));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_untyped_call (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  int i;

  emit_call_insn (gen_call (operands[0], const0_rtx, NULL, const0_rtx));

  for (i = 0; i < XVECLEN (operands[2], 0); i++)
    {
      rtx set = XVECEXP (operands[2], 0, i);
      emit_move_insn (SET_DEST (set), SET_SRC (set));
    }

  /* The optimizer does not know that the call sets the function value
     registers we stored in the result block.  We avoid problems by
     claiming that all hard registers are used and clobbered at this
     point.  */
  emit_insn (gen_blockage ());

  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_call_insn (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (3,
		gen_rtx_CALL (VOIDmode,
	operand0,
	const0_rtx),
		operand1,
		operand2)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_399 PARAMS ((rtx *));
rtx
gen_split_399 (operands)
      rtx *operands ATTRIBUTE_UNUSED;
{
  rtx _val = 0;
  start_sequence ();
{
  ia64_split_call (NULL_RTX, operands[0], operands[1], operands[2],
		   operands[3], true, false);
  DONE;
}
  emit_insn (const0_rtx);
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_400 PARAMS ((rtx *));
rtx
gen_split_400 (operands)
      rtx *operands ATTRIBUTE_UNUSED;
{
  rtx _val = 0;
  start_sequence ();
{
  ia64_split_call (NULL_RTX, operands[0], operands[1], operands[2],
		   operands[3], false, false);
  DONE;
}
  emit_insn (const0_rtx);
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_401 PARAMS ((rtx *));
rtx
gen_split_401 (operands)
      rtx *operands ATTRIBUTE_UNUSED;
{
  rtx _val = 0;
  start_sequence ();
{
  ia64_split_call (operands[0], operands[1], operands[2], operands[3],
		   operands[4], true, false);
  DONE;
}
  emit_insn (const0_rtx);
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_402 PARAMS ((rtx *));
rtx
gen_split_402 (operands)
      rtx *operands ATTRIBUTE_UNUSED;
{
  rtx _val = 0;
  start_sequence ();
{
  ia64_split_call (operands[0], operands[1], operands[2], operands[3],
		   operands[4], false, false);
  DONE;
}
  emit_insn (const0_rtx);
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_403 PARAMS ((rtx *));
rtx
gen_split_403 (operands)
      rtx *operands ATTRIBUTE_UNUSED;
{
  rtx _val = 0;
  start_sequence ();
{
  ia64_split_call (NULL_RTX, operands[0], NULL_RTX, operands[1],
		   operands[2], true, true);
  DONE;
}
  emit_insn (const0_rtx);
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_tablejump (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
    operands[1] = operand1;
{
  rtx op0 = operands[0];
  rtx addr;

  /* ??? Bother -- do_tablejump is "helpful" and pulls the table
     element into a register without bothering to see whether that
     is necessary given the operand predicate.  Check for MEM just
     in case someone fixes this.  */
  if (GET_CODE (op0) == MEM)
    addr = XEXP (op0, 0);
  else
    {
      /* Otherwise, cheat and guess that the previous insn in the
	 stream was the memory load.  Grab the address from that.
	 Note we have to momentarily pop out of the sequence started
	 by the insn-emit wrapper in order to grab the last insn.  */
      rtx last, set;

      end_sequence ();
      last = get_last_insn ();
      start_sequence ();
      set = single_set (last);

      if (! rtx_equal_p (SET_DEST (set), op0)
	  || GET_CODE (SET_SRC (set)) != MEM)
	abort ();
      addr = XEXP (SET_SRC (set), 0);
      if (rtx_equal_p (addr, op0))
	abort ();
    }

  /* Jump table elements are stored pc-relative.  That is, a displacement
     from the entry to the label.  Thus to convert to an absolute address
     we add the address of the memory from which the value is loaded.  */
  operands[0] = expand_simple_binop (DImode, PLUS, op0, addr,
				     NULL_RTX, 1, OPTAB_DIRECT);
}
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_jump_insn (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	pc_rtx,
	operand0),
		gen_rtx_USE (VOIDmode,
	gen_rtx_LABEL_REF (VOIDmode,
	operand1)))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_prologue ()
{
  rtx _val = 0;
  start_sequence ();
  {
{
  ia64_expand_prologue ();
  DONE;
}
  }
  emit_insn (const1_rtx);
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_epilogue ()
{
  rtx _val = 0;
  start_sequence ();
  {
{
  ia64_expand_epilogue (0);
  DONE;
}
  }
  emit_jump_insn (gen_rtx_RETURN (VOIDmode));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_sibcall_epilogue ()
{
  rtx _val = 0;
  start_sequence ();
  {
{
  ia64_expand_epilogue (1);
  DONE;
}
  }
  emit_jump_insn (gen_rtx_RETURN (VOIDmode));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_gr_spill (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx operand3;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[4];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
operands[3] = gen_rtx_REG (DImode, AR_UNAT_REGNUM);
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
  }
  emit (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSPEC (DImode,
	gen_rtvec (2,
		operand1,
		operand2),
	10)),
		gen_rtx_CLOBBER (VOIDmode,
	operand3))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_gr_restore (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx operand3;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[4];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
operands[3] = gen_rtx_REG (DImode, AR_UNAT_REGNUM);
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
  }
  emit (gen_rtx_PARALLEL (VOIDmode,
	gen_rtvec (2,
		gen_rtx_SET (VOIDmode,
	operand0,
	gen_rtx_UNSPEC (DImode,
	gen_rtvec (2,
		operand1,
		operand2),
	11)),
		gen_rtx_USE (VOIDmode,
	operand3))));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_trap ()
{
  return gen_rtx_TRAP_IF (VOIDmode,
	const1_rtx,
	const0_rtx);
}

rtx
gen_conditional_trap (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
    operands[1] = operand1;
{
  operands[0] = ia64_expand_compare (GET_CODE (operands[0]), VOIDmode);
}
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_TRAP_IF (VOIDmode,
	operand0,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_save_stack_nonlocal (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
    operands[1] = operand1;
{
  emit_library_call (gen_rtx_SYMBOL_REF (Pmode,
					 "__ia64_save_stack_nonlocal"),
		     0, VOIDmode, 2, XEXP (operands[0], 0), Pmode,
		     operands[1], Pmode);
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_USE (VOIDmode,
	operand0));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_nonlocal_goto (operand0, operand1, operand2, operand3)
     rtx operand0;
     rtx operand1;
     rtx operand2;
     rtx operand3;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[4];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
    operands[3] = operand3;
{
  emit_library_call (gen_rtx_SYMBOL_REF (Pmode, "__ia64_nonlocal_goto"),
		     LCT_NORETURN, VOIDmode, 3,
		     operands[1], Pmode,
		     copy_to_reg (XEXP (operands[2], 0)), Pmode,
		     operands[3], Pmode);
  emit_barrier ();
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
    operand3 = operands[3];
  }
  emit_insn (gen_rtx_USE (VOIDmode,
	operand0));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand1));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand2));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand3));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

extern rtx gen_split_414 PARAMS ((rtx *));
rtx
gen_split_414 (operands)
      rtx *operands ATTRIBUTE_UNUSED;
{
  rtx _val = 0;
  start_sequence ();
{
  ia64_reload_gp ();
  DONE;
}
  emit_insn (const0_rtx);
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_eh_epilogue (operand0, operand1, operand2)
     rtx operand0;
     rtx operand1;
     rtx operand2;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[3];
    operands[0] = operand0;
    operands[1] = operand1;
    operands[2] = operand2;
{
  rtx bsp = gen_rtx_REG (Pmode, 10);
  rtx sp = gen_rtx_REG (Pmode, 9);

  if (GET_CODE (operands[0]) != REG || REGNO (operands[0]) != 10)
    {
      emit_move_insn (bsp, operands[0]);
      operands[0] = bsp;
    }
  if (GET_CODE (operands[2]) != REG || REGNO (operands[2]) != 9)
    {
      emit_move_insn (sp, operands[2]);
      operands[2] = sp;
    }
  emit_insn (gen_rtx_USE (VOIDmode, sp));
  emit_insn (gen_rtx_USE (VOIDmode, bsp));

  cfun->machine->ia64_eh_epilogue_sp = sp;
  cfun->machine->ia64_eh_epilogue_bsp = bsp;
}
    operand0 = operands[0];
    operand1 = operands[1];
    operand2 = operands[2];
  }
  emit_insn (gen_rtx_USE (VOIDmode,
	operand0));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand1));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand2));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_restore_stack_nonlocal (operand0, operand1)
     rtx operand0;
     rtx operand1;
{
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[2];
    operands[0] = operand0;
    operands[1] = operand1;
{
  emit_library_call (gen_rtx_SYMBOL_REF (Pmode,
					 "__ia64_restore_stack_nonlocal"),
		     0, VOIDmode, 1,
		     copy_to_reg (XEXP (operands[1], 0)), Pmode);
  DONE;
}
    operand0 = operands[0];
    operand1 = operands[1];
  }
  emit_insn (gen_rtx_USE (VOIDmode,
	operand0));
  emit_insn (gen_rtx_USE (VOIDmode,
	operand1));
  _val = get_insns ();
  end_sequence ();
  return _val;
}

rtx
gen_mf ()
{
  rtx operand0;
  rtx _val = 0;
  start_sequence ();
  {
    rtx operands[1];
{
  operands[0] = gen_rtx_MEM (BLKmode, gen_rtx_SCRATCH (DImode));
  MEM_VOLATILE_P (operands[0]) = 1;
}
    operand0 = operands[0];
  }
  emit_insn (gen_rtx_SET (VOIDmode,
	gen_rtx_MEM (BLKmode,
	operand0),
	gen_rtx_UNSPEC (BLKmode,
	gen_rtvec (1,
		gen_rtx_MEM (BLKmode,
	operand0)),
	18)));
  _val = get_insns ();
  end_sequence ();
  return _val;
}



void
add_clobbers (pattern, insn_code_number)
     rtx pattern ATTRIBUTE_UNUSED;
     int insn_code_number;
{
  switch (insn_code_number)
    {
    case 228:
      XVECEXP (pattern, 0, 1) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (DImode));
      XVECEXP (pattern, 0, 2) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (DImode));
      break;

    case 227:
    case 226:
      XVECEXP (pattern, 0, 2) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (DImode));
      XVECEXP (pattern, 0, 3) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (DImode));
      break;

    case 184:
      XVECEXP (pattern, 0, 1) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode));
      XVECEXP (pattern, 0, 2) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode));
      XVECEXP (pattern, 0, 3) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode));
      XVECEXP (pattern, 0, 4) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode));
      XVECEXP (pattern, 0, 5) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (BImode));
      break;

    case 150:
      XVECEXP (pattern, 0, 1) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode));
      XVECEXP (pattern, 0, 2) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (DFmode));
      XVECEXP (pattern, 0, 3) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (BImode));
      break;

    case 185:
    case 128:
    case 127:
    case 114:
      XVECEXP (pattern, 0, 1) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode));
      XVECEXP (pattern, 0, 2) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode));
      XVECEXP (pattern, 0, 3) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (BImode));
      break;

    case 149:
    case 113:
      XVECEXP (pattern, 0, 1) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode));
      XVECEXP (pattern, 0, 2) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode));
      XVECEXP (pattern, 0, 3) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (TFmode));
      XVECEXP (pattern, 0, 4) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (BImode));
      break;

    case 65:
      XVECEXP (pattern, 0, 1) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (BImode));
      break;

    case 108:
    case 107:
    case 24:
      XVECEXP (pattern, 0, 1) = gen_rtx_CLOBBER (VOIDmode,
	gen_rtx_SCRATCH (DImode));
      break;

    default:
      abort ();
    }
}


int
added_clobbers_hard_reg_p (insn_code_number)
     int insn_code_number;
{
  switch (insn_code_number)
    {
    case 228:
    case 227:
    case 226:
    case 184:
    case 150:
    case 185:
    case 128:
    case 127:
    case 114:
    case 149:
    case 113:
    case 65:
    case 108:
    case 107:
    case 24:
      return 0;

    default:
      abort ();
    }
}
