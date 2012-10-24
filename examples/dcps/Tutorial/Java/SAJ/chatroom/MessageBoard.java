/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

/************************************************************************
 * LOGICAL_NAME:    MessageBoard.java
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the Java programming language.
 * DATE             june 2007.
 ************************************************************************
 * 
 * This file contains the implementation for the 'MessageBoard' executable.
 * 
 ***/

package chatroom;

import DDS.*;
import Chat.*;

public class MessageBoard {

   
    public static final int TERMINATION_MESSAGE = -1; 

    
    public static void main(String[] args) {
        /* Generic DDS entities */
        DomainParticipantFactory  dpf;
        DomainParticipant         parentDP;
        ExtDomainParticipant      participant;
        Topic                     chatMessageTopic;
        Topic                     nameServiceTopic;
        TopicDescription          namedMessageTopic;
        Subscriber                chatSubscriber;
        DataReader                parentReader;

        /* Type-specific DDS entities */
        ChatMessageTypeSupport  chatMessageTS;
        NameServiceTypeSupport  nameServiceTS;
        NamedMessageTypeSupport namedMessageTS;
        NamedMessageDataReader  chatAdmin;
        NamedMessageSeqHolder   msgSeq           = new NamedMessageSeqHolder();
        SampleInfoSeqHolder     infoSeq          = new SampleInfoSeqHolder();

        /* QosPolicy holders */
        TopicQosHolder          reliableTopicQos = new TopicQosHolder();
        TopicQosHolder          settingTopicQos  = new TopicQosHolder();
        SubscriberQosHolder     subQos           = new SubscriberQosHolder();
        String[]                parameterList;

        /* DDS Identifiers */
        String                  domain           = null;
        int                     status;

        /* Others */
        boolean                 terminated       = false;
        String                  partitionName    = new String("ChatRoom");
        String                  chatMessageTypeName;
        String                  nameServiceTypeName;
        String                  namedMessageTypeName;

        /* Options: MessageBoard [ownID] */
        /* Messages having owner ownID will be ignored */
        parameterList = new String[1];
        
        if (args.length >0) {
            parameterList[0] = args[0];
        }
        else
        {
            parameterList[0] = new String("0");
        }
          
        /* Create a DomainParticipantFactory and a DomainParticipant 
           (using Default QoS settings. */
        dpf = DomainParticipantFactory.get_instance ();
        ErrorHandler.checkHandle(
            dpf, "DDS.DomainParticipantFactory.get_instance");
        parentDP = dpf.create_participant(
            domain, PARTICIPANT_QOS_DEFAULT.value, null, STATUS_MASK_NONE.value);
        ErrorHandler.checkHandle(
            parentDP, "DDS.DomainParticipantFactory.create_participant");
        
        /* Register the required datatype for ChatMessage. */
        chatMessageTS = new ChatMessageTypeSupport();
        ErrorHandler.checkHandle(
            chatMessageTS, "new ChatMessageTypeSupport");
        chatMessageTypeName = chatMessageTS.get_type_name();
        status = chatMessageTS.register_type(parentDP, chatMessageTypeName);
        ErrorHandler.checkStatus(
            status, "Chat.ChatMessageTypeSupport.register_type");
        
        /* Register the required datatype for NameService. */
        nameServiceTS = new NameServiceTypeSupport();
        ErrorHandler.checkHandle(
            nameServiceTS, "new NameServiceTypeSupport");
        nameServiceTypeName = nameServiceTS.get_type_name();
        status = nameServiceTS.register_type(parentDP, nameServiceTypeName);
        ErrorHandler.checkStatus(
            status, "Chat.NameServiceTypeSupport.register_type");
        
        /* Register the required datatype for NamedMessage. */
        namedMessageTS = new NamedMessageTypeSupport();
        ErrorHandler.checkHandle(
            namedMessageTS, "new NamedMessageTypeSupport");
        namedMessageTypeName = namedMessageTS.get_type_name();
        status = namedMessageTS.register_type(parentDP, namedMessageTypeName);
        ErrorHandler.checkStatus(
            status, "Chat.NamedMessageTypeSupport.register_type");
        
        /* Narrow the normal participant to its extended representative */
        participant = ExtDomainParticipantHelper.narrow(parentDP);
        ErrorHandler.checkHandle(
            participant, "ExtDomainParticipantHelper.narrow");   
        
        /* Set the ReliabilityQosPolicy to RELIABLE. */
        status = participant.get_default_topic_qos(reliableTopicQos);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.get_default_topic_qos");
        reliableTopicQos.value.reliability.kind = 
            ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
        
        /* Make the tailored QoS the new default. */
        status = participant.set_default_topic_qos(reliableTopicQos.value);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.set_default_topic_qos");

        /* Use the changed policy when defining the ChatMessage topic */
        chatMessageTopic = participant.create_topic(
            "Chat_ChatMessage", 
            chatMessageTypeName, 
            reliableTopicQos.value, 
            null,
            STATUS_MASK_NONE.value);
        ErrorHandler.checkHandle(
            chatMessageTopic, 
            "DDS.DomainParticipant.create_topic (ChatMessage)");
        
        /* Set the DurabilityQosPolicy to TRANSIENT. */
        status = participant.get_default_topic_qos(settingTopicQos);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.get_default_topic_qos");
        settingTopicQos.value.durability.kind = 
            DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS;

        /* Create the NameService Topic. */
        nameServiceTopic = participant.create_topic( 
            "Chat_NameService", 
            nameServiceTypeName, 
            settingTopicQos.value, 
            null,
            STATUS_MASK_NONE.value);
        ErrorHandler.checkHandle(
            nameServiceTopic, 
            "DDS.DomainParticipant.create_topic (NameService)");

        /* Create a multitopic that substitutes the userID 
           with its corresponding userName. */
        namedMessageTopic = participant.create_simulated_multitopic(
            "Chat_NamedMessage", 
            namedMessageTypeName, 
            "SELECT userID, name AS userName, index, content " +
                "FROM Chat_NameService NATURAL JOIN Chat_ChatMessage " +
                "WHERE userID <> %0",
            parameterList);
        ErrorHandler.checkHandle(
            namedMessageTopic, 
            "ExtDomainParticipant.create_simulated_multitopic");

        /* Adapt the default SubscriberQos to read from the 
           "ChatRoom" Partition. */
        status = participant.get_default_subscriber_qos (subQos);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.get_default_subscriber_qos");
        subQos.value.partition.name = new String[1];
        subQos.value.partition.name[0] = partitionName;

        /* Create a Subscriber for the MessageBoard application. */
        chatSubscriber = participant.create_subscriber(
            subQos.value, null, STATUS_MASK_NONE.value);
        ErrorHandler.checkHandle(
            chatSubscriber, "DDS.DomainParticipant.create_subscriber");
        
        /* Create a DataReader for the NamedMessage Topic 
           (using the appropriate QoS). */
        parentReader = chatSubscriber.create_datareader( 
            namedMessageTopic, 
            DATAREADER_QOS_USE_TOPIC_QOS.value, 
            null,
            STATUS_MASK_NONE.value);
        ErrorHandler.checkHandle(
            parentReader, "DDS.Subscriber.create_datareader");
        
        /* Narrow the abstract parent into its typed representative. */
        chatAdmin = NamedMessageDataReaderHelper.narrow(parentReader);
        ErrorHandler.checkHandle(
            chatAdmin, "Chat.NamedMessageDataReaderHelper.narrow");
        
        /* Print a message that the MessageBoard has opened. */
        System.out.println(
                "MessageBoard has opened: send a ChatMessage " +
                "with userID = -1 to close it....\n");

        while (!terminated) {
            /* Note: using read does not remove the samples from
               unregistered instances from the DataReader. This means
               that the DataRase would use more and more resources.
               That's why we use take here instead. */

            status = chatAdmin.take( 
                msgSeq, 
                infoSeq, 
                LENGTH_UNLIMITED.value, 
                ANY_SAMPLE_STATE.value, 
                ANY_VIEW_STATE.value, 
                ALIVE_INSTANCE_STATE.value );
            ErrorHandler.checkStatus(
                status, "Chat.NamedMessageDataReader.take");

            for (int i = 0; i < msgSeq.value.length; i++) {
                if (msgSeq.value[i].userID == TERMINATION_MESSAGE) {
                    System.out.println(
                        "Termination message received: exiting...");
                    terminated = true;
                } else {
                    System.out.println(
                        msgSeq.value[i].userName + ": " + 
                        msgSeq.value[i].content);
                }
            }

            status = chatAdmin.return_loan(msgSeq, infoSeq);
            ErrorHandler.checkStatus(
                status, "Chat.ChatMessageDataReader.return_loan");
            
            msgSeq.value = null;
            infoSeq.value = null;
            
            /* Sleep for some amount of time, as not to consume 
               too much CPU cycles. */
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        /* Remove the DataReader */
        status = chatSubscriber.delete_datareader(chatAdmin);
        ErrorHandler.checkStatus(
            status, "DDS.Subscriber.delete_datareader");
    
        /* Remove the Subscriber. */
        status = participant.delete_subscriber(chatSubscriber);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.delete_subscriber");
    
        /* Remove the Topics. */
        status = participant.delete_simulated_multitopic(namedMessageTopic);
        ErrorHandler.checkStatus(
            status, "DDS.ExtDomainParticipant.delete_simulated_multitopic");
    
        status = participant.delete_topic(nameServiceTopic);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.delete_topic (nameServiceTopic)");
    
        status = participant.delete_topic(chatMessageTopic);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.delete_topic (chatMessageTopic)");
    
        /* Remove the DomainParticipant. */
        status = dpf.delete_participant(parentDP);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipantFactory.delete_participant");
    }

}
