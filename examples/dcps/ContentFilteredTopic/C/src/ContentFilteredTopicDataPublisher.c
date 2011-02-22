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
 * LOGICAL_NAME:    ContentFilteredTopicDataPublisher.c
 * FUNCTION:        Publisher's main for the ContentFilteredTopicData OpenSplice example.
 * MODULE:          OpenSplice ContentFilteredTopic example for the C programming language.
 * DATE             September 2010.
 ************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <string.h>
#include "dds_dcps.h"
#include "CheckStatus.h"
#include "DDSEntitiesManager.h"

int main(int argc, char *argv[])
{
   char* partition_name = "ContentFilteredTopic example";
   char* publisher_name;
   int i, sampleIndex, nb_iteration;
   c_bool isStoppingSubscriber;
   float price;
   char* geTicker = "GE";
   int geTickerLength = strlen(geTicker);
   char* msftTicker = "MSFT";
   int msftTickerLength = strlen(msftTicker);
   os_time os_delay100 = { 0, 100000000 };
   os_time os_delay2000 = { 2, 0 };

   DDS_Publisher ContentFilteredTopicDataPublisher;
   DDS_DataWriter ContentFilteredTopicDataDataWriter;
   ContentFilteredTopicData_Stock* geStockSample;
   ContentFilteredTopicData_Stock* msftStockSample;
   DDS_InstanceHandle_t geInstanceHandle;
   DDS_InstanceHandle_t msftInstanceHandle;

   createParticipant( partition_name );

   // Register Stock Topic's type in the DDS Domain.
   g_StockTypeSupport = (DDS_TypeSupport) ContentFilteredTopicData_StockTypeSupport__alloc();
   checkHandle(g_StockTypeSupport, "ContentFilteredTopicData_StockTypeSupport__alloc");
   registerStockType(g_StockTypeSupport);
   // Create Stock Topic in the DDS Domain.
   g_StockTypeName = (char*) ContentFilteredTopicData_StockTypeSupport_get_type_name(g_StockTypeSupport);
   g_StockTopic = createTopic("StockTrackerExclusive", g_StockTypeName);
   DDS_free(g_StockTypeName);
   DDS_free(g_StockTypeSupport);

   // Create the Publisher's in the DDS Domain.
   ContentFilteredTopicDataPublisher = createPublisher();

   // Request a Writer from the the Publisher.
   ContentFilteredTopicDataDataWriter = createDataWriter(ContentFilteredTopicDataPublisher, g_StockTopic, FALSE);

   // Publish a Stock Sample reflecting the state of the Msg DataWriter.
   geStockSample = ContentFilteredTopicData_Stock__alloc();
   msftStockSample = ContentFilteredTopicData_Stock__alloc();

   msftStockSample->ticker = DDS_string_alloc(msftTickerLength);
   snprintf(msftStockSample->ticker, msftTickerLength + 1, "%s", msftTicker);
   msftStockSample->price = 25.00f;

   geStockSample->ticker = DDS_string_alloc(geTickerLength);
   snprintf(geStockSample->ticker, geTickerLength + 1, "%s", geTicker);
   geStockSample->price = 12.00f;

   geInstanceHandle = ContentFilteredTopicData_StockDataWriter_register_instance(ContentFilteredTopicDataDataWriter, geStockSample);
   msftInstanceHandle = ContentFilteredTopicData_StockDataWriter_register_instance(ContentFilteredTopicDataDataWriter, msftStockSample);  

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

   ContentFilteredTopicData_StockDataWriter_unregister_instance(ContentFilteredTopicDataDataWriter, geStockSample, geInstanceHandle);
   ContentFilteredTopicData_StockDataWriter_unregister_instance(ContentFilteredTopicDataDataWriter, msftStockSample, msftInstanceHandle);



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

