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
 * LOGICAL_NAME:    UserLoad.c
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C programming language.
 * DATE             june 2007.
 ************************************************************************
 * 
 * This file contains the implementation for the 'UserLoad' executable.
 * 
 ***/

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

#include "dds_dcps.h"
#include "CheckStatus.h"
#include "Chat.h"

#define TERMINATION_MESSAGE -1 

/* entities required by all threads. */
static DDS_GuardCondition           escape;

/* Sleeper thread: sleeps 60 seconds and then triggers the WaitSet. */
void *
delayedEscape(
    void *arg)
{
    DDS_ReturnCode_t status;
    
    sleep(60);     /* wait for 60 sec. */
    status = DDS_GuardCondition_set_trigger_value(escape, TRUE);
    checkStatus(status, "DDS_GuardCondition_set_trigger_value");

    return NULL;
}

#ifdef _VXWORKS
int UserLoad_main (int argc, char ** argv);
int UserLoad (char * args)
{
   int argc=1;
   char *argv[256];
   char *str1;
   argv[0] = (char*) strdup ("UserLoad");
   str1 = (char*) strtok(args, " ");
   while (str1)
   {
      argv[argc] = (char*) strdup (str1);
      argc++;
      str1 = strtok(NULL, " ");
   }
   return UserLoad_main (argc, argv);
}
int UserLoad_main (int argc, char ** argv)
#else
int main (int argc, char ** argv)
#endif
{
    /* Generic DDS entities */
    DDS_DomainParticipant           participant;
    DDS_Topic                       chatMessageTopic;
    DDS_Topic                       nameServiceTopic;
    DDS_Subscriber                  chatSubscriber;
    DDS_QueryCondition              singleUser;
    DDS_ReadCondition               newUser;
    DDS_StatusCondition             leftUser;
    DDS_GuardCondition              guard;
    DDS_WaitSet                     userLoadWS;
    DDS_LivelinessChangedStatus     livChangStatus;

    /* QosPolicy holders */
    DDS_TopicQos                    *setting_topic_qos;
    DDS_TopicQos                    *reliable_topic_qos;
    DDS_SubscriberQos               *sub_qos;
    DDS_DataReaderQos               *message_qos;

    /* DDS Identifiers */
    DDS_DomainId_t                  domain = NULL;
    DDS_ReturnCode_t                status;
    DDS_ConditionSeq                *guardList = NULL;
    DDS_Duration_t                  timeout = DDS_DURATION_INFINITE;

    /* Type-specific DDS entities */
    Chat_ChatMessageTypeSupport     chatMessageTS;
    Chat_NameServiceTypeSupport     nameServiceTS;
    Chat_NameServiceDataReader      nameServer;
    Chat_ChatMessageDataReader      loadAdmin;
    DDS_sequence_Chat_ChatMessage   msgList = { 0, 0, DDS_OBJECT_NIL, FALSE };
    DDS_sequence_Chat_NameService   nsList = { 0, 0, DDS_OBJECT_NIL, FALSE };
    DDS_SampleInfoSeq               infoSeq    = { 0, 0, DDS_OBJECT_NIL, FALSE };
    DDS_SampleInfoSeq               infoSeq2   = { 0, 0, DDS_OBJECT_NIL, FALSE };

    /* Others */
    DDS_StringSeq                   args;
    int                             closed = 0;
    DDS_unsigned_long               i, j;
    DDS_long                        prevCount = 0;
    char                            *partitionName;
    char                            *chatMessageTypeName = NULL;
    char                            *nameServiceTypeName = NULL;
    pthread_t                       tid;
    pthread_attr_t                  tattr;
 
    printf("Starting UserLoad : disconnect a Chatter with userID = -1 to close it....\n\n");

    fflush(stdout);
    /* Create a DomainParticipant (using the 'TheParticipantFactory' convenience macro). */
    participant = DDS_DomainParticipantFactory_create_participant (
        DDS_TheParticipantFactory, 
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
    sub_qos->partition.name._buffer[0] = DDS_string_alloc (strlen(partitionName) + 1);
    checkHandle(sub_qos->partition.name._buffer[0], "DDS_string_alloc");
    strcpy (sub_qos->partition.name._buffer[0], partitionName);

    /* Create a Subscriber for the UserLoad application. */
    chatSubscriber = DDS_DomainParticipant_create_subscriber(participant, sub_qos, NULL, DDS_STATUS_MASK_NONE);
    checkHandle(chatSubscriber, "DDS_DomainParticipant_create_subscriber");
    
    /* Create a DataReader for the NameService Topic (using the appropriate QoS). */
    nameServer = DDS_Subscriber_create_datareader( 
        chatSubscriber, 
        nameServiceTopic, 
        DDS_DATAREADER_QOS_USE_TOPIC_QOS, 
        NULL,
        DDS_STATUS_MASK_NONE);
    checkHandle(nameServer, "DDS_Subscriber_create_datareader (NameService)");

    /* Adapt the DataReaderQos for the ChatMessageDataReader to keep track of all messages. */
    message_qos = DDS_DataReaderQos__alloc();
    checkHandle(message_qos, "DDS_DataReaderQos__alloc");
    status = DDS_Subscriber_get_default_datareader_qos(chatSubscriber, message_qos);
    checkStatus(status, "DDS_Subscriber_get_default_datareader_qos");
    status = DDS_Subscriber_copy_from_topic_qos(chatSubscriber, message_qos, reliable_topic_qos);
    checkStatus(status, "DDS_Subscriber_copy_from_topic_qos");
    message_qos->history.kind = DDS_KEEP_ALL_HISTORY_QOS;

    /* Create a DataReader for the ChatMessage Topic (using the appropriate QoS). */
    loadAdmin = DDS_Subscriber_create_datareader( 
        chatSubscriber, 
        chatMessageTopic, 
        message_qos, 
        NULL,
        DDS_STATUS_MASK_NONE);
    checkHandle(loadAdmin, "DDS_Subscriber_create_datareader (ChatMessage)");
    
    /* Initialize the Query Arguments. */
    args._length = 1;
    args._maximum = 1;
    args._buffer = DDS_StringSeq_allocbuf(1);
    checkHandle(args._buffer, "DDS_StringSeq_allocbuf");
    args._buffer[0] = DDS_string_alloc (12);  /* Enough for maximum size numbers. */
    checkHandle(args._buffer[0], "DDS_string_alloc");
    sprintf(args._buffer[0], "%d", 0);
    
    /* Create a QueryCondition that will contain all messages with userID=ownID */
    singleUser = DDS_DataReader_create_querycondition( 
        loadAdmin, 
        DDS_ANY_SAMPLE_STATE, 
        DDS_ANY_VIEW_STATE, 
        DDS_ANY_INSTANCE_STATE, 
        "userID=%0", 
        &args);
    checkHandle(singleUser, "DDS_DataReader_create_querycondition (singleUser Query)");
    
    /* Create a ReadCondition that will contain new users only */
    newUser = DDS_DataReader_create_readcondition( 
        nameServer, 
        DDS_NOT_READ_SAMPLE_STATE, 
        DDS_NEW_VIEW_STATE, 
        DDS_ALIVE_INSTANCE_STATE);
    checkHandle(newUser, "DDS_DataReader_create_readcondition (newUser)");

    /* Obtain a StatusCondition that triggers only when a Writer changes Liveliness */
    leftUser = DDS_DataReader_get_statuscondition(loadAdmin);
    checkHandle(leftUser, "DDS_DataReader_get_statuscondition");
    status = DDS_StatusCondition_set_enabled_statuses(leftUser, DDS_LIVELINESS_CHANGED_STATUS);
    checkStatus(status, "DDS_StatusCondition_set_enabled_statuses");

    /* Create a bare guard which will be used to close the room */
    escape = DDS_GuardCondition__alloc();
    checkHandle(escape, "DDS_GuardCondition__alloc");

    /* Create a waitset and add the ReadConditions */
    userLoadWS = DDS_WaitSet__alloc();
    checkHandle(userLoadWS, "DDS_WaitSet__alloc");
    status = DDS_WaitSet_attach_condition(userLoadWS, newUser);
    checkStatus(status, "DDS_WaitSet_attach_condition (newUser)");
    status = DDS_WaitSet_attach_condition(userLoadWS, leftUser);
    checkStatus(status, "DDS_WaitSet_attach_condition (leftUser)");
    status = DDS_WaitSet_attach_condition(userLoadWS, escape);
    checkStatus(status, "DDS_WaitSet_attach_condition (escape)");
    
    /* Initialize and pre-allocate the GuardList used to obtain the triggered Conditions. */
    guardList = DDS_ConditionSeq__alloc();
    checkHandle(guardList, "DDS_ConditionSeq__alloc");
    guardList->_maximum = 3;
    guardList->_length = 0;
    guardList->_buffer = DDS_ConditionSeq_allocbuf(3);
    checkHandle(guardList->_buffer, "DDS_ConditionSeq_allocbuf");
    
    /* Remove all known Users that are not currently active. */
    status = Chat_NameServiceDataReader_take(
        nameServer,
        &nsList, 
        &infoSeq, 
        DDS_LENGTH_UNLIMITED, 
        DDS_ANY_SAMPLE_STATE, 
        DDS_ANY_VIEW_STATE, 
        DDS_NOT_ALIVE_INSTANCE_STATE);
    checkStatus(status, "Chat_NameServiceDataReader_take");
    status = Chat_NameServiceDataReader_return_loan(nameServer, &nsList, &infoSeq);
    checkStatus(status, "Chat_NameServiceDataReader_return_loan");
    
    /* Start the sleeper thread. */
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
    pthread_create (&tid, &tattr, delayedEscape, NULL);
    pthread_attr_destroy(&tattr);

    while (!closed) {
        /* Wait until at least one of the Conditions in the waitset triggers. */
        status = DDS_WaitSet_wait(userLoadWS, guardList, &timeout);
        checkStatus(status, "DDS_WaitSet_wait");

        /* Walk over all guards to display information */
        for (i = 0; i < guardList->_length; i++) {
            guard = guardList->_buffer[i];
            if (guard == newUser) {
                /* The newUser ReadCondition contains data */
                status = Chat_NameServiceDataReader_read_w_condition( 
                    nameServer, 
                    &nsList, 
                    &infoSeq, 
                    DDS_LENGTH_UNLIMITED, 
                    newUser);
                checkStatus(status, "Chat_NameServiceDataReader_read_w_condition");
                
                for (j = 0; j < nsList._length; j++) {
                    printf ("New user: %s\n", nsList._buffer[j].name);
                }
                status = Chat_NameServiceDataReader_return_loan(nameServer, &nsList, &infoSeq);
                checkStatus(status, "Chat_NameServiceDataReader_return_loan");

            } else if (guard == leftUser) {
                /* Some liveliness has changed (either a DataWriter joined or a DataWriter left) */
                status = DDS_DataReader_get_liveliness_changed_status(loadAdmin, &livChangStatus);
                checkStatus(status, "DDS_DataReader_get_liveliness_changed_status");
                if (livChangStatus.alive_count < prevCount) {
                    /* A user has left the ChatRoom, since a DataWriter lost its liveliness */
                    /* Take the effected users so tey will not appear in the list later on. */
                    status = Chat_NameServiceDataReader_take( 
                        nameServer, 
                        &nsList, 
                        &infoSeq, 
                        DDS_LENGTH_UNLIMITED, 
                        DDS_ANY_SAMPLE_STATE, 
                        DDS_ANY_VIEW_STATE, 
                        DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE);
                    checkStatus(status, "Chat_NameServiceDataReader_take");
    
                    for (j = 0; j < nsList._length; j++) {
                        /* re-apply query arguments */
                        sprintf(args._buffer[0], "%d", nsList._buffer[j].userID);
                        status = DDS_QueryCondition_set_query_parameters(singleUser, &args);
                        checkStatus(status, "DDS_QueryCondition_set_query_parameters");
    
                        /* Read this users history */
                        status = Chat_ChatMessageDataReader_take_w_condition( 
                            loadAdmin, 
                            &msgList, 
                            &infoSeq2, 
                            DDS_LENGTH_UNLIMITED, 
                            singleUser);
                        checkStatus(status, "Chat_ChatMessageDataReader_take_w_condition");
                                                
                        /* Display the user and his history */
                        printf (
                            "Departed user %s has sent %d messages\n", 
                            nsList._buffer[j].name,
                            msgList._length);
                        status = Chat_ChatMessageDataReader_return_loan(loadAdmin, &msgList, &infoSeq2);
                        checkStatus(status, "Chat_ChatMessageDataReader_return_loan");
                        if(nsList._buffer[j].userID == TERMINATION_MESSAGE) {
                           printf("Termination message received: exiting...\n");
                           closed = 1;
                        }
                    }
                    status = Chat_NameServiceDataReader_return_loan(nameServer, &nsList, &infoSeq);
                    checkStatus(status, "Chat_NameServiceDataReader_return_loan");
                }
                prevCount = livChangStatus.alive_count;

            } else if (guard == escape) {
                printf ("UserLoad has terminated.\n");
                closed = 1;
            }
            else
            {
                assert(0);
            };
        } /* for */
    } /* while (!closed) */

    /* Remove all Conditions from the WaitSet. */
    status = DDS_WaitSet_detach_condition(userLoadWS, escape);
    checkStatus(status, "DDS_WaitSet_detach_condition (escape)");
    status = DDS_WaitSet_detach_condition(userLoadWS, leftUser);
    checkStatus(status, "DDS_WaitSet_detach_condition (leftUser)");
    status = DDS_WaitSet_detach_condition(userLoadWS, newUser);
    checkStatus(status, "DDS_WaitSet_detach_condition (newUser)");

    /* Free all resources */
    DDS_free(guardList);
    DDS_free(args._buffer);
    DDS_free(userLoadWS);
    DDS_free(escape);
    DDS_free(setting_topic_qos);
    DDS_free(reliable_topic_qos);
    DDS_free(nameServiceTypeName);
    DDS_free(chatMessageTypeName);
    DDS_free(nameServiceTS);
    DDS_free(chatMessageTS);    
    status = DDS_DomainParticipant_delete_contained_entities(participant);
    checkStatus(status, "DDS_DomainParticipant_delete_contained_entities");
    status = DDS_DomainParticipantFactory_delete_participant(
        DDS_TheParticipantFactory, 
        participant);
    checkStatus(status, "DDS_DomainParticipantFactory_delete_participant");
    
    return 0;
}
