/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#include "jni_topic.h"
#include "jni_misc.h"
#include "jni_participant.h"
#include "os_heap.h"
#include "os_stdlib.h"

jni_topic
jni_topicNew(
    jni_participant p,
    const char* name,
    const char* typeName,
    const char* keyList,
    v_topicQos qos)
{
    jni_topic topic;
    u_topic ut;
    size_t size, temp;
    
    topic = NULL;
    
    if((p != NULL) && (p->uparticipant != NULL)){
        ut = u_topicNew(p->uparticipant, name, typeName, keyList, qos);
        
        if(ut != NULL){
            temp = 1;
            topic = jni_topic(os_malloc((size_t)(C_SIZEOF(jni_topic))));
            topic->utopic = ut;
            
            size = (size_t)(strlen(name) + temp);
            jni_topicDescription(topic)->name = (char*)(os_malloc(size));
           os_strncpy(jni_topicDescription(topic)->name, name, size);
            
            size = (size_t)(strlen(typeName) + temp);
            jni_topicDescription(topic)->typeName = (char*)(os_malloc(size));
           os_strncpy(jni_topicDescription(topic)->typeName, typeName, size);
            
            if(keyList != NULL){
                size = (size_t)(strlen(keyList) + temp);
                jni_topicDescription(topic)->keyList = (char*)(os_malloc(size));
               os_strncpy(jni_topicDescription(topic)->keyList, keyList, size);
            }
            else{
                jni_topicDescription(topic)->keyList = NULL;
            }
            
            jni_topicDescription(topic)->participant = p;
        }
    }
    return topic;
}    

jni_result
jni_topicFree(
    jni_topic topic)
{
    jni_result r;
    
    if(topic != NULL){
        r = jni_convertResult(u_topicFree(topic->utopic));
        os_free(jni_topicDescription(topic)->name);
        os_free(jni_topicDescription(topic)->typeName);
        
        if(jni_topicDescription(topic)->keyList != NULL){
            os_free(jni_topicDescription(topic)->keyList);
        }
        os_free(topic);
    }
    else{
        r = JNI_RESULT_BAD_PARAMETER;
    }
    return r;
}
