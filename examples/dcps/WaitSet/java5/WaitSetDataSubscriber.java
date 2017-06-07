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
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.omg.dds.core.Condition;
import org.omg.dds.core.Duration;
import org.omg.dds.core.GuardCondition;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.StatusCondition;
import org.omg.dds.core.WaitSet;
import org.omg.dds.core.policy.Durability;
import org.omg.dds.core.policy.History;
import org.omg.dds.core.policy.Partition;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.core.status.LivelinessChangedStatus;
import org.omg.dds.core.status.Status;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.domain.DomainParticipantFactory;
import org.omg.dds.sub.DataReader;
import org.omg.dds.sub.DataReaderQos;
import org.omg.dds.sub.InstanceState;
import org.omg.dds.sub.QueryCondition;
import org.omg.dds.sub.ReadCondition;
import org.omg.dds.sub.Sample;
import org.omg.dds.sub.SampleState;
import org.omg.dds.sub.Subscriber;
import org.omg.dds.sub.SubscriberQos;
import org.omg.dds.sub.Subscriber.DataState;
import org.omg.dds.sub.ViewState;
import org.omg.dds.topic.Topic;
import org.omg.dds.topic.TopicQos;

import WaitSetData.*;

public class WaitSetDataSubscriber {


	@SuppressWarnings("unchecked")
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

    		Duration wait_timeout = Duration.newDuration(20, TimeUnit.SECONDS, env);

    		 // create Subscriber with partition qos
            Partition partition = policyFactory.Partition().withName(partitionName);
            SubscriberQos subQos = participant.getDefaultSubscriberQos().withPolicy(partition);
            Subscriber sub = participant.createSubscriber(subQos);

            // create dataReader
            DataReaderQos drQos = sub.copyFromTopicQos(sub.getDefaultDataReaderQos(),topic.getQos());
            DataReader<Msg> reader = sub.createDataReader(topic,drQos);


    		/* 1- Create a ReadCondition that will contain new Msg only */
            DataState readDs = sub.createDataState();
            readDs = readDs.with(InstanceState.ALIVE)
                   .with(ViewState.NEW)
                   .with(SampleState.NOT_READ);
    		ReadCondition<Msg> newMsg = reader.createReadCondition(readDs);

    		/* 2- Create QueryCondition */
    		// create a query string
    		List<String> params = new ArrayList<String>();
            params.add("Hello again");
            String expr = "message=%0";


    		// Create QueryCondition
    		System.out.println("=== [WaitSetDataSubscriber] Query : message = "
    				+ '"' + "Hello again" + '"');
    		DataState queryDs = sub.createDataState();
    		queryDs = queryDs.withAnyInstanceState()
                   .withAnySampleState()
                   .withAnyViewState();
    		QueryCondition<Msg> queryCond = reader.createQueryCondition(queryDs,expr,params);
    		/*
    		 * 3- Obtain a StatusCondition associated to a Writer and that triggers
    		 * only when the Writer changes Liveliness
    		 */
    		StatusCondition<DataReader<Msg>> leftMsgWriter = reader.getStatusCondition();
    		leftMsgWriter.setEnabledStatuses(LivelinessChangedStatus.class);

    		/* 4- Create a GuardCondition which will be used to close the subscriber */
    	    GuardCondition escape = GuardCondition.newGuardCondition(env);

    		/*
    		 * Create a waitset and add the 4 Conditions created above :
    		 * ReadCondition, QueryCondition, StatusCondition, GuardCondition
    		 */
    		WaitSet newMsgWS = WaitSet.newWaitSet(env);
    		newMsgWS.attachCondition(newMsg); // ReadCondition
    		newMsgWS.attachCondition(queryCond); // QueryCondition
    		newMsgWS.attachCondition(leftMsgWriter); // StatusCondition
    		newMsgWS.attachCondition(escape); // GuardCondition

    		HashSet<Condition> triggeredConditions = new HashSet<Condition>();

    		System.out.println("=== [WaitSetDataSubscriber] Ready ...");

    		// var used to manage the status condition
    		int prevCount = 0;
    		LivelinessChangedStatus livChangStatus;

    		boolean closed = false;
    		boolean escaped = false;
    		boolean writerLeft = false;
    		int count = 0;
    		while (!closed && count < 20 ) {
    			/*
    			 * Wait until at least one of the Conditions in the waitset
    			 * triggers.
    			 */
    		    try {
        			newMsgWS.waitForConditions(triggeredConditions, wait_timeout);
        			for (Condition cond : triggeredConditions) {
        			    /* check condition and do an action */
        			    if (cond == newMsg) {
        			        /* The newMsg ReadCondition contains data */
        			        List<Sample<Msg>> samples = new ArrayList<Sample<Msg>>();
        	                reader.select().dataState(readDs).take(samples);
        	                for (Sample<Msg> sample : samples) {
        	                    Msg msg = sample.getData();
        	                    if (msg != null) { /* Check if the sample is valid. */
            	               		System.out.println("    --- New message received ---");
            						System.out.println("    userID  : "+ msg.userID);
            						System.out.println("    Message : \""+ msg.message + "\"");
        	                    }
        					}
        				} else if (cond == queryCond) {
        					/* The queryCond QueryCondition contains data */
        				    List<Sample<Msg>> samples = new ArrayList<Sample<Msg>>();
                            reader.select().dataState(queryDs).Content(expr,params).take(samples);
                            for (Sample<Msg> sample : samples) {
                                Msg msg = sample.getData();
                                if (msg != null) { /* Check if the sample is valid. */
                                    System.out.println("    --- message received (with QueryCOndition on message field) ---");
                                    System.out.println("    userID  : "+ msg.userID);
                                    System.out.println("    Message : \""+ msg.message + "\"");
                                }
                            }
        				} else if (cond == leftMsgWriter) {
        					/*
        					 * Some liveliness has changed (either a DataWriter joined
        					 * or a DataWriter left)
        					 */
        				    livChangStatus = reader.getLivelinessChangedStatus();
            				if (livChangStatus.getAliveCount() < prevCount) {
        						/* a DataWriter left meaning its lost its liveliness to the reader*/
        						System.out.println("!!! a MsgWriter lost its liveliness");
        						System.out.println("=== Triggering escape condition");
                                escape.setTriggerValue(true);
                                writerLeft = true;
        					} else {
        						/* a DataWriter joined */
        						System.out.println("!!! a MsgWriter joined");
        					}
        					prevCount = livChangStatus.getAliveCount();
        				} else if (cond == escape) {
        					// SubscriberUsingWaitset terminated.
        					System.out.println("!!! escape condition triggered - count = " + count);
        					escaped = true;
        					escape.setTriggerValue(false);
        				} else {
        					assert (false); // error
        				}
        			}
    		    } catch (TimeoutException te) {
    		        System.out.println("!!! [INFO] WaitSet timedout - count = " + count);
    		    }
    			++count;
    			closed = escaped && writerLeft;
    		} /* while (!closed) */
    		if (count >= 20) {
    		    System.out.println("*** Error : Timed out - count = " + count + " ***");
    		}

    		/* Remove all Conditions from the WaitSetData. */
    		newMsgWS.detachCondition(newMsg);
    		newMsgWS.detachCondition(escape);
    		newMsgWS.detachCondition(leftMsgWriter);
    		newMsgWS.detachCondition(queryCond);
    		System.out.println("=== [Subscriber] Closed");
    		// cleanup
    		participant.close();
    	} catch(Exception e) {
            System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
        }
	}
}
