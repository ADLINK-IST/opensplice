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

#include "saj_DomainParticipant.h"
#include "saj_utilities.h"
#include "saj_qosUtils.h"
#include "saj_domainParticipantListener.h"
#include "saj_extDomainParticipantListener.h"
#include "saj_topicListener.h"
#include "saj_extTopicListener.h"
#include "saj_publisherListener.h"
#include "saj_subscriberListener.h"
#include "saj_copyOut.h"
#include "saj_copyCache.h"

#include "gapi.h"

#include "v_public.h"
#include "v_dataReaderInstance.h"
#include "u_instanceHandle.h"

#include "os_report.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_DomainParticipantImpl_##name

static saj_returnCode saj_domainParticipantInitBuiltinSubscriber(
                            JNIEnv *env,
                            gapi_domainParticipant participant,
                            gapi_subscriber subscriber);

static saj_returnCode saj_domainParticipantInitBuiltinDataReader(
                            JNIEnv *env,
                            gapi_domainParticipant participant,
                            gapi_subscriber subscriber,
                            const gapi_char* topicName,
                            const gapi_char* typeName,
                            const gapi_char* dataReaderClassName,
                            const gapi_char* dataReaderConstructorSignature);

/*
 * Method:      jniCreatePublisher
 * Param :      PublisherQos
 * Param :      PublisherListener
 * Return:      Publisher
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniCreatePublisher)(
    JNIEnv *env,
    jobject this,
    jobject publisher_qos,
    jobject jlistener,
    jint jmask)
{
    jobject javaPublisher;
    gapi_publisher gapiPublisher;
    gapi_domainParticipant participant;
    gapi_publisherQos* pubQos;
    const struct gapi_publisherListener *listener;
    saj_returnCode rc;

    javaPublisher = NULL;
    listener = NULL;
    gapiPublisher = GAPI_OBJECT_NIL;

    if ((*env)->IsSameObject (env, publisher_qos, GET_CACHED(PUBLISHER_QOS_DEFAULT)) == JNI_FALSE) {
        pubQos = gapi_publisherQos__alloc();
        rc = saj_PublisherQosCopyIn(env, publisher_qos, pubQos);
    } else {
    pubQos = (gapi_publisherQos *)GAPI_PUBLISHER_QOS_DEFAULT;
        rc = SAJ_RETCODE_OK;
    }

    participant = (gapi_domainParticipant) saj_read_gapi_address(env, this);

    if(rc == SAJ_RETCODE_OK){
        listener = saj_publisherListenerNew(env, jlistener);

        if(listener != NULL){
            saj_write_java_listener_address(env, gapiPublisher,  listener->listener_data);
        }
        gapiPublisher = gapi_domainParticipant_create_publisher(participant, pubQos, listener, (gapi_statusMask)jmask);

        if (gapiPublisher != GAPI_OBJECT_NIL){
            gapi_domainParticipantQos *dpqos = gapi_domainParticipantQos__alloc();
            rc = saj_construct_java_object(env,  PACKAGENAME "PublisherImpl",
                                            (PA_ADDRCAST)gapiPublisher,
                                            &javaPublisher);

            if(dpqos){
                if(gapi_domainParticipant_get_qos(participant, dpqos) == GAPI_RETCODE_OK){
                    if(dpqos->entity_factory.autoenable_created_entities) {
                        gapi_entity_enable(gapiPublisher);
                    }
                }
                gapi_free(dpqos);
            }

        } else if(listener != NULL){
            saj_listenerDataFree(env, saj_listenerData(listener->listener_data));
        }
    }
    if (pubQos != (gapi_publisherQos *)GAPI_PUBLISHER_QOS_DEFAULT) {
        gapi_free(pubQos);
    }

    return javaPublisher;
}

/*
 * Method:      jniDeletePublisher
 * Param :      Publisher
 * Return:      Return code
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDeletePublisher) (
    JNIEnv* env,
    jobject this,
    jobject publisher)
{
    gapi_publisher gapiPublisher;
    gapi_domainParticipant participant;
    gapi_returnCode_t grc;
    c_bool must_free;

    participant = (gapi_domainParticipant) saj_read_gapi_address(env, this);
    gapiPublisher = (gapi_publisher) saj_read_gapi_address(env, publisher);

    must_free = saj_setThreadEnv(env);
    grc = gapi_domainParticipant_delete_publisher(participant, gapiPublisher);
    saj_delThreadEnv(must_free);

    return (jint)grc;
}

/*
 * Method:      jniCreateSubscriber
 * Param :      SubscriberQos
 * Param :      SubscriberListener
 * Return:      Subscriber
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniCreateSubscriber) (
    JNIEnv * env,
    jobject this,
    jobject qos,
    jobject jlistener,
    jint jmask)
{
    gapi_subscriberQos* gapiSubscriberQos;
    jobject javaSubscriber;
    saj_returnCode rc;
    gapi_domainParticipant participant;
    gapi_subscriber gapiSubscriber;
    struct gapi_subscriberListener *listener;

    javaSubscriber = NULL;
    gapiSubscriber = GAPI_OBJECT_NIL;
    listener = NULL;

    if ((*env)->IsSameObject (env, qos, GET_CACHED(SUBSCRIBER_QOS_DEFAULT)) == JNI_FALSE) {
        gapiSubscriberQos = gapi_subscriberQos__alloc();
        rc = saj_SubscriberQosCopyIn(env, qos, gapiSubscriberQos);
    } else {
    gapiSubscriberQos = (gapi_subscriberQos *)GAPI_SUBSCRIBER_QOS_DEFAULT;
    rc = SAJ_RETCODE_OK;
    }

    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);

    if(rc == SAJ_RETCODE_OK){
        listener = saj_subscriberListenerNew(env, jlistener);

        if(listener != NULL){
            saj_write_java_listener_address(env, gapiSubscriber, listener->listener_data);
        }
        gapiSubscriber = gapi_domainParticipant_create_subscriber(
                                                participant, gapiSubscriberQos,
                                                listener, (gapi_statusMask)jmask);
        if(gapiSubscriber != GAPI_OBJECT_NIL){
            gapi_domainParticipantQos *dpqos = gapi_domainParticipantQos__alloc();
            rc = saj_construct_java_object(env, PACKAGENAME "SubscriberImpl",
                                              (PA_ADDRCAST)gapiSubscriber,
                                              &javaSubscriber);

            if(dpqos){
                if(gapi_domainParticipant_get_qos(participant, dpqos) == GAPI_RETCODE_OK){
                    if(dpqos->entity_factory.autoenable_created_entities) {
                        gapi_entity_enable(gapiSubscriber);
                    }
                }
                gapi_free(dpqos);
            }
        } else if(listener != NULL){
            saj_listenerDataFree(env, saj_listenerData(listener->listener_data));
        }
    }
    if (gapiSubscriberQos != (gapi_subscriberQos *)GAPI_SUBSCRIBER_QOS_DEFAULT) {
    gapi_free(gapiSubscriberQos);
    }

    return javaSubscriber;
}

/*
 * Method:        jniDeleteSubscriber
 * Param :        Subscriber
 * Return:        return code
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDeleteSubscriber) (
    JNIEnv *env,
    jobject this,
    jobject jsubscriber)
{
    gapi_domainParticipant participant;
    gapi_subscriber subscriber;
    gapi_returnCode_t grc;
    c_bool must_free;

    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
    subscriber = (gapi_subscriber)saj_read_gapi_address(env, jsubscriber);

    must_free = saj_setThreadEnv(env);
    grc = gapi_domainParticipant_delete_subscriber(participant, subscriber);
    saj_delThreadEnv(must_free);

    return (jint)grc;
}

/*
 * Method:      jniGetBuiltinSubscriber
 * Return:      subscriber
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetBuiltinSubscriber) (
    JNIEnv *env,
    jobject this)
{
    jobject jsubscriber;
    gapi_subscriber subscriber;
    gapi_domainParticipant participant;
    saj_returnCode rc;


    jsubscriber = NULL;
    subscriber = GAPI_OBJECT_NIL;
    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);

    subscriber = gapi_domainParticipant_get_builtin_subscriber(participant);

    if (subscriber != NULL){
        jsubscriber = saj_read_java_address(subscriber);

        /**If java builtin subscriber does not exist, create it.*/
        if(jsubscriber == NULL){
            rc = saj_construct_java_object(env, PACKAGENAME "SubscriberImpl",
                                              (PA_ADDRCAST)subscriber,
                                              &jsubscriber);

            if(rc == SAJ_RETCODE_OK){
                rc = saj_domainParticipantInitBuiltinSubscriber(env, participant, subscriber);
            }
        }
    }
    return jsubscriber;
}

/*
 * Method:      jniCreateTopic
 * Param :      topic name
 * Param :      topic type
 * Param :      qos list
 * Param :      a listener
 * Return:      topic
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniCreateTopic) (
    JNIEnv  *env,
    jobject this,
    jstring jtopic_name,
    jstring jtype_name,
    jobject qos,
    jobject jlistener,
    jint jmask)
{
    gapi_topic gapiTopic;
    jobject javaTopic;
    const gapi_char* topicName;
    const gapi_char* typeName;
    gapi_domainParticipant participant;
    gapi_topicQos* tqos;
    struct gapi_topicListener* listener;
    saj_returnCode rc;
    jclass tempClass;
    jboolean result;

    gapiTopic = GAPI_OBJECT_NIL;
    javaTopic = NULL;
    topicName = NULL;
    typeName = NULL;

    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);

    /* process the jstring objects */
    if(jtopic_name != NULL){
        topicName = (*env)->GetStringUTFChars(env, jtopic_name, 0);
    }
    if(jtype_name != NULL){
        typeName = (*env)->GetStringUTFChars(env, jtype_name, 0);
    }
    if ((*env)->IsSameObject (env, qos, GET_CACHED(TOPIC_QOS_DEFAULT)) == JNI_FALSE) {
        tqos = gapi_topicQos__alloc();
        rc = saj_TopicQosCopyIn(env, qos, tqos);
    } else {
    tqos = (gapi_topicQos *)GAPI_TOPIC_QOS_DEFAULT;
    rc = SAJ_RETCODE_OK;
    }

    /* We can check Java instances in the jni layer, so here we check
     * that if the mask is set to GAPI_ALL_DATA_DISPOSED_STATUS we have
     * been given an ExtDomainParticipantListener to call. If not then
     * an error is reported. */
    if(jmask & GAPI_ALL_DATA_DISPOSED_STATUS) {
        tempClass = (*env)->FindClass(env, "DDS/ExtTopicListener");
        result = (*env)->IsInstanceOf(env, jlistener, tempClass);
        if(result == JNI_FALSE) {
            OS_REPORT(OS_ERROR, "dcpssaj", GAPI_ERRORCODE_INCONSISTENT_VALUE, "ExtTopicListener must be used when the ALL_DATA_DISPOSED_STATUS bit is set.");
          rc = SAJ_RETCODE_ERROR;
        }
    }

    if(rc == SAJ_RETCODE_OK){
        if(jmask & GAPI_ALL_DATA_DISPOSED_STATUS) {
            listener = saj_extTopicListenerNew(env, jlistener);
        } else {
            listener = saj_topicListenerNew(env, jlistener);
        }

        if(listener != NULL){
            saj_write_java_listener_address(env, gapiTopic, listener->listener_data);
        }
        gapiTopic = gapi_domainParticipant_create_topic(participant, topicName,
                                                        typeName, tqos,
                                                        listener, (gapi_statusMask)jmask);

        if (gapiTopic != GAPI_OBJECT_NIL){
            /* create a new java Topic object */
            rc = saj_construct_java_object(env, PACKAGENAME "TopicImpl",
                                                (PA_ADDRCAST)gapiTopic,
                                                &javaTopic);
        } else if(listener != NULL){
            saj_listenerDataFree(env, saj_listenerData(listener->listener_data));
        }

        if(jtopic_name != NULL){
            (*env)->ReleaseStringUTFChars(env, jtopic_name, topicName);
        }
        if(jtype_name != NULL){
            (*env)->ReleaseStringUTFChars(env, jtype_name, typeName);
        }
    }
    if (tqos != (gapi_topicQos *)GAPI_TOPIC_QOS_DEFAULT) {
        gapi_free(tqos);
    }

    return javaTopic;
}

/*
 * Method:    jniDeleteTopic
 * Param :    Topic
 * Return:    return code
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDeleteTopic) (
    JNIEnv  *env,
    jobject this,
    jobject jtopic)
{
    gapi_domainParticipant participant;
    gapi_topic topic;
    gapi_returnCode_t grc;
    c_bool must_free;

    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
    topic = (gapi_topic)saj_read_gapi_address(env, jtopic);

    must_free = saj_setThreadEnv(env);
    grc = gapi_domainParticipant_delete_topic(participant, topic);
    saj_delThreadEnv(must_free);

    return (jint)grc;
}

/*
 * Method: jniFindTopic
 * Param : topic name
 * Param : Duration
 * Return: Topic
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniFindTopic) (
    JNIEnv  *env,
    jobject this,
    jstring jtopic_name,
    jobject duration)
{
    jobject javaTopic;
    gapi_topic gapiTopic;
    gapi_duration_t timeout;
    const gapi_char* topicName;
    gapi_domainParticipant participant;

    javaTopic = NULL;
    gapiTopic = GAPI_OBJECT_NIL;
    topicName = NULL;
    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);

    /* process the jstring objects */
    if(jtopic_name != NULL){
        topicName = (*env)->GetStringUTFChars(env, jtopic_name, 0);
    }
    if(duration != NULL){
        saj_durationCopyIn(env, duration, &timeout);
        gapiTopic = gapi_domainParticipant_find_topic(participant,
                                            topicName,
                                            (const gapi_duration_t *)&timeout);
    } else {
        gapiTopic = gapi_domainParticipant_find_topic(participant,
                                            topicName, NULL);
    }

    /* check if the gapi has found a Topic */
    if (gapiTopic != GAPI_OBJECT_NIL){
        javaTopic = saj_read_java_address(gapiTopic);

        /**The Java Topic cannot exist already, because this is a copy of
         * the existing one if it already exists.
         */
        if(javaTopic == NULL){
            saj_construct_java_object(env, PACKAGENAME "TopicImpl",
                                            (PA_ADDRCAST)gapiTopic,
                                            &javaTopic);
        }
    }
    if(jtopic_name != NULL){
        (*env)->ReleaseStringUTFChars(env, jtopic_name, topicName);
    }
    return javaTopic;
}

/*
 * Method: jniLookupTopicdescription
 * Param : topic name
 * Return: TopicDescription;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniLookupTopicdescription) (
    JNIEnv  *env,
    jobject this,
    jstring jtopic_name)
{
    jobject javaTopicDescription;
    gapi_domainParticipant participant;
    gapi_topicDescription gapiTopicDescription;
    const gapi_char* topicName;

    javaTopicDescription = NULL;
    gapiTopicDescription = GAPI_OBJECT_NIL;
    topicName = NULL;

    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);

    if(jtopic_name != NULL){
        topicName = (*env)->GetStringUTFChars(env, jtopic_name, 0);
    }
    gapiTopicDescription = gapi_domainParticipant_lookup_topicdescription(
                                                    participant, topicName);
    if (gapiTopicDescription != GAPI_OBJECT_NIL){
        javaTopicDescription = saj_read_java_address(gapiTopicDescription);

        /* If Java TopicDescription does not already exist, it means it is an
         * internal topic and the user has not used the builtin subscriber
         * yet. This means the topic must be allocated now.
         */
        if(javaTopicDescription == NULL){
            /*
             * Create a Topic and NOT a TopicDescription on purpose!
             * This can only happen when this is a builtin Topic that has
             * not been used already.
             */
            saj_construct_java_object(env, PACKAGENAME
                                            "TopicImpl",
                                            (PA_ADDRCAST)gapiTopicDescription,
                                            &javaTopicDescription);
        }
    }
    if(jtopic_name != NULL){
        (*env)->ReleaseStringUTFChars(env, jtopic_name, topicName);
    }
    return javaTopicDescription;
}

/*
 * Method: jniCreateContentfilteredtopic
 * Param : content filter name
 * Param : related Topic
 * Param : filter expression
 * Param : expression parameters
 * Return: ContentFilteredTopic
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniCreateContentfilteredtopic) (
    JNIEnv  *env,
    jobject this,
    jstring jname,
    jobject related_topic,
    jstring jfilter_expression,
    jobjectArray jfilter_parameters)
{
    const gapi_char *filter_expression;
    const gapi_char *name;
    gapi_stringSeq *gapi_filter_parameters;
    gapi_contentFilteredTopic gapiContentFilterTopic;
    gapi_domainParticipant participant;
    gapi_topic topic;
    jobject javaContentFilterTopic;
    saj_returnCode rc;

    filter_expression = NULL;
    name = NULL;
    gapi_filter_parameters = NULL;
    gapiContentFilterTopic = GAPI_OBJECT_NIL;
    javaContentFilterTopic = NULL;
    rc = SAJ_RETCODE_OK;

    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
    topic = (gapi_topic)saj_read_gapi_address(env, related_topic);

    if(jname != NULL){
        name = (*env)->GetStringUTFChars(env, jname, 0);
    }
    if(jfilter_expression != NULL){
        filter_expression = (*env)->GetStringUTFChars(env, jfilter_expression, 0);
    }
    if((filter_expression != NULL) && (name != NULL)){
        gapi_filter_parameters = gapi_sequence_malloc();

        /* create a StringSeq from the java object */
        if(jfilter_parameters != NULL){
            rc = saj_stringSequenceCopyIn(env, jfilter_parameters,
                                                    gapi_filter_parameters);
        }
        if (rc == SAJ_RETCODE_OK){
            gapiContentFilterTopic =
                    gapi_domainParticipant_create_contentfilteredtopic(
                            participant, (const gapi_char *)name, topic,
                            (const gapi_char *)filter_expression,
                            (const gapi_stringSeq *)gapi_filter_parameters);
        }
        gapi_sequence_free(gapi_filter_parameters);
    }

    if (gapiContentFilterTopic != GAPI_OBJECT_NIL){
        rc = saj_construct_java_object(env,
                                PACKAGENAME "ContentFilteredTopicImpl",
                                (PA_ADDRCAST)gapiContentFilterTopic,
                                &javaContentFilterTopic);
    }
    if(jname != NULL){
        (*env)->ReleaseStringUTFChars(env, jname, name);
    }
    if(jfilter_expression != NULL){
        (*env)->ReleaseStringUTFChars(env, jfilter_expression, filter_expression);
    }
    return javaContentFilterTopic;
}

/*
 * Method: jniDeleteContentfilteredtopic
 * Param : ContentFilteredTopic
 * Return: Return code
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDeleteContentfilteredtopic) (
    JNIEnv  *env,
    jobject this,
    jobject contentFilterTopic)
{
    gapi_domainParticipant participant;
    gapi_contentFilteredTopic cft;
    gapi_returnCode_t grc;
    c_bool must_free;

    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
    cft = (gapi_contentFilteredTopic)saj_read_gapi_address(env, contentFilterTopic);

    must_free = saj_setThreadEnv(env);
    grc = gapi_domainParticipant_delete_contentfilteredtopic(participant,cft);
    saj_delThreadEnv(must_free);

    return (jint)grc;
}

/*
 * Method:  jniCreateMultitopic
 * Param :      name
 * Param :  type name
 * Param :  subscription expression
 * Param :  expression parameters
 * Return:      MultiTopic
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniCreateMultitopic) (
    JNIEnv *env,
    jobject this,
    jstring jname,
    jstring jtype_name,
    jstring jsubscription_expression,
    jobjectArray jexpression_parameters)
{
    const gapi_char *name;
    const gapi_char *type_name;
    const gapi_char *subscription_expression;
    gapi_stringSeq *expression_parameters;
    gapi_multiTopic gapiMultiTopic;
    jobject javaMultiTopic;
    saj_returnCode rc;
    gapi_domainParticipant participant;

    name = NULL;
    type_name = NULL;
    subscription_expression = NULL;
    expression_parameters = NULL;
    gapiMultiTopic = GAPI_OBJECT_NIL;
    javaMultiTopic = NULL;
    rc = SAJ_RETCODE_OK;

    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);

    /* convert jstrings to c strings */
    if(jname != NULL){
        name = (*env)->GetStringUTFChars(env, jname, 0);
    }
    if(jtype_name != NULL){
        type_name = (*env)->GetStringUTFChars(env, jtype_name, 0);
    }
    if(jsubscription_expression != NULL){
        subscription_expression = (*env)->GetStringUTFChars(env,
                                                jsubscription_expression, 0);
    }
    if ( (name != NULL) && (type_name != NULL) && (subscription_expression != NULL)){
        expression_parameters = gapi_stringSeq__alloc();

        if(jexpression_parameters != NULL){
            rc = saj_stringSequenceCopyIn(env, jexpression_parameters, expression_parameters);
        }
        if(rc == SAJ_RETCODE_OK){
            gapiMultiTopic = gapi_domainParticipant_create_multitopic(
                                participant,
                                (const gapi_char *)name,
                                (const gapi_char *)type_name,
                                (const gapi_char *)subscription_expression,
                                (const gapi_stringSeq *)expression_parameters);
        }
        gapi_sequence_free(expression_parameters);
    }
    if (gapiMultiTopic != GAPI_OBJECT_NIL){
        rc = saj_construct_java_object(env, PACKAGENAME "MultiTopicImpl",
                                            (PA_ADDRCAST)gapiMultiTopic,
                                            &javaMultiTopic);
    }
    /* release the jstrings and the c strings*/
    if(jname != NULL){
        (*env)->ReleaseStringUTFChars(env, jname, name);
    }
    if(jtype_name != NULL){
        (*env)->ReleaseStringUTFChars(env, jtype_name, type_name);
    }
    if(jsubscription_expression != NULL){
        (*env)->ReleaseStringUTFChars(env, jsubscription_expression,
                                            subscription_expression);
    }
    return javaMultiTopic;
}

/*
 * Method: jniDeleteMultitopic
 * Param : MultiTopic
 * Return: Return code
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDeleteMultitopic) (
    JNIEnv        *env,
    jobject       this,
    jobject       jMultiTopic)
{
    gapi_domainParticipant participant;
    gapi_multiTopic multiTopic;
    gapi_returnCode_t grc;
    c_bool must_free;

    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
    multiTopic = (gapi_multiTopic)saj_read_gapi_address(env, jMultiTopic);

    must_free = saj_setThreadEnv(env);
    grc = gapi_domainParticipant_delete_multitopic(participant, multiTopic);
    saj_delThreadEnv(must_free);

    return (jint)grc;
}

/*
 * Method:  jniDeleteContainedEntities
 * Return:      Return code
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDeleteContainedEntities) (
    JNIEnv        *env,
    jobject       this)
{
    gapi_domainParticipant participant;
    jint result;
    c_bool must_free;

    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);

    must_free = saj_setThreadEnv(env);
    result = (jint)gapi_domainParticipant_delete_contained_entities(participant);
    saj_delThreadEnv(must_free);

    return result;
}

/*
 * Method: jniSetQos
 * Param : DomainParticipantQos
 * Return: Return code
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetQos) (
    JNIEnv *env,
    jobject this,
    jobject jDomainParticipantQos)
{
    gapi_domainParticipantQos* gapiQos;
    gapi_domainParticipant participant;
    saj_returnCode rc;
    gapi_returnCode_t result;

    gapiQos = gapi_domainParticipantQos__alloc();
    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
    rc = saj_DomainParticipantQosCopyIn(env, jDomainParticipantQos, gapiQos);
    result = GAPI_RETCODE_ERROR;

    if(rc == SAJ_RETCODE_OK){
        result = gapi_domainParticipant_set_qos(participant, gapiQos);
    }
    gapi_free(gapiQos);

    return (jint)result;
}

/*
 * Method: jniGetQos
 * Param : DomainParticipantQosHolder
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetQos) (
    JNIEnv        *env,
    jobject       this,
    jobject       jDomainParticipantQosHolder)
{
    gapi_domainParticipantQos* gapiQos;
    saj_returnCode rc;
    jobject javaQos;
    gapi_domainParticipant participant;
    gapi_returnCode_t result;

    if(jDomainParticipantQosHolder != NULL){
        participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
        javaQos = NULL;
        gapiQos = gapi_domainParticipantQos__alloc();
        result = gapi_domainParticipant_get_qos(participant, gapiQos);

        if(result == GAPI_RETCODE_OK){
            rc = saj_DomainParticipantQosCopyOut(env, gapiQos, &javaQos);
            gapi_free(gapiQos);

            if(rc == SAJ_RETCODE_OK){
                /* store the DomainParticipantQos object in the Holder object */
                (*env)->SetObjectField(env, jDomainParticipantQosHolder,
                        GET_CACHED(domainParticipantQosHolder_value_fid), javaQos);

                /* delete the local reference to the DomainParticipantQos object */
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

/*
 * Method: jniSetListener
 * Param : DomainParticipantListener
 * Param : Listener
 * Param : StatusKind mask
 * Return: Return code
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetListener) (
    JNIEnv        *env,
    jobject       this,
    jobject       jlistener,
    jint          mask)
{
    struct gapi_domainParticipantListener *listener;
    gapi_domainParticipant participant;
    gapi_returnCode_t grc = GAPI_RETCODE_OK;

    jclass tempClass;
    jboolean result;

    /* We can check Java instances in the jni layer, so here we check
     * that if the mask is set to GAPI_ALL_DATA_DISPOSED_STATUS we have
     * been given an ExtDomainParticipantListener to call. If not then
     * an error is reported and a GAPI_RETCODE_BAD_PARAMETER status is
     * returned. */
    if(mask & GAPI_ALL_DATA_DISPOSED_STATUS) {
        tempClass = (*env)->FindClass(env, "DDS/ExtDomainParticipantListener");
        result = (*env)->IsInstanceOf(env, jlistener, tempClass);
        if(result == JNI_FALSE) {
            OS_REPORT(OS_ERROR, "dcpssaj", 0, "ExtDomainParticipantListener must be used when the ALL_DATA_DISPOSED_STATUS bit is set.");
            grc = GAPI_RETCODE_BAD_PARAMETER;
        }
    }

    if(grc == GAPI_RETCODE_OK){
        participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);

        if(mask & GAPI_ALL_DATA_DISPOSED_STATUS) {
        listener = saj_extDomainParticipantListenerNew(env, jlistener);
        } else {
            listener = saj_domainParticipantListenerNew(env, jlistener);
    }

        if(listener != NULL){
            saj_write_java_listener_address(env, participant, listener->listener_data);
        }
        grc = gapi_domainParticipant_set_listener(participant, listener,
                                                    (unsigned long int)mask);

        if((grc != GAPI_RETCODE_OK) && (listener != NULL)){
            saj_listenerDataFree(env, saj_listenerData(listener->listener_data));
        }
    }
    return (jint)grc;
}

/*
 * Method: jniGetListener
 * Return: DomainParticipantListener
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetListener) (
    JNIEnv *env,
    jobject this)
{
    jobject jlistener;
    struct gapi_domainParticipantListener gapiListener;
    gapi_domainParticipant participant;

    jlistener = NULL;
    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
    gapiListener = gapi_domainParticipant_get_listener(participant);

    jlistener = saj_read_java_listener_address(gapiListener.listener_data);

    return jlistener;
}

/*
 * Method: jniIgnoreParticipant
 * Param : InstanceHandle
 * Return: Return code
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniIgnoreParticipant) (
    JNIEnv *env,
    jobject this,
    jlong handle)
{
    gapi_domainParticipant participant;
    gapi_returnCode_t grc;

    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
    grc = gapi_domainParticipant_ignore_participant(participant,
                                        (const gapi_instanceHandle_t)handle);
    return (jint)grc;
}

/*
 * Method: jniIgnoreTopic
 * Param : InstanceHandle
 * Return: Return code
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniIgnoreTopic) (
    JNIEnv *env,
    jobject this,
    jlong handle)
{
    gapi_domainParticipant participant;
    gapi_returnCode_t grc;

    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
    grc = gapi_domainParticipant_ignore_topic(participant,
                                        (const gapi_instanceHandle_t)handle);
    return (jint)grc;
}

/*
 * Method:    jniIgnorePublication
 * Param : InstanceHandle
 * Return: Return code
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniIgnorePublication) (
    JNIEnv *env,
    jobject this,
    jlong handle)
{
    gapi_domainParticipant participant;
    gapi_returnCode_t grc;

    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
    grc = gapi_domainParticipant_ignore_publication(participant,
                                        (const gapi_instanceHandle_t)handle);
    return (jint)grc;
}

/*
 * Method: jniIgnoreSubscription
 * Param : InstanceHandle
 * Return: Return code
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniIgnoreSubscription) (
    JNIEnv *env,
    jobject this,
    jlong handle)
{
    gapi_domainParticipant participant;
    gapi_returnCode_t grc;

    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
    grc = gapi_domainParticipant_ignore_subscription (participant,
                                        (const gapi_instanceHandle_t)handle);
    return (jint)grc;
}

/*
 * Method: jniGetDomainId
 * Return: DomainId_t
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetDomainId) (
    JNIEnv *env,
    jobject this)
{
    gapi_domainParticipant participant;
    jint result;

    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
    result = (jint)gapi_domainParticipant_get_domain_id(participant);

    return result;
}

/*
 * Method:    jniAssertLiveliness
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniAssertLiveliness) (
    JNIEnv *env,
    jobject this)
{
    gapi_domainParticipant participant;
    jint result;

    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
    result = (jint)gapi_domainParticipant_assert_liveliness(participant);
    return result;
}

/*
 * Class:     org_opensplice_gapi_dcps_DomainParticipantImpl
 * Method:    jniSetDefaultPublisherQos
 * Signature: (LDDS/PublisherQos;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetDefaultPublisherQos) (
    JNIEnv *env,
    jobject this,
    jobject jqos)
{
    gapi_publisherQos* publisherQos;
    gapi_domainParticipant participant;
    saj_returnCode rc;
    jint result;

    result = (jint)GAPI_RETCODE_ERROR;
    publisherQos = gapi_publisherQos__alloc();
    rc = saj_PublisherQosCopyIn(env, jqos, publisherQos);

    if (rc == SAJ_RETCODE_OK){
        participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
        result = (jint)gapi_domainParticipant_set_default_publisher_qos(
                                                participant, publisherQos);
    }
    gapi_free(publisherQos);

    return result;
}


/*
 * Class:     org_opensplice_gapi_dcps_DomainParticipantImpl
 * Method:    jniGetDefaultPublisherQos
 * Signature: (LDDS/PublisherQosHolder;)V
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetDefaultPublisherQos) (
    JNIEnv *env,
    jobject this,
    jobject jQosHolder)
{
    saj_returnCode rc;
    gapi_returnCode_t result;
    jobject javaQos;
    gapi_domainParticipant participant;
    gapi_publisherQos *gapiQos;

    javaQos = NULL;


    if(jQosHolder != NULL){
        gapiQos = gapi_publisherQos__alloc();
        participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
        result = gapi_domainParticipant_get_default_publisher_qos(participant, gapiQos);

        if(result == GAPI_RETCODE_OK){
            rc = saj_PublisherQosCopyOut(env, gapiQos, &javaQos);
            gapi_free(gapiQos);

            if (rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jQosHolder,
                                       GET_CACHED(publisherQosHolder_value_fid),
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

/*
 * Class:     org_opensplice_gapi_dcps_DomainParticipantImpl
 * Method:    jniSetDefaultSubscriberQos
 * Signature: (LDDS/SubscriberQos;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetDefaultSubscriberQos) (
    JNIEnv *env,
    jobject this,
    jobject jqos)
{
    gapi_subscriberQos* subscriberQos;
    gapi_domainParticipant participant;
    saj_returnCode rc;
    jint result;

    result = (jint)GAPI_RETCODE_ERROR;
    subscriberQos = gapi_subscriberQos__alloc();
    rc = saj_SubscriberQosCopyIn(env, jqos, subscriberQos);

    if (rc == SAJ_RETCODE_OK){
        participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
        result = (jint)gapi_domainParticipant_set_default_subscriber_qos(
                                                participant, subscriberQos);
    }
    gapi_free(subscriberQos);

    return result;
}

/*
 * Class:     org_opensplice_gapi_dcps_DomainParticipantImpl
 * Method:    jniGetDefaultSubscriberQos
 * Signature: (LDDS/SubscriberQosHolder;)V
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetDefaultSubscriberQos) (
    JNIEnv *env,
    jobject this,
    jobject jqosHolder)
{
    saj_returnCode rc;
    gapi_returnCode_t result;
    jobject javaQos;
    gapi_domainParticipant participant;
    gapi_subscriberQos *gapiQos;

    javaQos = NULL;
    rc = SAJ_RETCODE_ERROR;

    if(jqosHolder != NULL){
        gapiQos = gapi_subscriberQos__alloc();

        participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
        result = gapi_domainParticipant_get_default_subscriber_qos(participant, gapiQos);

        if(result == GAPI_RETCODE_OK){
            rc = saj_SubscriberQosCopyOut(env, gapiQos, &javaQos);
            gapi_free(gapiQos);

            if (rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jqosHolder,
                                       GET_CACHED(subscriberQosHolder_value_fid),
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

/*
 * Class:     org_opensplice_gapi_dcps_DomainParticipantImpl
 * Method:    jniSetDefaultTopicQos
 * Signature: (LDDS/TopicQos;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetDefaultTopicQos) (
    JNIEnv *env,
    jobject this,
    jobject jqos)
{
    gapi_topicQos* topicQos;
    gapi_domainParticipant participant;
    saj_returnCode rc;
    jint result;

    result = (jint)GAPI_RETCODE_ERROR;
    topicQos = gapi_topicQos__alloc();
    rc = saj_TopicQosCopyIn(env, jqos, topicQos);

    if (rc == SAJ_RETCODE_OK){
        participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
        result = (jint)gapi_domainParticipant_set_default_topic_qos(
                                                participant, topicQos);
    }
    gapi_free(topicQos);

    return result;
}

/*
 * Class:     org_opensplice_gapi_dcps_DomainParticipantImpl
 * Method:    jniGetDefaultTopicQos
 * Signature: (LDDS/TopicQosHolder;)V
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetDefaultTopicQos) (
    JNIEnv *env,
    jobject this,
    jobject jqosHolder)
{
    saj_returnCode rc;
    gapi_returnCode_t result;
    jobject javaQos;
    gapi_domainParticipant participant;
    gapi_topicQos *gapiQos;

    javaQos = NULL;

    if(jqosHolder != NULL){
        gapiQos = gapi_topicQos__alloc();

        participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
        result = gapi_domainParticipant_get_default_topic_qos(participant, gapiQos);

        if(result == GAPI_RETCODE_OK){
            rc = saj_TopicQosCopyOut(env, gapiQos, &javaQos);
            gapi_free(gapiQos);

            if (rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jqosHolder,
                                       GET_CACHED(topicQosHolder_value_fid), javaQos);
            } else {
                result = GAPI_RETCODE_ERROR;
            }
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return (jint)result;
}


struct copyInstanceHandle {
    JNIEnv *env;
    c_ulong index;
    jlongArray seq;
};

static c_bool
copyInstanceHandle(
    v_dataReaderInstance instance,
    c_voidp arg)
{
    c_bool result = TRUE;
    struct copyInstanceHandle *a = (struct copyInstanceHandle *)arg;
    JNIEnv *env = a->env;
    jlong ghandle;
    jint length;

    if (a->index == 0) {
        length = (jint) v_dataReaderInstanceGetNotEmptyInstanceCount(instance);

        /*buffer alloc*/
        if (a->seq == NULL || length != (*env)->GetArrayLength(env, a->seq)) {
            a->seq = (*env)->NewLongArray(env, length);
        } /* else ok */
    }

    ghandle = (jlong) u_instanceHandleNew((v_public)instance);
    (*env)->SetLongArrayRegion(env, a->seq, (jint) a->index, 1, &ghandle);
    a->index++;

    return result;
}
/*
 * Class:     org_opensplice_dds_dcps_DomainParticipantImpl
 * Method:    jniGetDiscoveredParticipants
 * Signature: (LDDS/InstanceHandleSeqHolder;)I
 */
JNIEXPORT jint JNICALL Java_org_opensplice_dds_dcps_DomainParticipantImpl_jniGetDiscoveredParticipants
  (JNIEnv *env, jobject this, jobject jseqHolder)
{
    gapi_domainParticipant participant;
    jint result;
    jlongArray jarray;

    struct copyInstanceHandle cih;

    result = (jint)GAPI_RETCODE_ERROR;

    if(jseqHolder != NULL){
        participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);

        jarray = (jlongArray) (*env)->GetObjectField(env, jseqHolder,
                                GET_CACHED(instanceHandleSeqHolder_value_fid));
        cih.env = env;
        cih.index = 0;
        cih.seq = jarray;

        result = (jint)gapi_domainParticipant_get_discovered_participants(
                                                participant, copyInstanceHandle, &cih);
        if(result == GAPI_RETCODE_OK) {

            if(jarray != cih.seq){
                (*env)->SetObjectField(env, jseqHolder,
                        GET_CACHED(instanceHandleSeqHolder_value_fid), (jobject) cih.seq);
                (*env)->DeleteLocalRef(env, jarray);
            }
        }

    } else {
        result = (jint)GAPI_RETCODE_BAD_PARAMETER;
    }
    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_DomainParticipantImpl
 * Method:    jniGetDiscoveredParticipantData
 * Signature: (JLDDS/ParticipantBuiltinTopicDataHolder;)I
 */
JNIEXPORT jint JNICALL Java_org_opensplice_dds_dcps_DomainParticipantImpl_jniGetDiscoveredParticipantData
  (JNIEnv *env, jobject this, jobject jdataHolder, jlong jhandle, jlong copyCache)
{
    gapi_domainParticipant participant;
    gapi_instanceHandle_t handle = (gapi_instanceHandle_t)jhandle;
    jint result;
    jobject jParticipantData;
    C_STRUCT(saj_dstInfo) dstInfo;
    sajReaderCopyCache *rcc = saj_copyCacheReaderCache((saj_copyCache)(PA_ADDRCAST)copyCache);

    if(jdataHolder != NULL){
        participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
        jParticipantData = (*env)->GetObjectField (env, jdataHolder, rcc->dataHolder_value_fid);
        dstInfo.javaEnv = env;
        dstInfo.javaObject = jParticipantData;
        dstInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        result = (jint)gapi_domainParticipant_get_discovered_participant_data(
                participant,
                &dstInfo,
                handle,
                saj_copyOutStruct);
        if(result == GAPI_RETCODE_OK) {
            if (dstInfo.javaObject != NULL) {
                if (dstInfo.javaObject != jParticipantData) {
                    (*env)->SetObjectField (env, jdataHolder, rcc->dataHolder_value_fid, dstInfo.javaObject);
                } /* else ok */
            } else {
                result = (jint)GAPI_RETCODE_ERROR;
            }
        }
    } else {
        result = (jint)GAPI_RETCODE_BAD_PARAMETER;
    }

    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_DomainParticipantImpl
 * Method:    jniGetDiscoveredTopics
 * Signature: (LDDS/InstanceHandleSeqHolder;)I
 */
JNIEXPORT jint JNICALL Java_org_opensplice_dds_dcps_DomainParticipantImpl_jniGetDiscoveredTopics
  (JNIEnv *env, jobject this, jobject jseqHolder)
{
    gapi_domainParticipant participant;
    jint result;
    jlongArray jarray;

    struct copyInstanceHandle cih;

    result = (jint)GAPI_RETCODE_ERROR;

    if(jseqHolder != NULL){
        participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);

        jarray = (jlongArray) (*env)->GetObjectField(env, jseqHolder,
                                GET_CACHED(instanceHandleSeqHolder_value_fid));
        cih.env = env;
        cih.index = 0;
        cih.seq = jarray;

        result = (jint)gapi_domainParticipant_get_discovered_topics(
                                                participant, copyInstanceHandle, &cih);
        if(result == GAPI_RETCODE_OK) {

            if(jarray != cih.seq){
                (*env)->SetObjectField(env, jseqHolder,
                        GET_CACHED(instanceHandleSeqHolder_value_fid), (jobject) cih.seq);
                (*env)->DeleteLocalRef(env, jarray);
            }
        }

    } else {
        result = (jint)GAPI_RETCODE_BAD_PARAMETER;
    }
    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_DomainParticipantImpl
 * Method:    jniGetDiscoveredTopicData
 * Signature: (JLDDS/TopicBuiltinTopicDataHolder;)I
 */
JNIEXPORT jint JNICALL Java_org_opensplice_dds_dcps_DomainParticipantImpl_jniGetDiscoveredTopicData
(JNIEnv *env, jobject this, jobject jdataHolder, jlong jhandle, jlong copyCache)
{
  gapi_domainParticipant participant;
  gapi_instanceHandle_t handle = (gapi_instanceHandle_t)jhandle;
  jint result;
  jobject jTopicData;
  C_STRUCT(saj_dstInfo) dstInfo;
  sajReaderCopyCache *rcc = saj_copyCacheReaderCache((saj_copyCache)(PA_ADDRCAST)copyCache);

  if(jdataHolder != NULL){
      participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);
      jTopicData = (*env)->GetObjectField (env, jdataHolder, rcc->dataHolder_value_fid);
      dstInfo.javaEnv = env;
      dstInfo.javaObject = jTopicData;
      dstInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
      result = (jint)gapi_domainParticipant_get_discovered_topic_data(
              participant,
              &dstInfo,
              handle,
              saj_copyOutStruct);
      if(result == GAPI_RETCODE_OK) {
          if (dstInfo.javaObject != NULL) {
              if (dstInfo.javaObject != jTopicData) {
                  (*env)->SetObjectField (env, jdataHolder, rcc->dataHolder_value_fid, dstInfo.javaObject);
              } /* else ok */
          } else {
              result = (jint)GAPI_RETCODE_ERROR;
          }
      }
  } else {
      result = (jint)GAPI_RETCODE_BAD_PARAMETER;
  }

  return result;
}

/*
 * Class:     org_opensplice_dds_dcps_DomainParticipantImpl
 * Method:    jniContainsEntity
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_opensplice_dds_dcps_DomainParticipantImpl_jniContainsEntity
  (JNIEnv *env, jobject this, jlong jhandle)
{
    gapi_domainParticipant participant;
    gapi_instanceHandle_t handle = (gapi_instanceHandle_t)jhandle;
    jboolean result;

    participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);

    result = (jboolean)gapi_domainParticipant_contains_entity(participant, handle);

    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_DomainParticipantImpl
 * Method:    jniGetCurrentTime
 * Signature: (LDDS/Time_tHolder;)I
 */
JNIEXPORT jint JNICALL Java_org_opensplice_dds_dcps_DomainParticipantImpl_jniGetCurrentTime
  (JNIEnv *env, jobject this, jobject jtimeHolder)
{
    saj_returnCode rc;
    gapi_domainParticipant participant;
    gapi_time_t current_time;
    jint result;
    jobject timestamp;

    result = (jint)GAPI_RETCODE_ERROR;
    rc = SAJ_RETCODE_ERROR;

    if(jtimeHolder != NULL){
        participant = (gapi_domainParticipant)saj_read_gapi_address(env, this);

        result = (jint)gapi_domainParticipant_get_current_time(
                                                participant, &current_time);
        if(result == GAPI_RETCODE_OK) {
            timestamp = (*env)->GetObjectField (env, jtimeHolder, GET_CACHED(time_tHolder_value_fid));
            saj_exceptionCheck(env);

            rc = saj_timeCopyOut(env, &current_time, &timestamp);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jtimeHolder, GET_CACHED(time_tHolder_value_fid), timestamp);
            } else {
                result = (jint)GAPI_RETCODE_ERROR;
            }
        }
    } else {
        result = (jint)GAPI_RETCODE_BAD_PARAMETER;
    }
    return result;

}





static saj_returnCode
saj_domainParticipantInitBuiltinSubscriber(
    JNIEnv *env,
    gapi_domainParticipant participant,
    gapi_subscriber subscriber)
{
    saj_returnCode rc;

    rc = saj_domainParticipantInitBuiltinDataReader(
                            env, participant, subscriber,
                            "DCPSParticipant", "DDS::ParticipantBuiltinTopicData",
                            "DDS/ParticipantBuiltinTopicDataDataReaderImpl",
                            "(LDDS/ParticipantBuiltinTopicDataTypeSupport;)V");

    if(rc == SAJ_RETCODE_OK){
        rc = saj_domainParticipantInitBuiltinDataReader(
                            env, participant, subscriber,
                            "DCPSTopic", "DDS::TopicBuiltinTopicData",
                            "DDS/TopicBuiltinTopicDataDataReaderImpl",
                            "(LDDS/TopicBuiltinTopicDataTypeSupport;)V");

        if(rc == SAJ_RETCODE_OK){
            rc = saj_domainParticipantInitBuiltinDataReader(
                            env, participant, subscriber,
                            "DCPSPublication", "DDS::PublicationBuiltinTopicData",
                            "DDS/PublicationBuiltinTopicDataDataReaderImpl",
                            "(LDDS/PublicationBuiltinTopicDataTypeSupport;)V");

            if(rc == SAJ_RETCODE_OK){
                rc = saj_domainParticipantInitBuiltinDataReader(
                            env, participant, subscriber,
                            "DCPSSubscription", "DDS::SubscriptionBuiltinTopicData",
                            "DDS/SubscriptionBuiltinTopicDataDataReaderImpl",
                            "(LDDS/SubscriptionBuiltinTopicDataTypeSupport;)V");
            }
        }
    }
    return rc;
}

static saj_returnCode
saj_domainParticipantInitBuiltinDataReader(
    JNIEnv *env,
    gapi_domainParticipant participant,
    gapi_subscriber subscriber,
    const gapi_char* topicName,
    const gapi_char* typeName,
    const gapi_char* dataReaderClassName,
    const gapi_char* dataReaderConstructorSignature)
{
    gapi_dataReader dataReader;
    gapi_typeSupport typeSupport;
    gapi_topicDescription description;
    jobject jtypeSupport;
    jobject jdataReader;
    jobject jtopic;
    saj_returnCode rc;

    typeSupport = gapi_domainParticipant_get_typesupport(participant, typeName);
    jtypeSupport = saj_read_java_address(typeSupport);
    dataReader = gapi_subscriber_lookup_datareader(
                    subscriber, topicName);

    if((dataReader != NULL) && (jtypeSupport != NULL)){
        rc = saj_construct_typed_java_object(
                        env, dataReaderClassName,
                        (PA_ADDRCAST)dataReader, &jdataReader,
                        dataReaderConstructorSignature,
                        jtypeSupport);

        if(rc == SAJ_RETCODE_OK){
            description = gapi_dataReader_get_topicdescription(dataReader);
            jtopic = saj_read_java_address(description);

            if(description != NULL){
                if(jtopic == NULL){
                    /*Builtin topic not allocated yet in Java, so do it now.*/
                    rc = saj_construct_java_object(env,  PACKAGENAME "TopicImpl",
                                                    (PA_ADDRCAST)description,
                                                    &jtopic);
                }
            }
        }
    } else {
        rc = SAJ_RETCODE_ERROR;
    }
    return rc;
}

#undef SAJ_FUNCTION
