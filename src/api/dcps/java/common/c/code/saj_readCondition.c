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
#include "saj_ReadCondition.h"
#include "saj_utilities.h"
#include "u_observable.h"
#include "saj__report.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_ReadConditionImpl_##name

#define DDS_SAMPLE_STATE_FLAGS \
        (DDS_READ_SAMPLE_STATE | \
         DDS_NOT_READ_SAMPLE_STATE)

#define DDS_VIEW_STATE_FLAGS \
        (DDS_NEW_VIEW_STATE | \
         DDS_NOT_NEW_VIEW_STATE)

#define DDS_INSTANCE_STATE_FLAGS \
        (DDS_ALIVE_INSTANCE_STATE | \
         DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE | \
         DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE)

#define DDS_SAMPLE_MASK(sample_states, view_states, instance_states) \
        (sample_states & DDS_SAMPLE_STATE_FLAGS) | \
        ((view_states & DDS_VIEW_STATE_FLAGS) << 2) | \
        ((instance_states & DDS_INSTANCE_STATE_FLAGS) << 4)

JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniReadConditionNew)(
    JNIEnv *env,
    jobject object,
    jlong uReader,
    jobject condition,
    jint sample_mask,
    jint view_mask,
    jint instance_mask)
{
    u_query uQuery = 0;
    u_sampleMask mask;
    saj_queryData queryData;

    OS_UNUSED_ARG(object);

    if (uReader != 0) {
        mask = DDS_SAMPLE_MASK(sample_mask, view_mask, instance_mask);
        uQuery = u_queryNew(SAJ_VOIDP(uReader), NULL, "1=1", NULL, 0, mask);
        if (uQuery != NULL) {
            queryData = os_malloc(sizeof(C_STRUCT(saj_queryData)));
            queryData->condition = NEW_GLOBAL_REF(env, condition);
            queryData->mask = DDS_SAMPLE_MASK(sample_mask, view_mask, instance_mask);
            u_observableSetUserData(u_observable(uQuery), queryData);
        }
    }

    return SAJ_JLONG(uQuery);
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniReadConditionFree) (
    JNIEnv  *env,
    jobject this,
    jlong uQuery)
{
    int result = SAJ_RETCODE_ALREADY_DELETED;
    u_result uResult;
    saj_queryData queryData;

    OS_UNUSED_ARG(this);
    assert(uQuery != 0);

    queryData = u_observableSetUserData(SAJ_VOIDP(uQuery), NULL);
    if (queryData != NULL) {
        DELETE_GLOBAL_REF(env, queryData->condition);
        os_free(queryData);
        uResult = u_objectClose(SAJ_VOIDP(uQuery));
        result = saj_retcode_from_user_result(uResult);
    } else {
        result = SAJ_RETCODE_ERROR;
    }

    return result;
}

static c_bool
test_sample_states (
    c_object o,
    c_voidp args)
{
    OS_UNUSED_ARG(o);
    OS_UNUSED_ARG(args);
    return TRUE; /* state evaluation is now in the kernel. */
}

/**
 * Class:     org_opensplice_dds_dcps_ConditionImpl
 * Method:    jniGetTriggerValue
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL
SAJ_FUNCTION(jniGetTriggerValue)(
    JNIEnv *env,
    jobject jcondition,
    jlong uQuery)
{
    u_bool result = FALSE;
    u_sampleMask mask;
    saj_queryData queryData;

    assert(uQuery);
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jcondition);

    queryData = u_observableGetUserData(SAJ_VOIDP(uQuery));
    mask = queryData->mask;
    result = u_queryTest(SAJ_VOIDP(uQuery), test_sample_states, &mask);

    return (jboolean)result;
}

#undef SAJ_FUNCTION
