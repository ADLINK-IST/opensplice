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

#include "jni_participant.h"
#include "jni_publisher.h"
#include "jni_subscriber.h"
#include "jni_writer.h"
#include "jni_reader.h"
#include "jni_misc.h"
#include "jni_topic.h"
#include "jni_partition.h"
#include "v_topic.h"
#include "u_partition.h"
#include "u_entity.h"
#include "c_stringSupport.h"
#include "c_iterator.h"
#include "os_heap.h"
#include "os_stdlib.h"

static c_bool jni_participantMayTopicBeDeleted(jni_participant p, const c_char* topicName);

jni_participant 
jni_participantNew(
    const c_char* uri,
    const char* name,
    long domainId,
    v_qos qos)
{
    jni_participant p;
    u_participant up;
    
    up = u_participantNew(uri, 0, name, qos, TRUE);
    p = NULL;
    
    if(up != NULL){
        p = os_malloc(sizeof *p);
        if (p) {
            p->uparticipant = up;
            p->domainId     = domainId;
            p->publishers   = c_iterNew(NULL);
            p->subscribers  = c_iterNew(NULL);
            p->topics       = c_iterNew(NULL);
            p->partitions   = c_iterNew(NULL);
        }
    }
    return p;
}

jni_result
jni_participantFree(
    jni_participant p)
{
    jni_result r;
    jni_partition partition;
    
    r = JNI_RESULT_OK;
    
    if(p != NULL){
        if((p->publishers != NULL) && (c_iterLength(p->publishers) != 0)){
            r = JNI_RESULT_PRECONDITION_NOT_MET;
        }
        else if((p->subscribers != NULL) && (c_iterLength(p->subscribers) != 0)){
            r = JNI_RESULT_PRECONDITION_NOT_MET;
        }
        else if((p->topics != NULL) && (c_iterLength(p->topics) != 0)){
            r = JNI_RESULT_PRECONDITION_NOT_MET;
        }
        else if((p->partitions != NULL) && (c_iterLength(p->partitions) != 0)){
            partition = jni_partition(c_iterTakeFirst(p->partitions));
            
            while((partition != NULL) && (r == JNI_RESULT_OK)){
                r = jni_partitionFree(partition);
                partition = jni_partition(c_iterTakeFirst(p->partitions));
            }
        }
        else{
          /*DO NOTHING.*/
        }
        
        if(r == JNI_RESULT_OK){
            if(p->publishers != NULL){
                c_iterFree(p->publishers);
            }
            if(p->subscribers != NULL){
                c_iterFree(p->subscribers);
            }
            if(p->topics != NULL){
                c_iterFree(p->topics);
            }
            if(p->partitions != NULL){
                c_iterFree(p->partitions);
            }
            if(p->uparticipant != NULL){
                r = jni_convertResult(u_participantFree(p->uparticipant));
            }
            else{
                r = JNI_RESULT_OK;
            }
            os_free(p);
        }        
    }
    else{
        r = JNI_RESULT_BAD_PARAMETER;
    }
    return r;
}

jni_publisher
jni_createPublisher(
    jni_participant p,
    v_publisherQos qos)
{
    jni_publisher pub;
    pub = NULL;
    
    if(p != NULL){
        pub = jni_publisherNew(p, qos);
        
        if(pub != NULL){
            p->publishers = c_iterInsert(p->publishers, pub);
        }
    }
    return pub;
}    

jni_result
jni_deletePublisher(
    jni_participant p, 
    jni_publisher pub)
{
    jni_result r;
    
    if((p == NULL) || (pub == NULL)){
        r = JNI_RESULT_BAD_PARAMETER;
    }
    else if(p->publishers == NULL){
      r = JNI_RESULT_PRECONDITION_NOT_MET;
    }
    else if(pub->participant != p){
      r = JNI_RESULT_PRECONDITION_NOT_MET;
    }
    else{
        r = jni_publisherFree(pub);
        
        if(r == JNI_RESULT_OK){
            c_iterTake(p->publishers, pub);
        }
    }
    return r;
}    

jni_subscriber
jni_createSubscriber(
    jni_participant p, 
    v_subscriberQos qos)
{
    jni_subscriber sub;
    
    sub = NULL;
    
    if( p != NULL){
        sub = jni_subscriberNew(p, qos);
        
        if(sub != NULL){
           p->subscribers = c_iterInsert(p->subscribers, sub);
        }
    }
    return sub;
}

jni_result
jni_deleteSubscriber(
    jni_participant p, 
    jni_subscriber sub)
{
    jni_result r;
    
    if((p == NULL) || (sub == NULL)){
        r = JNI_RESULT_BAD_PARAMETER;
    }
    else if(p->subscribers == NULL){
       r = JNI_RESULT_PRECONDITION_NOT_MET;
    }
    else if(sub->participant != p){
      r = JNI_RESULT_PRECONDITION_NOT_MET;
    }
    else{
        r = jni_subscriberFree(sub);
        
        if(r == JNI_RESULT_OK){
            c_iterTake(p->subscribers, sub);
        }
    }
    return r;
}

struct jni_topicArg {
    const c_char* topicName;
    c_char* keyExpr;
    u_result result;
};

void
jni_getTopicKeyExpression(
    v_entity entity,
    c_voidp args)
{
    v_kernel vk;
    c_iter vtopics;
    c_array keyList;
    c_char* keyExpr;
    c_long nrOfKeys, totalSize, i;
    c_string fieldName, actualFieldName;
    struct jni_topicArg *arg;
    
    arg = (struct jni_topicArg *)args;
    vk = v_objectKernel(entity);
    
    if(vk != NULL){
        vtopics = v_resolveTopics(vk, arg->topicName);
            
        if(c_iterLength(vtopics) == 0){
            c_iterFree(vtopics);
        }
        else{
            keyList = v_topicMessageKeyList(c_iterTakeFirst(vtopics));
            c_iterFree(vtopics);                
            nrOfKeys = c_arraySize(keyList);

            if (nrOfKeys>0) {
                totalSize = 0;
                
                for (i=0;i<nrOfKeys;i++) {
                    fieldName = c_fieldName(keyList[i]);
                    totalSize += (strlen(fieldName)+1-9/*skip 'userdata.'*/);
                }
                keyExpr = (c_char *)os_malloc((size_t)(totalSize+1));
                keyExpr[0] = 0;
                
                for (i=0;i<nrOfKeys;i++) {
                    fieldName = c_fieldName(keyList[i]);
                    actualFieldName = c_skipUntil(fieldName, ".");
                    actualFieldName++; /*skip '.' */
                    os_strcat(keyExpr,actualFieldName);
                    
                    if (i<(nrOfKeys-1)) { 
                        os_strcat(keyExpr,","); 
                    }
                }
                arg->keyExpr = keyExpr;
            } else{
                /*No keys, do nothing.*/
            }
            arg->result = U_RESULT_OK;
        }
    }
}

/**This function only works if the topic already exists.
 * In the future this must be extended.
 */
jni_topic
jni_createTopic(
    jni_participant p,
    const char* name,
    const char* typeName,
    v_topicQos qos)
{
    jni_topic topic;
    u_result result;
    struct jni_topicArg arg;
    
    topic = NULL;
    
    if((p != NULL) && (jni_lookupTopic(p, name) == NULL)){
        arg.topicName = name;
        arg.keyExpr = NULL;
        arg.result = U_RESULT_ILL_PARAM;
        result = u_entityAction(u_entity(p->uparticipant), jni_getTopicKeyExpression, &arg);       
                
        if((result == U_RESULT_OK) && (arg.result == U_RESULT_OK)){
            topic = jni_topicNew(p, name, typeName, arg.keyExpr, qos);
            
            if(arg.keyExpr != NULL){
                os_free(arg.keyExpr);
            }
            
            if(topic != NULL){
                p->topics = c_iterInsert(p->topics, topic);
            }
        }
       
    }
    return topic;
}

jni_topic
jni_lookupTopic(
    jni_participant p,
    const char* name)
{
    c_iter topics;
    jni_topic topic, temp;
    int found;
    
    topic = NULL;
    
    if((name != NULL) && (p != NULL)){
        topics = c_iterCopy(p->topics);
        found = 0;
        temp = jni_topic(c_iterTakeFirst(topics));
        
        while( (temp != NULL) && (!found)){
            
            if(strcmp(jni_topicDescription(temp)->name, name) == 0){
                topic = temp;
                found = 1;
            }
            temp = jni_topic(c_iterTakeFirst(topics));
        }
        c_iterFree(topics);
    }
    return topic;
}

jni_result
jni_deleteTopic(
    jni_participant p,
    jni_topic top)
{
    jni_result r;
    c_bool found;
        
    if((top == NULL) || (p == NULL)){
        r = JNI_RESULT_BAD_PARAMETER;
    }
    else if(p->topics == NULL){
        r = JNI_RESULT_PRECONDITION_NOT_MET;
    }
    else if( (jni_topicDescription(top)->participant) != p ){
        r = JNI_RESULT_PRECONDITION_NOT_MET;
    }
    else if(!(jni_participantMayTopicBeDeleted(p, jni_topicDescription(top)->name))){
        r = JNI_RESULT_PRECONDITION_NOT_MET;
    }
    else{
        found = c_iterContains(p->topics, top);
        
        if(!found){
            r = JNI_RESULT_PRECONDITION_NOT_MET;
        }
        else{
            r = jni_topicFree(top);
            
            if(r == JNI_RESULT_OK){
                c_iterTake(p->topics, top);
            }
        } 
    } 
    return r;
}

jni_result
jni_participantAddPartition(
    jni_participant p,
    const c_char* partitionName)
{
    jni_result r;
    c_iter partCopy;
    jni_partition partition, partNew;
    int found;
    
    r = JNI_RESULT_OK;
    
    if((p == NULL) || (partitionName == NULL)){
        r = JNI_RESULT_BAD_PARAMETER;
    }
    else if((strcmp(partitionName, "*") == 0) ||
            (strcmp(partitionName, "") == 0)){
        /*DO NOTHING*/
    } 
    else{
        /*Check if the partitionName already exists.*/
        partCopy = c_iterCopy(p->partitions);
        partition = jni_partition(c_iterTakeFirst(partCopy));
        found = 0;
        
        while((partition != NULL) && (!found)){
            if(strcmp(partition->name, partitionName) == 0){
                found = 1;
            }
            partition = jni_partition(c_iterTakeFirst(partCopy));
        }    
        c_iterFree(partCopy);
        
        if(!found){
            partNew = jni_partitionNew(p, partitionName, NULL);
            
            if(partNew == NULL){
                r = JNI_RESULT_ERROR;
            }
            else{
                p->partitions = c_iterInsert(p->partitions, partNew);
            }
        }
    }
    return r;
}

jni_result
jni_deleteParticipantEntities(
    jni_participant p)
{
    jni_result r;
    c_iter pubCopy, subCopy, topCopy, partCopy;
    jni_publisher pub;
    jni_subscriber sub;
    jni_topic top;
    jni_partition partition;
    int proceed;
    
    assert(p != NULL);
    proceed = 1;
    r = JNI_RESULT_OK;
    
    if(p->publishers != NULL){
        pubCopy = c_iterCopy(p->publishers);
        pub = jni_publisher(c_iterTakeFirst(pubCopy));
        
        while( (pub != NULL) && proceed){
            r = jni_deletePublisherEntities(pub);
            
            if(r != JNI_RESULT_OK){
                c_iterFree(pubCopy);
                proceed = 0;
            }
            if(proceed){
                r = jni_deletePublisher(p, pub);
                
                if(r != JNI_RESULT_OK){
                    c_iterFree(pubCopy);
                    proceed = 0;
                }
            }
            pub = jni_publisher(c_iterTakeFirst(pubCopy));
        }
        if(proceed){
            c_iterFree(pubCopy);
            c_iterFree(p->publishers);
            p->publishers = c_iterNew(NULL);
        }
    }
    if((p->subscribers != NULL) && proceed){
        subCopy = c_iterCopy(p->subscribers);
        sub = jni_subscriber(c_iterTakeFirst(subCopy));
        
        while((sub != NULL) && proceed){
            r = jni_deleteSubscriberEntities(sub);
            
            if(r != JNI_RESULT_OK){
                c_iterFree(subCopy);
                proceed = 0;
            }
            if(proceed){
                r = jni_deleteSubscriber(p, sub);
                
                if(r != JNI_RESULT_OK){
                    c_iterFree(subCopy);
                    proceed = 0;
                }
            }
            sub = jni_subscriber(c_iterTakeFirst(subCopy));
        }
        if(proceed){
            c_iterFree(subCopy);
            c_iterFree(p->subscribers);
            p->subscribers = c_iterNew(NULL);
        }
    }
    if((p->topics != NULL) && proceed){
        topCopy = c_iterCopy(p->topics);
        top = jni_topic(c_iterTakeFirst(topCopy));
        
        while((top != NULL) && proceed){
            r = jni_deleteTopic(p, top);
            
            if(r != JNI_RESULT_OK){
                c_iterFree(topCopy);
                proceed = 0;
            }
            
            top = jni_topic(c_iterTakeFirst(topCopy));
        }
        if(proceed){
            c_iterFree(topCopy);
            c_iterFree(p->topics);
            p->topics = c_iterNew(NULL);
        }
    }
    if((p->partitions != NULL) && proceed){
        partCopy = c_iterCopy(p->partitions);
        partition = jni_partition(c_iterTakeFirst(partCopy));
        
        while((partition != NULL) && proceed){
            r = jni_partitionFree(partition);
            
            if(r != JNI_RESULT_OK){
                c_iterFree(partCopy);
                proceed = 0;
            }
            partition = jni_partition(c_iterTakeFirst(partCopy));
        }
        if(proceed){
            c_iterFree(partCopy);
            c_iterFree(p->partitions);
            p->partitions = c_iterNew(NULL);
        }
    }
    return r;
}

static c_bool
jni_participantMayTopicBeDeleted(
    jni_participant p, 
    const c_char* topicName)
{
    c_iter pubCopy, subCopy;
    jni_subscriber sub;
    jni_publisher pub;
    jni_writer writer;
    jni_reader reader;
    c_bool mayDelete;
    
    pubCopy = c_iterCopy(p->publishers);
    pub = jni_publisher(c_iterTakeFirst(pubCopy));
    mayDelete = TRUE;
    
    while((pub != NULL) && mayDelete){
        writer = jni_publisherLookupWriter(pub, topicName);
        
        if(writer != NULL){
            mayDelete = FALSE;
        }
        pub = jni_publisher(c_iterTakeFirst(pubCopy));
    }
    c_iterFree(pubCopy);
    
    if(mayDelete){
        subCopy = c_iterCopy(p->subscribers);
        sub = jni_subscriber(c_iterTakeFirst(subCopy));
        
        while((sub != NULL) && mayDelete){
            reader = jni_subscriberLookupReader(sub, topicName);
            
            if(reader != NULL){
                mayDelete = FALSE;
            }
            sub = jni_subscriber(c_iterTakeFirst(subCopy));
        }
        c_iterFree(subCopy);
    }
    return mayDelete;
}
