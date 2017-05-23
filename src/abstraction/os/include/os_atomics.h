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

#ifndef PA_ATOMICS_H
#define PA_ATOMICS_H

#include <stddef.h>
#include <limits.h>

#include "os_if.h"
#include "os_defs.h"
#include "os_abstract.h"
#include "os_inline.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* Note: os_atomics_inlines.c overrules PA_HAVE_INLINE, PA_INLINE and
   PA_ATOMICS_OMIT_FUNCTIONS */

#if ! PA_HAVE_INLINE && ! defined PA_ATOMICS_OMIT_FUNCTIONS
#define PA_ATOMICS_OMIT_FUNCTIONS 1
#endif

#if ! PA_ATOMIC_SUPPORT && (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >= 40100
#include "os_atomics_gcc.h"
#endif

#if ! PA_ATOMIC_SUPPORT && defined _WIN32
/* Windows.h causes HUGE problems when included too early, primarily
   because you can't include only a subset and later include the rest
*/
#undef PA_HAVE_INLINE
#undef PA_INLINE
#define PA_INLINE
#include "os_atomics_win32.h"
#endif

#if ! PA_ATOMIC_SUPPORT && defined __sun
#include "os_atomics_solaris.h"
#endif

#if ! PA_ATOMIC_SUPPORT && defined __INTEGRITY
#include "os_atomics_integrity.h"
#endif

#if ! PA_ATOMIC_SUPPORT && defined _VX_CPU_FAMILY
#include "os_atomics_vxworks.h"
#endif

#if ! PA_ATOMIC_SUPPORT && defined __GNUC__ && defined __i386
#include "os_atomics_gcc_x86.h"
#endif

#if ! PA_ATOMIC_SUPPORT &&                                              \
  ((defined __GNUC__ && defined __ppc) ||                               \
   (defined __vxworks && defined __PPC__))
/* VxWorks uses GCC but removed the __GNUC__ macro ... */
#include "os_atomics_gcc_ppc.h"
#endif

#if ! PA_ATOMIC_SUPPORT && defined __GNUC__ && defined __sparc__
#include "os_atomics_gcc_sparc.h"
#endif

#if ! PA_ATOMIC_SUPPORT && defined __GNUC__ && defined __arm__
#include "os_atomics_gcc_arm.h"
#endif

#if ! PA_ATOMIC_SUPPORT
#error "No support for atomic operations on this platform"
#endif

#if defined OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#if ! PA_HAVE_INLINE

/* LD, ST */
OS_API os_uint32 pa_ld32 (const volatile pa_uint32_t *x);
#if PA_ATOMIC64_SUPPORT
OS_API os_uint64 pa_ld64 (const volatile pa_uint64_t *x);
#endif
OS_API os_address pa_ldptr (const volatile pa_uintptr_t *x);
OS_API void *pa_ldvoidp (const volatile pa_voidp_t *x);
OS_API void pa_st32 (volatile pa_uint32_t *x, os_uint32 v);
#if PA_ATOMIC64_SUPPORT
OS_API void pa_st64 (volatile pa_uint64_t *x, os_uint64 v);
#endif
OS_API void pa_stptr (volatile pa_uintptr_t *x, os_address v);
OS_API void pa_stvoidp (volatile pa_voidp_t *x, void *v);
/* INC */
OS_API void pa_inc32 (volatile pa_uint32_t *x);
#if PA_ATOMIC64_SUPPORT
OS_API void pa_inc64 (volatile pa_uint64_t *x);
#endif
OS_API void pa_incptr (volatile pa_uintptr_t *x);
OS_API os_uint32 pa_inc32_nv (volatile pa_uint32_t *x);
#if PA_ATOMIC64_SUPPORT
OS_API os_uint64 pa_inc64_nv (volatile pa_uint64_t *x);
#endif
OS_API os_address pa_incptr_nv (volatile pa_uintptr_t *x);
/* DEC */
OS_API void pa_dec32 (volatile pa_uint32_t *x);
#if PA_ATOMIC64_SUPPORT
OS_API void pa_dec64 (volatile pa_uint64_t *x);
#endif
OS_API void pa_decptr (volatile pa_uintptr_t *x);
OS_API os_uint32 pa_dec32_nv (volatile pa_uint32_t *x);
#if PA_ATOMIC64_SUPPORT
OS_API os_uint64 pa_dec64_nv (volatile pa_uint64_t *x);
#endif
OS_API os_address pa_decptr_nv (volatile pa_uintptr_t *x);
/* ADD */
OS_API void pa_add32 (volatile pa_uint32_t *x, os_uint32 v);
#if PA_ATOMIC64_SUPPORT
OS_API void pa_add64 (volatile pa_uint64_t *x, os_uint64 v);
#endif
OS_API void pa_addptr (volatile pa_uintptr_t *x, os_address v);
OS_API void pa_addvoidp (volatile pa_voidp_t *x, ptrdiff_t v);
OS_API os_uint32 pa_add32_nv (volatile pa_uint32_t *x, os_uint32 v);
#if PA_ATOMIC64_SUPPORT
OS_API os_uint64 pa_add64_nv (volatile pa_uint64_t *x, os_uint64 v);
#endif
OS_API os_address pa_addptr_nv (volatile pa_uintptr_t *x, os_address v);
OS_API void *pa_addvoidp_nv (volatile pa_voidp_t *x, ptrdiff_t v);
/* SUB */
OS_API void pa_sub32 (volatile pa_uint32_t *x, os_uint32 v);
#if PA_ATOMIC64_SUPPORT
OS_API void pa_sub64 (volatile pa_uint64_t *x, os_uint64 v);
#endif
OS_API void pa_subptr (volatile pa_uintptr_t *x, os_address v);
OS_API void pa_subvoidp (volatile pa_voidp_t *x, ptrdiff_t v);
OS_API os_uint32 pa_sub32_nv (volatile pa_uint32_t *x, os_uint32 v);
#if PA_ATOMIC64_SUPPORT
OS_API os_uint64 pa_sub64_nv (volatile pa_uint64_t *x, os_uint64 v);
#endif
OS_API os_address pa_subptr_nv (volatile pa_uintptr_t *x, os_address v);
OS_API void *pa_subvoidp_nv (volatile pa_voidp_t *x, ptrdiff_t v);
/* AND */
OS_API void pa_and32 (volatile pa_uint32_t *x, os_uint32 v);
#if PA_ATOMIC64_SUPPORT
OS_API void pa_and64 (volatile pa_uint64_t *x, os_uint64 v);
#endif
OS_API void pa_andptr (volatile pa_uintptr_t *x, os_address v);
OS_API os_uint32 pa_and32_ov (volatile pa_uint32_t *x, os_uint32 v);
#if PA_ATOMIC64_SUPPORT
OS_API os_uint64 pa_and64_ov (volatile pa_uint64_t *x, os_uint64 v);
#endif
OS_API os_address pa_andptr_ov (volatile pa_uintptr_t *x, os_address v);
OS_API os_uint32 pa_and32_nv (volatile pa_uint32_t *x, os_uint32 v);
#if PA_ATOMIC64_SUPPORT
OS_API os_uint64 pa_and64_nv (volatile pa_uint64_t *x, os_uint64 v);
#endif
OS_API os_address pa_andptr_nv (volatile pa_uintptr_t *x, os_address v);
/* OR */
OS_API void pa_or32 (volatile pa_uint32_t *x, os_uint32 v);
#if PA_ATOMIC64_SUPPORT
OS_API void pa_or64 (volatile pa_uint64_t *x, os_uint64 v);
#endif
OS_API void pa_orptr (volatile pa_uintptr_t *x, os_address v);
OS_API os_uint32 pa_or32_ov (volatile pa_uint32_t *x, os_uint32 v);
#if PA_ATOMIC64_SUPPORT
OS_API os_uint64 pa_or64_ov (volatile pa_uint64_t *x, os_uint64 v);
#endif
OS_API os_address pa_orptr_ov (volatile pa_uintptr_t *x, os_address v);
OS_API os_uint32 pa_or32_nv (volatile pa_uint32_t *x, os_uint32 v);
#if PA_ATOMIC64_SUPPORT
OS_API os_uint64 pa_or64_nv (volatile pa_uint64_t *x, os_uint64 v);
#endif
OS_API os_address pa_orptr_nv (volatile pa_uintptr_t *x, os_address v);
/* CAS */
OS_API int pa_cas32 (volatile pa_uint32_t *x, os_uint32 exp, os_uint32 des);
#if PA_ATOMIC64_SUPPORT
OS_API int pa_cas64 (volatile pa_uint64_t *x, os_uint64 exp, os_uint64 des);
#endif
OS_API int pa_casptr (volatile pa_uintptr_t *x, os_address exp, os_address des);
OS_API int pa_casvoidp (volatile pa_voidp_t *x, void *exp, void *des);
/* FENCES */
OS_API void pa_fence (void);
OS_API void pa_fence_acq (void);
OS_API void pa_fence_rel (void);

#endif /* PA_HAVE_INLINE */

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* PA_ATOMICS_H */
