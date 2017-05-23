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
import java.util.ArrayList;
import org.omg.dds.core.Duration;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.StatusCondition;
import org.omg.dds.core.WaitSet;
import org.omg.dds.core.policy.Partition;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.core.status.DataAvailableStatus;
import org.omg.dds.core.status.Status;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.sub.*;
import org.omg.dds.topic.Topic;
import java.util.Arrays;
import java.util.Collection;
import java.util.concurrent.TimeUnit;
import org.omg.dds.core.GuardCondition;
import org.omg.dds.core.policy.History;
import org.omg.dds.core.policy.ResourceLimits;
import org.omg.dds.domain.DomainParticipantFactory;

/**
 * This class serves as a container holding initialized entities used for
 * subscribing
 */
class SubEntities {
    public SubEntities(String partitionName) throws Exception,
            NullPointerException {
        /*
         * Select DDS implementation and initialize DDS ServiceEnvironment
         */
        System.setProperty(
                ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                "org.opensplice.dds.core.OsplServiceEnvironment");
        env = ServiceEnvironment.createInstance(SubEntities.class
                .getClassLoader());
        PolicyFactory policyFactory = env.getSPI().getPolicyFactory();
        Partition subscriberPartition = policyFactory.Partition().withName(
                Arrays.asList(partitionName));

        /*
         * A DomainParticipant is created for the default domain.
         */
        domainParticipantFactory = DomainParticipantFactory.getInstance(env);
        participant = domainParticipantFactory.createParticipant();
        /*
         * A Topic is created for our sample type on the domain participant.
         */

        topic = participant.createTopic("Throughput",
                ThroughputModule.DataType.class);

        /*
         * A Subscriber is created on the domain participant.
         */
        SubscriberQos subQos = participant.getDefaultSubscriberQos();
        subQos = subQos.withPolicies(subscriberPartition);
        subscriber = participant.createSubscriber(subQos);

        /*
         * A DDS::DataReader is created on the Subscriber & Topic with the
         * default Topic QoS.
         */
        DataReaderQos drQos = subscriber.getDefaultDataReaderQos();
        Duration max_blocking_time = env.getSPI().newDuration(10,
                TimeUnit.SECONDS);
        Reliability reliable = policyFactory.Reliability().withReliable()
                .withMaxBlockingTime(max_blocking_time);
        History history = policyFactory.History().withKeepAll();
        ResourceLimits resourcelimits = policyFactory.ResourceLimits()
                .withMaxSamples(400); // JM 100 for Writer
        drQos = drQos.withPolicies(reliable).withPolicies(history)
                .withPolicies(resourcelimits);
        reader = subscriber.createDataReader(topic, drQos);

        /*
         * A StatusCondition is created which is triggered when data is
         * available to read
         */
        StatusCondition<DataReader<ThroughputModule.DataType>> condition = reader
                .getStatusCondition();
        Collection<Class<? extends Status>> statuses = new ArrayList<Class<? extends Status>>();
        statuses.add(DataAvailableStatus.class);
        condition.setEnabledStatuses(statuses);

        /*
         * A WaitSet is created and the data available status condition is
         * attached
         */
        waitSet = env.getSPI().newWaitSet();
        waitSet.attachCondition(condition);

        terminated = env.getSPI().newGuardCondition();
        waitSet.attachCondition(terminated);
    }

    public ServiceEnvironment env;
    public DomainParticipantFactory domainParticipantFactory;
    public DomainParticipant participant;
    public ThroughputModule.DataType typeSupport;
    public Topic<ThroughputModule.DataType> topic;
    public Subscriber subscriber;
    public DataReader<ThroughputModule.DataType> reader;
    public WaitSet waitSet;
    public StatusCondition<DataReader<ThroughputModule.DataType>> dataAvailable;
    public GuardCondition terminated;
}
