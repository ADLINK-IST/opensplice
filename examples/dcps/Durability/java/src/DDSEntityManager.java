/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

import org.opensplice.dds.dcps.TypeSupportImpl;

import DDS.*;

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
	private String m_durability_kind;
	private boolean m_autodispose_unregistered_instances;

	DDSEntityManager(String durability_kind) {
		m_durability_kind = durability_kind;
	}

	DDSEntityManager(String durability_kind,
			Boolean autodispose_unregistered_instances) {
		m_durability_kind = durability_kind;
		m_autodispose_unregistered_instances = autodispose_unregistered_instances;
	}

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

	Topic createTopic(String topicName) {
		int status = -1;
		participant.get_default_topic_qos(topicQos);
		topicQos.value.reliability.kind = DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
		if (m_durability_kind.equals("transient")) {
			topicQos.value.durability.kind = DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS;
		} else if (m_durability_kind.equals("persistent")) {
			topicQos.value.durability.kind = DurabilityQosPolicyKind.PERSISTENT_DURABILITY_QOS;
		}
		status = participant.set_default_topic_qos(topicQos.value);
		ErrorHandler.checkStatus(status,
				"DomainParticipant.set_default_topic_qos");
		topic = participant.create_topic(topicName, typeName, topicQos.value,
				null, DDS.STATUS_MASK_NONE.value);
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
				STATUS_MASK_NONE.value);
		ErrorHandler.checkHandle(publisher,
				"DomainParticipant.create_publisher");
	}

	public void deletePublisher() {
		participant.delete_publisher(publisher);
	}

	public void createWriter() {
		publisher.get_default_datawriter_qos(WQosH);
		publisher.copy_from_topic_qos(WQosH, topicQos.value);
		WQosH.value.writer_data_lifecycle.autodispose_unregistered_instances = m_autodispose_unregistered_instances;
		writer = publisher.create_datawriter(topic, WQosH.value, null,
				STATUS_MASK_NONE.value);
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

	public void createReader() {
		subscriber.get_default_datareader_qos(RQosH);
		subscriber.copy_from_topic_qos(RQosH, topicQos.value);
		reader = subscriber.create_datareader(topic, RQosH.value, null,
				STATUS_MASK_NONE.value);
		ErrorHandler.checkHandle(reader, "Subscriber.create_datareader");
		// If the logic of your application requires it
		// wait (block) until all historical data are received or
		// until the timeout has elapsed
		DDS.Duration_t a_timeout = new Duration_t();
		a_timeout.sec = 2;
		a_timeout.nanosec = 10000000;
		reader.wait_for_historical_data(a_timeout);
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
