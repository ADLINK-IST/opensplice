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
/** \file os/win32/code/os_cond.c
 *  \brief WIN32 condition varaibles
 *
 * Implements condition variables
 */

#include "os_if.h"
#include "os_errno.h"
#include "os_cond.h"
#include "os_report.h"
#include "os_init.h"
#include "os_heap.h"
#include "os__debug.h"
#include "os__sync.h"

#include "os_win32incs.h"
#ifndef LITE
#include "os__sharedmem.h"
#endif
#include "../common/code/os_cond_attr.c"

#ifndef NDEBUG
#define OS__COND_SIG 0x1810BEE5 /* I Ate 2 Bees */
#endif

/************ PUBLIC *****************/

/** \brief Initialize the condition variable taking the condition
 *         attributes into account
 *
 * \b os_condInit calls \b pthread_cond_init to intialize the posix condition
 * variable.
 *
 * In case the scope attribute is \b OS_SCOPE_SHARED, the posix
 * condition variable "pshared" attribute is set to \b PTHREAD_PROCESS_SHARED
 * otherwise it is set to \b PTHREAD_PROCESS_PRIVATE.
 *
 * When in single process mode, a request for a SHARED variable will
 * implictly create a PRIVATE equivalent.  This is an optimisation
 * because there is no need for "shared" multi process variables in
 * single process mode.
 */
_Check_return_
_When_(condAttr == NULL, _Pre_satisfies_(mutex->scope == OS_SCOPE_PRIVATE))
_When_(condAttr != NULL, _Pre_satisfies_(mutex->scope == condAttr->scopeAttr))
os_result
os_condInit (
    _Out_ _When_(return != os_resultSuccess, _Post_invalid_) os_cond *cond,
    _In_ os_mutex *mutex,
    _In_opt_ const os_condAttr *condAttr)
{
    os_result result = os_resultFail;

    assert(cond);
    OS_UNUSED_ARG(mutex);

    if(!condAttr){
        os_condAttr defAttr;

        os_condAttrInit(&defAttr);
        cond->scope = defAttr.scopeAttr;
    } else {
        cond->scope = condAttr->scopeAttr;
    }

    switch (cond->scope) {
        case OS_SCOPE_PRIVATE:
            os_syncCondInitialize(&cond->cond.priv);
            result = os_resultSuccess;
            break;
        case OS_SCOPE_SHARED:
            result = os_syncSharedCondInitialize(&cond->cond.sha);
            break;
    }

#ifndef NDEBUG
    cond->signature = OS__COND_SIG;
#endif
    return result;
}

/** \brief Destroy the condition variable
 *
 * \b os_condDestroy calls \b pthread_cond_destroy to destroy the
 * posix condition variable.
 */
void
os_condDestroy(
    _Inout_ _Post_invalid_ os_cond *cond)
{
    assert (cond);

#ifndef NDEBUG
    assert(cond->signature == OS__COND_SIG);
#endif
    switch (cond->scope) {
        case OS_SCOPE_PRIVATE:
            /* No delete for native conditions */
            break;
        case OS_SCOPE_SHARED:
            os_syncSharedCondDelete(&cond->cond.sha);
            break;
    }
#ifndef NDEBUG
    cond->signature = 0;
#endif
}

/** \brief Wait for the condition
 *
 * \b os_condWait calls \b pthread_cond_wait to wait
 * for the condition.
 */
void
os_condWait (
    os_cond *cond,
    os_mutex *mutex)
{
    os_result r = os_resultFail;

    assert (cond);
    assert (mutex);

#ifndef NDEBUG
    assert(cond->signature == OS__COND_SIG);
    assert(cond->scope == mutex->scope);
#endif
    switch (cond->scope) {
        case OS_SCOPE_PRIVATE:
            r = os_syncCondSleep(&cond->cond.priv, &mutex->lock.sha, INFINITE);
            break;
        case OS_SCOPE_SHARED:
            r = os_syncSharedCondSleep(&cond->cond.sha, &mutex->lock.sha, INFINITE);
            break;
    }

    if(r != os_resultSuccess) {
       /* TODO: Report failure through Application Event Log (OSPL-7672)? */
       abort();
    }
}

/** \brief Wait for the condition but return when the specified
 *         time has expired before the condition is triggered
 *
 * \b os_condTimedWait calls \b condTimedWait to
 * wait for the condition with a timeout.
 *
 * \b os_condTimedWait provides an relative time to wait. This
 * is passed with a resolution of 1ms as a relative time to
 * condTimedWait.
 */
os_result
os_condTimedWait (
    os_cond *cond,
    os_mutex *mutex,
    os_duration timeout)
{
    os_result r = os_resultFail;
    DWORD wait_time;

    assert(cond);
    assert(mutex);
    assert(OS_DURATION_ISPOSITIVE(timeout));

#ifndef NDEBUG
    assert(cond->signature == OS__COND_SIG);
    assert(cond->scope == mutex->scope);
#endif
    if (timeout < 0) {
        wait_time = 0;
    } else {
        wait_time = (DWORD)(timeout/1000000);
    }

    switch (cond->scope) {
        case OS_SCOPE_PRIVATE:
            r = os_syncCondSleep(&cond->cond.priv, &mutex->lock.priv, wait_time);
            break;
        case OS_SCOPE_SHARED:
            r = os_syncSharedCondSleep(&cond->cond.sha, &mutex->lock.sha, wait_time);
            break;
    }

    if(r != os_resultSuccess && r != os_resultTimeout) {
        /* TODO: Report failure through Application Event Log (OSPL-7672)? */
        abort();
    }

    return r;
}

/** \brief Signal the condition and wakeup one thread waiting
 *         for the condition
 *
 * \b os_condSignal calls \b pthread_cond_signal to signal
 * the condition.
 */
void
os_condSignal (
    os_cond *cond)
{
    assert(cond);

#ifndef NDEBUG
    assert(cond->signature == OS__COND_SIG);
#endif

    switch (cond->scope) {
        case OS_SCOPE_PRIVATE:
            os_syncCondWake(&cond->cond.priv);
            break;
        case OS_SCOPE_SHARED:
            os_syncSharedCondWake(&cond->cond.sha);
            break;
    }
}

/** \brief Signal the condition and wakeup all thread waiting
 *         for the condition
 *
 * \b os_condBroadcast calls \b pthread_cond_broadcast to broadcast
 * the condition.
 */
void
os_condBroadcast (
    os_cond *cond)
{
    assert(cond);

#ifndef NDEBUG
    assert(cond->signature == OS__COND_SIG);
#endif

    switch (cond->scope) {
        case OS_SCOPE_PRIVATE:
            os_syncCondWakeAll(&cond->cond.priv);
            break;
        case OS_SCOPE_SHARED:
            os_syncSharedCondWakeAll(&cond->cond.sha);
            break;
    }
}
