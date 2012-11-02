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

#include "saj_utilities.h"
#include "saj_qosUtils.h"
#include "saj_domainParticipantListener.h"
#include "saj_extDomainParticipantListener.h"
#include "saj_DomainParticipantFactory.h"
#include "os_process.h"
#include "os_stdlib.h"
#include "os_heap.h"
#include "os_report.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_DomainParticipantFactoryImpl_##name

/*
 * Method: jniGetInstance
 * Param : domainId
 * Return: DDS.DomainParticipantFactory;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetInstance)(
  JNIEnv *env,
  jclass jDomainParticipantFactory)
{
    jobject jParticipantFactory;
    gapi_domainParticipantFactory gapiParticipantFactory;
    saj_returnCode retCode;
    char* ldPreload;
    jParticipantFactory = NULL;

    ldPreload = os_getenv("LD_PRELOAD");

    if(ldPreload){
        if(strstr(ldPreload, "jsig") == NULL){
            os_procSetSignalHandlingEnabled(0);
        }
    } else {
        os_procSetSignalHandlingEnabled(0);
    }
    retCode = saj_InitializeSAJ(env);

    if(retCode == SAJ_RETCODE_OK){
        gapiParticipantFactory = gapi_domainParticipantFactory_get_instance ();

        if(gapiParticipantFactory != NULL){
            saj_construct_java_object(env, PACKAGENAME"DomainParticipantFactoryImpl",
                                          (PA_ADDRCAST)gapiParticipantFactory,
                                          &jParticipantFactory);
        }
    }
    return jParticipantFactory;
}

/*
 * Method: jniCreateParticipant
 * Param : domain id
 * Param : DDS.DomainParticipantQos
 * Param : DDS.DomainParticipantListener
 * Return: DDS.DomainParticipant
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniCreateParticipant) (
  JNIEnv  *env,
  jobject this,
  jstring jDomainId,
  jobject jParticipantQos,
  jobject jlistener,
  jint jmask)
{
    jobject javaDomainParticipant;
    gapi_domainParticipant gapiDomainParticipant;
    const gapi_char* domainId;
    gapi_domainParticipantQos* participantQos;
    const struct gapi_domainParticipantListener *listener;
    gapi_domainParticipantFactory factory;
    saj_returnCode rc;
    JavaVM *vm;

    jclass tempClass;
    jboolean result;

    javaDomainParticipant = NULL;
    gapiDomainParticipant = GAPI_OBJECT_NIL;
    domainId = NULL;

	if ((*env)->IsSameObject (env, jParticipantQos, GET_CACHED(PARTICIPANT_QOS_DEFAULT)) == JNI_FALSE) {
        /* Not default QoS */
            participantQos = gapi_domainParticipantQos__alloc();
            rc = saj_DomainParticipantQosCopyIn(env, jParticipantQos, participantQos);
        } else {
        /* Default QoS */
        participantQos = (gapi_domainParticipantQos *)GAPI_PARTICIPANT_QOS_DEFAULT;
        rc = SAJ_RETCODE_OK;
    }

    /* We can check Java instances in the jni layer, so here we check
     * that if the mask is set to GAPI_ALL_DATA_DISPOSED_STATUS we have
     * been given an ExtDomainParticipantListener to call. If not then
     * an error is reported and a GAPI_RETCODE_BAD_PARAMETER status is
     * returned. */
    if(jmask & GAPI_ALL_DATA_DISPOSED_STATUS) {
        tempClass = (*env)->FindClass(env, "DDS/ExtDomainParticipantListener");
        result = (*env)->IsInstanceOf(env, jlistener, tempClass);
        if(result == JNI_FALSE) {
            OS_REPORT(OS_ERROR, "dcpssaj", 0, "ExtDomainParticipantListener must be used when the ALL_DATA_DISPOSED_STATUS bit is set.");
            rc = SAJ_RETCODE_ERROR;
        }  
    }

        if(rc == SAJ_RETCODE_OK){
            if(jDomainId != NULL){
                domainId = (*env)->GetStringUTFChars(env, jDomainId, 0);
            }

            factory = (gapi_domainParticipantFactory)saj_read_gapi_address(env, this);
            if(jmask & GAPI_ALL_DATA_DISPOSED_STATUS) {
	        listener = saj_extDomainParticipantListenerNew(env, jlistener);
            } else {
                listener = saj_domainParticipantListenerNew(env, jlistener);
	    }

            if(listener != NULL){
                saj_write_java_listener_address(env, gapiDomainParticipant,
                                                listener->listener_data);
            }

            if((*env)->GetJavaVM(env, &vm) == 0){
                gapiDomainParticipant =
                            gapi_domainParticipantFactory_create_participant(
                                factory, (const gapi_domainId_t)domainId,
                                (const gapi_domainParticipantQos *)participantQos,
                                (const struct gapi_domainParticipantListener *)listener,
                                (const gapi_statusMask)jmask,
                                saj_listenerAttach, saj_listenerDetach, (void*)vm);
            }

        if (participantQos != (gapi_domainParticipantQos *)GAPI_PARTICIPANT_QOS_DEFAULT) {
                gapi_free(participantQos);
        }

            if (gapiDomainParticipant != NULL){
                gapi_domainParticipantFactoryQos *dpfqos = gapi_domainParticipantFactoryQos__alloc();
                rc = saj_construct_java_object(env,
                                                 PACKAGENAME "DomainParticipantImpl",
                                                 (PA_ADDRCAST)gapiDomainParticipant,
                                                 &javaDomainParticipant);
                if(dpfqos){
                    if(gapi_domainParticipantFactory_get_qos(factory, dpfqos) == GAPI_RETCODE_OK){
                        if(dpfqos->entity_factory.autoenable_created_entities) {
                            gapi_entity_enable(gapiDomainParticipant);
                        }
                    }
                    gapi_free(dpfqos);
                }

        } else if(listener != NULL){
            saj_listenerDataFree(env, saj_listenerData(listener->listener_data));
        }
        if(jDomainId != NULL){
            (*env)->ReleaseStringUTFChars(env, jDomainId, domainId);
        }
    }

    return javaDomainParticipant;
}

/*
 * Method: jniDeleteParticipant
 * Param : DDS.DomainParticipant
 * Return: return code
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDeleteParticipant) (
    JNIEnv  *env,
    jobject this,
    jobject jParticipant)
{
    gapi_returnCode_t rc;
    gapi_domainParticipantFactory factory;
    gapi_domainParticipant participant;
    saj_userData ud;
    c_bool must_free;

    factory = (gapi_domainParticipantFactory)saj_read_gapi_address(env, this);
    participant = (gapi_domainParticipant)saj_read_gapi_address(env, jParticipant);

    ud = gapi_object_get_user_data(participant);

    must_free = saj_setThreadEnv(env);
    rc = gapi_domainParticipantFactory_delete_participant(factory, participant);
    saj_delThreadEnv(must_free);

    return (jint)rc;
}

/*
 * Method: jniLookupParticipant
 * Param : domain id
 * Return: DDS.DomainParticipant
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniLookupParticipant) (
    JNIEnv  *env,
    jobject this,
    jstring jDomainId)
{
    jobject javaDomainParticipant;
    gapi_domainParticipant gapiDomainParticipant;
    gapi_domainParticipantFactory factory;
    const gapi_char* domainId;

    domainId = NULL;
    javaDomainParticipant = NULL;
    gapiDomainParticipant =  GAPI_OBJECT_NIL;

    if(jDomainId != NULL){
        domainId = (*env)->GetStringUTFChars(env, jDomainId, 0);
    }
    factory = (gapi_domainParticipantFactory)saj_read_gapi_address(env, this);


    gapiDomainParticipant = gapi_domainParticipantFactory_lookup_participant(
	                               factory, (const gapi_domainId_t) domainId);

    if (gapiDomainParticipant != NULL){
        javaDomainParticipant = saj_read_java_address(gapiDomainParticipant);
    }
    if(jDomainId != NULL){
        (*env)->ReleaseStringUTFChars(env, jDomainId, domainId);
    }
	return javaDomainParticipant;
}
/*
 * Method: jniSetDefaultParticipantQos
 * Param : DDS.DomainParticipantQos
 * Return: return code
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetDefaultParticipantQos) (
    JNIEnv  *env,
    jobject this,
    jobject jParticipantQos)
{
    gapi_domainParticipantQos* participantQos;
    gapi_domainParticipantFactory factory;
    saj_returnCode rc;
    gapi_returnCode_t result;

    result = GAPI_RETCODE_ERROR;
    participantQos = gapi_domainParticipantQos__alloc();
    rc = saj_DomainParticipantQosCopyIn(env, jParticipantQos, participantQos);

    if (rc == SAJ_RETCODE_OK){
        factory = (gapi_domainParticipantFactory)saj_read_gapi_address(env, this);
        result  = gapi_domainParticipantFactory_set_default_participant_qos(
                                                    factory, participantQos);
        gapi_free(participantQos);
    }
    return (jint)result;
}

/*
 * Method: jniGetDefaultParticipantQos
 * Param : DDS.DomainParticipantQosHolder
 */
JNIEXPORT jint JNICALL SAJ_FUNCTION(jniGetDefaultParticipantQos) (
  JNIEnv  *env,
  jobject this,
  jobject jQosHolder)
{
    saj_returnCode rc;
    jobject javaQos;
    gapi_domainParticipantQos* gapiQos;
    gapi_returnCode_t result;

    javaQos = NULL;

    if(jQosHolder != NULL){
        gapiQos = gapi_domainParticipantQos__alloc();

        result = gapi_domainParticipantFactory_get_default_participant_qos (
                (gapi_domainParticipantFactory)saj_read_gapi_address(env, this),
                gapiQos);

        if(result == GAPI_RETCODE_OK){
            rc = saj_DomainParticipantQosCopyOut(env, gapiQos, &javaQos);
            gapi_free(gapiQos);

            if (rc == SAJ_RETCODE_OK){
                /* store the java qos in the holder object */
                (*env)->SetObjectField(env, jQosHolder,
                                       GET_CACHED(domainParticipantQosHolder_value_fid),
                                       javaQos);
                (*env)->DeleteLocalRef(env, javaQos);
            } else {
                result = GAPI_RETCODE_ERROR;
            }
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return (jint)result;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetQos) (
    JNIEnv        *env,
    jobject       this,
    jobject       jDomainParticipantFactoryQosHolder)
{
    gapi_domainParticipantFactoryQos* gapiQos;
    saj_returnCode rc;
    jobject javaQos;
    gapi_domainParticipantFactory factory;
    gapi_returnCode_t result;

    if(jDomainParticipantFactoryQosHolder != NULL){
        factory = (gapi_domainParticipantFactory)saj_read_gapi_address(env, this);
        javaQos = NULL;

        gapiQos = gapi_domainParticipantFactoryQos__alloc();
        result = gapi_domainParticipantFactory_get_qos(factory, gapiQos);

        if(result == GAPI_RETCODE_OK){
            rc = saj_DomainParticipantFactoryQosCopyOut(env, gapiQos, &javaQos);

            if(rc == SAJ_RETCODE_OK){
                /* store the DomainParticipantFactoryQos object in the Holder object */
                (*env)->SetObjectField(env, jDomainParticipantFactoryQosHolder,
                        GET_CACHED(domainParticipantFactoryQosHolder_value_fid), javaQos);

                /* delete the local reference to the DomainParticipantQos object */
                (*env)->DeleteLocalRef(env, javaQos);
            } else {
                result = GAPI_RETCODE_ERROR;
            }
        }
        gapi_free(gapiQos);
    }
    return (jint)result;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetQos) (
    JNIEnv *env,
    jobject this,
    jobject jDomainParticipantFactoryQos)
{
    gapi_domainParticipantFactoryQos* gapiQos;
    gapi_domainParticipantFactory factory;
    saj_returnCode rc;
    gapi_returnCode_t result;

    gapiQos = gapi_domainParticipantFactoryQos__alloc();
    factory = (gapi_domainParticipantFactory)saj_read_gapi_address(env, this);
    rc = saj_DomainParticipantFactoryQosCopyIn(env, jDomainParticipantFactoryQos, gapiQos);
    result = GAPI_RETCODE_ERROR;

    if(rc == SAJ_RETCODE_OK){
        result = gapi_domainParticipantFactory_set_qos(factory, gapiQos);
    }
    gapi_free(gapiQos);

    return (jint)result;
}

JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniLookupDomain) (
    JNIEnv *env,
    jobject this,
    jstring jDomainId)
{
    gapi_domainParticipantFactory factory;
    gapi_domain gapiDomain;
    jobject javaDomain;
    const gapi_char* domainId;

    domainId   = NULL;
    javaDomain = NULL;
    gapiDomain = GAPI_OBJECT_NIL;

    if(jDomainId != NULL){
        domainId = (*env)->GetStringUTFChars(env, jDomainId, 0);
    }
    factory = (gapi_domainParticipantFactory)saj_read_gapi_address(env, this);

    gapiDomain = gapi_domainParticipantFactory_lookup_domain(factory, (gapi_domainId_t)domainId);

    if (gapiDomain != NULL){
        javaDomain = saj_read_java_address(gapiDomain);
    }
    if(jDomainId != NULL){
        (*env)->ReleaseStringUTFChars(env, jDomainId, domainId);
    }
    return javaDomain;
}

/*
 * Method: jniDeleteDomain
 * Param : DDS.Domain
 * Return: return code
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDeleteDomain) (
    JNIEnv  *env,
    jobject this,
    jobject jDomain)
{
    gapi_returnCode_t rc;
    gapi_domainParticipantFactory factory;
    gapi_domain domain;
    c_bool must_free;

    factory = (gapi_domainParticipantFactory)saj_read_gapi_address(env, this);
    domain = (gapi_domain)saj_read_gapi_address(env, jDomain);

    must_free = saj_setThreadEnv(env);
    rc = gapi_domainParticipantFactory_delete_domain(factory, domain);
    saj_delThreadEnv(must_free);

    return (jint)rc;
}

/*
 * Method: jniDeleteContainedEntities
 * Param :
 * Return: return code
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDeleteContainedEntities)(
    JNIEnv  *env,
    jobject this)
{
    gapi_returnCode_t rc;
    gapi_domainParticipantFactory factory;
    c_bool must_free;

    factory = (gapi_domainParticipantFactory)saj_read_gapi_address(env, this);

    must_free = saj_setThreadEnv(env);
    rc = gapi_domainParticipantFactory_delete_contained_entities(factory);
    saj_delThreadEnv(must_free);

    return (jint)rc;
}

#undef SAJ_FUNCTION
