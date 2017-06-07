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

#include "u_writer.h"
#include "u__publisher.h"
#include "u__handle.h"
#include "u__types.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u__topic.h"
#include "u__participant.h"
#include "u__instanceHandle.h"
#include "u__user.h"

#include "v_writerInstance.h"
#include "v_writer.h"
#include "v_writerQos.h"
#include "v_public.h"
#include "v_instance.h"
#include "v_message.h"
#include "v_spliced.h"
#include "v_participant.h"
#include "v_topicAdapter.h"

#include "os_report.h"


#define u_writerReadClaim(_this, writer, claim) \
        u_observableReadClaim(u_observable(_this), (v_public *)(writer), claim)

#define u_writerWriteClaim(_this, writer, claim) \
        u_observableWriteClaim(u_observable(_this), (v_public *)(writer), claim)

#define u_writerRelease(_this, claim) \
        u_observableRelease(u_observable(_this), claim)

#define u_writerTopicName(_this) \
        (v_topicName((_this)->topic) ? v_topicName((_this)->topic) : "No Name")


static u_result
u__writerDeinitW(
    void *_this)
{
    return u__entityDeinitW(_this);
}

static void
u__writerFreeW(
    void *_this)
{
    u__entityFreeW(_this);
}

static u_result
u_writerInit(
    u_writer _this,
    const v_writer kw,
    const u_publisher publisher)
{
    return u_entityInit(u_entity(_this), v_entity(kw), u_observableDomain(u_observable(publisher)));
}

u_writer
u_writerNew(
    const u_publisher publisher,
    const os_char *name,
    const u_topic topic,
    const u_writerQos qos)
{
    u_writer _this = NULL;
    v_publisher kp;
    v_topicAdapter kta;
    v_writer kw;
    u_result result;

    assert(publisher);
    assert(topic);

    if (name == NULL) {
        name = "No name specified";
    }

    result = u_observableWriteClaim(u_observable(publisher),(v_public*)(&kp), C_MM_RESERVATION_HIGH);
    if (result == U_RESULT_OK){
        assert(kp);
        result = u_observableWriteClaim(u_observable(topic),(v_public*)(&kta), C_MM_RESERVATION_ZERO);
        if (result == U_RESULT_OK) {
            assert(kta);
            kw = v_writerNew(kp,name,v_topicAdapterTopic(kta),qos);
            if (kw != NULL) {
                _this = u_objectAlloc(sizeof(*_this), U_WRITER, u__writerDeinitW, u__writerFreeW);
                if (_this != NULL) {
                    result = u_writerInit(_this, kw, publisher);
                    if (result != U_RESULT_OK) {
                        OS_REPORT(OS_ERROR, "u_writerNew", result,
                                    "Writer initialization failed. "
                                    "For DataWriter: <%s>.", name);
                        u_objectFree (u_object (_this));
                        _this = NULL;
                    }
                }
                c_free(kw);
            } else {
                OS_REPORT(OS_ERROR, "u_writerNew", U_RESULT_OUT_OF_MEMORY,
                            "Create kernel entity failed. "
                            "For DataWriter: <%s>.", name);
            }
            u_observableRelease(u_observable(topic), C_MM_RESERVATION_ZERO);
        } else {
            OS_REPORT(OS_ERROR, "u_writerNew", U_RESULT_INTERNAL_ERROR,
                        "Claim Topic (0x%"PA_PRIxADDR") failed. "
                        "For DataWriter: <%s>.", (os_address)topic, name);
        }
        u_observableRelease(u_observable(publisher), C_MM_RESERVATION_HIGH);
    } else {
        OS_REPORT(OS_WARNING, "u_writerNew", U_RESULT_INTERNAL_ERROR,
                    "Claim Publisher (0x%"PA_PRIxADDR") failed. "
                    "For DataWriter: <%s>.", (os_address)publisher, name);
    }
    return _this;
}

u_result
u_writerWaitForAcknowledgments(
    const u_writer _this,
    os_duration timeout)
{
    v_writer writer;
    u_result result;

    assert(_this);

    result = u_writerReadClaim(_this, &writer,C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        result = v_writerWaitForAcknowledgments(writer, timeout);
        u_writerRelease(_this, C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_writerGetQos (
    const u_writer _this,
    u_writerQos *qos)
{
    u_result result;
    v_writer vWriter;
    v_writerQos vQos;

    assert(_this);
    assert(qos);

    result = u_writerReadClaim(_this, &vWriter, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        vQos = v_writerGetQos(v_writer(vWriter));
        *qos = u_writerQosNew(vQos);
        c_free(vQos);
        u_writerRelease(_this,C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_writerSetQos (
    const u_writer _this,
    const u_writerQos qos)
{
    u_result result;
    v_writer vWriter;

    assert(_this);
    assert(qos);

    result = u_writerReadClaim(_this, &vWriter, C_MM_RESERVATION_HIGH);
    if (result == U_RESULT_OK) {
        result = u_resultFromKernel(v_writerSetQos(vWriter, qos));
        u_writerRelease(_this, C_MM_RESERVATION_HIGH);
    }
    return result;
}

v_copyin_result
u_writerDefaultCopy(
    c_type type,
    const void *data,
    void *to)
{
    c_copyIn(type,data,&to);

    return V_COPYIN_RESULT_OK;
}

u_result
u_resultFromKernelWriteResult (
    v_writeResult vr)
{
    u_result result;

    switch (vr) {
    case V_WRITE_SUCCESS:
    case V_WRITE_REJECTED:
        result = U_RESULT_OK;
    break;
    case V_WRITE_ERROR:
        result = U_RESULT_INTERNAL_ERROR;
    break;
    case V_WRITE_TIMEOUT:
        result = U_RESULT_TIMEOUT;
    break;
    case V_WRITE_OUT_OF_RESOURCES:
        result = U_RESULT_OUT_OF_RESOURCES;
    break;
    case V_WRITE_PRE_NOT_MET:
        result = U_RESULT_PRECONDITION_NOT_MET;
    break;
    default:
        result = U_RESULT_UNDEFINED;
    break;
    }
    return result;
}

/* General write-with-handle algorithm used by all writer-functions */

typedef v_writeResult (*u_handleWriteAction)(v_writer writer,
                                             v_message message,
                                             os_timeW timestamp,
                                             v_writerInstance instance);

static u_result
u_writeWithHandleAction(
    const u_writer _this,
    u_writerCopy copy,
    void *data,
    os_timeW timestamp,
    u_instanceHandle handle,
    u_handleWriteAction action)
{
    v_writerInstance instance;
    u_result result;
    v_writer writer;
    v_message message;
    void *to;
    v_copyin_result copyResult;
    u_bool nilHandle;
    v_writeResult writeResult;

    assert(_this != NULL);
    assert(action != NULL);
    assert(copy != NULL);

    result = u_observableWriteClaim(u_observable(_this), (v_public *)(&writer), C_MM_RESERVATION_ZERO);
    if ((result == U_RESULT_OK) && (writer != NULL)) {
        if (u_instanceHandleIsNil(handle)) {
            instance = NULL;
            nilHandle = TRUE;
        } else {
            nilHandle = FALSE;
            result = u_instanceHandleClaim(handle, &instance);
            if ((result == U_RESULT_OK) &&
                 (instance != NULL) &&
                 (v_instanceEntity(instance) != (c_voidp)writer))
            {
                result = U_RESULT_PRECONDITION_NOT_MET;
                OS_REPORT(OS_ERROR, "u_writeWithHandleAction", result,
                          "Precondition not met: Instance handle does not belong to this DataWriter");
                u_instanceHandleRelease(handle);
            } else if(result == V_RESULT_HANDLE_EXPIRED){

                /* if the instance is already deleted,
                 * then the result of an unregister is PRECONDITION_NOT_MET.
                 * RP : is that statment correct in this generic function?
                 */
                result = U_RESULT_PRECONDITION_NOT_MET;
                OS_REPORT(OS_ERROR, "u_writeWithHandleAction", result,
                          "Precondition not met: Instance handle is already deleted");
                assert(instance == NULL);
            }
        }

        if (result == U_RESULT_OK) {
            if (data != NULL) {
                message = v_topicMessageNew_s(writer->topic);
                if (message) {
                    to = (void *) (message + 1);
                    copyResult = copy(v_topicDataType(writer->topic), data, to);
                    V_MESSAGE_STAMP(message,writerAllocTime);

                    if (!V_COPYIN_RESULT_IS_OK(copyResult)) {
                        c_free(message);
                        if (V_COPYIN_RESULT_IS_OUT_OF_MEMORY(copyResult)) {
                            result = U_RESULT_OUT_OF_MEMORY;
                            OS_REPORT(OS_ERROR, "u_writeWithHandleAction", result,
                                      "Out of memory: unable to create message for Topic '%s'.",
                                      u_writerTopicName(writer));
                        } else {
                            result = U_RESULT_ILL_PARAM;
                            OS_REPORT(OS_ERROR, "u_writeWithHandleAction", result,
                                      "Bad parameter: Data is invalid.");
                        }
                    }
                } else {
                    result = U_RESULT_OUT_OF_MEMORY;
                    OS_REPORT(OS_ERROR, "u_writeWithHandleAction", result,
                                "Out of memory: unable to create message for Topic '%s'.",
                                u_writerTopicName(writer));
                }
            } else {
                if (instance == NULL) {
                    if(nilHandle){
                        result = U_RESULT_PRECONDITION_NOT_MET;
                        OS_REPORT(OS_ERROR, "u_writeWithHandleAction", result,
                                        "Precondition not met: instance handle nil and Data = NULL");
                    } else {
                        result = U_RESULT_ILL_PARAM;
                        OS_REPORT(OS_ERROR, "u_writeWithHandleAction", result,
                                "Bad parameter: Data = NULL and Instance = NULL");
                    }
                } else {
                    message = v_writerInstanceCreateMessage_s(instance);
                    if (message == NULL) {
                        result = U_RESULT_OUT_OF_MEMORY;
                        OS_REPORT(OS_ERROR, "u_writeWithHandleAction", result,
                                    "Out of memory: unable to create message for Topic '%s'.",
                                    u_writerTopicName(writer));
                    }
                }
            }
            if (result == U_RESULT_OK) {
                writeResult = action(writer, message, timestamp, instance);
                result = u_resultFromKernelWriteResult(writeResult);
                c_free(message);
            }
            u_instanceHandleRelease(handle);
        }
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }
    return result;
}

/* -------------------------------- u_writerWrite --------------------------- */

u_result
u_writerWrite(
    const u_writer _this,
    u_writerCopy copy,
    void *data,
    os_timeW timestamp,
    u_instanceHandle handle)
{
    u_result result;

    assert(_this != NULL);
    assert(data != NULL);
    if (u_entityEnabled(u_entity(_this))) {
        result = u_writeWithHandleAction(_this,
                                         copy,
                                         data,
                                         timestamp,
                                         handle,
                                         v_writerWrite);
    } else {
        result = U_RESULT_PRECONDITION_NOT_MET;
        OS_REPORT(OS_ERROR, "u_writerWrite", result,
                  "Precondition not met: DataWriter is not enabled");
    }
    return result;
}

/* -------------------------------- u_writerDispose ------------------------- */

u_result
u_writerDispose(
    const u_writer _this,
    u_writerCopy copy,
    void *data,
    os_timeW timestamp,
    u_instanceHandle handle)
{
    u_result result;

    assert(_this != NULL);

    if (u_entityEnabled(u_entity(_this))) {
        result = u_writeWithHandleAction(_this,
                                         copy,
                                         data,
                                         timestamp,
                                         handle,
                                         v_writerDispose);
    } else {
        result = U_RESULT_PRECONDITION_NOT_MET;
        OS_REPORT(OS_ERROR, "u_writerDispose", result,
                  "Precondition not met: DataWriter not enabled");
    }
    return result;
}

/* -------------------------------- u_writerWriteDispose ------------------------- */

u_result
u_writerWriteDispose(
    const u_writer _this,
    u_writerCopy copy,
    void *data,
    os_timeW timestamp,
    u_instanceHandle handle)
{
    u_result result;

    assert(_this != NULL);
    assert(data != NULL);

    if (u_entityEnabled(u_entity(_this))) {
       result = u_writeWithHandleAction(_this,
                                        copy,
                                        data,
                                        timestamp,
                                        handle,
                                        v_writerWriteDispose);
    } else {
        result = U_RESULT_PRECONDITION_NOT_MET;
        OS_REPORT(OS_ERROR, "u_writerWriteDispose", result,
                  "Precondition not met: DataWriter not enabled");
    }
    return result;
}

u_result
u_writerAssertLiveliness(
    const u_writer _this)
{
    u_result result;
    v_writer writer;

    assert(_this != NULL);

    result = u_writerReadClaim(_this, &writer, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert(writer);
        v_writerAssertLiveliness(writer);
        u_writerRelease(_this, C_MM_RESERVATION_ZERO);
    }
    return result;
}

/* --------------------------- u_writerRegisterInstance --------------------- */


u_result
u_writerRegisterInstance(
    const u_writer _this,
    u_writerCopy copy,
    void *data,
    os_timeW timestamp,
    u_instanceHandle *handle)
{
    v_writer writer;
    v_message message;
    void *to;
    u_result result, wresult;
    v_copyin_result copyResult;
    v_writeResult writeResult;
    v_writerInstance instance;

    assert(_this != NULL);
    assert(data != NULL);

    if (u_entityEnabled(u_entity(_this))) {
        result = u_observableWriteClaim(u_observable(_this), (v_public *)(&writer), C_MM_RESERVATION_ZERO);
        if (result == U_RESULT_OK) {
            assert(writer);
            message = v_topicMessageNew_s(writer->topic);
            if (message) {
                to = (void *) (message + 1);
                copyResult = copy(v_topicDataType(writer->topic), data, to);

                if (V_COPYIN_RESULT_IS_OK(copyResult)) {
                    writeResult = v_writerRegister(writer,
                                                   message,
                                                   timestamp,
                                                   &instance);
                    c_free(message);
                    wresult = u_resultFromKernelWriteResult(writeResult);
                    if (wresult == U_RESULT_OK) {
                        *handle = u_instanceHandleNew(v_public(instance));
                    }
                    c_free(instance);
                    if (result == U_RESULT_OK) {
                        result = wresult;
                    }
                } else {
                    c_free(message);
                    if (V_COPYIN_RESULT_IS_OUT_OF_MEMORY(copyResult)) {
                        result = U_RESULT_OUT_OF_MEMORY;
                        OS_REPORT(OS_ERROR, "u_writerRegisterInstance", result,
                              "Out of memory: unable to create message for Topic '%s'.",
                              u_writerTopicName(writer));
                    } else {
                        result = U_RESULT_ILL_PARAM;
                        OS_REPORT(OS_ERROR, "u_writerRegisterInstance", result,
                                  "Bad parameter: Data is invalid.");
                    }
                }
            } else {
                result = U_RESULT_OUT_OF_MEMORY;
                OS_REPORT(OS_ERROR, "u_writerRegisterInstance", result,
                            "Out of memory: unable to create message for Topic '%s'.",
                            u_writerTopicName(writer));
            }
            u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
        }
    } else {
        result = U_RESULT_PRECONDITION_NOT_MET;
        OS_REPORT(OS_ERROR, "u_writerRegisterInstance", result,
                  "Precondition not met: DataWriter not enabled");
    }
    return result;
}

/* ---------------------- u_writerUnregisterInstance ------------------------ */


u_result
u_writerUnregisterInstance(
    const u_writer _this,
    u_writerCopy copy,
    void *data,
    os_timeW timestamp,
    u_instanceHandle handle)
{
    u_result result;

    assert(_this != NULL);

    if (u_entityEnabled(u_entity(_this))) {
        result = u_writeWithHandleAction(_this,
                                         copy,
                                         data,
                                         timestamp,
                                         handle,
                                         v_writerUnregister);
    } else {
        result = U_RESULT_PRECONDITION_NOT_MET;
        OS_REPORT(OS_ERROR, "u_writerUnregisterInstance", result,
                  "Precondition not met: DataWriter not enabled");
    }
    return result;
}

u_result
u_writerLookupInstance(
    const u_writer _this,
    u_writerCopy copy,
    void* keyTemplate,
    u_instanceHandle *handle)
{
    v_writer writer;
    v_message message;
    void *to;
    u_result result;
    v_copyin_result copyResult;
    v_writerInstance instance;

    assert(_this != NULL);
    assert(keyTemplate != NULL);
    assert(handle != NULL);

    if (u_entityEnabled(u_entity(_this))) {
        result = u_writerReadClaim(_this, &writer, C_MM_RESERVATION_ZERO);
        if (result == U_RESULT_OK) {
            assert(writer);
            message = v_topicMessageNew_s(writer->topic);
            if (message) {
                to = (void *) (message + 1);
                copyResult = copy(v_topicDataType(writer->topic), keyTemplate, to);

                if (V_COPYIN_RESULT_IS_OK(copyResult)) {
                    instance = v_writerLookupInstance(writer, message);
                    *handle = u_instanceHandleNew(v_public(instance));
                    c_free(message);
                    c_free(instance);
                } else {
                    c_free(message);
                    if (V_COPYIN_RESULT_IS_OUT_OF_MEMORY(copyResult)) {
                        result = U_RESULT_OUT_OF_MEMORY;
                        OS_REPORT(OS_ERROR,
                                  "u_writerLookupInstance", result,
                                  "Out of memory: unable to create message for Topic '%s'.",
                                  u_writerTopicName(writer));
                    } else {
                        result = U_RESULT_ILL_PARAM;
                        OS_REPORT(OS_ERROR, "u_writerLookupInstance", result,
                                  "Bad parameter: Data is invalid.");
                    }
                }
            } else {
                result = U_RESULT_OUT_OF_MEMORY;
                OS_REPORT(OS_ERROR,
                            "u_writerLookupInstance", result,
                            "Out of memory: unable to create message for Topic '%s'.",
                            u_writerTopicName(writer));
            }
            u_writerRelease(_this, C_MM_RESERVATION_ZERO);
        }
    } else {
        result = U_RESULT_PRECONDITION_NOT_MET;
        OS_REPORT(OS_ERROR, "u_writerLookupInstance", result,
                  "Precondition not met: DataWriter not enabled");
    }
    return result;
}

/* ---------------------- u_writerCopyKeysFromInstanceHandle -------------------------- */

u_result
u_writerCopyKeysFromInstanceHandle (
    const u_writer _this,
    u_instanceHandle handle,
    u_writerCopyKeyAction action,
    void *copyArg)
{
    v_writerInstance instance;
    u_result result;
    v_writer writer;
    v_message message;
    void *from;

    assert(_this != NULL);
    assert(action != NULL);

    result = u_instanceHandleClaim(handle, &instance);

    if ((result == U_RESULT_OK) && (instance != NULL)) {
        result = u_writerReadClaim(_this, &writer, C_MM_RESERVATION_ZERO);
        if (result == U_RESULT_OK) {
            if (v_writerContainsInstance(writer, instance)) {
                message = v_writerInstanceCreateMessage_s(instance);
                if (message) {
                    from = (void *) (message + 1);
                    action(from, copyArg);
                    c_free(message);
                } else {
                    result = U_RESULT_OUT_OF_MEMORY;
                    OS_REPORT(OS_WARNING, "u_dataWriterCopyKeysFromInstanceHandle", result,
                        "Failed to create keytemplate message"
                        "<dataWriterInstance = 0x%"PA_PRIxADDR">", (os_address)instance);
                }
            } else {
                result = U_RESULT_ILL_PARAM;
                OS_REPORT(OS_WARNING, "u_dataWriterCopyKeysFromInstanceHandle", result,
                    "Instance handle does not belong to writer"
                    "<_this = 0x%"PA_PRIxADDR" handle = %"PA_PRIu64">", (os_address)_this, (os_uint64)handle);
            }
            u_writerRelease(_this, C_MM_RESERVATION_ZERO);
        }
        u_instanceHandleRelease(handle);
    } else if (result == U_RESULT_HANDLE_EXPIRED) {
        result = U_RESULT_PRECONDITION_NOT_MET;
        OS_REPORT(OS_WARNING, "u_dataWriterCopyKeysFromInstanceHandle", result,
                            "Precondition not met: Instance handle is not registered.");
    }

    return result;
}

u_result
u_writerGetLivelinessLostStatus (
    const u_writer _this,
    u_bool reset,
    u_statusAction action,
    void *arg)
{
    v_writer writer;
    u_result result;

    assert(_this != NULL);
    assert(action != NULL);

    result = u_writerReadClaim(_this, &writer, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert(writer);
        result = u_resultFromKernel(
                     v_writerGetLivelinessLostStatus(writer,reset,action,arg));
        u_writerRelease(_this, C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_writerGetDeadlineMissedStatus (
    const u_writer _this,
    u_bool reset,
    u_statusAction action,
    void *arg)
{
    v_writer writer;
    u_result result;

    assert(_this != NULL);
    assert(action != NULL);

    result = u_writerReadClaim(_this, &writer, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert(writer);
        result = u_resultFromKernel(
                     v_writerGetDeadlineMissedStatus(writer,reset,action,arg));
        u_writerRelease(_this, C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_writerGetIncompatibleQosStatus (
    const u_writer _this,
    u_bool reset,
    u_statusAction action,
    void *arg)
{
    v_writer writer;
    u_result result;

    assert(_this != NULL);
    assert(action != NULL);

    result = u_writerReadClaim(_this, &writer, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert(writer);
        result = u_resultFromKernel(
                     v_writerGetIncompatibleQosStatus(writer,reset,action,arg));
        u_writerRelease(_this,C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_writerGetPublicationMatchStatus (
    const u_writer _this,
    u_bool reset,
    u_statusAction action,
    void *arg)
{
    v_writer writer;
    u_result result;

    assert(_this != NULL);
    assert(action != NULL);

    result = u_writerReadClaim(_this, &writer, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert(writer);
        result = u_resultFromKernel(
                     v_writerGetPublicationMatchedStatus(writer,reset,action,arg));
        u_writerRelease(_this, C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_writerGetMatchedSubscriptions (
    const u_writer _this,
    u_subscriptionInfo_action action,
    void *arg)
{
    v_writer writer;
    v_spliced spliced;
    v_kernel kernel;
    u_result result;
    c_iter participants;
    v_participant participant;

    assert(_this != NULL);
    assert(action != NULL);

    result = u_writerReadClaim(_this, &writer, C_MM_RESERVATION_HIGH);
    if (result == U_RESULT_OK) {
        assert(writer);
        kernel = v_objectKernel(writer);

        participants = v_resolveParticipants(kernel, V_SPLICED_NAME);
        assert(c_iterLength(participants) == 1);
        participant = v_participant(c_iterTakeFirst(participants));
        spliced = v_spliced(participant);
        c_free(participant);
        c_iterFree(participants);

        result = u_resultFromKernel(
                     v_splicedGetMatchedSubscriptions(spliced, writer, action, arg));
        u_writerRelease(_this, C_MM_RESERVATION_HIGH);
    }
    return result;
}

u_result
u_writerGetMatchedSubscriptionData (
    const u_writer _this,
    u_instanceHandle subscription_handle,
    u_subscriptionInfo_action action,
    void *arg)
{
    v_writer writer;
    v_spliced spliced;
    v_kernel kernel;
    u_result result;
    c_iter participants;
    v_participant participant;

    assert(_this != NULL);
    assert(action != NULL);

    result = u_writerReadClaim(_this, &writer, C_MM_RESERVATION_HIGH);
    if (result == U_RESULT_OK) {
        assert(writer);
        kernel = v_objectKernel(writer);

        participants = v_resolveParticipants(kernel, V_SPLICED_NAME);
        assert(c_iterLength(participants) == 1);
        participant = v_participant(c_iterTakeFirst(participants));
        spliced = v_spliced(participant);
        c_free(participant);
        c_iterFree(participants);

        result = u_resultFromKernel(
                     v_splicedGetMatchedSubscriptionData(
                                     spliced,
                                     writer,
                                     u_instanceHandleToGID(subscription_handle),
                                     action,
                                     arg));
        u_writerRelease(_this, C_MM_RESERVATION_HIGH);
    }
    return result;
}
