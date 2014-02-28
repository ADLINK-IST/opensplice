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
#include "saj_QueryCondition.h"
#include "saj_utilities.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_QueryConditionImpl_##name

/**
 * Class:     org_opensplice_dds_dcps_QueryConditionImpl
 * Method:    jniGetQueryExpression
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
SAJ_FUNCTION(jniGetQueryExpression)(
    JNIEnv *env,
    jobject jqueryCondition)
{
    gapi_queryCondition queryCondition;
    jstring jexpression = NULL;
    gapi_string expression;
    
    queryCondition = (gapi_queryCondition) saj_read_gapi_address(env, 
                                                        jqueryCondition);
    expression = gapi_queryCondition_get_query_expression(queryCondition);
    
    if(expression != NULL){
        jexpression = (*env)->NewStringUTF(env, expression);
        gapi_free(expression);
    }
    return jexpression;
}

/**
 * Class:     org_opensplice_dds_dcps_QueryConditionImpl
 * Method:    jniGetQueryArguments
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetQueryParameters)(
    JNIEnv *env,
    jobject jqueryCondition,
    jobject jseqHolder)
{
    gapi_queryCondition queryCondition;
    gapi_stringSeq *seq;
    jobjectArray jseq;
    saj_returnCode rc;
    gapi_returnCode_t result;
    
    if(jseqHolder){
        queryCondition = (gapi_queryCondition) saj_read_gapi_address(env, 
                                                        jqueryCondition);
        seq = gapi_stringSeq__alloc();
        
        if(seq){
            seq->_maximum = 0;
            seq->_length = 0;
            seq->_release = 0;
            seq->_buffer = NULL;
            
            result = gapi_queryCondition_get_query_parameters(queryCondition, seq);
                        
            if(result == GAPI_RETCODE_OK){
                rc = saj_stringSequenceCopyOut(env, *seq, &jseq);
                
                if(rc == SAJ_RETCODE_OK){        
                    (*env)->SetObjectField(env, jseqHolder, GET_CACHED(stringSeqHolder_stringSeq_fid), jseq);
                    (*env)->DeleteLocalRef(env, jseq);
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

/**
 * Class:     org_opensplice_dds_dcps_QueryConditionImpl
 * Method:    jniSetQueryArguments
 * Signature: ([Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetQueryParameters)(
    JNIEnv *env,
    jobject jqueryCondition,
    jobjectArray jargArray)
{
    gapi_queryCondition queryCondition;
    gapi_stringSeq *seq;
    saj_returnCode rc;
    jint jresult;
    
    rc = SAJ_RETCODE_OK;
    jresult = (jint)GAPI_RETCODE_BAD_PARAMETER;
    seq = gapi_stringSeq__alloc();
    
    if(jargArray != NULL){
        rc = saj_stringSequenceCopyIn(env, jargArray, seq);
    }
    queryCondition = (gapi_queryCondition) saj_read_gapi_address(env, 
                                                        jqueryCondition);
    
    if(rc == SAJ_RETCODE_OK){
        jresult = (jint)gapi_queryCondition_set_query_parameters(queryCondition,
                                                                seq);
    }
    gapi_free(seq);
    
    return jresult;
}

#undef SAJ_FUNCTION
