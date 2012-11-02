
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
 * LOGICAL_NAME:    QueryConditionDataQuerySubscriber.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation for the 'QueryConditionDataContentSubscriber' executable.
 *
 ***/
#include <string>
#include <sstream>
#include <iostream>
#include "DDSEntityManager.h"
#include "ccpp_QueryConditionData.h"


using namespace DDS;
using namespace QueryConditionData;

int main(int argc, char *argv[])
{
  // usage : QueryConditionDataQuerySubscriber <query_string>
  const char *QueryConditionDataToSubscribe;
  if (argc > 1)
  {
    QueryConditionDataToSubscribe = argv[1];
  }
  else
  {
    cerr << "*** [QueryConditionDataQuerySubscriber] Query string not specified" <<
      endl;
    cerr << "*** usage : QueryConditionDataQuerySubscriber <query_string>" << endl;
    return  - 1;
  }

  StockSeq msgList;
  SampleInfoSeq infoSeq;
  os_time delay_200ms = { 0, 200000000 };

  DDSEntityManager *mgr = new DDSEntityManager();

  // create domain participant
  char partition_name[] = "QueryCondition example";
  mgr->createParticipant(partition_name);

  //create type
  StockTypeSupport_var st = new StockTypeSupport();
  mgr->registerType(st.in());

  //create Topic
  char topic_name[] = "StockTrackerExclusive";
  mgr->createTopic(topic_name);

  //create Subscriber
  mgr->createSubscriber();

  mgr->createReader(false);

  DataReader_ptr dreader = mgr->getReader();
  StockDataReader_var QueryConditionDataReader = StockDataReader::_narrow(dreader);
  checkHandle(QueryConditionDataReader, "StockDataReader::_narrow");

  // create a query string
  StringSeq queryStr;
  queryStr.length(1);
  queryStr[0] = QueryConditionDataToSubscribe;

  // Create QueryCondition
  cout << "=== [QueryConditionDataQuerySubscriber] Query : ticker = " <<
    QueryConditionDataToSubscribe << endl;
  QueryCondition_var queryCondition = QueryConditionDataReader->create_querycondition(ANY_SAMPLE_STATE,
    ANY_VIEW_STATE, ANY_INSTANCE_STATE, "ticker=%0", queryStr);
  checkHandle(queryCondition.in(), "create_querycondition");

  cout << "=== [QueryConditionDataQuerySubscriber] Ready ..." << endl;

  bool closed = false;
  ReturnCode_t status =  - 1;
  int count = 0;
  while (!closed && count < 1500 )
  {
    status = QueryConditionDataReader->take_w_condition(msgList, infoSeq, LENGTH_UNLIMITED,
      queryCondition);
    checkStatus(status, "QueryConditionDataDataReader::take");
    for (CORBA::ULong i = 0; i < msgList.length(); i++)
    {
      if (msgList[i].price ==  - 1.0f)
      {
        closed = true;
        break;
      }
      cout << msgList[i].ticker << ": " << msgList[i].price << endl;

    }

    status = QueryConditionDataReader->return_loan(msgList, infoSeq);
    checkStatus(status, "StockDataReader::return_loan");
    os_nanoSleep(delay_200ms);
    ++count;
  }

  cout << "=== [QueryConditionDataQuerySubscriber] Market Closed" << endl;

  // cleanup
  // Delete the QueryCondition
  QueryConditionDataReader->delete_readcondition(queryCondition.in());

  mgr->deleteReader(QueryConditionDataReader.in ());
  mgr->deleteSubscriber();
  mgr->deleteTopic();
  mgr->deleteParticipant();

  delete mgr;
  return 0;
}
