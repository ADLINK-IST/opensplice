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

#include "saj_StatusCondition.h"
#include "saj_utilities.h"
#include "u_statusCondition.h"
#include "u_observable.h"
#include "saj__report.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_StatusConditionImpl_##name

JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniStatusConditionNew)(
    JNIEnv *env,
    jobject jstatusCondition,
    jlong uEntity)
{
    u_statusCondition uStatusCondition = NULL;
    jobject condition;

    if (uEntity != 0) {
        uStatusCondition = u_statusConditionNew(SAJ_VOIDP(uEntity));
        if (uStatusCondition != NULL) {
            condition = NEW_GLOBAL_REF(env, jstatusCondition);
            u_observableSetUserData(u_observable(uStatusCondition), condition);
        }
    }

    return SAJ_JLONG(uStatusCondition);
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniStatusConditionFree) (
    JNIEnv  *env,
    jobject this,
    jlong uStatusCondition)
{
    int result = SAJ_RETCODE_ALREADY_DELETED;
    u_result uResult;
    jobject condition;

    OS_UNUSED_ARG(this);

    condition = u_observableSetUserData(SAJ_VOIDP(uStatusCondition), NULL);
    if (condition != NULL) {
        DELETE_GLOBAL_REF(env, condition);
        uResult = u_objectClose(SAJ_VOIDP(uStatusCondition));
        result = saj_retcode_from_user_result(uResult);
    } else {
        result = SAJ_RETCODE_ERROR;
    }

    return result;
}


/**
 * Class:     org_opensplice_dds_dcps_StatusConditionImpl
 * Method:    jniSetEnabledStatuses
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetEnabledStatuses)(
    JNIEnv *env,
    jobject jstatusCondition,
    jlong uStatusCondition,
    jint jmask)
{
    saj_returnCode result;
    u_result uResult;
    u_eventMask uMask = saj_statusMask_to_eventMask(jmask);

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jstatusCondition);

    uResult = u_statusCondition_set_mask(SAJ_VOIDP(uStatusCondition), uMask);
    result = saj_retcode_from_user_result(uResult);

    return (jint)result;
}

JNIEXPORT jboolean JNICALL
SAJ_FUNCTION(jniGetTriggerValue)(
    JNIEnv *env,
    jobject jstatusCondition,
    jlong uStatusCondition)
{
    c_ulong triggerValue;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jstatusCondition);

    (void)u_statusCondition_get_triggerValue(SAJ_VOIDP(uStatusCondition), &triggerValue);

    return (jboolean)(triggerValue > 0);
}

#undef SAJ_FUNCTION
