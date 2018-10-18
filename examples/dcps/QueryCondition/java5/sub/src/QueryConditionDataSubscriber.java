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

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.omg.dds.core.Duration;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.WaitSet;
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
import org.omg.dds.sub.QueryCondition;
import org.omg.dds.sub.Sample;
import org.omg.dds.sub.Subscriber;
import org.omg.dds.sub.Subscriber.DataState;
import org.omg.dds.sub.SubscriberQos;
import org.omg.dds.topic.Topic;
import org.omg.dds.topic.TopicQos;

import StockMarket.Stock;

public class QueryConditionDataSubscriber {

    public static void main(String[] args) {
        try {
            String stkToSubscribe = null;
            String partitionName = "QueryCondition example";
            if (args.length > 0) {
                stkToSubscribe = args[0];
            } else {
                System.out.println("Invalid Arguments \n");
                System.out.println("Expected argument MSFT or GE");
                System.exit(-1);
            }

            System.setProperty(
                      ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                      "org.opensplice.dds.core.OsplServiceEnvironment");
            ServiceEnvironment env = ServiceEnvironment
                      .createInstance(QueryConditionDataSubscriber.class.getClassLoader());

            DomainParticipantFactory dpf = DomainParticipantFactory.getInstance(env);

            // create Domain Participant
            DomainParticipant participant = dpf.createParticipant();

            // create Topic with reliable, transient and ownership qos
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

            // create dataReader with the attached listener
            DataReaderQos drQos = sub.copyFromTopicQos(sub.getDefaultDataReaderQos(),topic.getQos());
            DataReader<Stock> reader = sub.createDataReader(topic,drQos);

            // Read Events
            List<String> params = new ArrayList<String>();
            params.add(new String(stkToSubscribe));
            String expr = "ticker=%0";
            DataState ds = sub.createDataState();
            ds = ds.withAnyInstanceState()
                   .withAnyViewState()
                   .withAnySampleState();
            QueryCondition<Stock> queryCond = reader.createQueryCondition(ds,expr,params);

            // Create a waitset to wait for data matching our condition
            WaitSet myWS = WaitSet.newWaitSet(env);
            myWS.attachCondition(queryCond);
            Duration waitTimeout = Duration.newDuration(60, TimeUnit.SECONDS, env);

            boolean terminate = false;
            int count = 0;
            System.out.println("Ready");
            while (!terminate && count < 1500) { // We dont want the example to run indefinitely
                /* wait until we got data that matches our condition */
                try {
                    myWS.waitForConditions(waitTimeout);
                } catch(TimeoutException te) {
                    System.out.println("Waitset timeout did you provide a valid stock to filter(MSFT or GE)?");
                }
                List<Sample<Stock>> samples = new ArrayList<Sample<Stock>>();
                reader.select().dataState(ds).Content(expr,params).take(samples);
                for (Sample<Stock> sample : samples) {
                    Stock stock = sample.getData();
                    if (stock != null) { /* Check if the sample is valid. */
                        if (stock.price == -1.0f) {
                            terminate = true;
                            break;
                        }
                        System.out.println(stock.ticker + ": "+ stock.price);
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
