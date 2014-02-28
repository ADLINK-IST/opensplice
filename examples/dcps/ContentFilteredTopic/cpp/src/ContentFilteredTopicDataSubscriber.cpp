
/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

/************************************************************************
 * LOGICAL_NAME:    ContentFilteredTopicDataSubscriber.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             September 2010.
 ***********************************************************************/
#include <string>
#include <sstream>
#include <iostream>
#include "DDSEntityManager.h"
#include "ccpp_ContentFilteredTopicData.h"

#include "example_main.h"
#define MAX_MSG_LEN 256

using namespace DDS;
using namespace StockMarket;

int OSPL_MAIN (int argc, char *argv[])
{
  // usage : ContentFilteredTopicDataSubscriber <subscription_string>
  const char *ContentFilteredTopicDataToSubscribe;
  os_time delay_200ms = { 0, 200000000 };
  char buf[MAX_MSG_LEN];

  if (argc > 1)
  {
    ContentFilteredTopicDataToSubscribe = argv[1];
  }
  else
  {
    cerr <<
      "*** [ContentFilteredTopicDataSubscriber] Subscription string not specified" <<
      endl;
    cerr << "*** usage : ContentFilteredTopicDataSubscriber <subscription_string>" <<
      endl;
    return  - 1;
  }

  StockSeq msgList;
  SampleInfoSeq infoSeq;

  DDSEntityManager mgr;

  // create domain participant
  char partition_name[] = "ContentFilteredTopic example";
  mgr.createParticipant(partition_name);

  //create type
  StockTypeSupport_var st = new StockTypeSupport();
  mgr.registerType(st.in());

  //create Topic
  char topic_name[] = "StockTrackerExclusive";
  mgr.createTopic(topic_name);

  //create Subscriber
  mgr.createSubscriber();

  char sTopicName[] = "MyStockTopic";
  // create subscription filter
  snprintf(buf, MAX_MSG_LEN, "ticker = '%s'", ContentFilteredTopicDataToSubscribe);
  DDS::String_var sFilter = DDS::string_dup(buf);
  // Filter expr
  StringSeq sSeqExpr;
  sSeqExpr.length(0);
  // create topic
  mgr.createContentFilteredTopic(sTopicName, sFilter.in(), sSeqExpr);
  // create Filtered DataReader
  cout << "=== [ContentFilteredTopicDataSubscriber] Subscription filter : " << sFilter
    << endl;
  mgr.createReader(true);

  DataReader_var dreader = mgr.getReader();
  StockDataReader_var ContentFilteredTopicDataReader = StockDataReader::_narrow(dreader.in());
  checkHandle(ContentFilteredTopicDataReader.in(), "StockDataReader::_narrow");

  cout << "=== [ContentFilteredTopicDataSubscriber] Ready ..." << endl;

  bool closed = false;
  ReturnCode_t status =  - 1;
  int count = 0;
  while (!closed && count < 1500) // We dont want the example to run indefinitely
  {
    status = ContentFilteredTopicDataReader->take(msgList, infoSeq, LENGTH_UNLIMITED,
      ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
    checkStatus(status, "ContentFilteredTopicDataDataReader::take");
    for (DDS::ULong i = 0; i < msgList.length(); i++)
    {
        if(infoSeq[i].valid_data)
        {
           if (msgList[i].price ==  - 1.0f)
           {
              closed = true;
              break;
           }
        }
     cout << "=== [ContentFilteredTopicDataSubscriber] receives stockQuote :  ("<< msgList[i].ticker << ", " << msgList[i].price << ')'<< endl;
    }

    status = ContentFilteredTopicDataReader->return_loan(msgList, infoSeq);
    checkStatus(status, "StockDataReader::return_loan");
    os_nanoSleep(delay_200ms);
    ++count;
  }

  cout << "=== [ContentFilteredTopicDataSubscriber] Market Closed" << endl;

  //cleanup
  mgr.deleteReader(ContentFilteredTopicDataReader.in ());
  mgr.deleteSubscriber();
  mgr.deleteFilteredTopic();
  mgr.deleteTopic();
  mgr.deleteParticipant();

  return 0;
}
