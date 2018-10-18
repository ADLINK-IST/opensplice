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

package chatroom;

import java.util.Collection;
import java.util.HashSet;

import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.policy.Durability;
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
import org.omg.dds.topic.Topic;
import org.omg.dds.topic.TopicQos;

import Chat.ChatMessage;
import Chat.NameService;

public class Chatter
{
    public static final int NUM_MSG = 10;
    public static final int TERMINATION_MESSAGE = -1;

    public static void main(String[] args)
    {
        int ownID = 1;
        String chatterName = null;
        /* Options: Chatter [ownID [name]] */
        if (args.length > 0) {
            ownID = Integer.parseInt(args[0]);
            if (args.length > 1) {
                chatterName = args[1];
            }
        }

        try {
            /* The service environment is created for the application. */
            System.setProperty(ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                    "org.opensplice.dds.core.OsplServiceEnvironment");

            ServiceEnvironment env = ServiceEnvironment.createInstance(Chatter.class.getClassLoader());

            /* A DomainParticipant is created for the default domain. */
            DomainParticipantFactory dpf = DomainParticipantFactory.getInstance(env);
            DomainParticipant participant = dpf.createParticipant();

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
             * Create a Publisher, a ChatMessageDataWriter and a
             * NameServiceDataWriter whose QoS settings match those of the Topic.
             */
            Partition chatroom = policyFactory.Partition().withName("ChatRoom");
            PublisherQos chatPartitionQos = participant.getDefaultPublisherQos().withPolicy(chatroom);
            Publisher publisher = participant.createPublisher(chatPartitionQos);

            DataWriterQos dwQos = publisher.copyFromTopicQos(publisher.getDefaultDataWriterQos(), reliableTopicQos);
            dwQos = dwQos.withPolicy(policyFactory.WriterDataLifecycle().withAutDisposeUnregisteredInstances(false));
            DataWriter<ChatMessage> chatMessageDataWriter = publisher.createDataWriter(chatMessageTopic,dwQos);

            dwQos = publisher.copyFromTopicQos(publisher.getDefaultDataWriterQos(), transientTopicQos);
            DataWriter<NameService> nameServiceDataWriter = publisher.createDataWriter(nameServiceTopic,dwQos);

            /* Initialize the NameService attributes. */
            NameService ns = new NameService();
            ns.userID = ownID;
            if (chatterName != null) {
                ns.name = chatterName;
            } else {
                ns.name = "Chatter " + ownID;
            }

            /*
             * Register the NameService message implicitly and write it into OpenSplice
             */
            nameServiceDataWriter.write(ns);

            ChatMessage msg = new ChatMessage();
            /* Initialize the chat messages. */
            msg.userID = ownID;
            msg.index = 0;
            if (ownID == TERMINATION_MESSAGE) {
                msg.content = "Termination message.";
            } else {
                msg.content = "Hi there, I will send you " + NUM_MSG + " messages.";
            }
            System.out.println("Writing message: \"" + msg.content + "\"");

            /*
             * Register the ChatMessage explicitly and write the opening
             * message into OpenSplice.
             */
            InstanceHandle ih = chatMessageDataWriter.registerInstance(msg);
            chatMessageDataWriter.write(msg,ih);

            /* Write any number of messages . */
            for (int i = 1; i <= NUM_MSG && ownID != TERMINATION_MESSAGE; i++) {
                try {
                    Thread.sleep (1000); /* do not run so fast! */
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                msg.index = i;
                msg.content = "Message no. " + i;
                System.out.println("Writing message: \"" + msg.content + "\"");
                chatMessageDataWriter.write(msg,ih);
            }

            /*
             * Unregister the instance. The autodispose_unregistered_instances
             * QoS is true, so this will cause the instance to be disposed as
             * well, so no need to do this explicitly here. */
            chatMessageDataWriter.unregisterInstance(ih);
        } catch (Exception e) {
            System.out.println ("An error occured");
            e.printStackTrace();
        }
        System.out.println("Completed chatter");
    }
}
