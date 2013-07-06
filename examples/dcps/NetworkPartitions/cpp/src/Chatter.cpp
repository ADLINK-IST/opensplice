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

/************************************************************************
 * LOGICAL_NAME:    Chatter.cpp
 * FUNCTION:        OpenSplice NetworkPartition example code.
 * MODULE:          NetworkPartition example.
 * DATE             june 2007.
 ************************************************************************
 *
 * This file contains the implementation for the 'Chatter' executable.
 *
 ***/
#include <string>
#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#else
#include "os_stdlib.h"
#include <Windows.h>

static void sleep(int secs)
{
    Sleep(secs * 1000);
}
#endif
#include "ccpp_dds_dcps.h"
#include "CheckStatus.h"
#include "ccpp_NetworkPartitionsData.h"
#include "example_main.h"

#define MAX_MSG_LEN 256
#define NUM_MSG 10
#define TERMINATION_MESSAGE -1

using namespace DDS;
using namespace NetworkPartitionsData;

int
OSPL_MAIN (
    int argc,
    char *argv[])
{
    /* Generic DDS entities */
    DomainParticipantFactory_var    dpf;
    DomainParticipant_var           participant;
    Topic_var                       chatMessageTopic;
    Publisher_var                   chatPublisher;
    DataWriter_ptr                  parentWriter;

    /* QosPolicy holders */
    TopicQos                        reliable_topic_qos;
    PublisherQos                    pub_qos;
    DataWriterQos                   dw_qos;

    /* DDS Identifiers */
    DomainId_t                      domain = 0;
    InstanceHandle_t                userHandle;
    ReturnCode_t                    status;

    /* Type-specific DDS entities */
    ChatMessageTypeSupport_var      chatMessageTS;
    ChatMessageDataWriter_var       talker;

    /* Sample definitions */
    ChatMessage                     *msg;   /* Example on Heap */

    /* Others */
    int                             ownID = 1;
    int                             i;
    const char                      *partitionName = "ChatRoom1";
    char                            *chatMessageTypeName = NULL;
    char                            buf[MAX_MSG_LEN];

#ifdef INTEGRITY
#ifdef CHATTER_QUIT
    ownID = -1;
#else
    ownID = 1;
#endif
#else
    /* Options: Chatter [ownID] */
    if (argc > 1) {
        ownID = atoi(argv[1]);
    }
#endif

    /* Create a DomainParticipantFactory and a DomainParticipant (using Default QoS settings. */
    dpf = DomainParticipantFactory::get_instance ();
    checkHandle(dpf.in(), "DDS::DomainParticipantFactory::get_instance");
    participant = dpf->create_participant(domain, PARTICIPANT_QOS_DEFAULT, NULL, STATUS_MASK_NONE);
    checkHandle(participant.in(), "DDS::DomainParticipantFactory::create_participant");

    /* Register the required datatype for ChatMessage. */
    chatMessageTS = new ChatMessageTypeSupport();
    checkHandle(chatMessageTS.in(), "new ChatMessageTypeSupport");
    chatMessageTypeName = chatMessageTS->get_type_name();
    status = chatMessageTS->register_type(
        participant.in(),
        chatMessageTypeName);
    checkStatus(status, "NetworkPartitionsData::ChatMessageTypeSupport::register_type");

    /* Set the ReliabilityQosPolicy to RELIABLE. */
    status = participant->get_default_topic_qos(reliable_topic_qos);
    checkStatus(status, "DDS::DomainParticipant::get_default_topic_qos");
    reliable_topic_qos.reliability.kind = RELIABLE_RELIABILITY_QOS;

    /* Make the tailored QoS the new default. */
    status = participant->set_default_topic_qos(reliable_topic_qos);
    checkStatus(status, "DDS::DomainParticipant::set_default_topic_qos");

    /* Use the changed policy when defining the ChatMessage topic */
    chatMessageTopic = participant->create_topic(
        "Chat_ChatMessage",
        chatMessageTypeName,
        reliable_topic_qos,
        NULL,
        STATUS_MASK_NONE);
    checkHandle(chatMessageTopic.in(), "DDS::DomainParticipant::create_topic (ChatMessage)");

    /* Adapt the default PublisherQos to write into the "ChatRoom1" Partition. */
    status = participant->get_default_publisher_qos (pub_qos);
    checkStatus(status, "DDS::DomainParticipant::get_default_publisher_qos");
    pub_qos.partition.name.length(1);
    pub_qos.partition.name[0] = partitionName;

    /* Create a Publisher for the chatter application. */
    chatPublisher = participant->create_publisher(pub_qos, NULL, STATUS_MASK_NONE);
    checkHandle(chatPublisher.in(), "DDS::DomainParticipant::create_publisher");

    /* Create a DataWriter for the ChatMessage Topic (using the appropriate QoS). */
    parentWriter = chatPublisher->create_datawriter(
        chatMessageTopic.in(),
        DATAWRITER_QOS_USE_TOPIC_QOS,
        NULL,
        STATUS_MASK_NONE);
    checkHandle(parentWriter, "DDS::Publisher::create_datawriter (chatMessage)");

    /* Narrow the abstract parent into its typed representative. */
    talker = ChatMessageDataWriter::_narrow(parentWriter);
    checkHandle(talker.in(), "NetworkPartitionsData::ChatMessageDataWriter::_narrow");

    /* Initialize the chat messages on Heap. */
    msg = new ChatMessage();
    checkHandle(msg, "new ChatMessage");
    msg->userID = ownID;
    msg->index = 0;
    if (ownID == TERMINATION_MESSAGE) {
        snprintf(buf, MAX_MSG_LEN, "Termination message.");
    } else {
        snprintf(buf, MAX_MSG_LEN, "Hi there, I will send you %d more messages.", NUM_MSG);
    }
    msg->content = string_dup(buf);
    cout << "Writing message: \"" << msg->content  << "\"" << endl;

    /* Register a chat message for this user (pre-allocating resources for it!!) */
    userHandle = talker->register_instance(*msg);

    /* Write a message using the pre-generated instance handle. */
    status = talker->write(*msg, userHandle);
    checkStatus(status, "NetworkPartitionsData::ChatMessageDataWriter::write");

    sleep (1); /* do not run so fast! */

    /* Write any number of messages, re-using the existing string-buffer: no leak!!. */
    for (i = 1; i <= NUM_MSG && ownID != TERMINATION_MESSAGE; i++) {
        msg->index = i;
        snprintf(buf, MAX_MSG_LEN, "Message no. %d", i);
        msg->content = string_dup(buf);
        cout << "Writing message: \"" << msg->content << "\"" << endl;
        status = talker->write(*msg, userHandle);
        checkStatus(status, "NetworkPartitionsData::ChatMessageDataWriter::write");
        sleep (1); /* do not run so fast! */
    }

    /* Leave the room by disposing and unregistering the message instance. */
    status = talker->dispose(*msg, userHandle);
    checkStatus(status, "NetworkPartitionsData::ChatMessageDataWriter::dispose");
    status = talker->unregister_instance(*msg, userHandle);
    checkStatus(status, "NetworkPartitionsData::ChatMessageDataWriter::unregister_instance");

    /* Release the data-samples. */
    delete msg;     // msg allocated on heap: explicit de-allocation required!!

    /* Remove the DataWriters */
    status = chatPublisher->delete_datawriter( talker.in() );
    checkStatus(status, "DDS::Publisher::delete_datawriter (talker)");

    /* Remove the Publisher. */
    status = participant->delete_publisher( chatPublisher.in() );
    checkStatus(status, "DDS::DomainParticipant::delete_publisher");

    status = participant->delete_topic( chatMessageTopic.in() );
    checkStatus(status, "DDS::DomainParticipant::delete_topic (chatMessageTopic)");

    /* Remove the type-names. */
    string_free(chatMessageTypeName);

    /* Remove the DomainParticipant. */
    status = dpf->delete_participant( participant.in() );
    checkStatus(status, "DDS::DomainParticipantFactory::delete_participant");

    cout << "Completed Chatter example" << endl;
    return 0;
}
