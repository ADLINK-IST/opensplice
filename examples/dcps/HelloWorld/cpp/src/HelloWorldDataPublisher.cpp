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
 * LOGICAL_NAME:    HelloWorldDataPublisher.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation for the 'HelloWorldDataPublisher' executable.
 *
 ***/
#include <string>
#include <sstream>
#include <iostream>
#include "DDSEntityManager.h"
#include "ccpp_HelloWorldData.h"
#include "os.h"

#include "example_main.h"

using namespace DDS;
using namespace HelloWorldData;

/* entry point exported and demangled so symbol can be found in shared library */
extern "C"
{
  OS_API_EXPORT
  int HelloWorldDataPublisher(int argc, char *argv[]);
}

int HelloWorldDataPublisher(int argc, char *argv[])
{
  os_time delay_1s = { 1, 0 };
  DDSEntityManager mgr;

  // create domain participant
  mgr.createParticipant("HelloWorld example");

  //create type
  MsgTypeSupport_var mt = new MsgTypeSupport();
  mgr.registerType(mt.in());

  //create Topic
  char topic_name[] = "HelloWorldData_Msg";
  mgr.createTopic(topic_name);

  //create Publisher
  mgr.createPublisher();

  // create DataWriter :
  // If autodispose_unregistered_instances is set to true (default value),
  // you will have to start the subscriber before the publisher
  bool autodispose_unregistered_instances = false;
  mgr.createWriter(autodispose_unregistered_instances);

  // Publish Events
  DataWriter_var dwriter = mgr.getWriter();
  MsgDataWriter_var HelloWorldWriter = MsgDataWriter::_narrow(dwriter.in());

  Msg msgInstance; /* Example on Stack */
  msgInstance.userID = 1;
  msgInstance.message = DDS::string_dup("Hello World");
  cout << "=== [Publisher] writing a message containing :" << endl;
  cout << "    userID  : " << msgInstance.userID << endl;
  cout << "    Message : \"" << msgInstance.message << "\"" << endl;

  ReturnCode_t status = HelloWorldWriter->write(msgInstance, DDS::HANDLE_NIL);
  checkStatus(status, "MsgDataWriter::write");
  os_nanoSleep(delay_1s);

  /* Remove the DataWriters */
  mgr.deleteWriter();

  /* Remove the Publisher. */
  mgr.deletePublisher();

  /* Remove the Topics. */
  mgr.deleteTopic();

  /* Remove Participant. */
  mgr.deleteParticipant();

  return 0;
}

int OSPL_MAIN (int argc, char *argv[])
{
  return HelloWorldDataPublisher (argc, argv);
}
