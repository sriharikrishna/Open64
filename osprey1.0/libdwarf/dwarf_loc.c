/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2.1 of the GNU Lesser General Public License 
  as published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU Lesser General Public 
  License along with this program; if not, write the Free Software 
  Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307, 
  USA.

  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/



#include "config.h"
#include "dwarf_incl.h"
#include "dwarf_loc.h"


/*
    Given a Dwarf_Block that represents a location expression,
    this function returns a pointer to a Dwarf_Locdesc struct 
    that has its ld_cents field set to the number of location 
    operators in the block, and its ld_s field pointing to a 
    contiguous block of Dwarf_Loc structs.  However, the 
    ld_lopc and ld_hipc values are uninitialized.  Returns 
    NULL on error.  This function assumes that the length of 
    the block is greater than 0.  Zero length location expressions 
    to represent variables that have been optimized away are 
    handled in the calling function.
*/
static Dwarf_Locdesc *
_dwarf_get_locdesc (
    Dwarf_Debug		dbg,
    Dwarf_Block		*loc_block,
    Dwarf_Error		*error
)
{
	/* 
	    Size of the block containing 
	    the location expression. 
	*/
    Dwarf_Unsigned	loc_len;

	/* 
	    Sweeps the block containing 
	    the location expression.
	*/
    Dwarf_Small		*loc_ptr;

	/* Current location operator. */
    Dwarf_Small		atom;

	/* Offset of current operator from start of block. */
    Dwarf_Unsigned	offset;

	/* Operands of current location operator. */
    Dwarf_Unsigned	operand1, operand2;

	/* Used to chain the Dwarf_Loc_Chain_s structs. */
    Dwarf_Loc_Chain	curr_loc, prev_loc, head_loc = NULL;

	/* Count of the number of location operators. */
    Dwarf_Unsigned	op_count;

        /* Contiguous block of Dwarf_Loc's for Dwarf_Locdesc. */
    Dwarf_Loc		*block_loc;

	/* Dwarf_Locdesc pointer to be returned. */
    Dwarf_Locdesc	*locdesc;

    Dwarf_Word		leb128_length;
    Dwarf_Unsigned	i;

    /* ***** BEGIN CODE ***** */

    loc_len = loc_block->bl_len;
    loc_ptr = loc_block->bl_data;

    offset = 0; op_count = 0;
    while (offset < loc_len) {

	operand1 = 0;
	operand2 = 0;
	op_count++;

	atom = *(Dwarf_Small *)loc_ptr;
	loc_ptr ++;
	offset++;

	curr_loc = (Dwarf_Loc_Chain)_dwarf_get_alloc(dbg, DW_DLA_LOC_CHAIN, 1);
	if (curr_loc == NULL)
	    {_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL); return(NULL);}
	curr_loc->lc_offset = offset;
	curr_loc->lc_atom = atom;
	switch (atom) {
	    
	    case DW_OP_reg0 :
	    case DW_OP_reg1 :
	    case DW_OP_reg2 :
	    case DW_OP_reg3 :
	    case DW_OP_reg4 :
	    case DW_OP_reg5 :
	    case DW_OP_reg6 :
	    case DW_OP_reg7 :
	    case DW_OP_reg8 :
	    case DW_OP_reg9 :
	    case DW_OP_reg10 :
	    case DW_OP_reg11 :
	    case DW_OP_reg12 :
	    case DW_OP_reg13 :
	    case DW_OP_reg14 :
	    case DW_OP_reg15 :
	    case DW_OP_reg16 :
	    case DW_OP_reg17 :
	    case DW_OP_reg18 :
	    case DW_OP_reg19 :
	    case DW_OP_reg20 :
	    case DW_OP_reg21 :
	    case DW_OP_reg22 :
	    case DW_OP_reg23 :
	    case DW_OP_reg24 :
	    case DW_OP_reg25 :
	    case DW_OP_reg26 :
	    case DW_OP_reg27 :
	    case DW_OP_reg28 :
	    case DW_OP_reg29 :
	    case DW_OP_reg30 :
	    case DW_OP_reg31 :
				break;

	    case DW_OP_regx :
				operand1 = _dwarf_decode_u_leb128(loc_ptr, 
				    &leb128_length);
				loc_ptr = loc_ptr + leb128_length;
				offset = offset + leb128_length;
				break;
			
	    case DW_OP_lit0 :
	    case DW_OP_lit1 :
	    case DW_OP_lit2 :
	    case DW_OP_lit3 :
	    case DW_OP_lit4 :
	    case DW_OP_lit5 :
	    case DW_OP_lit6 :
	    case DW_OP_lit7 :
	    case DW_OP_lit8 :
	    case DW_OP_lit9 :
	    case DW_OP_lit10 :
	    case DW_OP_lit11 :
	    case DW_OP_lit12 :
	    case DW_OP_lit13 :
	    case DW_OP_lit14 :
	    case DW_OP_lit15 :
	    case DW_OP_lit16 :
	    case DW_OP_lit17 :
	    case DW_OP_lit18 :
	    case DW_OP_lit19 :
	    case DW_OP_lit20 :
	    case DW_OP_lit21 :
	    case DW_OP_lit22 :
	    case DW_OP_lit23 :
	    case DW_OP_lit24 :
	    case DW_OP_lit25 :
	    case DW_OP_lit26 :
	    case DW_OP_lit27 :
	    case DW_OP_lit28 :
	    case DW_OP_lit29 :
	    case DW_OP_lit30 :
	    case DW_OP_lit31 :
				operand1 = atom - DW_OP_lit0;
				break;

	    case DW_OP_addr :
				READ_UNALIGNED(dbg,operand1,Dwarf_Unsigned,
				    loc_ptr,
				    dbg->de_pointer_size);
				loc_ptr += dbg->de_pointer_size;
				offset += dbg->de_pointer_size;
				break;

	    case DW_OP_const1u :
				operand1 = *(Dwarf_Small *)loc_ptr;
				loc_ptr = loc_ptr + 1;
				offset = offset + 1;
				break;
			
	    case DW_OP_const1s :
				operand1 = *(Dwarf_Sbyte *)loc_ptr;
				loc_ptr = loc_ptr + 1;
				offset = offset + 1;
				break;

	    case DW_OP_const2u :
				READ_UNALIGNED(dbg,operand1,Dwarf_Unsigned,
				 loc_ptr, 2);
				loc_ptr = loc_ptr + 2;
				offset = offset + 2;
				break;

	    case DW_OP_const2s :
				READ_UNALIGNED(dbg,operand1, Dwarf_Unsigned,
					loc_ptr, 2);
				loc_ptr = loc_ptr + 2;
				offset = offset + 2;
				break;

	    case DW_OP_const4u :
				READ_UNALIGNED(dbg,operand1,Dwarf_Unsigned,
					 loc_ptr, 4);
				loc_ptr = loc_ptr + 4;
				offset = offset + 4;
				break;

	    case DW_OP_const4s :
				READ_UNALIGNED(dbg,operand1,Dwarf_Unsigned,
					 loc_ptr, 4);
				loc_ptr = loc_ptr + 4;
				offset = offset + 4;
				break;

	    case DW_OP_const8u :
				READ_UNALIGNED(dbg,operand1,Dwarf_Unsigned,
					 loc_ptr, 8);
				loc_ptr = loc_ptr + 8;
				offset = offset + 8;
				break;

	    case DW_OP_const8s :
				READ_UNALIGNED(dbg,operand1, Dwarf_Unsigned,
					loc_ptr ,8);
				loc_ptr = loc_ptr + 8;
				offset = offset + 8;
				break;

	    case DW_OP_constu :
				operand1 = _dwarf_decode_u_leb128(loc_ptr, 
				    &leb128_length);
				loc_ptr = loc_ptr + leb128_length;
				offset = offset + leb128_length;
				break;

	    case DW_OP_consts :
				operand1 = _dwarf_decode_s_leb128(loc_ptr, 
				    &leb128_length);
				loc_ptr = loc_ptr + leb128_length;
				offset = offset + leb128_length;
				break;

	    case DW_OP_fbreg :
				operand1 = _dwarf_decode_s_leb128(loc_ptr, 
				    &leb128_length);
				loc_ptr = loc_ptr + leb128_length;
				offset = offset + leb128_length;
				break;

	    case DW_OP_breg0 :
	    case DW_OP_breg1 :
	    case DW_OP_breg2 :
	    case DW_OP_breg3 :
	    case DW_OP_breg4 :
	    case DW_OP_breg5 :
	    case DW_OP_breg6 :
	    case DW_OP_breg7 :
	    case DW_OP_breg8 :
	    case DW_OP_breg9 :
	    case DW_OP_breg10 :
	    case DW_OP_breg11 :
	    case DW_OP_breg12 :
	    case DW_OP_breg13 :
	    case DW_OP_breg14 :
	    case DW_OP_breg15 :
	    case DW_OP_breg16 :
	    case DW_OP_breg17 :
	    case DW_OP_breg18 :
	    case DW_OP_breg19 :
	    case DW_OP_breg20 :
	    case DW_OP_breg21 :
	    case DW_OP_breg22 :
	    case DW_OP_breg23 :
	    case DW_OP_breg24 :
	    case DW_OP_breg25 :
	    case DW_OP_breg26 :
	    case DW_OP_breg27 :
	    case DW_OP_breg28 :
	    case DW_OP_breg29 :
	    case DW_OP_breg30 :
	    case DW_OP_breg31 :
				operand1 = _dwarf_decode_s_leb128(loc_ptr,
				    &leb128_length);
				loc_ptr = loc_ptr + leb128_length;
				offset = offset + leb128_length;
				break;

	    case DW_OP_bregx :
				operand1 = _dwarf_decode_s_leb128(loc_ptr,
				    &leb128_length);
				loc_ptr = loc_ptr + leb128_length;
				offset = offset + leb128_length;

				operand2 = _dwarf_decode_u_leb128(loc_ptr,
				    &leb128_length);
				loc_ptr = loc_ptr + leb128_length;
				offset = offset + leb128_length;
				break;
	    
	    case DW_OP_dup :
	    case DW_OP_drop :
				break;

	    case DW_OP_pick :
				operand1 = *(Dwarf_Small *)loc_ptr;
				loc_ptr = loc_ptr + 1;
				offset = offset + 1;
				break;

	    case DW_OP_over :
	    case DW_OP_swap :
	    case DW_OP_rot :
	    case DW_OP_deref :
				break;

	    case DW_OP_deref_size :
				operand1 = *(Dwarf_Small *)loc_ptr;
				loc_ptr = loc_ptr + 1;
				offset = offset + 1;
				break;

	    case DW_OP_xderef :
				break;

	    case DW_OP_xderef_size :
				operand1 = *(Dwarf_Small *)loc_ptr;
				loc_ptr = loc_ptr + 1;
				offset = offset + 1;
				break;

	    case DW_OP_abs :
	    case DW_OP_and :
	    case DW_OP_div :
	    case DW_OP_minus :
	    case DW_OP_mod :
	    case DW_OP_mul :
	    case DW_OP_neg :
	    case DW_OP_not :
	    case DW_OP_or :
	    case DW_OP_plus :
				break;

	    case DW_OP_plus_uconst :
				operand1 = _dwarf_decode_u_leb128(loc_ptr,
				    &leb128_length);
				loc_ptr = loc_ptr + leb128_length;
				offset = offset + leb128_length;
				break;

	    case DW_OP_shl :
	    case DW_OP_shr :
	    case DW_OP_shra :
	    case DW_OP_xor :
				break;

	    case DW_OP_le :
	    case DW_OP_ge :
	    case DW_OP_eq :
	    case DW_OP_lt :
	    case DW_OP_gt :
	    case DW_OP_ne :
				break;

	    case DW_OP_skip :
	    case DW_OP_bra :
				READ_UNALIGNED(dbg,operand1,Dwarf_Unsigned,
					 loc_ptr, 2);
				loc_ptr = loc_ptr + 2;
				offset = offset + 2;
				break;

	    case DW_OP_piece :
				operand1 = _dwarf_decode_u_leb128(loc_ptr,
				    &leb128_length);
				loc_ptr = loc_ptr + leb128_length;
				offset = offset + leb128_length;
				break;

	    case DW_OP_nop :
				break;

	    default :
				_dwarf_error(dbg, error, DW_DLE_LOC_EXPR_BAD);
				return(NULL);
	}

	
	curr_loc->lc_number = operand1;
	curr_loc->lc_number2 = operand2;

	if (head_loc == NULL)
	    head_loc = prev_loc = curr_loc;
	else {
	    prev_loc->lc_next = curr_loc;
	    prev_loc = curr_loc;
	}
    }

    block_loc = 
	(Dwarf_Loc *)_dwarf_get_alloc(dbg, DW_DLA_LOC_BLOCK, op_count);
    if (block_loc == NULL)
	{_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL); return(NULL);}

    curr_loc = head_loc;
    for (i = 0; i < op_count; i++) {
        (block_loc + i)->lr_atom = curr_loc->lc_atom;
	(block_loc + i)->lr_number = curr_loc->lc_number;
	(block_loc + i)->lr_number2 = curr_loc->lc_number2;
	(block_loc + i)->lr_offset = curr_loc->lc_offset;

	prev_loc = curr_loc;
	curr_loc = curr_loc->lc_next;
	dwarf_dealloc(dbg, prev_loc, DW_DLA_LOC_CHAIN);
    }

    locdesc = (Dwarf_Locdesc *)_dwarf_get_alloc(dbg, DW_DLA_LOCDESC, 1);
    if (locdesc == NULL)
	{_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL); return(NULL);}

    locdesc->ld_cents = op_count;
    locdesc->ld_s = block_loc;

    return(locdesc);
}


/* 
	Handles only location expressions at the moment.
*/
int
dwarf_loclist (
    Dwarf_Attribute	attr,
    Dwarf_Locdesc	**llbuf,
    Dwarf_Signed        *listlen,
    Dwarf_Error		*error
)
{
	/* Dwarf_Debug of input die. */
    Dwarf_Debug		dbg;

	/* 
	    Dwarf_Attribute that describes the 
	    DW_AT_location in die, if present. 
	*/
    Dwarf_Attribute	loc_attr = attr;

	/* 
	    Pointer to Dwarf_Block that describes
	    the location expression.
	*/
    Dwarf_Block		*loc_block;

	/* A pointer to the current Dwarf_Locdesc read. */
    Dwarf_Locdesc	*locdesc;

    int blkres;

        /* ***** BEGIN CODE ***** */
    if (loc_attr == NULL) {
        _dwarf_error(NULL,error,DW_DLE_ATTR_NULL);
	return(DW_DLV_ERROR);
    }
    if (loc_attr->ar_cu_context == NULL) {
        _dwarf_error(NULL,error,DW_DLE_ATTR_NO_CU_CONTEXT);
        return(DW_DLV_ERROR);
    }

    dbg = attr->ar_cu_context->cc_dbg;
    if (dbg == NULL) {
        _dwarf_error(NULL,error,DW_DLE_ATTR_DBG_NULL);
        return(DW_DLV_ERROR);
    }

    blkres =  dwarf_formblock(loc_attr,&loc_block, error);
    if (blkres != DW_DLV_OK ) {
	return(blkres);
    }

	/* Don't know what to do for 0 length location expressions. */
    if (loc_block->bl_len != 0) {
	locdesc = _dwarf_get_locdesc(dbg, loc_block, error);
	if (locdesc == NULL) {
	    /* low level error already set: let it be passed back */
	    return(DW_DLV_ERROR);
	}

	*llbuf = locdesc;
	dwarf_dealloc(dbg, loc_block, DW_DLA_BLOCK);
	*listlen = 1;
	return(DW_DLV_OK);
    }
    /* strange situation: internal error? */
    _dwarf_error(dbg, error, DW_DLE_LOC_EXPR_BAD);
    return DW_DLV_ERROR;
}
