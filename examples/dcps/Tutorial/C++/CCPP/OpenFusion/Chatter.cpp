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
 * LOGICAL_NAME:    Chatter.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             june 2007.
 ************************************************************************
 * 
 * This file contains the implementation for the 'Chatter' executable.
 * 
 ***/
#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include "ccpp_dds_dcps.h"
#include "CheckStatus.h"
#include "ccpp_Chat.h"

#define MAX_MSG_LEN 256
#define NUM_MSG 10
#define TERMINATION_MESSAGE -1 

using namespace DDS;
using namespace Chat;

int 
main (
    int argc,
    char *argv[]) 
{
    /* Generic DDS entities */
    DomainParticipantFactory_var    dpf;
    DomainParticipant_var           participant;
    Topic_var                       chatMessageTopic;
    Topic_var                       nameServiceTopic;
    Publisher_var                   chatPublisher;
    DataWriter_ptr                  parentWriter;

    /* QosPolicy holders */
    TopicQos                        reliable_topic_qos;
    TopicQos                        setting_topic_qos;
    PublisherQos                    pub_qos;
    DataWriterQos                   dw_qos;

    /* DDS Identifiers */
    DomainId_t                      domain = NULL;
    InstanceHandle_t                userHandle;
    ReturnCode_t                    status;

    /* Type-specific DDS entities */
    ChatMessageTypeSupport_var      chatMessageTS;
    NameServiceTypeSupport_var      nameServiceTS;
    ChatMessageDataWriter_var       talker;
    NameServiceDataWriter_var       nameServer;

    /* Sample definitions */
    ChatMessage                     *msg;   /* Example on Heap */
    NameService                     ns;     /* Example on Stack */
    
    /* Others */
    int                             ownID = 1;
    int                             i;
    char                            *chatterName = NULL;
    const char                      *partitionName = "ChatRoom";
    char                            *chatMessageTypeName = NULL;
    char                            *nameServiceTypeName = NULL;
    ostringstream                   buf;

        

    /* Options: Chatter [ownID [name]] */
    if (argc > 1) {
        istringstream args(argv[1]);
        args >> ownID;
        if (argc > 2) {
            chatterName = argv[2];
        }
    }

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
    checkStatus(status, "Chat::ChatMessageTypeSupport::register_type");
    
    /* Register the required datatype for NameService. */
    nameServiceTS = new NameServiceTypeSupport();
    checkHandle(nameServiceTS.in(), "new NameServiceTypeSupport");
    nameServiceTypeName =  nameServiceTS->get_type_name();
    status = nameServiceTS->register_type(
        participant.in(), 
        nameServiceTypeName);
    checkStatus(status, "Chat::NameServiceTypeSupport::register_type");
    
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
    
    /* Set the DurabilityQosPolicy to TRANSIENT. */
    status = participant->get_default_topic_qos(setting_topic_qos);
    checkStatus(status, "DDS::DomainParticipant::get_default_topic_qos");
    setting_topic_qos.durability.kind = TRANSIENT_DURABILITY_QOS;

    /* Create the NameService Topic. */
    nameServiceTopic = participant->create_topic( 
        "Chat_NameService", 
        nameServiceTypeName, 
        setting_topic_qos, 
        NULL,
        STATUS_MASK_NONE);
    checkHandle(nameServiceTopic.in(), "DDS::DomainParticipant::create_topic (NameService)");

    /* Adapt the default PublisherQos to write into the "ChatRoom" Partition. */
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
    checkHandle(talker.in(), "Chat::ChatMessageDataWriter::_narrow");

    /* Create a DataWriter for the NameService Topic (using the appropriate QoS). */
    status = chatPublisher->get_default_datawriter_qos(dw_qos);
    checkStatus(status, "DDS::Publisher::get_default_datawriter_qos");
    status = chatPublisher->copy_from_topic_qos(dw_qos, setting_topic_qos);
    checkStatus(status, "DDS::Publisher::copy_from_topic_qos");
    dw_qos.writer_data_lifecycle.autodispose_unregistered_instances = FALSE;
    parentWriter = chatPublisher->create_datawriter( 
        nameServiceTopic.in(), 
        dw_qos, 
        NULL,
        STATUS_MASK_NONE);
    checkHandle(parentWriter, "DDS::Publisher::create_datawriter (NameService)");
    
    /* Narrow the abstract parent into its typed representative. */
    nameServer = NameServiceDataWriter::_narrow(parentWriter);
    checkHandle(nameServer.in(), "Chat::NameServiceDataWriter::_narrow");
    
    /* Initialize the NameServer attributes located on stack. */
    ns.userID = ownID;
    if (chatterName) {
        ns.name = CORBA::string_dup(chatterName);
    } else {
        buf << "Chatter " << ownID;
        ns.name = CORBA::string_dup( buf.str().c_str() );
    }

    /* Write the user-information into the system (registering the instance implicitly). */
    status = nameServer->write(ns, HANDLE_NIL);
    checkStatus(status, "Chat::ChatMessageDataWriter::write");
    
    /* Initialize the chat messages on Heap. */
    msg = new ChatMessage();
    checkHandle(msg, "new ChatMessage");
    msg->userID = ownID;
    msg->index = 0;
    buf.str( string("") );
    if (ownID == TERMINATION_MESSAGE) {
        buf << "Termination message.";
    } else { 
        buf << "Hi there, I will send you " << NUM_MSG << " more messages.";
    }
    msg->content = CORBA::string_dup( buf.str().c_str() );
    cout << "Writing message: \"" << msg->content  << "\"" << endl;

    /* Register a chat message for this user (pre-allocating resources for it!!) */
    userHandle = talker->register_instance(*msg);

    /* Write a message using the pre-generated instance handle. */
    status = talker->write(*msg, userHandle);
    checkStatus(status, "Chat::ChatMessageDataWriter::write");

    sleep (1); /* do not run so fast! */
 
    /* Write any number of messages, re-using the existing string-buffer: no leak!!. */
    for (i = 1; i <= NUM_MSG && ownID != TERMINATION_MESSAGE; i++) {
        buf.str( string("") );
        msg->index = i;
        buf << "Message no. " << i;
        msg->content = CORBA::string_dup( buf.str().c_str() );
        cout << "Writing message: \"" << msg->content << "\"" << endl;
        status = talker->write(*msg, userHandle);
        checkStatus(status, "Chat::ChatMessageDataWriter::write");
        sleep (1); /* do not run so fast! */
    }

    /* Leave the room by disposing and unregistering the message instance. */
    status = talker->dispose(*msg, userHandle);
    checkStatus(status, "Chat::ChatMessageDataWriter::dispose");
    status = talker->unregister_instance(*msg, userHandle);
    checkStatus(status, "Chat::ChatMessageDataWriter::unregister_instance");

    /* Also unregister our name. */
    status = nameServer->unregister_instance(ns, HANDLE_NIL);
    checkStatus(status, "Chat::NameServiceDataWriter::unregister_instance");

    /* Release the data-samples. */
    delete msg;     // msg allocated on heap: explicit de-allocation required!!

    /* Remove the DataWriters */
    status = chatPublisher->delete_datawriter( talker.in() );
    checkStatus(status, "DDS::Publisher::delete_datawriter (talker)");
    
    status = chatPublisher->delete_datawriter( nameServer.in() );
    checkStatus(status, "DDS::Publisher::delete_datawriter (nameServer)");
    
    /* Remove the Publisher. */
    status = participant->delete_publisher( chatPublisher.in() );
    checkStatus(status, "DDS::DomainParticipant::delete_publisher");
    
    /* Remove the Topics. */
    status = participant->delete_topic( nameServiceTopic.in() );
    checkStatus(status, "DDS::DomainParticipant::delete_topic (nameServiceTopic)");
    
    status = participant->delete_topic( chatMessageTopic.in() );
    checkStatus(status, "DDS::DomainParticipant::delete_topic (chatMessageTopic)");

    /* Remove the type-names. */
    CORBA::string_free(chatMessageTypeName);
    CORBA::string_free(nameServiceTypeName);
    
    /* Remove the DomainParticipant. */
    status = dpf->delete_participant( participant.in() );
    checkStatus(status, "DDS::DomainParticipantFactory::delete_participant");

    return 0;
}
