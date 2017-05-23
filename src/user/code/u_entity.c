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
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u__types.h"
#include "u_partition.h"
#include "u__topic.h"
#include "u_group.h"
#include "u_groupQueue.h"
#include "u_publisher.h"
#include "u_subscriber.h"
#include "u_writer.h"
#include "u_dataReader.h"
#include "u_dataView.h"
#include "u_networkReader.h"
#include "u_query.h"
#include "u__participant.h"
#include "u_waitset.h"
#include "u__user.h"

#include "v_entity.h"
#include "v_listener.h"
#include "v_public.h"
#include "v_participant.h"
#include "v_publisher.h"
#include "v_subscriber.h"
#include "v_writer.h"
#include "v_partition.h"
#include "v_group.h"
#include "v_topic.h"
#include "v_status.h"
#include "v_service.h"
#include "v_networking.h"
#include "v_durability.h"
#include "v_cmsoap.h"
#include "v_rnr.h"
#include "v_dataReader.h"
#include "v_dataReaderQuery.h"
#include "v_dataViewQuery.h"
#include "v_spliced.h"
#include "v_waitset.h"
#include "v_networkReader.h"
#include "v_groupQueue.h"
#include "v_dataView.h"
#include "v_objectLoan.h"

#include "sd_serializerXML.h"
#include "os_report.h"

u_result
u_entityInit(
    const u_entity _this,
    const v_entity entity,
    u_domain domain)
{
    u_result result;

    assert(_this != NULL);

    result = u_observableInit(u_observable(_this), v_public(entity), domain);
    if (result == U_RESULT_OK) {
        _this->enabled = v_entityEnabled(entity);
        v_entitySetUserData(entity,_this); /* user data is passed with events */
    }
    return result;
}

u_result
u__entityDeinitW(
    void *_this)
{
    return u__observableDeinitW(_this);
}

void
u__entityFreeW(
    void *_this)
{
    u__observableFreeW(_this);
}

u_result
u_entityEnable(
    const u_entity _this)
{
    v_entity ke;
    u_result result;

    assert(_this != NULL);

    result = u_observableWriteClaim(u_observable(_this), (v_public *)&ke, C_MM_RESERVATION_HIGH);
    if (result == U_RESULT_OK) {
        result = v_entityEnable(ke);
        if (result == U_RESULT_OK) {
            _this->enabled = TRUE;
        }
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_HIGH);
    } else {
        OS_REPORT(OS_ERROR, "u_entityEnable", result,
                    "Unable to enable Entity (0x%"PA_PRIxADDR")",
                    (os_address)_this);
    }
    return result;
}

u_result
u_entitySetListener(
    const u_entity _this,
    u_listener listener,
    void *listenerData,
    u_eventMask interest)
{
    u_result result;
    v_entity ke, kl;

    assert(_this != NULL);

    result = u_observableReadClaim(u_observable(_this), (v_public *)&ke, C_MM_RESERVATION_ZERO);

    if(result == U_RESULT_OK){
        if (listener != NULL) {
            result = u_observableReadClaim(u_observable(listener), (v_public *)&kl, C_MM_RESERVATION_ZERO);
            if(result == U_RESULT_OK){
                result = v_entitySetListener(ke, v_listener(kl), listenerData, interest);
                u_observableRelease(u_observable(listener), C_MM_RESERVATION_ZERO);
            }
        } else {
            result = v_entitySetListener(ke, v_listener(NULL), NULL, interest);
        }
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_bool
u_entityEnabled (
    const u_entity _this)
{
    v_entity ke;
    u_bool enabled = FALSE;
    u_result result;

    assert(_this != NULL);

    enabled = _this->enabled;
    if (!enabled) {
        result = u_observableReadClaim(u_observable(_this), (v_public *)&ke,C_MM_RESERVATION_ZERO);
        if(result == U_RESULT_OK)
        {
            enabled = v_entityEnabled(ke);
            _this->enabled = enabled;
            u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
        } else {
            OS_REPORT(OS_ERROR, "u_entityEnabled", result,
                        "Unable to enable entity, result code = %d", result);
        }
    }
    return enabled;
}

u_result
u_entityWalkEntities(
    const u_entity _this,
    u_bool (*action)(v_entity e, void *arg),
    void *arg)
{
    u_result result;
    v_entity ke;
    u_bool completeness;

    assert(_this != NULL);
    assert(action != NULL);

    result = u_observableReadClaim(u_observable(_this), (v_public *)&ke, C_MM_RESERVATION_ZERO);
    if(result == U_RESULT_OK){
        completeness = v_entityWalkEntities(ke,action,arg);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
        if (completeness == TRUE) {
            result = U_RESULT_OK;
        } else {
            result = U_RESULT_INTERRUPTED;
        }
    } else {
        OS_REPORT(OS_ERROR,
                    "u_entityWalkEntities", result,
                    "u_entityClaim failed: entity kind = %s",
                    u_kindImage(u_objectKind(u_object(_this))));
    }
    return result;
}

u_result
u_entityWalkDependantEntities(
    const u_entity _this,
    u_bool (*action)(v_entity e, void *arg),
    void *arg)
{
    u_result result;
    v_entity ke;
    u_bool completeness;

    assert(_this != NULL);
    assert(action != NULL);

    result = u_observableReadClaim(u_observable(_this), (v_public *)&ke, C_MM_RESERVATION_ZERO);
    if(result == U_RESULT_OK){
        completeness = v_entityWalkDependantEntities(ke,action,arg);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
        if (completeness == TRUE) {
            result = U_RESULT_OK;
        } else {
            result = U_RESULT_INTERRUPTED;
        }
    } else {
        OS_REPORT(OS_ERROR,
                    "u_entityWalkDependantEntities", result,
                    "u_entityClaim failed: entity kind = %s",
                    u_kindImage(u_objectKind(u_object(_this))));
    }

    return result;
}

u_instanceHandle
u_entityGetInstanceHandle(
    const u_entity _this)
{
    v_entity ke;
    u_instanceHandle handle = U_INSTANCEHANDLE_NIL;
    u_result result;

    assert(_this != NULL);

    result = u_observableReadClaim(u_observable(_this), (v_public *)&ke, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
/* TODO : the handle retrieval is incorrect,
 *        must be retrieved from build-in reader.
 */
        handle = u_instanceHandleFromGID(v_publicGid(v_public(ke)));
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_ERROR, "u_entityGetInstanceHandle", result,
                  "Invalid handle detected, result code %d", result);
    }

    return handle;
}

os_char *
u_entityName(
    const u_entity _this)
{
    c_char *name = NULL;
    v_entity ke;
    u_result result;

    assert(_this != NULL);

    result = u_observableReadClaim(u_observable(_this), (v_public *)&ke, C_MM_RESERVATION_ZERO);

    if(result == U_RESULT_OK){
        name = v_entityName(ke);
        if (name) {
            name = os_strdup(name);
        } else {
            name = os_strdup("No Name");
        }
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    } else {
        name = os_strdup("Invalid Entity");
    }
    return name;
}

u_result
u_entityGetEventState (
    const u_entity _this,
    u_eventMask *eventState)
{
    v_entity ke;
    u_result result;

    assert(_this != NULL);
    assert(eventState != NULL);

    result = u_observableReadClaim(u_observable(_this), (v_public *)&ke,C_MM_RESERVATION_ZERO);
    if(result == U_RESULT_OK){
        *eventState = v_statusGetMask(ke->status);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_entityGetXMLQos (
    const u_entity _this,
    os_char **xml)
{
    v_entity ke;
    u_result result;

    assert(_this != NULL);
    assert(xml != NULL);

    result = u_observableReadClaim(u_observable(_this), (v_public *)&ke, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        *xml = v_entityGetXMLQos(ke);
        u_observableRelease(u_observable(_this),C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_entitySetXMLQos (
    const u_entity _this,
    const os_char *xml)
{
    v_entity ke;
    u_result result;

    assert(_this != NULL);
    assert(xml != NULL);

    result = u_observableReadClaim(u_observable(_this), (v_public *)&ke, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        result = u_resultFromKernel(v_entitySetXMLQos(ke, xml));
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_entityReleaseLoan(
    u_entity _this,
    v_objectLoan loan)
{
    v_entity ke;
    u_result result;

    if (_this && loan) {
        result = u_observableWriteClaim(u_observable(_this), (v_public *)&ke, C_MM_RESERVATION_ZERO);
        if (result == U_RESULT_OK) {
            v_objectLoanRelease(loan);
            u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
        }
    } else if (_this == NULL) {
        result = U_RESULT_ILL_PARAM;
    } else {
        result = U_RESULT_OK;
    }
    return result;
}

u_bool
u_entityDisableCallbacks(
    const u_entity _this)
{
    v_entity ke;
    u_result result;
    u_bool triggered = FALSE;

    if (_this != NULL) {
        result = u_observableWriteClaim(u_observable(_this), (v_public *)&ke, C_MM_RESERVATION_ZERO);
        if (result == U_RESULT_OK) {
            triggered = v_entityDisableCallbacks(ke);
            u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
        }
    }
    return triggered;
}
