

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

#include "example_main.h"

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

int OSPL_MAIN (int argc, char *argv[])
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
  DDSEntityManager mgr (durability_kind);
  bool isPersistent = (strcmp(argv[1], "persistent") == 0);

  // create domain participant
  char partition_name[] = "Durability example";
  mgr.createParticipant(partition_name);

  //create type
  MsgTypeSupport_var mt = new MsgTypeSupport();
  mgr.registerType(mt.in());

  //create Topic
  std::string topic_name = "CPPDurabilityData_Msg";
  if (isPersistent) {
      topic_name = "PersistentCPPDurabilityData_Msg";
  }
  mgr.createTopic(topic_name.c_str());

  //create Subscriber
  mgr.createSubscriber();

  // create DataReader
  mgr.createReader();

  DataReader_var dreader = mgr.getReader();
  MsgDataReader_var DurabilityDataReader = MsgDataReader::_narrow(dreader.in());
  checkHandle(DurabilityDataReader.in(), "MsgDataReader::_narrow");

  // Wait 40 seconds for historical data
  Duration_t waitTime = {40,0};
  ReturnCode_t status = DurabilityDataReader->wait_for_historical_data(waitTime);

  cout << "=== [Subscriber] Ready ..." << endl;

  bool closed = false;
  while (!closed && status == RETCODE_OK)
  {
    status = DurabilityDataReader->take(msgList, infoSeq, LENGTH_UNLIMITED,
      ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
    checkStatus(status, "msgDataReader::take");
    for (DDS::ULong j = 0; j < msgList.length(); j++)
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

  // For single process mode wait some time to ensure persistent data is stored to disk
  os_time cleanupDelay = {2, 0};
  os_nanoSleep(cleanupDelay);


  //cleanup
  mgr.deleteReader(DurabilityDataReader.in ());
  mgr.deleteSubscriber();
  mgr.deleteTopic();
  mgr.deleteParticipant();

  return 0;
}
