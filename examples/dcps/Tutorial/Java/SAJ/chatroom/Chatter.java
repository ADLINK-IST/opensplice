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
 * LOGICAL_NAME:    Chatter.java
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the Java programming language.
 * DATE             june 2007.
 ************************************************************************
 * 
 * This file contains the implementation for the 'Chatter' executable.
 * 
 ***/

package chatroom;

import DDS.*;
import Chat.*;

public class Chatter {
    
    public static final int NUM_MSG = 10;
    public static final int TERMINATION_MESSAGE = -1; 

	public static void main(String[] args) {
	    /* Generic DDS entities */
	    DomainParticipantFactory   dpf;
	    DomainParticipant          participant;
	    Topic                      chatMessageTopic;
	    Topic                      nameServiceTopic;
	    Publisher                  chatPublisher;
	    DataWriter                 parentWriter;

	    /* EntityQos holders */
	    TopicQosHolder         reliableTopicQos    = new TopicQosHolder();
	    TopicQosHolder         settingTopicQos     = new TopicQosHolder();
	    PublisherQosHolder     pubQos              = new PublisherQosHolder();
        DataWriterQosHolder    dwQos               = new DataWriterQosHolder();

        /* QosPolicy fields. */
        WriterDataLifecycleQosPolicy writerDataLifecycle;

	    /* DDS Identifiers */
	    String                 domain = null;
	    long                   userHandle;
	    int                    status;

	    /* Type-specific DDS entities */
	    ChatMessageTypeSupport chatMessageTS;
	    NameServiceTypeSupport nameServiceTS;
	    ChatMessageDataWriter  talker;
	    NameServiceDataWriter  nameServer;

	    /* Sample definitions */
	    ChatMessage            msg                = new ChatMessage();
	    NameService            ns                 = new NameService();
	    
	    /* Others */
	    int                    ownID = 1;
	    int                    i;
	    String                 chatterName = null;
	    String                 partitionName = new String("ChatRoom");
        String                 chatMessageTypeName;
        String                 nameServiceTypeName;
    

	    /* Options: Chatter [ownID [name]] */
	    if (args.length > 0) {
	        ownID = Integer.parseInt(args[0]);
	        if (args.length > 1) {
	            chatterName = args[1];
	        }
	    }
        
	    /* Create a DomainParticipantFactory and a DomainParticipant 
           (using Default QoS settings. */
	    dpf = DomainParticipantFactory.get_instance ();
	    ErrorHandler.checkHandle(
            dpf, "DDS.DomainParticipantFactory.get_instance");
	    participant = dpf.create_participant(
            domain, PARTICIPANT_QOS_DEFAULT.value, null, STATUS_MASK_NONE.value);
	    ErrorHandler.checkHandle(
            participant, "DDS.DomainParticipantFactory.create_participant");
	    
	    /* Register the required datatype for ChatMessage. */
	    chatMessageTS = new ChatMessageTypeSupport();
        ErrorHandler.checkHandle(
            chatMessageTS, "new ChatMessageTypeSupport");
        chatMessageTypeName = chatMessageTS.get_type_name();
	    status = chatMessageTS.register_type(
            participant, chatMessageTypeName);
	    ErrorHandler.checkStatus(
            status, "Chat.ChatMessageTypeSupport.register_type");
	    
	    /* Register the required datatype for NameService. */
	    nameServiceTS = new NameServiceTypeSupport();
        ErrorHandler.checkHandle(
            nameServiceTS, "new NameServiceTypeSupport");
        nameServiceTypeName = nameServiceTS.get_type_name();
	    status = nameServiceTS.register_type(
            participant, nameServiceTypeName);
	    ErrorHandler.checkStatus(
            status, "Chat.NameServiceTypeSupport.register_type");
	    
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

	    /* Adapt the default PublisherQos to write into the 
           "ChatRoom" Partition. */
	    status = participant.get_default_publisher_qos (pubQos);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.get_default_publisher_qos");
	    pubQos.value.partition.name = new String[1];
	    pubQos.value.partition.name[0] = partitionName;

	    /* Create a Publisher for the chatter application. */
	    chatPublisher = participant.create_publisher(
            pubQos.value, null, STATUS_MASK_NONE.value);
        ErrorHandler.checkHandle(
            chatPublisher, "DDS.DomainParticipant.create_publisher");
	    
	    /* Create a DataWriter for the ChatMessage Topic 
           (using the appropriate QoS). */
	    parentWriter = chatPublisher.create_datawriter(
	        chatMessageTopic, 
	        DATAWRITER_QOS_USE_TOPIC_QOS.value,
	        null,
            STATUS_MASK_NONE.value);
        ErrorHandler.checkHandle(
            parentWriter, "DDS.Publisher.create_datawriter (chatMessage)");
	    
	    /* Narrow the abstract parent into its typed representative. */
	    talker = ChatMessageDataWriterHelper.narrow(parentWriter);
        ErrorHandler.checkHandle(
            talker, "Chat.ChatMessageDataWriterHelper.narrow");

        /* Create a DataWriter for the NameService Topic 
           (using the appropriate QoS). */
        status = chatPublisher.get_default_datawriter_qos(dwQos);
        ErrorHandler.checkStatus(
            status, "DDS.Publisher.get_default_datawriter_qos");
        status = chatPublisher.copy_from_topic_qos(
            dwQos, settingTopicQos.value);
        ErrorHandler.checkStatus(status, "DDS.Publisher.copy_from_topic_qos");
        writerDataLifecycle = dwQos.value.writer_data_lifecycle;
        writerDataLifecycle.autodispose_unregistered_instances = false;
        parentWriter = chatPublisher.create_datawriter( 
            nameServiceTopic, 
            dwQos.value, 
            null,
            STATUS_MASK_NONE.value);
        ErrorHandler.checkHandle(
            parentWriter, "DDS.Publisher.create_datawriter (NameService)");
        
        /* Narrow the abstract parent into its typed representative. */
        nameServer = NameServiceDataWriterHelper.narrow(parentWriter);
        ErrorHandler.checkHandle(
            nameServer, "Chat.NameServiceDataWriterHelper.narrow");
        
        /* Initialize the NameServer attributes. */
        ns.userID = ownID;
        if (chatterName != null) {
            ns.name = chatterName;
        } else {
            ns.name = "Chatter " + ownID;
        }

        /* Write the user-information into the system 
           (registering the instance implicitly). */
        status = nameServer.write(ns, HANDLE_NIL.value);
        ErrorHandler.checkStatus(status, "Chat.ChatMessageDataWriter.write");
        
        /* Initialize the chat messages. */
        msg.userID = ownID;
        msg.index = 0;
        if (ownID == TERMINATION_MESSAGE) {
            msg.content = "Termination message.";
        } else { 
            msg.content = "Hi there, I will send you " + 
                NUM_MSG + " more messages.";
        }
        System.out.println("Writing message: \"" + msg.content + "\"");

        /* Register a chat message for this user 
           (pre-allocating resources for it!!) */
        userHandle = talker.register_instance(msg);

        /* Write a message using the pre-generated instance handle. */
        status = talker.write(msg, userHandle);
        ErrorHandler.checkStatus(status, "Chat.ChatMessageDataWriter.write");
     
        try {
            Thread.sleep (1000); /* do not run so fast! */
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        /* Write any number of messages . */
        for (i = 1; i <= NUM_MSG && ownID != TERMINATION_MESSAGE; i++) {
            msg.index = i;
            msg.content = "Message no. " + i;
            System.out.println("Writing message: \"" + msg.content + "\"");
            status = talker.write(msg, userHandle);
            ErrorHandler.checkStatus(
                status, "Chat.ChatMessageDataWriter.write");
            try {
                Thread.sleep (1000); /* do not run so fast! */
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    
        /* Leave the room by disposing and unregistering the message instance */
        status = talker.dispose(msg, userHandle);
        ErrorHandler.checkStatus(
            status, "Chat.ChatMessageDataWriter.dispose");
        status = talker.unregister_instance(msg, userHandle);
        ErrorHandler.checkStatus(
            status, "Chat.ChatMessageDataWriter.unregister_instance");

        /* Also unregister our name. */
        status = nameServer.unregister_instance(ns, HANDLE_NIL.value);
        ErrorHandler.checkStatus(
            status, "Chat.NameServiceDataWriter.unregister_instance");

        /* Remove the DataWriters */
        status = chatPublisher.delete_datawriter(talker);
        ErrorHandler.checkStatus(
            status, "DDS.Publisher.delete_datawriter (talker)");
        
        status = chatPublisher.delete_datawriter(nameServer);
        ErrorHandler.checkStatus(status, 
            "DDS.Publisher.delete_datawriter (nameServer)");
        
        /* Remove the Publisher. */
        status = participant.delete_publisher(chatPublisher);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.delete_publisher");
        
        /* Remove the Topics. */
        status = participant.delete_topic(nameServiceTopic);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.delete_topic (nameServiceTopic)");
        
        status = participant.delete_topic(chatMessageTopic);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.delete_topic (chatMessageTopic)");
        
        /* Remove the DomainParticipant. */
        status = dpf.delete_participant(participant);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipantFactory.delete_participant");
	}
}
