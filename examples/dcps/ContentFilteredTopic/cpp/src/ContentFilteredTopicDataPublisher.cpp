
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
 * LOGICAL_NAME:    ContentFilteredTopicDataPublisher.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation for the 'ContentFilteredTopicDataPublisher' executable.
 *
 ***/
#include <string>
#include <sstream>
#include <iostream>
#include "DDSEntityManager.h"
#include "ccpp_ContentFilteredTopicData.h"
#include "vortex_os.h"

#include "example_main.h"

using namespace DDS;
using namespace StockMarket;

int OSPL_MAIN (int argc, char *argv[])
{
  os_time delay_100ms = { 0, 100000000 };
  DDSEntityManager mgr;

  // create domain participant
  char partition_name[] = "ContentFilteredTopic example";
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
  StockDataWriter_var ContentFilteredTopicDataWriter = StockDataWriter::_narrow(dWriter.in());

  Stock geQuote;
  Stock msftQuote;

  geQuote.ticker = DDS::string_dup("GE");
  geQuote.price = 12.00f;
  msftQuote.ticker = DDS::string_dup("MSFT");
  msftQuote.price = 25.00f;


  InstanceHandle_t geHandle = ContentFilteredTopicDataWriter->register_instance(geQuote);
  InstanceHandle_t msftHandle = ContentFilteredTopicDataWriter->register_instance(msftQuote);

  // Publish Events
  ReturnCode_t status;
  // update ContentFilteredTopicData price every second
  for (int x = 0; x < 20; x++)
  {
    geQuote.price = geQuote.price + 0.5f;
    msftQuote.price = msftQuote.price + 1.5f;
    cout << "=== [ContentFilteredTopicDataPublisher] sends 2 stockQuotes : (GE, " << geQuote.price << ") (MSFT, " << msftQuote.price << ")" << endl;
    status = ContentFilteredTopicDataWriter->write(geQuote, geHandle);
    checkStatus(status, "StockDataWriter::write");
    status = ContentFilteredTopicDataWriter->write(msftQuote, msftHandle);
    checkStatus(status, "StockDataWriter::write");
    os_nanoSleep(delay_100ms);
  }
  // signal to terminate
  geQuote.price =  - 1;
  msftQuote.price =  - 1;
  ContentFilteredTopicDataWriter->write(geQuote, geHandle);
  ContentFilteredTopicDataWriter->write(msftQuote, msftHandle);
  cout << "Market Closed" << endl;

  /* Unregister the instances */
  ContentFilteredTopicDataWriter->unregister_instance(geQuote, geHandle);
  ContentFilteredTopicDataWriter->unregister_instance(msftQuote, msftHandle);

  /* Remove the DataWriters */
  mgr.deleteWriter(ContentFilteredTopicDataWriter.in ());

  /* Remove the Publisher. */
  mgr.deletePublisher();

  /* Remove the Topics. */
  mgr.deleteTopic();

  /* Remove Participant. */
  mgr.deleteParticipant();

  return 0;
}
