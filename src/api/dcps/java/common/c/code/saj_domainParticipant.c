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

#include "saj_DomainParticipant.h"
#include "saj_utilities.h"
#include "saj_qosUtils.h"
#include "saj_copyOut.h"
#include "saj_copyCache.h"

#include "v_public.h"
#include "v_dataReaderInstance.h"
#include "u_observable.h"
#include "u_instanceHandle.h"
#include "u_participant.h"
#include "os_time.h"

#include "os_report.h"
#include "saj__report.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_DomainParticipantImpl_##name

/*
 * Method: jniDomainParticipantNew
 * Param : participant name
 * Param : domain id
 * Param : participant qos
 * Return: u_participant
 */
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniDomainParticipantNew) (
    JNIEnv  *env,
    jobject this,
    jstring jname,
    jint domainId,
    jobject qos)
{
    u_participant uParticipant = NULL;
    u_participantQos uQos;
    saj_returnCode result;
    const os_char *name;
    jobject userData;

    name = GET_STRING_UTFCHAR(env, jname, 0);
    if (name != NULL){
        /*
         * Following is a sanity check.
         * Make sure that jlong can be used for address values.
         */
        if (sizeof(jlong) < sizeof(void *)) {
            OS_REPORT(OS_WARNING, OS_FUNCTION, 0,
                        "Risk of fatal address truncation on JNI interface!"
                        "For DomainParticipant: <%s>.", name);
        }
        uQos = u_participantQosNew(NULL);
        if (uQos != NULL) {
            result = saj_domainParticipantQosCopyIn(env, qos, uQos);
            if (result == SAJ_RETCODE_OK){
                uParticipant = u_participantNew(NULL, domainId, 30, name, uQos, FALSE);
                if (uParticipant != NULL) {
                    userData = NEW_GLOBAL_REF(env, this);
                    u_observableSetUserData(SAJ_VOIDP(uParticipant), userData);
                    SET_CACHED(y2038_enabled,u_observableGetY2038Ready(u_observable(SAJ_VOIDP(uParticipant))));
                    if (GET_CACHED(y2038_enabled) && GET_CACHED(time_t_constructor_mid_time64) == NULL) {
                        OS_REPORT(OS_API_INFO, OS_FUNCTION, 0,
                                    "Configuration for time beyond y2038 is enabled but the library does not support it!");
                    }
                }
            }

            u_participantQosFree(uQos);
        }
        RELEASE_STRING_UTFCHAR(env, jname, name);
    } else {
        result = SAJ_RETCODE_ERROR;
    }

    return SAJ_JLONG(uParticipant);

    CATCH_EXCEPTION:
    return 0;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDomainParticipantFree) (
    JNIEnv  *env,
    jobject this,
    jlong uParticipant)
{
    u_result uResult;
    jobject userData;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(this);

    if (uParticipant) {
        userData = u_observableSetUserData(u_observable(SAJ_VOIDP(uParticipant)), NULL);
        DELETE_GLOBAL_REF(env, userData);
    }
    uResult = u_objectClose(SAJ_VOIDP(uParticipant));
    return saj_retcode_from_user_result(uResult);
}

/*
 * Method: jniGetDomainId
 * Param : participant
 * Return: domainId
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetDomainId) (
    JNIEnv  *env,
    jobject this,
    jlong uParticipant)
{
    jint domainId;
    u_domainId_t uDomainId;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(this);

    uDomainId = u_participantGetDomainId(SAJ_VOIDP(uParticipant));
    if (uDomainId == U_DOMAIN_ID_INVALID) {
        domainId = GET_STATIC_FIELD(env, Int, DOMAIN_ID_INVALID, DOMAIN_ID_INVALID_value);
    } else {
        domainId = (jint)uDomainId;
    }

    return domainId;
    CATCH_EXCEPTION: return -1;
}

/*
 * Method: jniFindTopic
 * Param : topic name
 * Param : Duration
 * Return: Topic
 */
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniFindTopic) (
    JNIEnv  *env,
    jobject this,
    jlong uParticipant,
    jstring jtopic_name,
    jobject duration)
{
    saj_returnCode retcode = SAJ_RETCODE_OK;
    u_topic uTopic;
    os_duration timeout;
    const os_char* topicName;
    c_iter list = NULL;

    uTopic = NULL;
    topicName = NULL;

    OS_UNUSED_ARG(this);

    /* process the jstring objects */
    if (jtopic_name != NULL){
        topicName = GET_STRING_UTFCHAR(env, jtopic_name, 0);
        if (topicName == NULL) {
            retcode = SAJ_RETCODE_ERROR;
        }
    } else {
        retcode = SAJ_RETCODE_BAD_PARAMETER;
    }

    if (retcode == SAJ_RETCODE_OK) {
        if (duration != NULL){
            retcode = saj_durationCopyIn(env, duration, &timeout);
            if (retcode == SAJ_RETCODE_OK) {
                list = u_participantFindTopic(SAJ_VOIDP(uParticipant), topicName, timeout);
            }
        } else {
            list = u_participantFindTopic(SAJ_VOIDP(uParticipant), topicName, 0);
        }

        if (jtopic_name != NULL){
            RELEASE_STRING_UTFCHAR(env, jtopic_name, topicName);
        }
        if (list != NULL) {
            uTopic = c_iterTakeFirst(list);
            c_iterFree(list);
        }
    }

    return (jlong)(PA_ADDRCAST)uTopic;

    CATCH_EXCEPTION:
    return 0;
}

/*
 * Method: jniSetQos
 * Param : DomainParticipantQos
 * Return: Return code
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetQos) (
    JNIEnv *env,
    jobject this,
    jlong uParticipant,
    jobject jqos)
{
    saj_returnCode retcode = SAJ_RETCODE_ERROR;
    u_participantQos uQos;

    assert(jqos != NULL);
    OS_UNUSED_ARG(this);

    uQos = u_participantQosNew(NULL);
    if (uQos != NULL) {
        retcode = saj_domainParticipantQosCopyIn(env, jqos, uQos);
        if (retcode == SAJ_RETCODE_OK){
            retcode = saj_retcode_from_user_result(u_participantSetQos(SAJ_VOIDP(uParticipant), uQos));
        }
        u_participantQosFree(uQos);
    }

    return retcode;
}

/*
 * Method: jniGetQos
 * Param : DomainParticipantQosHolder
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetQos) (
    JNIEnv *env,
    jobject this,
    jlong uParticipant,
    jobject jqosHolder)
{
    saj_returnCode retcode = SAJ_RETCODE_BAD_PARAMETER;
    u_participantQos uQos;
    jobject jQos = NULL;

    assert(jqosHolder != NULL);
    OS_UNUSED_ARG(this);

    retcode = saj_retcode_from_user_result(u_participantGetQos(SAJ_VOIDP(uParticipant), &uQos));
    if (retcode == SAJ_RETCODE_OK){
        retcode = saj_domainParticipantQosCopyOut(env, uQos, &jQos);
        u_participantQosFree(uQos);
        if (retcode == SAJ_RETCODE_OK){
            SET_OBJECT_FIELD(env, jqosHolder, domainParticipantQosHolder_value, jQos);
            DELETE_LOCAL_REF(env, jQos);
        }
    }

    return retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

/*
 * Method:    jniAssertLiveliness
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniAssertLiveliness) (
    JNIEnv *env,
    jobject this,
    jlong uParticipant)
{
    u_result uResult;
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(this);
    uResult = u_participantAssertLiveliness(SAJ_VOIDP(uParticipant));
    return saj_retcode_from_user_result(uResult);
}

/*
 * Class:     org_opensplice_dds_dcps_DomainParticipantImpl
 * Method:    jniGetCurrentTime
 * Signature: (LDDS/Time_tHolder;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetCurrentTime) (
    JNIEnv *env,
    jobject this,
    jlong uParticipant,
    jobject jtimeHolder)
{
    os_timeW current_time;
    saj_returnCode retcode;
    jobject timestamp;

    assert(jtimeHolder != NULL);

    OS_UNUSED_ARG(this);
    OS_UNUSED_ARG(uParticipant);

    /* TODO: time should be a domain specific time. */
    current_time = os_timeWGet();
    timestamp = GET_OBJECT_FIELD(env, jtimeHolder, time_tHolder_value);
    retcode = saj_timeCopyOut(env, current_time, &timestamp);
    if (retcode == SAJ_RETCODE_OK){
        SET_OBJECT_FIELD(env, jtimeHolder, time_tHolder_value, timestamp);
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}


JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniIgnoreParticipant) (
    JNIEnv *env,
    jobject this,
    jlong uParticipant,
    jlong handle)
{
    u_result uResult;
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(this);
    uResult = u_participantIgnoreParticipant(SAJ_VOIDP(uParticipant),handle);
    return saj_retcode_from_user_result(uResult);
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniIgnoreSubscription) (
    JNIEnv *env,
    jobject this,
    jlong uParticipant,
    jlong handle)
{
    u_result uResult;
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(this);
    uResult = u_participantIgnoreSubscription(SAJ_VOIDP(uParticipant),handle);
    return saj_retcode_from_user_result(uResult);
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniIgnoreTopic) (
    JNIEnv *env,
    jobject this,
    jlong uParticipant,
    jlong handle)
{
    u_result uResult;
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(this);
    uResult = u_participantIgnoreTopic(SAJ_VOIDP(uParticipant),handle);
    return saj_retcode_from_user_result(uResult);
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniIgnorePublication) (
    JNIEnv *env,
    jobject this,
    jlong uParticipant,
    jlong handle)
{
    u_result uResult;
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(this);
    uResult = u_participantIgnorePublication(SAJ_VOIDP(uParticipant),handle);
    return saj_retcode_from_user_result(uResult);
}

/*
 * Class:     org_opensplice_dds_dcps_DomainParticipantImpl
 * Method:    jniRegisterType
 * Signature: (Ljava/lang/Object;LDDS/DomainParticipant;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniRegisterType) (
    JNIEnv *env,
    jclass object,
    jlong uParticipant,
    jstring type_name,
    jstring descriptor,
    jshort data_representation_id,
    jbyteArray type_hash,
    jbyteArray meta_data,
    jbyteArray extentions)
{
    saj_returnCode retcode;
    u_result uResult;
    u_domain uDomain;
    const os_char *desc;
    jbyte *hash;
    os_int32 hashLen;
    C_STRUCT(u_typeRepresentation) tr;

    assert(type_name);
    assert(descriptor);
    OS_UNUSED_ARG(object);

    uDomain = u_participantDomain(SAJ_VOIDP(uParticipant));
    desc = GET_STRING_UTFCHAR(env, descriptor, 0);
    uResult = u_domain_load_xml_descriptor(uDomain, desc);

    RELEASE_STRING_UTFCHAR(env, descriptor, desc);
    if ((uResult == U_RESULT_OK) &&
        (data_representation_id != V_OSPL_REPRESENTATION)) {
        assert(type_hash);
        assert(meta_data);

        memset(&tr, 0, sizeof(tr));
        tr.typeName = GET_STRING_UTFCHAR(env, type_name, 0);
        tr.dataRepresentationId = data_representation_id;
        hash = GET_BYTE_ARRAY_ELEMENTS(env, type_hash, NULL);
        hashLen = GET_ARRAY_LENGTH(env, type_hash);
        tr.typeHash = u_typeHashFromArray((const os_uchar*)hash, hashLen);
        RELEASE_BYTE_ARRAY_ELEMENTS(env, type_hash, hash, JNI_ABORT);

        tr.metaData = (const os_uchar*)GET_BYTE_ARRAY_ELEMENTS(env, meta_data, NULL);
        tr.metaDataLen = GET_ARRAY_LENGTH(env, meta_data);
        if (extentions) {
            tr.extentions = (const os_uchar*)GET_BYTE_ARRAY_ELEMENTS(env, extentions, NULL);
            tr.extentionsLen = GET_ARRAY_LENGTH(env, extentions);
        }

        uResult = u_participantRegisterTypeRepresentation(SAJ_VOIDP(uParticipant), &tr);
        if (extentions) {
            RELEASE_BYTE_ARRAY_ELEMENTS(env, extentions, (jbyte *)tr.extentions, JNI_ABORT);
        }
        RELEASE_BYTE_ARRAY_ELEMENTS(env, meta_data, (jbyte *)tr.metaData, JNI_ABORT);
        RELEASE_STRING_UTFCHAR(env, type_name, tr.typeName);
    }

    retcode = saj_retcode_from_user_result(uResult);
    return (jint)retcode;
    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

struct copy_cache_data {
    JNIEnv *env;
    const os_char *typeName;
    const os_char *typeAlias;
    const os_char *packageRedirects;
    saj_copyCache copyCache;
};

static void
create_copy_cache (
    v_public e,
    c_voidp arg)
{
    struct copy_cache_data *data = (struct copy_cache_data *)arg;

    c_base base;
    c_metaObject type;

    base = c_getBase(c_object(e));
    type = c_metaResolve (c_metaObject(base), data->typeName);
    data->copyCache = saj_copyCacheNew (data->env, type, data->packageRedirects);
    c_free(type);
}

/*
 * Class:     org_opensplice_dds_dcps_DomainParticipantImpl
 * Method:    jniAlloc
 * Signature: (Ljava/lang/Object;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniCopyCacheNew) (
    JNIEnv *env,
    jclass object,
    jlong uParticipant,
    jstring type_name,
    jstring type_alias,
    jstring package_redirects)
{
    struct copy_cache_data data;

    OS_UNUSED_ARG(object);

    (void)memset (&data, 0, sizeof (struct copy_cache_data));
    data.env = env;

    if (type_alias != NULL) {
        data.typeAlias = GET_STRING_UTFCHAR(env, type_alias, 0);
    }
    if (type_name != NULL) {
        data.typeName = GET_STRING_UTFCHAR(env, type_name, 0);
    }
    if (package_redirects != NULL) {
        data.packageRedirects = GET_STRING_UTFCHAR(env, package_redirects, 0);
    }

    (void)u_observableAction(SAJ_VOIDP(uParticipant), create_copy_cache, (c_voidp)&data);

    if (data.typeAlias != NULL) {
        RELEASE_STRING_UTFCHAR(env, type_alias, data.typeAlias);
    }
    if (data.typeName != NULL) {
        RELEASE_STRING_UTFCHAR(env, type_name, data.typeName);
    }
    if (data.packageRedirects) {
        RELEASE_STRING_UTFCHAR(env, package_redirects, data.packageRedirects);
    }

    return SAJ_JLONG(data.copyCache);
    CATCH_EXCEPTION: return 0;
}

/*
 * Method: jniDeleteHistoricalData
 * Param : partition_expression
 * Param : topic_expression
 * Return: Return Code
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDeleteHistoricalData) (
    JNIEnv  *env,
    jobject this,
    jlong uParticipant,
    jstring jpartition_expression,
    jstring jtopic_expression)
{
    saj_returnCode retcode = SAJ_RETCODE_OK;
    const os_char* partition_expression = NULL;
    const os_char* topic_expression= NULL;
    OS_UNUSED_ARG(this);

    /* process the jstring objects */
    if (jpartition_expression != NULL){
        partition_expression = GET_STRING_UTFCHAR(env, jpartition_expression, 0);
        if (partition_expression == NULL) {
            retcode = SAJ_RETCODE_ERROR;
        }
    } else {
        retcode = SAJ_RETCODE_BAD_PARAMETER;
    }
    if (retcode == SAJ_RETCODE_OK) {
        if (jtopic_expression != NULL){
            topic_expression = GET_STRING_UTFCHAR(env, jtopic_expression, 0);
            if (topic_expression == NULL) {
                retcode = SAJ_RETCODE_ERROR;
            }
        } else {
            retcode = SAJ_RETCODE_BAD_PARAMETER;
        }

        if (retcode == SAJ_RETCODE_OK) {
            retcode = saj_retcode_from_user_result(u_participantDeleteHistoricalData(SAJ_VOIDP(uParticipant), partition_expression, topic_expression));
            if (jpartition_expression != NULL){
                RELEASE_STRING_UTFCHAR(env, jpartition_expression, partition_expression);
            }
            if (jtopic_expression != NULL){
                RELEASE_STRING_UTFCHAR(env, jtopic_expression, topic_expression);
            }
        }
    }

    return (jint)retcode;
    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

/*
 * Class:     org_opensplice_dds_dcps_DomainparticipantImpl
 * Method:    jniSetProperty
 * Signature: (LDDS/Property;)I
 */

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetProperty) (
    JNIEnv * env,
    jobject this,
    jlong uParticipant,
    jobject jprop)
{
    saj_returnCode retcode = SAJ_RETCODE_OK;
    jstring jname, jvalue;
    const char *name, *value;

    OS_UNUSED_ARG(this);
    assert(jprop != NULL);

    jname = GET_OBJECT_FIELD(env, jprop, property_name);
    if (jname != NULL) {
        name = GET_STRING_UTFCHAR(env, jname, 0);
        jvalue = GET_OBJECT_FIELD(env, jprop, property_value);
        if (jvalue != NULL) {
            value = GET_STRING_UTFCHAR(env, jvalue, 0);
            retcode = saj_retcode_from_user_result(u_entitySetProperty(SAJ_VOIDP(uParticipant), name, value));
            RELEASE_STRING_UTFCHAR(env, jvalue, value);
            DELETE_LOCAL_REF(env, jvalue);
        } else {
            retcode = SAJ_RETCODE_BAD_PARAMETER;
        } 
        RELEASE_STRING_UTFCHAR(env, jname, name);
        DELETE_LOCAL_REF(env, jname);
    } else {
        retcode = SAJ_RETCODE_BAD_PARAMETER;
    }

    return retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

/*
 * Class:     org_opensplice_dds_dcps_DomainparticipantImpl
 * Method:    jniGetProperty
 * Signature: (LDDS/Property;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetProperty) (
    JNIEnv * env,
    jobject this,
    jlong uParticipant,
    jobject jprop)
{
    saj_returnCode retcode = SAJ_RETCODE_OK;
    jstring jname, jvalue;
    jobject jpropObject;
    const char *name;
    char *value=NULL;

    OS_UNUSED_ARG(this);
    assert(jprop != NULL);
    jpropObject = GET_OBJECT_FIELD(env, jprop, propertyHolder_value);
    if (jpropObject != NULL) {
        jname = GET_OBJECT_FIELD(env, jpropObject, property_name);
        if (jname != NULL) {
           name = GET_STRING_UTFCHAR(env, jname, 0);
            retcode = saj_retcode_from_user_result(u_entityGetProperty(SAJ_VOIDP(uParticipant), name, &value));
            if (retcode == SAJ_RETCODE_OK) {
                jvalue = NEW_STRING_UTF(env, value);
                if (jvalue != NULL) {
                    SET_OBJECT_FIELD(env, jpropObject, property_value, jvalue);
                } else {
                    retcode = SAJ_RETCODE_ERROR;
                }
            }
           RELEASE_STRING_UTFCHAR(env, jname, name);
           DELETE_LOCAL_REF(env, jname);
        } else {
            retcode = SAJ_RETCODE_BAD_PARAMETER;
        }
        DELETE_LOCAL_REF(env, jpropObject);
    } else {
        retcode = SAJ_RETCODE_BAD_PARAMETER;
    }
    return retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

#undef SAJ_FUNCTION
