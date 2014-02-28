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
 * LOGICAL_NAME:    WaitSetDataPublisher.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation for the 'WaitSetDataPublisher' executable.
 *
 ***/
#include <string>
#include <iostream>
#include "DDSEntityManager.h"
#include "ccpp_WaitSetData.h"
#include "os.h"

#include "example_main.h"

using namespace DDS;
using namespace WaitSetData;

int OSPL_MAIN (int argc, char *argv[])
{
  os_time delay_500ms = { 0, 500000000 };
  DDSEntityManager mgr;

  // create domain participant
  // create domain participant
  char partition_name[] = "WaitSet example";
  mgr.createParticipant(partition_name);

  //create type
  MsgTypeSupport_var mt = new MsgTypeSupport();
  mgr.registerType(mt.in());

  //create Topic
  char topic_name[] = "WaitSetData_Msg";
  mgr.createTopic(topic_name);

  //create Publisher
  mgr.createPublisher();

  // create DataWriter
  mgr.createWriter();

  // Publish Events
  DataWriter_var dwriter = mgr.getWriter();
  MsgDataWriter_var WaitSetDataWriter = MsgDataWriter::_narrow(dwriter.in());

  Msg msgInstance; /* Example on Stack */

  msgInstance.userID = 1;
  msgInstance.message = DDS::string_dup("First Hello");
  cout << "=== [Publisher] writing a message containing :" << endl;
  cout << "    userID  : " << msgInstance.userID << endl;
  cout << "    Message : \"" << msgInstance.message << "\"" << endl;

  ReturnCode_t status = WaitSetDataWriter->write(msgInstance, DDS::HANDLE_NIL);
  checkStatus(status, "MsgDataWriter::write1");
  os_nanoSleep(delay_500ms);
  // Write a second message
  msgInstance.message = DDS::string_dup("Hello again");
  status = WaitSetDataWriter->write(msgInstance, DDS::HANDLE_NIL);
  checkStatus(status, "MsgDataWriter::write2");

  cout << endl << "=== [Publisher] writing a message containing :" << endl;
  cout << "    userID  : " << msgInstance.userID << endl;
  cout << "    Message : \"" << msgInstance.message << "\"" << endl;
  os_nanoSleep(delay_500ms);

  /* Remove the DataWriters */
  mgr.deleteWriter(WaitSetDataWriter.in ());

  /* Remove the Publisher. */
  mgr.deletePublisher();

  /* Remove the Topics. */
  mgr.deleteTopic();

  /* Remove Participant. */
  mgr.deleteParticipant();

  return 0;
}
