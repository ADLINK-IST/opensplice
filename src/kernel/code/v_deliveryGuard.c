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
#include "os_abstract.h"

v_deliveryGuard
v_deliveryGuardNew(
    v_deliveryService service,
    v_writer writer)
{
    C_STRUCT(v_deliveryGuard) template;
    c_type type;
    v_deliveryGuard _this;
    v_result result = V_RESULT_UNDEFINED;

    assert(C_TYPECHECK(service,v_deliveryService));
    assert(C_TYPECHECK(writer,v_writer));

    if (service && writer) {
        /* lookup or create a writer specific delivery guard */
        template.writerGID = v_publicGid(v_public(writer));
        _this = v_deliveryServiceLookupGuard(service, &template);
        if (!_this) {
            type = c_subType(service->guards);
            _this = c_new(type);
            c_free(type);

            if (_this) {
                if (c_mutexInit(c_getBase(_this), &(_this->mutex)) == SYNC_RESULT_SUCCESS) {
                    _this->writerGID = template.writerGID;
                    _this->owner = (c_voidp)service; /* backref used by the free */
                    _this->gidType = c_resolve(c_getBase(_this),"kernelModule::v_gid");

                    type = c_resolve(c_getBase(_this),"kernelModuleI::v_deliveryWaitList");
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
                    type = c_resolve(c_getBase(_this),"kernelModuleI::v_deliveryPublisher");
                    _this->publications = c_tableNew(type,"readerGID.systemId,readerGID.localId,readerGID.serial");
                    c_free(type);

                    result = v_deliveryServiceAddGuard(service,_this);
                }
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
            OS_REPORT(OS_CRITICAL,
                "v_deliveryGuardFree",result,
                "Failed to remove guard from delivery-service; _this = 0x%"PA_PRIxADDR", owner = 0x%"PA_PRIxADDR".",
                (os_address)_this, (os_address)owner);
        } else {
            c_free(_this);
        }
    } else {
        result = V_RESULT_PRECONDITION_NOT_MET;
        OS_REPORT(OS_CRITICAL,
                  "v_deliveryGuardFree",result,
                  "Precondition not met; _this == NULL.");
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
    OS_UNUSED_ARG(result);
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
    v_result result;

    assert(C_TYPECHECK(_this,v_deliveryGuard));

    if (_this && msg) {
        c_mutexLock(&_this->mutex);
        c_walk(_this->waitlists,waitlistNotify,msg);
        c_mutexUnlock(&_this->mutex);
        result = V_RESULT_OK;
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

    result = v_deliveryWaitListIgnore(v_deliveryWaitList(o), readerGID);
    assert(result == V_RESULT_OK);
    OS_UNUSED_ARG(result);
    return TRUE;
}

v_result
v_deliveryGuardIgnore(
    v_deliveryGuard _this /* must be locked */,
    v_gid readerGID)
{
    assert(_this);
    assert(C_TYPECHECK(_this,v_deliveryGuard));

    c_walk(_this->waitlists, waitlistIgnore, &readerGID);

    return V_RESULT_OK;
}
