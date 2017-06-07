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

import LifecycleData.Msg;

public class LifecycleDataPublisher {

	public static void usage() {
		System.out.println("*** ERROR");
		System.out.println("*** usage : LifecyclePublisher <autodispose_flag> <writer_action>");
		System.out.println("***         . autodispose_flag = false | true");
		System.out.println("***         . dispose | unregister | stoppub");
		System.exit(0);
	}

	public static void main(String args[]) {
		System.out.println("=== args_length=" + args.length);
		if (args.length < 2) {
			usage();
		}
		if (!(args[0].equals("false")) && !(args[0].equals("true"))
                    && !(args[1].equals("dispose")) && !(args[1].equals("unregister")) && !(args[1].equals("stoppub")))
		{
		  usage();
		}
		boolean autodispose = (args[0].equals("true"));
		String partitionName = "Lifecycle example";
		try {
            System.setProperty(
                      ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                      "org.opensplice.dds.core.OsplServiceEnvironment");
            ServiceEnvironment env = ServiceEnvironment
                      .createInstance(LifecycleDataPublisher.class.getClassLoader());

            DomainParticipantFactory dpf = DomainParticipantFactory.getInstance(env);

            // create Domain Participant
            DomainParticipant participant = dpf.createParticipant();

            // create Topic with reliable and transient qos
            PolicyFactory policyFactory = env.getSPI().getPolicyFactory();
            Reliability reliable = policyFactory.Reliability().withReliable();
            Durability durability = policyFactory.Durability().withTransient();
            TopicQos topicQos = participant.getDefaultTopicQos().withPolicies(reliable,durability);
            Collection<Class<? extends Status>> status = new HashSet<Class<? extends Status>>();
            Topic<Msg> topic = participant.createTopic("Lifecycle_Msg", Msg.class,topicQos, null, status);

            // create Publisher with partition qos
            Partition partition = policyFactory.Partition().withName(partitionName);
            PublisherQos pubQos = participant.getDefaultPublisherQos().withPolicy(partition);
            Publisher pub = participant.createPublisher(pubQos);

            // create DataWriter with autDisposeUnregisteredInstances qos
            WriterDataLifecycle wdlq = policyFactory.WriterDataLifecycle().withAutDisposeUnregisteredInstances(autodispose);
            DataWriterQos dwQos = pub.copyFromTopicQos(pub.getDefaultDataWriterQos().withPolicy(wdlq),topic.getQos());
            DataWriter<Msg> writer = pub.createDataWriter(topic,dwQos);
            DataWriter<Msg> writerStopper = pub.createDataWriter(topic,dwQos);

    		try {
    			Thread.sleep(500);
    		} catch (InterruptedException e) {
    			e.printStackTrace();
    		}


    		if (args[1].equals("dispose")){
    		    // Send Msg (topic to monitor)
    		    Msg msgInstance = new Msg();
    		    msgInstance.userID = 1;
    		    msgInstance.message = "Lifecycle_1";
    		    msgInstance.writerStates = "SAMPLE_SENT -> INSTANCE_DISPOSED -> DATAWRITER_DELETED";
    		    System.out.println("=== [Publisher]  :");
    		    System.out.println("    userID  : " + msgInstance.userID);
    		    System.out.println("    Message  : " + msgInstance.message);
    		    System.out.println("    writerStates : " + msgInstance.writerStates);
    		    writer.write(msgInstance, InstanceHandle.nilHandle(env));
    		    try {
    			      Thread.sleep(500);
    			} catch (InterruptedException e) {
    			      e.printStackTrace();
    			}
    			System.out.println("=== [Publisher]  : SAMPLE_SENT");

    			// Dispose instance
    			writer.dispose(InstanceHandle.nilHandle(env), msgInstance);
    		    System.out.println("=== [Publisher]  : INSTANCE_DISPOSED");
    		}
    		else if (args[1].equals("unregister")){
    		    // Send Msg (topic to monitor)
    		    Msg msgInstance = new Msg();
    		    msgInstance.userID = 2;
    		    msgInstance.message = "Lifecycle_2";
    		    msgInstance.writerStates = "SAMPLE_SENT -> INSTANCE_UNREGISTERED -> DATAWRITER_DELETED";
    		    System.out.println("=== [Publisher]  :");
    		    System.out.println("    userID  : " + msgInstance.userID);
    		    System.out.println("    Message  : " + msgInstance.message);
    		    System.out.println("    writerStates : " + msgInstance.writerStates);
    		    writer.write(msgInstance, InstanceHandle.nilHandle(env));
    		    try {
    			      Thread.sleep(500);
    			} catch (InterruptedException e) {
    			      e.printStackTrace();
    			}
    		    System.out.println("=== [Publisher]  : SAMPLE_SENT");
    		    // Unregister instance : the auto_dispose_unregisterd_instances flag
    		    // is currently ignored and the instance is never disposed
    		    // automatically
    		    writer.unregisterInstance(InstanceHandle.nilHandle(env), msgInstance);
    		    System.out.println("=== [Publisher]  : INSTANCE_UNREGISTERED");
    		}
    		else if (args[1].equals("stoppub")){
    		    // Send Msg (topic to monitor)
    		    Msg msgInstance = new Msg();
    		    msgInstance.userID = 3;
    		    msgInstance.message = "Lifecycle_3";
    		    msgInstance.writerStates = "SAMPLE_SENT -> DATAWRITER_DELETED";
    		    System.out.println("=== [Publisher]  :");
    		    System.out.println("    userID  : " + msgInstance.userID);
    		    System.out.println("    Message  : " + msgInstance.message);
    		    System.out.println("    writerStates : " + msgInstance.writerStates);
    		    writer.write(msgInstance, InstanceHandle.nilHandle(env));
    		    try {
    			      Thread.sleep(500);
    			} catch (InterruptedException e) {
    			      e.printStackTrace();
    			}
    		    System.out.println("=== [Publisher]  : SAMPLE_SENT");
    		}

    		// let the subscriber treat the previous writer state !!!!
    		System.out
    				.println("=== [Publisher] waiting 500ms to let the subscriber treat the previous write state ...");
    		try {
    			Thread.sleep(500);
    		} catch (InterruptedException e) {
    			e.printStackTrace();
    		}

    		// Clean-up the dataWriter
    		writer.close();
    		try {
    			Thread.sleep(500);
    		} catch (InterruptedException e) {
    			e.printStackTrace();
    		}
    		System.out.println("=== [Publisher]  : DATAWRITER_DELETED");

    		// Stop the subscriber
    		Msg msgInstance = new Msg();
    		msgInstance.userID = 4;
    		msgInstance.message = "Lifecycle_4";
    		msgInstance.writerStates = "STOPPING_SUBSCRIBER";
    		System.out.println("=== [Publisher]  :");
    		System.out.println("    userID  : " + msgInstance.userID);
    		System.out.println("    Message  : " + msgInstance.message);
    		System.out.println("    writerStates : " + msgInstance.writerStates);
    		writerStopper.write(msgInstance, InstanceHandle.nilHandle(env));
    		try {
    		    Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
     		participant.close();

    	} catch(Exception e) {
            System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
        }
	}
}
