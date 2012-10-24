/*************************************************************************
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
#include <sstream>
#include <iostream>
#include "DDSEntityManager.h"
#include "ccpp_ListenerData.h"

#include "ListenerDataListener.h"
#include "os.h"

using namespace DDS;
using namespace ListenerData;

int main(int argc, char *argv[])
{
  MsgSeq msgList;
  SampleInfoSeq infoSeq;
  Duration_t timeout = { 0, 200000000 };
  int count = 0;

  DDSEntityManager *mgr = new DDSEntityManager();

   // create domain participant
  char partition_name[] = "Listener example";
  mgr->createParticipant(partition_name);

  //create type
  MsgTypeSupport_var st = new MsgTypeSupport();
  mgr->registerType(st.in());

  //create Topic
  char topic_name[] = "ListenerData_Msg";
  mgr->createTopic(topic_name);

  //create Subscriber
  mgr->createSubscriber();

  // create DataReader
  mgr->createReader();

  DataReader_ptr dreader = mgr->getReader();
  //MsgDataReader_var MsgReader = MsgDataReader::_narrow(dreader);
  ListenerDataListener *myListener = new ListenerDataListener();
  myListener->m_MsgReader = MsgDataReader::_narrow(dreader);
  checkHandle(myListener->m_MsgReader, "MsgDataReader::_narrow");

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
    // msleep (2);
    // cout << endl << "=== [ListenerDataSubscriber] waiting waitset ..." << endl;
    ws->wait(condSeq, timeout);
	myListener->m_guardCond->set_trigger_value(FALSE);
	++count;
 }

  cout << "===[ListenerDataSubscriber] Market Closed" << endl;

  //cleanup
  mgr->deleteReader(myListener->m_MsgReader.in ());
  mgr->deleteSubscriber();
  mgr->deleteTopic();
  mgr->deleteParticipant();

  delete mgr;
  return 0;
}
