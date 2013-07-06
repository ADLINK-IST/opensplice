/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "v__kernel.h"
#include "v__deliveryWaitList.h"
#include "v_public.h"

#include "c_sync.h"

#include "os_report.h"

v_result
v_deliveryWaitListWait (
    v_deliveryWaitList _this,
    v_duration timeout)
{
    v_result result = V_RESULT_OK;
    c_syncResult r;

    assert(C_TYPECHECK(_this,v_deliveryWaitList));

    if (_this->readerGID != NULL) {
        c_mutexLock(&_this->mutex);
        if(c_timeCompare(timeout, C_TIME_INFINITE) != C_EQ)
        {
            r = c_condTimedWait(&_this->cv,&_this->mutex,timeout);
        }
        else
        {
            r = c_condWait(&_this->cv,&_this->mutex);
        }
        c_mutexUnlock(&_this->mutex);

        switch (r) {
        case SYNC_RESULT_SUCCESS:
           result = V_RESULT_OK;
        break;
        case SYNC_RESULT_TIMEOUT:
           result = V_RESULT_TIMEOUT;
        break;
        case SYNC_RESULT_UNAVAILABLE:
        case SYNC_RESULT_BUSY:
        case SYNC_RESULT_INVALID:
        case SYNC_RESULT_FAIL:
        default:
            result = V_RESULT_PRECONDITION_NOT_MET;
        break;
        }
    }

    return result;
}

v_result
v_deliveryWaitListNotify (
    v_deliveryWaitList _this,
    v_deliveryInfoTemplate msg)
{
    c_ulong size, i, count;
    v_gid *list;

    assert(C_TYPECHECK(_this,v_deliveryWaitList));
    assert(msg);

    list = (v_gid *)_this->readerGID;
    if(msg->userData.sequenceNumber == _this->sequenceNumber)
    {
        count = 0;
        
        size = c_arraySize(_this->readerGID);
        for (i=0; i<size; i++) {
            if (v_gidEqual(list[i],msg->userData.readerGID)) {
                /* Set the found readerGID to zero,
                 * iThe waitlist can be unblocked when
                 * all expected systemIds are zero.
                 * In that case count will be 0.
                 */
                v_gidSetNil(list[i]);
            }
            count += v_gidSystemId(list[i]);
        }
        if (count == 0) {
            c_free(_this->readerGID);
            _this->readerGID = NULL;
            c_mutexLock(&_this->mutex);
            c_condSignal(&_this->cv);
            c_mutexUnlock(&_this->mutex);
        }
    }
    return V_RESULT_OK;
}

v_result
v_deliveryWaitListIgnore (
    v_deliveryWaitList _this,
    v_gid readerGID)
{
    c_ulong size, i, count;
    v_gid *list;

    assert(C_TYPECHECK(_this,v_deliveryWaitList));

    count = 0;
    size = c_arraySize(_this->readerGID);
    list = (v_gid *)_this->readerGID;
    for (i=0; i<size; i++) {
        if (v_gidEqual(list[i],readerGID)) {
            /* Set the found reader gid to zero,
             * iThe waitlist can be unblocked when
             * all expected systemIds are zero.
             * In that case count will be 0.
             */
            v_gidSetNil(list[i]);
        }
        count += v_gidSystemId(list[i]);
    }
    if (count == 0) {
        c_free(_this->readerGID);
        _this->readerGID = NULL;
        c_mutexLock(&_this->mutex);
        c_condSignal(&_this->cv);
        c_mutexUnlock(&_this->mutex);
    }
    return V_RESULT_OK;
}

C_CLASS(copySystemIdsArg);
C_STRUCT(copySystemIdsArg)
{
    c_array readerGID;
    c_long index;
};

static c_bool
copySystemIds (
    c_object o,
    c_voidp arg)
{
    v_deliveryPublisher p = v_deliveryPublisher(o);
    copySystemIdsArg a = (copySystemIdsArg)arg;
    v_gid *list;

    list = (v_gid *)a->readerGID;
    list[a->index++] = p->readerGID;
    assert(p->count > 0);
    return TRUE;
}

static c_array
copyReaderGIDsFromPublications(
    v_deliveryGuard _this)
{
    /* copy system Ids from _this->publications */
    C_STRUCT(copySystemIdsArg) arg;
    c_long size;

    if (_this->publications) {
        size = c_count(_this->publications);
    } else {
        size = 0;
    }
    if (size > 0) {
        arg.readerGID = c_arrayNew(_this->gidType,size);
        arg.index = 0;
        c_walk(_this->publications, copySystemIds, &arg);
    } else {
        arg.readerGID = NULL;
    }

    return arg.readerGID;
}

v_deliveryWaitList
v_deliveryWaitListNew(
    v_deliveryGuard _this,
    v_message msg)
{
    v_deliveryWaitList waitlist = NULL;
    v_deliveryWaitList found;
    c_syncResult sr;
    c_type type;

    assert(C_TYPECHECK(_this,v_deliveryGuard));

    if (_this) {
        /* lookup or create a writer specific admin.
         */
        type = c_subType(_this->waitlists);
        waitlist = c_new(type);
        c_free(type);
        if (waitlist) {
            waitlist->sequenceNumber = msg->sequenceNumber;
            waitlist->readerGID = copyReaderGIDsFromPublications(_this);
            waitlist->guard = _this;
            c_mutexInit(&waitlist->mutex, SHARED_MUTEX);
            c_condInit(&waitlist->cv, &waitlist->mutex, SHARED_COND);
            /* When modifying the contents of the WaitList collection in the deliveryGuard,
             * we must first acquire its lock. */
            sr = c_mutexLock(&_this->mutex);
            if (sr == SYNC_RESULT_SUCCESS)
            {
                found = c_tableInsert(_this->waitlists, waitlist);
                sr = c_mutexUnlock(&_this->mutex);
            }
            if (sr == SYNC_RESULT_SUCCESS) {
                if (found != waitlist) {
                    /* This should not happen! */
                    OS_REPORT(OS_ERROR,
                              "v_deliveryWaitListNew",0,
                              "detected inconsistent waitlist admin.");
                    c_free(waitlist);
                    waitlist = NULL;
                }
            } else {
                waitlist = NULL;
                OS_REPORT_2(OS_ERROR,
                          "v_deliveryWaitListNew",0,
                          "Failed to claim/release mutex; _this = 0x%x, msg = 0x%x.",
                          _this, msg);
            }
        } else {
            OS_REPORT(OS_ERROR,
                      "v_deliveryWaitListNew",0,
                      "Failed to allocate v_deliveryWaitList object.");
            assert(FALSE);
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "v_deliveryWaitListNew",0,
                  "Operation failed: illegal parameter (_this == NULL).");
    }
    return waitlist;
}

v_result
v_deliveryWaitListFree(
    v_deliveryWaitList _this)
{
    v_deliveryWaitList found;
    v_deliveryGuard guard;
    c_syncResult sr;
    v_result result;

    assert(C_TYPECHECK(_this,v_deliveryWaitList));

    /* lookup or create a writer specific admin.
     */
    if (_this) {
        /* When modifying the contents of the WaitList collection in the deliveryGuard,
         * we must first acquire the lock of its guard. */
        guard = v_deliveryGuard(_this->guard);
        sr = c_mutexLock(&guard->mutex);
        if (sr == SYNC_RESULT_SUCCESS)
        {
            found = c_remove(guard->waitlists, _this, NULL, NULL);
            sr = c_mutexUnlock(&guard->mutex);
        }
        if (sr == SYNC_RESULT_SUCCESS) {
            assert(found == _this);
            assert(c_refCount(found) == 2);
            c_free(found);
            c_free(_this);
            result = V_RESULT_OK;
        } else {
            result = V_RESULT_INTERNAL_ERROR;
            OS_REPORT_1(OS_ERROR,
                      "v_deliveryWaitListFree",0,
                      "Failed to claim/release mutex; _this = 0x%x.", _this);
        }
    } else {
        result = V_RESULT_ILL_PARAM;
    }
    return result;
}

