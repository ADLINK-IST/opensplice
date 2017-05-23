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
import org.omg.dds.core.policy.History;
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

import WaitSetData.*;

public class WaitSetDataPublisher {

	public static void main(String args[]) {
	    try {
    		// create domain participant
    		String partitionName = "WaitSet example";
    		System.setProperty(
                    ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                    "org.opensplice.dds.core.OsplServiceEnvironment");
            ServiceEnvironment env = ServiceEnvironment
                    .createInstance(WaitSetDataPublisher.class.getClassLoader());

            DomainParticipantFactory dpf = DomainParticipantFactory.getInstance(env);

            // create Domain Participant
            DomainParticipant participant = dpf.createParticipant();

            // create Topic with reliable, transient and ownership qos
            PolicyFactory policyFactory = env.getSPI().getPolicyFactory();
            Reliability reliable = policyFactory.Reliability().withReliable();
            Durability durability = policyFactory.Durability().withTransient();
            History history = policyFactory.History().withKeepLast(2);
            TopicQos topicQos = participant.getDefaultTopicQos().withPolicies(reliable,durability,history);
            Collection<Class<? extends Status>> status = new HashSet<Class<? extends Status>>();
            Topic<Msg> topic = participant.createTopic("WaitSetData_Msg", Msg.class,topicQos, null, status);

            // create Publisher with partition qos
            Partition partition = policyFactory.Partition().withName(partitionName);
            PublisherQos pubQos = participant.getDefaultPublisherQos().withPolicy(partition);
            Publisher pub = participant.createPublisher(pubQos);

            // create DataWriter with autDisposeUnregisteredInstances set to false
            WriterDataLifecycle wdlq = policyFactory.WriterDataLifecycle().withAutDisposeUnregisteredInstances(false);
            DataWriterQos dwQos = pub.copyFromTopicQos(pub.getDefaultDataWriterQos().withPolicies(wdlq),topic.getQos());
            DataWriter<Msg> writer = pub.createDataWriter(topic,dwQos);

    		Msg msgInstance = new Msg(); /* Example on Stack */
    		msgInstance.userID = 1;
    		msgInstance.message = "First Hello";
    		System.out.println("=== [Publisher] writing a message containing :");
    		System.out.println("    userID  : " + msgInstance.userID);
    		System.out.println("    Message : \"" + msgInstance.message + "\"");

    		writer.write(msgInstance, InstanceHandle.nilHandle(env));
    		// Sleep(500ms);
    		try {
    			Thread.sleep(500);// sleep for 500 Ms
    		} catch (InterruptedException ie) {
    			// ignore
    		}
    		// Write a second message
    		msgInstance.message = "Hello again";
    		writer.write(msgInstance, InstanceHandle.nilHandle(env));

    		System.out.println("=== [Publisher] writing a message containing :");
    		System.out.println("    userID  : " + msgInstance.userID);
    		System.out.println("    Message : \"" + msgInstance.message + "\"");
    		// Sleep(500ms);
    		try {
    			Thread.sleep(500);// sleep for 500 Ms
    		} catch (InterruptedException ie) {
    			// ignore
    		}
    		participant.close();
    	} catch(Exception e) {
            System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
        }
	}
}