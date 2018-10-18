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
#include "saj_utilities.h"
#include "u_observable.h"
#include "saj__report.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_WaitSetImpl_##name

/**
 * Class:     DDS_WaitSet
 * Method:    jniWaitSetNew
 * Signature: ()Z
 */
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniWaitSetNew)(
    JNIEnv *env,
    jclass jwaitSetClass)
{
    u_waitset uWaitSet = NULL;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jwaitSetClass);

    if (GET_CACHED(condition_class) == NULL){
        jmethodID getTrigggerValue_mid;
        jclass tempClass = FIND_CLASS(env, "DDS/ConditionOperations");
        SET_CACHED(condition_ops_class, NEW_GLOBAL_REF(env, tempClass));
        DELETE_LOCAL_REF(env, tempClass);
        tempClass = FIND_CLASS(env, "DDS/Condition");
        SET_CACHED(condition_class, NEW_GLOBAL_REF(env, tempClass));
        DELETE_LOCAL_REF(env, tempClass);
        getTrigggerValue_mid = GET_METHOD_ID(env, GET_CACHED(condition_ops_class), "get_trigger_value", "()Z");
        SET_CACHED(conditionGetTriggerValue_mid, getTrigggerValue_mid);
    }
    uWaitSet = u_waitsetNew2();

    return (jlong)(PA_ADDRCAST)uWaitSet;

    CATCH_EXCEPTION:
    return 0;
}

/**
 * Class:     DDS_WaitSet
 * Method:    jniWaitSetFree
 * Signature: ()V
 */
JNIEXPORT void JNICALL
SAJ_FUNCTION(jniWaitSetFree)(
    JNIEnv *env,
    jobject jwaitSet,
    jlong uWaitset)
{
    assert(uWaitset);

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jwaitSet);

    (void)u_objectClose(SAJ_VOIDP(uWaitset));
}

typedef struct {
    JNIEnv *env;
    jobjectArray activeConditions;
    jobjectArray guards;
    jint maxConditions;
    jint nrTriggeredConditions;
} WaitActionArg;

static void addConditionToArray(
    JNIEnv *env,
    jobjectArray *triggeredConditions,
    jobject condition,
    jint *nrTriggeredConditions,
    jint maxConditions)
{
    jint curLength;

    curLength = GET_ARRAY_LENGTH(env, *triggeredConditions);
    if (curLength < ++(*nrTriggeredConditions))
    {
        jint i;
        jint newLength;
        jobjectArray tmpCondArr;

        /* When new Conditions are added to a waiting WaitSet maxConditions
         * can be too small, this could result in an OutOfBounds exception.
         * If maxConditions is too small allocate 16 extra elements to minimize
         * reallocation if multiple new Conditions are added.
         */
        newLength = (*nrTriggeredConditions) > maxConditions ? (*nrTriggeredConditions) + 16 : maxConditions;
        tmpCondArr = NEW_OBJECTARRAY(env, newLength, GET_CACHED(condition_class), NULL);
        for (i = 0; i < curLength; i++)
        {
            jobject element = GET_OBJECTARRAY_ELEMENT(env, *triggeredConditions, i);
            SET_OBJECTARRAY_ELEMENT(env, tmpCondArr, i, element);
            DELETE_LOCAL_REF(env, element);
        }
        DELETE_LOCAL_REF(env, *triggeredConditions);
        *triggeredConditions = tmpCondArr;
    }
    (*env)->SetObjectArrayElement(env, *triggeredConditions, *nrTriggeredConditions - 1, condition);
    CHECK_EXCEPTION(env);

    return;

    CATCH_EXCEPTION:
    ;/* Log some error message. */
}

static os_boolean
waitAction (
    c_voidp userData,
    c_voidp arg)
{
    jobject condition;
    os_boolean proceed = OS_TRUE;
    WaitActionArg *waitArg = (WaitActionArg *) arg;

    if (userData == NULL) {
        /* Handle guardConditions. */
        jint i;
        jint nrGuards = 0;
        if (waitArg->guards) {
            nrGuards = GET_ARRAY_LENGTH(waitArg->env, waitArg->guards);
        }
        for (i = 0; i < nrGuards; i++)
        {
            jboolean triggerValue;

            condition = GET_OBJECTARRAY_ELEMENT(waitArg->env, waitArg->guards, i);
            assert(condition);
            triggerValue = CALL_BOOLEAN_METHOD(waitArg->env, condition, GET_CACHED(conditionGetTriggerValue_mid));
            if (triggerValue)
            {
                addConditionToArray(waitArg->env, &waitArg->activeConditions,
                        condition, &waitArg->nrTriggeredConditions, waitArg->maxConditions);
            }
            DELETE_LOCAL_REF(waitArg->env, condition);
        }
        proceed = (waitArg->nrTriggeredConditions == 0) ? OS_TRUE : OS_FALSE;
    } else {
        saj_queryData queryData;

        switch (u_objectKind(userData))
        {
        case U_QUERY:
            queryData = u_observableGetUserData(userData);
            assert(queryData != NULL);
            condition = queryData->condition;
        break;
        case U_STATUSCONDITION:
            condition = (jobject) u_observableGetUserData(userData);
        break;
        default:
            /* TODO : Error report. */
            printf("jniwaitSet callback action encountered unexpected userData object: %s\n",
                    u_kindImage(u_objectKind(userData)));
            assert(0);
            condition = NULL;
        break;
        }
        if (condition) {
            addConditionToArray(waitArg->env, &waitArg->activeConditions,
                                condition, &waitArg->nrTriggeredConditions,
                                waitArg->maxConditions);
        }
    }

    CATCH_EXCEPTION:
    return proceed;
}

/**
 * Class:     DDS_WaitSet
 * Method:    jniWait
 * Signature: (LDDS/ConditionSeqHolder;LDDS/Duration_t;)I
 */
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniWait)(
    JNIEnv *env,
    jobject jwaitSet,
    jlong uWaitset,
    jobject jseqHolder,
    jobjectArray attachedGuards,
    jint maxConditions,
    jint durationSec,
    jint durationNanoSec)
{
    WaitActionArg waitArg;
    os_duration duration;
    jint result = SAJ_RETCODE_ERROR;
    jlong resultCombo;
    u_result uResult;

    OS_UNUSED_ARG(jwaitSet);

    duration = SAJ_DURATION_INIT(durationSec, durationNanoSec);
    assert(OS_DURATION_ISPOSITIVE(duration));

    waitArg.env = env;
    waitArg.nrTriggeredConditions = 0;
    waitArg.activeConditions = GET_OBJECT_FIELD(env, jseqHolder, conditionSeqHolder_value);
    waitArg.guards = attachedGuards;
    waitArg.maxConditions = maxConditions;

    uResult = u_waitsetWaitAction2(SAJ_VOIDP(uWaitset), waitAction, &waitArg, duration);
    if (uResult == U_RESULT_DETACHING) {
        result = -1;
    } else {
        result = saj_retcode_from_user_result(uResult);
    }

    if (result == SAJ_RETCODE_OK){
         SET_OBJECT_FIELD(env, jseqHolder, conditionSeqHolder_value, waitArg.activeConditions);
    }

    CATCH_EXCEPTION:
    /* resultCombo is a 64-bit combination of the returncode and the nrTriggeredConditions variable.
       It is combined in its consitituent parts below. */
    resultCombo = ((jlong) waitArg.nrTriggeredConditions) << 32 | result;

    return resultCombo;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniAttachCondition)(
    JNIEnv *env,
    jobject jwaitSet,
    jlong uWaitset,
    jlong uCondition,
    jlong userData)
{
    saj_returnCode rc = SAJ_RETCODE_BAD_PARAMETER;
    u_result uResult;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jwaitSet);

    if ((uWaitset != 0) && (uCondition != 0) && (userData != 0)) {
        uResult = u_waitsetAttach(SAJ_VOIDP(uWaitset), SAJ_VOIDP(uCondition), SAJ_VOIDP(userData));
        rc = saj_retcode_from_user_result(uResult);
    }

    return (jint)rc;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDetachCondition)(
    JNIEnv *env,
    jobject jwaitSet,
    jlong uWaitset,
    jlong uCondition)
{
    saj_returnCode rc = SAJ_RETCODE_BAD_PARAMETER;
    u_result uResult;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jwaitSet);

    if ((uWaitset != 0) && (uCondition != 0)) {
        uResult = u_waitsetDetach_s(SAJ_VOIDP(uWaitset), SAJ_VOIDP(uCondition));
        rc = saj_retcode_from_user_result(uResult);
    }

    return (jint)rc;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniTrigger)(
    JNIEnv *env,
    jobject jwaitSet,
    jlong uWaitset,
    jlong userData)
{
    saj_returnCode rc = SAJ_RETCODE_BAD_PARAMETER;
    u_result uResult;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jwaitSet);

    if (uWaitset != 0) {
        uResult = u_waitsetNotify(SAJ_VOIDP(uWaitset), SAJ_VOIDP(userData));
        rc = saj_retcode_from_user_result(uResult);
    }

    return (jint)rc;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniWaitSetGetDomainId)(
    JNIEnv *env,
    jobject jwaitSet,
    jlong uWaitset)
{
    u_domainId_t domainId = -1;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jwaitSet);

    if (uWaitset != 0) {
        domainId = u_waitsetGetDomainId(SAJ_VOIDP(uWaitset));
    }

    return (jint)domainId;
}



#undef SAJ_FUNCTION
