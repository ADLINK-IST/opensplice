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
import java.util.Collection;
import java.util.HashSet;
import java.util.List;

import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.policy.Durability;
import org.omg.dds.core.policy.Ownership;
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

import OwnershipData.*;

public class OwnershipDataSubscriber {
    static String partitionName = "Ownership example";

	public static void main(String args[]) {
	    try {
            System.setProperty(
                      ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                      "org.opensplice.dds.core.OsplServiceEnvironment");
            ServiceEnvironment env = ServiceEnvironment
                      .createInstance(OwnershipDataSubscriber.class.getClassLoader());

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

            // create Subscriber with partition qos
            Partition partition = policyFactory.Partition().withName(partitionName);
            SubscriberQos subQos = participant.getDefaultSubscriberQos().withPolicy(partition);
            Subscriber sub = participant.createSubscriber(subQos);

            // create dataReader with the attached listener
            DataReaderQos drQos = sub.copyFromTopicQos(sub.getDefaultDataReaderQos(),topic.getQos());
            DataReader<Stock> reader = sub.createDataReader(topic,drQos);

    		System.out.println("===[Subscriber] Ready ...");
    		System.out.println("   Ticker   Price   Publisher   ownership strength");
    		boolean closed = false;
    		int count = 0;
    		while (!closed && count < 1500) {
    		    List<Sample<Stock>> samples = new ArrayList<Sample<Stock>>();
    	        reader.take(samples);
    	        for (Sample<Stock> sample : samples) {
    	            Stock stock = sample.getData();
                    if (stock != null) {
						if (stock.price < 0.0f) {
							closed = true;
							break;
						}
						System.out.printf("   %s %8.1f    %s        %d\n",
						        stock.ticker,
						        stock.price,
						        stock.publisher,
						        stock.strength);
					}
				}

    			try {
    				Thread.sleep(200);// sleep for 200 ms
    			} catch (InterruptedException ie) {
    			    System.out.println("Sleep interrupted " + ie.getMessage());
    	            ie.printStackTrace();
    			}
    			++count;
    		}
    		System.out.println("===[Subscriber] Market Closed");

    		// cleanup
    		participant.close();
	    } catch(Exception e) {
            System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
        }

	}
}