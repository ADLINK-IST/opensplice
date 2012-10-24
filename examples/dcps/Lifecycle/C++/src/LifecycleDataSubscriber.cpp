

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
 * LOGICAL_NAME:    LifecycleSubscriber.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             September 2010.
 ************************************************************************
 * 
 * This file contains the implementation for the 'LifecycleSubscriber' executable.
 * 
 ***/
#include <string>
#include <sstream>
#include <iostream>
#include "DDSEntityManager.h"
#include "ccpp_LifecycleData.h"
#include "os.h"

using namespace DDS;
using namespace LifecycleData;

char *sSampleState[] = 
{
  (char*)"READ_SAMPLE_STATE", (char*)"NOT_READ_SAMPLE_STATE"
};
char *sViewState[] = 
{
  (char*)"NEW_VIEW_STATE", (char*)"NOT_NEW_VIEW_STATE"
};
char *sInstanceState[] = 
{
  (char*)"ALIVE_INSTANCE_STATE", (char*)"NOT_ALIVE_DISPOSED_INSTANCE_STATE", 
    (char*)"NOT_ALIVE_NO_WRITERS_INSTANCE_STATE"
};
int index(int i)
{
  int j = (log10((float)i) / log10((float)2));
  return j;
}

int main(int argc, char *argv[])
{
  os_time delay_20ms =  { 0, 20000000 };
  os_time delay_200ms = { 0, 200000000 };
  MsgSeq msgList;
  SampleInfoSeq infoSeq;
  
 // create domain participant
  char partition_name[] = "Lifecycle example";
 
  //------------------ Msg topic --------------------//

  DDSEntityManager *mgr = new DDSEntityManager();

  // create domain participant
  mgr->createParticipant(partition_name);

  //create type
  MsgTypeSupport_var mt = new MsgTypeSupport();
  mgr->registerType(mt.in());

  //create Topic
  char topic_name[] = "Lifecycle_Msg";
  mgr->createTopic(topic_name);
 
  //create Subscriber
  mgr->createSubscriber();

  // create DataReader
  mgr->createReader();

  DataReader_ptr dreader = mgr->getReader();
  MsgDataReader_var LifecycleReader = MsgDataReader::_narrow(dreader);
  checkHandle(LifecycleReader, "MsgDataReader::_narrow");

  cout << "=== [Subscriber] Ready ..." << endl;

  bool closed = false;
  ReturnCode_t status;
  int nbIter = 1;
  int nbIterMax = 100;
  while ((closed == FALSE) && (nbIter < nbIterMax))
  {
      status = LifecycleReader->read(msgList, infoSeq, LENGTH_UNLIMITED,
        ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
      checkStatus(status, "msgDataReader::read");
      for (CORBA::ULong j = 0; j < msgList.length(); j++)
      { 
        cout << endl << " Message        : " <<  msgList[j].message.in() << endl;
        cout << " writerStates   : " <<  msgList[j].writerStates.in() << endl;
        cout << " valid_data     : " << (int)infoSeq[j].valid_data  << endl;
        cout << "sample_state:" << sSampleState[index(infoSeq[j].sample_state)] << "-view_state:" << sViewState[index(infoSeq[j].view_state)] << "-instance_state:" << sInstanceState[index(infoSeq[j].instance_state)] << endl;
        os_nanoSleep(delay_200ms);
	closed = (strcmp(msgList[j].writerStates.in(), "STOPPING_SUBSCRIBER") == 0);
      }
      status = LifecycleReader->return_loan(msgList, infoSeq);
      checkStatus(status, "MsgDataReader::return_loan");
      os_nanoSleep(delay_20ms);
      nbIter++;
  }
  cout << "=== [Subscriber] stopping after "<< nbIter << " iterations - closed=" << closed << endl;
  if (nbIter == nbIterMax)  cout << "*** Error : max " << nbIterMax <<   "iterations reached" << endl;
  //cleanup
  // Lifecycle topic
  mgr->deleteReader(LifecycleReader.in ());
  mgr->deleteSubscriber();
  mgr->deleteTopic();
  mgr->deleteParticipant();
  delete mgr;
  return 0;
}
