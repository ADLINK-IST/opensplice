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
#include "saj_MultiTopic.h"
#include "saj_utilities.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_MultiTopicImpl_##name

/**
 * Class:     org_opensplice_dds_dcps_MultiTopicImpl
 * Method:    jniGetSubscriptionExpression
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
SAJ_FUNCTION(jniGetSubscriptionExpression)(
    JNIEnv *env,
    jobject jmultiTopic)
{
    gapi_multiTopic multiTopic;
    jstring jexpression = NULL;
    gapi_string expression;
    
    multiTopic = (gapi_multiTopic) saj_read_gapi_address(env, jmultiTopic);
    expression = gapi_multiTopic_get_subscription_expression(multiTopic);
    
    if(expression != NULL){
        jexpression = (*env)->NewStringUTF(env, expression);
        gapi_free(expression);
    }
    return jexpression;
}

/**
 * Class:     org_opensplice_dds_dcps_MultiTopicImpl
 * Method:    jniGetExpressionParameters
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetExpressionParameters)(
    JNIEnv *env,
    jobject jmultiTopic,
    jobject jseqHolder)
{
    gapi_multiTopic multiTopic;
    gapi_stringSeq *seq;
    jobjectArray jseq;
    saj_returnCode rc;
    gapi_returnCode_t result;
    
    if(jseqHolder){
        multiTopic = (gapi_multiTopic) saj_read_gapi_address(env, jmultiTopic);
        seq = gapi_stringSeq__alloc();
        
        if(seq){
            seq->_maximum = 0;
            seq->_length = 0;
            seq->_release = 0;
            seq->_buffer = NULL;
            
            result = gapi_multiTopic_get_expression_parameters(multiTopic, seq);
            
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
 * Class:     org_opensplice_dds_dcps_MultiTopicImpl
 * Method:    jniSetExpressionParameters
 * Signature: ([Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetExpressionParameters)(
    JNIEnv *env,
    jobject jmultiTopic,
    jobjectArray jexprArray)
{
    gapi_multiTopic multiTopic;
    gapi_stringSeq *seq;
    saj_returnCode rc;
    jint jresult;
    
    multiTopic = (gapi_multiTopic) saj_read_gapi_address(env, jmultiTopic);
    jresult = (jint)GAPI_RETCODE_BAD_PARAMETER;
    
    rc = SAJ_RETCODE_OK;
    seq = gapi_stringSeq__alloc();

    if(jexprArray != NULL){
        rc = saj_stringSequenceCopyIn(env, jexprArray, seq);
    }
        
    if(rc == SAJ_RETCODE_OK){
        jresult = (jint)gapi_multiTopic_set_expression_parameters(
                                                            multiTopic, seq);
        
    }
    gapi_free(seq);
    
    return jresult;
}

#undef SAJ_FUNCTION
