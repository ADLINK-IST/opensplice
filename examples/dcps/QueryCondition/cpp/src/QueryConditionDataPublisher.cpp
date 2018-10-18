
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
#include "vortex_os.h"

#include "example_main.h"

using namespace DDS;
using namespace StockMarket;

int OSPL_MAIN (int argc, char *argv[])
{
  os_time delay_100ms = { 0, 100000000 };
  DDSEntityManager mgr;

  // create domain participant
  char partition_name[] = "QueryCondition example";
  mgr.createParticipant(partition_name);

  StockTypeSupport_var st = new StockTypeSupport();
  mgr.registerType(st.in());

  //create Topic
  char topic_name[] = "StockTrackerExclusive";
  mgr.createTopic(topic_name);

  //create Publisher
  mgr.createPublisher();

  // create DataWriter
  mgr.createWriter();
  DataWriter_var dWriter = mgr.getWriter();
  StockDataWriter_var QueryConditionDataWriter = StockDataWriter::_narrow(dWriter.in());

  Stock *geQuote = new Stock();
  Stock *msftQuote = new Stock();

  geQuote->ticker = DDS::string_dup("GE");
  geQuote->price = 12.00f;
  msftQuote->ticker = DDS::string_dup("MSFT");
  msftQuote->price = 25.00f;


  InstanceHandle_t geHandle = QueryConditionDataWriter->register_instance(*geQuote);
  InstanceHandle_t msftHandle = QueryConditionDataWriter->register_instance(*msftQuote);

  // Publish Events
  ReturnCode_t status;
  // update QueryConditionData price every second
  for (int x = 0; x < 20; x++)
  {
    geQuote->price = geQuote->price + 0.5f;
    msftQuote->price = msftQuote->price + 1.5f;
    status = QueryConditionDataWriter->write(*geQuote, geHandle);
    checkStatus(status, "StockDataWriter::write");
    status = QueryConditionDataWriter->write(*msftQuote, msftHandle);
    checkStatus(status, "StockDataWriter::write");
    os_nanoSleep(delay_100ms);
    cout << "GE : " << geQuote->price << " MSFT : " << msftQuote->price << endl;
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
  mgr.deleteWriter(QueryConditionDataWriter.in ());

  /* Remove the Publisher. */
  mgr.deletePublisher();

  /* Remove the Topics. */
  mgr.deleteTopic();

  /* Remove Participant. */
  mgr.deleteParticipant();

  return 0;
}
