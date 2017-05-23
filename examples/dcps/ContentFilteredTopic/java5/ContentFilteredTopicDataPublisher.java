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
import java.util.Random;

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

import StockMarket.Stock;

public class ContentFilteredTopicDataPublisher {

	public static void main(String[] args) {
	    try {
    	    System.setProperty(
    	              ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
    	              "org.opensplice.dds.core.OsplServiceEnvironment");
    	    ServiceEnvironment env = ServiceEnvironment
    	              .createInstance(ContentFilteredTopicDataPublisher.class.getClassLoader());

    		String partitionName = "ContentFilteredTopic example";

    		DomainParticipantFactory dpf = DomainParticipantFactory.getInstance(env);

    		// create Domain Participant
            DomainParticipant participant = dpf.createParticipant();

    		// create Topic with reliable and transient qos
            PolicyFactory policyFactory = env.getSPI().getPolicyFactory();
            Reliability reliable = policyFactory.Reliability().withReliable();
            Durability durability = policyFactory.Durability().withTransient();
            TopicQos topicQos = participant.getDefaultTopicQos().withPolicies(reliable,durability);
            Collection<Class<? extends Status>> status = new HashSet<Class<? extends Status>>();
    		Topic<Stock> topic = participant.createTopic("StockTrackerExclusive", Stock.class,topicQos, null, status);

    		// create Publisher with partition qos
    		Partition partition = policyFactory.Partition().withName(partitionName);
    		PublisherQos pubQos = participant.getDefaultPublisherQos().withPolicy(partition);
    		Publisher pub = participant.createPublisher(pubQos);

    		// create DataWriter with autDisposeUnregisteredInstances to false
    		WriterDataLifecycle wdlq = policyFactory.WriterDataLifecycle().withAutDisposeUnregisteredInstances(false);
    		DataWriterQos dwQos = pub.copyFromTopicQos(pub.getDefaultDataWriterQos().withPolicy(wdlq),topic.getQos());
    		DataWriter<Stock> writer = pub.createDataWriter(topic,dwQos);

    		// Publish Events
    		Stock msft = new Stock();
    		msft.ticker = "MSFT";
    		msft.price = 19.95f;

    		Stock ge = new Stock();
    		ge.ticker = "GE";
    		ge.price = 10.00f;

    		// register the two stock instances
    		InstanceHandle msftHandle = writer.registerInstance(msft);
    		InstanceHandle geHandle = writer.registerInstance(ge);

    		// update data every second
    		for (int x = 0; x < 20; x++) {
    			msft.price = msft.price + (new Random()).nextFloat();
    			writer.write(msft, msftHandle);

    			ge.price = ge.price + (new Random()).nextFloat();
    			writer.write(ge, geHandle);

    			try {
    				Thread.sleep(1000);
    			} catch (InterruptedException e) {
    				e.printStackTrace();
    			}
    		}

    		msft.price = -1.0f; // signal to terminate
    		ge.price = -1.0f; // signal to terminate

    		writer.write(msft, msftHandle);
    		writer.write(ge, geHandle);
    		try {
    			Thread.sleep(1000);
    		} catch (InterruptedException e) {
    			e.printStackTrace();
    		}

    		// clean up
    		writer.dispose(msftHandle, msft);
    		writer.dispose(geHandle, ge);
    		writer.unregisterInstance(msftHandle, msft);
    		writer.unregisterInstance(geHandle, ge);

	    } catch(Exception e) {
	        System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
	    }
	}
}
