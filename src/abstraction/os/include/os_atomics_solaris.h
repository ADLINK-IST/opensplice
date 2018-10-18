/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef OSPL_NO_SOLARIS_ATOMICS

#include <atomic.h>

/* Solaris on SPARC uses the TSO memory model, so barriers aren't
   needed for ordering w.r.t. atomic operations. */

#define PA_ATOMIC64_SUPPORT 1

#if ! PA_ATOMICS_OMIT_FUNCTIONS

/* LD, ST */

PA_INLINE os_uint32 pa_ld32 (const volatile pa_uint32_t *x) { return x->v; }
PA_INLINE os_uint64 pa_ld64 (const volatile pa_uint64_t *x) { return x->v; }
PA_INLINE os_address pa_ldptr (const volatile pa_uintptr_t *x) { return x->v; }
PA_INLINE void *pa_ldvoidp (const volatile pa_voidp_t *x) { return (void *) pa_ldptr (x); }

PA_INLINE void pa_st32 (volatile pa_uint32_t *x, os_uint32 v) { x->v = v; }
PA_INLINE void pa_st64 (volatile pa_uint64_t *x, os_uint64 v) { x->v = v; }
PA_INLINE void pa_stptr (volatile pa_uintptr_t *x, os_address v) { x->v = v; }
PA_INLINE void pa_stvoidp (volatile pa_voidp_t *x, void *v) { pa_stptr (x, (os_address) v); }

/* INC */

PA_INLINE void pa_inc32 (volatile pa_uint32_t *x) {
  atomic_inc_32 (&x->v);
}
PA_INLINE void pa_inc64 (volatile pa_uint64_t *x) {
  atomic_inc_64 (&x->v);
}
PA_INLINE void pa_incptr (volatile pa_uintptr_t *x) {
  atomic_inc_ulong (&x->v);
}
PA_INLINE os_uint32 pa_inc32_nv (volatile pa_uint32_t *x) {
  return atomic_inc_32_nv (&x->v);
}
PA_INLINE os_uint64 pa_inc64_nv (volatile pa_uint64_t *x) {
  return atomic_inc_64_nv (&x->v);
}
PA_INLINE os_address pa_incptr_nv (volatile pa_uintptr_t *x) {
  return atomic_inc_ulong_nv (&x->v);
}

/* DEC */

PA_INLINE void pa_dec32 (volatile pa_uint32_t *x) {
  atomic_dec_32 (&x->v);
}
PA_INLINE void pa_dec64 (volatile pa_uint64_t *x) {
  atomic_dec_64 (&x->v);
}
PA_INLINE void pa_decptr (volatile pa_uintptr_t *x) {
  atomic_dec_ulong (&x->v);
}
PA_INLINE os_uint32 pa_dec32_nv (volatile pa_uint32_t *x) {
  return atomic_dec_32_nv (&x->v);
}
PA_INLINE os_uint64 pa_dec64_nv (volatile pa_uint64_t *x) {
  return atomic_dec_64_nv (&x->v);
}
PA_INLINE os_address pa_decptr_nv (volatile pa_uintptr_t *x) {
  return atomic_dec_ulong_nv (&x->v);
}

/* ADD */

PA_INLINE void pa_add32 (volatile pa_uint32_t *x, os_uint32 v) {
  atomic_add_32 (&x->v, v);
}
PA_INLINE void pa_add64 (volatile pa_uint64_t *x, os_uint64 v) {
  atomic_add_64 (&x->v, v);
}
PA_INLINE void pa_addptr (volatile pa_uintptr_t *x, os_address v) {
  atomic_add_long (&x->v, v);
}
PA_INLINE void pa_addvoidp (volatile pa_voidp_t *x, ptrdiff_t v) {
  atomic_add_ptr (&x->v, v);
}
PA_INLINE os_uint32 pa_add32_nv (volatile pa_uint32_t *x, os_uint32 v) {
  return atomic_add_32_nv (&x->v, v);
}
PA_INLINE os_uint64 pa_add64_nv (volatile pa_uint64_t *x, os_uint64 v) {
  return atomic_add_64_nv (&x->v, v);
}
PA_INLINE os_address pa_addptr_nv (volatile pa_uintptr_t *x, os_address v) {
  return atomic_add_long_nv (&x->v, v);
}
PA_INLINE void *pa_addvoidp_nv (volatile pa_voidp_t *x, ptrdiff_t v) {
  return atomic_add_ptr_nv (&x->v, v);
}

/* SUB */

PA_INLINE void pa_sub32 (volatile pa_uint32_t *x, os_uint32 v) {
  atomic_add_32 (&x->v, -v);
}
PA_INLINE void pa_sub64 (volatile pa_uint64_t *x, os_uint64 v) {
  atomic_add_64 (&x->v, -v);
}
PA_INLINE void pa_subptr (volatile pa_uintptr_t *x, os_address v) {
  atomic_add_long (&x->v, -v);
}
PA_INLINE void pa_subvoidp (volatile pa_voidp_t *x, ptrdiff_t v) {
  atomic_add_ptr (&x->v, -v);
}
PA_INLINE os_uint32 pa_sub32_nv (volatile pa_uint32_t *x, os_uint32 v) {
  return atomic_add_32_nv (&x->v, -v);
}
PA_INLINE os_uint64 pa_sub64_nv (volatile pa_uint64_t *x, os_uint64 v) {
  return atomic_add_64_nv (&x->v, -v);
}
PA_INLINE os_address pa_subptr_nv (volatile pa_uintptr_t *x, os_address v) {
  return atomic_add_long_nv (&x->v, -v);
}
PA_INLINE void *pa_subvoidp_nv (volatile pa_voidp_t *x, ptrdiff_t v) {
  return atomic_add_ptr_nv (&x->v, -v);
}

/* AND */

PA_INLINE void pa_and32 (volatile pa_uint32_t *x, os_uint32 v) {
  atomic_and_32 (&x->v, v);
}
PA_INLINE void pa_and64 (volatile pa_uint64_t *x, os_uint64 v) {
  atomic_and_64 (&x->v, v);
}
PA_INLINE void pa_andptr (volatile pa_uintptr_t *x, os_address v) {
  atomic_and_ulong (&x->v, v);
}
PA_INLINE os_uint32 pa_and32_ov (volatile pa_uint32_t *x, os_uint32 v) {
  os_uint32 oldval, newval;
  do { oldval = x->v; newval = oldval & v; } while (atomic_cas_32 (&x->v, oldval, newval) != oldval);
  return oldval;
}
PA_INLINE os_uint64 pa_and64_ov (volatile pa_uint64_t *x, os_uint64 v) {
  os_uint64 oldval, newval;
  do { oldval = x->v; newval = oldval & v; } while (atomic_cas_64 (&x->v, oldval, newval) != oldval);
  return oldval;
}
PA_INLINE os_address pa_andptr_ov (volatile pa_uintptr_t *x, os_address v) {
  os_address oldval, newval;
  do { oldval = x->v; newval = oldval & v; } while (atomic_cas_ulong (&x->v, oldval, newval) != oldval);
  return oldval;
}
PA_INLINE os_uint32 pa_and32_nv (volatile pa_uint32_t *x, os_uint32 v) {
  return atomic_and_32_nv (&x->v, v);
}
PA_INLINE os_uint64 pa_and64_nv (volatile pa_uint64_t *x, os_uint64 v) {
  return atomic_and_64_nv (&x->v, v);
}
PA_INLINE os_address pa_andptr_nv (volatile pa_uintptr_t *x, os_address v) {
  return atomic_and_ulong_nv (&x->v, v);
}

/* OR */

PA_INLINE void pa_or32 (volatile pa_uint32_t *x, os_uint32 v) {
  atomic_or_32 (&x->v, v);
}
PA_INLINE void pa_or64 (volatile pa_uint64_t *x, os_uint64 v) {
  atomic_or_64 (&x->v, v);
}
PA_INLINE void pa_orptr (volatile pa_uintptr_t *x, os_address v) {
  atomic_or_ulong (&x->v, v);
}
PA_INLINE os_uint32 pa_or32_ov (volatile pa_uint32_t *x, os_uint32 v) {
  os_uint32 oldval, newval;
  do { oldval = x->v; newval = oldval | v; } while (atomic_cas_32 (&x->v, oldval, newval) != oldval);
  return oldval;
}
PA_INLINE os_uint64 pa_or64_ov (volatile pa_uint64_t *x, os_uint64 v) {
  os_uint64 oldval, newval;
  do { oldval = x->v; newval = oldval | v; } while (atomic_cas_64 (&x->v, oldval, newval) != oldval);
  return oldval;
}
PA_INLINE os_address pa_orptr_ov (volatile pa_uintptr_t *x, os_address v) {
  os_address oldval, newval;
  do { oldval = x->v; newval = oldval | v; } while (atomic_cas_ulong (&x->v, oldval, newval) != oldval);
  return oldval;
}
PA_INLINE os_uint32 pa_or32_nv (volatile pa_uint32_t *x, os_uint32 v) {
  return atomic_or_32_nv (&x->v, v);
}
PA_INLINE os_uint64 pa_or64_nv (volatile pa_uint64_t *x, os_uint64 v) {
  return atomic_or_64_nv (&x->v, v);
}
PA_INLINE os_address pa_orptr_nv (volatile pa_uintptr_t *x, os_address v) {
  return atomic_or_ulong_nv (&x->v, v);
}

/* CAS */

PA_INLINE int pa_cas32 (volatile pa_uint32_t *x, os_uint32 exp, os_uint32 des) {
  return atomic_cas_32 (&x->v, exp, des) == exp;
}
PA_INLINE int pa_cas64 (volatile pa_uint64_t *x, os_uint64 exp, os_uint64 des) {
  return atomic_cas_64 (&x->v, exp, des) == exp;
}
PA_INLINE int pa_casptr (volatile pa_uintptr_t *x, os_address exp, os_address des) {
  return atomic_cas_ulong (&x->v, exp, des) == exp;
}
PA_INLINE int pa_casvoidp (volatile pa_voidp_t *x, void *exp, void *des) {
  return atomic_cas_ptr (&x->v, exp, des) == exp;
}

/* FENCES */

PA_INLINE void pa_fence (void) {
  membar_exit ();
  membar_enter ();
}
PA_INLINE void pa_fence_acq (void) {
  membar_enter ();
}
PA_INLINE void pa_fence_rel (void) {
  membar_exit ();
}

#endif /* not omit functions */

#define PA_ATOMIC_SUPPORT 1

#endif /* ! defined OSPL_NO_SOLARIS_ATOMICS */
