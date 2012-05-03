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
 * LOGICAL_NAME:    UserLoad.java
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the Java programming language.
 * DATE             june 2007.
 ************************************************************************
 * 
 * This file contains the implementation for the 'UserLoad' executable.
 * 
 ***/

package chatroom;

import DDS.*;
import Chat.*;

public class UserLoad extends Thread {

    /* entities required by all threads. */
    public static GuardCondition            escape;
    public static final int                 TERMINATION_MESSAGE = -1; 
    
    /**
     * Sleeper thread: sleeps 60 seconds and then triggers the WaitSet.
     */
    public void run() {
        int status;
        
        try {
            sleep(60000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        status = escape.set_trigger_value(true);
        ErrorHandler.checkStatus(
            status, "DDS.GuardCondition.set_trigger_value");
    }

    public static void main(String[] args) {
        /* Generic DDS entities */
        DomainParticipant               participant;
        Topic                           chatMessageTopic;
        Topic                           nameServiceTopic;
        Subscriber                      chatSubscriber;
        DataReader                      parentReader;
        QueryCondition                  singleUser;
        ReadCondition                   newUser;
        StatusCondition                 leftUser;
        WaitSet                         userLoadWS;
        LivelinessChangedStatusHolder   livChangStatus = 
            new LivelinessChangedStatusHolder();

        /* QosPolicy holders */
        TopicQosHolder          settingTopicQos  = new TopicQosHolder();
        TopicQosHolder          reliableTopicQos = new TopicQosHolder();
        SubscriberQosHolder     subQos           = new SubscriberQosHolder();
        DataReaderQosHolder     messageQos       = new DataReaderQosHolder();

        /* DDS Identifiers */
        String                  domain           = null;
        int                     status;
        ConditionSeqHolder      guardList        = new ConditionSeqHolder();

        /* Type-specific DDS entities */
        ChatMessageTypeSupport  chatMessageTS;
        NameServiceTypeSupport  nameServiceTS;
        NameServiceDataReader   nameServer;
        ChatMessageDataReader   loadAdmin;
        ChatMessageSeqHolder    msgList          = new ChatMessageSeqHolder();
        NameServiceSeqHolder    nsList           = new NameServiceSeqHolder();
        SampleInfoSeqHolder     infoSeq          = new SampleInfoSeqHolder();
        SampleInfoSeqHolder     infoSeq2         = new SampleInfoSeqHolder();

        /* Others */
        String[]                params;
        String                  chatMessageTypeName;
        String                  nameServiceTypeName;
        boolean                 closed           = false;
        int                     prevCount        = 0;

        /* Create a DomainParticipant (using the 
           'TheParticipantFactory' convenience macro). */
        participant = TheParticipantFactory.value.create_participant (
            domain, 
            PARTICIPANT_QOS_DEFAULT.value, 
            null,
            STATUS_MASK_NONE.value);
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
            nameServiceTopic, "DDS.DomainParticipant.create_topic");

        /* Adapt the default SubscriberQos to read from the 
           "ChatRoom" Partition. */
        status = participant.get_default_subscriber_qos (subQos);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.get_default_subscriber_qos");
        subQos.value.partition.name = new String[1];
        subQos.value.partition.name[0] = new String("ChatRoom");

        /* Create a Subscriber for the UserLoad application. */
        chatSubscriber = participant.create_subscriber(
            subQos.value, null, STATUS_MASK_NONE.value);
        ErrorHandler.checkHandle(
            chatSubscriber, "DDS.DomainParticipant.create_subscriber");
        
        /* Create a DataReader for the NameService Topic 
           (using the appropriate QoS). */
        parentReader = chatSubscriber.create_datareader( 
            nameServiceTopic, 
            DATAREADER_QOS_USE_TOPIC_QOS.value, 
            null,
            STATUS_MASK_NONE.value);
        ErrorHandler.checkHandle(
            parentReader, "DDS.Subscriber.create_datareader (NameService)");

        /* Narrow the abstract parent into its typed representative. */
        nameServer = NameServiceDataReaderHelper.narrow(parentReader);
        ErrorHandler.checkHandle(
            nameServer, "Chat.NameServiceDataReaderHelper.narrow");
        
        /* Adapt the DataReaderQos for the ChatMessageDataReader to 
           keep track of all messages. */
        status = chatSubscriber.get_default_datareader_qos(messageQos);
        ErrorHandler.checkStatus(
            status, "DDS.Subscriber.get_default_datareader_qos");
        status = chatSubscriber.copy_from_topic_qos(
            messageQos, reliableTopicQos.value);
        ErrorHandler.checkStatus(
            status, "DDS.Subscriber.copy_from_topic_qos");
        messageQos.value.history.kind = 
            HistoryQosPolicyKind.KEEP_ALL_HISTORY_QOS;

        /* Create a DataReader for the ChatMessage Topic 
           (using the appropriate QoS). */
        parentReader = chatSubscriber.create_datareader(
            chatMessageTopic, 
            messageQos.value, 
            null, 
            STATUS_MASK_NONE.value);
        ErrorHandler.checkHandle(
            parentReader, "DDS.Subscriber.create_datareader (ChatMessage)");
        
        /* Narrow the abstract parent into its typed representative. */
        loadAdmin = ChatMessageDataReaderHelper.narrow(parentReader);
        ErrorHandler.checkHandle(
            loadAdmin, "Chat.ChatMessageDataReaderHelper.narrow");
        
        /* Initialize the Query Arguments. */
        params = new String[1];
        params[0] = new String("0");
        
        /* Create a QueryCondition that will contain all messages 
           with userID=ownID */
        singleUser = loadAdmin.create_querycondition( 
            ANY_SAMPLE_STATE.value, 
            ANY_VIEW_STATE.value, 
            ANY_INSTANCE_STATE.value, 
            "userID=%0", 
            params);
        ErrorHandler.checkHandle(
            singleUser, "DDS.DataReader.create_querycondition");
        
        /* Create a ReadCondition that will contain new users only */
        newUser = nameServer.create_readcondition( 
            NOT_READ_SAMPLE_STATE.value, 
            NEW_VIEW_STATE.value, 
            ALIVE_INSTANCE_STATE.value);
        ErrorHandler.checkHandle(
            newUser, "DDS.DataReader.create_readcondition");

        /* Obtain a StatusCondition that triggers only when a Writer 
           changes Liveliness */
        leftUser = loadAdmin.get_statuscondition();
        ErrorHandler.checkHandle(
            leftUser, "DDS.DataReader.get_statuscondition");
        status = leftUser.set_enabled_statuses(
            LIVELINESS_CHANGED_STATUS.value);
        ErrorHandler.checkStatus(
            status, "DDS.StatusCondition.set_enabled_statuses");

        /* Create a bare guard which will be used to close the room */
        escape = new GuardCondition();

        /* Create a waitset and add the ReadConditions */
        userLoadWS = new WaitSet();
        status = userLoadWS.attach_condition(newUser);
        ErrorHandler.checkStatus(
            status, "DDS.WaitSet.attach_condition (newUser)");
        status = userLoadWS.attach_condition(leftUser);
        ErrorHandler.checkStatus(
            status, "DDS.WaitSet.attach_condition (leftUser)");
        status = userLoadWS.attach_condition(escape);
        ErrorHandler.checkStatus(
            status, "DDS.WaitSet.attach_condition (escape)");
     
        /* Initialize and pre-allocate the GuardList used to obtain 
           the triggered Conditions. */
        guardList.value = new Condition[3];
        
        /* Remove all known Users that are not currently active. */
        status = nameServer.take( 
            nsList, 
            infoSeq, 
            LENGTH_UNLIMITED.value, 
            ANY_SAMPLE_STATE.value, 
            ANY_VIEW_STATE.value, 
            NOT_ALIVE_INSTANCE_STATE.value);
        ErrorHandler.checkStatus(
            status, "Chat.NameServiceDataReader.take");
        status = nameServer.return_loan(nsList, infoSeq);
        ErrorHandler.checkStatus(
            status, "Chat.NameServiceDataReader.return_loan");
        
        /* Start the sleeper thread. */
        new UserLoad().start();
        
        /* Print a message that the UserLoad has opened. */
        System.out.println(
                "UserLoad has opened: disconnect a Chatter " +
                "with userID = " + 
                TERMINATION_MESSAGE +
                "to close it....\n");
      
        while (!closed) {
            /* Wait until at least one of the Conditions in the 
               waitset triggers. */
            status = userLoadWS._wait(guardList, DURATION_INFINITE.value);
            ErrorHandler.checkStatus(status, "DDS.WaitSet._wait");

            /* Walk over all guards to display information */
            for (int i = 0; i < guardList.value.length; i++) {
                if ( guardList.value[i] == newUser ) {
                    /* The newUser ReadCondition contains data */
                    status = nameServer.read_w_condition( 
                        nsList, 
                        infoSeq, 
                        LENGTH_UNLIMITED.value, 
                        newUser);
                    ErrorHandler.checkStatus(
                        status, 
                        "Chat.NameServiceDataReader.read_w_condition");
                    
                    for (int j = 0; j < nsList.value.length; j++) {
                        System.out.println(
                            "New user: " + nsList.value[j].name);
                    }
                    status = nameServer.return_loan(nsList, infoSeq);
                    ErrorHandler.checkStatus(
                        status, "Chat.NameServiceDataReader.return_loan");

                } else if ( guardList.value[i] == leftUser ) {
                    // Some liveliness has changed (either a DataWriter 
                    // joined or a DataWriter left)
                    status = loadAdmin.get_liveliness_changed_status(
                        livChangStatus);
                    ErrorHandler.checkStatus(
                        status, 
                        "DDS.DataReader.get_liveliness_changed_status");
                    if (livChangStatus.value.alive_count < prevCount) {
                        /* A user has left the ChatRoom, since a DataWriter 
                           lost its liveliness. Take the effected users 
                           so they will not appear in the list later on. */
                        status = nameServer.take( 
                            nsList, 
                            infoSeq, 
                            LENGTH_UNLIMITED.value, 
                            ANY_SAMPLE_STATE.value, 
                            ANY_VIEW_STATE.value, 
                            NOT_ALIVE_NO_WRITERS_INSTANCE_STATE.value);
                        ErrorHandler.checkStatus(
                            status, "Chat.NameServiceDataReader.take");
        
                        for (int j = 0; j < nsList.value.length; j++) {
                            /* re-apply query arguments */
                            params[0] = 
                                Integer.toString(nsList.value[j].userID);
                            status = singleUser.set_query_parameters(params);
                            ErrorHandler.checkStatus(
                                status, 
                                "DDS.QueryCondition.set_query_parameters");
        
                            /* Read this users history */
                            status = loadAdmin.take_w_condition( 
                                msgList, 
                                infoSeq2, 
                                LENGTH_UNLIMITED.value, 
                                singleUser );
                            ErrorHandler.checkStatus(
                                status, 
                                "Chat.ChatMessageDataReader.take_w_condition");
                            
                            /* Display the user and his history */
                            System.out.println(
                                "Departed user " + nsList.value[j].name + 
                                " has sent " + msgList.value.length + 
                                " messages.");
                            status = loadAdmin.return_loan(msgList, infoSeq2);
                            ErrorHandler.checkStatus(
                                status, 
                                "Chat.ChatMessageDataReader.return_loan");
                            msgList.value = null;
                            infoSeq2.value = null;
                            if(nsList.value[j].userID == TERMINATION_MESSAGE)
                            {
                            	System.out.println("Termination message received: exiting...");
                                closed = true;
                            }
                        }
                        status = nameServer.return_loan(nsList, infoSeq);
                        ErrorHandler.checkStatus(
                            status, 
                            "Chat.NameServiceDataReader.return_loan");
                        nsList.value = null;
                        infoSeq.value = null;
                    }
                    prevCount = livChangStatus.value.alive_count;

                } else if ( guardList.value[i] == escape ) {
                    System.out.println("UserLoad has terminated.");
                    closed = true;
                }
                else
                {
                    assert false : "Unknown Condition";
                };
            } /* for */
        } /* while (!closed) */

        /* Remove all Conditions from the WaitSet. */
        status = userLoadWS.detach_condition(escape);
        ErrorHandler.checkStatus(
            status, "DDS.WaitSet.detach_condition (escape)");
        status = userLoadWS.detach_condition(leftUser);
        ErrorHandler.checkStatus(
            status, "DDS.WaitSet.detach_condition (leftUser)");
        status = userLoadWS.detach_condition(newUser);
        ErrorHandler.checkStatus(
            status, "DDS.WaitSet.detach_condition (newUser)");

        /* Free all resources */
        status = participant.delete_contained_entities();
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.delete_contained_entities");
        status = TheParticipantFactory.value.delete_participant(participant);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipantFactory.delete_participant");

    }

}
