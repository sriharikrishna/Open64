// Boilerplate support routines for -*- C++ -*- dynamic memory management.
// Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation

// This file is part of GNU CC.

// GNU CC is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// GNU CC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with GNU CC; see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

#include "new"

extern "C" void free (void *);

#define WEAK(x) \
  x __attribute__ ((weak)); \
  x

#ifdef L_op_vnew
WEAK(void * operator new[] (size_t sz) throw (std::bad_alloc))
{
  return ::operator new(sz);
}
#endif

#ifdef L_op_vnewnt
WEAK(void *operator new[] (size_t sz, const std::nothrow_t& nothrow) throw())
{
  return ::operator new(sz, nothrow);
}
#endif

#ifdef L_op_delete
WEAK (void operator delete (void *ptr) throw ())
{
  if (ptr)
    free (ptr);
}
#endif

#ifdef L_op_vdel
WEAK (void operator delete[] (void *ptr) throw ())
{
  ::operator delete (ptr);
}
#endif

#ifdef L_op_delnt
WEAK (void operator delete (void *ptr, const std::nothrow_t&) throw ())
{
  if (ptr)
    free (ptr);
}
#endif

#ifdef L_op_vdelnt
WEAK (void operator delete[] (void *ptr, const std::nothrow_t&) throw ())
{
  ::operator delete (ptr);
}
#endif
