/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef OS_ACAS_H
#define OS_ACAS_H

PA_INLINE _Bool os_cas32 (volatile os_os_uint32 *x, os_uint32 exp, os_uint32 des)
{
    return TestAndSet((Address*)(&x->v), exp, des);
}

#if PA_ATOMIC64_SUPPORT
PA_INLINE _Bool os_cas64 (volatile os_os_uint64 *x, os_uint64 exp, os_uint64 des)
{
    return TestAndSet((Address*)(&x->v), exp, des);
}
#endif

PA_INLINE _Bool os_casptr (volatile os_uintptr_t *x, uintptr_t exp, uintptr_t des)
{
    return TestAndSet((Address*)(&x->v), exp, des);
}

PA_INLINE _Bool os_casvoidp (volatile os_voidp_t *x, void *exp, void *des)
{
    return TestAndSet((Address*)x, (uintptr_t)exp, (uintptr_t)des);
}

#endif /** OS_ACAS_H */
