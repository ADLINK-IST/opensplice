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

#include "jni_publisher.h"
#include "jni_participant.h"
#include "jni_writer.h"
#include "jni_misc.h"
#include "jni_topic.h"
#include "os_heap.h"

jni_publisher
jni_publisherNew(
    jni_participant p,
    v_publisherQos qos)
{
    jni_publisher pub;
    u_publisher up;
    
    pub = NULL;
    
    if((p != NULL) && (p->uparticipant != NULL)){
        up = u_publisherNew(p->uparticipant, NULL, qos, TRUE);
    
        if(up != NULL){
            pub                 = jni_publisher(os_malloc((size_t)(C_SIZEOF(jni_publisher))));
            pub->upublisher     = up;
            pub->participant    = p;
            pub->writers        = c_iterNew(NULL);
        }
        else{
            printf("Publisher == NULL\n");
        }
    }
    return pub;
}    

jni_result
jni_publisherFree(
    jni_publisher p)
{
    jni_result r;
    
    if(p != NULL){
        if((p->writers != NULL) && (c_iterLength(p->writers) != 0)){
            r = JNI_RESULT_PRECONDITION_NOT_MET;
        }
        else{
            r = JNI_RESULT_OK;
            
            if(p->writers != NULL){
                c_iterFree(p->writers);
            }
            if(p->upublisher != NULL){
                r = jni_convertResult(u_publisherFree(p->upublisher));
            }
            os_free(p);
        }
    }
    else{
        r = JNI_RESULT_BAD_PARAMETER;
    }
    
    return r;
}

jni_result
jni_deletePublisherEntities(
    jni_publisher pub)
{
    jni_writer wri;
    c_iter copy;
    jni_result r;
    int ok = 1;
    
    if(pub != NULL){
        copy = c_iterCopy(pub->writers);
        wri = jni_writer(c_iterTakeFirst(copy));
        
        while(wri != NULL){
            r = jni_writerFree(wri);
            
            if(r != JNI_RESULT_OK){
                ok = 0;  
            }
            wri = jni_writer(c_iterTakeFirst(copy));
        }
        c_iterFree(copy);
        
        if(ok){
           c_iterFree(pub->writers);
           pub->writers = c_iterNew(NULL); 
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

jni_writer
jni_createWriter(
    jni_publisher pub,
    jni_topic top,
    v_writerQos qos)
{
    jni_writer wri;
    
    if(pub != NULL){
        wri = jni_writerNew(pub, top, qos);
        
        if(wri != NULL){
            pub->writers = c_iterInsert(pub->writers, wri);
        }
    }
    else{
        wri = NULL;
    }
    return wri;
}

jni_result
jni_deleteWriter(
    jni_publisher pub,
    jni_writer wri)
{
    jni_result r;
    c_bool found;
    
    if((pub == NULL) || (wri == NULL)){
        r = JNI_RESULT_BAD_PARAMETER;
    }   
    else if(wri->publisher != pub){
        r = JNI_RESULT_PRECONDITION_NOT_MET;
    }
    else{
        found = c_iterContains(pub->writers, wri);
        
        if(found){
            c_iterTake(pub->writers, wri);
            r = jni_convertResult(u_writerFree(wri->uwriter));
            sd_serializerFree(wri->deserializer);
            os_free(wri);
        
            if(r != JNI_RESULT_OK) {
                c_iterInsert(pub->writers, wri);
            }
        }
        else {
            r = JNI_RESULT_PRECONDITION_NOT_MET;
        }
    }
    return r;
}

jni_result
jni_publisherPublish(
    jni_publisher pub,
    const c_char* partitionExpr)
{
    jni_result r;
    
    if((pub == NULL) || (pub->upublisher == NULL)){
        r = JNI_RESULT_BAD_PARAMETER;
    } 
    else {
        r = jni_participantAddPartition(pub->participant, partitionExpr);
        
        if(r == JNI_RESULT_OK){
            r = jni_convertResult(u_publisherPublish(pub->upublisher, partitionExpr));
        }
    }
    return r;
}

jni_writer
jni_publisherLookupWriter(
    jni_publisher pub,
    const c_char* topicName)
{
    jni_writer w, writer;
    c_iter writers;
    
    w = NULL;
    
    if((pub != NULL) && (pub->upublisher != NULL) && (topicName != NULL)){
        writers = c_iterCopy(pub->writers);
        writer = jni_writer(c_iterTakeFirst(writers));
        
        
        while((writer != NULL) && (w == NULL)){            
            if(strcmp(jni_topicDescription(writer->topic)->name, topicName) == 0){
               w = writer;
            }
            writer = jni_writer(c_iterTakeFirst(writers));
        }
        c_iterFree(writers);
    }
       
    return w;
}
