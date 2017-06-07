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

#include "u_reader.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u__instanceHandle.h"
#include "u_dataReader.h"
#include "u_dataView.h"
#include "u_query.h"
#include "u__user.h"
#include "u__types.h"
#include "v_public.h"
#include "v_reader.h"
#include "v_dataReader.h"
#include "v_spliced.h"
#include "v_participant.h"

#include "os_report.h"

u_result
u_readerInit(
    u_reader _this,
    v_reader reader,
    u_subscriber subscriber)
{
    u_result result;

    if (_this != NULL) {
        result = u_entityInit(u_entity(_this), v_entity(reader), u_observableDomain(u_observable(subscriber)));
    } else {
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u__readerDeinitW(
    void *_this)
{
    return u__entityDeinitW(_this);
}

void
u__readerFreeW(
    void *_this)
{
    u__entityFreeW(_this);
}

u_result
u_readerGetDeadlineMissedStatus(
    const u_reader _this,
    u_bool reset,
    u_statusAction action,
    void *arg)
{
    v_reader reader;
    u_result result;

    assert(_this);
    assert(action);

    result = u_observableReadClaim(u_observable(_this), (v_public *)(&reader),C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        result = u_resultFromKernel(
                     v_readerGetDeadlineMissedStatus(reader,
                                                     reset,
                                                     (u_statusAction)action,
                                                     arg));
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }  else {
        OS_REPORT(OS_ERROR, "u_readerDeadlineMissedStatus", result,
                  "Illegal handle detected");
    }
    return result;
}

u_result
u_readerGetIncompatibleQosStatus(
    const u_reader _this,
    u_bool reset,
    u_statusAction action,
    void *arg)
{
    v_reader reader;
    u_result result;

    assert(_this);
    assert(action);

    result = u_observableReadClaim(u_observable(_this), (v_public *)(&reader), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK){
        result = u_resultFromKernel(
                     v_readerGetIncompatibleQosStatus(reader,
                                                      reset,
                                                      (u_statusAction)action,
                                                      arg));
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_ERROR, "u_readerGetIncompatibleQosStatus", result,
                  "Illegal handle detected");
    }
    return result;
}

u_result
u_readerGetSampleRejectedStatus(
    const u_reader _this,
    u_bool reset,
    u_statusAction action,
    void *arg)
{
    v_reader reader;
    u_result result;

    assert(_this);
    assert(action);

    result = u_observableReadClaim(u_observable(_this), (v_public *)(&reader), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK){
        result = u_resultFromKernel(
                     v_readerGetSampleRejectedStatus(reader,
                                                     reset,
                                                     (u_statusAction)action,
                                                     arg));
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }  else {
         OS_REPORT(OS_ERROR, "u_readerGetSampleRejectedStatus", result,
                   "Illegal handle detected");
    }
    return result;
}

u_result
u_readerGetLivelinessChangedStatus(
    const u_reader _this,
    u_bool reset,
    u_statusAction action,
    void *arg)
{
    v_reader reader;
    u_result result;

    assert(_this);
    assert(action);

    result = u_observableReadClaim(u_observable(_this), (v_public *)(&reader), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK){
        result = u_resultFromKernel(
                     v_readerGetLivelinessChangedStatus(reader,
                                                        reset,
                                                        (u_statusAction)action,
                                                        arg));
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_readerGetSampleLostStatus(
    const u_reader _this,
    u_bool reset,
    u_statusAction action,
    void *arg)
{
    v_reader reader;
    u_result result;

    assert(_this);
    assert(action);

    result = u_observableReadClaim(u_observable(_this), (v_public *)(&reader), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK){
        result = u_resultFromKernel(
                     v_readerGetSampleLostStatus(reader,
                                                 reset,
                                                 (u_statusAction)action,
                                                 arg));
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_ERROR, "u_readerGetSampleLostStatus", result,
                  "Illegal handle detected");
    }
    return result;
}

u_result
u_readerGetSubscriptionMatchStatus(
    const u_reader _this,
    u_bool reset,
    u_statusAction action,
    void *arg)
{
    v_reader reader;
    u_result result;

    assert(_this);
    assert(action);

    result = u_observableReadClaim(u_observable(_this), (v_public *)(&reader), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK){
        result = u_resultFromKernel(
                     v_readerGetSubscriptionMatchedStatus(reader,
                                                          reset,
                                                          (u_statusAction)action,
                                                          arg));
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_ERROR, "u_readerGetSubscriptionMatchStatus", result,
                  "Illegal handle detected");
    }
    return result;
}

u_result
u_readerGetMatchedPublications (
    const u_reader _this,
    u_publicationInfo_action action,
    void *arg)
{
    v_dataReader reader;
    v_spliced spliced;
    v_kernel kernel;
    u_result result;
    c_iter participants;
    v_participant participant;

    assert(_this);
    assert(action);

    result = u_observableReadClaim(u_observable(_this), (v_public *)(&reader), C_MM_RESERVATION_ZERO);
    if ((result == U_RESULT_OK) && (reader != NULL)) {
        kernel = v_objectKernel(reader);

        participants = v_resolveParticipants(kernel, V_SPLICED_NAME);
        assert(c_iterLength(participants) == 1);
        participant = v_participant(c_iterTakeFirst(participants));
        spliced = v_spliced(participant);
        c_free(participant);
        c_iterFree(participants);

        result = u_resultFromKernel(
                     v_splicedGetMatchedPublications(spliced,
                                                     v_dataReader(reader),
                                                     action,
                                                     arg));
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_readerGetMatchedPublicationData (
    const u_reader _this,
    u_instanceHandle publication_handle,
    u_publicationInfo_action action,
    void *arg)
{
    v_dataReader reader;
    v_spliced spliced;
    v_kernel kernel;
    u_result result;
    c_iter participants;
    v_participant participant;

    assert(_this);
    assert(action);

    result = u_observableReadClaim(u_observable(_this), (v_public *)(&reader), C_MM_RESERVATION_ZERO);
    if ((result == U_RESULT_OK) && (reader != NULL)) {
        kernel = v_objectKernel(reader);

        participants = v_resolveParticipants(kernel, V_SPLICED_NAME);
        assert(c_iterLength(participants) == 1);
        participant = v_participant(c_iterTakeFirst(participants));
        spliced = v_spliced(participant);
        c_free(participant);
        c_iterFree(participants);

        result = u_resultFromKernel(
                     v_splicedGetMatchedPublicationData(spliced,
                                                        v_dataReader(reader),
                                                        u_instanceHandleToGID(publication_handle),
                                                        action,
                                                        arg));
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }
    return result;

}

u_result
u_readerRead(
    const u_reader r,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout)
{
    u_result result;

    assert(r);
    assert(action);

    switch (u_objectKind(u_object(r))) {
    case U_READER:
        result = u_dataReaderRead(u_dataReader(r), mask, action, actionArg, timeout);
    break;
    case U_DATAVIEW:
        result = u_dataViewRead(u_dataView(r), mask, action, actionArg, timeout);
    break;
    case U_QUERY:
        /* mask is not used as the query itself has a fixed mask. */
        result = u_queryRead(u_query(r), action, actionArg, timeout);
    break;
    default:
        result = U_RESULT_ILL_PARAM;
    break;
    }
    return result;
}

u_result
u_readerTake(
    const u_reader r,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout)
{
    u_result result;

    assert(r);
    assert(action);

    switch (u_objectKind(u_object(r))) {
    case U_READER:
        result = u_dataReaderTake(u_dataReader(r), mask, action, actionArg, timeout);
    break;
    case U_DATAVIEW:
        result = u_dataViewTake(u_dataView(r), mask, action, actionArg, timeout);
    break;
    case U_QUERY:
        /* mask is not used as the query itself has a fixed mask. */
        result = u_queryTake(u_query(r), action, actionArg, timeout);
    break;
    default:
        result = U_RESULT_ILL_PARAM;
    break;
    }
    return result;
}

u_result
u_readerReadInstance(
    const u_reader r,
    u_instanceHandle h,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout)
{
    u_result result;

    assert(r);
    assert(action);

    switch (u_objectKind(u_object(r))) {
    case U_READER:
        result = u_dataReaderReadInstance(u_dataReader(r), h, mask, action, actionArg, timeout);
    break;
    case U_DATAVIEW:
        result = u_dataViewReadInstance(u_dataView(r), h, mask, action, actionArg, timeout);
    break;
    case U_QUERY:
        /* mask is not used as the query itself has a fixed mask. */
        result = u_queryReadInstance(u_query(r), h, action, actionArg, timeout);
    break;
    default:
        result = U_RESULT_ILL_PARAM;
    break;
    }
    return result;
}

u_result
u_readerTakeInstance(
    const u_reader r,
    u_instanceHandle h,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout)
{
    u_result result;

    assert(r);
    assert(action);

    switch (u_objectKind(u_object(r))) {
    case U_READER:
        result = u_dataReaderTakeInstance(u_dataReader(r), h, mask, action, actionArg, timeout);
    break;
    case U_DATAVIEW:
        result = u_dataViewTakeInstance(u_dataView(r), h, mask, action, actionArg, timeout);
    break;
    case U_QUERY:
        /* mask is not used as the query itself has a fixed mask. */
        result = u_queryTakeInstance(u_query(r), h, action, actionArg, timeout);
    break;
    default:
        result = U_RESULT_ILL_PARAM;
    break;
    }
    return result;
}


u_result
u_readerReadNextInstance(
    const u_reader r,
    u_instanceHandle h,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout)
{
    u_result result;

    assert(r);
    assert(action);

    switch (u_objectKind(u_object(r))) {
    case U_READER:
        result = u_dataReaderReadNextInstance(u_dataReader(r), h, mask, action, actionArg, timeout);
    break;
    case U_DATAVIEW:
        result = u_dataViewReadNextInstance(u_dataView(r), h, mask, action, actionArg, timeout);
    break;
    case U_QUERY:
        /* mask is not used as the query itself has a fixed mask. */
        result = u_queryReadNextInstance(u_query(r), h, action, actionArg, timeout);
    break;
    default:
        result = U_RESULT_ILL_PARAM;
    break;
    }
    return result;
}

u_result
u_readerTakeNextInstance(
    const u_reader r,
    u_instanceHandle h,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout)
{
    u_result result;

    assert(r);
    assert(action);

    switch (u_objectKind(u_object(r))) {
    case U_READER:
        result = u_dataReaderTakeNextInstance(u_dataReader(r), h, mask, action, actionArg, timeout);
    break;
    case U_DATAVIEW:
        result = u_dataViewTakeNextInstance(u_dataView(r), h, mask, action, actionArg, timeout);
    break;
    case U_QUERY:
        /* mask is not used as the query itself has a fixed mask. */
        result = u_queryTakeNextInstance(u_query(r), h, action, actionArg, timeout);
    break;
    default:
        result = U_RESULT_ILL_PARAM;
    break;
    }
    return result;
}

u_result
u_readerProtectCopyOutEnter(
    u_entity _this)
{
    u_result result;

    result = u_domainProtect(u_observable(_this)->domain);

    return result;
}

void
u_readerProtectCopyOutExit(
    u_entity _this)
{
    OS_UNUSED_ARG(_this);

    u_domainUnprotect();
}

