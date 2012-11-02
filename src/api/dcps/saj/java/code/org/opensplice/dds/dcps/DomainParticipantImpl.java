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


package org.opensplice.dds.dcps;

/**
 * Implementation of the {@link DDS.DomainParticipant} interface.
 */
public class DomainParticipantImpl extends EntityImpl implements DDS.DomainParticipant {

    private long participantDataCopyCache;
    private long topicBuiltinTopicDataCopyCache;

    /* see DDS.DomainParticipantOperations for javadoc */
    public DDS.Publisher create_publisher (DDS.PublisherQos qos, DDS.PublisherListener a_listener,int mask) {
        return jniCreatePublisher(qos, a_listener,mask);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int delete_publisher (DDS.Publisher p) {
        return jniDeletePublisher(p);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public DDS.Subscriber create_subscriber (DDS.SubscriberQos qos, DDS.SubscriberListener a_listener,int mask) {
        return jniCreateSubscriber(qos, a_listener,mask);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int delete_subscriber (DDS.Subscriber s) {
        return jniDeleteSubscriber(s);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public DDS.Subscriber get_builtin_subscriber () {
        return jniGetBuiltinSubscriber();
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public DDS.Topic create_topic (String topic_name, String type_name, DDS.TopicQos qos, DDS.TopicListener a_listener,int mask) {
        return jniCreateTopic(topic_name, type_name, qos, a_listener,mask);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int delete_topic (DDS.Topic a_topic) {
        return jniDeleteTopic(a_topic);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public DDS.Topic find_topic (String topic_name, DDS.Duration_t timeout) {
        return jniFindTopic(topic_name, timeout);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public DDS.TopicDescription lookup_topicdescription (String name) {
        return jniLookupTopicdescription(name);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public DDS.ContentFilteredTopic create_contentfilteredtopic (String name, DDS.Topic related_topic, String filter_expression, String[] expression_parameters) {
        return jniCreateContentfilteredtopic(name, related_topic, filter_expression, expression_parameters);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int delete_contentfilteredtopic (DDS.ContentFilteredTopic a_contentfilteredtopic) {
        return jniDeleteContentfilteredtopic(a_contentfilteredtopic);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public DDS.MultiTopic create_multitopic (String name, String type_name, String subscription_expression, String[] expression_parameters) {
        return jniCreateMultitopic(name, type_name, subscription_expression, expression_parameters);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int delete_multitopic (DDS.MultiTopic a_multitopic) {
        return jniDeleteMultitopic(a_multitopic);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int delete_contained_entities () {
        return jniDeleteContainedEntities();
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int set_qos (DDS.DomainParticipantQos qos) {
    	return jniSetQos(qos);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int get_qos (DDS.DomainParticipantQosHolder qos) {
        return jniGetQos(qos);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int set_listener (DDS.DomainParticipantListener a_listener, int mask) {
        return jniSetListener(a_listener, mask);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public DDS.DomainParticipantListener get_listener () {
        return jniGetListener();
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int ignore_participant (long handle) {
        return jniIgnoreParticipant(handle);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int ignore_topic (long handle) {
        return jniIgnoreTopic(handle);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int ignore_publication (long handle) {
        return jniIgnorePublication(handle);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int ignore_subscription (long handle) {
        return jniIgnoreSubscription(handle);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public String get_domain_id () {
        return jniGetDomainId();
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int assert_liveliness () {
        return jniAssertLiveliness();
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int set_default_publisher_qos (DDS.PublisherQos qos) {
        return jniSetDefaultPublisherQos(qos);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int get_default_publisher_qos (DDS.PublisherQosHolder qos) {
        return jniGetDefaultPublisherQos(qos);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int set_default_subscriber_qos (DDS.SubscriberQos qos) {
        return jniSetDefaultSubscriberQos(qos);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int get_default_subscriber_qos (DDS.SubscriberQosHolder qos) {
        return jniGetDefaultSubscriberQos(qos);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int set_default_topic_qos (DDS.TopicQos qos) {
        return jniSetDefaultTopicQos(qos);
    }

    /* see DDS.DomainParticipantOperations for javadoc */
    public int get_default_topic_qos (DDS.TopicQosHolder qos) {
        return jniGetDefaultTopicQos(qos);
    }

    public int get_discovered_participants (DDS.InstanceHandleSeqHolder participant_handles) {
        return jniGetDiscoveredParticipants(participant_handles);
    }

    public int get_discovered_participant_data (DDS.ParticipantBuiltinTopicDataHolder participant_data, long handle) {
        return jniGetDiscoveredParticipantData(participant_data, handle, participantDataCopyCache);
    }

    public int get_discovered_topics (DDS.InstanceHandleSeqHolder topic_handles) {
        return jniGetDiscoveredTopics(topic_handles);
    }

    public int get_discovered_topic_data (DDS.TopicBuiltinTopicDataHolder topic_data, long handle) {
        return jniGetDiscoveredTopicData(topic_data, handle, topicBuiltinTopicDataCopyCache);
    }

    public boolean contains_entity (long a_handle) {
        return jniContainsEntity(a_handle);
    }

    public int get_current_time (DDS.Time_tHolder current_time) {
        return jniGetCurrentTime(current_time);
    }

    void setParticipantDataCopyCache(long copyCache)
    {
    	this.participantDataCopyCache = copyCache;
    }
    
    void setTopicBuiltinTopicDataCopyCache(long copyCache)
    {
    	this.topicBuiltinTopicDataCopyCache = copyCache;
    }

    private native DDS.Publisher jniCreatePublisher(DDS.PublisherQos qos, DDS.PublisherListener a_listener, int mask);
    private native int jniDeletePublisher(DDS.Publisher p);
    private native DDS.Subscriber jniCreateSubscriber(DDS.SubscriberQos qos, DDS.SubscriberListener a_listener, int mask);
    private native int jniDeleteSubscriber(DDS.Subscriber s);
    private native DDS.Subscriber jniGetBuiltinSubscriber();
    private native DDS.Topic jniCreateTopic(String topic_name, String type_name, DDS.TopicQos qos, DDS.TopicListener a_listener, int mask);
    private native int jniDeleteTopic(DDS.Topic a_topic);
    private native DDS.Topic jniFindTopic(String topic_name, DDS.Duration_t timeout);
    private native DDS.TopicDescription jniLookupTopicdescription(String name);
    private native DDS.ContentFilteredTopic jniCreateContentfilteredtopic(String name, DDS.Topic related_topic, String filter_expression, String[] expression_parameters);
    private native int jniDeleteContentfilteredtopic(DDS.ContentFilteredTopic a_contentfilteredtopic);
    private native DDS.MultiTopic jniCreateMultitopic(String name, String type_name, String subscription_expression, String[] expression_parameters);
    private native int jniDeleteMultitopic(DDS.MultiTopic a_multitopic);
    private native int jniDeleteContainedEntities();
    private native int jniSetQos(DDS.DomainParticipantQos qos);
    private native int jniGetQos(DDS.DomainParticipantQosHolder qos);
    private native int jniSetListener(DDS.DomainParticipantListener a_listener, int mask);
    private native DDS.DomainParticipantListener jniGetListener();
    private native int jniIgnoreParticipant(long handle);
    private native int jniIgnoreTopic(long handle);
    private native int jniIgnorePublication(long handle);
    private native int jniIgnoreSubscription(long handle);
    private native String jniGetDomainId();
    private native int jniAssertLiveliness();
    private native int jniSetDefaultPublisherQos(DDS.PublisherQos qos);
    private native int jniGetDefaultPublisherQos(DDS.PublisherQosHolder qos);
    private native int jniSetDefaultSubscriberQos(DDS.SubscriberQos qos);
    private native int jniGetDefaultSubscriberQos(DDS.SubscriberQosHolder qos);
    private native int jniSetDefaultTopicQos(DDS.TopicQos qos);
    private native int jniGetDefaultTopicQos(DDS.TopicQosHolder qos);
    private native int jniGetDiscoveredParticipants(DDS.InstanceHandleSeqHolder participant_handles);
    private native int jniGetDiscoveredParticipantData(DDS.ParticipantBuiltinTopicDataHolder participant_data, long handle, long copyCache);
    private native int jniGetDiscoveredTopics(DDS.InstanceHandleSeqHolder topic_handles);
    private native int jniGetDiscoveredTopicData(DDS.TopicBuiltinTopicDataHolder topic_data, long handle, long copyCache);
    private native boolean jniContainsEntity(long a_handle);
    private native int jniGetCurrentTime(DDS.Time_tHolder current_time);

}
