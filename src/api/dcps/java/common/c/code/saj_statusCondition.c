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

#include "saj_StatusCondition.h"
#include "saj_utilities.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_StatusConditionImpl_##name

/**
 * Class:     org_opensplice_dds_dcps_StatusConditionImpl
 * Method:    jniGetEnabledStatuses
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetEnabledStatuses)(
    JNIEnv *env,
    jobject jstatusCondition)
{
    gapi_statusCondition statusCondition;
    
    statusCondition = (gapi_statusCondition) saj_read_gapi_address(env, jstatusCondition);

    return (jint)gapi_statusCondition_get_enabled_statuses(statusCondition);
}

/**
 * Class:     org_opensplice_dds_dcps_StatusConditionImpl
 * Method:    jniSetEnabledStatuses
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetEnabledStatuses)(
    JNIEnv *env,
    jobject jstatusCondition,
    jint jmask)
{
    gapi_statusCondition statusCondition;
    
    statusCondition = (gapi_statusCondition) saj_read_gapi_address(env, jstatusCondition);

    return (jint)gapi_statusCondition_set_enabled_statuses(statusCondition, 
                                            (const gapi_statusMask)jmask);
}

/**
 * Class:     org_opensplice_dds_dcps_StatusConditionImpl
 * Method:    jniGetEntity
 * Signature: ()LDDS/Entity;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetEntity)(
    JNIEnv *env,
    jobject jstatusCondition)
{
    gapi_statusCondition statusCondition;
    gapi_entity entity;
    jobject jentity;
    
    jentity = NULL;
    statusCondition = (gapi_statusCondition) saj_read_gapi_address(env, jstatusCondition);
    entity = gapi_statusCondition_get_entity(statusCondition);
    
    if (entity != NULL){
        jentity = saj_read_java_address(entity);
    }
    return jentity;
}

#undef SAJ_FUNCTION
