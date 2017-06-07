/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
/************************************************************************
 *  
 * Copyright (c) 2007
 * PrismTech Ltd.
 * All rights Reserved.
 * 
 * LOGICAL_NAME:    DDSEntityManager.java
 * FUNCTION:        OpenSplice example code.
 * MODULE:          Tutorial for the Java programming language.
 * DATE             April 2009.
 ************************************************************************
 * 
 * This file contains the implementation for the 'DDSEntityManager' executable.
 * 
 ***/

import org.opensplice.dds.dcps.TypeSupportImpl;

import DDS.STATUS_MASK_NONE; 
import DDS.DOMAIN_ID_DEFAULT;
import DDS.ContentFilteredTopic;
import DDS.DATAREADER_QOS_USE_TOPIC_QOS;
import DDS.DATAWRITER_QOS_USE_TOPIC_QOS;
import DDS.DataReader;
import DDS.DataWriter;
import DDS.DataWriterQosHolder;
import DDS.DomainParticipant;
import DDS.DomainParticipantFactory;
import DDS.PARTICIPANT_QOS_DEFAULT;
import DDS.Publisher;
import DDS.PublisherQosHolder;
import DDS.ReliabilityQosPolicyKind;
import DDS.Subscriber;
import DDS.SubscriberQosHolder;
import DDS.Topic;
import DDS.TopicQosHolder;
import DDS.DurabilityQosPolicyKind;

public class DDSEntityManager {

	private DomainParticipantFactory dpf;
	private DomainParticipant participant;
	private Topic topic;
	private ContentFilteredTopic filteredTopic;
	private TopicQosHolder topicQos = new TopicQosHolder();
	private PublisherQosHolder pubQos = new PublisherQosHolder();
	private SubscriberQosHolder subQos = new SubscriberQosHolder();

	private Publisher publisher;
	private DataWriter writer;

	private Subscriber subscriber;
	private DataReader reader;

	private String typeName;
	private String partitionName;

	public void createParticipant(String partitionName) {
		dpf = DomainParticipantFactory.get_instance();
		ErrorHandler.checkHandle(dpf, "DomainParticipantFactory.get_instance");

		participant = dpf.create_participant(DOMAIN_ID_DEFAULT.value,
				PARTICIPANT_QOS_DEFAULT.value, null, STATUS_MASK_NONE.value);
		ErrorHandler.checkHandle(dpf,
				"DomainParticipantFactory.create_participant");
		this.partitionName = partitionName;
	}

	public void deleteParticipant() {
		dpf.delete_participant(participant);
	}

	public void registerType(TypeSupportImpl ts) {
		typeName = ts.get_type_name();
		int status = ts.register_type(participant, typeName);
		ErrorHandler.checkStatus(status, "register_type");
	}

	public void createTopic(String topicName) {
		int status = -1;
		participant.get_default_topic_qos(topicQos);
		topicQos.value.reliability.kind = ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
		topicQos.value.durability.kind = DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS;
		status = participant.set_default_topic_qos(topicQos.value);
		ErrorHandler.checkStatus(status,
				"DomainParticipant.set_default_topic_qos");
		topic = participant.create_topic(topicName, typeName, topicQos.value,
				null, STATUS_MASK_NONE.value);
		ErrorHandler.checkHandle(topic, "DomainParticipant.create_topic");
	}

	public void createContentFilteredTopic(String topicName, String filter,
			String[] expr) {
		filteredTopic = participant.create_contentfilteredtopic(topicName,
				topic, filter, expr);
		ErrorHandler.checkHandle(filteredTopic,
				"DomainParticipant.create_contentfilteredtopic");
	}

	public void deleteTopic() {
		int status = participant.delete_topic(topic);
		ErrorHandler.checkStatus(status, "DDS.DomainParticipant.delete_topic");
	}

	public void deleteFilteredTopic() {
		if (filteredTopic == null)
			return;
		int status = participant.delete_contentfilteredtopic(filteredTopic);
		ErrorHandler.checkStatus(status,
				"DDS.DomainParticipant.delete_contentfilteredtopic");
	}

	public void createPublisher() {
		int status = participant.get_default_publisher_qos(pubQos);
		ErrorHandler.checkStatus(status,
				"DomainParticipant.get_default_publisher_qos");

		pubQos.value.partition.name = new String[1];
		pubQos.value.partition.name[0] = partitionName;
		publisher = participant.create_publisher(pubQos.value, null,
				STATUS_MASK_NONE.value);
		ErrorHandler.checkHandle(publisher,
				"DomainParticipant.create_publisher");
	}

	public void deletePublisher() {
		participant.delete_publisher(publisher);
	}

	public void createWriter() {
		TopicQosHolder topicQos = new TopicQosHolder();
		topic.get_qos(topicQos);
		 
		DataWriterQosHolder writerQos = new DataWriterQosHolder();
		publisher.get_default_datawriter_qos(writerQos);
		publisher.copy_from_topic_qos(writerQos,topicQos.value);
		
		writerQos.value.writer_data_lifecycle.autodispose_unregistered_instances = false;
		writer = publisher.create_datawriter(topic,
				writerQos.value, null, STATUS_MASK_NONE.value);
		ErrorHandler.checkHandle(writer, "Publisher.create_datawriter");
	}

	public void createSubscriber() {
		int status = participant.get_default_subscriber_qos(subQos);
		ErrorHandler.checkStatus(status,
				"DomainParticipant.get_default_subscriber_qos");

		subQos.value.partition.name = new String[1];
		subQos.value.partition.name[0] = partitionName;
		subscriber = participant.create_subscriber(subQos.value, null,
				STATUS_MASK_NONE.value);
		ErrorHandler.checkHandle(subscriber,
				"DomainParticipant.create_subscriber");
	}

	public void deleteSubscriber() {
		participant.delete_subscriber(subscriber);
	}

	public void createReader(boolean filtered) {
		if (!filtered)
			reader = subscriber.create_datareader(topic,
					DATAREADER_QOS_USE_TOPIC_QOS.value, null, STATUS_MASK_NONE.value);
		else
			reader = subscriber.create_datareader(filteredTopic,
					DATAREADER_QOS_USE_TOPIC_QOS.value, null, STATUS_MASK_NONE.value);
		ErrorHandler.checkHandle(reader, "Subscriber.create_datareader");
	}

	public DataReader getReader() {
		return reader;
	}

	public DataWriter getWriter() {
		return writer;
	}

	public Publisher getPublisher() {
		return publisher;
	}

	public Subscriber getSubscriber() {
		return subscriber;
	}

	public Topic getTopic() {
		return topic;
	}

	public DomainParticipant getParticipant() {
		return participant;
	}
}
