/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "saj_WaitSet.h"
#include "saj_utilities.h"

#define SAJ_FUNCTION(name) Java_DDS_WaitSet_##name

/**
 * Class:     DDS_WaitSet
 * Method:    jniWaitSetAlloc
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL
SAJ_FUNCTION(jniWaitSetAlloc)(
    JNIEnv *env,
    jobject jwaitSet)
{
    gapi_waitSet waitSet;
    jboolean jresult;
    
    jresult = JNI_FALSE;
    waitSet = gapi_waitSet__alloc();
 
    if(waitSet != NULL){
        saj_register_weak_java_object(env, (PA_ADDRCAST)waitSet, jwaitSet);
        jresult = JNI_TRUE;
    }
    return jresult;
}

/**
 * Class:     DDS_WaitSet
 * Method:    jniWaitSetFree
 * Signature: ()V
 */
JNIEXPORT void JNICALL
SAJ_FUNCTION(jniWaitSetFree)(
    JNIEnv *env,
    jobject jwaitSet)
{
    gapi_waitSet waitSet;
    saj_userData ud;
    
    waitSet = (gapi_waitSet) saj_read_gapi_address(env, jwaitSet);
    ud = gapi_object_get_user_data(waitSet);
    
    if(ud != NULL){
        saj_destroy_weak_user_data(env, ud);
        gapi_free(waitSet);
    }    
    return;
}

/**
 * Class:     DDS_WaitSet
 * Method:    jniWait
 * Signature: (LDDS/ConditionSeqHolder;LDDS/Duration_t;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniWait)(
    JNIEnv *env,
    jobject jwaitSet,
    jobject jseqHolder,
    jobject jduration)
{
    gapi_waitSet waitSet;
    gapi_conditionSeq *conditionSeq;
    gapi_duration_t duration;
    jobjectArray jConditionSeq;
    saj_returnCode rc;
    gapi_returnCode_t result;
    
    if(jseqHolder != NULL){
        jConditionSeq = NULL;
        waitSet = (gapi_waitSet) saj_read_gapi_address(env, jwaitSet);
        rc = saj_durationCopyIn(env, jduration, &duration);
        
        if(rc == SAJ_RETCODE_OK){
            conditionSeq = gapi_conditionSeq__alloc();
            
            if(conditionSeq){
                conditionSeq->_maximum = 0;
                conditionSeq->_length = 0;
                conditionSeq->_release = 0;
                conditionSeq->_buffer = NULL;
            
                result = gapi_waitSet_wait(waitSet, conditionSeq, &duration);

                if((result == GAPI_RETCODE_OK) || (result == GAPI_RETCODE_TIMEOUT)){
                    rc = saj_LookupExistingConditionSeq(env, conditionSeq, &jConditionSeq);
                
                    if ( rc == SAJ_RETCODE_OK){
                        (*env)->SetObjectField(
                            env, 
                            jseqHolder,
                            GET_CACHED(conditionSeqHolder_value_fid),
                            jConditionSeq);
                    } else {
                        result = GAPI_RETCODE_ERROR;
                    }
                }
                gapi_free(conditionSeq);
            } else {
                result = GAPI_RETCODE_OUT_OF_RESOURCES;
            }
        } else {
            result = GAPI_RETCODE_ERROR;
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return (jint)result;
}

/**
 * Class:     DDS_WaitSet
 * Method:    jniAttachCondition
 * Signature: (LDDS/Condition;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniAttachCondition)(
    JNIEnv *env,
    jobject jwaitSet,
    jobject jcondition)
{
    gapi_waitSet waitSet;
    gapi_condition condition;
    
    waitSet = (gapi_waitSet) saj_read_gapi_address(env, jwaitSet);
    condition = (gapi_condition) saj_read_gapi_address(env, jcondition);

    return (jint)gapi_waitSet_attach_condition(waitSet, condition);
}

/**
 * Class:     DDS_WaitSet
 * Method:    jniDetachCondition
 * Signature: (LDDS/Condition;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDetachCondition)(
    JNIEnv *env,
    jobject jwaitSet,
    jobject jcondition)
{
    gapi_waitSet waitSet;
    gapi_condition condition;
    
    waitSet = (gapi_waitSet) saj_read_gapi_address(env, jwaitSet);
    condition = (gapi_condition) saj_read_gapi_address(env, jcondition);

    return (jint)gapi_waitSet_detach_condition(waitSet, condition);
}

/**
 * Class:     DDS_WaitSet
 * Method:    jniGetConditions
 * Signature: (LDDS/ConditionSeqHolder;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetConditions)(
    JNIEnv *env,
    jobject jwaitSet,
    jobject jseqHolder)
{
    gapi_waitSet waitSet;
    gapi_conditionSeq *seq;
    jobjectArray jSeq;
    saj_returnCode rc;
    gapi_returnCode_t result;
    
    if(jseqHolder != NULL){
        waitSet = (gapi_waitSet) saj_read_gapi_address(env, jwaitSet);
        seq = gapi_conditionSeq__alloc();
            
        if(seq){
            seq->_maximum = 0;
            seq->_length = 0;
            seq->_release = 0;
            seq->_buffer = NULL;
            
            result = gapi_waitSet_get_conditions(waitSet, seq);
            
            if (result == GAPI_RETCODE_OK){
                rc = saj_LookupExistingConditionSeq(env, seq, &jSeq);
                
                if (rc == SAJ_RETCODE_OK){
                    (*env)->SetObjectField(
                        env, 
                        jseqHolder,
                        GET_CACHED(conditionSeqHolder_value_fid),
                        jSeq
                    );
                } else {
                    result = GAPI_RETCODE_ERROR;
                }
            }
            gapi_free(seq);
        } else {
            result = GAPI_RETCODE_OUT_OF_RESOURCES;
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return (jint)result;
}

#undef SAJ_FUNCTION
