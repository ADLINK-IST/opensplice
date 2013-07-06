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
 * LOGICAL_NAME:    MessageBoard.cpp
 * FUNCTION:        OpenSplice NetworkPartition example code.
 * MODULE:          NetworkPartition example.
 * DATE             june 2007.
 ************************************************************************
 *
 * This file contains the implementation for the 'MessageBoard' executable.
 *
 ***/

#include <iostream>
#include <string.h>
#include "ccpp_dds_dcps.h"
#include "CheckStatus.h"
#include "ccpp_NetworkPartitionsData.h"
#include "example_main.h"
#ifndef _WIN32
#include <unistd.h>
#else
#include "Windows.h"
static void sleep(int secs)
{
    Sleep(secs * 1000);
}
#endif

using namespace DDS;
using namespace NetworkPartitionsData;



#define TERMINATION_MESSAGE -1



int
OSPL_MAIN (
    int argc,
    char *argv[])
{
    /* Generic DDS entities */
    DomainParticipantFactory_var    dpf;
    DomainParticipant_var           participant;
    Topic_var                       chatMessageTopic;
    Subscriber_var                  chatSubscriber;
    DataReader_ptr                  parentReader;

    /* Type-specific DDS entities */
    ChatMessageTypeSupport_var      chatMessageTS;
    ChatMessageDataReader_var       chatAdmin;
    ChatMessageSeq_var              msgSeq = new ChatMessageSeq();
    SampleInfoSeq_var               infoSeq = new SampleInfoSeq();

    /* QosPolicy holders */
    TopicQos                        reliable_topic_qos;
    SubscriberQos                   sub_qos;
    DDS::StringSeq                  parameterList;

    /* DDS Identifiers */
    DomainId_t                      domain = 0;
    ReturnCode_t                    status;

    /* Others */
    bool                            terminated = false;
    const char *                    partitionName = "ChatRoom1";
    char  *                         chatMessageTypeName = NULL;

    /* Options: MessageBoard [ownID] */
    /* Messages having owner ownID will be ignored */
    parameterList.length(1);

    if (argc > 1) {
        parameterList[0] = string_dup(argv[1]);
    }
    else
    {
        parameterList[0] = "0";
    }

    /* Create a DomainParticipantFactory and a DomainParticipant (using Default QoS settings. */
    dpf = DomainParticipantFactory::get_instance();
    checkHandle(dpf.in(), "DDS::DomainParticipantFactory::get_instance");
    participant = dpf->create_participant (
        domain,
        PARTICIPANT_QOS_DEFAULT,
        NULL,
        STATUS_MASK_NONE);
    checkHandle(participant, "DDS::DomainParticipantFactory::create_participant");

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
    reliable_topic_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;

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

    /* Adapt the default SubscriberQos to read from the "ChatRoom1" Partition. */
    status = participant->get_default_subscriber_qos (sub_qos);
    checkStatus(status, "DDS::DomainParticipant::get_default_subscriber_qos");
    sub_qos.partition.name.length(1);
    sub_qos.partition.name[0] = partitionName;

    /* Create a Subscriber for the MessageBoard application. */
    chatSubscriber = participant->create_subscriber(sub_qos, NULL, STATUS_MASK_NONE);
    checkHandle(chatSubscriber.in(), "DDS::DomainParticipant::create_subscriber");

    /* Create a DataReader for the NamedMessage Topic (using the appropriate QoS). */
    parentReader = chatSubscriber->create_datareader(
        chatMessageTopic.in(),
        DATAREADER_QOS_USE_TOPIC_QOS,
        NULL,
        STATUS_MASK_NONE);
    checkHandle(parentReader, "DDS::Subscriber::create_datareader");

    /* Narrow the abstract parent into its typed representative. */
    chatAdmin = ChatMessageDataReader::_narrow(parentReader);
    checkHandle(chatAdmin.in(), "NetworkPartitionsData::ChatMessageDataReader::_narrow");

    /* Print a message that the MessageBoard has opened. */
    cout << "MessageBoard has opened: send a ChatMessage with userID = -1 to close it...." << endl << endl;

    while (!terminated) {
        /* Note: using read does not remove the samples from
           unregistered instances from the DataReader. This means
           that the DataRase would use more and more resources.
           That's why we use take here instead. */

        status = chatAdmin->take(
            msgSeq,
            infoSeq,
            LENGTH_UNLIMITED,
            ANY_SAMPLE_STATE,
            ANY_VIEW_STATE,
            ALIVE_INSTANCE_STATE );
        checkStatus(status, "NetworkPartitionsData::ChatMessageDataReader::take");

        for (ULong i = 0; i < msgSeq->length(); i++) {
            ChatMessage *msg = &(msgSeq[i]);
            if (msg->userID == TERMINATION_MESSAGE) {
                cout << "Termination message received: exiting..." << endl;
                terminated = true;
            } else {
                cout << msg->userID << ": " << msg->content << endl;
            }
        }

        status = chatAdmin->return_loan(msgSeq, infoSeq);
        checkStatus(status, "NetworkPartitionsData::ChatMessageDataReader::return_loan");

        /* Sleep for some amount of time, so as not to consume too many CPU cycles. */
        sleep(1);
    }

    /* Remove the DataReader */
    status = chatSubscriber->delete_datareader(chatAdmin.in());
    checkStatus(status, "DDS::Subscriber::delete_datareader");

    /* Remove the Subscriber. */
    status = participant->delete_subscriber(chatSubscriber.in());
    checkStatus(status, "DDS::DomainParticipant::delete_subscriber");

    /* Remove the Topic. */
    status = participant->delete_topic(chatMessageTopic.in());
    checkStatus(status, "DDS::DomainParticipant::delete_topic (chatMessageTopic)");

    /* De-allocate the type-names. */
    string_free(chatMessageTypeName);

    /* Remove the DomainParticipant. */
    status = dpf->delete_participant(participant.in());
    checkStatus(status, "DDS::DomainParticipantFactory::delete_participant");

    return 0;
}
