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

import java.io.IOException;
import java.util.Collection;
import java.util.HashSet;

import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.policy.Durability;
import org.omg.dds.core.policy.Partition;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.core.policy.WriterDataLifecycle;
import org.omg.dds.core.status.Status;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.domain.DomainParticipantFactory;
import org.omg.dds.pub.DataWriter;
import org.omg.dds.pub.DataWriterQos;
import org.omg.dds.pub.Publisher;
import org.omg.dds.pub.PublisherQos;
import org.omg.dds.topic.Topic;
import org.omg.dds.topic.TopicQos;

import DurabilityData.Msg;

public class DurabilityDataPublisher {

	public static void usage() {
		System.err.println("*** ERROR");
		System.err
				.println("*** usage : DurabilityDataPublisher <durability_kind> <autodispose_flag> <automatic_flag>");
		System.err
				.println("***         . durability_kind = transient | persistent");
		System.err.println("***         . autodispose_flag = false | true");
		System.err.println("***         . automatic_flag = false | true");
		System.exit(0);
	}

	public static void main(String args[]) {

		if (args.length < 3) {
			usage();
		}
		if ((!args[0].equals("transient") && !args[0].equals("persistent"))
				|| (!args[1].equals("false") && !args[1].equals("true"))
				|| (!args[2].equals("false") && !args[2].equals("true"))) {
			usage();
		}
		String durability_kind = args[0];
		boolean autodispose_unregistered_instances = (args[1].equals("true"));
		boolean automatic = (args[2].equals("true"));
		boolean isPersistent = (args[0].equals("persistent"));
		String partitionName = "Durability example";

		try {
            System.setProperty(
                      ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                      "org.opensplice.dds.core.OsplServiceEnvironment");
            ServiceEnvironment env = ServiceEnvironment
                      .createInstance(DurabilityDataPublisher.class.getClassLoader());

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


            // create Publisher with partition qos
            Partition partition = policyFactory.Partition().withName(partitionName);
            PublisherQos pubQos = participant.getDefaultPublisherQos().withPolicy(partition);
            Publisher pub = participant.createPublisher(pubQos);

    		// create DataWriter with autDisposeUnregisteredInstances qos
            WriterDataLifecycle wdlq = policyFactory.WriterDataLifecycle().withAutDisposeUnregisteredInstances(autodispose_unregistered_instances);
            DataWriterQos dwQos = pub.copyFromTopicQos(pub.getDefaultDataWriterQos().withPolicy(wdlq),topic.getQos());
            DataWriter<Msg> writer = pub.createDataWriter(topic,dwQos);

    		Msg msgSample = new Msg();
    		writer.registerInstance(msgSample);
    		for (int i = 0; i < 10; i++) {
    			msgSample.id = i;
    			msgSample.content = "" + i;
    			writer.write(msgSample, InstanceHandle.nilHandle(env));

    		}

    		if (!automatic) {
    			char c = 0;
    			System.out.println("Enter E to exit");
    			while (c != 'E') {
    				try {
    					c = (char) System.in.read();
    					System.in.skip(System.in.available());
    					System.out.println(c);
    				} catch (IOException e) {
    				    System.out.println("Error occured: " + e.getMessage());
    					e.printStackTrace();
    				}
    			}
    		} else {
    		    try {
                    Thread.sleep(30000); // sleep for 30 sec
                }
                catch(InterruptedException ie) {
                    // nothing to do
                }
    		}
            // clean up
            participant.close();
		} catch(Exception e) {
		    System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
		}
	}
}
