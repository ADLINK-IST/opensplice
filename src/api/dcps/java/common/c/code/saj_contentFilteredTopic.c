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
#include "saj_ContentFilteredTopic.h"
#include "saj_utilities.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_ContentFilteredTopicImpl_##name

/**
 * Class:     org_opensplice_dds_dcps_ContentFilteredTopicImpl
 * Method:    jniGetFilterExpression
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
SAJ_FUNCTION(jniGetFilterExpression)(
    JNIEnv *env,
    jobject jcfTopic)
{
    gapi_contentFilteredTopic cfTopic;
    jstring jexpression;
    gapi_string expression;
    
    jexpression = NULL;
    cfTopic = (gapi_contentFilteredTopic) saj_read_gapi_address(env, jcfTopic);
    expression = gapi_contentFilteredTopic_get_filter_expression(cfTopic);
    
    if(expression != NULL){
        jexpression = (*env)->NewStringUTF(env, expression);
        gapi_free(expression);
    }
    return jexpression;
}

/**
 * Class:     org_opensplice_dds_dcps_ContentFilteredTopicImpl
 * Method:    jniGetExpressionParameters
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetExpressionParameters)(
    JNIEnv *env,
    jobject jcfTopic,
    jobject jseqHolder)
{
    gapi_contentFilteredTopic cfTopic;
    gapi_stringSeq *seq;
    jobjectArray jseq;
    saj_returnCode rc;
    gapi_returnCode_t result;
    
    if(jseqHolder){
        cfTopic = (gapi_contentFilteredTopic) saj_read_gapi_address(env, jcfTopic);
        seq = gapi_stringSeq__alloc();
        
        if(seq){
            seq->_maximum = 0;
            seq->_length = 0;
            seq->_release = 0;
            seq->_buffer = NULL;
            
            result = gapi_contentFilteredTopic_get_expression_parameters(cfTopic, seq);
            
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
 * Class:     org_opensplice_dds_dcps_ContentFilteredTopicImpl
 * Method:    jniSetExpressionParameters
 * Signature: ([Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetExpressionParameters)(
    JNIEnv *env,
    jobject jcfTopic,
    jobjectArray jexprArray)
{
    gapi_contentFilteredTopic cfTopic;
    gapi_stringSeq *seq;
    saj_returnCode rc;
    jint jresult;
    
    jresult = (jint)GAPI_RETCODE_BAD_PARAMETER;
    rc = SAJ_RETCODE_OK;
        
    cfTopic = (gapi_contentFilteredTopic) saj_read_gapi_address(env, jcfTopic);
    seq = gapi_stringSeq__alloc();
    
    if(jexprArray != NULL){    
        rc = saj_stringSequenceCopyIn(env, jexprArray, seq);
    }
    if(rc == SAJ_RETCODE_OK){
        jresult = (jint)gapi_contentFilteredTopic_set_expression_parameters(
                                                                cfTopic, seq);
        if(jexprArray != NULL){
            gapi_free(seq);
        }
    }
    return jresult;
}   

/**
 * Class:     org_opensplice_dds_dcps_ContentFilteredTopicImpl
 * Method:    jniGetRelatedTopic
 * Signature: ()LDDS/Topic;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetRelatedTopic)(
    JNIEnv *env,
    jobject jcfTopic)
{
    jobject jtopic;
    gapi_contentFilteredTopic cfTopic;
    gapi_topic topic;
    
    jtopic = NULL;
    
    cfTopic = (gapi_contentFilteredTopic)saj_read_gapi_address(env, jcfTopic);
    topic = gapi_contentFilteredTopic_get_related_topic(cfTopic);
    
    if (topic != GAPI_OBJECT_NIL){
        jtopic = saj_read_java_address(topic);
    }
    return jtopic;
}


#undef SAJ_FUNCTION
