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

#include "saj_Object.h"
#include "saj_utilities.h"
#include "u_object.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_ObjectImpl_##name

/*
 * Class:     org_opensplice_dds_dcps_ObjectImpl
 * Method:    jniUObjectFree
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniUObjectFree)(
    JNIEnv *env,
    jobject jobject,
    jlong uObject)
{
    u_result uResult;
    int retcode;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jobject);

    uResult = u_objectFree_s(SAJ_VOIDP(uObject));
    retcode = saj_retcode_from_user_result(uResult);

    return retcode;
}

#undef SAJ_FUNCTION
