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
/**
 * @file api/cm/jni/code/cmj_factory.c
 * @brief Interface from Java C&M API to C&M XML API.
 *
 * This interface provides a communication mechanism between the Java C&M API
 * and the XML C&M API using the Java Native Interface.
 *
 * The interface is built upon the XML C&M API. The layering looks like:
@verbatim
    -------------------
    |  Java C&M API   |
    -------------------------------------
    |      Java       :     Java        |
    |  Communication  :      XML        |
    |      layer      :(de)serialization|
    -------------------------------------
    |   JNI layer     |
    -------------------
    |   XML C&M API   |
    -------------------
    |   User layer    |
    -------------------
@endverbatim
 * Documentation is located in the C-file because the header file is
 * generated.
 */
#include "cmj_factory.h"
#include <jni.h>
#include "cmx_factory.h"
#include "cmx_entity.h"
#include "cmx_participant.h"
#include "cmx_topic.h"
#include "cmx_reader.h"
#include "cmx_writer.h"
#include "cmx_readerSnapshot.h"
#include "cmx_writerSnapshot.h"
#include "cmx_snapshot.h"
#include "cmx_service.h"
#include "cmx_subscriber.h"
#include "cmx_publisher.h"
#include "cmx_domain.h"
#include "cmx_dataReader.h"
#include "cmx_query.h"
#include "cmx_reader.h"
#include "cmx_storage.h"
#include "cmx_waitset.h"
#include "c_typebase.h"
#include "v_kernel.h"
#include "os_heap.h"
#include "os_signalHandler.h"
#include "os_stdlib.h"

#define FUNCTION(name) Java_org_opensplice_cm_com_JniCommunicator_##name

static c_bool   cmj_isInitialized   = FALSE;
static void     cmj_checkConnection (JNIEnv* env);


/**
 * @brief Initializes the Control & Monitoring API.
 *
 * - Class:     JniCommunicator
 * - Method:    jniInitialise
 * - Signature: ()Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @result Whether or not the initialisation succeeded.When succeeded:
 *         @verbatim<result>OK</result>@endverbatim is returned,
 *         @verbatim<result>FAILED</result>@endverbatim otherwise.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniInitialise)(
    JNIEnv *env,
    jobject this)
{
    const c_char* result = "<result>OK</result>";
    char* ldPreload;
    jstring jresult;
    jobject thisCopy;

    if (strcmp(result, "<result>OK</result>") == 0){
        thisCopy = this;

        ldPreload = os_getenv("LD_PRELOAD");
        if(ldPreload){
            if(strstr(ldPreload, "jsig") == NULL){
                os_signalHandlerSetEnabled(0);
            }
        } else {
            os_signalHandlerSetEnabled(0);
        }
        result = cmx_initialise();

        if(strcmp(result, "<result>OK</result>") == 0){
            cmj_isInitialized = TRUE;
        }

        cmj_checkConnection(env);
    }

    jresult = (*env)->NewStringUTF(env, result);
    return jresult;
}

/**
 * @brief Detaches the Control & Monitoring API.
 *
 * - Class:     JniCommunicator
 * - Method:    jniDetach
 * - Signature: ()Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @result Whether or not the detach succeeded.When succeeded:
 *         @verbatim<result>OK</result>@endverbatim is returned,
 *         @verbatim<result>FAILED</result>@endverbatim otherwise.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniDetach)(
    JNIEnv *env,
    jobject this)
{
    const c_char* result;
    jstring jresult;
    jobject thisCopy;

    thisCopy = this;
    cmj_checkConnection(env);
    result = cmx_detach();

    if(strcmp(result, "<result>OK</result>") == 0){
        cmj_isInitialized = FALSE;
    }

    jresult = (*env)->NewStringUTF(env, result);

    return jresult;
}

/**
 * @brief Frees the supplied entity.
 *
 * - Class:     JniCommunicator
 * - Method:    jniEntityFree
 * - Signature: (Ljava/lang/String;)V
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jentity The entity that needs to be freed.
 */
JNIEXPORT void JNICALL
FUNCTION(jniEntityFree)(
    JNIEnv *env,
    jobject this,
    jstring jentity)
{
    const c_char* xmlEntity;
    c_char* entity;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    xmlEntity = (*env)->GetStringUTFChars(env, jentity, 0);
    entity = (c_char*)(os_malloc(strlen(xmlEntity) + 1));
    os_strcpy(entity, xmlEntity);
    cmx_entityFree(entity);
    (*env)->ReleaseStringUTFChars(env, jentity, xmlEntity);

}

/**
 * @brief Resolves the current status of the supplied entity.
 *
 * - Class:     JniCommunicator
 * - Method:    jniEntityStatus
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jentity The entity, which status must be resolved.
 * @return The status of the supplied entity, or null when the entity could
 *         not be resolved.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniEntityStatus)(
    JNIEnv *env,
    jobject this,
    jstring jentity)
{
    const c_char* xmlEntity;
    c_char* xmlStatus;
    jstring jstatus;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jstatus = NULL;
    xmlEntity = (*env)->GetStringUTFChars(env, jentity, 0);
    xmlStatus = cmx_entityStatus(xmlEntity);
    (*env)->ReleaseStringUTFChars(env, jentity, xmlEntity);

    if(xmlStatus != NULL){
        jstatus = (*env)->NewStringUTF(env, xmlStatus);
        os_free(xmlStatus);
    }

    return jstatus;
}

/**
 * @brief Resolves the current qos of the supplied entity.
 *
 * - Class:     JniCommunicator
 * - Method:    jniEntityQoS
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jentity The entity, which qos must be resolved.
 * @return The qos of the supplied entity, or null when the entity could
 *         not be resolved.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniEntityQoS)(
    JNIEnv *env,
    jobject this,
    jstring jentity)
{
    const c_char* xmlEntity;
    c_char* xmlQoS;
    jstring jqos;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jqos = NULL;
    xmlEntity = (*env)->GetStringUTFChars(env, jentity, 0);
    xmlQoS = cmx_entityQoS(xmlEntity);
    (*env)->ReleaseStringUTFChars(env, jentity, xmlEntity);

    if(xmlQoS != NULL){
        jqos = (*env)->NewStringUTF(env, xmlQoS);
        os_free(xmlQoS);
    }

    return jqos;
}

/**
 * @brief Resets (a part of) the statistics of the supplied entity.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniEntityResetStatistics
 * - Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jentity The entity, which statistics must be resetted.
 * @return The result of the reset action.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniEntityResetStatistics)(
    JNIEnv *env,
    jobject this,
    jstring jentity,
    jstring jfieldName)
{
    const c_char* xmlEntity;
    const c_char* fieldName;
    const c_char* result;
    jobject thisCopy;
    jstring jresult;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;
    fieldName = NULL;
    xmlEntity = (*env)->GetStringUTFChars(env, jentity, 0);

    if(jfieldName != NULL){
        fieldName = (*env)->GetStringUTFChars(env, jfieldName, 0);
    }
    result = cmx_entityResetStatistics(xmlEntity, fieldName);
    (*env)->ReleaseStringUTFChars(env, jentity, xmlEntity);

    if(jfieldName != NULL){
        (*env)->ReleaseStringUTFChars(env, jfieldName, fieldName);
    }
    jresult = (*env)->NewStringUTF(env, result);

    return jresult;
}

/**
 * @brief Resolves the statistics of the supplied entity.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniEntityGetStatistics
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jentity The entity, which statistics must be resolved.
 * @return The statistics of the entity.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniEntityGetStatistics)(
    JNIEnv *env,
    jobject this,
    jstring jentity)
{
    const c_char* xmlEntity;
    c_char* statistics;
    jobject thisCopy;
    jstring jresult;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;
    xmlEntity = (*env)->GetStringUTFChars(env, jentity, 0);

    statistics = cmx_entityStatistics(xmlEntity);
    (*env)->ReleaseStringUTFChars(env, jentity, xmlEntity);

    if(statistics != NULL){
        jresult = (*env)->NewStringUTF(env, statistics);
        os_free(statistics);
    }
    return jresult;
}

/**
 * @brief Resolves the statistics of the supplied entity.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniEntityGetStatistics
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jentity The entity, which statistics must be resolved.
 * @return The statistics of the entity.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniEntitiesGetStatistics)(
    JNIEnv *env,
    jobject this,
    jstring jentities)
{
    const c_char* xmlEntities;
    c_char* statistics;
    jobject thisCopy;
    jstring jresult;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;
    xmlEntities = (*env)->GetStringUTFChars(env, jentities, 0);
    statistics = cmx_entitiesStatistics(xmlEntities);
    (*env)->ReleaseStringUTFChars(env, jentities, xmlEntities);

    if(statistics != NULL){
        jresult = (*env)->NewStringUTF(env, statistics);
        os_free(statistics);
    }
    return jresult;
}

/**
 * @brief Applies the supplied QoS to the supplied entity.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniEntitySetQoS
 * - Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this THe Java object that called this function.
 * @param jentity The entity, where to apply the QoS to.
 * @param jqos The new QoS for the supplied entity.
 * @return The result of the apply action.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniEntitySetQoS)(
    JNIEnv *env,
    jobject this,
    jstring jentity,
    jstring jqos)
{
    const c_char* xmlEntity;
    const c_char* xmlQos;
    const c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    xmlQos = NULL;
    thisCopy = this;
    jresult = NULL;
    xmlEntity = (*env)->GetStringUTFChars(env, jentity, 0);

    if(jqos != NULL){
        xmlQos = (*env)->GetStringUTFChars(env, jqos, 0);
    }
    xmlResult = cmx_entitySetQoS(xmlEntity, xmlQos);

    if(jqos != NULL){
        (*env)->ReleaseStringUTFChars(env, jqos, xmlQos);
    }
    (*env)->ReleaseStringUTFChars(env, jentity, xmlEntity);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
    }
    return jresult;

}

/**
 * @brief Enables the supplied entity.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniEntityEnable
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jentity The entity that must be enabled.
 * @return The result of the enable action.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniEntityEnable)(
    JNIEnv *env,
    jobject this,
    jstring jentity)
{
    const c_char* xmlEntity;
    const c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;

    xmlEntity = (*env)->GetStringUTFChars(env, jentity, 0);
    xmlResult = cmx_entityEnable(xmlEntity);
    (*env)->ReleaseStringUTFChars(env, jentity, xmlEntity);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
    }
    return jresult;
}

/**
 * @brief Provides access to all entities that the supplied entity owns and
 *        that match the supplied filter.
 *
 * - Class:     JniCommunicator
 * - Method:    jniGetOwnedEntities
 * - Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jentity The entity, which owned entities must be resolved.
 * @param jfilter The filter that the owned entities must match.
 * @return The list of entities that are owned by the supplied entity and
 *         match the supplied filter.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniGetOwnedEntities)(
    JNIEnv *env,
    jobject this,
    jstring jentity,
    jstring jfilter)
{
    const c_char* xmlEntity;
    const c_char* xmlFilter;
    c_char* xmlEntities;
    jstring jentities;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jentities = NULL;
    xmlEntity = (*env)->GetStringUTFChars(env, jentity, 0);
    xmlFilter = (*env)->GetStringUTFChars(env, jfilter, 0);

    xmlEntities = cmx_entityOwnedEntities(xmlEntity, xmlFilter);

    (*env)->ReleaseStringUTFChars(env, jentity, xmlEntity);
    (*env)->ReleaseStringUTFChars(env, jfilter, xmlFilter);

    if(xmlEntities != NULL){
       jentities = (*env)->NewStringUTF(env, xmlEntities);
       os_free(xmlEntities);
    }

    return jentities;
}

/**
 * @brief Provides access to all entities that depend on the supplied entity and
 *        match the supplied filter.
 *
 * - Class:     JniCommunicator
 * - Method:    jniGetDependantEntities
 * - Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jentity The entity, which dependant entities must be resolved.
 * @param jfilter The filter that the dependant entities must match.
 * @return The list of entities that depend on the supplied entity and
 *         match the supplied filter.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniGetDependantEntities)(
    JNIEnv *env,
    jobject this,
    jstring jentity,
    jstring jfilter)
{
    const c_char* xmlEntity;
    const c_char* xmlFilter;
    c_char* xmlEntities;
    jstring jentities;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jentities = NULL;
    xmlEntity = (*env)->GetStringUTFChars(env, jentity, 0);
    xmlFilter = (*env)->GetStringUTFChars(env, jfilter, 0);

    xmlEntities = cmx_entityDependantEntities(xmlEntity, xmlFilter);

    (*env)->ReleaseStringUTFChars(env, jentity, xmlEntity);
    (*env)->ReleaseStringUTFChars(env, jfilter, xmlFilter);

    if(xmlEntities != NULL){
       jentities = (*env)->NewStringUTF(env, xmlEntities);
       os_free(xmlEntities);
    }

    return jentities;
}

/**
 * @brief Creates a participant.
 *
 * - Class:     JniCommunicator
 * - Method:    jniCreateParticipant
 * - Signature: (Ljava/lang/String;ILjava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param juri The URI of the kernel the participant must participate in.
 * @param jtimeout The maximum amount of time the function may keep attempting
 *                 to create a participant when creation fails(in milliseconds).
 * @param jname The name for the participant.
 * @param jqos The QoS for the participant.
 * @return The newly created participant, or null if it could not be created.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniCreateParticipant)(
    JNIEnv *env,
    jobject this,
    jstring juri,
    jint jtimeout,
    jstring jname,
    jstring jqos)
{
    const c_char* uri;
    const c_char* name;
    const c_char* qos;
    c_char* xmlParticipant;
    jstring jparticipant;
    jobject thisCopy;

    cmj_checkConnection(env);
    qos = NULL;
    thisCopy = this;
    jparticipant = NULL;
    uri = (*env)->GetStringUTFChars(env, juri, 0);
    name = (*env)->GetStringUTFChars(env, jname, 0);

    if(jqos != NULL){
        qos = (*env)->GetStringUTFChars(env, jqos, 0);
    }
    xmlParticipant = cmx_participantNew(uri, (c_long)jtimeout, name, qos);

    if(jqos != NULL){
        (*env)->ReleaseStringUTFChars(env, jqos, qos);
    }
    if(xmlParticipant != NULL){
        cmx_participantAutoDetach(xmlParticipant, TRUE);
        jparticipant = (*env)->NewStringUTF(env, xmlParticipant);
        os_free(xmlParticipant);
    }
    (*env)->ReleaseStringUTFChars(env, juri, uri);
    (*env)->ReleaseStringUTFChars(env, jname, name);

    return jparticipant;
}

/**
 * @brief Resolves all participants that participate in the same kernel as the
 *        supplied participant, including the supplied participant itself.
 *
 * - Class:     JniCommunicator
 * - Method:    jniParticipantAllParticipants
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jparticipant The participant, which kernel must be resolved.
 * @return A list of all participants that participate in the same kernel as
 *         the supplied participant.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniParticipantAllParticipants)(
    JNIEnv *env,
    jobject this,
    jstring jparticipant)
{
    const c_char* xmlParticipant;
    c_char* xmlEntities;
    jstring jentities;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jentities = NULL;
    xmlParticipant = (*env)->GetStringUTFChars(env, jparticipant, 0);
    xmlEntities = cmx_participantAllParticipants(xmlParticipant);
    (*env)->ReleaseStringUTFChars(env, jparticipant, xmlParticipant);

    if(xmlEntities != NULL){
       jentities = (*env)->NewStringUTF(env, xmlEntities);
       os_free(xmlEntities);
    }

    return jentities;
}

/**
 * @brief Resolves all topics that are located in the same kernel as the
 *        supplied participant.
 *
 * - Class:     JniCommunicator
 * - Method:    jniParticipantAllTopics
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jparticipant The participant, which kernel must be resolved.
 * @return A list of all topics that are located in the same kernel as
 *         the supplied participant.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniParticipantAllTopics)(
    JNIEnv *env,
    jobject this,
    jstring jparticipant)
{
    const c_char* xmlParticipant;
    c_char* xmlEntities;
    jstring jentities;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jentities = NULL;
    xmlParticipant = (*env)->GetStringUTFChars(env, jparticipant, 0);
    xmlEntities = cmx_participantAllTopics(xmlParticipant);
    (*env)->ReleaseStringUTFChars(env, jparticipant, xmlParticipant);

    if(xmlEntities != NULL){
       jentities = (*env)->NewStringUTF(env, xmlEntities);
       os_free(xmlEntities);
    }

    return jentities;
}

/**
 * @brief Resolves all domains that are located in the same kernel as the
 *        supplied participant.
 *
 * - Class:     JniCommunicator
 * - Method:    jniParticipantAllDomains
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jparticipant The participant, which kernel must be resolved.
 * @return A list of all domains that are located in the same kernel as
 *         the supplied participant.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniParticipantAllDomains)(
    JNIEnv *env,
    jobject this,
    jstring jparticipant)
{
    const c_char* xmlParticipant;
    c_char* xmlEntities;
    jstring jentities;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jentities = NULL;
    xmlParticipant = (*env)->GetStringUTFChars(env, jparticipant, 0);
    xmlEntities = cmx_participantAllDomains(xmlParticipant);
    (*env)->ReleaseStringUTFChars(env, jparticipant, xmlParticipant);

    if(xmlEntities != NULL){
       jentities = (*env)->NewStringUTF(env, xmlEntities);
       os_free(xmlEntities);
    }

    return jentities;
}

/**
 * @brief Finds the set of topics that match the supplied topic name expression.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniParticipantFindTopic
 * - Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jparticipant The participant that is used to determine the domain to
 *                     look in.
 * @return The set of resolved topics in XML format.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniParticipantFindTopic)(
    JNIEnv *env,
    jobject this,
    jstring jparticipant,
    jstring jname)
{
    const c_char* xmlParticipant;
    const c_char* xmlName;
    c_char* xmlEntities;
    jstring jentities;
    jobject thisCopy;

    thisCopy = this;
    jentities = NULL;
    xmlName = NULL;

    cmj_checkConnection(env);
    xmlParticipant = (*env)->GetStringUTFChars(env, jparticipant, 0);

    if(jname != NULL){
        xmlName = (*env)->GetStringUTFChars(env, jname, 0);
    }
    xmlEntities = cmx_participantFindTopic(xmlParticipant, xmlName);
    (*env)->ReleaseStringUTFChars(env, jparticipant, xmlParticipant);

    if(jname != NULL){
        (*env)->ReleaseStringUTFChars(env, jname, xmlName);
    }
    if(xmlEntities != NULL){
       jentities = (*env)->NewStringUTF(env, xmlEntities);
       os_free(xmlEntities);
    }
    return jentities;
}

/**
 * @brief Registers the supplied XML type in the domain the supplied
 *        participant participates in.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniRegisterType
 * - Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jparticipant The participant that is used to determine the domain to
 *                     look in.
 * @return The result of the registration.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniRegisterType)(
    JNIEnv *env,
    jobject this,
    jstring jparticipant,
    jstring jtype)
{
    jobject thisCopy;
    const c_char* xmlParticipant;
    const c_char* xmlType;
    const c_char* xmlResult;
    jstring jresult;

    cmj_checkConnection(env);
    thisCopy = this;
    xmlParticipant = (*env)->GetStringUTFChars(env, jparticipant, 0);
    xmlType = (*env)->GetStringUTFChars(env, jtype, 0);

    xmlResult = cmx_participantRegisterType(xmlParticipant, xmlType);

    (*env)->ReleaseStringUTFChars(env, jtype, xmlType);
    (*env)->ReleaseStringUTFChars(env, jparticipant, xmlParticipant);

    if(xmlResult != NULL){
       jresult = (*env)->NewStringUTF(env, xmlResult);
    } else {
        jresult = NULL;
    }
    return jresult;
}

/**
 * @brief Resolves the data type of the supplied topic.
 *
 * - Class:     JniCommunicator
 * - Method:    jniGetTopicDataType
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jtopic The topic, which data type must be resolved.
 * @return The data type of the supplied topic (in XML format), or null of the
 *         topic is not available (anymore).
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniGetTopicDataType)(
    JNIEnv *env,
    jobject this,
    jstring jtopic)
{
    const c_char* xmlTopic;
    c_char* xmlDatatype;
    jstring jdatatype;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jdatatype = NULL;
    xmlTopic = (*env)->GetStringUTFChars(env, jtopic, 0);
    xmlDatatype = cmx_topicDataType(xmlTopic);
    (*env)->ReleaseStringUTFChars(env, jtopic, xmlTopic);

    if(xmlDatatype != NULL){
        jdatatype = (*env)->NewStringUTF(env, xmlDatatype);
        os_free(xmlDatatype);
    }

    return jdatatype;
}

/**
 * Resolves the data type of a reader.
 *
 * -Class:     org_opensplice_api_cm_com_JniCommunicator
 * -Method:    jniReaderDataType
 * -Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jreader The reader to resolve the type of.
 * @return The XML representation of the type.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniReaderDataType)(
    JNIEnv *env,
    jobject this,
    jstring jreader)
{
    const c_char* xmlReader;
    c_char* xmlDatatype;
    jstring jdatatype;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jdatatype = NULL;
    xmlReader = (*env)->GetStringUTFChars(env, jreader, 0);
    xmlDatatype = cmx_readerDataType(xmlReader);
    (*env)->ReleaseStringUTFChars(env, jreader, xmlReader);

    if(xmlDatatype != NULL){
        jdatatype = (*env)->NewStringUTF(env, xmlDatatype);
        os_free(xmlDatatype);
    }

    return jdatatype;
}

/**
 * @brief Takes one sample from the supplied reader.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniReaderTake
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jreader The reader to take data from
 * @return The XML representation of the taken sample or NULL if none could be
 *         read.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniReaderTake)(
    JNIEnv *env,
    jobject this,
    jstring jreader)
{
    const c_char* xmlReader;
    c_char* xmlSample;
    jstring jsample;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jsample = NULL;
    xmlReader = (*env)->GetStringUTFChars(env, jreader, 0);
    xmlSample = cmx_readerTake(xmlReader);
    (*env)->ReleaseStringUTFChars(env, jreader, xmlReader);

    if(xmlSample != NULL){
        jsample = (*env)->NewStringUTF(env, xmlSample);
        os_free(xmlSample);
    }

    return jsample;
}

/**
 * @brief Reads the next instance of the supplied readers database.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniReaderReadNext
 * - Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jreader The reader to read data from
 * @return The XML representation of the read sample or NULL if none could be
 *         read.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniReaderReadNext)(
    JNIEnv *env,
    jobject this,
    jstring jreader,
    jstring jlocalId,
    jstring jextId)
{
    const c_char* xmlReader;
    const c_char* xmlLocalId;
    const c_char* xmlExtId;
    c_char* xmlSample;
    jstring jsample;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jsample = NULL;

    if(jreader != NULL){
        xmlReader = (*env)->GetStringUTFChars(env, jreader, 0);
        xmlLocalId = (*env)->GetStringUTFChars(env, jlocalId, 0);
        xmlExtId = (*env)->GetStringUTFChars(env, jextId, 0);

        xmlSample = cmx_readerReadNext(xmlReader, xmlLocalId, xmlExtId);

        (*env)->ReleaseStringUTFChars(env, jreader, xmlReader);
        (*env)->ReleaseStringUTFChars(env, jlocalId, xmlLocalId);
        (*env)->ReleaseStringUTFChars(env, jextId, xmlExtId);

        if(xmlSample != NULL){
            jsample = (*env)->NewStringUTF(env, xmlSample);
            os_free(xmlSample);
        }
    }
    return jsample;
}

/**
 * @brief Reads one sample from the supplied reader.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniReaderRead
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jreader The reader to read data from
 * @return The XML representation of the read sample or NULL if none could be
 *         read.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniReaderRead)(
    JNIEnv *env,
    jobject this,
    jstring jreader)
{
    const c_char* xmlReader;
    c_char* xmlSample;
    jstring jsample;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jsample = NULL;
    xmlReader = (*env)->GetStringUTFChars(env, jreader, 0);
    xmlSample = cmx_readerRead(xmlReader);
    (*env)->ReleaseStringUTFChars(env, jreader, xmlReader);

    if(xmlSample != NULL){
        jsample = (*env)->NewStringUTF(env, xmlSample);
        os_free(xmlSample);
    }

    return jsample;
}

/**
 * @brief Resolves the state of the supplied service.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniGetServiceState
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jservice The service, which state must be resolved.
 * @return The state of the service, or null if the service is not available
 *         (anymore).
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniGetServiceState)(
    JNIEnv *env,
    jobject this,
    jstring jservice)
{
    const c_char* xmlService;
    c_char* xmlState;
    jstring jstate;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jstate = NULL;
    xmlService = (*env)->GetStringUTFChars(env, jservice, 0);
    xmlState = cmx_serviceGetState(xmlService);
    (*env)->ReleaseStringUTFChars(env, jservice, xmlService);

    if(xmlState != NULL){
        jstate = (*env)->NewStringUTF(env, xmlState);
        os_free(xmlState);
    }

    return jstate;
}

/**
 * @brief Resolves the version of the CM API.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniGetServiceState
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @return The version of the service
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniGetVersion)(
    JNIEnv *env,
    jobject this)
{
    c_char* xmlVersion;
    jstring jversion;
    jobject thisCopy;

    cmj_checkConnection(env);
    jversion = NULL;

    xmlVersion = cmx_getVersion();
    if(xmlVersion != NULL){
        jversion = (*env)->NewStringUTF(env, xmlVersion);
        os_free(xmlVersion);
    }

    return jversion;
}

/**
 * @brief Makes a snapshot of the supplied reader.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniReaderSnapshotNew
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jreader The reader to make a snapshot of.
 * @return The created snapshot or NULL if it could not be created.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniReaderSnapshotNew)(
    JNIEnv *env,
    jobject this,
    jstring jreader)
{
    c_char* result;
    const c_char* reader;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;
    reader = (*env)->GetStringUTFChars(env, jreader, 0);
    result = cmx_readerSnapshotNew(reader);
    (*env)->ReleaseStringUTFChars(env, jreader, reader);

    if(result != NULL){
        jresult = (*env)->NewStringUTF(env, result);
        os_free(result);
    }

    return jresult;
}

/**
 * @brief Makes a snapshot of the supplied writer.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniWriterSnapshotNew
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jwriter The writer to make a snapshot of.
 * @return The created snapshot or NULL if it could not be created.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniWriterSnapshotNew)(
    JNIEnv *env,
    jobject this,
    jstring jwriter)
{
    c_char* result;
    const c_char* writer;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;
    writer = (*env)->GetStringUTFChars(env, jwriter, 0);
    result = cmx_writerSnapshotNew(writer);
    (*env)->ReleaseStringUTFChars(env, jwriter, writer);

    if(result != NULL){
        jresult = (*env)->NewStringUTF(env, result);
        os_free(result);
    }
    return jresult;
}

/**
 * @brief Frees the supplied snapshot.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniReaderSnapshotFree
 * - Signature: (Ljava/lang/String;)V
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jsnapshot The snapshot to free.
 */
JNIEXPORT void JNICALL
FUNCTION(jniSnapshotFree)(
    JNIEnv *env,
    jobject this,
    jstring jsnapshot)
{
    const c_char* snapshot;
    c_char* snapshotCopy;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    snapshot = (*env)->GetStringUTFChars(env, jsnapshot, 0);
    snapshotCopy = (c_char*)(os_malloc(strlen(snapshot) + 1));
    os_strcpy(snapshotCopy, snapshot);
    cmx_snapshotFree(snapshotCopy);
    (*env)->ReleaseStringUTFChars(env, jsnapshot, snapshot);

}

/**
 * @brief Reads a sample from the supplied snapshot.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniReaderSnapshotRead
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jsnapshot The snapshot where to read the sample from.
 * @return The read sample or NULL if no data was available.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniSnapshotRead)(
    JNIEnv *env,
    jobject this,
    jstring jsnapshot)
{
    const c_char* snapshot;
    c_char* result;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;

    snapshot = (*env)->GetStringUTFChars(env, jsnapshot, 0);
    result = cmx_snapshotRead(snapshot);
    (*env)->ReleaseStringUTFChars(env, jsnapshot, snapshot);

    if(result != NULL){
        jresult = (*env)->NewStringUTF(env, result);
        os_free(result);
    }

    return jresult;
}

/**
 * @brief Takes a sample from the supplied snapshot.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniReaderSnapshotTake
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jsnapshot The snapshot where to take the sample from.
 * @return The taken sample or NULL if no data was available.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniSnapshotTake)(
    JNIEnv *env,
    jobject this,
    jstring jsnapshot)
{
    const c_char* snapshot;
    c_char* result;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;

    snapshot = (*env)->GetStringUTFChars(env, jsnapshot, 0);
    result = cmx_snapshotTake(snapshot);
    (*env)->ReleaseStringUTFChars(env, jsnapshot, snapshot);

    if(result != NULL){
        jresult = (*env)->NewStringUTF(env, result);
        os_free(result);
    }

    return jresult;
}

/**
 * @brief Resolves the data type of the supplied writer.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniWriterDataType
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jwriter The writer where to resolve the data type of.
 * @return The XML representation of the data type.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniWriterDataType)(
    JNIEnv *env,
    jobject this,
    jstring jwriter)
{
    const c_char* xmlWriter;
    c_char* xmlDatatype;
    jstring jdatatype;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jdatatype = NULL;
    xmlWriter = (*env)->GetStringUTFChars(env, jwriter, 0);
    xmlDatatype = cmx_writerDataType(xmlWriter);
    (*env)->ReleaseStringUTFChars(env, jwriter, xmlWriter);

    if(xmlDatatype != NULL){
        jdatatype = (*env)->NewStringUTF(env, xmlDatatype);
        os_free(xmlDatatype);
    }

    return jdatatype;
}

/**
 * @brief Writes one instance of userData into the system using the supplied writer.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniWriterWrite
 * - Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jwriter The writer to write the data with.
 * @param jdata The data to write.
 * @return Whether or not the write succeeded.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniWriterWrite)(
    JNIEnv *env,
    jobject this,
    jstring jwriter,
    jstring jdata)
{
    const c_char* xmlWriter;
    const c_char* xmlData;
    const c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;
    xmlData = NULL;
    xmlResult = NULL;
    xmlWriter = (*env)->GetStringUTFChars(env, jwriter, 0);

    if(jdata != NULL){
        xmlData = (*env)->GetStringUTFChars(env, jdata, 0);
        xmlResult = cmx_writerWrite(xmlWriter, xmlData);
    }
    (*env)->ReleaseStringUTFChars(env, jdata, xmlData);
    (*env)->ReleaseStringUTFChars(env, jwriter, xmlWriter);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
    }

    return jresult;
}

/**
 * @brief Disposes one instance of userData into the system using the supplied writer.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniWriterDispose
 * - Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jwriter The writer to dispose the data with.
 * @param jdata The data to dispose.
 * @return Whether or not the dispose succeeded.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniWriterDispose)(
    JNIEnv *env,
    jobject this,
    jstring jwriter,
    jstring jdata)
{
    const c_char* xmlWriter;
    const c_char* xmlData;
    const c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;
    xmlData = NULL;
    xmlResult = NULL;
    xmlWriter = (*env)->GetStringUTFChars(env, jwriter, 0);

    if(jdata != NULL){
        xmlData = (*env)->GetStringUTFChars(env, jdata, 0);
        xmlResult = cmx_writerDispose(xmlWriter, xmlData);
    }
    (*env)->ReleaseStringUTFChars(env, jdata, xmlData);
    (*env)->ReleaseStringUTFChars(env, jwriter, xmlWriter);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
    }

    return jresult;
}

/**
 * @brief WriteDisposes one instance of userData into the system using the supplied writer.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniWriterWriteDispose
 * - Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jwriter The writer to writeDispose the data with.
 * @param jdata The data to writeDispose.
 * @return Whether or not the writeDispose succeeded.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniWriterWriteDispose)(
    JNIEnv *env,
    jobject this,
    jstring jwriter,
    jstring jdata)
{
    const c_char* xmlWriter;
    const c_char* xmlData;
    const c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;
    xmlData = NULL;
    xmlResult = NULL;
    xmlWriter = (*env)->GetStringUTFChars(env, jwriter, 0);

    if(jdata != NULL){
        xmlData = (*env)->GetStringUTFChars(env, jdata, 0);
        xmlResult = cmx_writerWriteDispose(xmlWriter, xmlData);
    }
    (*env)->ReleaseStringUTFChars(env, jdata, xmlData);
    (*env)->ReleaseStringUTFChars(env, jwriter, xmlWriter);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
    }

    return jresult;
}

/**
 * @brief Registers one instance of userData into the system using the supplied writer.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniWriterRegister
 * - Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jwriter The writer to register the data with.
 * @param jdata The data to register.
 * @return Whether or not the register succeeded.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniWriterRegister)(
    JNIEnv *env,
    jobject this,
    jstring jwriter,
    jstring jdata)
{
    const c_char* xmlWriter;
    const c_char* xmlData;
    const c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;
    xmlData = NULL;
    xmlResult = NULL;
    xmlWriter = (*env)->GetStringUTFChars(env, jwriter, 0);

    if(jdata != NULL){
        xmlData = (*env)->GetStringUTFChars(env, jdata, 0);
        xmlResult = cmx_writerRegister(xmlWriter, xmlData);
    }
    (*env)->ReleaseStringUTFChars(env, jdata, xmlData);
    (*env)->ReleaseStringUTFChars(env, jwriter, xmlWriter);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
    }

    return jresult;
}

/**
 * @brief Unregisters one instance of userData into the system using the supplied writer.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniWriterUnregister
 * - Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jwriter The writer to unregister the data with.
 * @param jdata The data to unregister.
 * @return Whether or not the unregister succeeded.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniWriterUnregister)(
    JNIEnv *env,
    jobject this,
    jstring jwriter,
    jstring jdata)
{
    const c_char* xmlWriter;
    const c_char* xmlData;
    const c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;
    xmlData = NULL;
    xmlResult = NULL;
    xmlWriter = (*env)->GetStringUTFChars(env, jwriter, 0);

    if(jdata != NULL){
        xmlData = (*env)->GetStringUTFChars(env, jdata, 0);
        xmlResult = cmx_writerUnregister(xmlWriter, xmlData);
    }
    (*env)->ReleaseStringUTFChars(env, jdata, xmlData);
    (*env)->ReleaseStringUTFChars(env, jwriter, xmlWriter);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
    }

    return jresult;
}

/**
 * @brief Creates a new publisher.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniPublisherNew
 * - Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jparticipant The participant to attach the entity to.
 * @param jname The name for the entity.
 * @param jqos The qos for the entity.
 * @return The XML representation of the created entity.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniPublisherNew)(
    JNIEnv *env,
    jobject this,
    jstring jparticipant,
    jstring jname,
    jstring jqos)
{
    const c_char* xmlParticipant;
    const c_char* xmlName;
    const c_char* xmlQos;
    c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    xmlQos = NULL;
    thisCopy = this;
    jresult = NULL;

    if(jqos != NULL){
        xmlQos = (*env)->GetStringUTFChars(env, jqos, 0);
    }
    xmlParticipant = (*env)->GetStringUTFChars(env, jparticipant, 0);
    xmlName = (*env)->GetStringUTFChars(env, jname, 0);

    xmlResult = cmx_publisherNew(xmlParticipant, xmlName, xmlQos);

    if(jqos != NULL){
        (*env)->ReleaseStringUTFChars(env, jqos, xmlQos);
    }
    (*env)->ReleaseStringUTFChars(env, jname, xmlName);
    (*env)->ReleaseStringUTFChars(env, jparticipant, xmlParticipant);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
        os_free(xmlResult);
    }

    return jresult;
}

/**
 * @brief Creates a new subscriber.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniSubscriberNew
 * - Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jparticipant The participant to attach the entity to.
 * @param jname The name for the entity.
 * @param jqos The qos for the entity
 * @return The XML representation of the created entity.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniSubscriberNew)(
    JNIEnv *env,
    jobject this,
    jstring jparticipant,
    jstring jname,
    jstring jqos)
{
    const c_char* xmlParticipant;
    const c_char* xmlName;
    const c_char* xmlQos;
    c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    xmlQos = NULL;
    thisCopy = this;
    jresult = NULL;

    if(jqos != NULL){
        xmlQos = (*env)->GetStringUTFChars(env, jqos, 0);
    }
    xmlParticipant = (*env)->GetStringUTFChars(env, jparticipant, 0);
    xmlName = (*env)->GetStringUTFChars(env, jname, 0);
    xmlResult = cmx_subscriberNew(xmlParticipant, xmlName, xmlQos);

    if(jqos != NULL){
        (*env)->ReleaseStringUTFChars(env, jqos, xmlQos);
    }
    (*env)->ReleaseStringUTFChars(env, jname, xmlName);
    (*env)->ReleaseStringUTFChars(env, jparticipant, xmlParticipant);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
        os_free(xmlResult);
    }
    return jresult;
}

/**
 * @brief Creates a new domain.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniDomainNew
 * - Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jparticipant The participant to attach the entity to.
 * @param jname The name for the entity.
 * @return The XML representation of the created entity.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniDomainNew)(
    JNIEnv *env,
    jobject this,
    jstring jparticipant,
    jstring jname)
{
    const c_char* xmlParticipant;
    const c_char* xmlName;
    c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;
    xmlParticipant = (*env)->GetStringUTFChars(env, jparticipant, 0);
    xmlName = (*env)->GetStringUTFChars(env, jname, 0);
    xmlResult = cmx_domainNew(xmlParticipant, xmlName);
    (*env)->ReleaseStringUTFChars(env, jname, xmlName);
    (*env)->ReleaseStringUTFChars(env, jparticipant, xmlParticipant);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
        os_free(xmlResult);
    }

    return jresult;
}

/**
 * @brief Creates a new topic.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniTopicNew
 * - Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jparticipant The participant to attach the entity to.
 * @param jname The name for the entity.
 * @param jname The type name for the topic.
 * @param jkeyList The keyList for the topic.
 * @param jqos The QoS for the topic.
 * @return The XML representation of the created entity.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniTopicNew)(
    JNIEnv *env,
    jobject this,
    jstring jparticipant,
    jstring jname,
    jstring jtypeName,
    jstring jkeyList,
    jstring jqos)
{
    const c_char* xmlParticipant;
    const c_char* xmlName;
    const c_char* xmlTypeName;
    const c_char* xmlKeyList;
    const c_char* xmlQos;
    c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    xmlQos = NULL;
    thisCopy = this;
    jresult = NULL;
    xmlParticipant = (*env)->GetStringUTFChars(env, jparticipant, 0);
    xmlName = (*env)->GetStringUTFChars(env, jname, 0);
    xmlTypeName = (*env)->GetStringUTFChars(env, jtypeName, 0);
    xmlKeyList = NULL;

    if(jkeyList != NULL){
        xmlKeyList = (*env)->GetStringUTFChars(env, jkeyList, 0);
    }
    if(jqos != NULL){
        xmlQos = (*env)->GetStringUTFChars(env, jqos, 0);
    }
    xmlResult = cmx_topicNew(xmlParticipant, xmlName, xmlTypeName, xmlKeyList, xmlQos);

    (*env)->ReleaseStringUTFChars(env, jname, xmlName);
    (*env)->ReleaseStringUTFChars(env, jtypeName, xmlTypeName);

    if(jqos != NULL){
        (*env)->ReleaseStringUTFChars(env, jqos, xmlQos);
    }
    if(jkeyList != NULL){
        (*env)->ReleaseStringUTFChars(env, jkeyList, xmlKeyList);
    }
    (*env)->ReleaseStringUTFChars(env, jparticipant, xmlParticipant);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
        os_free(xmlResult);
    }
    return jresult;
}

#if 0
/**
 * @brief Creates a new view.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniViewNew
 * - Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jparticipant The participant to attach the entity to.
 * @param jname The name for the entity.
 * @return The XML representation of the created entity.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniViewNew)(
    JNIEnv *env,
    jobject this,
    jstring jparticipant,
    jstring jname,
    jstring jexpression,
    jstring jqos)
{
    const c_char* xmlParticipant;
    const c_char* xmlName;
    const c_char* xmlExpression;
    const c_char* xmlQos;
    c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;
    xmlParticipant = (*env)->GetStringUTFChars(env, jparticipant, 0);
    xmlName = (*env)->GetStringUTFChars(env, jname, 0);
    xmlExpression = (*env)->GetStringUTFChars(env, jexpression, 0);
    xmlQos = NULL;

    if(jqos != NULL){
        xmlQos = (*env)->GetStringUTFChars(env, jqos, 0);
    }
    xmlResult = cmx_viewNew(xmlParticipant, xmlName, xmlExpression, xmlQos);

    (*env)->ReleaseStringUTFChars(env, jname, xmlName);
    (*env)->ReleaseStringUTFChars(env, jexpression, xmlExpression);
    (*env)->ReleaseStringUTFChars(env, jparticipant, xmlParticipant);

    if(jqos != NULL){
        (*env)->ReleaseStringUTFChars(env, jqos, xmlQos);
    }
    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
        os_free(xmlResult);
    }

    return jresult;
}
#endif

/**
 * @brief Creates a new query.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniQueryNew
 * - Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jreader The reader to attach the entity to.
 * @param jname The name for the entity.
 * @param jexpression The query expression for the entity.
 * @return The XML representation of the created entity.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniQueryNew)(
    JNIEnv *env,
    jobject this,
    jstring jreader,
    jstring jname,
    jstring jexpression)
{
    const c_char* xmlReader;
    const c_char* xmlName;
    const c_char* xmlExpression;
    c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;
    xmlReader = (*env)->GetStringUTFChars(env, jreader, 0);
    xmlName = (*env)->GetStringUTFChars(env, jname, 0);
    xmlExpression = (*env)->GetStringUTFChars(env, jexpression, 0);

    xmlResult = cmx_queryNew(xmlReader, xmlName, xmlExpression);

    (*env)->ReleaseStringUTFChars(env, jname, xmlName);
    (*env)->ReleaseStringUTFChars(env, jexpression, xmlExpression);
    (*env)->ReleaseStringUTFChars(env, jreader, xmlReader);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
        os_free(xmlResult);
    }

    return jresult;
}

/**
 * @brief Makes the publisher publish in the domains that match the supplied
 * expression.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniPublisherPublish
 * - Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jpublisher The publisher.
 * @param jexpression The domain expression.
 * @return Whether or not the action succeeded.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniPublisherPublish)(
    JNIEnv *env,
    jobject this,
    jstring jpublisher,
    jstring jexpression)
{
    const c_char* xmlPublisher;
    const c_char* xmlExpression;
    const c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;
    xmlPublisher = (*env)->GetStringUTFChars(env, jpublisher, 0);
    xmlExpression = (*env)->GetStringUTFChars(env, jexpression, 0);

    xmlResult = cmx_publisherPublish(xmlPublisher, xmlExpression);

    (*env)->ReleaseStringUTFChars(env, jexpression, xmlExpression);
    (*env)->ReleaseStringUTFChars(env, jpublisher, xmlPublisher);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
    }

    return jresult;
}

/**
 * @brief Creates a new writer.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniCreateWriter
 * - Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jpublisher The publisher to attach the entity to.
 * @param jname The name for the entity.
 * @param jtopic The topic that the writer must write.
 * @param jqos The QoS for the writer.
 * @return The XML representation of the created entity.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniCreateWriter)(
    JNIEnv *env,
    jobject this,
    jstring jpublisher,
    jstring jname,
    jstring jtopic,
    jstring jqos)
{
    const c_char* xmlPublisher;
    const c_char* xmlName;
    const c_char* xmlTopic;
    const c_char* xmlQos;
    c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    xmlQos = NULL;
    thisCopy = this;
    jresult = NULL;
    xmlPublisher = (*env)->GetStringUTFChars(env, jpublisher, 0);
    xmlName = (*env)->GetStringUTFChars(env, jname, 0);
    xmlTopic = (*env)->GetStringUTFChars(env, jtopic, 0);

    if(jqos != NULL){
        xmlQos = (*env)->GetStringUTFChars(env, jqos, 0);
    }
    xmlResult = cmx_writerNew(xmlPublisher, xmlName, xmlTopic, xmlQos);

    if(jqos != NULL){
        (*env)->ReleaseStringUTFChars(env, jqos, xmlQos);
    }
    (*env)->ReleaseStringUTFChars(env, jname, xmlName);
    (*env)->ReleaseStringUTFChars(env, jtopic, xmlTopic);
    (*env)->ReleaseStringUTFChars(env, jpublisher, xmlPublisher);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
        os_free(xmlResult);
    }

    return jresult;
}

/**
 * @brief Makes the subscriber subscribe to the domains that match the supplied
 * expression.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniSubscriberSubscribe
 * - Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jsubscriber The subscriber.
 * @param jexpression The domain expression.
 * @return Whether or not the action succeeded.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniSubscriberSubscribe)(
    JNIEnv *env,
    jobject this,
    jstring jsubscriber,
    jstring jexpression)
{
    const c_char* xmlSubscriber;
    const c_char* xmlExpression;
    const c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;
    xmlSubscriber = (*env)->GetStringUTFChars(env, jsubscriber, 0);
    xmlExpression = (*env)->GetStringUTFChars(env, jexpression, 0);

    xmlResult = cmx_subscriberSubscribe(xmlSubscriber, xmlExpression);

    (*env)->ReleaseStringUTFChars(env, jexpression, xmlExpression);
    (*env)->ReleaseStringUTFChars(env, jsubscriber, xmlSubscriber);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
    }

    return jresult;
}

/**
 * @brief Creates a new dataReader.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniCreateDataReader
 * - Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jsubscriber The subscriber to attach the entity to.
 * @param jname The name for the entity.
 * @param jview The view that must be applied on the reader.
 * @param jqos The QoS for the reader.
 * @return The XML representation of the created entity.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniCreateDataReader)(
    JNIEnv *env,
    jobject this,
    jstring jsubscriber,
    jstring jname,
    jstring jview,
    jstring jqos)
{
    const c_char* xmlSubscriber;
    const c_char* xmlName;
    const c_char* xmlView;
    const c_char* xmlQos;
    c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    xmlQos = NULL;
    thisCopy = this;
    jresult = NULL;
    xmlSubscriber = (*env)->GetStringUTFChars(env, jsubscriber, 0);
    xmlName = (*env)->GetStringUTFChars(env, jname, 0);
    xmlView = (*env)->GetStringUTFChars(env, jview, 0);

    if(jqos != NULL){
        xmlQos = (*env)->GetStringUTFChars(env, jqos, 0);
    }
    xmlResult = cmx_dataReaderNew(xmlSubscriber, xmlName, xmlView, xmlQos);

    if(jqos != NULL){
        (*env)->ReleaseStringUTFChars(env, jqos, xmlQos);
    }
    (*env)->ReleaseStringUTFChars(env, jname, xmlName);
    (*env)->ReleaseStringUTFChars(env, jview, xmlView);
    (*env)->ReleaseStringUTFChars(env, jsubscriber, xmlSubscriber);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
        os_free(xmlResult);
    }

    return jresult;
}

/**
 * @brief Creates a new waitset.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniWaitsetNew
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jparticipant the participant where the waitset must be created in.
 * @return The XML representation of the created entity.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniWaitsetNew)(
    JNIEnv *env,
    jobject this,
    jstring jparticipant)
{
    const c_char* xmlParticipant;
    c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;
    xmlParticipant = (*env)->GetStringUTFChars(env, jparticipant, 0);
    xmlResult = cmx_waitsetNew(xmlParticipant);
    (*env)->ReleaseStringUTFChars(env, jparticipant, xmlParticipant);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
        os_free(xmlResult);
    }
    return jresult;
}

/**
 * @brief Attaches the supplied entity to the supplied waitset.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniWaitsetAttach
 * - Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jwaitset The waitset where to attach the entity to.
 * @param jentity The entity to attach to the waitset.
 * @return Whether or not the action succeeded.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniWaitsetAttach)(
    JNIEnv *env,
    jobject this,
    jstring jwaitset,
    jstring jentity)
{
    const c_char* xmlWaitset;
    const c_char* xmlEntity;
    const c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;

    xmlWaitset = (*env)->GetStringUTFChars(env, jwaitset, 0);
    xmlEntity = (*env)->GetStringUTFChars(env, jentity, 0);

    xmlResult = cmx_waitsetAttach(xmlWaitset, xmlEntity);

    (*env)->ReleaseStringUTFChars(env, jwaitset, xmlWaitset);
    (*env)->ReleaseStringUTFChars(env, jentity, xmlEntity);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
    }
    return jresult;
}

/**
 * @brief Detaches the supplied entity from the supplied waitset.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniWaitsetDetach
 * - Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jwaitset The waitset where to detach the entity from.
 * @param jentity The entity to detach from the waitset.
 * @return Whether or not the action succeeded.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniWaitsetDetach)(
    JNIEnv *env,
    jobject this,
    jstring jwaitset,
    jstring jentity)
{
    const c_char* xmlWaitset;
    const c_char* xmlEntity;
    const c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;

    xmlWaitset = (*env)->GetStringUTFChars(env, jwaitset, 0);
    xmlEntity = (*env)->GetStringUTFChars(env, jentity, 0);

    xmlResult = cmx_waitsetDetach(xmlWaitset, xmlEntity);

    (*env)->ReleaseStringUTFChars(env, jwaitset, xmlWaitset);
    (*env)->ReleaseStringUTFChars(env, jentity, xmlEntity);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
    }
    return jresult;
}

/**
 * @brief Waits in a blocking call until the supplied waitset is triggered.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniWaitsetWait
 * - Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jwaitset The waitset where to wait for.
 * @return A list of entities that had an event.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniWaitsetWait)(
    JNIEnv *env,
    jobject this,
    jstring jwaitset)
{
    const c_char* xmlWaitset;
    c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;

    xmlWaitset = (*env)->GetStringUTFChars(env, jwaitset, 0);
    xmlResult = cmx_waitsetWait(xmlWaitset);
    (*env)->ReleaseStringUTFChars(env, jwaitset, xmlWaitset);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
        os_free(xmlResult);
    }
    return jresult;
}

/**
 * @brief Waits maxim jsec.jnsec (s.ns) time until the supplied waitset is triggered.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniWaitsetTimedWait
 * - Signature: (Ljava/lang/String;II)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jwaitset The waitset where to wait for.
 * @return A list of entities that had an event.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniWaitsetTimedWait)(
    JNIEnv *env,
    jobject this,
    jstring jwaitset,
    jint jsec,
    jint jnsec)
{
    const c_char* xmlWaitset;
    c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;
    c_time timeout;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;

    timeout.seconds = (c_long)jsec;
    timeout.nanoseconds = (c_ulong)jnsec;

    xmlWaitset = (*env)->GetStringUTFChars(env, jwaitset, 0);
    xmlResult = cmx_waitsetTimedWait(xmlWaitset, timeout);

    (*env)->ReleaseStringUTFChars(env, jwaitset, xmlWaitset);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
        os_free(xmlResult);
    }
    return jresult;
}

/**
 * @brief Resolves the event mask of the supplied waitset.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniWaitsetGetEventMask
 * - Signature: (Ljava/lang/String;)I
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jwaitset The waitset to resolve the event mask from.
 * @return The event mask of the waitset.
 */
JNIEXPORT jint JNICALL
FUNCTION(jniWaitsetGetEventMask)(
    JNIEnv *env,
    jobject this,
    jstring jwaitset)
{
    const c_char* xmlWaitset;
    jint jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = 0;

    xmlWaitset = (*env)->GetStringUTFChars(env, jwaitset, 0);
    jresult = (jint)cmx_waitsetGetEventMask(xmlWaitset);
    (*env)->ReleaseStringUTFChars(env, jwaitset, xmlWaitset);

    return jresult;
}

/**
 * @brief Sets the event mask of the supplied waitset.
 *
 * - Class:     org_opensplice_api_cm_com_JniCommunicator
 * - Method:    jniWaitsetSetEventMask
 * - Signature: (Ljava/lang/String;I)Ljava/lang/String;
 *
 * @param env The JNI environment.
 * @param this The Java object that called this function.
 * @param jwaitset The waitset to set the event mask of.
 * @param jmask The event mask to apply to the waitset.
 * @return Whether the applying of the mask succeeded.
 */
JNIEXPORT jstring JNICALL
FUNCTION(jniWaitsetSetEventMask)(
    JNIEnv *env,
    jobject this,
    jstring jwaitset,
    jint jmask)
{
    const c_char* xmlWaitset;
    const c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;

    xmlWaitset = (*env)->GetStringUTFChars(env, jwaitset, 0);
    xmlResult = cmx_waitsetSetEventMask(xmlWaitset, (c_ulong)jmask);
    (*env)->ReleaseStringUTFChars(env, jwaitset, xmlWaitset);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
    }
    return jresult;
}

/**
 * @brief Checks whether the connection is still alive.
 *
 * If it is not alive, the <code>connectionAlive</code> attribute of the
 * <code>JniCommunicator</code> is set to false.
 * @param env The current JNI environment.
 */
static void
cmj_checkConnection(
    JNIEnv* env)
{
    jclass communicator;
    jfieldID field;
    jboolean value;

    if((cmj_isInitialized == TRUE) && (cmx_isInitialized() == FALSE)){
        communicator = (*env)->FindClass(env, "org/opensplice/cm/com/JniCommunicator");

        if(communicator != NULL){
            field = (*env)->GetStaticFieldID(env, communicator, "connectionAlive", "Z");

            if(field != NULL){
                value = JNI_FALSE;
                (*env)->SetStaticBooleanField(env, communicator, field, value);
            } else {
                printf("Field could not be found.\n");
            }
        } else {
           printf("Communicator class could not be found.\n");
        }
    }
}

JNIEXPORT jstring JNICALL
FUNCTION(jniDataReaderWaitForHistoricalData)(
    JNIEnv *env,
    jobject this,
    jstring jdataReader,
    jint jsec,
    jint jnsec)
{
    const c_char* xmlDataReader;
    const c_char* xmlResult;
    jstring jresult;
    jobject thisCopy;
    c_time timeout;

    cmj_checkConnection(env);
    thisCopy = this;
    jresult = NULL;

    timeout.seconds = (c_long)jsec;
    timeout.nanoseconds = (c_ulong)jnsec;

    xmlDataReader = (*env)->GetStringUTFChars(env, jdataReader, 0);
    xmlResult = cmx_dataReaderWaitForHistoricalData(xmlDataReader, timeout);

    (*env)->ReleaseStringUTFChars(env, jdataReader, xmlDataReader);

    if(xmlResult != NULL){
        jresult = (*env)->NewStringUTF(env, xmlResult);
    }
    return jresult;
}


JNIEXPORT jstring JNICALL
FUNCTION(jniStorageOpen) (
        JNIEnv *env,
        jobject this,
        jstring  jattrs)
{
    const c_char* attrs;
    c_char* xmlStorage;
    jobject unusedThis;
    jstring jresult;

    unusedThis = this;
    jresult = NULL;

    cmj_checkConnection(env);

    attrs = (*env)->GetStringUTFChars(env, jattrs, 0);
    xmlStorage = cmx_storageOpen(attrs);
    (*env)->ReleaseStringUTFChars(env, jattrs, attrs);

    if(xmlStorage != NULL){
       jresult = (*env)->NewStringUTF(env, xmlStorage);
       os_free(xmlStorage);
    }

    return jresult;
}


JNIEXPORT jstring JNICALL
FUNCTION(jniStorageClose) (
        JNIEnv *env,
        jobject this,
        jstring jstorage)
{
    const c_char* xmlStorage;
    c_char* xmlResult;
    jobject unusedThis;
    jstring jresult;

    unusedThis = this;
    jresult = NULL;

    cmj_checkConnection(env);

    xmlStorage = (*env)->GetStringUTFChars(env, jstorage, 0);
    xmlResult = cmx_storageClose(xmlStorage);
    (*env)->ReleaseStringUTFChars(env, jstorage, xmlStorage);

    if(xmlResult != NULL){
       jresult = (*env)->NewStringUTF(env, xmlResult);
       os_free(xmlResult);
    }

    return jresult;
}

JNIEXPORT jstring JNICALL
FUNCTION(jniStorageAppend) (
        JNIEnv *env,
        jobject this,
        jstring jstorage,
        jstring jmetadata,
        jstring jdata)
{
    const c_char* xmlStorage;
    const c_char* xmlMetadata;
    const c_char* xmlData;
    c_char* xmlResult;
    jobject unusedThis;
    jstring jresult;

    unusedThis = this;
    jresult = NULL;

    cmj_checkConnection(env);

    xmlStorage = (*env)->GetStringUTFChars(env, jstorage, 0);
    xmlMetadata = (*env)->GetStringUTFChars(env, jmetadata, 0);
    xmlData = (*env)->GetStringUTFChars(env, jdata, 0);
    xmlResult = cmx_storageAppend(xmlStorage, xmlMetadata, xmlData);
    (*env)->ReleaseStringUTFChars(env, jdata, xmlData);
    (*env)->ReleaseStringUTFChars(env, jmetadata, xmlMetadata);
    (*env)->ReleaseStringUTFChars(env, jstorage, xmlStorage);

    if(xmlResult != NULL){
       jresult = (*env)->NewStringUTF(env, xmlResult);
       os_free(xmlResult);
    }

    return jresult;
}

JNIEXPORT jstring JNICALL
FUNCTION(jniStorageRead) (
        JNIEnv *env,
        jobject this,
        jstring jstorage)
{
    const c_char* xmlStorage;
    c_char* xmlResult;
    jobject unusedThis;
    jstring jresult;

    unusedThis = this;
    jresult = NULL;

    cmj_checkConnection(env);

    xmlStorage = (*env)->GetStringUTFChars(env, jstorage, 0);
    xmlResult = cmx_storageRead(xmlStorage);
    (*env)->ReleaseStringUTFChars(env, jstorage, xmlStorage);

    if(xmlResult != NULL){
       jresult = (*env)->NewStringUTF(env, xmlResult);
       os_free(xmlResult);
    }

    return jresult;
}

JNIEXPORT jstring JNICALL
FUNCTION(jniStorageGetType) (
        JNIEnv *env,
        jobject this,
        jstring jstorage,
        jstring jtypename)
{
    const c_char* xmlStorage;
    const c_char* xmlTypeName;
    c_char* xmlResult;
    jobject unusedThis;
    jstring jresult;

    unusedThis = this;
    jresult = NULL;

    cmj_checkConnection(env);

    xmlStorage = (*env)->GetStringUTFChars(env, jstorage, 0);
    xmlTypeName = (*env)->GetStringUTFChars(env, jtypename, 0);
    xmlResult = cmx_storageGetType(xmlStorage, xmlTypeName);
    (*env)->ReleaseStringUTFChars(env, jtypename, xmlTypeName);
    (*env)->ReleaseStringUTFChars(env, jstorage, xmlStorage);

    if(xmlResult != NULL){
       jresult = (*env)->NewStringUTF(env, xmlResult);
       os_free(xmlResult);
    }

    return jresult;
}
