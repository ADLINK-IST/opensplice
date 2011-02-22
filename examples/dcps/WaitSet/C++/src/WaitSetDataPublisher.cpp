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
#include <sstream>
#include <iostream>
#include "DDSEntityManager.h"
#include "ccpp_WaitSetData.h"
#include "os.h"

using namespace DDS;
using namespace WaitSetData;

int main(int argc, char *argv[])
{
  os_time delay_500ms = { 0, 500000000 };
  DDSEntityManager *mgr = new DDSEntityManager();

  // create domain participant
  // create domain participant
  char partition_name[] = "WaitSet example";
  mgr->createParticipant(partition_name);

  //create type
  MsgTypeSupport_var mt = new MsgTypeSupport();
  mgr->registerType(mt.in());

  //create Topic
  char topic_name[] = "WaitSetData_Msg";
  mgr->createTopic(topic_name);

  //create Publisher
  mgr->createPublisher();

  // create DataWriter
  mgr->createWriter();

  // Publish Events
  DataWriter_ptr dwriter = mgr->getWriter();
  MsgDataWriter_var WaitSetDataWriter = MsgDataWriter::_narrow(dwriter);

  Msg msgInstance; /* Example on Stack */

  msgInstance.userID = 1;
  msgInstance.message = CORBA::string_dup("First Hello");
  cout << "=== [Publisher] writing a message containing :" << endl;
  cout << "    userID  : " << msgInstance.userID << endl;
  cout << "    Message : \"" << msgInstance.message << "\"" << endl;

  ReturnCode_t status = WaitSetDataWriter->write(msgInstance, DDS::HANDLE_NIL);
  checkStatus(status, "MsgDataWriter::write1");
  os_nanoSleep(delay_500ms);
  // Write a second message
  msgInstance.message = CORBA::string_dup("Hello again");
  status = WaitSetDataWriter->write(msgInstance, DDS::HANDLE_NIL);
  checkStatus(status, "MsgDataWriter::write2");

  cout << endl << "=== [Publisher] writing a message containing :" << endl;
  cout << "    userID  : " << msgInstance.userID << endl;
  cout << "    Message : \"" << msgInstance.message << "\"" << endl;
  os_nanoSleep(delay_500ms);
  
  // clean up
  status = WaitSetDataWriter->dispose(msgInstance, DDS::HANDLE_NIL);
  checkStatus(status, "MsgDataWriter::dispose");
  status = WaitSetDataWriter->unregister_instance(msgInstance, DDS::HANDLE_NIL);
  checkStatus(status, "MsgDataWriter::unregister_instance");

  /* Remove the DataWriters */
  mgr->deleteWriter(WaitSetDataWriter.in ());

  /* Remove the Publisher. */
  mgr->deletePublisher();

  /* Remove the Topics. */
  mgr->deleteTopic();

  /* Remove Participant. */
  mgr->deleteParticipant();

  delete mgr;
  return 0;
}
