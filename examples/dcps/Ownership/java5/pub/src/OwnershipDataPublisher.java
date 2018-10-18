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
import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.policy.Durability;
import org.omg.dds.core.policy.Ownership;
import org.omg.dds.core.policy.OwnershipStrength;
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

import OwnershipData.Stock;

public class OwnershipDataPublisher {
	static String partitionName = "Ownership example";

	public static void main(String args[]) {
	    try {
    		if (args.length < 4) {
    			System.err
    					.println("*** [Publisher] usage : Publisher <publisher_name> <ownership_strength> <nb_iterations> <stop_subscriber_flag>");
    			System.exit(-1);
    		}

    		String publisher_name = args[0];
    		int ownership_strength = Integer.parseInt(args[1]);
    		int nb_iteration = Integer.parseInt(args[2]);
    		boolean stop_subscriber = (Integer.parseInt(args[3]) == 1);

    		System.setProperty(
                    ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                    "org.opensplice.dds.core.OsplServiceEnvironment");
    		ServiceEnvironment env = ServiceEnvironment
    		        .createInstance(OwnershipDataPublisher.class.getClassLoader());

    		DomainParticipantFactory dpf = DomainParticipantFactory.getInstance(env);

    		// create Domain Participant
    		DomainParticipant participant = dpf.createParticipant();

    		// create Topic with reliable, transient and ownership qos
    		PolicyFactory policyFactory = env.getSPI().getPolicyFactory();
    		Reliability reliable = policyFactory.Reliability().withReliable();
    		Durability durability = policyFactory.Durability().withTransient();
    		Ownership ownership = policyFactory.Ownership().withExclusive();
    		TopicQos topicQos = participant.getDefaultTopicQos().withPolicies(reliable,durability,ownership);
    		Collection<Class<? extends Status>> status = new HashSet<Class<? extends Status>>();
    		Topic<Stock> topic = participant.createTopic("OwnershipStockTracker", Stock.class,topicQos, null, status);

    		// create Publisher with partition qos
    		Partition partition = policyFactory.Partition().withName(partitionName);
    		PublisherQos pubQos = participant.getDefaultPublisherQos().withPolicy(partition);
    		Publisher pub = participant.createPublisher(pubQos);

    		// create DataWriter with autDisposeUnregisteredInstances qos
    		WriterDataLifecycle wdlq = policyFactory.WriterDataLifecycle().withAutDisposeUnregisteredInstances(false);
    		OwnershipStrength ownerShipStrength = policyFactory.OwnershipStrength().withValue(ownership_strength);
    		DataWriterQos dwQos = pub.copyFromTopicQos(pub.getDefaultDataWriterQos().withPolicies(wdlq,ownerShipStrength),topic.getQos());
    		DataWriter<Stock> writer = pub.createDataWriter(topic,dwQos);

    		Stock stock = new Stock();
            stock.ticker = "MSFT";
            stock.price = 0;
            stock.publisher = publisher_name;
            stock.strength = ownership_strength;
            InstanceHandle userHandle = writer.registerInstance(stock);

    		// Publisher publishes the prices in dollars
    		System.out.println("=== [Publisher] Publisher " + publisher_name + " with strength : " + ownership_strength);
    		System.out.println(" / sending " + nb_iteration + " prices ..." + " stop_subscriber flag=" + args[3]);
    		// The subscriber should display the prices sent by the publisher with
    		// the highest ownership strength
    		float price = 10.0f;
    		for (int x = 0; x < nb_iteration; x++) {
    			 stock.price = price;
    	         writer.write(stock, userHandle);
    			// Sleep(delay_200ms);
    			try {
    				Thread.sleep(200);// sleep for 200 ms
    			} catch (InterruptedException ie) {
    			}
    			price = price + 0.5f;
    		}
    		if (stop_subscriber) {
    			// send a price = -1 to stop subscriber
    			stock.price = -1.0f;
    			System.out.println("=== Stopping the subscriber");
    			writer.write(stock, userHandle);
    		}
    		// clean up
    		participant.close();
	    } catch(Exception e) {
            System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
        }
	}
}
