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

#if (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >= 40100

/* I don't quite know how to check whether 64-bit operations are
   supported, but my guess is that the size of a pointer is a fairly
   good indication.  Define PA_ATOMIC64_SUPPORT beforehand to override
   this.  (Obviously, I'm also assuming that os_address is an unsigned
   integer exactly as wide as a pointer, even though it may be
   wider.) */
#ifndef PA_ATOMIC64_SUPPORT
#ifdef PA__64BIT
#define PA_ATOMIC64_SUPPORT 1
#else
#define PA_ATOMIC64_SUPPORT 0
#endif
#endif

#if ! PA_ATOMICS_OMIT_FUNCTIONS

/* Eliminate C warnings */

#if ! defined (__cplusplus)
PA_INLINE os_uint32 pa_ld32 (const volatile pa_uint32_t *x);
PA_INLINE os_address pa_ldptr (const volatile pa_uintptr_t *x);
PA_INLINE void *pa_ldvoidp (const volatile pa_voidp_t *x);
PA_INLINE void pa_st32 (volatile pa_uint32_t *x, os_uint32 v);
PA_INLINE void pa_stptr (volatile pa_uintptr_t *x, os_address v);
PA_INLINE void pa_stvoidp (volatile pa_voidp_t *x, void *v);
PA_INLINE void pa_inc32 (volatile pa_uint32_t *x);
PA_INLINE void pa_incptr (volatile pa_uintptr_t *x);
PA_INLINE os_uint32 pa_inc32_nv (volatile pa_uint32_t *x);
PA_INLINE os_address pa_incptr_nv (volatile pa_uintptr_t *x);
PA_INLINE void pa_dec32 (volatile pa_uint32_t *x);
PA_INLINE void pa_decptr (volatile pa_uintptr_t *x);
PA_INLINE os_uint32 pa_dec32_nv (volatile pa_uint32_t *x);
PA_INLINE os_address pa_decptr_nv (volatile pa_uintptr_t *x);
PA_INLINE void pa_add32 (volatile pa_uint32_t *x, os_uint32 v);
PA_INLINE void pa_addptr (volatile pa_uintptr_t *x, os_address v);
PA_INLINE void pa_addvoidp (volatile pa_voidp_t *x, ptrdiff_t v);
PA_INLINE os_uint32 pa_add32_nv (volatile pa_uint32_t *x, os_uint32 v);
PA_INLINE os_address pa_addptr_nv (volatile pa_uintptr_t *x, os_address v);
PA_INLINE void *pa_addvoidp_nv (volatile pa_voidp_t *x, ptrdiff_t v);
PA_INLINE void pa_sub32 (volatile pa_uint32_t *x, os_uint32 v);
PA_INLINE void pa_subptr (volatile pa_uintptr_t *x, os_address v);
PA_INLINE void pa_subvoidp (volatile pa_voidp_t *x, ptrdiff_t v);
PA_INLINE os_uint32 pa_sub32_nv (volatile pa_uint32_t *x, os_uint32 v);
PA_INLINE os_address pa_subptr_nv (volatile pa_uintptr_t *x, os_address v);
PA_INLINE void *pa_subvoidp_nv (volatile pa_voidp_t *x, ptrdiff_t v);
PA_INLINE void pa_and32 (volatile pa_uint32_t *x, os_uint32 v);
PA_INLINE void pa_andptr (volatile pa_uintptr_t *x, os_address v);
PA_INLINE os_uint32 pa_and32_ov (volatile pa_uint32_t *x, os_uint32 v);
PA_INLINE os_address pa_andptr_ov (volatile pa_uintptr_t *x, os_address v);
PA_INLINE os_uint32 pa_and32_nv (volatile pa_uint32_t *x, os_uint32 v);
PA_INLINE os_address pa_andptr_nv (volatile pa_uintptr_t *x, os_address v);
PA_INLINE void pa_or32 (volatile pa_uint32_t *x, os_uint32 v);
PA_INLINE void pa_orptr (volatile pa_uintptr_t *x, os_address v);
PA_INLINE os_uint32 pa_or32_ov (volatile pa_uint32_t *x, os_uint32 v);
PA_INLINE os_address pa_orptr_ov (volatile pa_uintptr_t *x, os_address v);
PA_INLINE os_uint32 pa_or32_nv (volatile pa_uint32_t *x, os_uint32 v);
PA_INLINE os_address pa_orptr_nv (volatile pa_uintptr_t *x, os_address v);
PA_INLINE int pa_cas32 (volatile pa_uint32_t *x, os_uint32 exp, os_uint32 des);
PA_INLINE int pa_casptr (volatile pa_uintptr_t *x, os_address exp, os_address des);
PA_INLINE int pa_casvoidp (volatile pa_voidp_t *x, void *exp, void *des);
PA_INLINE void pa_fence (void);
PA_INLINE void pa_fence_acq (void);
PA_INLINE void pa_fence_rel (void);
#if PA_ATOMIC64_SUPPORT
PA_INLINE os_uint64 pa_ld64 (const volatile pa_uint64_t *x);
PA_INLINE void pa_st64 (volatile pa_uint64_t *x, os_uint64 v);
PA_INLINE void pa_inc64 (volatile pa_uint64_t *x);
PA_INLINE os_uint64 pa_inc64_nv (volatile pa_uint64_t *x);
PA_INLINE void pa_dec64 (volatile pa_uint64_t *x);
PA_INLINE os_uint64 pa_dec64_nv (volatile pa_uint64_t *x);
PA_INLINE void pa_add64 (volatile pa_uint64_t *x, os_uint64 v);
PA_INLINE os_uint64 pa_add64_nv (volatile pa_uint64_t *x, os_uint64 v);
PA_INLINE void pa_sub64 (volatile pa_uint64_t *x, os_uint64 v);
PA_INLINE os_uint64 pa_sub64_nv (volatile pa_uint64_t *x, os_uint64 v);
PA_INLINE void pa_and64 (volatile pa_uint64_t *x, os_uint64 v);
PA_INLINE os_uint64 pa_and64_ov (volatile pa_uint64_t *x, os_uint64 v);
PA_INLINE os_uint64 pa_and64_nv (volatile pa_uint64_t *x, os_uint64 v);
PA_INLINE void pa_or64 (volatile pa_uint64_t *x, os_uint64 v);
PA_INLINE os_uint64 pa_or64_ov (volatile pa_uint64_t *x, os_uint64 v);
PA_INLINE os_uint64 pa_or64_nv (volatile pa_uint64_t *x, os_uint64 v);
PA_INLINE int pa_cas64 (volatile pa_uint64_t *x, os_uint64 exp, os_uint64 des);
#endif
#endif

/* LD, ST */

PA_INLINE os_uint32 pa_ld32 (const volatile pa_uint32_t *x) { return x->v; }
#if PA_ATOMIC64_SUPPORT
PA_INLINE os_uint64 pa_ld64 (const volatile pa_uint64_t *x) { return x->v; }
#endif
PA_INLINE os_address pa_ldptr (const volatile pa_uintptr_t *x) { return x->v; }
PA_INLINE void *pa_ldvoidp (const volatile pa_voidp_t *x) { return (void *) pa_ldptr (x); }

PA_INLINE void pa_st32 (volatile pa_uint32_t *x, os_uint32 v) { x->v = v; }
#if PA_ATOMIC64_SUPPORT
PA_INLINE void pa_st64 (volatile pa_uint64_t *x, os_uint64 v) { x->v = v; }
#endif
PA_INLINE void pa_stptr (volatile pa_uintptr_t *x, os_address v) { x->v = v; }
PA_INLINE void pa_stvoidp (volatile pa_voidp_t *x, void *v) { pa_stptr (x, (os_address) v); }

/* INC */

PA_INLINE void pa_inc32 (volatile pa_uint32_t *x) {
  __sync_fetch_and_add (&x->v, 1);
}
#if PA_ATOMIC64_SUPPORT
PA_INLINE void pa_inc64 (volatile pa_uint64_t *x) {
  __sync_fetch_and_add (&x->v, 1);
}
#endif
PA_INLINE void pa_incptr (volatile pa_uintptr_t *x) {
  (void)__sync_fetch_and_add (&x->v, 1);
}
PA_INLINE os_uint32 pa_inc32_nv (volatile pa_uint32_t *x) {
  return __sync_add_and_fetch (&x->v, 1);
}
#if PA_ATOMIC64_SUPPORT
PA_INLINE os_uint64 pa_inc64_nv (volatile pa_uint64_t *x) {
  return __sync_add_and_fetch (&x->v, 1);
}
#endif
PA_INLINE os_address pa_incptr_nv (volatile pa_uintptr_t *x) {
  return __sync_add_and_fetch (&x->v, 1);
}

/* DEC */

PA_INLINE void pa_dec32 (volatile pa_uint32_t *x) {
  __sync_fetch_and_sub (&x->v, 1);
}
#if PA_ATOMIC64_SUPPORT
PA_INLINE void pa_dec64 (volatile pa_uint64_t *x) {
  __sync_fetch_and_sub (&x->v, 1);
}
#endif
PA_INLINE void pa_decptr (volatile pa_uintptr_t *x) {
  (void)__sync_fetch_and_sub (&x->v, 1);
}
PA_INLINE os_uint32 pa_dec32_nv (volatile pa_uint32_t *x) {
  return __sync_sub_and_fetch (&x->v, 1);
}
#if PA_ATOMIC64_SUPPORT
PA_INLINE os_uint64 pa_dec64_nv (volatile pa_uint64_t *x) {
  return __sync_sub_and_fetch (&x->v, 1);
}
#endif
PA_INLINE os_address pa_decptr_nv (volatile pa_uintptr_t *x) {
  return __sync_sub_and_fetch (&x->v, 1);
}

/* ADD */

PA_INLINE void pa_add32 (volatile pa_uint32_t *x, os_uint32 v) {
  __sync_fetch_and_add (&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
PA_INLINE void pa_add64 (volatile pa_uint64_t *x, os_uint64 v) {
  __sync_fetch_and_add (&x->v, v);
}
#endif
PA_INLINE void pa_addptr (volatile pa_uintptr_t *x, os_address v) {
  (void)__sync_fetch_and_add (&x->v, v);
}
PA_INLINE void pa_addvoidp (volatile pa_voidp_t *x, ptrdiff_t v) {
  pa_addptr ((volatile pa_uintptr_t *) x, (os_address) v);
}
PA_INLINE os_uint32 pa_add32_nv (volatile pa_uint32_t *x, os_uint32 v) {
  return __sync_add_and_fetch (&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
PA_INLINE os_uint64 pa_add64_nv (volatile pa_uint64_t *x, os_uint64 v) {
  return __sync_add_and_fetch (&x->v, v);
}
#endif
PA_INLINE os_address pa_addptr_nv (volatile pa_uintptr_t *x, os_address v) {
  return __sync_add_and_fetch (&x->v, v);
}
PA_INLINE void *pa_addvoidp_nv (volatile pa_voidp_t *x, ptrdiff_t v) {
  return (void *) pa_addptr_nv ((volatile pa_uintptr_t *) x, (os_address) v);
}

/* SUB */

PA_INLINE void pa_sub32 (volatile pa_uint32_t *x, os_uint32 v) {
  __sync_fetch_and_sub (&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
PA_INLINE void pa_sub64 (volatile pa_uint64_t *x, os_uint64 v) {
  __sync_fetch_and_sub (&x->v, v);
}
#endif
PA_INLINE void pa_subptr (volatile pa_uintptr_t *x, os_address v) {
  (void)__sync_fetch_and_sub (&x->v, v);
}
PA_INLINE void pa_subvoidp (volatile pa_voidp_t *x, ptrdiff_t v) {
  pa_subptr ((volatile pa_uintptr_t *) x, (os_address) v);
}
PA_INLINE os_uint32 pa_sub32_nv (volatile pa_uint32_t *x, os_uint32 v) {
  return __sync_sub_and_fetch (&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
PA_INLINE os_uint64 pa_sub64_nv (volatile pa_uint64_t *x, os_uint64 v) {
  return __sync_sub_and_fetch (&x->v, v);
}
#endif
PA_INLINE os_address pa_subptr_nv (volatile pa_uintptr_t *x, os_address v) {
  return __sync_sub_and_fetch (&x->v, v);
}
PA_INLINE void *pa_subvoidp_nv (volatile pa_voidp_t *x, ptrdiff_t v) {
  return (void *) pa_subptr_nv ((volatile pa_uintptr_t *) x, (os_address) v);
}

/* AND */

PA_INLINE void pa_and32 (volatile pa_uint32_t *x, os_uint32 v) {
  __sync_fetch_and_and (&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
PA_INLINE void pa_and64 (volatile pa_uint64_t *x, os_uint64 v) {
  __sync_fetch_and_and (&x->v, v);
}
#endif
PA_INLINE void pa_andptr (volatile pa_uintptr_t *x, os_address v) {
  (void)__sync_fetch_and_and (&x->v, v);
}
PA_INLINE os_uint32 pa_and32_ov (volatile pa_uint32_t *x, os_uint32 v) {
  return __sync_fetch_and_and (&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
PA_INLINE os_uint64 pa_and64_ov (volatile pa_uint64_t *x, os_uint64 v) {
  return __sync_fetch_and_and (&x->v, v);
}
#endif
PA_INLINE os_address pa_andptr_ov (volatile pa_uintptr_t *x, os_address v) {
  return __sync_fetch_and_and (&x->v, v);
}
PA_INLINE os_uint32 pa_and32_nv (volatile pa_uint32_t *x, os_uint32 v) {
  return __sync_and_and_fetch (&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
PA_INLINE os_uint64 pa_and64_nv (volatile pa_uint64_t *x, os_uint64 v) {
  return __sync_and_and_fetch (&x->v, v);
}
#endif
PA_INLINE os_address pa_andptr_nv (volatile pa_uintptr_t *x, os_address v) {
  return __sync_and_and_fetch (&x->v, v);
}

/* OR */

PA_INLINE void pa_or32 (volatile pa_uint32_t *x, os_uint32 v) {
  __sync_fetch_and_or (&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
PA_INLINE void pa_or64 (volatile pa_uint64_t *x, os_uint64 v) {
  __sync_fetch_and_or (&x->v, v);
}
#endif
PA_INLINE void pa_orptr (volatile pa_uintptr_t *x, os_address v) {
  (void)__sync_fetch_and_or (&x->v, v);
}
PA_INLINE os_uint32 pa_or32_ov (volatile pa_uint32_t *x, os_uint32 v) {
  return __sync_fetch_and_or (&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
PA_INLINE os_uint64 pa_or64_ov (volatile pa_uint64_t *x, os_uint64 v) {
  return __sync_fetch_and_or (&x->v, v);
}
#endif
PA_INLINE os_address pa_orptr_ov (volatile pa_uintptr_t *x, os_address v) {
  return __sync_fetch_and_or (&x->v, v);
}
PA_INLINE os_uint32 pa_or32_nv (volatile pa_uint32_t *x, os_uint32 v) {
  return __sync_or_and_fetch (&x->v, v);
}
#if PA_ATOMIC64_SUPPORT
PA_INLINE os_uint64 pa_or64_nv (volatile pa_uint64_t *x, os_uint64 v) {
  return __sync_or_and_fetch (&x->v, v);
}
#endif
PA_INLINE os_address pa_orptr_nv (volatile pa_uintptr_t *x, os_address v) {
  return __sync_or_and_fetch (&x->v, v);
}

/* CAS */

PA_INLINE int pa_cas32 (volatile pa_uint32_t *x, os_uint32 exp, os_uint32 des) {
  return __sync_bool_compare_and_swap (&x->v, exp, des);
}
#if PA_ATOMIC64_SUPPORT
PA_INLINE int pa_cas64 (volatile pa_uint64_t *x, os_uint64 exp, os_uint64 des) {
  return __sync_bool_compare_and_swap (&x->v, exp, des);
}
#endif
PA_INLINE int pa_casptr (volatile pa_uintptr_t *x, os_address exp, os_address des) {
  return __sync_bool_compare_and_swap (&x->v, exp, des);
}
PA_INLINE int pa_casvoidp (volatile pa_voidp_t *x, void *exp, void *des) {
  return pa_casptr (x, (os_address) exp, (os_address) des);
}

/* FENCES */

PA_INLINE void pa_fence (void) {
  __sync_synchronize ();
}
PA_INLINE void pa_fence_acq (void) {
  pa_fence ();
}
PA_INLINE void pa_fence_rel (void) {
  pa_fence ();
}

#endif /* not omit functions */

#define PA_ATOMIC_SUPPORT 1
#endif
