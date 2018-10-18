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
#include "saj_Domain.h"
#include "saj_utilities.h"
#include "saj__report.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_DomainImpl_##name

/*
 * Method: jniDomainNew
 * Param : domain name
 * Param : domain id
 * Return: u_domain
 */
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniDomainNew) (
    JNIEnv  *env,
    jobject this,
    jint domainId)
{
    u_domain uDomain = NULL;
    u_result uResult;
    jobject userData;

    uResult = u_domainOpen(&uDomain, NULL, domainId, 30);
    if (uResult == U_RESULT_OK) {
        userData = NEW_GLOBAL_REF(env, this);
        u_observableSetUserData(SAJ_VOIDP(uDomain), userData);
    }
    return SAJ_JLONG(uDomain);
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDomainFree) (
    JNIEnv  *env,
    jobject this,
    jlong uDomain)
{
    u_result uResult;
    jobject userData;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(this);

    if (uDomain) {
        userData = u_observableSetUserData(u_observable(SAJ_VOIDP(uDomain)), NULL);
        DELETE_GLOBAL_REF(env, userData);
    }
    uResult = u_domainClose(SAJ_VOIDP(uDomain));
    if (uResult != U_RESULT_OK) {
        SAJ_REPORT(SAJ_RETCODE_ERROR, "Failed to close Domain.");
    }

    return saj_retcode_from_user_result(uResult);
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDomainId) (
    JNIEnv  *env,
    jobject this,
    jlong uDomain)
{
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(this);

    assert(uDomain != 0);

    return (jint)u_domainId(SAJ_VOIDP(uDomain));
}

/*
 * Class:     org_opensplice_dds_dcps_DomainImpl
 * Method:    jniCreatePersistentSnapshot
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniCreatePersistentSnapshot)(
    JNIEnv *env,
    jobject jdomain,
    jlong uDomain,
    jstring jpartition_expression,
    jstring jtopic_expression,
    jstring jURI)
{
    u_result uResult;
    saj_returnCode result;
    const os_char *partition_expression;
    const os_char *topic_expression;
    const os_char *URI;

    OS_UNUSED_ARG(jdomain);

    assert(jpartition_expression);
    assert(jtopic_expression);
    assert(jURI);

    partition_expression = GET_STRING_UTFCHAR(env, jpartition_expression, 0);
    topic_expression = GET_STRING_UTFCHAR(env, jtopic_expression, 0);
    URI = GET_STRING_UTFCHAR(env, jURI, 0);

    uResult = u_domainCreatePersistentSnapshot(u_domain((size_t)uDomain),
                                              partition_expression,
                                              topic_expression,
                                              URI);

    result = saj_retcode_from_user_result(uResult);
    RELEASE_STRING_UTFCHAR(env, jpartition_expression, partition_expression);
    RELEASE_STRING_UTFCHAR(env, jtopic_expression, topic_expression);
    RELEASE_STRING_UTFCHAR(env, jURI, URI);

    return (jint)result;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

#undef SAJ_FUNCTION
