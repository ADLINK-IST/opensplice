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
 * LOGICAL_NAME:    multitopic.c
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C programming language.
 * DATE             june 2007.
 ************************************************************************
 * 
 * This file contains the implementation for all operations required to simulate 
 * the MultiTopic behavior.
 * 
 ***/


#include <string.h>

#include "multitopic.h"
#include "Chat.h"
#include "dds_dcps.h"
#include "CheckStatus.h"

#define MAX_INT_LENGTH 15

/* DataReaderListener */
static struct DDS_DataReaderListener *msgListener = NULL;

struct MsgListenerState
{
    /* Type-specific DDS entities */
    Chat_ChatMessageDataReader       chatMessageDR;
    Chat_NameServiceDataReader       nameServiceDR;
    Chat_NamedMessageDataWriter      namedMessageDW;
    
    /* Query related stuff */
    DDS_QueryCondition               nameFinder;
    DDS_StringSeq                    *nameFinderParams;
};

/* Generic DDS entities */
static DDS_Topic                       chatMessageTopic;
static DDS_Topic                       nameServiceTopic;
static DDS_ContentFilteredTopic        filteredMessageTopic;
static DDS_Topic                       namedMessageTopic;
static DDS_Subscriber                  multiSub;
static DDS_Publisher                   multiPub;


DDS_MultiTopic
DDS_DomainParticipant_create_simulated_multitopic (
    DDS_DomainParticipant participant,
    const DDS_char *name,
    const DDS_char *type_name,
    const DDS_char *subscription_expression,
    const DDS_StringSeq *expression_parameters )
{
    /* Type-specific DDS entities */
    static Chat_ChatMessageDataReader       chatMessageDR;
    static Chat_NameServiceDataReader       nameServiceDR;
    static Chat_NamedMessageDataWriter      namedMessageDW;
    
    /* Query related stuff */
    static DDS_QueryCondition               nameFinder;
    static DDS_StringSeq                    *nameFinderParams;
    
    /* QosPolicy holders */
    DDS_TopicQos                    *namedMessageQos;
    DDS_SubscriberQos               *sub_qos;    
    DDS_PublisherQos                *pub_qos;

    /* Others */
    const char                      *partitionName = "ChatRoom";
    const char                      *nameFinderExpr;
    struct MsgListenerState         *listener_state;
    DDS_Duration_t                  infiniteTimeOut  = DDS_DURATION_INFINITE;
    DDS_ReturnCode_t                status;

    /* Lookup both components that constitute the multi-topic. */
    chatMessageTopic = DDS_DomainParticipant_find_topic(
        participant, 
        "Chat_ChatMessage", 
        &infiniteTimeOut);
    checkHandle(chatMessageTopic, "DDS_DomainParticipant_find_topic (Chat_ChatMessage)");

    nameServiceTopic = DDS_DomainParticipant_find_topic(
        participant, 
        "Chat_NameService", 
        &infiniteTimeOut);
    checkHandle(nameServiceTopic, "DDS_DomainParticipant_find_topic (Chat_NameService)");

    /* Create a ContentFilteredTopic to filter out our own ChatMessages. */
    filteredMessageTopic = DDS_DomainParticipant_create_contentfilteredtopic(
        participant, 
        "Chat_FilteredMessage", 
        chatMessageTopic,
        "userID <> %0",
        expression_parameters);
    checkHandle(filteredMessageTopic, "DDS_DomainParticipant_create_contentfilteredtopic");
        

    /* Adapt the default SubscriberQos to read from the "ChatRoom" Partition. */
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

    /* Create a private Subscriber for the multitopic simulator. */
    multiSub = DDS_DomainParticipant_create_subscriber(participant, sub_qos, NULL, DDS_STATUS_MASK_NONE);
    checkHandle(multiSub, "DDS_DomainParticipant_create_subscriber (for multitopic)");
    
    /* Create a DataReader for the FilteredMessage Topic (using the appropriate QoS). */
    chatMessageDR = DDS_Subscriber_create_datareader( 
        multiSub, 
        filteredMessageTopic, 
        DDS_DATAREADER_QOS_USE_TOPIC_QOS, 
        NULL,
        DDS_STATUS_MASK_NONE);
    checkHandle(chatMessageDR, "DDS_Subscriber_create_datareader (ChatMessage)");
    
    /* Create a DataReader for the nameService Topic (using the appropriate QoS). */
    nameServiceDR = DDS_Subscriber_create_datareader( 
        multiSub, 
        nameServiceTopic, 
        DDS_DATAREADER_QOS_USE_TOPIC_QOS, 
        NULL,
        DDS_STATUS_MASK_NONE);
    checkHandle(nameServiceDR, "DDS_Subscriber_create_datareader (NameService)");
    
    /* Define the SQL expression (using a parameterized value). */
    nameFinderExpr = "userID = %0";
    
    /* Allocate and assign the query parameters. */
    nameFinderParams = DDS_StringSeq__alloc();
    checkHandle(nameFinderParams, "DDS_StringSeq__alloc");
    nameFinderParams->_length = 1;
    nameFinderParams->_maximum = 1;
    nameFinderParams->_buffer = DDS_StringSeq_allocbuf (1);
    checkHandle(nameFinderParams->_buffer, "DDS_StringSeq_allocbuf");
    nameFinderParams->_buffer[0] = DDS_string_alloc ( MAX_INT_LENGTH  );
    checkHandle(nameFinderParams->_buffer[0], "DDS_string_alloc");
    strcpy(nameFinderParams->_buffer[0], "0");
    DDS_sequence_set_release(nameFinderParams, TRUE);

    /* Create a QueryCondition to only read corresponding nameService information by key-value. */
    nameFinder = DDS_DataReader_create_querycondition( 
        nameServiceDR, 
        DDS_ANY_SAMPLE_STATE, 
        DDS_ANY_VIEW_STATE, 
        DDS_ANY_INSTANCE_STATE,
        nameFinderExpr, 
        nameFinderParams);
    checkHandle(nameFinder, "DDS_DataReader_create_querycondition (nameFinder)");
    
    /* Create the Topic that simulates the multi-topic (use Qos from chatMessage).*/
    namedMessageQos = DDS_TopicQos__alloc();
    checkHandle(namedMessageQos, "DDS_TopicQos__alloc");
    status = DDS_Topic_get_qos(chatMessageTopic, namedMessageQos);
    checkStatus(status, "DDS_Topic_get_qos");
    
    /* Create the NamedMessage Topic whose samples simulate the MultiTopic */
    namedMessageTopic = DDS_DomainParticipant_create_topic( 
        participant, 
        "Chat_NamedMessage", 
        type_name, 
        namedMessageQos, 
        NULL,
        DDS_STATUS_MASK_NONE);
    checkHandle(namedMessageTopic, "DDS_DomainParticipant_create_topic (NamedMessage)");
    
    /* Adapt the default PublisherQos to write into the "ChatRoom" Partition. */
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

    /* Create a private Publisher for the multitopic simulator. */
    multiPub = DDS_DomainParticipant_create_publisher(participant, pub_qos, NULL, DDS_STATUS_MASK_NONE);
    checkHandle(multiPub, "DDS_DomainParticipant_create_publisher (for multitopic)");
    
    /* Create a DataWriter for the multitopic. */
    namedMessageDW = DDS_Publisher_create_datawriter( 
        multiPub, 
        namedMessageTopic, 
        DDS_DATAWRITER_QOS_USE_TOPIC_QOS, 
        NULL,
        DDS_STATUS_MASK_NONE);
    checkHandle(namedMessageDW, "DDS_Publisher_create_datawriter (NamedMessage)");

    /* Allocate the DataReaderListener interface. */
    msgListener = DDS_DataReaderListener__alloc();
    checkHandle(msgListener, "DDS_DataReaderListener__alloc");

    /* Fill the listener_data with pointers to all entities needed by the Listener implementation. */
    listener_state = malloc(sizeof(struct MsgListenerState));
    checkHandle(listener_state, "malloc");
    listener_state->chatMessageDR = chatMessageDR;
    listener_state->nameServiceDR = nameServiceDR;
    listener_state->namedMessageDW = namedMessageDW;
    listener_state->nameFinder = nameFinder;
    listener_state->nameFinderParams = nameFinderParams;
    msgListener->listener_data = listener_state;

    /* Assign the function pointer attributes to their implementation functions. */
    msgListener->on_data_available = (void (*)(void *, DDS_DataReader)) on_message_available;
    msgListener->on_requested_deadline_missed = NULL;
    msgListener->on_requested_incompatible_qos = NULL;
    msgListener->on_sample_rejected = NULL;
    msgListener->on_liveliness_changed = NULL;
    msgListener->on_subscription_matched = NULL;
    msgListener->on_sample_lost = NULL;
    
    /* Attach the DataReaderListener to the DataReader, only enabling the data_available event. */
    status = DDS_DataReader_set_listener(chatMessageDR, msgListener, DDS_DATA_AVAILABLE_STATUS);
    checkStatus(status, "DDS_DataReader_set_listener");
    
    /* Free up all resources that are no longer needed. */
    DDS_free(namedMessageQos);
    DDS_free(sub_qos);
    DDS_free(pub_qos);
    
    /* Return the simulated Multitopic. */
    return namedMessageTopic;
}

DDS_ReturnCode_t
DDS_DomainParticipant_delete_simulated_multitopic(
    DDS_DomainParticipant participant,
    DDS_TopicDescription smt 
)
{
    DDS_ReturnCode_t status;
    struct MsgListenerState *listener_state;
    
    /* Obtain all entities mentioned in the listener state. */
    listener_state = (struct MsgListenerState *) msgListener->listener_data;

    /* Remove the DataWriter */
    status = DDS_Publisher_delete_datawriter(multiPub, listener_state->namedMessageDW);
    checkStatus(status, "DDS_Publisher_delete_datawriter");
    
    /* Remove the Publisher. */
    status = DDS_DomainParticipant_delete_publisher(participant, multiPub);
    checkStatus(status, "DDS_DomainParticipant_delete_publisher");
    
    /* Remove the QueryCondition and its parameters. */
    DDS_free(listener_state->nameFinderParams);
    status = DDS_DataReader_delete_readcondition(
        listener_state->nameServiceDR,
        listener_state->nameFinder);
    checkStatus(status, "DDS_DataReader_delete_readcondition");
    
    /* Remove the DataReaders. */
    status = DDS_Subscriber_delete_datareader(multiSub, listener_state->nameServiceDR);
    checkStatus(status, "DDS_Subscriber_delete_datareader");
    status = DDS_Subscriber_delete_datareader(multiSub, listener_state->chatMessageDR);
    checkStatus(status, "DDS_Subscriber_delete_datareader");
    
    /* Remove the DataReaderListener and its state. */
    free(listener_state);
    DDS_free(msgListener);
    
    /* Remove the Subscriber. */
    status = DDS_DomainParticipant_delete_subscriber(participant, multiSub);
    checkStatus(status, "DDS_DomainParticipant_delete_subscriber");
    
    /* Remove the ContentFilteredTopic. */
    status = DDS_DomainParticipant_delete_contentfilteredtopic(participant, filteredMessageTopic);
    checkStatus(status, "DDS_DomainParticipant_delete_contentfilteredtopic");
    
    /* Remove all other topics. */
    status = DDS_DomainParticipant_delete_topic(participant, namedMessageTopic);
    checkStatus(status, "DDS_DomainParticipant_delete_topic (namedMessageTopic)");
    status = DDS_DomainParticipant_delete_topic(participant, nameServiceTopic);
    checkStatus(status, "DDS_DomainParticipant_delete_topic (nameServiceTopic)");
    status = DDS_DomainParticipant_delete_topic(participant, chatMessageTopic);
    checkStatus(status, "DDS_DomainParticipant_delete_topic (chatMessageTopic)");
    
    return status;
}


/* Implementation for the callback function "on_data_available". */
void on_message_available (
    void *listener_data,
    DDS_DataReader reader )
{
    struct MsgListenerState        *listener_state;
    DDS_sequence_Chat_ChatMessage  msgSeq      = { 0, 0, DDS_OBJECT_NIL, FALSE };
    DDS_sequence_Chat_NameService  nameSeq     = { 0, 0, DDS_OBJECT_NIL, FALSE };
    DDS_SampleInfoSeq              infoSeq1    = { 0, 0, DDS_OBJECT_NIL, FALSE };
    DDS_SampleInfoSeq              infoSeq2    = { 0, 0, DDS_OBJECT_NIL, FALSE };
    DDS_ReturnCode_t               status;
    DDS_unsigned_long              i;
    DDS_long                       previous = 0x80000000;
    DDS_string                     userName = DDS_string_alloc(1);

    
    /* Obtain all entities mentioned in the listener state. */
    listener_state = (struct MsgListenerState *) listener_data;

    /* Take all messages. */
    status = Chat_ChatMessageDataReader_take( 
        listener_state->chatMessageDR, 
        &msgSeq, 
        &infoSeq1, 
        DDS_LENGTH_UNLIMITED, 
        DDS_ANY_SAMPLE_STATE, 
        DDS_ANY_VIEW_STATE, 
        DDS_ANY_INSTANCE_STATE);
    checkStatus(status, "Chat_ChatMessageDataReader_take");
    
    /* For each message, extract the key-field and find the corresponding name. */
    for (i = 0; i < msgSeq._length; i++)
    {
        if (infoSeq1._buffer[i].valid_data)
        {
            Chat_NamedMessage joinedSample;
        
            /* Find the corresponding named message. */
            if (msgSeq._buffer[i].userID != previous)
            {
                previous = msgSeq._buffer[i].userID;
                snprintf(listener_state->nameFinderParams->_buffer[0], MAX_INT_LENGTH, "%d", previous);
                status = DDS_QueryCondition_set_query_parameters(
                    listener_state->nameFinder, 
                    listener_state->nameFinderParams);
                checkStatus(status, "DDS_QueryCondition_set_query_parameters");
                status = Chat_NameServiceDataReader_read_w_condition( 
                    listener_state->nameServiceDR, 
                    &nameSeq, 
                    &infoSeq2, 
                    DDS_LENGTH_UNLIMITED, 
                    listener_state->nameFinder);
                checkStatus(status, "Chat_NameServiceDataReader_read_w_condition");
                
                /* Extract Name (there should only be one result). */
                DDS_free(userName);
                if (status == DDS_RETCODE_NO_DATA)
                {
                    userName = DDS_string_alloc(40);
                    checkHandle(userName, "DDS_string_alloc");
                    snprintf(userName, 40, "Name not found!! id = %d", previous);
                }
                else
                {
                    userName = DDS_string_alloc( strlen(nameSeq._buffer[0].name) );
                    checkHandle(userName, "DDS_string_alloc");
                    strcpy(userName, nameSeq._buffer[0].name);
                }
    
                /* Release the name sample again. */                
                status = Chat_NameServiceDataReader_return_loan(listener_state->nameServiceDR, &nameSeq, &infoSeq2);
                checkStatus(status, "Chat_NameServiceDataReader_return_loan");
            }
            /* Write merged Topic with both userName and userID. */
            /* StringCopy not required since sample runs out of scope before string is released. */
            joinedSample.userName = userName;
            joinedSample.userID = msgSeq._buffer[i].userID;
            joinedSample.index = msgSeq._buffer[i].index;
            joinedSample.content = msgSeq._buffer[i].content;
            status = Chat_NamedMessageDataWriter_write(
                listener_state->namedMessageDW, 
                &joinedSample, 
                DDS_HANDLE_NIL);
            checkStatus(status, "Chat_NamedMessageDataWriter_write");
        }
    }
    status = Chat_ChatMessageDataReader_return_loan(listener_state->chatMessageDR, &msgSeq, &infoSeq1);
    checkStatus(status, "Chat_ChatMessageDataReader_return_loan");
}


