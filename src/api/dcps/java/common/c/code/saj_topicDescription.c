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
#include "saj_TopicDescription.h"
#include "saj_utilities.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_TopicDescriptionImpl_##name

/**
 * Class:     org_opensplice_dds_dcps_TopicDescriptionImpl
 * Method:    jniGetTypeName
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
SAJ_FUNCTION(jniGetTypeName)(
    JNIEnv *env, 
    jobject jdescription)
{
    gapi_topicDescription description;
    jstring jtypeName = NULL;
    gapi_string typeName;
    
    description = (gapi_topicDescription) saj_read_gapi_address(env, jdescription);
    typeName = gapi_topicDescription_get_type_name(description);
    
    if(typeName != NULL){
        jtypeName = (*env)->NewStringUTF(env, typeName);
        gapi_free(typeName);
    }
    return jtypeName;
}

/**
 * Class:     org_opensplice_dds_dcps_TopicDescriptionImpl
 * Method:    jniGetName
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
SAJ_FUNCTION(jniGetName)(
    JNIEnv *env,
    jobject jdescription)
{
    gapi_topicDescription description;
    jstring jname = NULL;
    gapi_string name;
    
    description = (gapi_topicDescription) saj_read_gapi_address(env, jdescription);
    name = gapi_topicDescription_get_name(description);
    
    if(name != NULL){
        jname = (*env)->NewStringUTF(env, name);
        gapi_free(name);
    }
    return jname;
}

/**
 * Class:     org_opensplice_dds_dcps_TopicDescriptionImpl
 * Method:    jniGetParticipant
 * Signature: ()LDDS/DomainParticipant;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetParticipant)(
    JNIEnv *env,
    jobject jdescription)
{
    gapi_topicDescription description;
    gapi_domainParticipant participant;
    jobject jparticipant;
    
    jparticipant = NULL;
    description = (gapi_topicDescription) saj_read_gapi_address(env, jdescription);
    participant = gapi_topicDescription_get_participant(description);
    
    if(participant != NULL){
        jparticipant = saj_read_java_address(participant);
    }
    return jparticipant;
}
  
#undef SAJ_FUNCTION
