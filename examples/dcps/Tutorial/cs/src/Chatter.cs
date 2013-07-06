/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
using System;
using System.Threading;
using DDS;
using DDS.OpenSplice;
using Chat;

namespace Chatroom
{
    class Chatter
    {
        public const int NUM_MSG = 10;
        public const int TERMINATION_MESSAGE = -1;

        static void Main(string[] args)
        {
            int ownID = 1;
            string chatterName = null;
            int domain = DDS.DomainId.Default;
            string partitionName = "ChatRoom";

            /* Options: Chatter [ownID [name]] */
            if (args.Length > 0)
            {
                ownID = int.Parse(args[0]);
                if (args.Length > 1)
                {
                    chatterName = args[1];
                }
            }
                 
            /* Create a DomainParticipantFactory and a DomainParticipant
               (using Default QoS settings. */
            DomainParticipantFactory dpf = DomainParticipantFactory.Instance;
            ErrorHandler.checkHandle(dpf, "DDS.DomainParticipantFactory.Instance");

            IDomainParticipant participant = dpf.CreateParticipant(domain, null, StatusKind.Any);
            ErrorHandler.checkHandle(participant, "DDS.DomainParticipantFactory.CreateParticipant");

            /* Register the required datatype for ChatMessage. */
            ChatMessageTypeSupport chatMessageTS = new ChatMessageTypeSupport();
            string chatMessageTypeName = chatMessageTS.TypeName;
            ReturnCode status = chatMessageTS.RegisterType(
                participant, chatMessageTypeName);
            ErrorHandler.checkStatus(
                status, "Chat.ChatMessageTypeSupport.RegisterType");

            /* Register the required datatype for NameService. */
            NameServiceTypeSupport nameServiceTS = new NameServiceTypeSupport();
            string nameServiceTypeName = nameServiceTS.TypeName;
            status = nameServiceTS.RegisterType(
                participant, nameServiceTypeName);
            ErrorHandler.checkStatus(
                status, "Chat.NameServiceTypeSupport.RegisterType");

            /* Initialise Qos variables */
            TopicQos reliableTopicQos = new TopicQos();
            TopicQos settingTopicQos = new TopicQos();
            PublisherQos pubQos = new PublisherQos();
            DataWriterQos dwQos = new DataWriterQos();
            DataWriterQos nsDwQos = new DataWriterQos();

            /* Set the ReliabilityQosPolicy to RELIABLE. */
            status = participant.GetDefaultTopicQos(ref reliableTopicQos);
            ErrorHandler.checkStatus(
                status, "DDS.DomainParticipant.GetDefaultTopicQos");
            reliableTopicQos.Reliability.Kind = ReliabilityQosPolicyKind.ReliableReliabilityQos;

            /* Make the tailored QoS the new default. */
            status = participant.SetDefaultTopicQos(reliableTopicQos);
            ErrorHandler.checkStatus(
                status, "DDS.DomainParticipant.SetDefaultTopicQos");

            /* Use the changed policy when defining the ChatMessage topic */
            ITopic chatMessageTopic = participant.CreateTopic(
                "Chat_ChatMessage",
                chatMessageTypeName,
                reliableTopicQos);
            ErrorHandler.checkHandle(
                chatMessageTopic,
                "DDS.DomainParticipant.CreateTopic (ChatMessage)");

            /* Set the DurabilityQosPolicy to TRANSIENT. */
            status = participant.GetDefaultTopicQos(ref settingTopicQos);
            ErrorHandler.checkStatus(
                status, "DDS.DomainParticipant.GetDefaultTopicQos");
            settingTopicQos.Durability.Kind = DurabilityQosPolicyKind.TransientDurabilityQos;

            /* Create the NameService Topic. */
            ITopic nameServiceTopic = participant.CreateTopic(
                "Chat_NameService",
                nameServiceTypeName,
                settingTopicQos);
            ErrorHandler.checkHandle(
                nameServiceTopic,
                "DDS.DomainParticipant.CreateTopic (NameService)");

            /* Adapt the default PublisherQos to write into the
               "ChatRoom" Partition. */
            status = participant.GetDefaultPublisherQos(ref pubQos);
            ErrorHandler.checkStatus(
                status, "DDS.DomainParticipant.GetDefaultPublisherQos");
            pubQos.Partition.Name = new string[1];
            pubQos.Partition.Name[0] = partitionName;

            /* Create a Publisher for the chatter application. */
            IPublisher chatPublisher = participant.CreatePublisher(pubQos);
            ErrorHandler.checkHandle(
                chatPublisher, "DDS.DomainParticipant.CreatePublisher");

            /* Create a DataWriter for the ChatMessage Topic
               (using the appropriate QoS). */
            chatPublisher.GetDefaultDataWriterQos(ref dwQos);
            status = chatPublisher.CopyFromTopicQos(ref dwQos, reliableTopicQos);
            ErrorHandler.checkStatus(status, "DDS.Publisher.CopyFromTopicQos");

            IDataWriter parentWriter = chatPublisher.CreateDataWriter(chatMessageTopic, dwQos);
            ErrorHandler.checkHandle(
                parentWriter, "DDS.Publisher.CreateDatawriter (chatMessage)");

            /* Narrow the abstract parent into its typed representative. */
            ChatMessageDataWriter talker = parentWriter as ChatMessageDataWriter;
            ErrorHandler.checkHandle(
                talker, "Chat.ChatMessageDataWriter");

            /* Create a DataWriter for the NameService Topic
               (using the appropriate QoS). */
            status = chatPublisher.GetDefaultDataWriterQos(ref nsDwQos);
            ErrorHandler.checkStatus(
                status, "DDS.Publisher.GetDefaultDatawriterQos");
            status = chatPublisher.CopyFromTopicQos(ref nsDwQos, settingTopicQos);
            ErrorHandler.checkStatus(status, "DDS.Publisher.CopyFromTopicQos");

            WriterDataLifecycleQosPolicy writerDataLifecycle = nsDwQos.WriterDataLifecycle;
            writerDataLifecycle.AutodisposeUnregisteredInstances = false;
            IDataWriter nsParentWriter = chatPublisher.CreateDataWriter(nameServiceTopic, nsDwQos);
            ErrorHandler.checkHandle(
                nsParentWriter, "DDS.Publisher.CreateDatawriter (NameService)");

            /* Narrow the abstract parent into its typed representative. */
            NameServiceDataWriter nameServer = nsParentWriter as NameServiceDataWriter;
            ErrorHandler.checkHandle(
                nameServer, "Chat.NameServiceDataWriterHelper");

            /* Initialize the NameServer attributes. */
            NameService ns = new NameService();
            ns.userID = ownID;
            if (chatterName != null)
            {
                ns.name = chatterName;
            }
            else
            {
                ns.name = "Chatter " + ownID;
            }

            /* Write the user-information into the system
               (registering the instance implicitly). */
            status = nameServer.Write(ns);
            ErrorHandler.checkStatus(status, "Chat.ChatMessageDataWriter.Write");

            /* Initialize the chat messages. */
            ChatMessage msg = new ChatMessage();
            msg.userID = ownID;
            msg.index = 0;
            if (ownID == TERMINATION_MESSAGE)
            {
                msg.content = "Termination message.";
            }
            else
            {
                msg.content = "Hi there, I will send you " +
                    NUM_MSG + " more messages.";
            }
            System.Console.WriteLine("Writing message: \"" + msg.content + "\"");

            /* Register a chat message for this user
               (pre-allocating resources for it!!) */
            InstanceHandle userHandle = talker.RegisterInstance(msg);

            /* Write a message using the pre-generated instance handle. */
            status = talker.Write(msg, userHandle);
            ErrorHandler.checkStatus(status, "Chat.ChatMessageDataWriter.Write");

            Thread.Sleep(1000);

            /* Write any number of messages . */
            for (int i = 1; i <= NUM_MSG && ownID != TERMINATION_MESSAGE; i++)
            {
                msg.index = i;
                msg.content = "Message no. " + i;
                Console.WriteLine("Writing message: \"" + msg.content + "\"");
                status = talker.Write(msg, userHandle);
                ErrorHandler.checkStatus(status, "Chat.ChatMessageDataWriter.Write");

                Thread.Sleep(1000); /* do not run so fast! */
            }

            /* Leave the room by disposing and unregistering the message instance */
            status = talker.Dispose(msg, userHandle);
            ErrorHandler.checkStatus(
                status, "Chat.ChatMessageDataWriter.Dispose");
            status = talker.UnregisterInstance(msg, userHandle);
            ErrorHandler.checkStatus(
                status, "Chat.ChatMessageDataWriter.unregister_instance");

            /* Also unregister our name. */
            status = nameServer.UnregisterInstance(ns, InstanceHandle.Nil);
            ErrorHandler.checkStatus(
                status, "Chat.NameServiceDataWriter.unregister_instance");

            /* Remove the DataWriters */
            status = chatPublisher.DeleteDataWriter(talker);
            ErrorHandler.checkStatus(
                status, "DDS.Publisher.DeleteDatawriter (talker)");

            status = chatPublisher.DeleteDataWriter(nameServer);
            ErrorHandler.checkStatus(status,
                "DDS.Publisher.DeleteDatawriter (nameServer)");

            /* Remove the Publisher. */
            status = participant.DeletePublisher(chatPublisher);
            ErrorHandler.checkStatus(
                status, "DDS.DomainParticipant.DeletePublisher");

            /* Remove the Topics. */
            status = participant.DeleteTopic(nameServiceTopic);
            ErrorHandler.checkStatus(
                status, "DDS.DomainParticipant.DeleteTopic (nameServiceTopic)");

            status = participant.DeleteTopic(chatMessageTopic);
            ErrorHandler.checkStatus(
                status, "DDS.DomainParticipant.DeleteTopic (chatMessageTopic)");

            /* Remove the DomainParticipant. */
            status = dpf.DeleteParticipant(participant);
            ErrorHandler.checkStatus(
                status, "DDS.DomainParticipantFactory.DeleteParticipant");

        }
    }
}
