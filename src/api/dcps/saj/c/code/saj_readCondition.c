#include "saj_readCondition.h"
#include "saj_utilities.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_ReadConditionImpl_##name

/**
 * Class:     org_opensplice_dds_dcps_ReadConditionImpl
 * Method:    jniGetSampleStateMask
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetSampleStateMask)(
    JNIEnv *env,
    jobject jreadCondition)
{
    gapi_readCondition readCondition;
    
    readCondition = (gapi_readCondition) saj_read_gapi_address(env, jreadCondition);
    
    return (jint)gapi_readCondition_get_sample_state_mask(readCondition);
}

/**
 * Class:     org_opensplice_dds_dcps_ReadConditionImpl
 * Method:    jniGetViewStateMask
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetViewStateMask)(
    JNIEnv *env,
    jobject jreadCondition)
{
    gapi_readCondition readCondition;
    
    readCondition = (gapi_readCondition) saj_read_gapi_address(env, jreadCondition);
    
    return (jint)gapi_readCondition_get_view_state_mask(readCondition);
}

/**
 * Class:     org_opensplice_dds_dcps_ReadConditionImpl
 * Method:    jniGetInstanceStateMask
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetInstanceStateMask)(
    JNIEnv *env,
    jobject jreadCondition)
{
    gapi_readCondition readCondition;
    
    readCondition = (gapi_readCondition) saj_read_gapi_address(env, jreadCondition);
    
    return (jint)gapi_readCondition_get_instance_state_mask(readCondition);
}

/**
 * Class:     org_opensplice_dds_dcps_ReadConditionImpl
 * Method:    jniGetDatareader
 * Signature: ()LDDS/DataReader;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetDatareader)(
    JNIEnv *env,
    jobject jreadCondition)
{
    gapi_readCondition readCondition;
    gapi_dataReader dataReader;
    jobject jdataReader;
    
    jdataReader = NULL;
    readCondition = (gapi_readCondition) saj_read_gapi_address(env, jreadCondition);
    dataReader = gapi_readCondition_get_datareader(readCondition);
    
    if(dataReader != NULL){
        jdataReader = saj_read_java_address(dataReader);
    }
    return jdataReader;
}
 
#undef SAJ_FUNCTION
