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
 * LOGICAL_NAME:    MessageBoard.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             june 2007.
 ************************************************************************
 * 
 * This file contains the implementation for the 'MessageBoard' executable.
 * 
 ***/
 
#include <iostream>
#include <string.h>
#include <unistd.h>

#include "ccpp_dds_dcps.h"
#include "CheckStatus.h"
#include "ccpp_Chat.h"
#include "multitopic.h"

using namespace DDS;
using namespace Chat;



#define TERMINATION_MESSAGE -1 



int
main (
    int argc,
    char *argv[])
{
    /* Generic DDS entities */
    DomainParticipantFactory_var    dpf;
    DomainParticipant_ptr           parentDP;
    ExtDomainParticipant_var        participant;
    Topic_var                       chatMessageTopic;
    Topic_var                       nameServiceTopic;
    TopicDescription_var            namedMessageTopic;
    Subscriber_var                  chatSubscriber;
    DataReader_ptr                  parentReader;

    /* Type-specific DDS entities */
    ChatMessageTypeSupport_var      chatMessageTS;
    NameServiceTypeSupport_var      nameServiceTS;
    NamedMessageTypeSupport_var     namedMessageTS;
    NamedMessageDataReader_var      chatAdmin;
    NamedMessageSeq_var             msgSeq = new NamedMessageSeq();
    SampleInfoSeq_var               infoSeq = new SampleInfoSeq();

    /* QosPolicy holders */
    TopicQos                        reliable_topic_qos;
    TopicQos                        setting_topic_qos;
    SubscriberQos                   sub_qos;
    DDS::StringSeq                  parameterList;

    /* DDS Identifiers */
    DomainId_t                      domain = NULL;
    ReturnCode_t                    status;

    /* Others */
    bool                            terminated = FALSE;
    const char *                    partitionName = "ChatRoom";
    char  *                         chatMessageTypeName = NULL;
    char  *                         nameServiceTypeName = NULL;
    char  *                         namedMessageTypeName = NULL;

#ifdef USE_NANOSLEEP
    struct timespec                 sleeptime;
    struct timespec                 remtime;
#endif

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
    parentDP = dpf->create_participant (
        domain, 
        PARTICIPANT_QOS_DEFAULT, 
        NULL,
        STATUS_MASK_NONE);
    checkHandle(parentDP, "DDS::DomainParticipantFactory::create_participant");
    
    /* Narrow the normal participant to its extended representative */
    participant = ExtDomainParticipantImpl::_narrow(parentDP);
    checkHandle(participant.in(), "DDS::ExtDomainParticipant::_narrow");

    /* Register the required datatype for ChatMessage. */
    chatMessageTS = new ChatMessageTypeSupport();
    checkHandle(chatMessageTS.in(), "new ChatMessageTypeSupport");
    chatMessageTypeName = chatMessageTS->get_type_name();
    status = chatMessageTS->register_type(
        participant.in(), 
        chatMessageTypeName);
    checkStatus(status, "Chat::ChatMessageTypeSupport::register_type");
    
    /* Register the required datatype for NameService. */
    nameServiceTS = new NameServiceTypeSupport();
    checkHandle(nameServiceTS.in(), "new NameServiceTypeSupport");
    nameServiceTypeName =  nameServiceTS->get_type_name();
    status = nameServiceTS->register_type(
        participant.in(), 
        nameServiceTypeName);
    checkStatus(status, "Chat::NameServiceTypeSupport::register_type");
    
    /* Register the required datatype for NamedMessage. */
    namedMessageTS = new NamedMessageTypeSupport();
    checkHandle(namedMessageTS.in(), "new NamedMessageTypeSupport");
    namedMessageTypeName = namedMessageTS->get_type_name();
    status = namedMessageTS->register_type(
        participant.in(), 
        namedMessageTypeName);
    checkStatus(status, "Chat::NamedMessageTypeSupport::register_type");
    
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
    
    /* Set the DurabilityQosPolicy to TRANSIENT. */
    status = participant->get_default_topic_qos(setting_topic_qos);
    checkStatus(status, "DDS::DomainParticipant::get_default_topic_qos");
    setting_topic_qos.durability.kind = DDS::TRANSIENT_DURABILITY_QOS;

    /* Create the NameService Topic. */
    nameServiceTopic = participant->create_topic( 
        "Chat_NameService", 
        nameServiceTypeName, 
        setting_topic_qos, 
        NULL,
        STATUS_MASK_NONE);
    checkHandle(nameServiceTopic.in(), "DDS::DomainParticipant::create_topic");
    
    /* Create a multitopic that substitutes the userID with its corresponding userName. */
    namedMessageTopic = participant->create_simulated_multitopic(
        "Chat_NamedMessage", 
        namedMessageTypeName, 
        "SELECT userID, name AS userName, index, content "
            "FROM Chat_NameService NATURAL JOIN Chat_ChatMessage WHERE userID <> %0",
        parameterList);
    checkHandle(namedMessageTopic.in(), "DDS::ExtDomainParticipant::create_simulated_multitopic");

    /* Adapt the default SubscriberQos to read from the "ChatRoom" Partition. */
    status = participant->get_default_subscriber_qos (sub_qos);
    checkStatus(status, "DDS::DomainParticipant::get_default_subscriber_qos");
    sub_qos.partition.name.length(1);
    sub_qos.partition.name[0] = partitionName;

    /* Create a Subscriber for the MessageBoard application. */
    chatSubscriber = participant->create_subscriber(sub_qos, NULL, STATUS_MASK_NONE);
    checkHandle(chatSubscriber.in(), "DDS::DomainParticipant::create_subscriber");
    
    /* Create a DataReader for the NamedMessage Topic (using the appropriate QoS). */
    parentReader = chatSubscriber->create_datareader( 
        namedMessageTopic.in(), 
        DATAREADER_QOS_USE_TOPIC_QOS, 
        NULL,
        STATUS_MASK_NONE);
    checkHandle(parentReader, "DDS::Subscriber::create_datareader");
    
    /* Narrow the abstract parent into its typed representative. */
    chatAdmin = Chat::NamedMessageDataReader::_narrow(parentReader);
    checkHandle(chatAdmin.in(), "Chat::NamedMessageDataReader::_narrow");
    
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
        checkStatus(status, "Chat::NamedMessageDataReader::take");

        for (ULong i = 0; i < msgSeq->length(); i++) {
            NamedMessage *msg = &(msgSeq[i]);
            if (msg->userID == TERMINATION_MESSAGE) {
                cout << "Termination message received: exiting..." << endl;
                terminated = TRUE;
            } else {
                cout << msg->userName << ": " << msg->content << endl;
            }
        }

        status = chatAdmin->return_loan(msgSeq, infoSeq);
        checkStatus(status, "Chat::ChatMessageDataReader::return_loan");
        
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
    status = chatSubscriber->delete_datareader(chatAdmin.in());
    checkStatus(status, "DDS::Subscriber::delete_datareader");

    /* Remove the Subscriber. */
    status = participant->delete_subscriber(chatSubscriber.in());
    checkStatus(status, "DDS::DomainParticipant::delete_subscriber");
    
    /* Remove the Topics. */
    status = participant->delete_simulated_multitopic(namedMessageTopic.in());
    checkStatus(status, "DDS::ExtDomainParticipant::delete_simulated_multitopic");

    status = participant->delete_topic(nameServiceTopic.in());
    checkStatus(status, "DDS::DomainParticipant::delete_topic (nameServiceTopic)");

    status = participant->delete_topic(chatMessageTopic.in());
    checkStatus(status, "DDS::DomainParticipant::delete_topic (chatMessageTopic)");

    /* De-allocate the type-names. */
    string_free(namedMessageTypeName);
    string_free(nameServiceTypeName);
    string_free(chatMessageTypeName);

    /* Remove the DomainParticipant. */
    status = dpf->delete_participant(participant.in());
    checkStatus(status, "DDS::DomainParticipantFactory::delete_participant");
    
    exit(0);    
}
