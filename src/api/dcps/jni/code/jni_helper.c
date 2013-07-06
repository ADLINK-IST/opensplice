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

#include "jni_helper.h"
#include "jni_participantFactory.h"
#include "jni_participant.h"
#include "jni_publisher.h"
#include "jni_subscriber.h"
#include "jni_writer.h"
#include "jni_reader.h"
#include "jni_topic.h"

jni_participant
jni_lookUpParticipant(
    JNIEnv *env,
    jni_participantFactory pf,
    jobject jentity)
{
    jni_participant p, found;
    c_iter pcopy;
    jobject e;
    jboolean test;
    
    found = NULL;
    
    if((pf != NULL) && (pf->participants != NULL)){    
        pcopy = c_iterCopy(pf->participants);
        p = jni_participant(c_iterTakeFirst(pcopy));
        
        while( (p != NULL) && (found == NULL) ){
            e = jni_entity(p)->javaObject;
            
            if(e != NULL){
                test = (*env)->IsSameObject(env, jentity, e);
                if(test){
                  found = p;
                }
            }
            p = jni_participant(c_iterTakeFirst(pcopy));
        }
        c_iterFree(pcopy);
    }
    return found;
}

jni_publisher
jni_lookUpPublisher(
    JNIEnv *env, 
    jni_participantFactory pf, 
    jobject jentity)
{
    c_iter pcopy, parcopy;
    jni_participant par;
    jni_publisher p, found;
    jobject e;
    jboolean test;
    found = NULL;
        
    if((pf != NULL) && (pf->participants != NULL)){
        parcopy = c_iterCopy(pf->participants);
        par = jni_participant(c_iterTakeFirst(parcopy));
        
        while( (par != NULL) && (found == NULL)){
          
            if(par->publishers != NULL){
                pcopy = c_iterCopy(par->publishers);
                p = jni_publisher(c_iterTakeFirst(pcopy));
                
                while( (p != NULL) && (found == NULL)){
                    e = jni_entity(p)->javaObject;
                    
                    if(e != NULL){
                        test = (*env)->IsSameObject(env, jentity, e);
                        if(test){
                            found = p;
                        }
                    }
                    p = jni_publisher(c_iterTakeFirst(pcopy));
                }
                c_iterFree(pcopy);
            }
            par = jni_participant(c_iterTakeFirst(parcopy));
        }
        c_iterFree(parcopy);
    }
    return found;
}    

jni_subscriber
jni_lookUpSubscriber(
    JNIEnv *env, 
    jni_participantFactory pf, 
    jobject jentity)
{
    c_iter scopy, parcopy;
    jni_participant par;
    jni_subscriber s, found;
    jobject e;
    jboolean test;
    
    found = NULL;
    
    if((pf != NULL) || (pf->participants != NULL)){
        parcopy = c_iterCopy(pf->participants);
        par = jni_participant(c_iterTakeFirst(parcopy));
        
        while( (par != NULL) && (found == NULL)){
          
            if(par->subscribers != NULL){
                scopy = c_iterCopy(par->subscribers);
                s = jni_subscriber(c_iterTakeFirst(scopy));
                
                while( (s != NULL) && (found == NULL)){
                    e = jni_entity(s)->javaObject;
                    
                    if(e != NULL){
                        test = (*env)->IsSameObject(env, jentity, e);
                        if(test){
                            found = s;
                        }
                    }
                    s = jni_subscriber(c_iterTakeFirst(scopy));
                }
                c_iterFree(scopy);
            }
            par = jni_participant(c_iterTakeFirst(parcopy));
        }
        c_iterFree(parcopy);
    }
    return found;   
}

jni_topic
jni_lookUpTopic(
    JNIEnv *env,
    jni_participantFactory pf,
    jobject jentity)
{
    c_iter tcopy, parcopy;
    jni_participant par;
    jni_topic t, found;
    jobject e;
    jboolean test;
    
    found = NULL;   
    
    if((pf != NULL) && (pf->participants != NULL)){
        parcopy = c_iterCopy(pf->participants);
        par = jni_participant(c_iterTakeFirst(parcopy));
        
        while( (par != NULL) && (found == NULL)){
          
            if(par->topics != NULL){
                tcopy = c_iterCopy(par->topics);
                t = jni_topic(c_iterTakeFirst(tcopy));
                
                while( (t != NULL) && (found == NULL)){
                    e = jni_entity(t)->javaObject;
                    
                    if(e != NULL){
                        test = (*env)->IsSameObject(env, jentity, e);
                        if(test){
                            found = t;
                        }
                    }
                    t = jni_topic(c_iterTakeFirst(tcopy));
                }
                c_iterFree(tcopy);
            }
            par = jni_participant(c_iterTakeFirst(parcopy));
        }
        c_iterFree(parcopy);
    }

    return found;
}

jni_writer
jni_lookUpWriter(
    JNIEnv *env, 
    jni_participantFactory pf,
    jobject jentity)
{
    c_iter pcopy, parcopy, wricopy;
    jni_participant par;
    jni_publisher p;
    jni_writer w, found;
    jobject e;
    jboolean test;
    
    found = NULL;
    
    if((pf != NULL) || (pf->participants != NULL)){
        parcopy = c_iterCopy(pf->participants);
        par = jni_participant(c_iterTakeFirst(parcopy));
        
        while( (par != NULL) && (found == NULL)){
          
            if(par->publishers != NULL){
                pcopy = c_iterCopy(par->publishers);
                p = jni_publisher(c_iterTakeFirst(pcopy));
                
                while( (p != NULL) && (found == NULL)){
                    
                    if(p->writers != NULL){
                        wricopy = c_iterCopy(p->writers);
                        w = jni_writer(c_iterTakeFirst(wricopy));
                        
                        while( (w != NULL) && (found == NULL) ){
                            e = jni_entity(w)->javaObject;
                            test = (*env)->IsSameObject(env, jentity, e);
                            
                            if((e != NULL) && (test)){
                                found = w;    
                            }
                            w = jni_writer(c_iterTakeFirst(wricopy));
                        }
                        c_iterFree(wricopy);
                    }
                    p = jni_publisher(c_iterTakeFirst(pcopy));
                }
                c_iterFree(pcopy);
            }
            par = jni_participant(c_iterTakeFirst(parcopy));
        }
        c_iterFree(parcopy);
    }
    return found;
}

jni_reader
jni_lookUpReader(
    JNIEnv *env, 
    jni_participantFactory pf,
    jobject jentity)
{
    c_iter scopy, parcopy, readcopy;
    jni_participant par;
    jni_subscriber s;
    jni_reader r, found;
    jobject e;
    jboolean test;
    
    found = NULL;
    
    if((pf != NULL) || (pf->participants != NULL)){
        parcopy = c_iterCopy(pf->participants);
        par = jni_participant(c_iterTakeFirst(parcopy));
        
        while( (par != NULL) && (found == NULL)){
          
            if(par->subscribers != NULL){
                scopy = c_iterCopy(par->subscribers);
                s = jni_subscriber(c_iterTakeFirst(scopy));
                
                while( (s != NULL) && (found == NULL)){
                    
                    if(s->readers != NULL){
                        readcopy = c_iterCopy(s->readers);
                        r = jni_reader(c_iterTakeFirst(readcopy));
                        
                        while( (r != NULL) && (found == NULL) ){
                            e = jni_entity(r)->javaObject;
                            test = (*env)->IsSameObject(env, jentity, e);
                            
                            if((e != NULL) && (test)){
                                found = r;    
                            }
                            r = jni_reader(c_iterTakeFirst(readcopy));
                        }
                        c_iterFree(readcopy);
                    }
                    s = jni_subscriber(c_iterTakeFirst(scopy));
                }
                c_iterFree(scopy);
            }
            par = jni_participant(c_iterTakeFirst(parcopy));
        }
        c_iterFree(parcopy);
    }
    return found;
}
