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

#include "saj_GuardCondition.h"
#include "saj_utilities.h"

#define SAJ_FUNCTION(name) Java_DDS_GuardCondition_##name

/**
 * Class:     DDS_GuardCondition
 * Method:    jniGuardConditionAlloc
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL
SAJ_FUNCTION(jniGuardConditionAlloc)(
    JNIEnv *env,
    jobject jguardCondition)
{
    gapi_guardCondition guardCondition;
    jboolean jresult;
    
    jresult = JNI_FALSE;
    guardCondition = gapi_guardCondition__alloc();
    
    if(guardCondition != NULL){
        saj_register_weak_java_object(env, (PA_ADDRCAST)guardCondition, 
                                                            jguardCondition);
        jresult = JNI_TRUE;
    }
    return jresult;
}

/**
 * Class:     DDS_GuardCondition
 * Method:    jniGuardConditionFree
 * Signature: ()V
 */
JNIEXPORT void JNICALL
SAJ_FUNCTION(jniGuardConditionFree)(
    JNIEnv *env, 
    jobject jguardCondition)
{
    gapi_guardCondition guardCondition;
    saj_userData ud;
    
    guardCondition = (gapi_waitSet) saj_read_gapi_address(env, jguardCondition);
    ud = gapi_object_get_user_data(guardCondition);
    saj_destroy_weak_user_data(env, ud);
    gapi_free(guardCondition);
    
    return;
}
  
/**
 * Class:     org_opensplice_dds_dcps_GuardConditionImpl
 * Method:    jniSetTriggerValue
 * Signature: (Z)V
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetTriggerValue)(
    JNIEnv *env,
    jobject jguardCondition,
    jboolean jvalue)
{
    gapi_guardCondition guardCondition;
    
    guardCondition = (gapi_guardCondition) saj_read_gapi_address(env, 
                                                            jguardCondition);
    return (jint)gapi_guardCondition_set_trigger_value(guardCondition, 
                                                    (const gapi_boolean)jvalue);
}

#undef SAJ_FUNCTION
