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

import LifecycleData.*;


public class LifecycleDataSubscriber {
	static String sSampleState[] = { "READ_SAMPLE_STATE",
			"NOT_READ_SAMPLE_STATE" };
	static String sViewState[] = { "NEW_VIEW_STATE", "NOT_NEW_VIEW_STATE" };
	static String sInstanceState[] = { "ALIVE_INSTANCE_STATE",
			"NOT_ALIVE_DISPOSED_INSTANCE_STATE",
			"NOT_ALIVE_NO_WRITERS_INSTANCE_STATE" };

	static int index(int i) {
		int j = (int) (Math.log10(i) / Math.log10(2));
		return j;
	}

	static String partitionName = "Lifecycle example";

	public static void main(String[] args) {

	    try {
            System.setProperty(
                      ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                      "org.opensplice.dds.core.OsplServiceEnvironment");
            ServiceEnvironment env = ServiceEnvironment
                      .createInstance(LifecycleDataSubscriber.class.getClassLoader());

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

            // create Subscriber with partition qos
            Partition partition = policyFactory.Partition().withName(partitionName);
            SubscriberQos subQos = participant.getDefaultSubscriberQos().withPolicy(partition);
            Subscriber sub = participant.createSubscriber(subQos);

            DataReaderQos drQos = sub.copyFromTopicQos(sub.getDefaultDataReaderQos(),topic.getQos());
            DataReader<Msg> reader = sub.createDataReader(topic, drQos);

    		boolean closed = false;
    		int nbIter = 1;
    		int nbIterMax = 100;
    		List<Sample<Msg>> samples = new ArrayList<Sample<Msg>>();

    		while ((!closed) && (nbIter < nbIterMax)){
    			reader.read(samples);
    			for (Sample<Msg> sample : samples) {
    			    Msg msg = sample.getData();
    			    if (msg != null) {
         				System.out.println('\n' + "Message : " + msg.message);
        				System.out.println("writerStates : " + msg.writerStates);
        				System.out.println("valid_data : 1");
        				System.out.println("sample_state:"+ sSampleState[index(sample.getSampleState().value)]
        						  + "-view_state:" + sViewState[index(sample.getViewState().value)]
        						  + "-instance_state:" + sInstanceState[index(sample.getInstanceState().value)]);
        				try {
        					Thread.sleep(200);
        				} catch (InterruptedException e) {
        					e.printStackTrace();
        				}
        				closed = (msg.writerStates.equals("STOPPING_SUBSCRIBER"));
    			    }
    			}
    			try {
    				Thread.sleep(20);
    			} catch (InterruptedException e) {
    				e.printStackTrace();
    			}
    			nbIter++;
    		}
    		System.out.println("=== [Subscriber] stopping after " + nbIter + "iterations - closed = " + closed);
    		if ( nbIter == nbIterMax) System.out.println("*** Error : max " + nbIterMax +   "iterations reached");

    		// cleanup
    		participant.close();
	    } catch(Exception e) {
            System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
        }
	}
}
