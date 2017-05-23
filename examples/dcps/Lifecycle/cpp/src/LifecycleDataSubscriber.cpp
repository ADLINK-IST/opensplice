

/*
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
#include "vortex_os.h"

#include "example_main.h"

using namespace DDS;
using namespace LifecycleData;


const char *
pSampleState(DDS::SampleStateKind k)
{
    const char *r;

    switch (k) {
    case DDS::READ_SAMPLE_STATE:
        r = "READ_SAMPLE_STATE";
        break;
    case DDS::NOT_READ_SAMPLE_STATE:
        r = "NOT_READ_SAMPLE_STATE";
        break;
    default:
        r = "INVALID_SAMPLE_STATE";
        break;
    }

    return r;
}

const char *
pViewState(DDS::ViewStateKind k)
{
    const char *r;

    switch (k) {
    case DDS::NEW_VIEW_STATE:
        r = "NEW_VIEW_STATE";
        break;
    case DDS::NOT_NEW_VIEW_STATE:
        r = "NOT_NEW_VIEW_STATE";
        break;
    default:
        r = "INVALID_VIEW_STATE";
        break;
    }

    return r;
}

const char *
pInstanceState(DDS::ViewStateKind k)
{
    const char *r;

    switch (k) {
    case DDS::ALIVE_INSTANCE_STATE:
        r = "ALIVE_INSTANCE_STATE";
        break;
    case DDS::NOT_ALIVE_DISPOSED_INSTANCE_STATE:
        r = "NOT_ALIVE_DISPOSED_INSTANCE_STATE";
        break;
    case DDS::NOT_ALIVE_NO_WRITERS_INSTANCE_STATE:
        r = "NOT_ALIVE_NO_WRITERS_INSTANCE_STATE";
        break;
    default:
        r = "INVALID_INSTANCE_STATE";
        break;
    }

    return r;
}


int OSPL_MAIN (int argc, char *argv[])
{
  os_time delay_200ms = { 0, 200000000 };
  MsgSeq msgList;
  SampleInfoSeq infoSeq;

 // create domain participant
  char partition_name[] = "Lifecycle example";

  //------------------ Msg topic --------------------//

  DDSEntityManager mgr;

  // create domain participant
  mgr.createParticipant(partition_name);

  //create type
  MsgTypeSupport_var mt = new MsgTypeSupport();
  mgr.registerType(mt.in());

  //create Topic
  char topic_name[] = "Lifecycle_Msg";
  mgr.createTopic(topic_name);

  //create Subscriber
  mgr.createSubscriber();

  // create DataReader
  mgr.createReader();

  DataReader_var dreader = mgr.getReader();
  MsgDataReader_var LifecycleReader = MsgDataReader::_narrow(dreader.in());
  checkHandle(LifecycleReader.in(), "MsgDataReader::_narrow");

  cout << "=== [Subscriber] Ready ..." << endl;

  bool closed = false;
  ReturnCode_t status;
  int nbIter = 1;
  int nbIterMax = 100;
  while ((closed == false) && (nbIter < nbIterMax))
  {
      status = LifecycleReader->read(msgList, infoSeq, LENGTH_UNLIMITED,
        ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
      checkStatus(status, "msgDataReader::read");
      if (msgList.length() > 0)
      {
          for (DDS::ULong j = 0; j < msgList.length(); j++)
          {
            cout << endl << " Message        : " <<  msgList[j].message.in() << endl;
            cout << " writerStates   : " <<  msgList[j].writerStates.in() << endl;
            cout << " valid_data     : " << (int)infoSeq[j].valid_data  << endl;
            cout << "sample_state:" << pSampleState(infoSeq[j].sample_state) << "-view_state:" << pViewState(infoSeq[j].view_state) << "-instance_state:" << pInstanceState(infoSeq[j].instance_state) << endl;
            closed = (strcmp(msgList[j].writerStates.in(), "STOPPING_SUBSCRIBER") == 0);
          }
      }
      nbIter++;
      status = LifecycleReader->return_loan(msgList, infoSeq);
      checkStatus(status, "MsgDataReader::return_loan");
      os_nanoSleep(delay_200ms);
  }
  cout << "=== [Subscriber] stopping after "<< nbIter << " iterations - closed=" << closed << endl;
  if (nbIter == nbIterMax)  cout << "*** Error : max " << nbIterMax <<   "iterations reached" << endl;
  //cleanup
  // Lifecycle topic
  mgr.deleteReader(LifecycleReader.in ());
  mgr.deleteSubscriber();
  mgr.deleteTopic();
  mgr.deleteParticipant();
  return 0;
}
