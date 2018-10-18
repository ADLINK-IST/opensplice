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
 * LOGICAL_NAME:    ContentFilteredTopicDataPublisher.c
 * FUNCTION:        Publisher's main for the ContentFilteredTopicData OpenSplice example.
 * MODULE:          OpenSplice ContentFilteredTopic example for the C programming language.
 * DATE             September 2010.
 ************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include "os_stdlib.h"
#endif
#include <string.h>
#include "dds_dcps.h"
#include "CheckStatus.h"
#include "DDSEntitiesManager.h"

#include "example_main.h"

int OSPL_MAIN (int argc, char *argv[])
{
   char* partition_name = "ContentFilteredTopic example";
   int sampleIndex, nb_iteration;
   const char geTicker[] = "GE";
   DDS_unsigned_long geTickerLength = sizeof(geTicker) - 1;
   const char msftTicker[] = "MSFT";
   DDS_unsigned_long msftTickerLength = sizeof(msftTicker) - 1;
   os_time os_delay100 = { 0, 100000000 };
   os_time os_delay2000 = { 2, 0 };

   DDS_Publisher ContentFilteredTopicDataPublisher;
   DDS_DataWriter ContentFilteredTopicDataDataWriter;
   StockMarket_Stock* geStockSample;
   StockMarket_Stock* msftStockSample;
   DDS_InstanceHandle_t geInstanceHandle;
   DDS_InstanceHandle_t msftInstanceHandle;

   createParticipant( partition_name );

   // Register Stock Topic's type in the DDS Domain.
   g_StockTypeSupport = (DDS_TypeSupport) StockMarket_StockTypeSupport__alloc();
   checkHandle(g_StockTypeSupport, "StockMarket_StockTypeSupport__alloc");
   registerStockType(g_StockTypeSupport);
   // Create Stock Topic in the DDS Domain.
   g_StockTypeName = StockMarket_StockTypeSupport_get_type_name(g_StockTypeSupport);
   g_StockTopic = createTopic("StockTrackerExclusive", g_StockTypeName);
   DDS_free(g_StockTypeName);
   DDS_free(g_StockTypeSupport);

   // Create the Publisher's in the DDS Domain.
   ContentFilteredTopicDataPublisher = createPublisher();

   // Request a Writer from the the Publisher.
   ContentFilteredTopicDataDataWriter = createDataWriter(ContentFilteredTopicDataPublisher, g_StockTopic, FALSE);

   // Publish a Stock Sample reflecting the state of the Msg DataWriter.
   geStockSample = StockMarket_Stock__alloc();
   msftStockSample = StockMarket_Stock__alloc();

   msftStockSample->ticker = DDS_string_alloc(msftTickerLength);
   snprintf(msftStockSample->ticker, msftTickerLength + 1, "%s", msftTicker);
   msftStockSample->price = 25.00f;

   geStockSample->ticker = DDS_string_alloc(geTickerLength);
   snprintf(geStockSample->ticker, geTickerLength + 1, "%s", geTicker);
   geStockSample->price = 12.00f;

   geInstanceHandle = StockMarket_StockDataWriter_register_instance(ContentFilteredTopicDataDataWriter, geStockSample);
   msftInstanceHandle = StockMarket_StockDataWriter_register_instance(ContentFilteredTopicDataDataWriter, msftStockSample);

   nb_iteration = 20;

   printf("=== ContentFilteredTopicDataPublisher");

   // The subscriber should display the prices sent by the publisher with the highest ownership strength
   sampleIndex = 0;
   do
   {
      printf("\n=== [ContentFilteredTopicDataPublisher] sends 2 stockQuotes : (GE, %.1f) (MSFT, %.1f)", geStockSample->price, msftStockSample->price);
      writeStockSample(ContentFilteredTopicDataDataWriter, geInstanceHandle, geStockSample);
      writeStockSample(ContentFilteredTopicDataDataWriter, msftInstanceHandle, msftStockSample);
      geStockSample->price += 0.5;
      msftStockSample->price += 1.5;
      os_nanoSleep(os_delay100);
   }
   while( ++sampleIndex < nb_iteration );

   // This special price gives the end signal to the Subscriber:
   // signal to terminate
   geStockSample->price =  -1.0f;
   msftStockSample->price =  -1.0f;
   writeStockSample(ContentFilteredTopicDataDataWriter, geInstanceHandle, geStockSample);
   writeStockSample(ContentFilteredTopicDataDataWriter, msftInstanceHandle, msftStockSample);

   // This to make sure the Subscriber will get all the Samples.
   os_nanoSleep(os_delay2000);

   printf("\nMarket Closed\n");

   StockMarket_StockDataWriter_unregister_instance(ContentFilteredTopicDataDataWriter, geStockSample, geInstanceHandle);
   StockMarket_StockDataWriter_unregister_instance(ContentFilteredTopicDataDataWriter, msftStockSample, msftInstanceHandle);



   // Cleanup DDS from the created Entities.
   deleteDataWriter(ContentFilteredTopicDataPublisher, ContentFilteredTopicDataDataWriter);
   deletePublisher(ContentFilteredTopicDataPublisher);
   deleteTopic(g_StockTopic);
   deleteParticipant();

   // Cleanup C allocations,
   // recursively freeing the allocated structures and sequences using the OpenSplice API.
   DDS_free(geStockSample);
   DDS_free(msftStockSample);

   // Print out an empty line, just to let behind a clean new line for the shell..
   // Removed to comply with expected results
   //printf("\n\r");
   return 0;
}

