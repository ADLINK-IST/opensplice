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
#include "saj_QueryCondition.h"
#include "saj_utilities.h"
#include "u_observable.h"
#include "saj__report.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_QueryConditionImpl_##name

JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniQueryConditionNew) (
    JNIEnv  *env,
    jobject this,
    jlong uReader,
    jobject condition,
    jint sample_mask,
    jint view_mask,
    jint instance_mask,
    jobject query_expression,
    jobjectArray query_parameters)
{
    saj_queryData queryData = NULL;
    u_query uQuery = NULL;
    u_sampleMask mask;
    jobject *jParam = NULL;

    os_char *name = NULL;
    const os_char *predicate = NULL;

    const char **params = NULL; /* Const because it will hold const char *elements. */
    os_uint32 nrOfParams = 0;
    os_uint32 i;

    OS_UNUSED_ARG(this);

    if ((uReader != 0) && (query_expression != NULL)) {
        predicate = GET_STRING_UTFCHAR(env, query_expression, 0);
        if (predicate != NULL) {
            if (query_parameters){
                nrOfParams = GET_ARRAY_LENGTH(env, query_parameters);
                if(nrOfParams > 0) {
                    jParam = os_malloc(nrOfParams * sizeof(jobject));
                    params = os_malloc(nrOfParams * sizeof(char *));
                    for (i =0; i<nrOfParams; i++) {
                        jParam[i] = GET_OBJECTARRAY_ELEMENT(env, query_parameters, i);
                        params[i] = GET_STRING_UTFCHAR(env, jParam[i], 0);
                    }
                }
            }
            mask = DDS_SAMPLE_MASK(sample_mask, view_mask, instance_mask);
            uQuery = u_queryNew(SAJ_VOIDP(uReader), name, predicate, params, nrOfParams, mask);
            for (i =0; i<nrOfParams; i++) {
                RELEASE_STRING_UTFCHAR(env, jParam[i], params[i]);
                DELETE_LOCAL_REF(env, jParam[i]);
            }
            if (params != NULL) {
                os_free((void *) params);
            }
            if (jParam != NULL) {
                os_free(jParam);
            }
            if (uQuery != NULL) {
                queryData = os_malloc(sizeof(C_STRUCT(saj_queryData)));
                queryData->condition = NEW_GLOBAL_REF(env, condition);
                queryData->mask = DDS_SAMPLE_MASK(sample_mask, view_mask, instance_mask);
                u_observableSetUserData(u_observable(uQuery), queryData);
            }
            RELEASE_STRING_UTFCHAR(env, query_expression, predicate);
        }
    }

    return SAJ_JLONG(uQuery);

    CATCH_EXCEPTION:
    return 0;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniQueryConditionFree) (
    JNIEnv  *env,
    jobject this,
    jlong uQuery)
{
    int result = SAJ_RETCODE_ALREADY_DELETED;
    u_result uResult;
    saj_queryData queryData;

    OS_UNUSED_ARG(this);

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

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetQueryParameters) (
    JNIEnv  *env,
    jobject this,
    jlong uQuery,
    jobjectArray query_parameters)
{
    int retcode = SAJ_RETCODE_ALREADY_DELETED;
    u_result uResult;
    jobject *jParam = NULL;
    const char **params = NULL; /* Const because it will hold const char *elements. */
    os_uint32 nrOfParams = 0;
    os_uint32 i;

    OS_UNUSED_ARG(this);

    if (query_parameters){
        nrOfParams = GET_ARRAY_LENGTH(env, query_parameters);
        if(nrOfParams > 0) {
            jParam = os_malloc(nrOfParams * sizeof(jobject));
            params = os_malloc(nrOfParams * sizeof(char *));
            for (i =0; i<nrOfParams; i++) {
                jParam[i] = GET_OBJECTARRAY_ELEMENT(env, query_parameters, i);
                params[i] = GET_STRING_UTFCHAR(env, jParam[i], 0);
            }
        }
    }
    uResult = u_querySet(SAJ_VOIDP(uQuery), params, nrOfParams);
    retcode = saj_retcode_from_user_result(uResult);
    for (i =0; i<nrOfParams; i++) {
        RELEASE_STRING_UTFCHAR(env, jParam[i], params[i]);
        DELETE_LOCAL_REF(env, jParam[i]);
    }
    if (params != NULL) {
        os_free((void *) params);
    }
    if (jParam != NULL) {
        os_free(jParam);
    }

    return retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

#undef SAJ_FUNCTION
