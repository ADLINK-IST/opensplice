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

#include "jni_subscriber.h"
#include "jni_participant.h"
#include "jni_reader.h"
#include "jni_misc.h"
#include "jni_topic.h"
#include "u_subscriber.h"
#include "os_heap.h"

jni_subscriber
jni_subscriberNew(
    jni_participant p,
    v_subscriberQos qos)
{
    jni_subscriber s;
    u_subscriber usub;
    
    s = NULL;
    
    if((p != NULL) && (p->uparticipant != NULL)){
        usub = u_subscriberNew(p->uparticipant, NULL, qos, TRUE);
        
        if(usub != NULL){
            s = os_malloc(sizeof *s);
            if (s) {
                s->usubscriber = usub;
                s->participant = p;
                s->readers = NULL;
            } else {
                /* Ignore return value since we are already in an error condition. */
                (void) u_subscriberFree(usub);
            }
        }
    }
    return s;
}

jni_result
jni_subscriberFree(
    jni_subscriber s)
{
    jni_result r;
    
    if(s != NULL){
         
        if((s->readers != NULL) && (c_iterLength(s->readers) != 0)){
            r = JNI_RESULT_PRECONDITION_NOT_MET;
        }
        else{
            r = JNI_RESULT_OK;
            
            if(s->readers != NULL){
                c_iterFree(s->readers);
            }
            if(s->usubscriber){
                r = jni_convertResult(u_subscriberFree(s->usubscriber));
            }
            os_free(s);
        }
    }
    else{
        r = JNI_RESULT_BAD_PARAMETER;
    }
    
    return r;
}

jni_result
jni_deleteSubscriberEntities(
    jni_subscriber sub)
{
    jni_reader rea;
    c_iter copy;
    jni_result r;
    int ok = 1;
    
    if(sub != NULL){
        copy = c_iterCopy(sub->readers);
        rea = jni_reader(c_iterTakeFirst(copy));
        
        while(rea != NULL){
            r = jni_readerFree(rea);
            
            if(r != JNI_RESULT_OK){
                ok = 0;  
            }
            rea = jni_reader(c_iterTakeFirst(copy));
        }
        c_iterFree(copy);
        
        if(ok){
           c_iterFree(sub->readers);
           sub->readers = c_iterNew(NULL); 
           r = JNI_RESULT_OK;
        }
        else{
            r = JNI_RESULT_ERROR;
        }
    }
    else{
        r = JNI_RESULT_BAD_PARAMETER;
    }
    return r;
}

jni_reader
jni_createReader(
    jni_subscriber sub,
    jni_topicDescription top,
    v_readerQos qos)
{
    jni_reader reader;
    
    reader = NULL;
    
    if((sub != NULL) && (top != NULL)){
        reader = jni_readerNew(sub, top, qos);
        
        if(reader != NULL){
            sub->readers = c_iterInsert(sub->readers, reader);
        }
    }
    return reader;
}

jni_result
jni_deleteReader(
    jni_subscriber sub,
    jni_reader reader)
{
    jni_result r;
    c_bool found;
    
    if((sub == NULL) || (reader == NULL)){
        r = JNI_RESULT_BAD_PARAMETER;
    }   
    else if(reader->subscriber != sub){
        r = JNI_RESULT_PRECONDITION_NOT_MET;
    }
    else{
        found = c_iterContains(sub->readers, reader);
        
        if(found){
            c_iterTake(sub->readers, reader);
            r = jni_readerFree(reader);
        
            if(r != JNI_RESULT_OK){
                c_iterInsert(sub->readers, reader);    
            }
        }
        else{
            r = JNI_RESULT_PRECONDITION_NOT_MET;
        }
    }
    return r;
}

jni_result
jni_subscriberSubscribe(
    jni_subscriber sub,
    const c_char* partitionExpr)
{
    jni_result r;
    
    if((sub == NULL) || (sub->usubscriber == NULL) || (partitionExpr == NULL)){
        r = JNI_RESULT_BAD_PARAMETER;
    } 
    else {
        r = jni_convertResult(u_subscriberSubscribe(sub->usubscriber, partitionExpr));
    }
    return r;
}

jni_reader
jni_subscriberLookupReader(
    jni_subscriber sub,
    const c_char* topicName)
{
    jni_reader r, reader;
    c_iter readers;
    
    r = NULL;
    
    if((sub != NULL) && (sub->usubscriber != NULL) && (topicName != NULL)){
        readers = c_iterCopy(sub->readers);
        reader = jni_reader(c_iterTakeFirst(readers));
        
        
        while((reader != NULL) && (r == NULL)){
            if(strcmp(reader->description->name, topicName) == 0){
               r = reader;
            }
            reader = jni_reader(c_iterTakeFirst(readers));
        }
        c_iterFree(readers);
    }
    return r;
}
