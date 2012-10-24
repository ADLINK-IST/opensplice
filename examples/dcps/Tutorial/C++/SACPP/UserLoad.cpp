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
 * LOGICAL_NAME:    UserLoad.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             june 2007.
 ************************************************************************
 * 
 * This file contains the implementation for the 'UserLoad' executable.
 * 
 ***/

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include "ccpp_dds_dcps.h"
#include "CheckStatus.h"
#include "ccpp_Chat.h"

#define TERMINATION_MESSAGE -1 

using namespace DDS;
using namespace Chat;

/* entities required by all threads. */
static DDS::GuardCondition_var          escape;

/* Sleeper thread: sleeps 60 seconds and then triggers the WaitSet. */
extern "C" void *
delayedEscape(
    void *arg)
{
    DDS::ReturnCode_t status;
    
    sleep(60);     /* wait for 60 sec. */
    status = escape->set_trigger_value(TRUE);
    checkStatus(status, "DDS::GuardCondition::set_trigger_value");

    return NULL;
}

int
main (
    int argc,
    char *argv[])
{
    /* Generic DDS entities */
    DomainParticipant_var           participant;
    Topic_var                       chatMessageTopic;
    Topic_var                       nameServiceTopic;
    Subscriber_var                  chatSubscriber;
    DataReader_ptr                  parentReader;
    QueryCondition_var              singleUser;
    ReadCondition_var               newUser;
    StatusCondition_var             leftUser;
    WaitSet_var                     userLoadWS;
    LivelinessChangedStatus         livChangStatus;

    /* QosPolicy holders */
    TopicQos                        setting_topic_qos;
    TopicQos                        reliable_topic_qos;
    SubscriberQos                   sub_qos;
    DataReaderQos                   message_qos;

    /* DDS Identifiers */
    DomainId_t                      domain = NULL;
    ReturnCode_t                    status;
    ConditionSeq                    guardList;

    /* Type-specific DDS entities */
    ChatMessageTypeSupport_var      chatMessageTS;
    NameServiceTypeSupport_var      nameServiceTS;
    NameServiceDataReader_var       nameServer;
    ChatMessageDataReader_var       loadAdmin;
    ChatMessageSeq                  msgList;
    NameServiceSeq                  nsList;
    SampleInfoSeq                   infoSeq;
    SampleInfoSeq                   infoSeq2;

    /* Others */
    StringSeq                       args;
    char *                          chatMessageTypeName = NULL;
    char *                          nameServiceTypeName = NULL;

    bool                            closed = false;
    Long                            prevCount = 0;
    pthread_t                       tid;
    pthread_attr_t                  tattr;
    
    printf("Starting UserLoad example.\n");
    fflush(stdout);

    /* Create a DomainParticipant (using the 'TheParticipantFactory' convenience macro). */
    participant = TheParticipantFactory->create_participant (
        domain, 
        PARTICIPANT_QOS_DEFAULT, 
        NULL,
        STATUS_MASK_NONE);
    checkHandle(participant.in(), "DDS::DomainParticipantFactory::create_participant");  

    /* Register the required datatype for ChatMessage. */
    chatMessageTS = new ChatMessageTypeSupport();
    checkHandle(chatMessageTS.in(), "new ChatMessageTypeSupport");
    chatMessageTypeName = chatMessageTS->get_type_name();
    status = chatMessageTS->register_type(participant.in(), chatMessageTypeName);
    checkStatus(status, "Chat::ChatMessageTypeSupport::register_type");
    
    /* Register the required datatype for NameService. */
    nameServiceTS = new NameServiceTypeSupport();
    checkHandle(nameServiceTS.in(), "new NameServiceTypeSupport");
    nameServiceTypeName =  nameServiceTS->get_type_name();
    status = nameServiceTS->register_type(participant.in(), nameServiceTypeName);
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
    checkHandle(nameServiceTopic.in(), "DDS::DomainParticipant::create_topic");

    /* Adapt the default SubscriberQos to read from the "ChatRoom" Partition. */
    status = participant->get_default_subscriber_qos (sub_qos);
    checkStatus(status, "DDS::DomainParticipant::get_default_subscriber_qos");
    sub_qos.partition.name.length(1);
    sub_qos.partition.name[0UL] = "ChatRoom";

    /* Create a Subscriber for the UserLoad application. */
    chatSubscriber = participant->create_subscriber(sub_qos, NULL, STATUS_MASK_NONE);
    checkHandle(chatSubscriber.in(), "DDS::DomainParticipant::create_subscriber");
    
    /* Create a DataReader for the NameService Topic (using the appropriate QoS). */
    parentReader = chatSubscriber->create_datareader( 
        nameServiceTopic.in(), 
        DATAREADER_QOS_USE_TOPIC_QOS, 
        NULL,
        STATUS_MASK_NONE);
    checkHandle(parentReader, "DDS::Subscriber::create_datareader (NameService)");

    /* Narrow the abstract parent into its typed representative. */
    nameServer = NameServiceDataReader::_narrow(parentReader);
    checkHandle(nameServer.in(), "Chat::NameServiceDataReader::_narrow");
    
    /* Adapt the DataReaderQos for the ChatMessageDataReader to keep track of all messages. */
    status = chatSubscriber->get_default_datareader_qos(message_qos);
    checkStatus(status, "DDS::Subscriber::get_default_datareader_qos");
    status = chatSubscriber->copy_from_topic_qos(message_qos, reliable_topic_qos);
    checkStatus(status, "DDS::Subscriber::copy_from_topic_qos");
    message_qos.history.kind = KEEP_ALL_HISTORY_QOS;

    /* Create a DataReader for the ChatMessage Topic (using the appropriate QoS). */
    parentReader = chatSubscriber->create_datareader( 
        chatMessageTopic.in(), 
        message_qos, 
        NULL,
        STATUS_MASK_NONE);
    checkHandle(parentReader, "DDS::Subscriber::create_datareader (ChatMessage)");
    
    /* Narrow the abstract parent into its typed representative. */
    loadAdmin = ChatMessageDataReader::_narrow(parentReader);
    checkHandle(loadAdmin.in(), "Chat::ChatMessageDataReader::_narrow");
    
    /* Initialize the Query Arguments. */
    args.length(1);
    args[0UL] = "0";
    
    /* Create a QueryCondition that will contain all messages with userID=ownID */
    singleUser = loadAdmin->create_querycondition( 
        ANY_SAMPLE_STATE, 
        ANY_VIEW_STATE, 
        ANY_INSTANCE_STATE, 
        "userID=%0", 
        args);
    checkHandle(singleUser.in(), "DDS::DataReader::create_querycondition");
    
    /* Create a ReadCondition that will contain new users only */
    newUser = nameServer->create_readcondition( 
        NOT_READ_SAMPLE_STATE, 
        NEW_VIEW_STATE, 
        ALIVE_INSTANCE_STATE);
    checkHandle(newUser.in(), "DDS::DataReader::create_readcondition");

    /* Obtain a StatusCondition that triggers only when a Writer changes Liveliness */
    leftUser = loadAdmin->get_statuscondition();
    checkHandle(leftUser.in(), "DDS::DataReader::get_statuscondition");
    status = leftUser->set_enabled_statuses(LIVELINESS_CHANGED_STATUS);
    checkStatus(status, "DDS::StatusCondition::set_enabled_statuses");

    /* Create a bare guard which will be used to close the room */
    escape = new GuardCondition();

    /* Create a waitset and add the ReadConditions */
    userLoadWS = new WaitSet();
    status = userLoadWS->attach_condition(newUser.in());
    checkStatus(status, "DDS::WaitSet::attach_condition (newUser)");
    status = userLoadWS->attach_condition(leftUser.in());
    checkStatus(status, "DDS::WaitSet::attach_condition (leftUser)");
    status = userLoadWS->attach_condition(escape.in());
    checkStatus(status, "DDS::WaitSet::attach_condition (escape)");
 
    /* Initialize and pre-allocate the GuardList used to obtain the triggered Conditions. */
    guardList.length(3);
    
    /* Remove all known Users that are not currently active. */
    status = nameServer->take( 
        nsList, 
        infoSeq, 
        LENGTH_UNLIMITED, 
        ANY_SAMPLE_STATE, 
        ANY_VIEW_STATE, 
        NOT_ALIVE_INSTANCE_STATE);
    checkStatus(status, "Chat::NameServiceDataReader::take");
    status = nameServer->return_loan(nsList, infoSeq);
    checkStatus(status, "Chat::NameServiceDataReader::return_loan");
    
    /* Start the sleeper thread. */
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
    pthread_create (&tid, &tattr, delayedEscape, NULL);
    pthread_attr_destroy(&tattr);

    /* Print a message that the UserLoad has opened. */
    cout << "UserLoad has opened: disconnect a Chatter with userID = " << TERMINATION_MESSAGE << " to close it...." << endl << endl;
    
    while (!closed) {
        /* Wait until at least one of the Conditions in the waitset triggers. */
        status = userLoadWS->wait(guardList, DURATION_INFINITE);
        checkStatus(status, "DDS::WaitSet::wait");

        /* Walk over all guards to display information */
        for (ULong i = 0; i < guardList.length(); i++) {
            if ( guardList[i].in() == newUser.in() ) {
                /* The newUser ReadCondition contains data */
                status = nameServer->read_w_condition( 
                    nsList, 
                    infoSeq, 
                    LENGTH_UNLIMITED, 
                    newUser.in() );
                checkStatus(status, "Chat::NameServiceDataReader::read_w_condition");
                
                for (ULong j = 0; j < nsList.length(); j++) {
                    cout << "New user: " << nsList[j].name << endl;
                }
                status = nameServer->return_loan(nsList, infoSeq);
                checkStatus(status, "Chat::NameServiceDataReader::return_loan");

            } else if ( guardList[i].in() == leftUser.in() ) {
                /* Some liveliness has changed (either a DataWriter joined or a DataWriter left) */
                status = loadAdmin->get_liveliness_changed_status(livChangStatus);
                checkStatus(status, "DDS::DataReader::get_liveliness_changed_status");
                if (livChangStatus.alive_count < prevCount) {
                    /* A user has left the ChatRoom, since a DataWriter lost its liveliness */
                    /* Take the effected users so tey will not appear in the list later on. */
                    status = nameServer->take( 
                        nsList, 
                        infoSeq, 
                        LENGTH_UNLIMITED, 
                        ANY_SAMPLE_STATE, 
                        ANY_VIEW_STATE, 
                        NOT_ALIVE_NO_WRITERS_INSTANCE_STATE);
                    checkStatus(status, "Chat::NameServiceDataReader::take");
    
                    for (ULong j = 0; j < nsList.length(); j++) {
                        /* re-apply query arguments */
                        ostringstream numberString;
                        numberString << nsList[j].userID;
                        args[0UL] = numberString.str().c_str();
                        status = singleUser->set_query_parameters(args);
                        checkStatus(status, "DDS::QueryCondition::set_query_parameters");
    
                        /* Read this users history */
                        status = loadAdmin->take_w_condition( 
                            msgList, 
                            infoSeq2, 
                            LENGTH_UNLIMITED, 
                            singleUser.in() );
                        checkStatus(status, "Chat::ChatMessageDataReader::take_w_condition");
                        
                        /* Display the user and his history */
                        cout << "Departed user " << nsList[j].name << " has sent " << 
                            msgList.length() << " messages." << endl;
                        status = loadAdmin->return_loan(msgList, infoSeq2);
                        checkStatus(status, "Chat::ChatMessageDataReader::return_loan");
                        if(nsList[j].userID == TERMINATION_MESSAGE) {
                           printf("Termination message received: exiting...\n");
                           closed = true;
                        }
                    }
                    status = nameServer->return_loan(nsList, infoSeq);
                    checkStatus(status, "Chat::NameServiceDataReader::return_loan");
                }
                prevCount = livChangStatus.alive_count;

            } else if ( guardList[i].in() == escape.in() ) {
                cout << "UserLoad has terminated." << endl;
                closed = true;
            }
            else
            {
                assert(0);
            };
        } /* for */
    } /* while (!closed) */

    /* Remove all Conditions from the WaitSet. */
    status = userLoadWS->detach_condition( escape.in() );
    checkStatus(status, "DDS::WaitSet::detach_condition (escape)");
    status = userLoadWS->detach_condition( leftUser.in() );
    checkStatus(status, "DDS::WaitSet::detach_condition (leftUser)");
    status = userLoadWS->detach_condition( newUser.in() );
    checkStatus(status, "DDS::WaitSet::detach_condition (newUser)");

    /* Remove the type-names. */
    string_free(chatMessageTypeName);
    string_free(nameServiceTypeName);

    /* Free all resources */
    status = participant->delete_contained_entities();
    checkStatus(status, "DDS::DomainParticipant::delete_contained_entities");
    status = TheParticipantFactory->delete_participant( participant.in() );
    checkStatus(status, "DDS::DomainParticipantFactory::delete_participant");
    
    return 0;
}
