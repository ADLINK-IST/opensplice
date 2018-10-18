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

#include "u_dataReader.h"
#include "u_dataView.h"
#include "u_subscriber.h"
#include "u__reader.h"
#include "u__user.h"
#include "u__handle.h"
#include "u__types.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u__instanceHandle.h"

#include "v_subscriber.h"
#include "v_topic.h"
#include "v_reader.h"
#include "v_dataReader.h"
#include "v_dataReaderInstance.h"
#include "v_reader.h"
#include "v_query.h"
#include "v_entity.h"
#include "v_public.h"
#include "c_iterator.h"

#include "os_report.h"

#define u_dataReaderReadClaim(_this, reader, claim) \
        u_observableReadClaim(u_observable(_this), (v_public *)(reader), claim)

#define u_dataReaderWriteClaim(_this, reader, claim) \
        u_observableWriteClaim(u_observable(_this), (v_public *)(reader), claim)

#define u_dataReaderRelease(_this, claim) \
        u_observableRelease(u_observable(_this), claim)


static u_result
u__dataReaderDeinitW(
    void *_this)
{
    return u__readerDeinitW(_this);
}

static void
u__dataReaderFreeW(
    void *_this)
{
    u__readerFreeW(_this);
}

static u_result
u_dataReaderInit(
    u_dataReader _this,
    const v_dataReader reader,
    const u_subscriber subscriber)
{
    return u_readerInit(u_reader(_this),v_reader(reader),subscriber);
}

u_dataReader
u_dataReaderNew(
    const u_subscriber s,
    const os_char *name,
    const os_char *expr,
    const c_value params[],
    os_uint32 nrOfParams,
    const u_readerQos qos)
{
    u_dataReader _this = NULL;
    v_subscriber ks = NULL;
    v_dataReader reader;
    u_result result;

    assert (s);

    if (name == NULL) {
        name = "No name specified";
    }

    result = u_observableWriteClaim(u_observable(s), (v_public *)(&ks), C_MM_RESERVATION_HIGH);
    if (result == U_RESULT_OK) {
        assert(ks);
        reader = v_dataReaderNewBySQL(ks, name, expr, params, nrOfParams, qos);
        if (reader != NULL) {
            _this = u_objectAlloc(sizeof(*_this), U_READER, u__dataReaderDeinitW, u__dataReaderFreeW);
            if (_this != NULL) {
                result = u_dataReaderInit(_this,reader,s);
                if (result != U_RESULT_OK) {
                    OS_REPORT(OS_ERROR, "u_dataReaderNew", result,
                                "Initialisation failed. "
                                "For DataReader: <%s>.", name);
                    u_objectFree (u_object (_this));
                    _this = NULL;
                }
            } else {
                OS_REPORT(OS_ERROR, "u_dataReaderNew", U_RESULT_INTERNAL_ERROR,
                            "Create user proxy failed. "
                            "For DataReader: <%s>.", name);
            }
            c_free(reader);
        } else {
            OS_REPORT(OS_ERROR, "u_dataReaderNew", U_RESULT_INTERNAL_ERROR,
                        "Create kernel entity failed. "
                        "For DataReader: <%s>.", name);
        }
        u_observableRelease(u_observable(s), C_MM_RESERVATION_HIGH);
    } else {
        OS_REPORT(OS_WARNING, "u_dataReaderNew", result,
                    "Claim Subscriber (0x%"PA_PRIxADDR") failed. "
                    "For DataReader: <%s>.", (os_address)s, name);
    }
    return _this;
}

struct handleBuffer {
    u_instanceHandle *buf;
    os_uint32 length;
};

static c_bool
copyInstanceHandle(
    v_dataReaderInstance instance,
    c_voidp arg)
{
    struct handleBuffer *a = (struct handleBuffer *)arg;

    if (a->length == 0) {
        c_ulong length = c_count(v_dataReaderInstanceGetNotEmptyInstanceSet(instance));
        a->buf = os_malloc(length * sizeof(u_instanceHandle));
    }
    if (a->buf)
    {
        a->buf[a->length++] = u_instanceHandleNew((v_public)instance);
    }
    return TRUE;
}

u_result
u_dataReaderGetInstanceHandles (
    const u_dataReader _this,
    u_result (*action)(u_instanceHandle *buf, os_uint32 length, void *arg),
    void *arg)
{
    v_dataReader reader;
    u_result result;
    struct handleBuffer copyArg;

    assert (_this);
    assert (action);

    result = u_dataReaderReadClaim(_this, &reader, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        copyArg.length = 0;
        copyArg.buf = NULL;
        v_dataReaderWalkInstances(reader, copyInstanceHandle, (void *)&copyArg);
        u_dataReaderRelease(_this, C_MM_RESERVATION_ZERO);
        result = action(copyArg.buf, copyArg.length, arg);
        os_free(copyArg.buf);
    }
    return result;
}

/***************************** read/take *********************************/

u_result
u_dataReaderRead(
    const u_dataReader _this,
    u_sampleMask mask,
    u_dataReaderAction action,
    void *argument,
    const os_duration timeout)
{
    u_result result;
    v_dataReader reader;

    assert(_this);
    assert(action);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_dataReaderReadClaim(_this, &reader, C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        result = u_resultFromKernel(v_dataReaderRead(reader, mask, action, argument, timeout));
        u_dataReaderRelease(_this, C_MM_RESERVATION_LOW);
    }

    return result;
}

u_result
u_dataReaderTake(
    const u_dataReader _this,
    u_sampleMask mask,
    u_dataReaderAction action,
    void *argument,
    const os_duration timeout)
{
    u_result result;
    v_dataReader reader;

    assert(_this);
    assert(action);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_dataReaderReadClaim(_this, &reader,C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        result = u_resultFromKernel(v_dataReaderTake(reader, mask, action, argument, timeout));
        u_dataReaderRelease(_this, C_MM_RESERVATION_LOW);
    }
    return result;
}

/***************************** read/take_(next)_instance **********************/

u_result
u_dataReaderReadInstance(
    const u_dataReader _this,
    u_instanceHandle handle,
    u_sampleMask mask,
    u_dataReaderAction action,
    void *argument,
    const os_duration timeout)
{
    v_dataReaderInstance instance;
    v_dataReader reader;
    u_result result;

    assert(_this);
    assert(action);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_dataReaderReadClaim(_this, &reader, C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        handle = u_instanceHandleFix(handle,v_collection(reader));
        result = u_instanceHandleClaim(handle, &instance);
        if (result == U_RESULT_OK) {
            result = u_resultFromKernel(v_dataReaderReadInstance(reader, instance, mask,
                                                                 action, argument, timeout));
            u_instanceHandleRelease(handle);
        }
        u_dataReaderRelease(_this, C_MM_RESERVATION_LOW);
    }
    return result;
}

u_result
u_dataReaderTakeInstance(
    const u_dataReader _this,
    u_instanceHandle handle,
    u_sampleMask mask,
    u_dataReaderAction action,
    void *argument,
    const os_duration timeout)
{
    v_dataReaderInstance instance;
    v_dataReader reader;
    u_result result;

    assert(_this);
    assert(action);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_dataReaderReadClaim(_this, &reader, C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        handle = u_instanceHandleFix(handle,v_collection(reader));
        result = u_instanceHandleClaim(handle, &instance);
        if (result == U_RESULT_OK) {
            result = u_resultFromKernel(v_dataReaderTakeInstance(reader, instance, mask,
                                                                 action, argument, timeout));
            u_instanceHandleRelease(handle);
        }
        u_dataReaderRelease(_this, C_MM_RESERVATION_LOW);
    }
    return result;
}

u_result
u_dataReaderReadNextInstance(
    const u_dataReader _this,
    u_instanceHandle handle,
    u_sampleMask mask,
    u_dataReaderAction action,
    void *argument,
    const os_duration timeout)
{
    v_dataReaderInstance instance;
    v_dataReader reader;
    u_result result;

    assert(_this);
    assert(action);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_dataReaderReadClaim(_this, &reader, C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        if (u_instanceHandleIsNil(handle)) {
            result = u_resultFromKernel(v_dataReaderReadNextInstance(reader, NULL, mask, action, argument, timeout));
        } else {
            handle = u_instanceHandleFix(handle,v_collection(reader));
            result = u_instanceHandleClaim(handle, &instance);
            if (result == U_RESULT_HANDLE_EXPIRED) {
#if 0
                /* The handle has become invalid and no instance including
                 * the key value can be found. Therefore set the instance
                 * to null and start reading from scratch.
                 * Doing an automatic correction of already deleted handles
                 * hides information for the user but can be a convenience
                 * for iterating over all instances.
                 * Conceptual this should be left out.
                 */
                result = u_resultFromKernel(v_dataReaderReadNextInstance(reader, NULL, readAction, &arg));
#endif
            } else if (result == U_RESULT_OK) {
                assert(instance != NULL);
                result = u_resultFromKernel(v_dataReaderReadNextInstance(reader, instance, mask,
                                                                         action, argument, timeout));
                u_instanceHandleRelease(handle);
            }
        }
        u_dataReaderRelease(_this, C_MM_RESERVATION_LOW);
    }
    return result;
}

u_result
u_dataReaderTakeNextInstance(
    const u_dataReader _this,
    u_instanceHandle handle,
    u_sampleMask mask,
    u_dataReaderAction action,
    void *argument,
    const os_duration timeout)
{
    v_dataReaderInstance instance;
    v_dataReader reader;
    u_result result;

    assert(_this);
    assert(action);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_dataReaderReadClaim(_this, &reader, C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        if (u_instanceHandleIsNil(handle)) {
            result = u_resultFromKernel(v_dataReaderTakeNextInstance(reader, NULL, mask, action, argument, timeout));
        } else {
            handle = u_instanceHandleFix(handle,v_collection(reader));
            result = u_instanceHandleClaim(handle, &instance);
            if (result == U_RESULT_HANDLE_EXPIRED) {
#if 0
                /* The handle has become invalid and no instance including
                 * the key value can be found. Therefore set the instance
                 * to null and start reading from scratch.
                 * Doing an automatic correction of already deleted handles
                 * hides information for the user but can be a convenience
                 * for iterating over all instances.
                 * Conceptual this should be left out.
                 */
                result = u_resultFromKernel(v_dataReaderTakeNextInstance(reader, NULL, readAction, &arg));
#endif
            } else if (result == U_RESULT_OK) {
                assert(instance != NULL);
                result = u_resultFromKernel(v_dataReaderTakeNextInstance(reader, instance, mask,
                                                                         action, argument, timeout));
                u_instanceHandleRelease(handle);
            }
        }
        u_dataReaderRelease(_this, C_MM_RESERVATION_LOW);
    }
    return result;
}

u_result
u_dataReaderWaitForHistoricalData(
    const u_dataReader _this,
    os_duration timeout)
{
    v_dataReader reader;
    u_result result;

    assert(_this);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_dataReaderReadClaim(_this, &reader, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        result = u_resultFromKernel(v_readerWaitForHistoricalData(v_reader(reader), timeout, FALSE));
        u_dataReaderRelease(_this, C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_dataReaderWaitForHistoricalDataWithCondition(
    const u_dataReader _this,
    const os_char* filter,
    const os_char* params[],
    os_uint32 paramsLength,
    os_timeW minSourceTime,
    os_timeW maxSourceTime,
    os_int32 max_samples,
    os_int32 max_instances,
    os_int32 max_samples_per_instance,
    os_duration timeout)
{
    v_dataReader reader;
    u_result result;

    assert(_this);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_dataReaderReadClaim(_this, &reader, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        result = u_resultFromKernel(v_readerWaitForHistoricalDataWithCondition(
            v_reader(reader), filter, params, paramsLength,
            minSourceTime, maxSourceTime,
            max_samples, max_instances, max_samples_per_instance,
            timeout, FALSE));
        u_dataReaderRelease(_this, C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_dataReaderLookupInstance(
    const u_dataReader _this,
    void *keyTemplate,
    u_copyIn copyIn,
    u_instanceHandle *handle)
{
    v_dataReaderInstance instance;
    u_result result;
    v_copyin_result copyResult;
    v_dataReader reader;
    v_message message;
    c_voidp to;
    v_topic topic;

    assert(_this);
    assert(keyTemplate);
    assert(copyIn);
    assert(handle);

    result = u_dataReaderReadClaim(_this, &reader, C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        topic = v_dataReaderGetTopic(reader);

        if(topic){
            message = v_topicMessageNew_s(topic);
            if (message) {
                to = (void *) (message + 1);
                copyResult = copyIn(v_topicDataType(topic), keyTemplate, to);
                if (V_COPYIN_RESULT_IS_OK(copyResult)) {
                    instance = v_dataReaderLookupInstance(reader, message);
                    *handle = u_instanceHandleNew(v_public(instance));
                    c_free(instance);
                } else {
                    if (V_COPYIN_RESULT_IS_OUT_OF_MEMORY(copyResult)) {
                        result = U_RESULT_OUT_OF_MEMORY;
                        OS_REPORT(OS_ERROR, "u_dataReaderLookupInstance", result,
                                  "Out of memory: unable to create message for Topic: '%s'.",
                                  v_entityName2(topic));
                    } else {
                        result = U_RESULT_ILL_PARAM;
                        OS_REPORT(OS_ERROR, "u_dataReaderLookupInstance", result,
                                  "Invalid key: unable to create message for Topic: '%s'.",
                                  v_entityName2(topic));
                    }
                }
                c_free(message);
            } else {
                c_char *name = v_topicName(topic);
                if (name == NULL) {
                    name = "No Name";
                }
                result = U_RESULT_OUT_OF_MEMORY;
                OS_REPORT(OS_ERROR, "u_dataReaderLookupInstance", result,
                          "Out of memory: unable to create message for Topic: '%s'.",
                          name);
            }
            c_free(topic);
        } else {
            result = U_RESULT_ALREADY_DELETED;
        }
        u_dataReaderRelease(_this, C_MM_RESERVATION_LOW);
    } else {
        OS_REPORT(OS_WARNING, "u_dataReaderLookupInstance", U_RESULT_INTERNAL_ERROR,
                  "dataReader could not be claimed.");
    }
    return result;
}

u_result
u_dataReaderCopyKeysFromInstanceHandle(
    const u_dataReader _this,
    u_instanceHandle handle,
    u_copyOut action,
    void *copyArg)
{
    v_dataReaderInstance instance;
    u_result result;
    v_dataReader reader;
    v_message message;
    void *from;

    assert(_this);
    assert(action);

    result = u_instanceHandleClaim(handle, &instance);
    if ((result == U_RESULT_OK) && (instance != NULL)) {
        result = u_dataReaderReadClaim(_this, &reader, C_MM_RESERVATION_ZERO);
        if (result == U_RESULT_OK) {
            if (v_dataReaderContainsInstance(reader, instance)) {
                message = v_dataReaderInstanceCreateMessage(instance);
                if (message) {
                    from = (void *) (message + 1);
                    action(from, copyArg);
                    c_free(message);
                } else {
                    result = U_RESULT_OUT_OF_MEMORY;
                    OS_REPORT(OS_ERROR, "u_dataReaderCopyKeysFromInstanceHandle", result,
                        "Failed to create keytemplate message"
                        "<dataReaderInstance = 0x%"PA_PRIxADDR">", (os_address)instance);
                }
            } else {
                result = U_RESULT_ILL_PARAM;
                OS_REPORT(OS_ERROR, "u_dataReaderCopyKeysFromInstanceHandle", result,
                    "Instance handle does not belong to reader"
                    "<_this = 0x%"PA_PRIxADDR" handle = %"PA_PRIu64">", (os_address)_this, (os_uint64)handle);
            }
            u_dataReaderRelease(_this, C_MM_RESERVATION_ZERO);
        }
        u_instanceHandleRelease(handle);
    } else if (result == U_RESULT_HANDLE_EXPIRED) {
        result = U_RESULT_ALREADY_DELETED;
    }
    return result;
}

u_result
u_dataReaderGetQos (
    const u_dataReader _this,
    u_readerQos *qos)
{
    u_result result;
    v_dataReader reader;
    v_readerQos vQos;

    assert(_this);
    assert(qos);

    result = u_dataReaderReadClaim(_this, &reader, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        vQos = v_readerGetQos(v_reader(reader));
        *qos = u_readerQosNew(vQos);
        c_free(vQos);
        u_dataReaderRelease(_this, C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_dataReaderSetQos (
    const u_dataReader _this,
    const u_readerQos qos)
{
    u_result result;
    v_dataReader reader;

    assert(_this);
    assert(qos);

    result = u_dataReaderReadClaim(_this, &reader,C_MM_RESERVATION_HIGH);
    if (result == U_RESULT_OK) {
        result = u_resultFromKernel(v_dataReaderSetQos(reader, qos));
        u_dataReaderRelease(_this, C_MM_RESERVATION_HIGH);
    }
    return result;
}

c_ulong
u_dataReaderGetNumberOpenTransactions(
    u_dataReader _this)
{
    u_result result;
    v_dataReader reader;
    c_ulong count = 0;

    assert(_this);

    result = u_dataReaderReadClaim(_this, &reader, C_MM_RESERVATION_NO_CHECK);
    if (result == U_RESULT_OK) {
        count = v_dataReaderGetNumberOpenTransactions(reader);
        u_dataReaderRelease(_this, C_MM_RESERVATION_NO_CHECK);
    }
    return count;
}

