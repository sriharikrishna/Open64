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


/* si.h
 ************************************
 *
 *  Description:
 *
 *      Access to generated hardware specific scheduling information.  The
 *      data structures and functions declared in this file are actually
 *      defined in files generated using targ_info/generate/si_gen.c.  The
 *      generated C file is compiled into a DSO and this header provides
 *      access to the information contained in the DSO.
 *
 *  Resource checking:
 *
 *      Cycle level
 *
 *          type SI_RRW
 *
 *              A Resource Reservation table Word.  This is the type of an
 *              entry in a resource reservation table.  The client is
 *              responsible for keeping a resource reservation table the
 *              containing one SI_RRW for each cycle being scheduled.  This is
 *              also the type of a single cycle resource requirement.  The
 *              following functions are provided in order to manipulate single
 *              cycle elements of resource reservation tables and resource
 *              requiests:
 *
 *                  SI_RRW SI_RRW_Initial()
 *                      Return the value of an initalized (no reserved
 *                      resources) resource reservation entry.
 *
 *                  SI_RRW SI_RRW_Reserve( SI_RRW table, SI_RRW request )
 *                      Reserve the resource in <request> from <table> and
 *                      return the result.  IMPORTANT: The resources might not
 *                      be available, the the result must be checked (see
 *                      immediately below).
 *
 *                  bool SI_RRW_Has_Overuse( SI_RRW table_entry )
 *                      Does <table_entry> have a resource overuse?
 *
 *                  SI_RRW_Unreserve( SI_RRW table, SI_RRW requirement )
 *                      Undoes what SI_RRW_Reserve does.
 *
 *      Multi cycle resource requirements
 *
 *          type SI_RR
 *
 *              These represent a sequence of single cycle resource
 *              requirements, one for each consecutive cycle in a schedule.
 *              The following access functions are available:
 *
 *                  INT SI_RR_Length( SI_RR req )
 *                      Return the number of consecutive cycles with SI_RRW's
 *                      in <req>
 *
 *                  SI_RRW SI_RR_Cycle_RRW( SI_RR req, UINT cycle )
 *                      Return <req>'s SI_RRW for the given <cycle>
 *
 *      Resource totals:
 *
 *          Sometimes we need to know more about resources than just checking
 *          for availability.  In particular, during software pipelining, we
 *          calculate the MII of the loop by counting all the resource usages
 *          of each kind in the loop and comparing to the number available per
 *          cycle.  We'd also like to be able to print the totals in order to
 *          inform the user of critical resource bottlenecks.  The types and
 *          functions defined in this section should be used for this purpose.
 *
 *          type SI_RESOURCE_ID
 *
 *              An integer type which Represents a single type of resource.
 *              It may also be used by the client as an index into a table of
 *              counts.  The size of such a table should be the number of
 *              differetn types of resource defined for the hardware target, a
 *              value given by:
 *
 *                  INT SI_rexource_count
 *
 *              The following access functions are defined for
 *              SI_RESOURCE_IDs:
 *
 *                  const char* SI_RESOURCE_ID_Name( SI_RESOURCE_ID id )
 *                      The client supplied name of the resource.
 *
 *                  UINT SI_RESOURCE_ID_Avail_Per_Cycle(
 *                      SI_RESOURCE_ID id
 *                  )
 *                      How many of them are available per cycle.
 *
 *          type SI_RESOURCE_TOTAL
 *
 *              Represents the total number of a particular kind of resource
 *              used over all the cycles of a resource requirement.  It has
 *              the following access functions:
 *
 *                  SI_RESOURCE_ID SI_RESOURCE_TOTAL_Resource_Id(
 *                      SI_RESOURCE_TOTAL* total
 *                  )
 *                      Return the RESOURCE_ID whose usage is described by
 *                      <total>.
 *
 *                  UINT SI_RESOURCE_TOTAL_Total_Used(
 *                      SI_RESOURCE_TOTAL* total
 *                  )
 *                      Return the usage count of the RESOURCE_ID whose usage
 *                      is described by <total>.
 *
 *      Calculating resource relevance
 *
 *          Our software pipelining pruning heuristics use a notion of
 *          resource relevance.  In order to facilitate this we provide:
 *
 *              type SI_RESOURCE_ID_SET
 *
 *                  SI_RESOURCE_ID_SET SI_RESOURCE_ID_SET_Universe()
 *                      Universal set of resource ids.
 *
 *                  SI_RESOURCE_ID_SET SI_RESOURCE_ID_SET_Empty()
 *                      Empty set of resource ids.
 *
 *                  SI_RESOURCE_ID_SET
 *                  SI_RESOURCE_ID_SET_Intersection( SI_RESOURCE_ID_SET s0,
 *                                                   SI_RESOURCE_ID_SET s1 )
 *
 *                  bool SI_RESOURCE_ID_SET_Intersection_Non_Empty(
 *                      SI_RESOURCE_ID_SET s0,
 *                      SI_RESOURCE_ID_SET s1 )
 *                      
 *                      Is the intersection of <s0> and <s1> non-empty?
 *
 *                  bool SI_RESOURCE_ID_SET_Intersection4_Non_Empty(
 *                      SI_RESOURCE_ID_SET s0,
 *                      SI_RESOURCE_ID_SET s1,
 *                      SI_RESOURCE_ID_SET s2,
 *                      SI_RESOURCE_ID_SET s3 )
 *
 *                      Is the intersection of <s0>..<s3> non-empty?
 *
 *                  SI_RESOURCE_ID_SET_Complement( SI_RESOURCE_ID_SET s )
 *                      Return the complement set of <s>.
 *
 *
 *  Skewed pipes
 *
 *      Beast (and perhaps other contemplated machines features a "skewed"
 *      pipe which allows it to issue dependent instuctions in the same
 *      cycle.  This is somewhat described in si_gen.h.  The exact best way
 *      for either a compiler or a simulator to use this information is still
 *      somewhat open, but we provide access to the essential information:
 *
 *          type ISSUE_SLOT
 *
 *              Represents one of the possible issuse slots provided by the
 *              machine.  It has the following access functions:
 *
 *                  const char* SI_ISSUE_SLOT_Name( SI_ISSUE_SLOT* slot )
 *                      Returns the name supplied by the si_gen client for the
 *                      <slot>.
 *
 *                  INT SI_ISSUE_SLOT_Skew( SI_ISSUE_SLOT* slot )
 *                      Returns the skew associated with the <slot>.  This must
 *                      be added to the operand access and result available
 *                      times.
 *
 *                  INT SI_ISSUE_SLOT_Avail_Per_Cycle( SI_SCHED_INFO* slot )
 *                      How many instructions can occupy <slot> per cycle.
 *
 *              Access to all the issue slots in the machine is provided by:
 *
 *                  INT SI_ISSUE_SLOT_Count(void)
 *                      How many issue slots does the target hardware
 *                      provide.  If 0, then no issue slots were defined and
 *                      the target machine isn't "skewed" at all, i.e. don't
 *                      worry about it.
 *
 *                  SI_ISSUE_SLOT* SI_Ith_Issue_Slot( UINT i )
 *                      Return the Ith issue slot in the target architecture.
 *                      Instructions sheculed in the same cycle should be
 *                      emitted in issue slot order.
 *
 *  Impossible to schedule IIs
 *
 *      Some opcodes just cannot be scheduled in certain IIs.  For example, we
 *      cannot schedule floating point divides on beast in IIs which are small
 *      even multiples of 3.  To represent this we provide:
 *
 *          type SI_BAD_II_SET
 *
 *              with the following related functions:
 *
 *		    const INT SI_BAD_II_SET_MAX
 *			The largest possible bad II (for allocating
 *			data structures indexed by II)
 *
 *                  SI_BAD_II_SET SI_BAD_II_SET_Union( SI_BAD_II_SET s1,
 *                                                     SI_BAD_II_SET s1 )
 *                      Return the union of the given sets.
 *
 *                  bool SI_BAD_II_SET_MemberP( SI_BAD_II_SET s, INT i )
 *                      Is <i> a member of <s>?
 *
 *                  SI_BAD_II_SET SI_BAD_II_SET_Empty()
 *                      Returns the empty bad II set.
 *
 *
 *  TOPCODE relative information
 *
 *      All the scheduing information for a particular TOPCODE is accessible
 *      via TSI (Top Scheduling Information) functions:
 *
 *          const char* TSI_Name( TOP top )
 *              si_gen client supplied name for <top>'s scheduling information
 *              instruction group.
 *
 *          SI_ID TSI_Id( TOP top )
 *              Return the SI_ID of the scheduling information associated with
 *              <top>.  See below for a description of the SI_ID type.
 *
 *          INT TSI_Operand_Access_Time( TOP top, INT operand_index )
 *              Time <top> accesses it's <operand_index>'th operand.
 *
 *          INT TSI_Result_Avail_Time( TOP top,INT result_index )
 *              Time <top> makes it's <result_index>'th result available.
 *
 *          INT TSI_Load_Access_Time( TOP top )
 *              Time <top> (a load) reads it's value from memory.
 *
 *          INT TSI_Last_Issue_Cycle( TOP top )
 *              Time <top> issues its last instruction (non-zero only
 *		for simulated instructions).
 *
 *          INT TSI_Store_Avail_Time( TOP top )
 *              Time <top> (a store) makes it's result available in memory.
 *
 *          SS_RR TSI_Resource_Requirement( TOP top )
 *              Resource requirement to schedule <top>.
 *
 *          SI_BAD_II_SET TSI_II_Bad_IIs( TOP top )
 *              Returns a set indicating impossible IIs for this resource
 *              class.
 *
 *          SI_RR TSI_II_Resource_Requirement( TOP top, UINT ii )
 *              A resource requirement for scheduling a <top> in a pipelined
 *              loop with <ii> cycles.  Guaranteed to have at most <ii> cycles
 *              worth of resource requirement.  This will be NULL if
 *              ii is a member of the bad IIs set.
 *
 *          const SI_RESOURCE_ID_SET*
 *          TSI_II_Cycle_Resource_Ids_Used( SI_ID, id, INT ii )
 *              See SI_ID_II_Cycle_Resource_Ids_Used.
 *
 *          UINT TSI_Resource_Total_Vector_Size( TOP top )
 *          SI_RESOURCE_TOTAL* TSI_Resource_Total_Vector( TOP top )
 *              A vector and its size that gives the total resource usage for
 *              each SI_RESOURCE_ID for the given <top>.  There will be one
 *              entry for each resource class together with a count of the
 *              number of resource it uses.
 *
 *          bool TSI_Write_Write_Interlock( TOP top )
 *              For simulation.  Do <top> instructions interlock when they
 *              write to a register already written to but not yet available.
 *
 *  Scheduling information common to a group of TOPCODEs
 *
 *      Principally for software pipelining, it is necessary to deal with
 *      scheduling information not just by opcode, but by the underlying
 *      groups of opcodes with identical scheduling information.  To faciliate
 *      this we provide:
 *
 *          TYPE SI_ID
 *
 *              which is an integer type with the following additional access
 *              functions:
 *
 *                  INT SI_ID_Count()
 *                      Returns the number of SI_IDs
 *
 *                  const SI_RESOURCE_ID_SET*
 *                  SI_ID_II_Cycle_Resource_Ids_Used( SI_ID id, INT ii )
 *                      For the given <ii> returns a pointer to the first
 *                      element of a vector of resource id sets.  This vector
 *                      is indexed by cycle relative to issue and each cycle's
 *                      set contains just the resources used by the opcodes in
 *                      the scheduling group.  Used to compute common resoruce
 *                      usage for software pipelinings pruning heuristics.
 *                      Perhaps it is suprising, but we don't seem to know the
 *                      length of these directly.  Rather we are always able
 *                      to derive that information from the topcode relative
 *                      TSI_II_Resource_Requirement.
 *
 ************************************
 */

/*  $REVISION: $
 *  $Date: 2003-12-09 19:18:15 $
 *  $Author: eraxxon $
 *  $Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/common/targ_info/access/ti_si.h,v $
 */

#ifndef SI_INCLUDED
#define SI_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SI_RCS_ID
#define SI_RCS_ID
#ifdef _KEEP_RCS_ID
static const char SI_rcs_id[] = "$Source: /m_home/m_utkej/Argonne/cvs2svn/cvs/Open64/osprey1.0/common/targ_info/access/ti_si.h,v $ $Revision: 1.2 $";
#endif
#endif

#include <topcode.h>

typedef enum topcode TOPCODE;

/****************************************************************************
 ****************************************************************************/

typedef struct {
  mUINT64 dw[2];
} SI_BAD_II_SET;

enum { SI_BAD_II_SET_MAX=127 };

inline SI_BAD_II_SET SI_BAD_II_SET_Union( SI_BAD_II_SET s1, SI_BAD_II_SET s2 )
{
  SI_BAD_II_SET the_union;

  the_union.dw[0] = s1.dw[0] | s2.dw[0];
  the_union.dw[1] = s1.dw[1] | s2.dw[1];

  return the_union;
}

inline INT SI_BAD_II_SET_MemberP( SI_BAD_II_SET s, UINT i )
{
  UINT bitnum = i - 1;

  if ( bitnum > (UINT)SI_BAD_II_SET_MAX ) return 0;

  return (s.dw[bitnum / 64]  & (1ULL << (bitnum % 64))) != 0;
}

inline SI_BAD_II_SET SI_BAD_II_SET_Empty( void )
{
  const SI_BAD_II_SET empty_set = {{0,0}};

  return empty_set;
}

/****************************************************************************
 ****************************************************************************/

typedef UINT SI_RESOURCE_ID;

typedef const struct {
  const char* name;
  SI_RESOURCE_ID id;
  mUINT8 avail_per_cycle;
  mUINT8 word_index;
  mUINT8 bit_index;
} SI_RESOURCE;


inline const char* SI_RESOURCE_Name( SI_RESOURCE* res )
{
  return res->name;
}

inline UINT SI_RESOURCE_Id( SI_RESOURCE* res )
{
  return res->id;
}

inline UINT SI_RESOURCE_Avail_Per_Cycle( SI_RESOURCE* res )
{
  return res->avail_per_cycle;
}

inline UINT SI_RESOURCE_Word_Index( SI_RESOURCE* res )
{
  return res->word_index;
}

inline UINT SI_RESOURCE_Bit_Index( SI_RESOURCE* res )
{
  return res->bit_index;
}

extern const INT SI_resource_count;
#pragma weak SI_resource_count

extern SI_RESOURCE* const SI_resources[];
#pragma weak SI_resources

inline const char* SI_RESOURCE_ID_Name( SI_RESOURCE_ID id )
{
  return SI_RESOURCE_Name(SI_resources[id]);
}

inline UINT SI_RESOURCE_ID_Avail_Per_Cycle( SI_RESOURCE_ID id )
{
  return SI_RESOURCE_Avail_Per_Cycle(SI_resources[id]);
}

/****************************************************************************
 ****************************************************************************/

typedef mUINT64 SI_RESOURCE_ID_SET;

inline SI_RESOURCE_ID_SET SI_RESOURCE_ID_SET_Universe(void)
{
  return    (SI_RESOURCE_ID_SET)-1
	 >> (sizeof(SI_RESOURCE_ID_SET) * 8 - SI_resource_count);
}

inline SI_RESOURCE_ID_SET SI_RESOURCE_ID_SET_Empty(void)
{
  return (SI_RESOURCE_ID_SET)0;
}

inline SI_RESOURCE_ID_SET
SI_RESOURCE_ID_SET_Intersection( SI_RESOURCE_ID_SET s0,
                                 SI_RESOURCE_ID_SET s1 )
{
  return s0 & s1;
}

inline INT
SI_RESOURCE_ID_SET_Intersection_Non_Empty( SI_RESOURCE_ID_SET s0,
                                           SI_RESOURCE_ID_SET s1 )
{
  return (s0 & s1) != (SI_RESOURCE_ID_SET)0;
}

inline INT
SI_RESOURCE_ID_SET_Intersection4_Non_Empty( SI_RESOURCE_ID_SET s0,
                                            SI_RESOURCE_ID_SET s1,
                                            SI_RESOURCE_ID_SET s2,
                                            SI_RESOURCE_ID_SET s3 )
{
  return (s0 & s1 & s2 & s3) != (SI_RESOURCE_ID_SET)0;
}

inline SI_RESOURCE_ID_SET
SI_RESOURCE_ID_SET_Complement( SI_RESOURCE_ID_SET s )
{
  return (~s) & SI_RESOURCE_ID_SET_Universe();
}

/****************************************************************************
 ****************************************************************************/

/* SI_RRW -- A resource reservation word */
typedef mUINT64 SI_RRW;

extern const SI_RRW SI_RRW_initializer;
#pragma weak SI_RRW_initializer

extern const SI_RRW SI_RRW_overuse_mask;
#pragma weak SI_RRW_overuse_mask

inline SI_RRW SI_RRW_Initial(void)
{
  return SI_RRW_initializer;
}

inline SI_RRW SI_RRW_Reserve( SI_RRW table, SI_RRW requirement )
{
  return table + requirement;
}

inline SI_RRW SI_RRW_Has_Overuse( SI_RRW word_with_reservations )
{
  return (word_with_reservations & SI_RRW_overuse_mask) != 0;
}

inline SI_RRW SI_RRW_Unreserve( SI_RRW table, SI_RRW requirement )
{
  return table - requirement;
}

/****************************************************************************
 ****************************************************************************/

typedef const struct {
  const char* name;
  mINT32 skew;
  mINT32 avail_per_cycle;
} SI_ISSUE_SLOT;

inline const char* SI_ISSUE_SLOT_Name( SI_ISSUE_SLOT* slot )
{
  return slot->name;
}

inline INT SI_ISSUE_SLOT_Skew( SI_ISSUE_SLOT* slot )
{
  return slot->skew;
}

inline INT SI_ISSUE_SLOT_Avail_Per_Cycle( SI_ISSUE_SLOT* slot )
{
  return slot->avail_per_cycle;
}

extern const INT SI_issue_slot_count;
#pragma weak SI_issue_slot_count

extern SI_ISSUE_SLOT* const SI_issue_slots[];
#pragma weak SI_issue_slots

inline INT SI_ISSUE_SLOT_Count(void)
{
  return SI_issue_slot_count;
}

inline SI_ISSUE_SLOT* SI_Ith_Issue_Slot( UINT i )
{
  return SI_issue_slots[i];
}

/****************************************************************************
 ****************************************************************************/

typedef const struct {
  SI_RESOURCE* resource;
  mINT32 total_used;
} SI_RESOURCE_TOTAL;

inline SI_RESOURCE*
SI_RESOURCE_TOTAL_Resource( SI_RESOURCE_TOTAL* pair )
{
  return pair->resource;
}

inline SI_RESOURCE_ID SI_RESOURCE_TOTAL_Resource_Id( SI_RESOURCE_TOTAL* pair )
{
  return SI_RESOURCE_Id(SI_RESOURCE_TOTAL_Resource(pair));
}

inline UINT SI_RESOURCE_TOTAL_Avail_Per_Cycle(SI_RESOURCE_TOTAL* pair)
{
  return SI_RESOURCE_Avail_Per_Cycle(SI_RESOURCE_TOTAL_Resource(pair));
}

inline INT SI_RESOURCE_TOTAL_Total_Used( SI_RESOURCE_TOTAL* pair )
{
  return pair->total_used;
}

/****************************************************************************
 ****************************************************************************/

typedef const SI_RRW* SI_RR;

inline UINT SI_RR_Length( SI_RR req )
{
  return (INT) req[0];
}

inline SI_RRW SI_RR_Cycle_RRW( SI_RR req, UINT cycle )
{
  /* TODO: make this compilable with and without defs.h 
  assert(cycle <= req[0]);
  */
  return req[cycle+1];
}

/****************************************************************************
 ****************************************************************************/
typedef UINT SI_ID;

typedef const struct {
  const char* name;
  SI_ID id;
  const mUINT8 *operand_access_times;
  const mUINT8 *result_available_times;
  mINT32 load_access_time;
  mINT32 last_issue_cycle;
  mINT32 store_available_time;
  SI_RR rr;
  const SI_RESOURCE_ID_SET *resources_used;
  mUINT32 ii_info_size;
  const SI_RR *ii_rr;
  const SI_RESOURCE_ID_SET * const *ii_resources_used;
  SI_BAD_II_SET bad_iis;
  mINT32 valid_issue_slot_count;
  SI_ISSUE_SLOT * const *valid_issue_slots;
  mINT32 resource_total_vector_size;
  SI_RESOURCE_TOTAL *resource_total_vector;
  mUINT8 write_write_interlock;
} SI;

extern SI* const SI_top_si[];
#pragma weak SI_top_si

inline const char* TSI_Name( TOP top )
{
  return SI_top_si[(INT) top]->name;
}

inline SI_ID TSI_Id( TOP top )
{
  return SI_top_si[top]->id;
}

inline INT
TSI_Operand_Access_Time( TOP top, INT operand_index )
{
  return SI_top_si[(INT) top]->operand_access_times[operand_index];
}

inline INT
TSI_Result_Available_Time( TOP top, INT result_index )
{
  return SI_top_si[(INT) top]->result_available_times[result_index];
}

inline INT
TSI_Load_Access_Time( TOP top )
{
  return SI_top_si[(INT) top]->load_access_time;
}

inline INT
TSI_Last_Issue_Cycle( TOP top )
{
  return SI_top_si[(INT) top]->last_issue_cycle;
}

inline INT
TSI_Store_Available_Time( TOP top )
{
  return SI_top_si[(INT) top]->store_available_time;
}

inline SI_RR TSI_Resource_Requirement( TOP top )
{
  return SI_top_si[(INT) top]->rr;
}

inline SI_BAD_II_SET TSI_Bad_IIs( TOP top )
{
  return SI_top_si[(INT) top]->bad_iis;
}

inline SI_RR TSI_II_Resource_Requirement( TOP top, INT ii )
{
  SI* const info = SI_top_si[(INT) top];

  if ( ii > info->ii_info_size ) return info->rr;

  return info->ii_rr[ii - 1];
}

inline const SI_RESOURCE_ID_SET*
TSI_II_Cycle_Resource_Ids_Used( TOP opcode, INT ii )
{
  SI* const info = SI_top_si[(INT)opcode];
  if ( ii > info->ii_info_size ) return info->resources_used;

  return info->ii_resources_used[ii - 1];
}

inline UINT TSI_Valid_Issue_Slot_Count( TOP top )
{
  return SI_top_si[(INT) top]->valid_issue_slot_count;
}

inline SI_ISSUE_SLOT* TSI_Valid_Issue_Slots( TOP top, UINT i )
{
  return SI_top_si[(INT) top]->valid_issue_slots[i];
}

inline UINT TSI_Resource_Total_Vector_Size( TOP top )
{
  return SI_top_si[(INT) top]->resource_total_vector_size;
}

inline SI_RESOURCE_TOTAL* TSI_Resource_Total_Vector( TOP top )
{
  return SI_top_si[(INT) top]->resource_total_vector;
}

inline INT TSI_Write_Write_Interlock( TOP top )
{
  return SI_top_si[(INT) top]->write_write_interlock;
}

/****************************************************************************
 ****************************************************************************/

extern const INT SI_ID_count;
#pragma weak SI_ID_count

inline INT SI_ID_Count(void)
{
  return SI_ID_count;
}

extern SI* const SI_ID_si[];
#pragma weak SI_ID_si

inline const SI_RESOURCE_ID_SET*
SI_ID_II_Cycle_Resource_Ids_Used( SI_ID id, INT ii )
{
  SI* const info = SI_ID_si[id];
  if ( ii > info->ii_info_size ) return info->resources_used;

  return info->ii_resources_used[ii - 1];
}
  

#ifdef __cplusplus
}
#endif
#endif
