/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
#include "ccpp_dds_dcps.h"
#include "CheckStatus.h"
#include "ccpp_Chat.h"
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

#include "example_main.h"

#define MAX_MSG_LEN 256
#define NUM_MSG 10
#define TERMINATION_MESSAGE -1

using namespace DDS;
using namespace Chat;

#ifdef _WRS_KERNEL
int chatter_main (int argc, char ** argv);
extern "C" {
   int chatter (char * args);
}
int chatter (char * args)
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
   return chatter_main (argc, argv);
}
int chatter_main (int argc, char ** argv)
#else
int
OSPL_MAIN (
    int argc,
    char *argv[])
#endif
{
    /* Generic DDS entities */
    DomainParticipantFactory_var    dpf;
    DomainParticipant_var           participant;
    Topic_var                       chatMessageTopic;
    Topic_var                       nameServiceTopic;
    Publisher_var                   chatPublisher;
    DataWriter_var                  parentWriter;

    /* QosPolicy holders */
    TopicQos                        reliable_topic_qos;
    TopicQos                        setting_topic_qos;
    PublisherQos                    pub_qos;
    DataWriterQos                   dw_qos;

    /* DDS Identifiers */
    DomainId_t                      domain = DOMAIN_ID_DEFAULT;
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
    char                            buf [MAX_MSG_LEN];



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
        ownID = atoi(argv[1]);
        if (argc > 2) {
            chatterName = argv[2];
        }
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
    checkHandle(parentWriter.in(), "DDS::Publisher::create_datawriter (chatMessage)");

    /* Narrow the abstract parent into its typed representative. */
    talker = ChatMessageDataWriter::_narrow(parentWriter.in());
    checkHandle(talker.in(), "Chat::ChatMessageDataWriter::_narrow");

    /* Create a DataWriter for the NameService Topic (using the appropriate QoS). */
    status = chatPublisher->get_default_datawriter_qos(dw_qos);
    checkStatus(status, "DDS::Publisher::get_default_datawriter_qos");
    status = chatPublisher->copy_from_topic_qos(dw_qos, setting_topic_qos);
    checkStatus(status, "DDS::Publisher::copy_from_topic_qos");
    dw_qos.writer_data_lifecycle.autodispose_unregistered_instances = false;
    parentWriter = chatPublisher->create_datawriter(
        nameServiceTopic.in(),
        dw_qos,
        NULL,
        STATUS_MASK_NONE);
    checkHandle(parentWriter, "DDS::Publisher::create_datawriter (NameService)");

    /* Narrow the abstract parent into its typed representative. */
    nameServer = NameServiceDataWriter::_narrow(parentWriter.in());
    checkHandle(nameServer.in(), "Chat::NameServiceDataWriter::_narrow");

    /* Initialize the NameServer attributes located on stack. */
    ns.userID = ownID;
    if (chatterName) {
        ns.name = DDS::string_dup(chatterName);
    }
    else
    {
        snprintf(buf, MAX_MSG_LEN, "Chatter %d", ownID);
        ns.name = DDS::string_dup( buf );
    }

    /* Write the user-information into the system (registering the instance implicitly). */
    status = nameServer->write(ns, HANDLE_NIL);
    checkStatus(status, "Chat::ChatMessageDataWriter::write");

    /* Initialize the chat messages on Heap. */
    msg = new ChatMessage();
    checkHandle(msg, "new ChatMessage");
    msg->userID = ownID;
    msg->index = 0;
    if (ownID == TERMINATION_MESSAGE) {
        snprintf (buf, MAX_MSG_LEN, "Termination message.");
    } else {
        snprintf (buf, MAX_MSG_LEN, "Hi There, I will send you %d more messages.", NUM_MSG);
    }
    msg->content = DDS::string_dup( buf );
    cout << "Writing message: \"" << msg->content  << "\"" << endl;

    /* Register a chat message for this user (pre-allocating resources for it!!) */
    userHandle = talker->register_instance(*msg);

    /* Write a message using the pre-generated instance handle. */
    status = talker->write(*msg, userHandle);
    checkStatus(status, "Chat::ChatMessageDataWriter::write");

    sleep (1); /* do not run so fast! */

    /* Write any number of messages, re-using the existing string-buffer: no leak!!. */
    for (i = 1; i <= NUM_MSG && ownID != TERMINATION_MESSAGE; i++) {
        msg->index = i;
        snprintf (buf, MAX_MSG_LEN, "Message no. %d", i);
        msg->content = DDS::string_dup(buf);
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
    DDS::string_free(chatMessageTypeName);
    DDS::string_free(nameServiceTypeName);

    /* Remove the DomainParticipant. */
    status = dpf->delete_participant( participant.in() );
    checkStatus(status, "DDS::DomainParticipantFactory::delete_participant");

    cout << "Completed Chatter example." << endl;

    return 0;
}
