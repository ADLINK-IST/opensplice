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

import java.util.Collection;
import java.util.HashSet;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.omg.dds.core.Duration;
import org.omg.dds.core.GuardCondition;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.WaitSet;
import org.omg.dds.core.policy.Deadline;
import org.omg.dds.core.policy.Durability;
import org.omg.dds.core.policy.Partition;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.core.status.DataAvailableStatus;
import org.omg.dds.core.status.RequestedDeadlineMissedStatus;
import org.omg.dds.core.status.Status;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.domain.DomainParticipantFactory;
import org.omg.dds.sub.DataReaderQos;
import org.omg.dds.sub.Subscriber;
import org.omg.dds.sub.SubscriberQos;
import org.omg.dds.topic.Topic;
import org.omg.dds.topic.TopicQos;

import ListenerData.Msg;

public class ListenerDataSubscriber {
    static String partitionName = "Listener example";

	public static void main(String[] args) {
	    try {
            System.setProperty(
                      ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                      "org.opensplice.dds.core.OsplServiceEnvironment");
            ServiceEnvironment env = ServiceEnvironment
                      .createInstance(ListenerDataSubscriber.class.getClassLoader());

            DomainParticipantFactory dpf = DomainParticipantFactory.getInstance(env);

            // create Domain Participant
            DomainParticipant participant = dpf.createParticipant();

            // create Topic with reliable, transient and deadline qos
            PolicyFactory policyFactory = env.getSPI().getPolicyFactory();
            Reliability reliable = policyFactory.Reliability().withReliable();
            Durability durability = policyFactory.Durability().withTransient();
            Deadline deadline = policyFactory.Deadline().withPeriod(Duration.newDuration(1, TimeUnit.SECONDS, env));
            TopicQos topicQos = participant.getDefaultTopicQos().withPolicies(reliable,durability,deadline);
            Collection<Class<? extends Status>> status = new HashSet<Class<? extends Status>>();
            Topic<Msg> topic = participant.createTopic("ListenerData_Msg", Msg.class,topicQos, null, status);

            // create Subscriber with partition qos
            Partition partition = policyFactory.Partition().withName(partitionName);
            SubscriberQos subQos = participant.getDefaultSubscriberQos().withPolicy(partition);
            Subscriber sub = participant.createSubscriber(subQos);


            GuardCondition myGC = GuardCondition.newGuardCondition(env);
            ListenerDataListener myListener = new ListenerDataListener(myGC);
            Collection<Class<? extends Status>> statuses = new HashSet<Class<? extends Status>>();
            statuses.add(DataAvailableStatus.class);
            statuses.add(RequestedDeadlineMissedStatus.class);

            // create dataReader with the attached listener
            DataReaderQos drQos = sub.copyFromTopicQos(sub.getDefaultDataReaderQos(),topic.getQos());
            sub.createDataReader(topic,drQos,myListener,statuses);

    		// waitset used to avoid spinning in the loop below
    		WaitSet ws = WaitSet.newWaitSet(env);
    		ws.attachCondition(myGC);
    		int count = 0;
    		Duration waitTimeout = Duration.newDuration(200000000, TimeUnit.NANOSECONDS, env);
            while (!myListener.closed && count < 1500) {

                // To avoid spinning here. We can either use a sleep or better a WaitSet.
                try {
                    ws.waitForConditions(waitTimeout);
                } catch (TimeoutException te) {
                    ++count;
                }
                myGC.setTriggerValue(false);

            }
    		// cleanup
    		participant.close();
	    } catch(Exception e) {
            System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
        }
	}
}
