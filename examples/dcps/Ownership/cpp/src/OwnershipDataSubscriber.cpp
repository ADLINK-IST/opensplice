
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
 * LOGICAL_NAME:    Subscriber.cpp
 * FUNCTION:        OpenSplice Ownership example code.
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
#include "ccpp_OwnershipData.h"
#include "vortex_os.h"

#include "example_main.h"

using namespace DDS;
using namespace OwnershipData;

int OSPL_MAIN (int argc, char *argv[])
{
  os_time delay_2ms = { 0, 2000000 };
  os_time delay_200ms = { 0, 200000000 };
  StockSeq msgList;
  SampleInfoSeq infoSeq;

  DDSEntityManager mgr;

  // create domain participant
  char partition_name[] = "Ownership example";
  mgr.createParticipant(partition_name);

  //create type
  StockTypeSupport_var st = new StockTypeSupport();
  mgr.registerType(st.in());

  //create Topic
  char topic_name[] = "OwnershipStockTracker";
  mgr.createTopic(topic_name);

  //create Subscriber
  mgr.createSubscriber();

  // create DataReader
  mgr.createReader();

  DataReader_var dreader = mgr.getReader();
  StockDataReader_var OwnershipDataReader = StockDataReader::_narrow(dreader.in());
  checkHandle(OwnershipDataReader.in(), "StockDataReader::_narrow");
  cout << "===[Subscriber] Ready ..." << endl;
  cout << "   Ticker   Price   Publisher   ownership strength" << endl;
  bool closed = false;
  ReturnCode_t status =  - 1;
  int count = 0;
  while (!closed && count < 1500 )
  {
    status = OwnershipDataReader->take(msgList, infoSeq, LENGTH_UNLIMITED,
      ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
    checkStatus(status, "OwnershipDataDataReader::take");
    if (msgList.length() > 0)
    {
      for (DDS::ULong i = 0; i < msgList.length(); i++)
      {
        if (infoSeq[i].valid_data)
        {
          if (msgList[i].price < - 0.0f)
          {
            closed = true;
            break;
          }
          cout << "   " << msgList[i].ticker.in() << " " << msgList[i].price << " " << msgList[i].publisher.in() << " " << msgList[i].strength << endl;
        }
     }

      status = OwnershipDataReader->return_loan(msgList, infoSeq);
      checkStatus(status, "StockDataReader::return_loan");
      os_nanoSleep(delay_2ms);
    }
    ++count;
    os_nanoSleep(delay_200ms);
  }

  cout << "===[Subscriber] Market Closed" << endl;

  //cleanup
  mgr.deleteReader(OwnershipDataReader.in ());
  mgr.deleteSubscriber();
  mgr.deleteTopic();
  mgr.deleteParticipant();

  return 0;
}
