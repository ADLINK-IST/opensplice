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
import org.omg.dds.core.Duration;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.policy.Partition;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.pub.DataWriter;
import org.omg.dds.pub.DataWriterQos;
import org.omg.dds.pub.Publisher;
import org.omg.dds.pub.PublisherQos;
import org.omg.dds.topic.Topic;
import java.util.Arrays;
import java.util.concurrent.TimeUnit;
import org.omg.dds.core.GuardCondition;
import org.omg.dds.core.policy.History;
import org.omg.dds.core.policy.ResourceLimits;
import org.omg.dds.domain.DomainParticipantFactory;

/**
 * This class serves as a container holding initialised entities used for
 * publishing
 */
class PubEntities {

    public PubEntities(String partitionName) throws Exception,
            NullPointerException {

        /*
         * Select DDS implementation and initialize DDS ServiceEnvironment
         */
        System.setProperty(
                ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                "org.opensplice.dds.core.OsplServiceEnvironment");
        env = ServiceEnvironment.createInstance(PubEntities.class
                .getClassLoader());
        PolicyFactory policyFactory = env.getSPI().getPolicyFactory();
        Partition publisherPartition = policyFactory.Partition().withName(
                Arrays.asList(partitionName));

        /**
         * A DomainParticipant is created for the default domain.
         */
        domainParticipantFactory = DomainParticipantFactory.getInstance(env);
        // ExampleError.CheckHandle(domainParticipantFactory,
        // "Entities, DDS.DomainParticipantFactory.get_instance");
        participant = domainParticipantFactory.createParticipant();
        // ExampleError.CheckHandle(participant,
        // "Entities, domainParticipantFactory.create_participant");

        /**
         * A Topic is created for our sample type on the domain participant.
         */
        topic = participant.createTopic("Throughput",
                ThroughputModule.DataType.class);
        // ExampleError.CheckHandle(topic,
        // "Entities, participant.create_topic");

        /**
         * A Publisher is created on the domain participant.
         */
        PublisherQos pubQos = participant.getDefaultPublisherQos();
        // ExampleError.CheckHandle(pubQos,
        // "Entities, participant.getDefaultPublisherQos");
        pubQos = pubQos.withPolicies(publisherPartition);
        publisher = participant.createPublisher(pubQos);
        // ExampleError.CheckHandle(publisher,
        // "Entities, participant.createPublisher");

        /**
         * A DataWriter is created on the Publisher & Topic with a modified Qos.
         */
        DataWriterQos dwQos = publisher.getDefaultDataWriterQos();
        // ExampleError.CheckHandle(dwQos,
        // "Entities, publisher.getDefaultDataWriterQos");
        Duration max_blocking_time = env.getSPI().newDuration(10,
                TimeUnit.SECONDS);
        Reliability reliable = policyFactory.Reliability().withReliable()
                .withMaxBlockingTime(max_blocking_time);
        History history = policyFactory.History().withKeepAll();
        // WriterDataLifecycle writer_data_lifecycle =
        // policyFactory.WriterDataLifecycle().withAutDisposeUnregisteredInstances(false);
        ResourceLimits resourcelimits = policyFactory.ResourceLimits()
                .withMaxSamples(100);
        dwQos = dwQos.withPolicies(reliable).withPolicies(history)
                .withPolicies(resourcelimits);
        writer = publisher.createDataWriter(topic, dwQos);
        // ExampleError.CheckHandle(writer,
        // "Entities, publisher.createDataWriter");

        terminated = env.getSPI().newGuardCondition();

    }

    public ServiceEnvironment env;
    public DomainParticipantFactory domainParticipantFactory;
    public DomainParticipant participant;
    public ThroughputModule.DataType typeSupport;
    public Topic<ThroughputModule.DataType> topic;
    public Publisher publisher;
    public DataWriter<ThroughputModule.DataType> writer;
    public GuardCondition terminated;
}
