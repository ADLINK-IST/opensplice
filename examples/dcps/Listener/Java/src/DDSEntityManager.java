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

import DDS.ANY_INSTANCE_STATE;
import DDS.ANY_SAMPLE_STATE;
import DDS.ANY_STATUS;
import DDS.ANY_VIEW_STATE;
import DDS.DATAREADER_QOS_USE_TOPIC_QOS;
import DDS.DATAWRITER_QOS_USE_TOPIC_QOS;
import DDS.DataReader;
import DDS.DataReaderQosHolder;
import DDS.DataWriter;
import DDS.DataWriterQosHolder;
import DDS.DomainParticipant;
import DDS.DomainParticipantFactory;
import DDS.ReliabilityQosPolicyKind;
import DDS.PARTICIPANT_QOS_DEFAULT;
import DDS.Publisher;
import DDS.PublisherQosHolder;
import DDS.QueryCondition;
import DDS.Subscriber;
import DDS.SubscriberQosHolder;
import DDS.Topic;
import DDS.TopicQosHolder;
import DDS.DurabilityQosPolicyKind;

public class DDSEntityManager {

	private DomainParticipantFactory dpf;
	private DomainParticipant participant;
	private Topic topic;
	private TopicQosHolder topicQos = new TopicQosHolder();
	private PublisherQosHolder pubQos = new PublisherQosHolder();
	private SubscriberQosHolder subQos = new SubscriberQosHolder();
	private QueryCondition qc;

	private DataWriterQosHolder WQosH = new DataWriterQosHolder();
	private DataReaderQosHolder RQosH = new DataReaderQosHolder();

	private Publisher publisher;
	private DataWriter writer;

	private Subscriber subscriber;
	private DataReader reader;

	private String typeName;
	private String partitionName;

	public void createParticipant(String partitionName) {
		dpf = DomainParticipantFactory.get_instance();
		ErrorHandler.checkHandle(dpf, "DomainParticipantFactory.get_instance");

		participant = dpf.create_participant(null,
				PARTICIPANT_QOS_DEFAULT.value, null, ANY_STATUS.value);
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

	Topic createTopic(String topicName) {
		int status = -1;
		participant.get_default_topic_qos(topicQos);
		topicQos.value.reliability.kind = ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
		topicQos.value.durability.kind = DDS.DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS;
		// DeadlineQoSPolicy : period used to trigger the listener
		// (on_requested_deadline_missed)
		topicQos.value.deadline.period.nanosec = 0;
		topicQos.value.deadline.period.sec = 5;

		status = participant.set_default_topic_qos(topicQos.value);
		ErrorHandler.checkStatus(status,
				"DomainParticipant.set_default_topic_qos");
		topic = participant.create_topic(topicName, typeName, topicQos.value,
				null, ANY_STATUS.value);
		ErrorHandler.checkHandle(topic, "DomainParticipant.create_topic");
		return topic;
	}

	/*
	 * Create QueryCondition
	 */
	public void createQueryCondition(DataReader reader, String arg) {
		String[] tab = new String[1];
		tab[0] = arg;
		qc = reader.create_querycondition(ANY_SAMPLE_STATE.value,
				ANY_VIEW_STATE.value, ANY_INSTANCE_STATE.value, "price>=%0",
				tab);
	}

	/*
	 * Delet contained entities
	 */
	public void deleteContainerEntities() {
		participant.delete_contained_entities();
	}

	/*
	 * Delete Topic
	 */

	public void deleteTopic() {

		int status = participant.delete_topic(topic);
		ErrorHandler.checkStatus(status, "DDS.DomainParticipant.delete_topic");
	}

	public void createPublisher() {
		int status = participant.get_default_publisher_qos(pubQos);
		ErrorHandler.checkStatus(status,
				"DomainParticipant.get_default_publisher_qos");

		pubQos.value.partition.name = new String[1];
		pubQos.value.partition.name[0] = partitionName;
		publisher = participant.create_publisher(pubQos.value, null,
				ANY_STATUS.value);
		ErrorHandler.checkHandle(publisher,
				"DomainParticipant.create_publisher");
	}

	public void deletePublisher() {
		participant.delete_publisher(publisher);
	}

	public void createWriter() {
		publisher.get_default_datawriter_qos(WQosH);
		publisher.copy_from_topic_qos(WQosH, topicQos.value);
		WQosH.value.writer_data_lifecycle.autodispose_unregistered_instances = false;
		writer = publisher.create_datawriter(topic, WQosH.value,null, ANY_STATUS.value);
		ErrorHandler.checkHandle(writer, "Publisher.create_datawriter");
	}

	public void createSubscriber() {
		int status = participant.get_default_subscriber_qos(subQos);
		ErrorHandler.checkStatus(status,
				"DomainParticipant.get_default_subscriber_qos");

		subQos.value.partition.name = new String[1];
		subQos.value.partition.name[0] = partitionName;
		subscriber = participant.create_subscriber(subQos.value, null,
				ANY_STATUS.value);
		ErrorHandler.checkHandle(subscriber,
				"DomainParticipant.create_subscriber");
	}

	public void deleteSubscriber() {
		participant.delete_subscriber(subscriber);
	}

	public void createReader() {
		subscriber.get_default_datareader_qos(RQosH);
		subscriber.copy_from_topic_qos(RQosH, topicQos.value);

		reader = subscriber.create_datareader(topic, RQosH.value, null,
				ANY_STATUS.value);
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

	public QueryCondition getQueryCondition() {
		return qc;
	}

	public DomainParticipant getParticipant() {
		return participant;
	}
}
