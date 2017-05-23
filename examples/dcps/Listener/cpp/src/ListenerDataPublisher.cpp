/*************************************************************************
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#include "vortex_os.h"

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
