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

import org.omg.dds.core.DDSException;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.policy.Durability;
import org.omg.dds.core.policy.History;
import org.omg.dds.core.policy.Partition;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.core.status.Status;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.domain.DomainParticipantFactory;
import org.omg.dds.pub.DataWriter;
import org.omg.dds.pub.DataWriterQos;
import org.omg.dds.pub.Publisher;
import org.omg.dds.pub.PublisherQos;
import org.omg.dds.sub.DataReader;
import org.omg.dds.sub.DataReaderListener;
import org.omg.dds.sub.DataReaderQos;
import org.omg.dds.sub.Sample;
import org.omg.dds.sub.Subscriber;
import org.omg.dds.sub.SubscriberQos;
import org.omg.dds.topic.ContentFilteredTopic;
import org.omg.dds.topic.Topic;
import org.omg.dds.topic.TopicQos;

import Chat.ChatMessage;
import Chat.NameService;
import Chat.NamedMessage;

public class MessageBoard {

    private static ServiceEnvironment env = null;
    private static DomainParticipantFactory dpf = null;
    private static DomainParticipant participant = null;
    private static PolicyFactory policyFactory = null;
    private static TopicQos reliableTopicQos = null;
    private static Topic<ChatMessage> chatMessageTopic = null;
    private static Topic<NameService> nameServiceTopic = null;
    private static Topic<NamedMessage> namedMessageTopic = null;
    private static Subscriber subscriber = null;
    private static Publisher publisher = null;
    private static DataReader<ChatMessage> chatMessageDataReader = null;
    private static DataReader<NameService> nameServiceDataReader = null;
    private static DataWriter<NamedMessage> namedMessageDataWriter = null;
    private static ContentFilteredTopic<ChatMessage> cfm =null;
    private static boolean terminated = false;
    public static final int TERMINATION_MESSAGE = -1;
    private static String[] parameterList = new String[1];

    public static void create_simulated_multitopic() {
        /*
         * A Topic is created for the Chat_ChatMessage sample type on the
         * domain participant.
         */
        Collection<Class<? extends Status>> status = new HashSet<Class<? extends Status>>();

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
        nameServiceTopic = participant.createTopic("Chat_NameService", NameService.class, transientTopicQos, null, status);

        /*
         * A Topic is created for the Chat_NamedMessage sample type on the
         * domain participant.
         */
        namedMessageTopic = participant.createTopic("Chat_NamedMessage", NamedMessage.class, reliableTopicQos, null, status);

        /* A content-filtered topic is created, filtering out our own ID. */
        List<String> p = new ArrayList<String>();
        p.add(parameterList[0]);
        String filter = "userID <> %0";
        cfm = participant.createContentFilteredTopic("Chat_FilteredMessage", chatMessageTopic, filter, p);

        /* Create a KEEP_ALL datareader for the chat-messages */
        DataReaderQos drQos = subscriber.copyFromTopicQos(subscriber.getDefaultDataReaderQos(), reliableTopicQos);
        History history = policyFactory.History().withKeepAll();
        drQos = drQos.withPolicy(history);
        chatMessageDataReader = subscriber.createDataReader(cfm, drQos);

        /* Create a TRANSIENT datareader for the name-service topic */
        drQos = subscriber.copyFromTopicQos(subscriber.getDefaultDataReaderQos(), transientTopicQos);
        nameServiceDataReader = subscriber.createDataReader(nameServiceTopic, drQos);

        /* Create a publisher and datawriter for the simulated multitopic */
        Partition chatroom = policyFactory.Partition().withName("ChatRoom");
        PublisherQos chatPartitionQos = participant.getDefaultPublisherQos().withPolicy(chatroom);
        publisher = participant.createPublisher(chatPartitionQos);
        DataWriterQos dwQos = publisher.copyFromTopicQos(publisher.getDefaultDataWriterQos(), reliableTopicQos);
        namedMessageDataWriter = publisher.createDataWriter(namedMessageTopic,dwQos);

        /* Create and set the listener for the ChatMessage */
        DataReaderListener<ChatMessage> drl = new DataReaderListenerClass(chatMessageDataReader, nameServiceDataReader, namedMessageDataWriter);
        chatMessageDataReader.setListener(drl);
    }

    public static void main(String[] args) {
        /* Options: MessageBoard [ownID] */
        /* Messages having owner ownID will be ignored */
        if (args.length >0) {
            parameterList[0] = args[0];
        }
        else {
            parameterList[0] = new String("0");
        }

        /* The service environment is created for the application. */
        System.setProperty(ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                "org.opensplice.dds.core.OsplServiceEnvironment");

        env = ServiceEnvironment.createInstance(MessageBoard.class.getClassLoader());

        /* A DomainParticipant is created for the default domain. */
        dpf = DomainParticipantFactory.getInstance(env);

        try {
            participant = dpf.createParticipant();

            /*
             * A TopicQos is created with Reliability set to Reliable to
             * guarantee delivery.
             */
            policyFactory = env.getSPI().getPolicyFactory();
            Reliability reliable = policyFactory.Reliability().withReliable();
            reliableTopicQos = participant.getDefaultTopicQos().withPolicy(reliable);
            /* Set the Reliable TopicQos as the new default */
            participant.setDefaultTopicQos(reliableTopicQos);

            /*
             * A Topic is created for the Chat_ChatMessage sample type on the
             * domain participant.
             */
            Collection<Class<? extends Status>> status = new HashSet<Class<? extends Status>>();

            chatMessageTopic = participant.createTopic("Chat_ChatMessage", ChatMessage.class, reliableTopicQos, null, status);

            /*
             * Create a Subscriber and a NamedMessageDataReader whose
             * QoS Settings comply with that on the topic.
             */
            Partition chatroom = policyFactory.Partition().withName("ChatRoom");
            SubscriberQos chatPartitionQos = participant.getDefaultSubscriberQos().withPolicy(chatroom);
            subscriber = participant.createSubscriber(chatPartitionQos);
            create_simulated_multitopic();

            DataReaderQos drQos = subscriber.copyFromTopicQos(subscriber.getDefaultDataReaderQos(), reliableTopicQos);
            History history = policyFactory.History().withKeepAll();
            drQos = drQos.withPolicy(history);
            DataReader<NamedMessage> namedMessageDataReader = subscriber.createDataReader(namedMessageTopic,drQos);

            System.out.println(
                    "MessageBoard has opened: send a ChatMessage " +
                    "with userID = -1 to close it....\n");

            while (!terminated) {
                /* Note: using read does not remove the samples from unregistered
                 * instances from the DataReader. This means that the DataReader
                 * would use more and more resources. That's why we use take here
                 * instead. */
                List<Sample<NamedMessage>> samples = new ArrayList<Sample<NamedMessage>>();
                namedMessageDataReader.take(samples);
                for (Sample<NamedMessage> sample : samples) {
                    NamedMessage message = sample.getData();
                    if (message != null) {
                        if (message.userID == TERMINATION_MESSAGE) {
                            System.out.println("Termination message received: exiting...");
                            terminated = true;
                        } else {
                            System.out.println(message.userName + " : " + message.content);
                        }
                    }
                }

                /* Sleep for some amount of time, as not to consume too much CPU cycles. */
                try {
                    Thread.sleep(100);
                } catch (InterruptedException ignore) { /* Do nothing */ }
            }
        } catch (DDSException e) {
            System.out.println("Error occurred. Terminating MessageBoard (" + e.getMessage() + ")");
        }
        System.out.println("MessageBoard has terminated.");
    }
}
