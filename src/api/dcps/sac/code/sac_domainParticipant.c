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

#include "gapi.h"

#include "dds_dcps.h"
#include "sac_structured.h"
#include "v_public.h"
#include "v_dataReaderInstance.h"
#include "u_instanceHandle.h"

/*     Publisher
 *     create_publisher(
 *         in PublisherQos qos,
 *         in PublisherListener a_listener);
 */
DDS_Publisher
DDS_DomainParticipant_create_publisher (
    DDS_DomainParticipant this,
    const DDS_PublisherQos *qos,
    const struct DDS_PublisherListener *a_listener,
    const DDS_StatusMask mask
    )
{
    DDS_Publisher publisher;
    struct gapi_publisherListener gListener;
    struct gapi_publisherListener *pListener = NULL;

    if ( a_listener ) {
        sac_copySacPublisherListener(a_listener, &gListener);
        pListener = &gListener;
    }

     publisher = gapi_domainParticipant_create_publisher (
                            (gapi_domainParticipant)this,
                            (const gapi_publisherQos *)qos,
                            (const struct gapi_publisherListener *)pListener,
                            (gapi_statusMask) mask
                        );

     if(publisher){
         gapi_domainParticipantQos *dpqos = gapi_domainParticipantQos__alloc();
         if(dpqos){
             if(gapi_domainParticipant_get_qos(this, dpqos) == GAPI_RETCODE_OK){
                 if(dpqos->entity_factory.autoenable_created_entities) {
                     gapi_entity_enable(publisher);
                 }
             }
             gapi_free(dpqos);
         }
     }

     return publisher;
}

/*     ReturnCode_t
 *     delete_publisher(
 *         in Publisher p);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_delete_publisher (
    DDS_DomainParticipant this,
    const DDS_Publisher p
    )
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_delete_publisher (
            (gapi_domainParticipant)this,
            (gapi_publisher)p
        );
}

/*     Subscriber
 *     create_subscriber(
 *         in SubscriberQos qos,
 *         in SubscriberListener a_listener);
 */
DDS_Subscriber
DDS_DomainParticipant_create_subscriber (
    DDS_DomainParticipant this,
    const DDS_SubscriberQos *qos,
    const struct DDS_SubscriberListener *a_listener,
    const DDS_StatusMask mask
    )
{
    DDS_Subscriber subscriber;
    struct gapi_subscriberListener gListener;
    struct gapi_subscriberListener *pListener = NULL;

    if ( a_listener ) {
        sac_copySacSubscriberListener(a_listener, &gListener);
        pListener = &gListener;
    }

    subscriber = gapi_domainParticipant_create_subscriber (
                            (gapi_domainParticipant)this,
                            (const gapi_subscriberQos *)qos,
                            (const struct gapi_subscriberListener *)pListener,
                            (gapi_statusMask) mask
                        );

    if(subscriber){
        gapi_domainParticipantQos *dpqos = gapi_domainParticipantQos__alloc();
        if(dpqos){
            if(gapi_domainParticipant_get_qos(this, dpqos) == GAPI_RETCODE_OK){
                if(dpqos->entity_factory.autoenable_created_entities) {
                    gapi_entity_enable(subscriber);
                }
            }
            gapi_free(dpqos);
        }
    }

    return subscriber;
}

/*     ReturnCode_t
 *     delete_subscriber(
 *         in Subscriber s);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_delete_subscriber (
    DDS_DomainParticipant this,
    const DDS_Subscriber s
    )
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_delete_subscriber (
            (gapi_domainParticipant)this,
            (gapi_subscriber)s
        );
}

/*     Subscriber
 *     get_builtin_subscriber();
 */
DDS_Subscriber
DDS_DomainParticipant_get_builtin_subscriber (
    DDS_DomainParticipant this
    )
{
    return (DDS_Subscriber)
        gapi_domainParticipant_get_builtin_subscriber (
            (gapi_domainParticipant)this
        );
}

/*     Topic
 *     create_topic(
 *         in string topic_name,
 *         in string type_name,
 *         in TopicQos qos,
 */
DDS_Topic
DDS_DomainParticipant_create_topic (
    DDS_DomainParticipant this,
    const DDS_char *topic_name,
    const DDS_char *type_name,
    const DDS_TopicQos *qos,
    const struct DDS_TopicListener *a_listener,
    const DDS_StatusMask mask
    )
{
    struct gapi_topicListener gListener;
    struct gapi_topicListener *pListener = NULL;

    if ( a_listener ) {
        sac_copySacTopicListener(a_listener, &gListener);
        pListener = &gListener;
    }

    return (DDS_Topic)
        gapi_domainParticipant_create_topic (
            (gapi_domainParticipant)this,
            (const gapi_char *)topic_name,
            (const gapi_char *)type_name,
            (const gapi_topicQos *)qos,
            (const struct gapi_topicListener *)pListener,
            (gapi_statusMask) mask
        );
}

/*     ReturnCode_t
 *     delete_topic(
 *         in Topic a_topic);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_delete_topic (
    DDS_DomainParticipant this,
    const DDS_Topic a_topic
    )
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_delete_topic (
            (gapi_domainParticipant)this,
            (gapi_topic)a_topic
        );
}

/*     Topic
 *     find_topic(
 *         in string topic_name,
 *         in Duration_t timeout);
 */
DDS_Topic
DDS_DomainParticipant_find_topic (
    DDS_DomainParticipant this,
    const DDS_char *topic_name,
    const DDS_Duration_t *timeout
    )
{
    return (DDS_Topic)
        gapi_domainParticipant_find_topic (
            (gapi_domainParticipant)this,
            (const gapi_char *)topic_name,
            (const gapi_duration_t *)timeout
        );
}

/*     TopicDescription
 *     lookup_topicdescription(
 *         in string name);
 */
DDS_TopicDescription
DDS_DomainParticipant_lookup_topicdescription (
    DDS_DomainParticipant this,
    const DDS_char *name
    )
{
    return (DDS_TopicDescription)
        gapi_domainParticipant_lookup_topicdescription (
            (gapi_domainParticipant)this,
            (const gapi_char *)name
        );
}

/*     ContentFilteredTopic
 *     create_contentfilteredtopic(
 *         in string name,
 *         in Topic related_topic,
 *         in string filter_expression,
 *         in StringSeq filter_parameters);
 */
DDS_ContentFilteredTopic
DDS_DomainParticipant_create_contentfilteredtopic (
    DDS_DomainParticipant this,
    const DDS_char *name,
    const DDS_Topic related_topic,
    const DDS_char *filter_expression,
    const DDS_StringSeq *filter_parameters
    )
{
    return (DDS_ContentFilteredTopic)
        gapi_domainParticipant_create_contentfilteredtopic (
            (gapi_domainParticipant)this,
            (const gapi_char *)name,
            (gapi_topic)related_topic,
            (const gapi_char *)filter_expression,
            (const gapi_stringSeq *)filter_parameters
        );
}

/*     ReturnCode_t
 *     delete_contentfilteredtopic(
 *         in ContentFilteredTopic a_contentfilteredtopic);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_delete_contentfilteredtopic (
    DDS_DomainParticipant this,
    const DDS_ContentFilteredTopic a_contentfilteredtopic
    )
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_delete_contentfilteredtopic (
            (gapi_domainParticipant)this,
            (gapi_contentFilteredTopic)a_contentfilteredtopic
        );
}

/*     MultiTopic
 *     create_multitopic(
 *         in string name,
 *         in string type_name,
 *         in string subscription_expression,
 *         in StringSeq expression_parameters);
 */
DDS_MultiTopic
DDS_DomainParticipant_create_multitopic (
    DDS_DomainParticipant this,
    const DDS_char *name,
    const DDS_char *type_name,
    const DDS_char *subscription_expression,
    const DDS_StringSeq *expression_parameters
    )
{
    return (DDS_MultiTopic)
        gapi_domainParticipant_create_multitopic (
            (gapi_domainParticipant)this,
            (const gapi_char *)name,
            (const gapi_char *)type_name,
            (const gapi_char *)subscription_expression,
            (const gapi_stringSeq *)expression_parameters
        );
}

/*     ReturnCode_t
 *     delete_multitopic(
 *         in MultiTopic a_multitopic);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_delete_multitopic (
    DDS_DomainParticipant this,
    const DDS_MultiTopic a_multitopic
    )
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_delete_multitopic (
            (gapi_domainParticipant)this,
            (gapi_multiTopic)a_multitopic
        );
}

/*     ReturnCode_t
 *     delete_contained_entities();
 */
DDS_ReturnCode_t
DDS_DomainParticipant_delete_contained_entities (
    DDS_DomainParticipant this
    )
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_delete_contained_entities (
            (gapi_domainParticipant)this);
}

/*     ReturnCode_t
 *     set_qos(
 *         in DomainParticipantQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_set_qos (
    DDS_DomainParticipant this,
    const DDS_DomainParticipantQos *qos
    )
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_set_qos (
            (gapi_domainParticipant)this,
            (const gapi_domainParticipantQos *)qos
        );
}

/*     ReturnCode_t
 *     get_qos(
 *         inout DomainParticipantQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_get_qos (
    DDS_DomainParticipant this,
    DDS_DomainParticipantQos *qos
    )
{
    return (DDS_ReturnCode_t)
    gapi_domainParticipant_get_qos (
        (gapi_domainParticipant)this,
        (gapi_domainParticipantQos *)qos
    );
}

/*     ReturnCode_t
 *     set_listener(
 *         in DomainParticipantListener a_listener,
 *         in StatusKindMask mask);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_set_listener (
    DDS_DomainParticipant this,
    const struct DDS_DomainParticipantListener *a_listener,
    const DDS_StatusMask mask
    )
{
    struct gapi_domainParticipantListener gListener;
    struct gapi_domainParticipantListener *pListener = NULL;

    if ( a_listener ) {
        sac_copySacDomainParticipantListener(a_listener, &gListener);
        pListener = &gListener;
    }

    return (DDS_ReturnCode_t)
        gapi_domainParticipant_set_listener (
            (gapi_domainParticipant)this,
            (const struct gapi_domainParticipantListener *)pListener,
            (gapi_statusMask)mask
        );
}

/*     DomainParticipantListener
 *     get_listener();
 */
struct DDS_DomainParticipantListener
DDS_DomainParticipant_get_listener (
    DDS_DomainParticipant this
    )
{
    struct DDS_DomainParticipantListener d;
    struct gapi_domainParticipantListener s;

    s = gapi_domainParticipant_get_listener ((gapi_domainParticipant)this);
    sac_copyGapiDomainParticipantListener (&s, &d);

    return d;
}

/*     ReturnCode_t
 *     ignore_participant(
 *         in InstanceHandle_t handle);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_ignore_participant (
    DDS_DomainParticipant this,
    const DDS_InstanceHandle_t handle
    )
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_ignore_participant (
            (gapi_domainParticipant)this,
            (gapi_instanceHandle_t)handle
        );
}

/*     ReturnCode_t
 *     ignore_topic(
 *         in InstanceHandle_t handle);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_ignore_topic (
    DDS_DomainParticipant this,
    const DDS_InstanceHandle_t handle
    )
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_ignore_topic (
            (gapi_domainParticipant)this,
            (gapi_instanceHandle_t)handle
        );
}

/*     ReturnCode_t
 *     ignore_publication(
 *         in InstanceHandle_t handle);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_ignore_publication (
    DDS_DomainParticipant this,
    const DDS_InstanceHandle_t handle
    )
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_ignore_publication (
            (gapi_domainParticipant)this,
            (gapi_instanceHandle_t)handle
        );
}

/*     ReturnCode_t
 *     ignore_subscription(
 *         in InstanceHandle_t handle);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_ignore_subscription (
    DDS_DomainParticipant this,
    const DDS_InstanceHandle_t handle
    )
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_ignore_subscription (
            (gapi_domainParticipant)this,
            (gapi_instanceHandle_t)handle
        );
}

/*     DomainId_t
 *     get_domain_id();
 */
DDS_DomainId_t
DDS_DomainParticipant_get_domain_id (
    DDS_DomainParticipant this
    )
{
    return (DDS_DomainId_t)
        gapi_domainParticipant_get_domain_id (
            (gapi_domainParticipant)this
        );
}

/*     void
 *     assert_liveliness();
 */
DDS_ReturnCode_t
DDS_DomainParticipant_assert_liveliness (
    DDS_DomainParticipant this
    )
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_assert_liveliness (
            (gapi_domainParticipant)this
        );
}

/*     ReturnCode_t
 *     set_default_publisher_qos(
 *         in PublisherQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_set_default_publisher_qos (
    DDS_DomainParticipant this,
    const DDS_PublisherQos *qos
    )
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_set_default_publisher_qos (
            (gapi_domainParticipant)this,
            (const gapi_publisherQos *)qos
        );
}

/*     ReturnCode_t
 *     get_default_publisher_qos(
 *         inout PublisherQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_get_default_publisher_qos (
    DDS_DomainParticipant this,
    DDS_PublisherQos *qos
    )
{
    return (DDS_ReturnCode_t)
    gapi_domainParticipant_get_default_publisher_qos (
        (gapi_domainParticipant)this,
        (gapi_publisherQos *)qos
    );

}

/*     ReturnCode_t
 *     set_default_subscriber_qos(
 *         in SubscriberQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_set_default_subscriber_qos (
    DDS_DomainParticipant this,
    const DDS_SubscriberQos *qos
    )
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_set_default_subscriber_qos (
            (gapi_domainParticipant)this,
            (const gapi_subscriberQos *)qos
        );
}

/*     ReturnCode_t
 *     get_default_subscriber_qos(
 *         inout SubscriberQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_get_default_subscriber_qos (
    DDS_DomainParticipant this,
    DDS_SubscriberQos *qos
    )
{
    return (DDS_ReturnCode_t)
    gapi_domainParticipant_get_default_subscriber_qos (
        (gapi_domainParticipant)this,
        (gapi_subscriberQos *)qos
    );

}

/*     ReturnCode_t
 *     set_default_topic_qos(
 *         in TopicQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_set_default_topic_qos (
    DDS_DomainParticipant this,
    const DDS_TopicQos *qos
    )
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_set_default_topic_qos (
            (gapi_domainParticipant)this,
            (const gapi_topicQos *)qos
        );
}

/*     ReturnCode_t
 *     get_default_topic_qos(
 *         inout TopicQos qos);
 */
    DDS_ReturnCode_t
DDS_DomainParticipant_get_default_topic_qos (
    DDS_DomainParticipant this,
    DDS_TopicQos *qos
    )
{
    return (DDS_ReturnCode_t)
    gapi_domainParticipant_get_default_topic_qos (
        (gapi_domainParticipant)this,
        (gapi_topicQos *)qos
    );

}

struct copyInstanceHandle {
    c_ulong index;
    DDS_InstanceHandleSeq *seq;
};

static c_bool
copyInstanceHandle(
    v_dataReaderInstance instance,
    c_voidp arg)
{
    c_bool result = TRUE;
    struct copyInstanceHandle *a = (struct copyInstanceHandle *)arg;
    DDS_InstanceHandle_t ghandle;
    DDS_unsigned_long length;

    if (a->index == 0) {
        length = c_count(v_dataReaderInstanceGetNotEmptyInstanceSet(instance));

        /*buffer alloc*/
        if (length > a->seq->_maximum) {

            /* if release is true free the current buffer*/
            if (a->seq->_release) {
                gapi_free(a->seq->_buffer);
            }
            /* reallocate a new buffer */
            a->seq->_buffer = gapi_instanceHandleSeq_allocbuf(length);
            a->seq->_release = TRUE;
            a->seq->_length = 0;
            a->seq->_maximum = length;
        } else {
            /* error */
        }
    }

    ghandle = u_instanceHandleNew((v_public)instance);
    if (a->index < a->seq->_maximum) {
        a->seq->_buffer[a->index++] = ghandle;
        a->seq->_length++;
    } else {
        /* error index out of bounds */
    }

    return result;
}


/*     ReturnCode_t
 *     get_discovered_participants (
 *         inout InstanceHandleSeq participant_handles);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_get_discovered_participants (
    DDS_DomainParticipant this,
    DDS_InstanceHandleSeq  *participant_handles)
{
    struct copyInstanceHandle cih;
    cih.index = 0;
    participant_handles->_length =0;
    cih.seq = participant_handles;

    return (DDS_ReturnCode_t)
        gapi_domainParticipant_get_discovered_participants (
            (gapi_domainParticipant)this,
            copyInstanceHandle,
            &cih
        );
}

static void
copyDiscoveredData(
    c_voidp from,
    c_voidp to)
{
    if (from) {
        gapi_participantBuiltinTopicData__copyOut(from, to);
    }
}

/*     ReturnCode_t
 *     get_discovered_participant_data (
 *         in InstanceHandle_t handle,
 *         inout ParticipantBuiltinTopicData *participant_data);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_get_discovered_participant_data (
    DDS_DomainParticipant this,
    DDS_ParticipantBuiltinTopicData *participant_data,
    DDS_InstanceHandle_t  handle)
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_get_discovered_participant_data (
            (gapi_domainParticipant)this,
            (gapi_participantBuiltinTopicData *)participant_data,
            (gapi_instanceHandle_t)handle,
            copyDiscoveredData
        );
}

/*     ReturnCode_t
 *     get_discovered_topics (
 *         inout InstanceHandleSeq topic_handles);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_get_discovered_topics (
    DDS_DomainParticipant this,
    DDS_InstanceHandleSeq  *topic_handles)
{
    struct copyInstanceHandle cih;
    cih.index = 0;
    topic_handles->_length =0;
    cih.seq = topic_handles;

    return (DDS_ReturnCode_t)
        gapi_domainParticipant_get_discovered_topics (
            (gapi_domainParticipant)this,
            copyInstanceHandle,
            &cih
        );
}

static void
copyDiscoveredTopicData(
    c_voidp from,
    c_voidp to)
{
    if (from) {
        gapi_topicBuiltinTopicData__copyOut(from, to);
    }
}

/*     ReturnCode_t
 *     get_discovered_topic_data (
 *         in InstanceHandle_t handle,
 *         inout TopicBuiltinTopicData *topic_data);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_get_discovered_topic_data (
    DDS_DomainParticipant this,
    DDS_TopicBuiltinTopicData *topic_data,
    DDS_InstanceHandle_t  handle)
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_get_discovered_topic_data (
                (gapi_domainParticipant)this,
                (gapi_topicBuiltinTopicData *)topic_data,
                (gapi_instanceHandle_t)handle,
                copyDiscoveredTopicData
            );
}

/*     Boolean
 *     contains_entity (
 *         in InstanceHandle_t a_hande);
 */
DDS_boolean
DDS_DomainParticipant_contains_entity (
    DDS_DomainParticipant this,
    DDS_InstanceHandle_t  a_handle)
{
    return (DDS_boolean)
        gapi_domainParticipant_contains_entity (
            (gapi_domainParticipant)this,
            (gapi_instanceHandle_t)a_handle
        );
}

/*     ReturnCode_t
 *     get_current_time (
 *         inout Time_t current_time);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_get_current_time (
    DDS_DomainParticipant this,
    DDS_Time_t  *current_time)
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_get_current_time (
            (gapi_domainParticipant)this,
            (gapi_time_t *)current_time
        );
}

/*     TypeSupport
 *     lookup_typesupport (
 *         in string type_name);
 */
DDS_TypeSupport
DDS_DomainParticipant_lookup_typesupport (
    DDS_DomainParticipant this,
    const DDS_char *type_name)
{
    return (DDS_TypeSupport)
        gapi_domainParticipant_lookup_typesupport (
            (gapi_domainParticipant)this,
            (const gapi_char *) type_name
        );
}

DDS_ReturnCode_t
DDS_DomainParticipant_delete_historical_data (
    DDS_DomainParticipant this,
    const DDS_string partition_expression,
    const DDS_string topic_expression)
{
    return (DDS_ReturnCode_t)
        gapi_domainParticipant_delete_historical_data(
            (gapi_domainParticipant)this,
            (gapi_string)partition_expression,
            (gapi_string)topic_expression
        );
}
