
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
 * LOGICAL_NAME:    HelloWorldDataSubscriber.cpp
 * FUNCTION:        OpenSplice HelloWorld example code.
 * MODULE:          HelloWorld for the C++ programming language.
 * DATE             June 2017.
 * AUTHOR:          R.C.G. Poth
 *
 * Description:
 *
 * This file contains the implementation for the 'HelloWorldDataSubscriber'
 * executable. The goal is to provide a simple example of how to publish
 * data.
 *
 * This executable:
 * - creates the entities required to publish a message in the Domain
 * - publishes the HelloWorld message
 * - delete all entities
 * - terminate execution
 *
 ***/
#include <iostream>
#include "ccpp_dds_dcps.h"        /* Include the DDS::DCPS API */
#include "ccpp_HelloWorldData.h"  /* Include the generated type specific (HelloWorld) DCPS API */
#include "example_main.h"         /* Include to define the application main() wrapper OSPL_MAIN. */

using namespace std;

/**
 * Check the return status for errors.
 * If there is an error, then report info message and terminate.
 **/
static void checkStatus(DDS::ReturnCode_t status, const char *info);

/**
 * Check whether a valid handle has been returned.
 * If not, then report info message and terminate.
 **/
static void checkHandle(void *handle, string info);

/* entry point exported and demangled so symbol can be found in shared library */
extern "C"
{
  OS_API_EXPORT
  int HelloWorldDataSubscriber(int argc, char *argv[]);
}

/* Main wrapper to allow embedded usage of the Subscriber application. */
int OSPL_MAIN (int argc, char *argv[])
{
  return HelloWorldDataSubscriber (argc, argv);
}

/*
 * The main function of the Subscriber application
 */
int HelloWorldDataSubscriber(int argc, char *argv[])
{
    /* The DDS entities required to publish data */
    DDS::DomainParticipantFactory_var factory;
    DDS::DomainId_t                   domain;
    DDS::DomainParticipant_var        participant;
    DDS::Topic_var                    topic;
    DDS::Subscriber_var               subscriber;
    DDS::DataReader_var               reader;

    /* The Application HelloWorld Data TypeSupport */
    HelloWorldData::MsgTypeSupport_var typesupport;

    DDS::ReturnCode_t result;
    DDS::String_var typeName;

    os_time delay_200ms = { 0, 200000000 };

    HelloWorldData::MsgSeq msgList;
    DDS::SampleInfoSeq infoSeq;

  // Get the DDS DomainParticipantFactory
    factory = DDS::DomainParticipantFactory::get_instance();
    checkHandle(factory, "get_instance() failed");

  // Create a domain participant entity for the Default Domain (Domain Id = 0)
    domain = DDS::DOMAIN_ID_DEFAULT;
    participant = factory->create_participant(domain, PARTICIPANT_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(participant, "create_participant() failed");

  // Register the application data type
    typesupport = new HelloWorldData::MsgTypeSupport();
    /* Get the IDL type name and use this to register the type. */
    typeName = typesupport->get_type_name();
    result = typesupport->register_type(participant, typeName);
    checkStatus(result, "register_type() failed");

  // Create Topic entity
    /* Create and initialize topic qos value on heap. */
    DDS::TopicQos tQos;
    result = participant->get_default_topic_qos(tQos);
    checkStatus(result, "get_default_topic_qos() failed");

    /* Fine tune topic qos, i.e. make topic reliable and transient (for late joining subscribers) */
    tQos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
    tQos.durability.kind = DDS::TRANSIENT_DURABILITY_QOS;
    /* Use the changed policy when defining the HelloWorld topic */
    topic = participant->create_topic("HelloWorldData_Msg", typeName, tQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(topic, "create_topic() failed");

  // Create Subscriber entity
    /* Create on heap and initialize subscriber qos value with the default value. */
    DDS::SubscriberQos sQos;
    result = participant->get_default_subscriber_qos(sQos);
    checkStatus(result, "get_default_subscriber_qos() failed");

    /* Fine tune the partition qos policy ito the partition from which the data will be received. */
    sQos.partition.name.length(1);
    sQos.partition.name[0] = "HelloWorld Partition";
    /* Create the subscriber. */
    subscriber = participant->create_subscriber(sQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(subscriber, "create_subscriber() failed");

  // create DataReader entity
    DDS::DataReaderQos rQos;
    result = subscriber->get_default_datareader_qos(rQos);
    checkStatus(result, "get_default_datareader_qos() failed");

    result = subscriber->get_default_datareader_qos(rQos);
    checkStatus(result, "copy_from_topic_qos() failed");

    reader = subscriber->create_datareader(topic, rQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(reader, "create_datareader() failed");

    /* Cast reader to 'HelloWorld' type specific interface. */
    HelloWorldData::MsgDataReader_var HelloWorldReader = HelloWorldData::MsgDataReader::_narrow(reader);
    checkHandle(HelloWorldReader, "MsgDataReader::_narrow() failed");

    cout << "=== [Subscriber] Ready ..." << endl;

    bool closed = false;
    int count = 0;
    while (!closed && count < 150) // We dont want the example to run indefinitely (max 30 seconds)
    {
        result = HelloWorldReader->take(msgList, infoSeq, DDS::LENGTH_UNLIMITED,
                                        DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
        checkStatus(result, "msgDataReader::take() failed");

        for (DDS::ULong j = 0; j < msgList.length(); j++)
        {
            cout << "=== [Subscriber] message received :" << endl;
            cout << "    userID  : " << msgList[j].userID << endl;
            cout << "    Message : \"" << msgList[j].message << "\"" << endl;
            closed = true;
        }
        result = HelloWorldReader->return_loan(msgList, infoSeq);
        checkStatus(result, "MsgDataReader::return_loan() failed");
        os_nanoSleep(delay_200ms);
        count++;
    }

  // Report 'No Data' if no message was received.
    if (count == 150) {
        cout << "=== [Subscriber] message received : No Data" << endl;
    }

  // Delete all entities before termination (good practice to cleanup resources)
    result = subscriber->delete_datareader(reader);
    checkStatus(result, "delete_datareader() failed");
    result = participant->delete_subscriber(subscriber);
    checkStatus(result, "delete_publisher() failed");
    result = participant->delete_topic(topic);
    checkStatus(result, "delete_topic() failed");
    result = factory->delete_participant(participant);
    checkStatus(result, "delete_participant() failed");

    return 0;
}

/* End of the Subscriber  example application.
 * Following are the implementation of error checking helper function.
 */

/* Array to hold the names for all ReturnCodes. */
string RetCodeName[13] =
{
    "DDS_RETCODE_OK", "DDS_RETCODE_ERROR", "DDS_RETCODE_UNSUPPORTED",
    "DDS_RETCODE_BAD_PARAMETER", "DDS_RETCODE_PRECONDITION_NOT_MET",
    "DDS_RETCODE_OUT_OF_RESOURCES", "DDS_RETCODE_NOT_ENABLED",
    "DDS_RETCODE_IMMUTABLE_POLICY", "DDS_RETCODE_INCONSISTENT_POLICY",
    "DDS_RETCODE_ALREADY_DELETED", "DDS_RETCODE_TIMEOUT", "DDS_RETCODE_NO_DATA",
    "DDS_RETCODE_ILLEGAL_OPERATION"
};

/**
 * Check the return status for errors. If there is an error, then terminate.
 **/
static void checkStatus(DDS::ReturnCode_t status, const char *info)
{
    if (status != DDS::RETCODE_OK && status != DDS::RETCODE_NO_DATA) {
        cerr << "Error in " << info << "with return code : " << RetCodeName[status].c_str() << endl;
        exit(1);
    }
}

/**
 * Check whether a valid handle has been returned. If not, then terminate.
 **/
static void checkHandle(void *handle, string info)
{
    if (!handle) {
        cerr << "Error in " << info.c_str() << ": Creation failed: invalid handle" << endl;
        exit(1);
    }
}

