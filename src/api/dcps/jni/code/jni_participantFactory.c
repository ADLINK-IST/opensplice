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

#include "jni_participantFactory.h"
#include "jni_participant.h"
#include "jni_nameService.h"
#include "jni_misc.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include "os_signalHandler.h"

static jni_participantFactory participantFactory = NULL;

jni_participantFactory
jni_getParticipantFactoryInstance()
{
    jni_nameService ns;
    char* ldPreload;
    
    if(participantFactory == NULL){
        ldPreload = os_getenv("LD_PRELOAD");

        if(ldPreload){
            if(strstr(ldPreload, "jsig") == NULL){
                os_signalHandlerSetEnabled(0);
            }
        } else {
            os_signalHandlerSetEnabled(0);
        }
        ns = jni_nameServiceNew();
        
        if(ns != NULL){
            participantFactory = jni_participantFactory(os_malloc((size_t)(C_SIZEOF(jni_participantFactory))));
            participantFactory->domains = c_iterNew(NULL);
            participantFactory->participants = c_iterNew(NULL);
        }
    }
    return participantFactory;
}

jni_result
jni_deleteParticipantFactory()
{
    jni_result r;
    
    if(participantFactory == NULL){
       r = JNI_RESULT_ALREADY_DELETED;
    }
    else{
        if(participantFactory->participants != NULL){
            if((participantFactory->participants != NULL) &&
                (c_iterLength(participantFactory->participants) != 0)){
                r = JNI_RESULT_PRECONDITION_NOT_MET;
            }
            else{
                r = jni_nameServiceFree();
                
                if(r == JNI_RESULT_OK){
                    c_iterFree(participantFactory->participants);
                    c_iterFree(participantFactory->domains);
                    os_free(participantFactory);
                    participantFactory = NULL;
                }
            }
        }
        else{
            r = jni_nameServiceFree();
            
            if(r == JNI_RESULT_OK){
                c_iterFree(participantFactory->domains);
                os_free(participantFactory);
                participantFactory = NULL;
            }
        }
    }
    return r;
}

jni_participant
jni_createParticipant(
    jni_participantFactory pf,
    long domainId, 
    v_qos qos)
{
    jni_participant p;
    const c_char* uri;

    p = NULL;
    
    if((pf != NULL) && (pf == participantFactory)){    
        uri = jni_nameServiceResolveURI(domainId);
                
        p = jni_participantNew(uri, "Java DCPS participant", domainId, qos);

        if(p != NULL){
            participantFactory->participants = c_iterInsert(participantFactory->participants, p);
        }
    }
    return p;
}    

jni_result
jni_deleteParticipant(
    jni_participant p)
{
    jni_result r;
    c_bool found;
       
    if((p == NULL) || (participantFactory == NULL)){
       r = JNI_RESULT_BAD_PARAMETER;
    }
    else{
        found = c_iterContains(participantFactory->participants, p);
        
        if(found){
            c_iterTake(participantFactory->participants, p);
            r = jni_participantFree(p);
        
            if(r != JNI_RESULT_OK){
                c_iterInsert(participantFactory->participants, p);
            }
        }
        else{
            r = JNI_RESULT_PRECONDITION_NOT_MET;
        }
    }   
    return r;
}
