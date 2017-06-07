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
 * LOGICAL_NAME:    Subscriber.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation for the 'Subscriber' executable.
 *
 ***/
#include <string>
#include <iostream>
#include "DDSEntityManager.h"
#include "ccpp_ListenerData.h"
#include "example_main.h"
#include "ListenerDataListener.h"
#include "vortex_os.h"

using namespace DDS;
using namespace ListenerData;

int OSPL_MAIN (int argc, char *argv[])
{
  MsgSeq msgList;
  SampleInfoSeq infoSeq;
  Duration_t timeout = { 0, 200000000 };
  int count = 0;

  DDSEntityManager mgr;

   // create domain participant
  char partition_name[] = "Listener example";
  mgr.createParticipant(partition_name);

  //create type
  MsgTypeSupport_var st = new MsgTypeSupport();
  mgr.registerType(st.in());

  //create Topic
  char topic_name[] = "ListenerData_Msg";
  mgr.createTopic(topic_name);

  //create Subscriber
  mgr.createSubscriber();

  // create DataReader
  mgr.createReader();

  DataReader_var dreader = mgr.getReader();
  ListenerDataListener *myListener = new ListenerDataListener();
  myListener->m_MsgReader = MsgDataReader::_narrow(dreader.in());
  checkHandle(myListener->m_MsgReader.in(), "MsgDataReader::_narrow");

  cout << "=== [ListenerDataSubscriber] set_listener" << endl;
  DDS::StatusMask mask =
           DDS::DATA_AVAILABLE_STATUS | DDS::REQUESTED_DEADLINE_MISSED_STATUS;
  myListener->m_MsgReader->set_listener(myListener, mask);
  cout << "=== [ListenerDataSubscriber] Ready ..." << endl;
  myListener->m_closed = false;

  // waitset used to avoid spinning in the loop below
  DDS::WaitSet_var ws = new DDS::WaitSet();
  ws->attach_condition(myListener->m_guardCond);
  DDS::ConditionSeq condSeq;
  while (!myListener->m_closed && count < 1500 ){
    // To avoid spinning here. We can either use a sleep or better a WaitSet.
    ws->wait(condSeq, timeout);
    myListener->m_guardCond->set_trigger_value(false);
    ++count;
 }

  cout << "===[ListenerDataSubscriber] Market Closed" << endl;

  //cleanup
  mgr.deleteReader(myListener->m_MsgReader.in ());
  mgr.deleteSubscriber();
  mgr.deleteTopic();
  mgr.deleteParticipant();
  delete myListener;
  cout << "Completed Listener example." << endl;
  return 0;
}
