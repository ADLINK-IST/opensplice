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

import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.concurrent.TimeUnit;

import org.omg.dds.core.Duration;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.policy.Durability;
import org.omg.dds.core.policy.Partition;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.core.status.Status;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.domain.DomainParticipantFactory;
import org.omg.dds.sub.DataReader;
import org.omg.dds.sub.DataReaderQos;
import org.omg.dds.sub.Sample;
import org.omg.dds.sub.Subscriber;
import org.omg.dds.sub.SubscriberQos;
import org.omg.dds.topic.Topic;
import org.omg.dds.topic.TopicQos;

import DurabilityData.*;

public class DurabilityDataSubscriber {
	public static void usage() {

		System.err.println("*** ERROR ***");
		System.err
				.println("*** usage : DurabilityDataSubscriber <durability_kind>");
		System.err
				.println("***         . durability_kind = transient | persistent");
	}

	public static void main(String args[]) {

		if (args.length < 1) {
			usage();
		}
		if ((!args[0].equals("transient")) && (!args[0].equals("persistent"))) {
			usage();
		}
		String durability_kind = args[0];
		boolean isPersistent = (args[0].equals("persistent"));
		String partitionName = "Durability example";

		try {
            System.setProperty(
                      ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                      "org.opensplice.dds.core.OsplServiceEnvironment");
            ServiceEnvironment env = ServiceEnvironment
                      .createInstance(DurabilityDataSubscriber.class.getClassLoader());

            DomainParticipantFactory dpf = DomainParticipantFactory.getInstance(env);

            // create Domain Participant
            DomainParticipant participant = dpf.createParticipant();

            // create Topic
            String topic_name = "Java5DurabilityData_Msg";
            if (isPersistent) {
                topic_name = "PersistentJava5DurabilityData_Msg";
            }
            PolicyFactory policyFactory = env.getSPI().getPolicyFactory();
            Durability durability = null;
            if (durability_kind.equals("transient")) {
                durability = policyFactory.Durability().withTransient();
            } else if (durability_kind.equals("persistent")) {
                durability = policyFactory.Durability().withPersistent();
            }
            // create Topic with reliable and durability qos
            Reliability reliable = policyFactory.Reliability().withReliable();
            TopicQos topicQos = participant.getDefaultTopicQos().withPolicies(reliable,durability);
            Collection<Class<? extends Status>> status = new HashSet<Class<? extends Status>>();
            Topic<Msg> topic = participant.createTopic(topic_name, Msg.class,topicQos, null, status);

            // create Subscriber with partition qos
            Partition partition = policyFactory.Partition().withName(partitionName);
            SubscriberQos subQos = participant.getDefaultSubscriberQos().withPolicy(partition);
            Subscriber sub = participant.createSubscriber(subQos);


            DataReaderQos drQos = sub.copyFromTopicQos(sub.getDefaultDataReaderQos(),topic.getQos());
            DataReader<Msg> reader = sub.createDataReader(topic,drQos);

            Duration wait_timeout = Duration.newDuration(40, TimeUnit.SECONDS, env);
            reader.waitForHistoricalData(wait_timeout);

    		System.out.println("=== [Subscriber] Ready ...");

    		boolean closed = false;
            int count = 0;
            while (!closed && count < 1500) { // We dont want the example to run indefinitely
    		    Iterator<Sample<Msg>> samples = reader.take();
                /* Process each Sample*/
                 while (samples.hasNext()) {
                    Sample<Msg> sample = samples.next();
                    Msg msg = sample.getData();
                    if (msg != null) { /* Check if the sample is valid. */
        				System.out.println(msg.content);
        				if (msg.content.compareTo("9") == 0) {
							closed = true;
						}
        			}

    			}
                try {
    				Thread.sleep(200);
    			}
    			catch(InterruptedException ie) {
    				// nothing to do
    			}
    			++count;
    		}
            // clean up
            participant.close();
		} catch(Exception e) {
            System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
        }

	}
}
