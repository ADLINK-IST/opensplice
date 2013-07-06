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
package org.opensplice.dds.dcps;

public class DomainParticipantBase extends EntityImpl {
	
	public class DomainParticipantBaseImpl extends DDS._DomainParticipantLocalBase {
		/* domainparticipant operations  */
		public DDS.Publisher create_publisher(DDS.PublisherQos qos, DDS.PublisherListener a_listener, int mask) { return null; }
		public int delete_publisher(DDS.Publisher p) { return 0; } 
		public DDS.Subscriber create_subscriber(DDS.SubscriberQos qos, DDS.SubscriberListener a_listener, int mask) { return null; }
		public int delete_subscriber(DDS.Subscriber s) { return 0; }
		public DDS.Subscriber get_builtin_subscriber() { return null; }
		public DDS.Topic create_topic(java.lang.String topic_name, java.lang.String type_name, DDS.TopicQos qos, DDS.TopicListener a_listener, int mask) { return null; }
		public int delete_topic(DDS.Topic a_topic) { return 0; }
		public DDS.Topic find_topic(java.lang.String topic_name, DDS.Duration_t timeout) { return null; }
		public DDS.TopicDescription lookup_topicdescription(java.lang.String name) { return null; }
		public DDS.ContentFilteredTopic create_contentfilteredtopic(java.lang.String name, DDS.Topic related_topic, java.lang.String filter_expression, java.lang.String[] expression_parameters) { return null; }
		public int delete_contentfilteredtopic(DDS.ContentFilteredTopic a_contentfilteredtopic) { return 0; }
		public DDS.MultiTopic create_multitopic(java.lang.String name, java.lang.String type_name, java.lang.String subscription_expression, java.lang.String[] expression_parameters) { return null; }
		public int delete_multitopic(DDS.MultiTopic a_multitopic) { return 0; }
		public int delete_contained_entities() { return 0; }
		public int set_qos(DDS.DomainParticipantQos qos) { return 0; }
		public int get_qos(DDS.DomainParticipantQosHolder qos) { return 0; }
		public int set_listener(DDS.DomainParticipantListener a_listener, int mask) { return 0; }
		public DDS.DomainParticipantListener get_listener() { return null; }
		public int ignore_participant(long handle) { return 0; }
		public int ignore_topic(long handle) { return 0; }
		public int ignore_publication(long handle) { return 0; }
		public int ignore_subscription(long handle) { return 0; }
		public int get_domain_id() { return 0; }
		public int assert_liveliness() { return 0; }
		public int set_default_publisher_qos(DDS.PublisherQos qos) { return 0; }
		public int get_default_publisher_qos(DDS.PublisherQosHolder qos) { return 0; } 
		public int set_default_subscriber_qos(DDS.SubscriberQos qos) { return 0; }
		public int get_default_subscriber_qos(DDS.SubscriberQosHolder qos) { return 0; }
		public int set_default_topic_qos(DDS.TopicQos qos) { return 0; }
		public int get_default_topic_qos(DDS.TopicQosHolder qos) { return 0; }
		public int get_discovered_participants(DDS.InstanceHandleSeqHolder participant_handles) { return 0; }
		public int get_discovered_participant_data(DDS.ParticipantBuiltinTopicDataHolder participant_data, long participant_handle) { return 0; }
		public int get_discovered_topics(DDS.InstanceHandleSeqHolder topic_handles) { return 0; }
		public int get_discovered_topic_data(DDS.TopicBuiltinTopicDataHolder topic_data, long topic_handle) { return 0; }
		public boolean contains_entity(long a_handle) { return false; }
		public int get_current_time(DDS.Time_tHolder current_time) { return 0; }
		
		/* entity operations */
		public int enable() { return 0; }
		public DDS.StatusCondition get_statuscondition() { return null; }
		public int get_status_changes() { return 0; }
		public long get_instance_handle() { return 0; }
	}
	
	private DDS._DomainParticipantLocalBase base;
	
	public DomainParticipantBase() {
		base = new DomainParticipantBaseImpl();
	}
	
	public String[] _ids() {
		return base._ids();
	}
	
}
