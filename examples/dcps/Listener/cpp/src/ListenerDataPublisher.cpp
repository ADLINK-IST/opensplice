/*************************************************************************
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
 * LOGICAL_NAME:    ListenerDataPublisher.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation for the 'ListenerDataPublisher' executable.
 *
 ***/
#include <string>
#include <sstream>
#include <iostream>
#include "DDSEntityManager.h"
#include "ccpp_ListenerData.h"
#include "os.h"

#include "example_main.h"

using namespace DDS;
using namespace ListenerData;

int OSPL_MAIN (int argc, char *argv[])
{
  os_time delay_2s = { 2, 0 };
  DDSEntityManager mgr;

  // create domain participant
  char partition_name[] = "Listener example";
  mgr.createParticipant(partition_name);

  //create type
  MsgTypeSupport_var mt = new MsgTypeSupport();
  mgr.registerType(mt.in());

  //create Topic
  char topic_name[] = "ListenerData_Msg";
  mgr.createTopic(topic_name);

  //create Publisher
  mgr.createPublisher();

  // create DataWriter
  mgr.createWriter();

  // Publish Events
  DataWriter_var dwriter = mgr.getWriter();
  MsgDataWriter_var listenerWriter = MsgDataWriter::_narrow(dwriter.in());

  Msg msgInstance; /* Example on Stack */
  msgInstance.userID = 1;
  msgInstance.message = DDS::string_dup("Hello World");
  cout << "=== [ListenerDataPublisher] writing a message containing :" << endl;
  cout << "    userID  : " << msgInstance.userID << endl;
  cout << "    Message : \"" << msgInstance.message << "\"" << endl;

  ReturnCode_t status = listenerWriter->write(msgInstance, NULL);
  checkStatus(status, "MsgDataWriter::write");
  os_nanoSleep(delay_2s);

  /* Remove the DataWriters */
  mgr.deleteWriter(listenerWriter.in ());

  /* Remove the Publisher. */
  mgr.deletePublisher();

  /* Remove the Topics. */
  mgr.deleteTopic();

  /* Remove Participant. */
  mgr.deleteParticipant();

  return 0;
}
