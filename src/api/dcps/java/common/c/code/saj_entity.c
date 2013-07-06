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

#include "saj_Entity.h"
#include "saj_utilities.h"
#include "saj_qosUtils.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_EntityImpl_##name

/**
 * Class:     org_opensplice_dds_dcps_EntityImpl
 * Method:    jniEnable
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniEnable)(
    JNIEnv *env,
    jobject jentity)
{
    gapi_entity entity;

    entity = (gapi_entity)saj_read_gapi_address(env, jentity);
    return (jint)gapi_entity_enable(entity);
}

/**
 * Class:     org_opensplice_dds_dcps_EntityImpl
 * Method:    jniGetStatuscondition
 * Signature: ()LDDS/StatusCondition;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetStatuscondition)(
    JNIEnv *env,
    jobject jentity)
{
    gapi_entity entity;
    gapi_statusCondition condition;
    jobject jcondition;
    saj_returnCode rc;

    jcondition = NULL;
    entity = (gapi_entity)saj_read_gapi_address(env, jentity);
    condition = gapi_entity_get_statuscondition(entity);

    if(condition != GAPI_OBJECT_NIL){
        jcondition = saj_read_java_address(condition);

        if(jcondition == NULL){
            rc = saj_construct_java_object(env,
                                            PACKAGENAME "StatusConditionImpl",
                                            (PA_ADDRCAST)condition,
                                            &jcondition);
            if(rc == SAJ_RETCODE_OK){
                saj_write_java_statusCondition_address(env, entity, condition, jcondition);
            }
        }
    }
    return jcondition;
}

/**
 * Class:     org_opensplice_dds_dcps_EntityImpl
 * Method:    jniGetStatusChanges
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetStatusChanges)(
    JNIEnv *env,
    jobject jentity)
{
    gapi_entity entity;
    gapi_statusMask mask;

    entity = (gapi_entity)saj_read_gapi_address(env, jentity);
    mask = gapi_entity_get_status_changes(entity);

    return (jint)mask;
}

/**
 * Class:     org_opensplice_dds_dcps_EntityImpl
 * Method:    jniGetInstanceHandle
 * Signature: ()I
 */
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniGetInstanceHandle)(
    JNIEnv *env,
    jobject jentity)
{
    gapi_entity entity;
    gapi_instanceHandle_t handle;

    entity = (gapi_entity)saj_read_gapi_address(env, jentity);
    handle = gapi_entity_get_instance_handle(entity);

    return (jlong)handle;
}

#undef SAJ_FUNCTION
