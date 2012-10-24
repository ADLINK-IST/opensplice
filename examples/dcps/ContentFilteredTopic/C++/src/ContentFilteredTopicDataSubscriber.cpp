
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


using namespace DDS;
using namespace ContentFilteredTopicData;

int main(int argc, char *argv[])
{
  // usage : ContentFilteredTopicDataSubscriber <subscription_string>
  const char *ContentFilteredTopicDataToSubscribe;
  os_time delay_200ms = { 0, 200000000 };

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

  DDSEntityManager *mgr = new DDSEntityManager();

  // create domain participant
  char partition_name[] = "ContentFilteredTopic example";
  mgr->createParticipant(partition_name);

  //create type
  StockTypeSupport_var st = new StockTypeSupport();
  mgr->registerType(st.in());

  //create Topic
  char topic_name[] = "StockTrackerExclusive";
  mgr->createTopic(topic_name);

  //create Subscriber
  mgr->createSubscriber();

  char sTopicName[] = "MyStockTopic";
  // create subscription filter
  ostringstream buf;
  buf << "ticker = '" << ContentFilteredTopicDataToSubscribe << "'";
  CORBA::String_var sFilter = CORBA::string_dup(buf.str().c_str());
  // Filter expr
  StringSeq sSeqExpr;
  sSeqExpr.length(0);
  // create topic
  mgr->createContentFilteredTopic(sTopicName, sFilter.in(), sSeqExpr);
  // create Filtered DataReader
  cout << "=== [ContentFilteredTopicDataSubscriber] Subscription filter : " << sFilter
    << endl;
  mgr->createReader(true);

  DataReader_ptr dreader = mgr->getReader();
  StockDataReader_var ContentFilteredTopicDataReader = StockDataReader::_narrow(dreader);
  checkHandle(ContentFilteredTopicDataReader, "StockDataReader::_narrow");

  cout << "=== [ContentFilteredTopicDataSubscriber] Ready ..." << endl;

  bool closed = false;
  ReturnCode_t status =  - 1;
  int count = 0;
  while (!closed && count < 1500) // We dont want the example to run indefinitely
  {
    status = ContentFilteredTopicDataReader->take(msgList, infoSeq, LENGTH_UNLIMITED,
      ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
    checkStatus(status, "ContentFilteredTopicDataDataReader::take");
    for (CORBA::ULong i = 0; i < msgList.length(); i++)
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
  mgr->deleteReader(ContentFilteredTopicDataReader.in ());
  mgr->deleteSubscriber();
  mgr->deleteFilteredTopic();
  mgr->deleteTopic();
  mgr->deleteParticipant();

  delete mgr;
  return 0;
}
