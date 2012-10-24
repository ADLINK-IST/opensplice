
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
#include "os.h"

using namespace DDS;
using namespace OwnershipData;

int main(int argc, char *argv[])
{
  os_time delay_2ms = { 0, 2000000 };
  os_time delay_200ms = { 0, 200000000 };
  StockSeq msgList;
  SampleInfoSeq infoSeq;

  DDSEntityManager *mgr = new DDSEntityManager();

  // create domain participant
  char partition_name[] = "Ownership example";
  mgr->createParticipant(partition_name);

  //create type
  StockTypeSupport_var st = new StockTypeSupport();
  mgr->registerType(st.in());

  //create Topic
  char topic_name[] = "StockTrackerExclusive";
  mgr->createTopic(topic_name);

  //create Subscriber
  mgr->createSubscriber();

  // create DataReader
  mgr->createReader();

  DataReader_ptr dreader = mgr->getReader();
  StockDataReader_var OwnershipDataReader = StockDataReader::_narrow(dreader);
  checkHandle(OwnershipDataReader, "StockDataReader::_narrow");
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
      for (CORBA::ULong i = 0; i < msgList.length(); i++)
      {
        if (infoSeq[i].valid_data)
        {
          if (msgList[i].price < - 0.0f)
          {
            closed = true;
            break;
          }
          printf("   %s %8.1f    %s        %d\n", msgList[i].ticker.in(),
            msgList[i].price, msgList[i].publisher.in(), msgList[i].strength);
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
  mgr->deleteReader(OwnershipDataReader.in ());
  mgr->deleteSubscriber();
  mgr->deleteTopic();
  mgr->deleteParticipant();

  delete mgr;
  return 0;
}
