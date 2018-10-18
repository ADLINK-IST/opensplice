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

#include "saj_Entity.h"
#include "saj_utilities.h"
#include "saj_qosUtils.h"
#include "u_entity.h"
#include "u_observable.h"
#include "saj__report.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_EntityImpl_##name

/**
 * Class:     org_opensplice_dds_dcps_EntityImpl
 * Method:    jniEnable
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniEnable)(
    JNIEnv *env,
    jobject jentity,
    jlong uEntity)
{
    u_result uResult = u_entityEnable(SAJ_VOIDP(uEntity));
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jentity);
    return saj_retcode_from_user_result(uResult);
}

JNIEXPORT jboolean JNICALL
SAJ_FUNCTION(jniIsEnable)(
    JNIEnv *env,
    jobject jentity,
    jlong uEntity)
{
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jentity);
    return (jboolean)u_entityEnabled(SAJ_VOIDP(uEntity));
}

/**
 * Class:     org_opensplice_dds_dcps_EntityImpl
 * Method:    jniGetStatusChanges
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetStatusChanges)(
    JNIEnv *env,
    jobject jentity,
    jlong uEntity)
{
    int result = SAJ_RETCODE_OK;
    u_result uResult;
    jint mask;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jentity);

    uResult = u_observableAction(SAJ_VOIDP(uEntity), saj_eventMask_to_statusMask, &mask);
    result = saj_retcode_from_user_result(uResult);
    if (result != SAJ_RETCODE_OK) {
        SAJ_REPORT(result, "Failed to retrieve status changes.");
    }

    return mask;
}

JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniGetInstanceHandle)(
    JNIEnv *env,
    jobject jentity,
    jlong uEntity)
{
    u_instanceHandle handle;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jentity);

    handle = u_entityGetInstanceHandle(SAJ_VOIDP(uEntity));

    return (jlong)handle;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetListener)(
    JNIEnv *env,
    jobject jentity,
    jlong uEntity,
    jlong uListener,
    jint mask)
{
    u_result uResult;
    int retcode;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jentity);

    uResult = u_entitySetListener(SAJ_VOIDP(uEntity),
                                  SAJ_VOIDP(uListener), NULL,
                                  saj_statusMask_to_eventMask(mask));
    retcode = saj_retcode_from_user_result(uResult);

    return retcode;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDisableCallbacks)(
    JNIEnv *env,
    jobject jentity,
    jlong uEntity)
{
    u_bool triggered;
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jentity);
    triggered = u_entityDisableCallbacks(SAJ_VOIDP(uEntity));
    return (jint)triggered;
}

#undef SAJ_FUNCTION
