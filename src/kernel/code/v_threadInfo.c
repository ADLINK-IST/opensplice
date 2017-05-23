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

#include "v__threadInfo.h"
#include "os_thread.h"
#include "os_atomics.h"
#include "os_heap.h"
#include "os_mutex.h"
#include "os_report.h"

_Check_return_
_Ret_notnull_
v_threadInfo
v_threadInfoNew(
    _In_ c_base base,
    _In_ c_ulong serial,
    _In_ c_ulonglong threadId)
{
    v_threadInfo tinfo;
    c_type tit;

    assert(base);

    tit = c_resolve(base, "kernelModuleI::v_threadInfo");
    assert(tit);

    tinfo = c_new(tit);
    c_free(tit);

    tinfo->serial = serial;
    tinfo->threadId = threadId;

    tinfo->lock = os_malloc(sizeof(os_mutex));
    os_mutexInit(tinfo->lock, NULL);

    return tinfo;
}

void
v_threadInfoWake(
    _In_ v_threadInfo _this)
{
    os_mutexLock(_this->lock);
    if(_this->mtx) {
        assert(_this->cnd);

        if(c_mutexTryLock(_this->mtx) == SYNC_RESULT_SUCCESS) {
            os_condBroadcast(_this->cnd);
            c_mutexUnlock(_this->mtx);
        }
    }
    os_mutexUnlock(_this->lock);
}

void
v_threadInfoReport(
    _In_ v_threadInfo _this)
{
    os_mutexLock(_this->lock);
    if(_this->protectCount) {
        assert((_this->serial & V_KERNEL_THREAD_FLAG_DOMAINID) == (_this->flags & V_KERNEL_THREAD_FLAG_DOMAINID));
        if(_this->flags & V_KERNEL_THREAD_FLAG_WAITING) {
            if(_this->mtx) {
                assert(_this->cnd);
                OS_REPORT(OS_INFO, "v_threadInfoReport", 0, "Thread %"PA_PRIx64" %sstill waiting (waitCount = %u, domainId = %u) on condition %p protected by mutex %p.",
                                    _this->threadId,
                                    _this->flags & V_KERNEL_THREAD_FLAG_SERVICETHREAD ? "(service thread) " : "",
                                    _this->protectCount,
                                    _this->flags & V_KERNEL_THREAD_FLAG_DOMAINID,
                                    _this->cnd,
                                    _this->mtx);
            } else {
                OS_REPORT(OS_INFO, "v_threadInfoReport", 0, "Thread %"PA_PRIx64" %ssleeping in kernel (waitCount = %u, domainId = %u).",
                                    _this->threadId,
                                    _this->flags & V_KERNEL_THREAD_FLAG_SERVICETHREAD ? "(service thread) " : "",
                                    _this->protectCount,
                                    _this->flags & V_KERNEL_THREAD_FLAG_DOMAINID);
            }
        } else {
            OS_REPORT(OS_INFO, "v_threadInfoReport", 0, "Thread %"PA_PRIx64" %sstill active in kernel (protectCount = %u, domainId = %u).",
                                _this->threadId,
                                _this->flags & V_KERNEL_THREAD_FLAG_SERVICETHREAD ? "(service thread) " : "",
                                _this->protectCount,
                                _this->flags & V_KERNEL_THREAD_FLAG_DOMAINID);
        }
    }
    os_mutexUnlock(_this->lock);
}

void
v_threadInfoSetWaitInfo(
    _Inout_ v_threadInfo _this,
    _In_opt_ c_cond *cnd,
    _In_opt_ c_mutex *mtx)
{
    os_mutexLock(_this->lock);
    _this->cnd = cnd;
    _this->mtx = mtx;
    os_mutexUnlock(_this->lock);
}

c_mutex *
v_threadInfoGetAndClearWaitInfo(
    _Inout_ v_threadInfo _this)
{
    c_mutex *mtx;
    os_mutexLock(_this->lock);
    mtx = _this->mtx;
    _this->cnd = _this->mtx = NULL;
    os_mutexUnlock(_this->lock);

    return mtx;
}

void
v_threadInfoSetFlags(
    _Inout_opt_ v_threadInfo _this,
    _In_ c_ulong flags)
{
    if(_this) {
        os_mutexLock(_this->lock);
        _this->flags = flags;
        os_mutexUnlock(_this->lock);
    }
}

void
v_threadInfoFree(
    _Inout_opt_ _Post_invalid_ v_threadInfo _this)
{
    if(_this) {
        os_mutexDestroy(_this->lock);
        os_free(_this->lock);

        c_free(_this);
    }
}

