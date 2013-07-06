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
using DDS;
using Chat;

namespace Chatroom
{
    class MessageBoard
    {
        public const int TERMINATION_MESSAGE = -1;

        static void Main(string[] args)
        {
            string partitionName = "ChatRoom";
            int domain = DDS.DomainId.Default;

            /* Options: MessageBoard [ownID] */
            /* Messages having owner ownID will be ignored */
            string[] parameterList = new string[1];

            if (args.Length > 0)
            {
                parameterList[0] = args[0];
            }
            else
            {
                parameterList[0] = "0";
            }

            /* Create a DomainParticipantFactory and a DomainParticipant
               (using Default QoS settings. */
            DomainParticipantFactory dpf = DomainParticipantFactory.Instance;
            ErrorHandler.checkHandle(
            dpf, "DDS.DomainParticipantFactory.Instance");

            IDomainParticipant parentDP = dpf.CreateParticipant(
            domain, null, StatusKind.Any);
            ErrorHandler.checkHandle(
                parentDP, "DDS.DomainParticipantFactory.CreateParticipant");

            /* Register the required datatype for ChatMessage. */
            ChatMessageTypeSupport chatMessageTS = new ChatMessageTypeSupport();
            string chatMessageTypeName = chatMessageTS.TypeName;
            ReturnCode status = chatMessageTS.RegisterType(parentDP, chatMessageTypeName);
            ErrorHandler.checkStatus(
                status, "Chat.ChatMessageTypeSupport.RegisterType");

            /* Register the required datatype for NameService. */
            NameServiceTypeSupport nameServiceTS = new NameServiceTypeSupport();
            ErrorHandler.checkHandle(
                nameServiceTS, "new NameServiceTypeSupport");
            string nameServiceTypeName = nameServiceTS.TypeName;
            status = nameServiceTS.RegisterType(parentDP, nameServiceTypeName);
            ErrorHandler.checkStatus(
                status, "Chat.NameServiceTypeSupport.RegisterType");

            /* Register the required datatype for NamedMessage. */
            NamedMessageTypeSupport namedMessageTS = new NamedMessageTypeSupport();
            ErrorHandler.checkHandle(
                namedMessageTS, "new NamedMessageTypeSupport");
            string namedMessageTypeName = namedMessageTS.TypeName;
            status = namedMessageTS.RegisterType(parentDP, namedMessageTypeName);
            ErrorHandler.checkStatus(
                status, "Chat.NamedMessageTypeSupport.RegisterType");

            /* Narrow the normal participant to its extended representative */
            ExtDomainParticipant participant = new ExtDomainParticipant(parentDP);

            /* Initialise QoS variables */
            TopicQos reliableTopicQos = new TopicQos();
            TopicQos settingTopicQos = new TopicQos();
            SubscriberQos subQos = new SubscriberQos();

            /* Set the ReliabilityQosPolicy to RELIABLE. */
            status = participant.GetDefaultTopicQos(ref reliableTopicQos);
            ErrorHandler.checkStatus(
                status, "DDS.DomainParticipant.GetDefaultTopicQos");
            reliableTopicQos.Reliability.Kind = ReliabilityQosPolicyKind.ReliableReliabilityQos;

            /* Make the tailored QoS the new default. */
            status = participant.SetDefaultTopicQos (reliableTopicQos);
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

            /* Create a multitopic that substitutes the userID
           with its corresponding userName. */
           ITopic namedMessageTopic = participant.CreateSimulatedMultitopic(
                "Chat_NamedMessage",
                namedMessageTypeName,
                "SELECT userID, name AS userName, index, content " +
                    "FROM Chat_NameService NATURAL JOIN Chat_ChatMessage " +
                    "WHERE userID <> %0",
                parameterList);
            ErrorHandler.checkHandle(
                namedMessageTopic,
                "ExtDomainParticipant.create_simulated_multitopic");

            /* Adapt the default SubscriberQos to read from the "ChatRoom" Partition. */
            status = participant.GetDefaultSubscriberQos(ref subQos);
            ErrorHandler.checkStatus(
                status, "DDS.DomainParticipant.GetDefaultSubscriberQos");
            subQos.Partition.Name = new string[1];
            subQos.Partition.Name[0] = partitionName;

            /* Create a Subscriber for the MessageBoard application. */
            ISubscriber chatSubscriber = participant.CreateSubscriber(subQos);
            ErrorHandler.checkHandle(
                chatSubscriber, "DDS.DomainParticipant.CreateSubscriber");

            /* Create a DataReader for the NamedMessage Topic
           (using the appropriate QoS). */
            IDataReader parentReader = chatSubscriber.CreateDataReader(
                namedMessageTopic);
            ErrorHandler.checkHandle(
                parentReader, "DDS.Subscriber.CreateDatareader");

            NamedMessageDataReader chatAdmin = parentReader as NamedMessageDataReader;
            /* Print a message that the MessageBoard has opened. */
            System.Console.WriteLine(
                "MessageBoard has opened: send a ChatMessage " +
                "with userID = -1 to close it....\n");

            bool terminated = false;

            NamedMessage[] messages = null;;
            SampleInfo[] infos = null;

            while (!terminated)
            {
                /* Note: using read does not remove the samples from
               unregistered instances from the DataReader. This means
               that the DataRase would use more and more resources.
               That's why we use take here instead. */

                status = chatAdmin.Take(
                    ref messages,
                    ref infos,
                    SampleStateKind.Any,
                    ViewStateKind.Any,
                    InstanceStateKind.Alive);
                ErrorHandler.checkStatus(
                    status, "Chat.NamedMessageDataReader.take");

                foreach (NamedMessage msg in messages)
                {
                    if (msg.userID == TERMINATION_MESSAGE)
                    {
                        System.Console.WriteLine("Termination message received: exiting...");
                        terminated = true;
                    }
                    else
                    {
                        System.Console.WriteLine("{0}: {1}", msg.userName, msg.content);
                    }
                }

                status = chatAdmin.ReturnLoan(ref messages, ref infos);
                ErrorHandler.checkStatus(status, "Chat.ChatMessageDataReader.ReturnLoan");
                System.Threading.Thread.Sleep(100);
            }

            /* Remove the DataReader */
            status = chatSubscriber.DeleteDataReader(chatAdmin);
            ErrorHandler.checkStatus(
                status, "DDS.Subscriber.DeleteDatareader");

            /* Remove the Subscriber. */
            status = participant.DeleteSubscriber(chatSubscriber);
            ErrorHandler.checkStatus(
                status, "DDS.DomainParticipant.DeleteSubscriber");

            /* Remove the Topics. */
            status = participant.DeleteSimulatedMultitopic(namedMessageTopic);
            ErrorHandler.checkStatus(
                status, "DDS.ExtDomainParticipant.DeleteSimulatedMultitopic");

            status = participant.DeleteTopic(nameServiceTopic);
            ErrorHandler.checkStatus(
                status, "DDS.DomainParticipant.DeleteTopic (nameServiceTopic)");

            status = participant.DeleteTopic(chatMessageTopic);
            ErrorHandler.checkStatus(
                status, "DDS.DomainParticipant.DeleteTopic (chatMessageTopic)");

            /* Remove the DomainParticipant. */
            status = dpf.DeleteParticipant(parentDP);
            ErrorHandler.checkStatus(
                status, "DDS.DomainParticipantFactory.DeleteParticipant");
        }
    }
}
