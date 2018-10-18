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
 * LOGICAL_NAME:    HelloWorldDataPublisher.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE:            June 2017.
 * AUTHOR:          R.C.G. Poth
 *
 * Description:
 *
 * This file contains the implementation for the 'HelloWorldDataPublisher'
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
  int HelloWorldDataPublisher(int argc, char *argv[]);
}

/* Main wrapper to allow embedded usage of the Publisher application. */
int OSPL_MAIN (int argc, char *argv[])
{
  return HelloWorldDataPublisher(argc, argv);
}

/*
 * The main function of the Publisher application
 */
int HelloWorldDataPublisher (int argc, char *argv[])
{
    /* The DDS entities required to publish data */
    DDS::DomainParticipantFactory_var factory;
    DDS::DomainId_t                   domain;
    DDS::DomainParticipant_var        participant;
    DDS::Topic_var                    topic;
    DDS::Publisher_var                publisher;
    DDS::DataWriter_var               writer;

    /* The Application HelloWorld Data TypeSupport */
    HelloWorldData::MsgTypeSupport_var typesupport;

    DDS::ReturnCode_t result;
    DDS::String_var typeName;

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

  // Create Publisher entity
    /* Create on heap and initialize publisher qos value with the default value. */
    DDS::PublisherQos pQos;
    result = participant->get_default_publisher_qos(pQos);
    checkStatus(result, "get_default_publisher_qos() failed");

    /* Fine tune the partition qos policy ito the partition in which the data will be published. */
    pQos.partition.name.length(1);
    pQos.partition.name[0] = "HelloWorld Partition";
    /* Create the publisher. */
    publisher = participant->create_publisher(pQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(publisher, "create_publisher() failed");

  // create DataWriter entity
    DDS::DataWriterQos wQos;
    result = publisher->get_default_datawriter_qos(wQos);
    checkStatus(result, "get_default_datawriter_qos() failed");

    result = publisher->copy_from_topic_qos(wQos, tQos);
    checkStatus(result, "copy_from_topic_qos() failed");

    /* Set the autodispose_unregistered_instances qos policy to false.
     * If autodispose_unregistered_instances is set to true (default value),
     * you will have to start the subscriber before the publisher
     */
    wQos.writer_data_lifecycle.autodispose_unregistered_instances = false;

    writer = publisher->create_datawriter(topic, wQos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(writer, "create_datawriter() failed");

    /* Cast writer to 'HelloWorld' type specific interface. */
    HelloWorldData::MsgDataWriter_var HelloWorldWriter = HelloWorldData::MsgDataWriter::_narrow(writer);
    checkHandle(HelloWorldWriter, "MsgDataWriter::_narrow() failed");

  // Create and Publish the HelloWorld message
    HelloWorldData::Msg msg; /* declare a Message on Stack */
    msg.userID = 1;
    msg.message = DDS::string_dup("Hello World");
    cout << "=== [Publisher] writing a message containing :" << endl;
    cout << "    userID  : " << msg.userID << endl;
    cout << "    Message : \"" << msg.message << "\"" << endl;

    result = HelloWorldWriter->write(msg, DDS::HANDLE_NIL);
    checkStatus(result, "write() failed");

    /* A small delay for convenience to avoid that the writer is deleted before the data is send */
    os_time delay_1s = { 1, 0 };
    os_nanoSleep(delay_1s);

  // Delete all entities before termination (good practice to cleanup resources)
    result = publisher->delete_datawriter(writer);
    checkStatus(result, "delete_datawriter() failed");
    result = participant->delete_publisher(publisher);
    checkStatus(result, "delete_publisher() failed");
    result = participant->delete_topic(topic);
    checkStatus(result, "delete_topic() failed");
    result = factory->delete_participant(participant);
    checkStatus(result, "delete_participant() failed");

    return 0;
}

/* End of the Publisher example application.
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

