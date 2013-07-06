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
#include "saj_Domain.h"
#include "saj_utilities.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_DomainImpl_##name

/*
 * Class:     org_opensplice_dds_dcps_DomainImpl
 * Method:    jniCreatePersistentSnapshot
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniCreatePersistentSnapshot)(
    JNIEnv *env,
    jobject jdomain,
    jstring jpartition_expression,
    jstring jtopic_expression,
    jstring jURI)
{
    gapi_domain domain;
    gapi_returnCode_t result;
    gapi_char *partition_expression,
              *topic_expression,
              *URI;

    if(jpartition_expression != NULL){
        partition_expression = (gapi_char*)(*env)->GetStringUTFChars(env, jpartition_expression, 0);
    } else {
        partition_expression = NULL;
    }
    if(jtopic_expression != NULL){
        topic_expression = (gapi_char*)(*env)->GetStringUTFChars(env, jtopic_expression, 0);
    } else {
        topic_expression = NULL;
    }
    if(jURI != NULL){
        URI = (gapi_char*)(*env)->GetStringUTFChars(env, jURI, 0);
    } else {
        URI = NULL;
    }
    domain = (gapi_domain) saj_read_gapi_address(env, jdomain);
    result = gapi_domain_create_persistent_snapshot(domain,
                                                    partition_expression,
                                                    topic_expression,
                                                    URI);
    if(jpartition_expression != NULL){
        (*env)->ReleaseStringUTFChars(env, jpartition_expression, partition_expression);
    }
    if(jtopic_expression != NULL){
        (*env)->ReleaseStringUTFChars(env, jtopic_expression, topic_expression);
    }
    if(jURI != NULL){
        (*env)->ReleaseStringUTFChars(env, jURI, URI);
    }

    return (jint)result;
}

#undef SAJ_FUNCTION
