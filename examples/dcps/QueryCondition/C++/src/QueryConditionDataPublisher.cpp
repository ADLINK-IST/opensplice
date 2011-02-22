
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
 * LOGICAL_NAME:    QueryConditionDataPublisher.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             September 2010.
 ************************************************************************
 * 
 * This file contains the implementation for the 'QueryConditionDataPublisher' executable.
 * 
 ***/
#include <string>
#include <sstream>
#include <iostream>
#include "DDSEntityManager.h"
#include "ccpp_QueryConditionData.h"
#include "os.h"


using namespace DDS;
using namespace QueryConditionData;

int main(int argc, char *argv[])
{
  os_time delay_100ms = { 0, 100000000 };
  DDSEntityManager *mgr = new DDSEntityManager();

  // create domain participant
  char partition_name[] = "QueryCondition example";
  mgr->createParticipant(partition_name);

  StockTypeSupport_var st = new StockTypeSupport();
  mgr->registerType(st.in());

  //create Topic
  char topic_name[] = "StockTrackerExclusive";
  mgr->createTopic(topic_name);

  //create Publisher
  mgr->createPublisher();

  // create DataWriter
  mgr->createWriter();
  DataWriter_ptr dWriter = mgr->getWriter();
  StockDataWriter_var QueryConditionDataWriter = StockDataWriter::_narrow(dWriter);

  Stock *geQuote = new Stock();
  Stock *msftQuote = new Stock();

  geQuote->ticker = CORBA::string_dup("GE");
  geQuote->price = 12.00f;
  msftQuote->ticker = CORBA::string_dup("MSFT");
  msftQuote->price = 25.00f;


  InstanceHandle_t geHandle = QueryConditionDataWriter->register_instance(*geQuote);
  InstanceHandle_t msftHandle = QueryConditionDataWriter->register_instance(*msftQuote);

  // Publish Events
  ReturnCode_t status;
  // update QueryConditionData price every second
  for (int x = 0; x < 20; x++)
  {
    geQuote->price = geQuote->price + 0.5;
    msftQuote->price = msftQuote->price + 1.5;
    status = QueryConditionDataWriter->write(*geQuote, geHandle);
    checkStatus(status, "StockDataWriter::write");
    status = QueryConditionDataWriter->write(*msftQuote, msftHandle);
    checkStatus(status, "StockDataWriter::write");
    os_nanoSleep(delay_100ms);
    printf("GE : %.1f MSFT : %.1f\n", geQuote->price, msftQuote->price);
  }
  // signal to terminate
  geQuote->price =  - 1;
  msftQuote->price =  - 1;
  QueryConditionDataWriter->write(*geQuote, geHandle);
  QueryConditionDataWriter->write(*msftQuote, msftHandle);
  cout << "Market Closed" << endl;

  // clean up
  status = QueryConditionDataWriter->dispose(*geQuote, geHandle);
  checkStatus(status, "dispose");
  status = QueryConditionDataWriter->dispose(*msftQuote, msftHandle);
  checkStatus(status, "dispose");

  status = QueryConditionDataWriter->unregister_instance(*geQuote, geHandle);
  checkStatus(status, "unregister_instance");
  status = QueryConditionDataWriter->unregister_instance(*msftQuote, msftHandle);
  checkStatus(status, "unregister_instance");

  /* Release the data-samples. */
  delete geQuote;
  delete msftQuote;

  /* Remove the DataWriters */
  mgr->deleteWriter(QueryConditionDataWriter.in ());

  /* Remove the Publisher. */
  mgr->deletePublisher();

  /* Remove the Topics. */
  mgr->deleteTopic();

  /* Remove Participant. */
  mgr->deleteParticipant();

  delete mgr;
  return 0;
}
