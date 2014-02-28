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
#include <jni.h>
#include "jni_helper.h"
#include "jni_reflection.h"
#include "jni_participantFactory.h"
#include "jni_participant.h"
#include "jni_publisher.h"
#include "jni_publisherQos.h"
#include "jni_subscriber.h"
#include "jni_subscriberQos.h"
#include "jni_reader.h"
#include "jni_writer.h"
#include "jni_topic.h"
#include "jni_handler.h"
#include "jni__handler.h"
#include <assert.h>
#include "os.h"
#include "os_report.h"

#define DCPSPACKAGE "org/opensplice/dds/dcps/"
#define FUNCTION(name) Java_org_opensplice_dds_dcps_jni_DCPSJNIHandler_##name

c_char*
jni_getFullName(
    c_char* class)
{
    c_char* result;

    result = (c_char*)(os_malloc(strlen(class) + strlen(DCPSPACKAGE) + 1));
    os_sprintf(result, "%s%s", DCPSPACKAGE, class);
    return result;
}

c_char*
jni_getFullRepresentation(
    c_char* class)
{
    c_char* result;

    result = (c_char*)(os_malloc(strlen(class) + strlen(DCPSPACKAGE) + 3));
    os_sprintf(result, "L%s%s;", DCPSPACKAGE, class);
    return result;
}

JNIEXPORT jint JNICALL
FUNCTION(jniGetParticipantFactoryInstance)(
    JNIEnv *env,
    jobject this)
{
    jni_result r;
    jni_participantFactory pf;

    pf = jni_getParticipantFactoryInstance();

    if(pf == NULL){
        OS_REPORT(OS_ERROR, CONT_DCPSJNI, 0, "Could not create instance of ParticipantFactory.");
        r = JNI_RESULT_ERROR;
    }
    else{
        r = JNI_RESULT_OK;
    }
    return r;
}

JNIEXPORT jint JNICALL
FUNCTION(jniDeleteParticipantFactoryInstance)(
    JNIEnv *env,
    jobject this)
{
    jni_result r;
    jni_participantFactory pf;

    pf = jni_getParticipantFactoryInstance();

    r = jni_deleteParticipantFactory(pf);

    return r;
}

JNIEXPORT jobject JNICALL
FUNCTION(jniCreateParticipant)(
    JNIEnv *env,
    jobject this,
    jobject participantFactory,
    jlong domainId,
    jobject qos,
    jobject listener)
{
    jni_participant participant;
    jclass participant_class;
    jobject jparticipant;
    jfieldID participantDomainId;
    c_char* fullClassName;

    long cDomainId;
    jni_participantFactory pf;

    pf = jni_getParticipantFactoryInstance();
    assert(pf != NULL);

    cDomainId = (long)domainId;
    participant = jni_createParticipant(pf, cDomainId, NULL);

    jparticipant = NULL;

    if(participant != NULL){
        fullClassName = jni_getFullName("DomainParticipant");
        participant_class = (*env)->FindClass(env, fullClassName);
        os_free(fullClassName);
        assert(participant_class != NULL);
        jparticipant = (*env)->AllocObject(env, participant_class);

        participantDomainId = (*env)->GetFieldID(env, participant_class, "domainId", "J");
        (*env)->SetLongField(env, jparticipant, participantDomainId, domainId);

        jni_initHashSet(env, jparticipant, "publishers");
        jni_initHashSet(env, jparticipant, "subscribers");
        jni_initHashSet(env, jparticipant, "topics");

        jni_entity(participant)->javaObject = (*env)->NewGlobalRef(env, jparticipant);
    }
    else{
        OS_REPORT(OS_ERROR, CONT_DCPSJNI, 0, "DomainParticipant could not be created.");
    }
    return jparticipant;
}

JNIEXPORT jint JNICALL
FUNCTION(jniDeleteParticipant)(
    JNIEnv *env,
    jobject this,
    jobject participant)
{
    jni_participant p;
    jni_result r;
    jni_participantFactory pf;
    jobject javaObject;

    pf = jni_getParticipantFactoryInstance();
    assert(pf != NULL);

    r = JNI_RESULT_BAD_PARAMETER;
    p = jni_lookUpParticipant(env, pf, participant);

    if(p != NULL){
        javaObject = jni_entity(p)->javaObject;
        r = jni_deleteParticipant(p);

        if(r == JNI_RESULT_OK){
            (*env)->DeleteGlobalRef(env, javaObject);
        }
        else{
            OS_REPORT(OS_ERROR, CONT_DCPSJNI, 0, "Supplied DomainParticipant could not be deleted.");
        }
    }
    return r;
}

JNIEXPORT jobject JNICALL
FUNCTION(jniCreatePublisher)(
    JNIEnv *env,
    jobject this,
    jobject participant,
    jobject qos,
    jobject listener)
{
    jni_participant p;
    jni_publisher pub;
    jclass pub_class;
    jobject jpublisher;
    jfieldID participantId;
    jni_participantFactory pf;
    jni_result r;
    c_char* fullClassName;

    pf = jni_getParticipantFactoryInstance();
    assert(pf != NULL);

    p = jni_lookUpParticipant(env, pf, participant);
    jpublisher = NULL;

    if(p != NULL){
        pub = jni_createPublisher(p, NULL);

        if(pub != NULL){
           r = jni_publisherPublish(pub, jni_publisherQosGetPartition(env, qos));

            if(r == JNI_RESULT_OK){
                fullClassName = jni_getFullName("Publisher");
                pub_class = (*env)->FindClass(env, fullClassName);
                os_free(fullClassName);
                assert(pub_class != NULL);
                jpublisher = (*env)->AllocObject(env, pub_class);

                fullClassName = jni_getFullRepresentation("DomainParticipant");
                participantId = (*env)->GetFieldID(env, pub_class, "participant",  fullClassName);
                os_free(fullClassName);
                (*env)->SetObjectField(env, jpublisher, participantId, participant);

                jni_initHashSet(env, jpublisher, "writers");
                jni_entity(pub)->javaObject = (*env)->NewGlobalRef(env, jpublisher);
            } else{
                OS_REPORT(OS_ERROR, CONT_DCPSJNI, 0, "Publisher could not be created.");
            }
        }
    }
    return jpublisher;
}

JNIEXPORT jint JNICALL
FUNCTION(jniDeletePublisher)(
    JNIEnv *env,
    jobject this,
    jobject publisher)
{
    jni_participant par;
    jni_publisher pub;
    jni_participantFactory pf;
    jni_result r;
    jobject javaObject;

    pf = jni_getParticipantFactoryInstance();
    assert(pf != NULL);
    pub = jni_lookUpPublisher(env, pf, publisher);

    if(pub == NULL){
       r = JNI_RESULT_BAD_PARAMETER;
       OS_REPORT(OS_ERROR, CONT_DCPSJNI, 0, "Supplied publisher could not be found.");
    }
    else{
        par = pub->participant;
        javaObject = jni_entity(pub)->javaObject;
        r = jni_deletePublisher(par, pub);

        if(r == JNI_RESULT_OK){
            (*env)->DeleteGlobalRef(env, javaObject);
        }
    }
    return r;
}

JNIEXPORT jobject JNICALL
FUNCTION(jniCreateSubscriber)(
    JNIEnv *env,
    jobject this,
    jobject participant,
    jobject qos,
    jobject listener)
{
    jni_participant p;
    jni_subscriber sub;
    jclass sub_class;
    jobject jsubscriber;
    jfieldID participantId;
    jni_participantFactory pf;
    jni_result r;
    c_char* fullClassName;

    pf = jni_getParticipantFactoryInstance();
    assert(pf != NULL);
    p = jni_lookUpParticipant(env, pf, participant);
    jsubscriber = NULL;

    if(p != NULL){
        sub = jni_createSubscriber(p, NULL);

        if(sub != NULL){
            r = jni_subscriberSubscribe(sub, jni_subscriberQosGetPartition(env, qos));

            if(r == JNI_RESULT_OK){
                fullClassName = jni_getFullName("Subscriber");
                sub_class = (*env)->FindClass(env, fullClassName);
                os_free(fullClassName);
                assert(sub_class != NULL);
                jsubscriber = (*env)->AllocObject(env, sub_class);
                fullClassName = jni_getFullRepresentation("DomainParticipant");
                participantId = (*env)->GetFieldID(env, sub_class, "participant", fullClassName);
                os_free(fullClassName);
                (*env)->SetObjectField(env, jsubscriber, participantId, participant);

                jni_initHashSet(env, jsubscriber, "readers");

                jni_entity(sub)->javaObject = (*env)->NewGlobalRef(env, jsubscriber);
            } else{
                OS_REPORT(OS_ERROR, CONT_DCPSJNI, 0, "Could not subscribe Subscriber to supplied Partition.");
            }
        }
        else{
            OS_REPORT(OS_ERROR, CONT_DCPSJNI, 0, "Subscriber could not be created.");
        }
    }
    return jsubscriber;
}

JNIEXPORT jint JNICALL
FUNCTION(jniDeleteSubscriber)(
    JNIEnv *env,
    jobject this,
    jobject subscriber)
{
    jni_subscriber sub;
    jni_participant par;
    jni_participantFactory pf;
    jni_result r;
    jobject javaObject;

    pf = jni_getParticipantFactoryInstance();
    assert(pf != NULL);
    sub = jni_lookUpSubscriber(env, pf, subscriber);

    if(sub == NULL){
       r = JNI_RESULT_BAD_PARAMETER;
       OS_REPORT(OS_ERROR, CONT_DCPSJNI, 0, "Supplied Subscriber could not be deleted.");
    }
    else{
        par = sub->participant;
        javaObject = jni_entity(sub)->javaObject;
        r = jni_deleteSubscriber(par, sub);

        if(r == JNI_RESULT_OK){
            (*env)->DeleteGlobalRef(env, javaObject);
        }
    }
    return r;
}

JNIEXPORT jobject JNICALL
FUNCTION(jniCreateTopic)(
    JNIEnv *env,
    jobject this,
    jobject participant,
    jstring topic_name,
    jstring topic_type_name,
    jobject qos,
    jobject listener)
{
    jni_participant p;
    jni_participantFactory pf;
    jobject jtopic;
    jclass top_class;
    jni_topic top;
    jfieldID participantId, topicNameId, topicTypeId;
    const char* topicName;
    const char* topicTypeName;
    c_char* fullClassName;

    pf = jni_getParticipantFactoryInstance();
    assert(pf != NULL);
    p = jni_lookUpParticipant(env, pf, participant);
    topicName = (*env)->GetStringUTFChars(env, topic_name, 0);
    topicTypeName = (*env)->GetStringUTFChars(env, topic_type_name, 0);
    top = jni_createTopic(p, topicName, topicTypeName, NULL);
    jtopic = NULL;

    if(top != NULL){
        (*env)->ReleaseStringUTFChars(env, topic_name, topicName);
        (*env)->ReleaseStringUTFChars(env, topic_type_name, topicTypeName);
        fullClassName = jni_getFullName("Topic");
        top_class = (*env)->FindClass(env, fullClassName);
        os_free(fullClassName);
        assert(top_class != NULL);

        jtopic = (*env)->AllocObject(env, top_class);
        fullClassName = jni_getFullRepresentation("DomainParticipant");
        participantId = (*env)->GetFieldID(env, top_class, "participant", fullClassName);
        os_free(fullClassName);
        (*env)->SetObjectField(env, jtopic, participantId, participant);

        topicNameId = (*env)->GetFieldID(env, top_class, "name", "Ljava/lang/String;");
        (*env)->SetObjectField(env, jtopic, topicNameId, topic_name);

        topicTypeId = (*env)->GetFieldID(env, top_class, "typeName", "Ljava/lang/String;");
        (*env)->SetObjectField(env, jtopic, topicTypeId, topic_type_name);

        jni_entity(top)->javaObject = (*env)->NewGlobalRef(env, jtopic);
    }
    return jtopic;
}

JNIEXPORT jint JNICALL
FUNCTION(jniDeleteTopic)(
    JNIEnv *env,
    jobject this,
    jobject topic)
{
    jni_topic t;
    jni_participant par;
    jni_participantFactory pf;
    jni_result r;
    jobject javaObject;

    pf = jni_getParticipantFactoryInstance();
    t = jni_lookUpTopic(env, pf, topic);

    if(t == NULL){
       r = JNI_RESULT_BAD_PARAMETER;
       OS_REPORT(OS_ERROR, CONT_DCPSJNI, 0, "Supplied Topic could not be deleted.");
    }
    else{
        par = jni_topicDescription(t)->participant;
        javaObject = jni_entity(t)->javaObject;
        r = jni_deleteTopic(par, t);

        if(r == JNI_RESULT_OK){
            (*env)->DeleteGlobalRef(env, javaObject);
        }
    }
    return r;
}

JNIEXPORT jobject JNICALL
FUNCTION(jniCreateDataWriter)(
    JNIEnv *env,
    jobject this,
    jobject publisher,
    jobject topic,
    jobject qos,
    jobject listener)
{
    jni_publisher pub;
    jni_topic top;
    jni_writer wri;
    jclass wri_class;
    jobject jwriter;
    jfieldID publisherId, topicId;
    jni_participantFactory pf;
    c_char* fullClassName;

    pf = jni_getParticipantFactoryInstance();

    pub = jni_lookUpPublisher(env, pf, publisher);
    top = jni_lookUpTopic(env, pf, topic);
    jwriter = NULL;

    if((pub != NULL) && (top != NULL)){
        wri = jni_createWriter(pub, top, NULL);

        if(wri != NULL){
            fullClassName = jni_getFullName("DataWriter");
            wri_class = (*env)->FindClass(env, fullClassName);
            os_free(fullClassName);
            assert(wri_class != NULL);

            jwriter = (*env)->AllocObject(env, wri_class);

            fullClassName = jni_getFullRepresentation("Publisher");
            publisherId = (*env)->GetFieldID(env, wri_class, "publisher", fullClassName);
            os_free(fullClassName);
            (*env)->SetObjectField(env, jwriter, publisherId, publisher);

            fullClassName = jni_getFullRepresentation("Topic");
            topicId = (*env)->GetFieldID(env, wri_class, "topic", fullClassName);
            os_free(fullClassName);
            (*env)->SetObjectField(env, jwriter, topicId, topic);

            jni_entity(wri)->javaObject = (*env)->NewGlobalRef(env, jwriter);
        }
    }
    return jwriter;
}

JNIEXPORT jint JNICALL
FUNCTION(jniDeleteDataWriter)(
    JNIEnv *env,
    jobject this,
    jobject writer)
{
    jni_writer w;
    jni_result r;
    jobject javaObject;
    jni_participantFactory pf;

    pf = jni_getParticipantFactoryInstance();

    w = jni_lookUpWriter(env, pf, writer);

    if(w == NULL){
        r = JNI_RESULT_BAD_PARAMETER;
        OS_REPORT(OS_ERROR, CONT_DCPSJNI, 0, "Supplied DataWriter could not be deleted.");
    }
    else{
        javaObject = jni_entity(w)->javaObject;
        r = jni_deleteWriter(w->publisher, w);

        if(r == JNI_RESULT_OK){
            (*env)->DeleteGlobalRef(env, javaObject);
        }
    }
    return r;
}

JNIEXPORT jobject JNICALL
FUNCTION(jniCreateDataReader)(
    JNIEnv *env,
    jobject this,
    jobject subscriber,
    jobject topic_description,
    jobject qos,
    jobject listener)
{
    jni_participantFactory pf;
    jni_subscriber sub;
    jni_topicDescription td;
    jni_reader reader;
    jclass rea_class;
    jobject jreader;
    jfieldID subscriberId, topicId;
    c_char* fullClassName;

    pf = jni_getParticipantFactoryInstance();

    sub = jni_lookUpSubscriber(env, pf, subscriber);
    td = jni_topicDescription(jni_lookUpTopic(env, pf, topic_description));

    jreader = NULL;

    if((sub != NULL) && (td != NULL)){
        reader = jni_createReader(sub, td, NULL);

        if(reader != NULL){
            fullClassName = jni_getFullName("DataReader");
            rea_class = (*env)->FindClass(env, fullClassName);
            os_free(fullClassName);
            assert(rea_class != NULL);

            jreader = (*env)->AllocObject(env, rea_class);

            fullClassName = jni_getFullRepresentation("Subscriber");
            subscriberId = (*env)->GetFieldID(env, rea_class, "subscriber", fullClassName);
            os_free(fullClassName);
            (*env)->SetObjectField(env, jreader, subscriberId, subscriber);

            fullClassName = jni_getFullRepresentation("TopicDescription");
            topicId = (*env)->GetFieldID(env, rea_class, "topicDescription", fullClassName);
            os_free(fullClassName);
            (*env)->SetObjectField(env, jreader, topicId, topic_description);

            jni_entity(reader)->javaObject = (*env)->NewGlobalRef(env, jreader);
        }
    }
    return jreader;
}

JNIEXPORT jint JNICALL
FUNCTION(jniDeleteDataReader)(
    JNIEnv *env,
    jobject this,
    jobject reader)
{
    jni_participantFactory pf;
    jni_reader rea;
    jni_result r;
    jobject javaObject;

    pf = jni_getParticipantFactoryInstance();
    rea = jni_lookUpReader(env, pf, reader);

    if(rea == NULL){
        r = JNI_RESULT_BAD_PARAMETER;
        OS_REPORT(OS_ERROR, CONT_DCPSJNI, 0, "Supplied DataReader could not be deleted.");
    }
    else{
        javaObject = jni_entity(rea)->javaObject;
        r = jni_deleteReader(rea->subscriber, rea);
        if(r == JNI_RESULT_OK){
            (*env)->DeleteGlobalRef(env, javaObject);
        }
    }
    return r;
}

JNIEXPORT jint JNICALL
FUNCTION(jniDeleteParticipantEntities)(
    JNIEnv *env,
    jobject this,
    jobject participant)
{
    jni_participantFactory pf;
    jni_participant p;
    jni_result r;

    pf = jni_getParticipantFactoryInstance();
    p = jni_lookUpParticipant(env, pf, participant);

    r = jni_deleteParticipantEntities(p);
    return r;
}

JNIEXPORT jstring JNICALL
FUNCTION(jniReaderRead)(
    JNIEnv *env,
    jobject this,
    jobject reader)
{
    jni_participantFactory pf;
    jni_reader r;
    c_char *result;
    jstring str;

    str = NULL;

    pf = jni_getParticipantFactoryInstance();
    r = jni_lookUpReader(env, pf, reader);

    result = jni_readerRead(r);

    if(result != NULL){
        str = (*env)->NewStringUTF(env, result);
        os_free(result);
    }
    return str;
}

JNIEXPORT jstring JNICALL
FUNCTION(jniReaderTake)(
    JNIEnv *env,
    jobject this,
    jobject reader)
{
    jni_participantFactory pf;
    jni_reader r;
    c_char* result;
    jstring str;

    str = NULL;

    pf = jni_getParticipantFactoryInstance();
    r = jni_lookUpReader(env, pf, reader);

    result = jni_readerTake(r);

    if(result != NULL){
        str = (*env)->NewStringUTF(env, result);
        os_free(result);
    }
    return str;
}

JNIEXPORT jint JNICALL
FUNCTION(jniWriterWrite)(
    JNIEnv *env,
    jobject this,
    jobject writer,
    jstring jsample)
{
    jni_participantFactory pf;
    jni_writer w;
    jni_result r;
    const c_char* sample;

    if(jsample != NULL){
        pf = jni_getParticipantFactoryInstance();
        w = jni_lookUpWriter(env, pf, writer);

        sample = (*env)->GetStringUTFChars(env, jsample, 0);
        r = jni_writerWrite(w, sample);
        (*env)->ReleaseStringUTFChars(env, jsample, sample);
    }
    else{
        r = JNI_RESULT_BAD_PARAMETER;
    }
    return r;
}


JNIEXPORT jint JNICALL
FUNCTION(jniCreateQueryCondition)(
    JNIEnv *env,
    jobject this,
    jobject reader,
    jstring query_expression,
    jobjectArray query_arguments)
{
    jni_participantFactory pf;
    jni_reader r;
    jni_result result;
    const c_char* query;

    pf = jni_getParticipantFactoryInstance();
    r = jni_lookUpReader(env, pf, reader);

    query = (*env)->GetStringUTFChars(env, query_expression, 0);
    result = jni_readerSetQuery(r, query, NULL);
    (*env)->ReleaseStringUTFChars(env, query_expression, query);

    return result;
}

void
jni_entityKernelAction(
    v_entity entity,
    c_voidp args)
{
    jni_entityKernelArg arg;

    arg = jni_entityKernelArg(args);

    if(entity != NULL){
        arg->kernel = v_objectKernel(entity);
    }
}
