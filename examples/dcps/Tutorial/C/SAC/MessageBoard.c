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
 * LOGICAL_NAME:    MessageBoard.c
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C programming language.
 * DATE             june 2007.
 ************************************************************************
 * 
 * This file contains the implementation for the 'MessageBoard' executable.
 * 
 ***/

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "dds_dcps.h"
#include "CheckStatus.h"
#include "Chat.h"
#include "multitopic.h"

#define TERMINATION_MESSAGE -1 

#ifdef _VXWORKS
int MessageBoard_main (int argc, char ** argv);
int MessageBoard (char * args)
{
   int argc=1;
   char *argv[256];
   char *str1;
   argv[0] = (char*) strdup ("MessageBoard");
   str1 = (char*) strtok(args, " ");
   while (str1)
   {
      argv[argc] = (char*) strdup (str1);
      argc++;
      str1 = strtok(NULL, " ");
   }
   return MessageBoard_main (argc, argv);
}
int MessageBoard_main (int argc, char ** argv)
#else
int main (int argc, char ** argv)
#endif
{
    /* Generic DDS entities */
    DDS_DomainParticipantFactory    dpf;
    DDS_DomainParticipant           participant;
    DDS_Topic                       chatMessageTopic;
    DDS_Topic                       nameServiceTopic;
    DDS_MultiTopic                  namedMessageTopic;
    DDS_Subscriber                  chatSubscriber;

    /* Type-specific DDS entities */
    Chat_ChatMessageTypeSupport     chatMessageTS;
    Chat_NameServiceTypeSupport     nameServiceTS;
    Chat_NamedMessageTypeSupport    namedMessageTS;
    Chat_NamedMessageDataReader     chatAdmin;
    DDS_sequence_Chat_NamedMessage  *msgSeq;
    DDS_SampleInfoSeq               *infoSeq;

    /* QosPolicy holders */
    DDS_TopicQos                    *reliable_topic_qos;
    DDS_TopicQos                    *setting_topic_qos;
    DDS_SubscriberQos               *sub_qos;
    DDS_StringSeq                   *parameterList;

    /* DDS Identifiers */
    DDS_DomainId_t                  domain = NULL;
    DDS_ReturnCode_t                status;

    /* Others */
    DDS_unsigned_long               i;
    DDS_boolean                     terminated = FALSE;
    char *                          partitionName;
    char *                          chatMessageTypeName = NULL;
    char *                          nameServiceTypeName = NULL;
    char *                          namedMessageTypeName = NULL;
#ifdef USE_NANOSLEEP
    struct timespec                 sleeptime;
    struct timespec                 remtime;
#endif
    
    /* Options: MessageBoard [ownID] */
    /* Messages having owner ownID will be ignored */
    parameterList = DDS_StringSeq__alloc();
    checkHandle(parameterList, "DDS_StringSeq__alloc");
    parameterList->_length = 1;
    parameterList->_maximum = 1;
    parameterList->_buffer = DDS_StringSeq_allocbuf(1);
    checkHandle(parameterList->_buffer, "DDS_StringSeq_allocbuf");
    
    if (argc > 1) {
        parameterList->_buffer[0] = DDS_string_alloc ( strlen(argv[1]) );
        checkHandle(parameterList->_buffer[0], "DDS_string_alloc");
        strcpy (parameterList->_buffer[0], argv[1]);
    }
    else
    {
        parameterList->_buffer[0] = DDS_string_alloc(1);
        checkHandle(parameterList->_buffer[0], "DDS_string_alloc");
        strcpy (parameterList->_buffer[0], "0");
    }
      
    /* Create a DomainParticipantFactory and a DomainParticipant (using Default QoS settings. */
    dpf = DDS_DomainParticipantFactory_get_instance ();
    checkHandle(dpf, "DDS_DomainParticipantFactory_get_instance");
    participant = DDS_DomainParticipantFactory_create_participant (
        dpf, 
        domain, 
        DDS_PARTICIPANT_QOS_DEFAULT, 
        NULL, 
        DDS_STATUS_MASK_NONE);
    checkHandle(participant, "DDS_DomainParticipantFactory_create_participant");  

    /* Register the required datatype for ChatMessage. */
    chatMessageTS = Chat_ChatMessageTypeSupport__alloc();
    checkHandle(chatMessageTS, "Chat_ChatMessageTypeSupport__alloc");
    chatMessageTypeName = Chat_ChatMessageTypeSupport_get_type_name(chatMessageTS);
    status = Chat_ChatMessageTypeSupport_register_type(
        chatMessageTS, 
        participant, 
        chatMessageTypeName);
    checkStatus(status, "Chat_ChatMessageTypeSupport_register_type");
    
    /* Register the required datatype for NameService. */
    nameServiceTS = Chat_NameServiceTypeSupport__alloc();
    checkHandle(nameServiceTS, "Chat_NameServiceTypeSupport__alloc");
    nameServiceTypeName = Chat_NameServiceTypeSupport_get_type_name(nameServiceTS);
    status = Chat_NameServiceTypeSupport_register_type(
        nameServiceTS, 
        participant, 
        nameServiceTypeName);
    checkStatus(status, "Chat_NameServiceTypeSupport_register_type");
    
    /* Register the required datatype for NamedMessage. */
    namedMessageTS = Chat_NamedMessageTypeSupport__alloc();
    checkHandle(namedMessageTS, "Chat_NamedMessageTypeSupport__alloc");
    namedMessageTypeName = Chat_NamedMessageTypeSupport_get_type_name(namedMessageTS);
    status = Chat_NamedMessageTypeSupport_register_type(
        namedMessageTS, 
        participant, 
        namedMessageTypeName);
    checkStatus(status, "Chat_NamedMessageTypeSupport_register_type");
    
    /* Set the ReliabilityQosPolicy to RELIABLE. */
    reliable_topic_qos = DDS_TopicQos__alloc();
    checkHandle(reliable_topic_qos, "DDS_TopicQos__alloc");
    status = DDS_DomainParticipant_get_default_topic_qos(participant, reliable_topic_qos);
    checkStatus(status, "DDS_DomainParticipant_get_default_topic_qos");
    reliable_topic_qos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    
    /* Make the tailored QoS the new default. */
    status = DDS_DomainParticipant_set_default_topic_qos(participant, reliable_topic_qos);
    checkStatus(status, "DDS_DomainParticipant_set_default_topic_qos");

    /* Use the changed policy when defining the ChatMessage topic */
    chatMessageTopic = DDS_DomainParticipant_create_topic( 
        participant, 
        "Chat_ChatMessage", 
        chatMessageTypeName, 
        reliable_topic_qos, 
        NULL, 
        DDS_STATUS_MASK_NONE);
    checkHandle(chatMessageTopic, "DDS_DomainParticipant_create_topic (ChatMessage)");
    
    /* Set the DurabilityQosPolicy to TRANSIENT. */
    setting_topic_qos = DDS_TopicQos__alloc();
    checkHandle(setting_topic_qos, "DDS_TopicQos__alloc");
    status = DDS_DomainParticipant_get_default_topic_qos(participant, setting_topic_qos);
    checkStatus(status, "DDS_DomainParticipant_get_default_topic_qos");
    setting_topic_qos->durability.kind = DDS_TRANSIENT_DURABILITY_QOS;

    /* Create the NameService Topic. */
    nameServiceTopic = DDS_DomainParticipant_create_topic( 
        participant, 
        "Chat_NameService", 
        nameServiceTypeName, 
        setting_topic_qos, 
        NULL,
        DDS_STATUS_MASK_NONE);
    checkHandle(nameServiceTopic, "DDS_DomainParticipant_create_topic");
    
    /* Create a multitopic that substitutes the userID with its corresponding userName. */
    namedMessageTopic = DDS_DomainParticipant_create_simulated_multitopic(
        participant, 
        "Chat_NamedMessage", 
        namedMessageTypeName, 
        "SELECT userID, name AS userName, index, content "
            "FROM Chat_NameService NATURAL JOIN Chat_ChatMessage WHERE userID <> %0",
        parameterList);
    checkHandle(namedMessageTopic, "DDS_DomainParticipant_simulate_multitopic");

    /* Adapt the default SubscriberQos to read from the "ChatRoom" Partition. */
    partitionName = "ChatRoom";
    sub_qos = DDS_SubscriberQos__alloc();
    checkHandle(sub_qos, "DDS_SubscriberQos__alloc");
    status = DDS_DomainParticipant_get_default_subscriber_qos (participant, sub_qos);
    checkStatus(status, "DDS_DomainParticipant_get_default_subscriber_qos");
    sub_qos->partition.name._length = 1;
    sub_qos->partition.name._maximum = 1;
    sub_qos->partition.name._buffer = DDS_StringSeq_allocbuf (1);
    checkHandle(sub_qos->partition.name._buffer, "DDS_StringSeq_allocbuf");
    sub_qos->partition.name._buffer[0] = DDS_string_alloc ( strlen(partitionName) );
    checkHandle(sub_qos->partition.name._buffer[0], "DDS_string_alloc");
    strcpy (sub_qos->partition.name._buffer[0], partitionName);

    /* Create a Subscriber for the MessageBoard application. */
    chatSubscriber = DDS_DomainParticipant_create_subscriber(participant, sub_qos, NULL, DDS_STATUS_MASK_NONE);
    checkHandle(chatSubscriber, "DDS_DomainParticipant_create_subscriber");
    
    /* Create a DataReader for the NamedMessage Topic (using the appropriate QoS). */
    chatAdmin = DDS_Subscriber_create_datareader( 
        chatSubscriber, 
        namedMessageTopic, 
        DDS_DATAREADER_QOS_USE_TOPIC_QOS, 
        NULL,
        DDS_STATUS_MASK_NONE);
    checkHandle(chatAdmin, "DDS_Subscriber_create_datareader");
    
    /* Print a message that the MessageBoard has opened. */
    printf("MessageBoard has opened: send a ChatMessage with userID = -1 to close it....\n\n");

    /* Allocate the sequence holders for the DataReader */
    msgSeq = DDS_sequence_Chat_NamedMessage__alloc();
    checkHandle(msgSeq, "DDS_sequence_Chat_NamedMessage__alloc");
    infoSeq = DDS_SampleInfoSeq__alloc();
    checkHandle(infoSeq, "DDS_SampleInfoSeq__alloc");

    while (!terminated) {
        /* Note: using read does not remove the samples from 
           unregistered instances from the DataReader. This means 
           that the DataRase would use more and more resources. 
           That's why we use take here instead. */

        status = Chat_NamedMessageDataReader_take( 
            chatAdmin, 
            msgSeq, 
            infoSeq, 
            DDS_LENGTH_UNLIMITED, 
            DDS_ANY_SAMPLE_STATE, 
            DDS_ANY_VIEW_STATE, 
            DDS_ALIVE_INSTANCE_STATE );
        checkStatus(status, "Chat_NamedMessageDataReader_take");

        for (i = 0; i < msgSeq->_length; i++) {
            Chat_NamedMessage *msg = &(msgSeq->_buffer[i]);
            if (msg->userID == TERMINATION_MESSAGE) {
                printf("Termination message received: exiting...\n");
                terminated = TRUE;
            } else {
                printf ("%s: %s\n", msg->userName, msg->content);
            }
        }

        status = Chat_NamedMessageDataReader_return_loan(chatAdmin, msgSeq, infoSeq);
        checkStatus(status, "Chat_ChatMessageDataReader_return_loan");
        
        /* Sleep for some amount of time, as not to consume too much CPU cycles. */
#ifdef USE_NANOSLEEP
        sleeptime.tv_sec = 0;
        sleeptime.tv_nsec = 100000000;
        nanosleep(&sleeptime, &remtime);
#else
        usleep(100000);
#endif
    }

    /* Remove the DataReader */
    status = DDS_Subscriber_delete_datareader(chatSubscriber, chatAdmin);
    checkStatus(status, "DDS_Subscriber_delete_datareader");
    
    /* Remove the Subscriber. */
    status = DDS_DomainParticipant_delete_subscriber(participant, chatSubscriber);
    checkStatus(status, "DDS_DomainParticipant_delete_subscriber");
    
    /* Remove the Topics. */
    status = DDS_DomainParticipant_delete_simulated_multitopic(participant, namedMessageTopic);
    checkStatus(status, "DDS_DomainParticipant_delete_simulated_multitopic");
    
    status = DDS_DomainParticipant_delete_topic(participant, nameServiceTopic);
    checkStatus(status, "DDS_DomainParticipant_delete_topic (nameServiceTopic)");
    
    status = DDS_DomainParticipant_delete_topic(participant, chatMessageTopic);
    checkStatus(status, "DDS_DomainParticipant_delete_topic (chatMessageTopic)");
    
    /* De-allocate the QoS policies. */
    DDS_free(reliable_topic_qos);
    DDS_free(setting_topic_qos);
    DDS_free(sub_qos);  /* Note that DDS_free recursively de-allocates all indirections as well!! */

    /* De-allocate the type-names and TypeSupport objects. */
    DDS_free(namedMessageTypeName);
    DDS_free(nameServiceTypeName);
    DDS_free(chatMessageTypeName);
    DDS_free(namedMessageTS);
    DDS_free(nameServiceTS);
    DDS_free(chatMessageTS);
    
    /* Remove the DomainParticipant. */
    status = DDS_DomainParticipantFactory_delete_participant(dpf, participant);
    checkStatus(status, "DDS_DomainParticipantFactory_delete_participant");

    return 0;
}
