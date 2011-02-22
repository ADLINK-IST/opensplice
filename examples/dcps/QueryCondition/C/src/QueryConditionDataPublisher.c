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
 * LOGICAL_NAME:    QueryConditionDataPublisher.c
 * FUNCTION:        Publisher's main for the QueryCondition OpenSplice example.
 * MODULE:          OpenSplice QueryCondition example for the C programming language.
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
   char* publisher_name;
   int i, sampleIndex, nb_iteration;
   c_bool isStoppingSubscriber;
   float price;
   char* geTicker = "GE";
   int geTickerLength = strlen(geTicker);
   char* msftTicker = "MSFT";
   int msftTickerLength = strlen(msftTicker);
   os_time os_delay100 = { 0, 100000000 };
   os_time os_delay2000 =  { 2, 0 };

   DDS_Publisher QueryConditionDataPublisher;
   DDS_DataWriter QueryConditionDataDataWriter;
   QueryConditionData_Stock* geStockSample;
   QueryConditionData_Stock* msftStockSample;
   DDS_InstanceHandle_t geInstanceHandle;
   DDS_InstanceHandle_t msftInstancetHandle;

   createParticipant("QueryCondition example");

   // Register Stock Topic's type in the DDS Domain.
   g_StockTypeSupport = (DDS_TypeSupport) QueryConditionData_StockTypeSupport__alloc();
   checkHandle(g_StockTypeSupport, "QueryConditionData_StockTypeSupport__alloc");
   registerStockType(g_StockTypeSupport);
   // Create Stock Topic in the DDS Domain.
   g_StockTypeName = (char*) QueryConditionData_StockTypeSupport_get_type_name(g_StockTypeSupport);
   g_StockTopic = createTopic("StockTrackerExclusive", g_StockTypeName);
   DDS_free(g_StockTypeName);
   DDS_free(g_StockTypeSupport);

   // Create the Publisher's in the DDS Domain.
   QueryConditionDataPublisher = createPublisher();

   // Request a Writer from the the Publisher.
   QueryConditionDataDataWriter = createDataWriter(QueryConditionDataPublisher, g_StockTopic, FALSE);

   // Publish a Stock Sample reflecting the state of the Msg DataWriter.
   geStockSample = QueryConditionData_Stock__alloc();
   msftStockSample = QueryConditionData_Stock__alloc();

   msftStockSample->ticker = DDS_string_alloc(msftTickerLength);
   snprintf(msftStockSample->ticker, msftTickerLength + 1, "%s", msftTicker);
   msftStockSample->price = 12.00f;

   geStockSample->ticker = DDS_string_alloc(geTickerLength);
   snprintf(geStockSample->ticker, geTickerLength + 1, "%s", geTicker);
   geStockSample->price = 25.00f;

   geInstanceHandle = QueryConditionData_StockDataWriter_register_instance(QueryConditionDataDataWriter, geStockSample);
   msftInstancetHandle = QueryConditionData_StockDataWriter_register_instance(QueryConditionDataDataWriter, msftStockSample);   

   nb_iteration = 20;
   // The subscriber should display the prices sent by the publisher with the highest ownership strength
   for( sampleIndex = 0; sampleIndex < nb_iteration ; ++sampleIndex )
   {
      printf("GE : %.1f MSFT : %.1f\n", geStockSample->price, msftStockSample->price);
      writeStockSample(QueryConditionDataDataWriter, geInstanceHandle, geStockSample);
      writeStockSample(QueryConditionDataDataWriter, msftInstancetHandle, msftStockSample);
      geStockSample->price += 0.5;
      msftStockSample->price += 1.5;

      os_nanoSleep(os_delay100);
   }

   // This special price gives the end signal to the Subscriber:
   // signal to terminate
   geStockSample->price =  -1.0f;
   msftStockSample->price =  -1.0f;
   writeStockSample(QueryConditionDataDataWriter, geInstanceHandle, geStockSample);
   writeStockSample(QueryConditionDataDataWriter, msftInstancetHandle, msftStockSample);

   // This to make sure the Subscriber will get all the Samples.
   os_nanoSleep(os_delay2000);
   printf("Market Closed");

   QueryConditionData_StockDataWriter_unregister_instance (QueryConditionDataDataWriter, geStockSample, geInstanceHandle);
   QueryConditionData_StockDataWriter_unregister_instance (QueryConditionDataDataWriter, msftStockSample, msftInstancetHandle);

   // Cleanup DDS from the created Entities.
   deleteDataWriter(QueryConditionDataPublisher, QueryConditionDataDataWriter);
   deletePublisher(QueryConditionDataPublisher);
   deleteTopic(g_StockTopic);
   deleteParticipant();

   // Cleanup C allocations,
   // recursively freeing the allocated structures and sequences using the OpenSplice API.
   DDS_free(geStockSample);
   DDS_free(msftStockSample);

   // Print out an empty line, just to let behind a clean new line for the shell..
   printf("\n\r");
   return 0;
}

