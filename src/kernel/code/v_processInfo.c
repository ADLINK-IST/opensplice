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

#include "v__processInfo.h"
#include "v__threadInfo.h"
#include "os_atomics.h"

_Check_return_
_Ret_maybenull_
v_processInfo
v_processInfoNew(
    _In_ v_kernel kernel,
    _In_ os_procId processId)
{
    v_processInfo processInfo;
    c_type type;

    assert(kernel);

    type = c_resolve(c_getBase(kernel), "kernelModuleI::v_processInfo");
    assert(type);

    processInfo = c_new_s(type);
    c_free(type);

    if(!processInfo) {
        goto err_new;
    }

    if(c_mutexInit(c_getBase(kernel), &processInfo->lock) != SYNC_RESULT_SUCCESS) {
        goto err_mutexInit;
    }

    processInfo->processId = processId;
    pa_st32 (&processInfo->protectCount, 0);
    pa_st32 (&processInfo->waitCount, 0);
    pa_st32 (&processInfo->blockedCount, 0);

    type = c_resolve(c_getBase(kernel), "kernelModuleI::v_threadInfo");
    assert(type);

    processInfo->threads = c_tableNew(type, "threadId");
    c_free(type);

    return processInfo;

err_mutexInit:
    c_free(processInfo);
err_new:
    return NULL;
}

/* c_action wrapper */
static c_bool
v__processInfoWakeThread (
    c_object o,
    c_voidp arg)
{
    OS_UNUSED_ARG(arg);

    assert(o);

    v_threadInfoWake(v_threadInfo(o));

    return TRUE;
}

void
v_processInfoWakeThreads(
    _Inout_ v_processInfo _this)
{
    c_mutexLock(&_this->lock);
    (void) c_walk((c_collection)_this->threads, &v__processInfoWakeThread, NULL);
    c_mutexUnlock(&_this->lock);
}

/* c_action wrapper */
static c_bool
v__processInfoReportThread (
    c_object o,
    c_voidp arg)
{
    OS_UNUSED_ARG(arg);

    assert(o);

    v_threadInfoReport(v_threadInfo(o));

    return TRUE;
}

void
v_processInfoReportThreads(
    _Inout_ v_processInfo _this)
{
    c_mutexLock(&_this->lock);
    (void) c_walk((c_collection)_this->threads, &v__processInfoReportThread, NULL);
    c_mutexUnlock(&_this->lock);
}

_Ret_notnull_
v_threadInfo
v_processInfoGetThreadInfo (
    _Inout_ v_processInfo _this,
    _In_ c_ulonglong tid)
{
    c_value tidval;
    v_threadInfo tinfo;

    tidval = c_ulonglongValue(tid);

    c_mutexLock(&_this->lock);
    assert(c_tableNofKeys(_this->threads) == 1);
    tinfo = c_tableFind(_this->threads, &tidval);

    if(tinfo == NULL) {
        tinfo = v_threadInfoNew(c_getBase(c_object(_this)), _this->serial, tid);
        (void) c_tableInsert(_this->threads, tinfo);
    }
    c_mutexUnlock(&_this->lock);
    c_free(tinfo); /* The threadInfo shouldn't be returned to the caller with a refcount */
    return tinfo;
}

void
v_processInfoFree(
    _Inout_opt_ _Post_invalid_ v_processInfo _this)
{
    if(_this) {
        v_threadInfo ti = NULL;

        assert(os_procIdSelf() == _this->processId);

        while((ti = c_take(_this->threads)) != NULL) {
            v_threadInfoFree(ti);
        }

        c_free(_this);
    }
}
