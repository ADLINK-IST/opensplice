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
using Chat;

namespace Chatroom
{

    class UserLoad
    {
        private static GuardCondition escape;

        private void doWait()
        {
            Thread.Sleep(60000);

            escape.SetTriggerValue(true);
        }

        static void Main(string[] args)
        {
            int domain = DDS.DomainId.Default;

            /* Create a DomainParticipant */
            DomainParticipantFactory dpf = DomainParticipantFactory.Instance;

            IDomainParticipant participant = dpf.CreateParticipant(
                domain,
                null,
                StatusKind.Any);
            ErrorHandler.checkHandle(
                participant, "DDS.DomainParticipantFactory.CreateParticipant");

            /* Register the required datatype for ChatMessage. */
            ChatMessageTypeSupport chatMessageTS = new ChatMessageTypeSupport();
            string chatMessageTypeName = chatMessageTS.TypeName;
            ReturnCode status = chatMessageTS.RegisterType(
                participant, chatMessageTypeName);
            ErrorHandler.checkStatus(
                status, "Chat.ChatMessageTypeSupport.RegisterType");

            /* Register the required datatype for NameService. */
            NameServiceTypeSupport nameServiceTS = new NameServiceTypeSupport();
            ErrorHandler.checkHandle(
                nameServiceTS, "new NameServiceTypeSupport");
            string nameServiceTypeName = nameServiceTS.TypeName;
            status = nameServiceTS.RegisterType(
                participant, nameServiceTypeName);
            ErrorHandler.checkStatus(
                status, "Chat.NameServiceTypeSupport.RegisterType");

            /* Set the ReliabilityQosPolicy to RELIABLE. */
            TopicQos reliableTopicQos = new TopicQos();
            status = participant.GetDefaultTopicQos(ref reliableTopicQos);
            ErrorHandler.checkStatus(
                status, "DDS.DomainParticipant.get_DefaultTopicQos");
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
            TopicQos settingTopicQos = new TopicQos();
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
                nameServiceTopic, "DDS.DomainParticipant.CreateTopic");

            /* Adapt the default SubscriberQos to read from the
           "ChatRoom" Partition. */
            SubscriberQos subQos = new SubscriberQos();
            status = participant.GetDefaultSubscriberQos(ref subQos);
            ErrorHandler.checkStatus(
                status, "DDS.DomainParticipant.GetDefaultSubscriberQos");
            subQos.Partition.Name = new string[1];
            subQos.Partition.Name[0] = "ChatRoom";

            /* Create a Subscriber for the UserLoad application. */
            ISubscriber chatSubscriber = participant.CreateSubscriber(subQos);
            ErrorHandler.checkHandle(
                chatSubscriber, "DDS.DomainParticipant.CreateSubscriber");

            /* Create a DataReader for the NameService Topic
               (using the appropriate QoS). */
            DataReaderQos nsQos = null;
            status = chatSubscriber.GetDefaultDataReaderQos(ref nsQos);
            ErrorHandler.checkStatus(
                status, "DDS.Subscriber.GetDefaultDataReaderQos");
            status = chatSubscriber.CopyFromTopicQos(ref nsQos, settingTopicQos);
            ErrorHandler.checkStatus(
                status, "DDS.Subscriber.CopyFromTopicQos");
            IDataReader parentReader = chatSubscriber.CreateDataReader(
                nameServiceTopic,
                nsQos,
                null,
                StatusKind.Any);
            ErrorHandler.checkHandle(
                parentReader, "DDS.Subscriber.CreateDatareader (NameService)");

            NameServiceDataReader nameServer = parentReader as NameServiceDataReader;

            /* Adapt the DataReaderQos for the ChatMessageDataReader to
               keep track of all messages. */
            DataReaderQos messageQos = new DataReaderQos();
            status = chatSubscriber.GetDefaultDataReaderQos(ref messageQos);
            ErrorHandler.checkStatus(
                status, "DDS.Subscriber.GetDefaultDataReaderQos");
            status = chatSubscriber.CopyFromTopicQos(
                ref messageQos, reliableTopicQos);
            ErrorHandler.checkStatus(
                status, "DDS.Subscriber.CopyFromTopicQos");
            messageQos.History.Kind = HistoryQosPolicyKind.KeepAllHistoryQos;

            /* Create a DataReader for the ChatMessage Topic
               (using the appropriate QoS). */
            parentReader = chatSubscriber.CreateDataReader(
                chatMessageTopic,
                messageQos);
            ErrorHandler.checkHandle(
                parentReader, "DDS.Subscriber.CreateDataReader (ChatMessage)");

            /* Narrow the abstract parent into its typed representative. */
            ChatMessageDataReader loadAdmin = parentReader as ChatMessageDataReader;

            /* Initialize the Query Arguments. */
            string[] parameters = { "0" };

            /* Create a QueryCondition that will contain all messages
               with userID=ownID */
            IQueryCondition singleUser = loadAdmin.CreateQueryCondition("userID=%0", parameters);
            ErrorHandler.checkHandle(singleUser, "DDS.DataReader.CreateQuerycondition");

            /* Create a ReadCondition that will contain new users only */
            IReadCondition newUser = nameServer.CreateReadCondition(SampleStateKind.NotRead, ViewStateKind.New, InstanceStateKind.Alive);
            ErrorHandler.checkHandle(newUser, "DDS.DataReader.create_readcondition");

            /* Obtain a StatusCondition that triggers only when a Writer
               changes Liveliness */
            IStatusCondition leftUser = loadAdmin.StatusCondition;
            ErrorHandler.checkHandle(leftUser, "DDS.DataReader.GetStatusCondition");
            status = leftUser.SetEnabledStatuses(StatusKind.LivelinessChanged);
            ErrorHandler.checkStatus(status, "DDS.StatusCondition.SetEnabledStatuses");

            /* Create a bare guard which will be used to close the room */
            escape = new GuardCondition();

            /* Create a waitset and add the ReadConditions */
            WaitSet userLoadWS = new WaitSet();
            status = userLoadWS.AttachCondition(newUser);
            ErrorHandler.checkStatus(status, "DDS.WaitSet.AttachCondition (newUser)");
            status = userLoadWS.AttachCondition(leftUser);
            ErrorHandler.checkStatus(status, "DDS.WaitSet.AttachCondition (leftUser)");
            status = userLoadWS.AttachCondition(escape);
            ErrorHandler.checkStatus(status, "DDS.WaitSet.AttachCondition (escape)");

            /* Initialize and pre-allocate the GuardList used to obtain
               the triggered Conditions. */
            ICondition[] guardList = new ICondition[3];

            /* Remove all known Users that are not currently active. */
            NameService[] nsMsgs = null;
            SampleInfo[] nsInfos = null;

            status = nameServer.Take(ref nsMsgs,
                ref nsInfos,
                Length.Unlimited,
                SampleStateKind.Any,
                ViewStateKind.Any,
                InstanceStateKind.NotAlive);
            ErrorHandler.checkStatus(
                status, "Chat.NameServiceDataReader.Take");
            status = nameServer.ReturnLoan(ref nsMsgs, ref nsInfos);
            ErrorHandler.checkStatus(
                status, "Chat.NameServiceDataReader.ReturnLoan");

            /* Start the sleeper thread. */
            new Thread(new UserLoad().doWait).Start();

            bool closed = false;

            int prevCount = 0;
            ChatMessage[] msgs = null;
            SampleInfo[] msgInfos = null;

            while (!closed)
            {
                /* Wait until at least one of the Conditions in the
                   waitset triggers. */
                status = userLoadWS.Wait(ref guardList, Duration.Infinite);
                ErrorHandler.checkStatus(status, "DDS.WaitSet.Wait");

                /* Walk over all guards to display information */
                foreach (ICondition guard in guardList)
                {
                    if (guard == newUser)
                    {
                        /* The newUser ReadCondition contains data */
                        status = nameServer.ReadWithCondition(ref nsMsgs, ref nsInfos, newUser);
                        ErrorHandler.checkStatus(status, "Chat.NameServiceDataReader.read_w_condition");

                        foreach (NameService ns in nsMsgs)
                        {
                            Console.WriteLine("New user: " + ns.name);
                        }
                        status = nameServer.ReturnLoan(ref nsMsgs, ref nsInfos);
                        ErrorHandler.checkStatus(status, "Chat.NameServiceDataReader.ReturnLoan");

                    }
                    else if (guard == leftUser)
                    {
                        // Some liveliness has changed (either a DataWriter
                        // joined or a DataWriter left)
                        LivelinessChangedStatus livelinessStatus = new LivelinessChangedStatus() ;
                        status = loadAdmin.GetLivelinessChangedStatus(ref livelinessStatus);
                        ErrorHandler.checkStatus(status, "DDS.DataReader.getLivelinessChangedStatus");
                        if (livelinessStatus.AliveCount < prevCount)
                        {
                            /* A user has left the ChatRoom, since a DataWriter
                               lost its liveliness. Take the effected users
                               so they will not appear in the list later on. */
                            status = nameServer.Take(
                                ref nsMsgs,
                                ref nsInfos,
                                Length.Unlimited,
                                SampleStateKind.Any,
                                ViewStateKind.Any,
                                InstanceStateKind.NotAliveNoWriters);
                            ErrorHandler.checkStatus(status, "Chat.NameServiceDataReader.Take");

                            foreach (NameService nsMsg in nsMsgs)
                            {
                                /* re-apply query arguments */
                                parameters[0] = nsMsg.userID.ToString();
                                status = singleUser.SetQueryParameters(parameters);
                                ErrorHandler.checkStatus(status, "DDS.QueryCondition.SetQueryParameters");

                                /* Read this user's history */
                                status = loadAdmin.TakeWithCondition(
                                    ref msgs,
                                    ref msgInfos,
                                    singleUser);

                                ErrorHandler.checkStatus(
                                    status,
                                    "Chat.ChatMessageDataReader.TakeWithCondition");

                                /* Display the user and his history */
                                Console.WriteLine("Departed user {0} has sent {1} messages ", nsMsg.name, msgs.Length);
                                status = loadAdmin.ReturnLoan(ref msgs, ref msgInfos);
                                ErrorHandler.checkStatus(status, "Chat.ChatMessageDataReader.ReturnLoan");
                            }
                            status = nameServer.ReturnLoan(ref nsMsgs, ref nsInfos);
                            ErrorHandler.checkStatus(status, "Chat.NameServiceDataReader.ReturnLoan");
                        }
                        prevCount = livelinessStatus.AliveCount;

                    }
                    else if (guard == escape)
                    {
                        Console.WriteLine("UserLoad has terminated.");
                        closed = true;
                    }
                    else
                    {
                        //System.Diagnostics.Debug.Fail("Unknown Condition");
                    }
                } /* for */
            } /* while (!closed) */

            /* Remove all Conditions from the WaitSet. */
            status = userLoadWS.DetachCondition(escape);
            ErrorHandler.checkStatus(status, "DDS.WaitSet.DetachCondition (escape)");
            status = userLoadWS.DetachCondition(leftUser);
            ErrorHandler.checkStatus(status, "DDS.WaitSet.DetachCondition (leftUser)");
            status = userLoadWS.DetachCondition(newUser);
            ErrorHandler.checkStatus(status, "DDS.WaitSet.DetachCondition (newUser)");

            /* Free all resources */
            status = participant.DeleteContainedEntities();
            ErrorHandler.checkStatus(status, "DDS.DomainParticipant.DeleteContainedEntities");
            status = dpf.DeleteParticipant(participant);
            ErrorHandler.checkStatus(status, "DDS.DomainParticipantFactory.DeleteParticipant");
        }
    }
}
