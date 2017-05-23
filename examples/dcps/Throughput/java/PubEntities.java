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

/**
 * This class serves as a container holding initialised entities used for publishing
 */
class PubEntities
{
    public PubEntities(String partitionName) throws Exception, NullPointerException
    {
        int status;

        /** A DDS::DomainParticipant is created for the default domain. */
        domainParticipantFactory = DDS.DomainParticipantFactory.get_instance();
        ExampleError.CheckHandle(domainParticipantFactory, "Entities, DDS.DomainParticipantFactory.get_instance");
        participant = domainParticipantFactory.create_participant(
            DDS.DOMAIN_ID_DEFAULT.value, DDS.PARTICIPANT_QOS_DEFAULT.value, null, DDS.STATUS_MASK_NONE.value);
        ExampleError.CheckHandle(participant, "Entities, domainParticipantFactory.create_participant");

        /** The sample type is created and registered */
        typeSupport = new ThroughputModule.DataTypeTypeSupport();
        status = typeSupport.register_type(participant, typeSupport.get_type_name());
        ExampleError.CheckStatus(status, "Entities, typeSupport.register_type");

        /** A DDS::Topic is created for our sample type on the domain participant. */
        topic = participant.create_topic("Throughput", typeSupport.get_type_name(),
                                            DDS.TOPIC_QOS_DEFAULT.value, null, DDS.STATUS_MASK_NONE.value);
        ExampleError.CheckHandle(topic, "Entities, participant.create_topic");

        /** A DDS::Publisher is created on the domain participant. */
        DDS.PublisherQosHolder pubQos = new DDS.PublisherQosHolder();
        status = participant.get_default_publisher_qos(pubQos);
        ExampleError.CheckStatus(status, "Entities, participant.get_default_publisher_qos");
        pubQos.value.partition.name = new String[1];
        pubQos.value.partition.name[0] = partitionName;
        publisher = participant.create_publisher(pubQos.value, null, DDS.STATUS_MASK_NONE.value);
        ExampleError.CheckHandle(publisher, "Entities, participant.create_publisher");

        /** A DDS::DataWriter is created on the Publisher & Topic with a modififed Qos. */
        DDS.DataWriterQosHolder dwQos = new DDS.DataWriterQosHolder();
        status = publisher.get_default_datawriter_qos(dwQos);
        ExampleError.CheckStatus(status, "Entities, publisher.get_default_datawriter_qos");
        dwQos.value.reliability.kind = DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
        dwQos.value.reliability.max_blocking_time.sec = 10;
        dwQos.value.history.kind = DDS.HistoryQosPolicyKind.KEEP_ALL_HISTORY_QOS;
        dwQos.value.resource_limits.max_samples = 100;
        DDS.DataWriter tmpWriter = publisher.create_datawriter(topic, dwQos.value, null, DDS.STATUS_MASK_NONE.value);
        writer = ThroughputModule.DataTypeDataWriterHelper.narrow(tmpWriter);
        ExampleError.CheckHandle(writer, "Entities, ThroughputModule.DataTypeDataWriterHelper.narrow");

        terminated = new DDS.GuardCondition();
    }

    public DDS.DomainParticipantFactory domainParticipantFactory;
    public DDS.DomainParticipant participant;
    public ThroughputModule.DataTypeTypeSupport typeSupport;
    public DDS.Topic topic;
    public DDS.Publisher publisher;
    public ThroughputModule.DataTypeDataWriter writer;
    public DDS.GuardCondition terminated;
}
