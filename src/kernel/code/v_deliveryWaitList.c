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
#include "v__kernel.h"
#include "v__deliveryWaitList.h"
#include "v_public.h"

#include "c_sync.h"

#include "os_report.h"
#include "os_abstract.h"

v_result
v_deliveryWaitListWait (
    v_deliveryWaitList _this,
    os_duration timeout)
{
    v_result result = V_RESULT_OK;

    assert(_this);
    assert(C_TYPECHECK(_this,v_deliveryWaitList));

    c_mutexLock(&_this->mutex);
    while (result == V_RESULT_OK && _this->readerGID != NULL) {
        /* TODO: Due to spurious wake-ups and the use of a *relative* timeout,
         * the provided timeout can be exceeded. When the hibernation work is
         * available, this should be eliminated by keeping track of the
         * monotonic clock or absolute sleeps should be supported instead.
         */
        result = v_condWait(&_this->cv, &_this->mutex, timeout);
    }
    c_mutexUnlock(&_this->mutex);
    return result;
}

v_result
v_deliveryWaitListNotify (
    v_deliveryWaitList _this,
    v_deliveryInfoTemplate msg)
{
    c_array toFree = NULL;

    assert(_this);
    assert(C_TYPECHECK(_this,v_deliveryWaitList));
    assert(msg);

    if(msg->userData.sequenceNumber == _this->sequenceNumber) {
        c_ulong size, i;
        v_gid *list;

        c_mutexLock(&_this->mutex);

        list = (v_gid *)_this->readerGID;

        size = c_arraySize(_this->readerGID);
        assert(size >= _this->waitCount);
        for (i = 0; i < size; i++) {
            if (v_gidEqual(list[i], msg->userData.readerGID)) {
                /* Set the found readerGID to zero, the waitlist can be
                 * unblocked when all expected systemIds are zero. In that case
                 * _this->waitCount will be 0.
                 */
                v_gidSetNil(list[i]);
                _this->waitCount--;
                break;
            }
        }

        if (_this->waitCount == 0) {
            toFree = _this->readerGID; /* Free the list outside the lock */
            _this->readerGID = NULL;
            c_condSignal(&_this->cv);
        }

        c_mutexUnlock(&_this->mutex);
    }

    c_free(toFree);

    return V_RESULT_OK;
}

v_result
v_deliveryWaitListIgnore (
    v_deliveryWaitList _this,
    v_gid readerGID)
{
    c_array toFree = NULL;
    c_ulong size, i;
    v_gid *list;

    assert(_this);
    assert(C_TYPECHECK(_this,v_deliveryWaitList));

    c_mutexLock(&_this->mutex);
    size = c_arraySize(_this->readerGID);
    assert(size >= _this->waitCount);
    list = (v_gid *)_this->readerGID;
    for (i = 0; i < size; i++) {
        if (v_gidEqual(list[i], readerGID)) {
            /* Set the found readerGID to zero, the waitlist can be
             * unblocked when all expected systemIds are zero. In that case
             * _this->waitCount will be 0.
             */
            v_gidSetNil(list[i]);
            _this->waitCount--;
            break;
        }
    }

    if (_this->waitCount == 0) {
        toFree = _this->readerGID; /* Free the list outside the lock */
        _this->readerGID = NULL;
        c_condSignal(&_this->cv);
    }
    c_mutexUnlock(&_this->mutex);

    c_free(toFree);

    return V_RESULT_OK;
}

C_CLASS(copyGIDsArg);
C_STRUCT(copyGIDsArg)
{
    c_array readerGID;
    c_ulong index;
};

static c_bool
copyGID (
    c_object o,
    c_voidp arg)
{
    v_deliveryPublisher p = v_deliveryPublisher(o);
    copyGIDsArg a = (copyGIDsArg)arg;
    v_gid *list;

    list = (v_gid *)a->readerGID;
    list[a->index++] = p->readerGID;
    assert(p->count > 0);

    return TRUE;
}

static c_array
copyReaderGIDsFromPublications(
    v_deliveryGuard _this /* must be locked */)
{
    /* copy system Ids from _this->publications */
    C_STRUCT(copyGIDsArg) arg;

    assert(_this);
    assert(C_TYPECHECK(_this, v_deliveryGuard));

    arg.readerGID = c_arrayNew(_this->gidType, c_count(_this->publications));
    arg.index = 0;
    c_walk(_this->publications, copyGID, &arg);

    return arg.readerGID;
}

v_deliveryWaitList
v_deliveryWaitListNew(
    v_deliveryGuard _this,
    v_message msg)
{
    v_deliveryWaitList waitlist = NULL;
    v_deliveryWaitList found;
    c_type type;

    assert(_this);
    assert(C_TYPECHECK(_this,v_deliveryGuard));

    /* lookup or create a writer specific admin. */
    type = c_subType(_this->waitlists);
    waitlist = c_new_s(type);
    c_free(type);
    if (waitlist) {
        if (c_mutexInit(c_getBase(waitlist), &waitlist->mutex) == SYNC_RESULT_SUCCESS) {
            c_condInit(c_getBase(waitlist), &waitlist->cv, &waitlist->mutex);
            waitlist->sequenceNumber = msg->sequenceNumber;
            waitlist->guard = _this;

            /* When modifying the contents of the WaitList collection in the deliveryGuard,
            * we must first acquire its lock.
            */
            c_mutexLock(&_this->mutex);
            waitlist->readerGID = copyReaderGIDsFromPublications(_this);
            waitlist->waitCount = c_arraySize(waitlist->readerGID);
            found = c_tableInsert(_this->waitlists, waitlist);
            assert(found == waitlist);
            (void)found;
            c_mutexUnlock(&_this->mutex);
        } else {
            c_free(waitlist);
            waitlist = NULL;
            OS_REPORT(OS_FATAL,
                    "v_deliveryWaitListNew",V_RESULT_INTERNAL_ERROR,
                    "Failed to initialize mutex for waitlist.");
        }
    } else {
        OS_REPORT(OS_FATAL,
                  "v_deliveryWaitListNew",V_RESULT_INTERNAL_ERROR,
                  "Failed to allocate v_deliveryWaitList object.");
    }

    return waitlist;
}

v_result
v_deliveryWaitListFree(
    v_deliveryWaitList _this)
{
    v_deliveryWaitList found;
    v_deliveryGuard guard;
    v_result result;

    assert(C_TYPECHECK(_this,v_deliveryWaitList));

    /* lookup or create a writer specific admin. */
    if (_this) {
        /* When modifying the contents of the WaitList collection in the deliveryGuard,
         * we must first acquire the lock of its guard.
         */
        guard = v_deliveryGuard(_this->guard);
        c_mutexLock(&guard->mutex);
        found = c_remove(guard->waitlists, _this, NULL, NULL);
        c_mutexUnlock(&guard->mutex);
        assert(found == _this);
        assert(c_refCount(found) == 2);
        c_free(found);
        c_free(_this);
        result = V_RESULT_OK;
    } else {
        result = V_RESULT_ILL_PARAM;
    }
    return result;
}

