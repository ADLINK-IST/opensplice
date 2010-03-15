/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
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

    if (_this->systemIds != NULL) {
        c_mutexLock(&_this->mutex);
        r = c_condTimedWait(&_this->cv,&_this->mutex,timeout);
        c_mutexUnlock(&_this->mutex);
        switch (r) {
        case SYNC_RESULT_SUCCESS: result = V_RESULT_OK; break;
        case SYNC_RESULT_TIMEOUT: result = V_RESULT_TIMEOUT; break;
        case SYNC_RESULT_UNAVAILABLE:
        case SYNC_RESULT_BUSY:
        case SYNC_RESULT_INVALID:
        case SYNC_RESULT_FAIL:
        default: result = V_RESULT_PRECONDITION_NOT_MET; break;
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
    c_ulong systemId;

    assert(C_TYPECHECK(_this,v_deliveryWaitList));
    assert(msg);

    systemId = msg->userData.writerGID.systemId;

synch_printf("v_deliveryWaitListNotify: systemId          = %d\n",systemId);
synch_printf("                          sequenceNumber    = %d\n",msg->userData.sequenceNumber);
synch_printf("v_deliveryWaitListNotify: was expecting:\n");
synch_printf("                          sequenceNumber    = %d\n",_this->sequenceNumber);
    if(   msg->userData.sequenceNumber == _this->sequenceNumber
      /* && v_gidEqual(msg->userData.writerGID, _this->writerGID)
       && v_gidEqual(msg->userData.writerInstanceGID, _this->writerInstanceGID)*/)
    {
        count = 0;
        size = c_arraySize(_this->systemIds);
        for (i=0; i<size; i++) {
            if ((c_ulong)_this->systemIds[i] == systemId) {
                /* Set the found systemId to zero,
                 * iThe waitlist can be unblocked when
                 * all expected systemIds are zero.
                 * In that case count will be 0.
                 */
synch_printf("v_deliveryWaitListNotify: systemId found so reset value\n");
                _this->systemIds[i] = 0;
            }
            count += (c_ulong)_this->systemIds[i];
        }
        if (count == 0) {
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
    c_ulong systemId)
{
    c_ulong size, i, count;

    assert(C_TYPECHECK(_this,v_deliveryWaitList));

    count = 0;
    size = c_arraySize(_this->systemIds);
    for (i=0; i<size; i++) {
        if ((c_ulong)_this->systemIds[i] == systemId) {
            /* Set the found systemId to zero,
             * iThe waitlist can be unblocked when
             * all expected systemIds are zero.
             * In that case count will be 0.
             */
synch_printf("v_deliveryWaitListIgnore: systemId found so reset value\n");
            _this->systemIds[i] = 0;
        }
        count += (c_ulong)_this->systemIds[i];
    }
    if (count == 0) {
        c_mutexLock(&_this->mutex);
        c_condSignal(&_this->cv);
        c_mutexUnlock(&_this->mutex);
    }
    return V_RESULT_OK;
}

C_CLASS(copySystemIdsArg);
C_STRUCT(copySystemIdsArg)
{
    c_array systemIds;
    c_long index;
};

static c_bool
copySystemIds (
    c_object o,
    c_voidp arg)
{
    v_deliveryPublisher p = v_deliveryPublisher(o);
    copySystemIdsArg a = (copySystemIdsArg)arg;

synch_printf("v_deliveryWaitListNew: found systemId %d\n", p->systemId);
    a->systemIds[a->index++] = p->systemId;
    assert(p->count > 0);
    return TRUE;
}

static c_array
copySystemIdsFromPublications(
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
        arg.systemIds = c_arrayNew(c_long_t(c_getBase(c_object(_this))),size);
        arg.index = 0;
synch_printf("v_deliveryWaitListNew: found %d systemIds\n", size);
        c_walk(_this->publications, copySystemIds, &arg);
    } else {
synch_printf("v_deliveryWaitListNew: found NO systemIds\n");
        arg.systemIds = NULL;
    }

    return arg.systemIds;
}

v_deliveryWaitList
v_deliveryWaitListNew(
    v_deliveryGuard _this,
    v_message msg)
{
    v_deliveryWaitList waitlist = NULL;
    v_deliveryWaitList found;
    c_type type;

    assert(C_TYPECHECK(_this,v_deliveryGuard));

    if (_this) {
synch_printf("v_deliveryWaitListNew: enter\n");
        /* lookup or create a writer specific admin.
         */
        type = c_subType(_this->waitlists);
        waitlist = c_new(type);
        c_free(type);
        if (waitlist) {
/*            waitlist->writerInstanceGID = msg->writerInstanceGID;
            waitlist->writerGID = msg->writerGID;*/
            waitlist->sequenceNumber = msg->sequenceNumber;
        }
        found = c_tableInsert(_this->waitlists, waitlist);
        if (found != waitlist) {
            /* This should not happen! */
            OS_REPORT(OS_ERROR,
                      "v_deliveryWaitListNew",0,
                      "detected inconsistent waitlist admin.");
            c_free(waitlist);
            waitlist = NULL;
        } else {
            waitlist->systemIds = copySystemIdsFromPublications(_this);
            waitlist->guard = _this;
        }
synch_printf("v_deliveryWaitListNew: exit\n");
    } else {
        OS_REPORT(OS_ERROR,
                  "v_deliveryWaitListNew",0,
                  "Operation failed: illegal parameter (_this == NULL).");
    }

    synch_printf("v_deliveryWaitNew: returning waitlist, values:\n");
    synch_printf("                          sequenceNumber    = %d\n",waitlist->sequenceNumber);

    return waitlist;
}

v_result
v_deliveryWaitListFree(
    v_deliveryWaitList _this)
{
    v_deliveryWaitList found;
    v_result result;
    c_type type;

    assert(C_TYPECHECK(_this,v_deliveryWaitList));

    if (_this) {
synch_printf("v_deliveryWaitListFree: enter\n");
        /* lookup or create a writer specific admin.
         */
        found = c_remove(v_deliveryGuard(_this->guard)->waitlists, _this, NULL, NULL);
        assert(found == _this);
        assert(c_refCount(found) == 2);
        c_free(found);
        c_free(_this);
        result = V_RESULT_OK;
synch_printf("v_deliveryWaitListFree: exit\n");
    } else {
        result = V_RESULT_ILL_PARAM;
    }
    return result;
}

