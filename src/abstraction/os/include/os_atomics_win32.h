/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

/* x86 has supported 64-bit CAS for a long time, so Windows ought to
   provide all the interlocked operations for 64-bit operands on x86
   platforms, but it doesn't. */

#undef OS_API
#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#if defined PA__64BIT
#define PA_ATOMIC64_SUPPORT 1
#else
#define PA_ATOMIC64_SUPPORT 0
#endif

#if defined PA__64BIT
#define PA_ATOMIC_PTROP(name) name##64
#else
#define PA_ATOMIC_PTROP(name) name
#endif

#if ! PA_ATOMICS_OMIT_FUNCTIONS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

/* Experience is that WinCE doesn't provide these, and that neither does VS8 */
#if ! defined OS_WINCE_DEFS_H && _MSC_VER > 1400
#if defined _M_IX86 || defined _M_ARM
#define PA_INTERLOCKED_AND _InterlockedAnd
#define PA_INTERLOCKED_OR _InterlockedOr
#define PA_INTERLOCKED_AND64 _InterlockedAnd64
#define PA_INTERLOCKED_OR64 _InterlockedOr64
#else
#define PA_INTERLOCKED_AND InterlockedAnd
#define PA_INTERLOCKED_OR InterlockedOr
#define PA_INTERLOCKED_AND64 InterlockedAnd64
#define PA_INTERLOCKED_OR64 InterlockedOr64
#endif
#endif

#if PA_HAVE_INLINE
#define PA_API_INLINE PA_INLINE
#else
#define PA_API_INLINE OS_API
#endif

/* LD, ST */

PA_API_INLINE os_uint32 pa_ld32 (const volatile pa_uint32_t *x) { return x->v; }
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE os_uint64 pa_ld64 (const volatile pa_uint64_t *x) { return x->v; }
#endif
PA_API_INLINE os_address pa_ldptr (const volatile pa_uintptr_t *x) { return x->v; }
PA_API_INLINE void *pa_ldvoidp (const volatile pa_voidp_t *x) { return (void *) pa_ldptr (x); }

PA_API_INLINE void pa_st32 (volatile pa_uint32_t *x, os_uint32 v) { x->v = v; }
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE void pa_st64 (volatile pa_uint64_t *x, os_uint64 v) { x->v = v; }
#endif
PA_API_INLINE void pa_stptr (volatile pa_uintptr_t *x, os_address v) { x->v = v; }
PA_API_INLINE void pa_stvoidp (volatile pa_voidp_t *x, void *v) { pa_stptr (x, (os_address) v); }

/* CAS */

PA_API_INLINE int pa_cas32 (volatile pa_uint32_t *x, os_uint32 exp, os_uint32 des) {
  return InterlockedCompareExchange (&x->v, des, exp) == exp;
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE int pa_cas64 (volatile pa_uint64_t *x, os_uint64 exp, os_uint64 des) {
  return InterlockedCompareExchange64 (&x->v, des, exp) == exp;
}
#endif
PA_API_INLINE int pa_casptr (volatile pa_uintptr_t *x, os_address exp, os_address des) {
  return PA_ATOMIC_PTROP (InterlockedCompareExchange) (&x->v, des, exp) == exp;
}
PA_API_INLINE int pa_casvoidp (volatile pa_voidp_t *x, void *exp, void *des) {
  return pa_casptr ((volatile pa_uintptr_t *) x, (os_address) exp, (os_address) des);
}

/* INC */

PA_API_INLINE void pa_inc32 (volatile pa_uint32_t *x) {
  InterlockedIncrement (&x->v);
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE void pa_inc64 (volatile pa_uint64_t *x) {
  InterlockedIncrement64 (&x->v);
}
#endif
PA_API_INLINE void pa_incptr (volatile pa_uintptr_t *x) {
  PA_ATOMIC_PTROP (InterlockedIncrement) (&x->v);
}
PA_API_INLINE os_uint32 pa_inc32_nv (volatile pa_uint32_t *x) {
  return InterlockedIncrement (&x->v);
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE os_uint64 pa_inc64_nv (volatile pa_uint64_t *x) {
  return InterlockedIncrement64 (&x->v);
}
#endif
PA_API_INLINE os_address pa_incptr_nv (volatile pa_uintptr_t *x) {
  return PA_ATOMIC_PTROP (InterlockedIncrement) (&x->v);
}

/* DEC */

PA_API_INLINE void pa_dec32 (volatile pa_uint32_t *x) {
  InterlockedDecrement (&x->v);
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE void pa_dec64 (volatile pa_uint64_t *x) {
  InterlockedDecrement64 (&x->v);
}
#endif
PA_API_INLINE void pa_decptr (volatile pa_uintptr_t *x) {
  PA_ATOMIC_PTROP (InterlockedDecrement) (&x->v);
}
PA_API_INLINE os_uint32 pa_dec32_nv (volatile pa_uint32_t *x) {
  return InterlockedDecrement (&x->v);
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE os_uint64 pa_dec64_nv (volatile pa_uint64_t *x) {
  return InterlockedDecrement64 (&x->v);
}
#endif
PA_API_INLINE os_address pa_decptr_nv (volatile pa_uintptr_t *x) {
  return PA_ATOMIC_PTROP (InterlockedDecrement) (&x->v);
}

/* ADD */

PA_API_INLINE void pa_add32 (volatile pa_uint32_t *x, os_uint32 v) {
  InterlockedExchangeAdd (&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE void pa_add64 (volatile pa_uint64_t *x, os_uint64 v) {
  InterlockedExchangeAdd64 (&x->v, v);
}
#endif
PA_API_INLINE void pa_addptr (volatile pa_uintptr_t *x, os_address v) {
  PA_ATOMIC_PTROP (InterlockedExchangeAdd) (&x->v, v);
}
PA_API_INLINE void pa_addvoidp (volatile pa_voidp_t *x, ptrdiff_t v) {
  pa_addptr ((volatile pa_uintptr_t *) x, (os_address) v);
}
PA_API_INLINE os_uint32 pa_add32_nv (volatile pa_uint32_t *x, os_uint32 v) {
  return InterlockedExchangeAdd (&x->v, v) + v;
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE os_uint64 pa_add64_nv (volatile pa_uint64_t *x, os_uint64 v) {
  return InterlockedExchangeAdd64 (&x->v, v) + v;
}
#endif
PA_API_INLINE os_address pa_addptr_nv (volatile pa_uintptr_t *x, os_address v) {
  return PA_ATOMIC_PTROP (InterlockedExchangeAdd) (&x->v, v) + v;
}
PA_API_INLINE void *pa_addvoidp_nv (volatile pa_voidp_t *x, ptrdiff_t v) {
  return (void *) pa_addptr_nv ((volatile pa_uintptr_t *) x, (os_address) v);
}

/* SUB */

PA_API_INLINE void pa_sub32 (volatile pa_uint32_t *x, os_uint32 v) {
  InterlockedExchangeAdd (&x->v, -v);
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE void pa_sub64 (volatile pa_uint64_t *x, os_uint64 v) {
  InterlockedExchangeAdd64 (&x->v, -v);
}
#endif
PA_API_INLINE void pa_subptr (volatile pa_uintptr_t *x, os_address v) {
  PA_ATOMIC_PTROP (InterlockedExchangeAdd) (&x->v, -v);
}
PA_API_INLINE void pa_subvoidp (volatile pa_voidp_t *x, ptrdiff_t v) {
  pa_subptr ((volatile pa_uintptr_t *) x, (os_address) v);
}
PA_API_INLINE os_uint32 pa_sub32_nv (volatile pa_uint32_t *x, os_uint32 v) {
  return InterlockedExchangeAdd (&x->v, -v) - v;
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE os_uint64 pa_sub64_nv (volatile pa_uint64_t *x, os_uint64 v) {
  return InterlockedExchangeAdd64 (&x->v, -v) - v;
}
#endif
PA_API_INLINE os_address pa_subptr_nv (volatile pa_uintptr_t *x, os_address v) {
  return PA_ATOMIC_PTROP (InterlockedExchangeAdd) (&x->v, -v) - v;
}
PA_API_INLINE void *pa_subvoidp_nv (volatile pa_voidp_t *x, ptrdiff_t v) {
  return (void *) pa_subptr_nv ((volatile pa_uintptr_t *) x, (os_address) v);
}

/* AND */

#if defined PA_INTERLOCKED_AND

PA_API_INLINE void pa_and32 (volatile pa_uint32_t *x, os_uint32 v) {
  PA_INTERLOCKED_AND (&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE void pa_and64 (volatile pa_uint64_t *x, os_uint64 v) {
  InterlockedAnd64 (&x->v, v);
}
#endif
PA_API_INLINE void pa_andptr (volatile pa_uintptr_t *x, os_address v) {
  PA_ATOMIC_PTROP (PA_INTERLOCKED_AND) (&x->v, v);
}
PA_API_INLINE os_uint32 pa_and32_ov (volatile pa_uint32_t *x, os_uint32 v) {
  return PA_INTERLOCKED_AND (&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE os_uint64 pa_and64_ov (volatile pa_uint64_t *x, os_uint64 v) {
  return InterlockedAnd64 (&x->v, v);
}
#endif
PA_API_INLINE os_address pa_andptr_ov (volatile pa_uintptr_t *x, os_address v) {
  return PA_ATOMIC_PTROP (PA_INTERLOCKED_AND) (&x->v, v);
}
PA_API_INLINE os_uint32 pa_and32_nv (volatile pa_uint32_t *x, os_uint32 v) {
  return PA_INTERLOCKED_AND (&x->v, v) & v;
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE os_uint64 pa_and64_nv (volatile pa_uint64_t *x, os_uint64 v) {
  return InterlockedAnd64 (&x->v, v) & v;
}
#endif
PA_API_INLINE os_address pa_andptr_nv (volatile pa_uintptr_t *x, os_address v) {
  return PA_ATOMIC_PTROP (PA_INTERLOCKED_AND) (&x->v, v) & v;
}

#else /* synthesize via CAS */

PA_API_INLINE os_uint32 pa_and32_ov (volatile pa_uint32_t *x, os_uint32 v) {
  os_uint64 oldval, newval;
  do { oldval = x->v; newval = oldval & v; } while (!pa_cas32 (x, oldval, newval));
  return oldval;
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE os_uint64 pa_and64_ov (volatile pa_uint64_t *x, os_uint64 v) {
  os_uint64 oldval, newval;
  do { oldval = x->v; newval = oldval & v; } while (!pa_cas64 (x, oldval, newval));
  return oldval;
}
#endif
PA_API_INLINE os_address pa_andptr_ov (volatile pa_uintptr_t *x, os_address v) {
  os_address oldval, newval;
  do { oldval = x->v; newval = oldval & v; } while (!pa_casptr (x, oldval, newval));
  return oldval;
}
PA_API_INLINE os_uint32 pa_and32_nv (volatile pa_uint32_t *x, os_uint32 v) {
  os_uint32 oldval, newval;
  do { oldval = x->v; newval = oldval & v; } while (!pa_cas32 (x, oldval, newval));
  return newval;
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE os_uint64 pa_and64_nv (volatile pa_uint64_t *x, os_uint64 v) {
  os_uint64 oldval, newval;
  do { oldval = x->v; newval = oldval & v; } while (!pa_cas64 (x, oldval, newval));
  return newval;
}
#endif
PA_API_INLINE os_address pa_andptr_nv (volatile pa_uintptr_t *x, os_address v) {
  os_address oldval, newval;
  do { oldval = x->v; newval = oldval & v; } while (!pa_casptr (x, oldval, newval));
  return newval;
}
PA_API_INLINE void pa_and32 (volatile pa_uint32_t *x, os_uint32 v) {
  (void) pa_and32_nv (x, v);
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE void pa_and64 (volatile pa_uint64_t *x, os_uint64 v) {
  (void) pa_and64_nv (x, v);
}
#endif
PA_API_INLINE void pa_andptr (volatile pa_uintptr_t *x, os_address v) {
  (void) pa_andptr_nv (x, v);
}

#endif

/* OR */

#if defined PA_INTERLOCKED_OR

PA_API_INLINE void pa_or32 (volatile pa_uint32_t *x, os_uint32 v) {
  PA_INTERLOCKED_OR (&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE void pa_or64 (volatile pa_uint64_t *x, os_uint64 v) {
  InterlockedOr64 (&x->v, v);
}
#endif
PA_API_INLINE void pa_orptr (volatile pa_uintptr_t *x, os_address v) {
  PA_ATOMIC_PTROP (PA_INTERLOCKED_OR) (&x->v, v);
}
PA_API_INLINE os_uint32 pa_or32_ov (volatile pa_uint32_t *x, os_uint32 v) {
  return PA_INTERLOCKED_OR (&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE os_uint64 pa_or64_ov (volatile pa_uint64_t *x, os_uint64 v) {
  return InterlockedOr64 (&x->v, v);
}
#endif
PA_API_INLINE os_address pa_orptr_ov (volatile pa_uintptr_t *x, os_address v) {
  return PA_ATOMIC_PTROP (PA_INTERLOCKED_OR) (&x->v, v);
}
PA_API_INLINE os_uint32 pa_or32_nv (volatile pa_uint32_t *x, os_uint32 v) {
  return PA_INTERLOCKED_OR (&x->v, v) | v;
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE os_uint64 pa_or64_nv (volatile pa_uint64_t *x, os_uint64 v) {
  return InterlockedOr64 (&x->v, v) | v;
}
#endif
PA_API_INLINE os_address pa_orptr_nv (volatile pa_uintptr_t *x, os_address v) {
  return PA_ATOMIC_PTROP (PA_INTERLOCKED_OR) (&x->v, v) | v;
}

#else /* synthesize via CAS */

PA_API_INLINE os_uint32 pa_or32_ov (volatile pa_uint32_t *x, os_uint32 v) {
  os_uint32 oldval, newval;
  do { oldval = x->v; newval = oldval | v; } while (!pa_cas32 (x, oldval, newval));
  return oldval;
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE os_uint64 pa_or64_ov (volatile pa_uint64_t *x, os_uint64 v) {
  os_uint64 oldval, newval;
  do { oldval = x->v; newval = oldval | v; } while (!pa_cas64 (x, oldval, newval));
  return oldval;
}
#endif
PA_API_INLINE os_address pa_orptr_ov (volatile pa_uintptr_t *x, os_address v) {
  os_address oldval, newval;
  do { oldval = x->v; newval = oldval | v; } while (!pa_casptr (x, oldval, newval));
  return oldval;
}
PA_API_INLINE os_uint32 pa_or32_nv (volatile pa_uint32_t *x, os_uint32 v) {
  os_uint32 oldval, newval;
  do { oldval = x->v; newval = oldval | v; } while (!pa_cas32 (x, oldval, newval));
  return newval;
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE os_uint64 pa_or64_nv (volatile pa_uint64_t *x, os_uint64 v) {
  os_uint64 oldval, newval;
  do { oldval = x->v; newval = oldval | v; } while (!pa_cas64 (x, oldval, newval));
  return newval;
}
#endif
PA_API_INLINE os_address pa_orptr_nv (volatile pa_uintptr_t *x, os_address v) {
  os_address oldval, newval;
  do { oldval = x->v; newval = oldval | v; } while (!pa_casptr (x, oldval, newval));
  return newval;
}
PA_API_INLINE void pa_or32 (volatile pa_uint32_t *x, os_uint32 v) {
  (void) pa_or32_nv (x, v);
}
#if PA_ATOMIC64_SUPPORT
PA_API_INLINE void pa_or64 (volatile pa_uint64_t *x, os_uint64 v) {
  (void) pa_or64_nv (x, v);
}
#endif
PA_API_INLINE void pa_orptr (volatile pa_uintptr_t *x, os_address v) {
  (void) pa_orptr_nv (x, v);
}

#endif

/* FENCES */

PA_API_INLINE void pa_fence (void) {
  volatile LONG tmp = 0;
  InterlockedExchange (&tmp, 0);
}
PA_API_INLINE void pa_fence_acq (void) {
  pa_fence ();
}
PA_API_INLINE void pa_fence_rel (void) {
  pa_fence ();
}

#undef PA_INTERLOCKED_AND
#undef PA_INTERLOCKED_OR
#undef PA_INTERLOCKED_AND64
#undef PA_INTERLOCKED_OR64
#undef PA_API_INLINE

#endif /* not omit functions */

#undef PA_ATOMIC_PTROP
#define PA_ATOMIC_SUPPORT 1
