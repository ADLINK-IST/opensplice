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

import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.policy.DestinationOrder;
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
import org.omg.dds.topic.ContentFilteredTopic;
import org.omg.dds.topic.Topic;
import org.omg.dds.topic.TopicQos;

import StockMarket.Stock;

public class ContentFilteredTopicDataSubscriber {

	public static void main(String[] args) {
		String stkToSubscribe = null;
		if (args.length > 0) {
			stkToSubscribe = args[0];
		}

		String partitionName = "ContentFilteredTopic example";

		try {
            System.setProperty(
                      ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                      "org.opensplice.dds.core.OsplServiceEnvironment");
            ServiceEnvironment env = ServiceEnvironment
                      .createInstance(ContentFilteredTopicDataSubscriber.class.getClassLoader());

            DomainParticipantFactory dpf = DomainParticipantFactory.getInstance(env);

            // create Domain Participant
            DomainParticipant participant = dpf.createParticipant();

            // create Topic with reliable and transient qos
            PolicyFactory policyFactory = env.getSPI().getPolicyFactory();
            Reliability reliable = policyFactory.Reliability().withReliable();
            Durability durability = policyFactory.Durability().withTransient();
            DestinationOrder des = policyFactory.DestinationOrder().withSourceTimestamp();
            TopicQos topicQos = participant.getDefaultTopicQos().withPolicies(reliable,durability,des);
            Collection<Class<? extends Status>> status = new HashSet<Class<? extends Status>>();
            Topic<Stock> topic = participant.createTopic("StockTrackerExclusive", Stock.class,topicQos, null, status);

            // create Subscriber with partition qos
            Partition partition = policyFactory.Partition().withName(partitionName);
            SubscriberQos subQos = participant.getDefaultSubscriberQos().withPolicy(partition);
            Subscriber sub = participant.createSubscriber(subQos);

    		String parameters[] = new String[0];
    		DataReader<Stock> reader = null;
    		ContentFilteredTopic<Stock> cfTopic = null;
    		DataReaderQos drQos = sub.copyFromTopicQos(sub.getDefaultDataReaderQos(),topic.getQos());
    		if (stkToSubscribe == null) {
    			// Subscribe to all stocks

                reader = sub.createDataReader(topic,drQos);
    		} else {
    			// create Content Filtered Topic
    			String sqlExpr = "ticker = '" + stkToSubscribe + "'";
    			cfTopic = participant.createContentFilteredTopic("MyStockTopic",topic, sqlExpr, parameters);

    			// create Filtered DataReader
    			reader = sub.createDataReader(cfTopic,drQos);
    		}

    		// Read Events
    		boolean terminate = false;
    		int count = 0;
    		System.out.println("Ready");
    		while (!terminate && count < 1500) { // We dont want the example to run indefinitely
    		    Iterator<Sample<Stock>> samples = reader.take();
    		    /* Process each Sample*/
    		     while (samples.hasNext()) {
    		        Sample<Stock> sample = samples.next();
    		        Stock stock = sample.getData();
    		        if (stock != null) { /* Check if the sample is valid. */
    		            if (stock.price == -1.0f) {
                            terminate = true;
                            break;
                        }
    		            System.out.println(stock.ticker + ": "
                                + stock.price);
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
    		System.out.println("Market Closed");
            // clean up
            participant.close();
		} catch(Exception e) {
		    System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
		}

	}
}
