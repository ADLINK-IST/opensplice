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
#include "v_public.h"
#include "v_message.h"

#include "v__kernel.h"
#include "v__deliveryGuard.h"
#include "v__deliveryWaitList.h"
#include "v__partition.h"

#include "v_writer.h"
#include "v_entity.h"
#include "v_topic.h"
#include "v_observer.h"

#include "c_sync.h"

#include "os_report.h"

v_deliveryGuard
v_deliveryGuardNew(
    v_deliveryService service,
    v_writer writer)
{
    C_STRUCT(v_deliveryGuard) template;
    c_type type;
    v_deliveryGuard _this;
    v_result result;

    assert(C_TYPECHECK(service,v_deliveryService));
    assert(C_TYPECHECK(writer,v_writer));

    if (service && writer) {
        /* lookup or create a writer specific delivery guard */
        template.writerGID = v_publicGid(v_public(writer));
        _this = v_deliveryServiceLookupGuard(service,&template);
        if (!_this) {
            type = c_subType(service->guards);
            _this = c_new(type);
            c_free(type);

            if (_this) {
                c_mutexInit(&(_this->mutex),SHARED_MUTEX);
                _this->writerGID = template.writerGID;
                _this->owner = (c_voidp)service; /* backref used by the free */
                _this->gidType = c_resolve(c_getBase(_this),"kernelModule::v_gid");

                type = c_resolve(c_getBase(_this),"kernelModule::v_deliveryWaitList");
                /* The sequence number is the key, as this is unique within each writer */
                _this->waitlists = c_tableNew(type,"sequenceNumber");
                c_free(type);

                /* The following subscription set attribute contains the
                 * subscriptions of nodes that have synchronous DataReaders
                 * that communicate with this particular DataWriter.
                 * This list will be maintained by the reception of builtin
                 * topics. The list does not need to be initialized with
                 * the correct data right now because the topic is transient
                 * so the whole state will be delivered and delivery will
                 * update the list.
                 */
                type = c_resolve(c_getBase(_this),"kernelModule::v_deliveryPublisher");
                _this->publications = c_tableNew(type,"readerGID.systemId,readerGID.localId,readerGID.serial");
                c_free(type);

                result = v_deliveryServiceAddGuard(service,_this);
                if (result != V_RESULT_OK) {
                    c_free(_this);
                    _this = NULL;
                }
            }
        }
    } else {
        _this = NULL;
    }
    return _this;
}

v_result
v_deliveryGuardFree(
    v_deliveryGuard _this)
{
    v_result result;

    assert(C_TYPECHECK(_this,v_deliveryGuard));

    if (_this) {
        v_deliveryService owner = v_deliveryService(_this->owner);
        result = v_deliveryServiceRemoveGuard(owner, _this);
        if (result != V_RESULT_OK) {
            OS_REPORT_2(OS_ERROR,
                "v_deliveryGuardFree",result,
                "Failed to remove guard from delivery-service; _this = 0x%x, owner = 0x%x.",
                _this, owner);
        } else {
            c_free(_this);
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "v_deliveryGuardFree",0,
                  "Precondition not met; _this == NULL.");
        result = V_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}

static c_bool
waitlistNotify(
    c_object o,
    c_voidp arg)
{
    v_deliveryInfoTemplate msg = (v_deliveryInfoTemplate)arg;
    v_result result;

    result = v_deliveryWaitListNotify(v_deliveryWaitList(o), msg);
    assert(result == V_RESULT_OK);
    return TRUE;
}

/* This method is called to pass a received delivery message to
 * a local synchronous DataWriter admin.
 * This method will subsequently notify all the blocking threads
 * about this delivery message.
 */
v_result
v_deliveryGuardNotify(
    v_deliveryGuard _this,
    v_deliveryInfoTemplate msg)
{
    c_syncResult sr;
    v_result result;

    assert(C_TYPECHECK(_this,v_deliveryGuard));

    result = V_RESULT_UNDEFINED;

    if (_this && msg) {
        sr = c_mutexLock(&_this->mutex);
        if (sr == SYNC_RESULT_SUCCESS) {
            c_walk(_this->waitlists,waitlistNotify,msg);
            sr = c_mutexUnlock(&_this->mutex);
        }

        if (sr != SYNC_RESULT_SUCCESS) {
            result = V_RESULT_INTERNAL_ERROR;
            OS_REPORT_2(OS_ERROR,
                      "v_deliveryGuardNotify",0,
                      "Failed to claim/release mutex; _this = 0x%x, msg = 0x%x.",
                      _this, msg);
        } else {
            result = V_RESULT_OK;
        }
    } else {
        result = V_RESULT_ILL_PARAM;
    }
    return result;
}

static c_bool
waitlistIgnore(
    c_object o,
    c_voidp arg)
{
    v_gid readerGID = *(v_gid *)arg;
    v_result result;

    result = v_deliveryWaitListIgnore(v_deliveryWaitList(o),
                                      readerGID);
    assert(result == V_RESULT_OK);
    return TRUE;
}

v_result
v_deliveryGuardIgnore(
    v_deliveryGuard _this,
    v_gid readerGID)
{
    c_syncResult sr;
    v_result result;

    assert(C_TYPECHECK(_this,v_deliveryGuard));

    result = V_RESULT_UNDEFINED;

    if (_this) {
        sr = c_mutexLock(&_this->mutex);
        if (sr == SYNC_RESULT_SUCCESS)
        {
            c_walk(_this->waitlists,waitlistIgnore,&readerGID);
            sr = c_mutexUnlock(&_this->mutex);
        }
        if (sr != SYNC_RESULT_SUCCESS)
        {
            result = V_RESULT_INTERNAL_ERROR;
            OS_REPORT_4(OS_ERROR,
                      "v_deliveryGuardIgnore",0,
                      "Failed to claim/release mutex; _this = 0x%x, readerGID = {%d,%d,%d}.",
                      _this, readerGID.systemId, readerGID.localId, readerGID.serial);
        }
    } else {
        result = V_RESULT_ILL_PARAM;
    }
    return result;
}
