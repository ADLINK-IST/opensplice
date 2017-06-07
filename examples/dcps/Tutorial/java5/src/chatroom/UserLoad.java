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

package chatroom;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.concurrent.TimeoutException;

import org.omg.dds.core.Condition;
import org.omg.dds.core.DDSException;
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
import org.omg.dds.sub.Sample;
import org.omg.dds.sub.SampleState;
import org.omg.dds.sub.Subscriber;
import org.omg.dds.sub.Subscriber.DataState;
import org.omg.dds.sub.SubscriberQos;
import org.omg.dds.topic.Topic;
import org.omg.dds.topic.TopicQos;

import Chat.ChatMessage;
import Chat.NameService;

public class UserLoad extends Thread {

    /* entities required by all threads. */
    public static GuardCondition escape;
    public static final int TERMINATION_MESSAGE = -1;

    /**
     * Sleeper thread: sleeps 60 seconds and then triggers the WaitSet.
     */
    @Override
    public void run() {
        try {
            if(!isInterrupted()) {
                sleep(60000);
                escape.setTriggerValue(true);
            }
        } catch (InterruptedException ignore) { /* Do nothing */ }
    }

    @SuppressWarnings("unchecked")
    public static void main(String[] args) {
        /* The service environment is created for the application. */
        System.setProperty(ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                "org.opensplice.dds.core.OsplServiceEnvironment");

        ServiceEnvironment env = ServiceEnvironment.createInstance(UserLoad.class.getClassLoader());

        /* A DomainParticipant is created for the default domain. */
        DomainParticipantFactory dpf = DomainParticipantFactory.getInstance(env);
        DomainParticipant participant = null;

        try {
            participant = dpf.createParticipant();

            /*
             * A TopicQos is created with Reliability set to Reliable to
             * guarantee delivery.
             */
            PolicyFactory policyFactory = env.getSPI().getPolicyFactory();
            Reliability reliable = policyFactory.Reliability().withReliable();
            TopicQos reliableTopicQos = participant.getDefaultTopicQos().withPolicy(reliable);
            /* Set the Reliable TopicQos as the new default */
            participant.setDefaultTopicQos(reliableTopicQos);

            /*
             * A Topic is created for the Chat_ChatMessage sample type on the
             * domain participant.
             */
            Collection<Class<? extends Status>> status = new HashSet<Class<? extends Status>>();

            Topic<ChatMessage> chatMessageTopic = participant.createTopic("Chat_ChatMessage", ChatMessage.class, reliableTopicQos, null, status);

            /*
             * A TopicQos is created with Durability set to Transient to
             * ensure that if a subscriber joins after the sample is written then DDS
             * will still retain the sample for it.
             */
            Durability durability = policyFactory.Durability().withTransient();
            TopicQos transientTopicQos = participant.getDefaultTopicQos().withPolicy(durability);

            /*
             * A Topic is created for the Chat_NameService sample type on the
             * domain participant.
             */
            Topic<NameService> nameServiceTopic = participant.createTopic("Chat_NameService", NameService.class, transientTopicQos, null, status);

            /*
             * The subscriber is set to use the ChatRoom partition.
             */
            Partition chatroom = policyFactory.Partition().withName("ChatRoom");
            SubscriberQos chatPartitionQos = participant.getDefaultSubscriberQos().withPolicy(chatroom);

            Subscriber subscriber = participant.createSubscriber(chatPartitionQos);
            DataReaderQos drQos = subscriber.copyFromTopicQos(subscriber.getDefaultDataReaderQos(), transientTopicQos);
            DataReader<NameService> nameServiceDataReader = subscriber.createDataReader(nameServiceTopic,drQos);

            drQos = subscriber.copyFromTopicQos(subscriber.getDefaultDataReaderQos(), reliableTopicQos);
            History history = policyFactory.History().withKeepAll();
            drQos = drQos.withPolicy(history);
            DataReader<ChatMessage> chatMessageDataReader = subscriber.createDataReader(chatMessageTopic,drQos);

            StatusCondition<DataReader<NameService>>  st = nameServiceDataReader.getStatusCondition();
            st.setEnabledStatuses(LivelinessChangedStatus.class);

            escape = GuardCondition.newGuardCondition(env);

            WaitSet ws = WaitSet.newWaitSet(env);
            ws.attachCondition(st);
            ws.attachCondition(escape);

            Duration waitTimeout = Duration.infiniteDuration(env);

            /* Start the sleeper thread. */
            Thread sleeper = new UserLoad();
            sleeper.start();

            boolean closed = false;
            HashSet<Condition> triggeredConditions = new HashSet<Condition>();
            int prevAliveCount =0;

            System.out.println(
                    "UserLoad has opened: disconnect a Chatter " +
                    "with userID = " + TERMINATION_MESSAGE + " to close it....\n");

            while (!closed) {
                try {
                    ws.waitForConditions(triggeredConditions,waitTimeout);
                } catch (TimeoutException e) {
                    e.printStackTrace();
                    closed = true;
                }
                for (Condition c : triggeredConditions) {

                    if (c == escape) {
                        closed = true;
                        System.out.println("Timer (60s) expired. Exiting...");
                    } else if (c == st) {
                        try {
                            LivelinessChangedStatus lcstatus = nameServiceDataReader.getLivelinessChangedStatus();
                            int currentAliveCount = lcstatus.getAliveCount();
                            if (prevAliveCount > currentAliveCount) {
                                /* user left */
                                DataState ds = subscriber.createDataState();
                                ds = ds.withAnySampleState().withAnyViewState().withNotAliveInstanceStates();
                                List<Sample<NameService>> samples = new ArrayList<Sample<NameService>>();
                                nameServiceDataReader.select().dataState(ds).take(samples);

                                for (Sample<NameService> sample : samples) {
                                    NameService ns = sample.getData();
                                    if (ns != null) { /* Check if the sample is valid. */
                                        List<Sample<ChatMessage>> cmSamples = new ArrayList<Sample<ChatMessage>>();
                                        chatMessageDataReader.read(cmSamples);
                                        /* Display the user and his history */
                                        System.out.println(
                                            "Departed user " + ns.name +
                                            " has sent " + cmSamples.size() +
                                            " messages.");
                                        if(ns.userID == TERMINATION_MESSAGE) {
                                            System.out.println("Termination message received: exiting...");
                                            closed = true;
                                            sleeper.interrupt();
                                        }
                                    }
                                }
                            } else {
                                /* user joined */
                                DataState ds = subscriber.createDataState();
                                ds = ds.with(SampleState.NOT_READ).withAnyViewState().with(InstanceState.ALIVE);
                                List<Sample<NameService>> samples = new ArrayList<Sample<NameService>>();
                                nameServiceDataReader.select().dataState(ds).read(samples);

                                /* Process each Sample and print its name and production time. */
                                for (Sample<NameService> sample : samples) {
                                    NameService ns = sample.getData();
                                    if (ns != null) { /* Check if the sample is valid. */
                                       System.out.println("New user: " + ns.name);
                                    }
                                }
                            }
                            prevAliveCount = currentAliveCount;
                        } catch(Exception e) {
                            closed = true;
                        }
                    } else {
                        /* unknown condtion */
                    }
                }
            }
        } catch (DDSException e) {
            System.out.println("Error occurred. Terminating UserLoad (" + e.getMessage() + ")");
        }

        System.out.println("UserLoad has terminated.");
    }
}
