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
 * LOGICAL_NAME:    Chatter.c
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C programming language.
 * DATE             june 2007.
 ************************************************************************
 * 
 * This file contains the implementation for the 'Chatter' executable.
 * 
 ***/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "dds_dcps.h"
#include "CheckStatus.h"
#include "Chat.h"

#define MAX_MSG_LEN 256
#define NUM_MSG 10
#define TERMINATION_MESSAGE -1 

#ifdef _VXWORKS
int Chatter_main (int argc, char ** argv);
int Chatter (char * args)
{
   int argc=1;
   char *argv[256];
   char *str1;
   argv[0] = (char*) strdup ("Chatter");
   str1 = (char*) strtok(args, " ");
   while (str1)
   {
      argv[argc] = (char*) strdup (str1);
      argc++;
      str1 = strtok(NULL, " ");
   }
   return Chatter_main (argc, argv);
}
int Chatter_main (int argc, char ** argv)
#else
int main (int argc, char ** argv)
#endif
{
    /* Generic DDS entities */
    DDS_DomainParticipantFactory    dpf;
    DDS_DomainParticipant           participant;
    DDS_Topic                       chatMessageTopic;
    DDS_Topic                       nameServiceTopic;
    DDS_Publisher                   chatPublisher;

    /* QosPolicy holders */
    DDS_TopicQos                    *reliable_topic_qos;
    DDS_TopicQos                    *setting_topic_qos;
    DDS_PublisherQos                *pub_qos;
    DDS_DataWriterQos               *dw_qos;

    /* DDS Identifiers */
    DDS_DomainId_t                  domain = NULL;
    DDS_InstanceHandle_t            userHandle;
    DDS_ReturnCode_t                status;

    /* Type-specific DDS entities */
    Chat_ChatMessageTypeSupport     chatMessageTS;
    Chat_NameServiceTypeSupport     nameServiceTS;
    Chat_ChatMessageDataWriter      talker;
    Chat_NameServiceDataWriter      nameServer;

    /* Sample definitions */
    Chat_ChatMessage                *msg;   /* Example on Heap */
    Chat_NameService                ns;     /* Example on Stack */
    
    /* Others */
    int                             ownID = 1;
    int                             i;
    char                            *chatMessageTypeName = NULL;
    char                            *nameServiceTypeName = NULL;
    char                            *chatterName = NULL;
    char                            *partitionName = NULL;
        
#ifdef INTEGRITY
#ifdef CHATTER_QUIT
    ownID = -1;
#else
    ownID = 1;
#endif
    chatterName = "dds_user";
#else
    /* Options: Chatter [ownID [name]] */
    if (argc > 1) {
        sscanf(argv[1], "%d", &ownID);
        if (argc > 2) {
            chatterName = argv[2];
        }
    }
#endif

    /* Create a DomainParticipantFactory and a DomainParticipant (using Default QoS settings). */
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

    /* Adapt the default PublisherQos to write into the "ChatRoom" Partition. */
    partitionName = "ChatRoom";
    pub_qos = DDS_PublisherQos__alloc();
    checkHandle(pub_qos, "DDS_PublisherQos__alloc");
    status = DDS_DomainParticipant_get_default_publisher_qos (participant, pub_qos);
    checkStatus(status, "DDS_DomainParticipant_get_default_publisher_qos");
    pub_qos->partition.name._length = 1;
    pub_qos->partition.name._maximum = 1;
    pub_qos->partition.name._buffer = DDS_StringSeq_allocbuf (1);
    checkHandle(pub_qos->partition.name._buffer, "DDS_StringSeq_allocbuf");
    pub_qos->partition.name._buffer[0] = DDS_string_alloc ( strlen(partitionName) );
    checkHandle(pub_qos->partition.name._buffer[0], "DDS_string_alloc");
    strcpy (pub_qos->partition.name._buffer[0], partitionName);

    /* Create a Publisher for the chatter application. */
    chatPublisher = DDS_DomainParticipant_create_publisher(participant, pub_qos, NULL, DDS_STATUS_MASK_NONE);
    checkHandle(chatPublisher, "DDS_DomainParticipant_create_publisher");
    
    /* Create a DataWriter for the ChatMessage Topic (using the appropriate QoS). */
    talker = DDS_Publisher_create_datawriter( 
        chatPublisher, 
        chatMessageTopic, 
        DDS_DATAWRITER_QOS_USE_TOPIC_QOS, 
        NULL,
        DDS_STATUS_MASK_NONE);
    checkHandle(talker, "DDS_Publisher_create_datawriter (chatMessage)");
    
    /* Create a DataWriter for the NameService Topic (using the appropriate QoS). */
    dw_qos = DDS_DataWriterQos__alloc();
    checkHandle(dw_qos, "DDS_DataWriterQos__alloc");
    status = DDS_Publisher_get_default_datawriter_qos (chatPublisher, dw_qos);
    checkStatus(status, "DDS_Publisher_get_default_datawriter_qos");
    status = DDS_Publisher_copy_from_topic_qos(chatPublisher, dw_qos, setting_topic_qos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");
    dw_qos->writer_data_lifecycle.autodispose_unregistered_instances = FALSE;
    nameServer = DDS_Publisher_create_datawriter( 
        chatPublisher, 
        nameServiceTopic, 
        dw_qos,
        NULL,
        DDS_STATUS_MASK_NONE);
    checkHandle(nameServer, "DDS_Publisher_create_datawriter (NameService)");

    /* Initialize the NameServer attributes located on stack. */
    ns.userID = ownID;
    ns.name = DDS_string_alloc(Chat_MAX_NAME+1);
    checkHandle(ns.name, "DDS_string_alloc");
    if (chatterName) {
        strncpy (ns.name, chatterName, Chat_MAX_NAME + 1);
    } else {
        snprintf(ns.name, Chat_MAX_NAME+1, "Chatter %d", ownID);
    }

    /* Write the user-information into the system (registering the instance implicitly). */
    status = Chat_NameServiceDataWriter_write(nameServer, &ns, DDS_HANDLE_NIL);
    checkStatus(status, "Chat_ChatMessageDataWriter_write");
    
    /* Initialize the chat messages on Heap. */
    msg = Chat_ChatMessage__alloc();
    checkHandle(msg, "Chat_ChatMessage__alloc");
    msg->userID = ownID;
    msg->index = 0;
    msg->content = DDS_string_alloc(MAX_MSG_LEN);
    checkHandle(msg->content, "DDS_string_alloc");
    if (ownID == TERMINATION_MESSAGE) {
        snprintf (msg->content, MAX_MSG_LEN, "Termination message.");
    } else { 
        snprintf (msg->content, MAX_MSG_LEN, "Hi there, I will send you %d more messages.", NUM_MSG);
    }
    printf("Writing message: %s\n", msg->content);

    /* Register a chat message for this user (pre-allocating resources for it!!) */
    userHandle = Chat_ChatMessageDataWriter_register_instance(talker, msg);

    /* Write a message using the pre-generated instance handle. */
    status = Chat_ChatMessageDataWriter_write(talker, msg, userHandle);
    checkStatus(status, "Chat_ChatMessageDataWriter_write");

    sleep (1); /* do not run so fast! */
 
    /* Write any number of messages, re-using the existing string-buffer: no leak!!. */
    for (i = 1; i <= NUM_MSG && ownID != TERMINATION_MESSAGE; i++) {
        msg->index = i;
        snprintf ( msg->content, MAX_MSG_LEN, "Message no. %d", msg->index);
        printf("Writing message: %s\n", msg->content);
        status = Chat_ChatMessageDataWriter_write(talker, msg, userHandle);
        checkStatus(status, "Chat_ChatMessageDataWriter_write");
        sleep (1); /* do not run so fast! */
    }

    /* Leave the room by disposing and unregistering the message instance. */
    status = Chat_ChatMessageDataWriter_dispose(talker, msg, userHandle);
    checkStatus(status, "Chat_ChatMessageDataWriter_dispose");
    status = Chat_ChatMessageDataWriter_unregister_instance(talker, msg, userHandle);
    checkStatus(status, "Chat_ChatMessageDataWriter_unregister_instance");

    /* Also unregister our name. */
    status = Chat_NameServiceDataWriter_unregister_instance(nameServer, &ns, DDS_HANDLE_NIL);
    checkStatus(status, "Chat_NameServiceDataWriter_unregister_instance");

    /* Release the data-samples. */
    DDS_free(ns.name); /* ns allocated on stack: explicit de-allocation of indirections!! */
    DDS_free(msg);     /* msg allocated on heap: implicit de-allocation of indirections!! */

    /* Remove the DataWriters */
    status = DDS_Publisher_delete_datawriter(chatPublisher, talker);
    checkStatus(status, "DDS_Publisher_delete_datawriter (talker)");
    
    status = DDS_Publisher_delete_datawriter(chatPublisher, nameServer);
    checkStatus(status, "DDS_Publisher_delete_datawriter (nameServer)");
    
    /* Remove the Publisher. */
    status = DDS_DomainParticipant_delete_publisher(participant, chatPublisher);
    checkStatus(status, "DDS_DomainParticipant_delete_publisher");
    
    /* Remove the Topics. */
    status = DDS_DomainParticipant_delete_topic(participant, nameServiceTopic);
    checkStatus(status, "DDS_DomainParticipant_delete_topic (nameServiceTopic)");
    
    status = DDS_DomainParticipant_delete_topic(participant, chatMessageTopic);
    checkStatus(status, "DDS_DomainParticipant_delete_topic (chatMessageTopic)");
    
    /* De-allocate the QoS policies. */
    DDS_free(reliable_topic_qos);
    DDS_free(setting_topic_qos);
    DDS_free(pub_qos);  /* Note that DDS_free recursively de-allocates all indirections as well!! */

    /* De-allocate the type-names and TypeSupport objects. */
    DDS_free(nameServiceTypeName);
    DDS_free(chatMessageTypeName);
    DDS_free(nameServiceTS);
    DDS_free(chatMessageTS);
    
    /* Remove the DomainParticipant. */
    status = DDS_DomainParticipantFactory_delete_participant(dpf, participant);
    checkStatus(status, "DDS_DomainParticipantFactory_delete_participant");

    printf("Completed chatter example.\n");
    fflush(stdout);
    return 0;
}
