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

#include "jni_subscriberQos.h"
#include "jni__handler.h"
#include <jni.h>
#include "v_kernel.h"
#include "v_subscriberQos.h"
#include "os_heap.h"


const c_char*
jni_subscriberQosGetPartition(
    JNIEnv* env, 
    jobject jqos)
{
    jfieldID partitionPolicyFieldID, partitionNameFieldID;
    jclass subQosClass, partPolClass;
    jobject partitionPolicy;
    jstring partitionName;
    const c_char* partition;
    c_char* fullClassName;
    
    partition = NULL;
    
    if(jqos != NULL){
        fullClassName = jni_getFullName("policy/SubscriberQos");
        subQosClass = (*env)->FindClass(env, fullClassName);
        os_free(fullClassName);
        assert(subQosClass != NULL);
        
        fullClassName = jni_getFullName("policy/PartitionQosPolicy");
        partPolClass = (*env)->FindClass(env, fullClassName);
        os_free(fullClassName);
        assert(partPolClass != NULL);
        
        fullClassName = jni_getFullRepresentation("policy/PartitionQosPolicy");
        partitionPolicyFieldID = (*env)->GetFieldID(env, subQosClass, "partition", 
                                    fullClassName); 
        partitionPolicy = (*env)->GetObjectField(env, jqos, partitionPolicyFieldID);
        
        partitionNameFieldID = (*env)->GetFieldID(env, partPolClass, "name", "Ljava/lang/String;");
        partitionName = (jstring) ((*env)->GetObjectField(env, partitionPolicy, partitionNameFieldID)); 
        partition = (*env)->GetStringUTFChars(env, partitionName, 0);
    }    
    return partition;
}
