

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
 * LOGICAL_NAME:    DurabilityDataSubscriber.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             September 2010.
 ************************************************************************
 * 
 * This file contains the implementation of the Subscriber for the 'Durability' example.
 * 
 ***/
#include <string>
#include <sstream>
#include <iostream>
#include "DDSEntityManager.h"
#include "ccpp_DurabilityData.h"


using namespace DDS;
using namespace DurabilityData;

void usage()
{
  //usage : DurabilityDataSubscriber <durability_kind>
  //		. durability_kind = transient | persistent
  cerr << "*** ERROR" << endl;
  cerr << "*** usage : DurabilityDataSubscriber <durability_kind>"
    << endl;
  cerr << "***         . durability_kind = transient | persistent" << endl;
  exit( - 1);
}

int main(int argc, char *argv[])
{
  MsgSeq msgList;
  SampleInfoSeq infoSeq;

  if (argc < 2)
  {
    usage();
  }
  if (strcmp(argv[1], "transient") && strcmp(argv[1], "persistent"))
  {
    usage();
  }
  string durability_kind(argv[1]);
  DDSEntityManager *mgr = new DDSEntityManager(durability_kind);

  // create domain participant
  char partition_name[] = "Durability example";
  mgr->createParticipant(partition_name);

  //create type
  MsgTypeSupport_var mt = new MsgTypeSupport();
  mgr->registerType(mt.in());

  //create Topic
  char topic_name[] = "DurabilityData_Msg";
  mgr->createTopic(topic_name);

  //create Subscriber
  mgr->createSubscriber();

  // create DataReader
  mgr->createReader();

  DataReader_ptr dreader = mgr->getReader();
  MsgDataReader_var DurabilityDataReader = MsgDataReader::_narrow(dreader);
  checkHandle(DurabilityDataReader, "MsgDataReader::_narrow");

  cout << "=== [Subscriber] Ready ..." << endl;

  bool closed = false;
  ReturnCode_t status =  - 1;
  while (!closed)
  {
    status = DurabilityDataReader->take(msgList, infoSeq, LENGTH_UNLIMITED,
      ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
    checkStatus(status, "msgDataReader::take");
    for (CORBA::ULong j = 0; j < msgList.length(); j++)
    {
      if (infoSeq[j].valid_data)
      {
        cout << msgList[j].content << endl;
        if (strcmp(msgList[j].content, "9") == 0)
          closed = true;
      }
    }

    status = DurabilityDataReader->return_loan(msgList, infoSeq);
    checkStatus(status, "MsgDataReader::return_loan");
  }

  //cleanup
  mgr->deleteReader(DurabilityDataReader.in ());
  mgr->deleteSubscriber();
  mgr->deleteTopic();
  mgr->deleteParticipant();

  delete mgr;
  return 0;
}
