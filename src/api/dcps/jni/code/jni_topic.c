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
    
    assert(name);
    assert(typeName);
    
    if((p == NULL) || (p->uparticipant == NULL)){
        goto err_badParam;
    }

    if((topic = os_malloc(sizeof *topic)) == NULL){
        goto err_mallocTopic;
    }

    jni_topicDescription(topic)->participant = p;

    if((topic->utopic = u_topicNew(p->uparticipant, name, typeName, keyList, qos)) == NULL){
        goto err_u_topicNew;
    }

    if((jni_topicDescription(topic)->name = os_strdup(name)) == NULL){
        goto err_strdupName;
    }

    if((jni_topicDescription(topic)->typeName = os_strdup(typeName)) == NULL){
        goto err_strdupTypeName;
    }

    if(keyList){
        if((jni_topicDescription(topic)->keyList = os_strdup(keyList)) == NULL){
            goto err_strdupKeyList;
        }
    } else  {
        jni_topicDescription(topic)->keyList = NULL;
    }

    return topic;

/* Error handling */
err_strdupKeyList:
    os_free(jni_topicDescription(topic)->typeName);
err_strdupTypeName:
    os_free(jni_topicDescription(topic)->name);
err_strdupName:
    /* Ignore return value since we are already in an error condition. */
    (void) u_topicFree(topic->utopic);
err_u_topicNew:
    os_free(topic);
err_mallocTopic:
err_badParam:
    return NULL;
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
