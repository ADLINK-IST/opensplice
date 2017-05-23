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
#include "saj_Condition.h"
#include "saj_utilities.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_ConditionImpl_##name

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_ConditionImpl_##name

static void
dummy_callback(
    v_public p,
    c_voidp arg)
{
    OS_UNUSED_ARG(p);
    OS_UNUSED_ARG(arg);
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniConditionIsAlive)(
    JNIEnv *env,
    jobject _this,
    jlong  uObject)
{
    int result = SAJ_RETCODE_ALREADY_DELETED;
    u_result uResult;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(_this);

    uResult = u_observableAction(u_observable(SAJ_VOIDP(uObject)), dummy_callback, NULL);
    result = saj_retcode_from_user_result(uResult);

    return result;
}


#undef SAJ_FUNCTION
