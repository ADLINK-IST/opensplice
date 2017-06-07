/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
 * LOGICAL_NAME:    OwnershipDataPublisher.c
 * FUNCTION:        Publisher's main for the OwnershipData OpenSplice example.
 * MODULE:          OpenSplice OwnershipData example for the C programming language.
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

void usage()
{
   fprintf(stderr, "\n\n *** ERROR");
   fprintf(stderr, "\n *** [Publisher] usage: \n \t Publisher <publisher_name> <ownership_strength> <nb_iterations>  <is_stopping_subscriber = 0 OR 1>");
   exit(-1);
}

int OSPL_MAIN (int argc, char *argv[])
{
   char* publisher_name;
   int i, sampleIndex, ownership_strength, nb_iteration;
   c_bool isStoppingSubscriber;
   float price;
   os_time os_delay200 = { 0, 200000000 };
   os_time os_delay2000 = { 2, 0 };


   DDS_Publisher OwnershipDataPublisher;
   DDS_DataWriter OwnershipDataDataWriter;
   OwnershipData_Stock* OwnershipDataSample;
   DDS_InstanceHandle_t userHandle = NULL;
   const char ticker [] = "MSFT";
   DDS_unsigned_long stringLength = sizeof(ticker) - 1;

   // usage : Publisher.exe <publisher_name> <ownership_strength> <nb_iterations>
   if( argc < 5 )
   {
      usage();
   }
   printf("\n\n Starting OwnershipDataPublisher...");
   printf("\n\n Parameters are:");
   for( i = 1; i < argc ; ++i )
   {
      printf("\n %d: %s", i, argv[i]);
   }

   publisher_name = argv[1];
   ownership_strength = atoi(argv[2]);
   nb_iteration = atoi(argv[3]);
   isStoppingSubscriber =  argv[4][0] == '1' ? TRUE : FALSE;

   createParticipant("Ownership example");

   // Register Stock Topic's type in the DDS Domain.
   g_StockTypeSupport = (DDS_TypeSupport) OwnershipData_StockTypeSupport__alloc();
   checkHandle(g_StockTypeSupport, "OwnershipData_StockTypeSupport__alloc");
   registerStockType(g_StockTypeSupport);
   // Create Stock Topic in the DDS Domain.
   g_StockTypeName = OwnershipData_StockTypeSupport_get_type_name(g_StockTypeSupport);
   g_StockTopic = createTopic("OwnershipStockTracker", g_StockTypeName);
   DDS_free(g_StockTypeName);
   DDS_free(g_StockTypeSupport);

   // Create the Publisher's in the DDS Domain.
   OwnershipDataPublisher = createPublisher();

   // Request a Writer from the the Publisher.
   OwnershipDataDataWriter = createDataWriter(OwnershipDataPublisher, g_StockTopic, FALSE, ownership_strength);

   // Publish a Stock Sample reflecting the state of the Msg DataWriter.
   OwnershipDataSample = OwnershipData_Stock__alloc();

   OwnershipDataSample->ticker = DDS_string_alloc(stringLength);
   snprintf(OwnershipDataSample->ticker, stringLength + 1, "%s", ticker);
   OwnershipDataSample->price = 0.0;
   OwnershipDataSample->publisher = DDS_string_dup(publisher_name);
   OwnershipDataSample->strength = ownership_strength;

   //Publisher publishes the prices in dollars
   printf("\n=== [Publisher] Publisher %d with strength : ", ownership_strength);
   printf("\n / sending %d prices...", nb_iteration);
   // The subscriber should display the prices sent by the publisher with the highest ownership strength
   price = 10.0f;
   for( sampleIndex = 0; sampleIndex < nb_iteration ; ++sampleIndex )
   {
      OwnershipDataSample->price = price;
      writeStockSample(OwnershipDataDataWriter, userHandle, OwnershipDataSample);
      os_nanoSleep(os_delay200);
      price = price + 0.5f;
   }
   if( isStoppingSubscriber == TRUE )
   {
      // This special price gives the end signal to the Subscriber:
      OwnershipDataSample->price =  (DDS_float) -1.0f;
      writeStockSample(OwnershipDataDataWriter, userHandle, OwnershipDataSample);
   }
   /* A short sleep ensures time is allowed for the sample to be written to the network.
   If the example is running in *Single Process Mode* exiting immediately might
   otherwise shutdown the domain services before this could occur */
   os_nanoSleep(os_delay2000);

   // Cleanup DDS from the created Entities.
   deleteDataWriter(OwnershipDataPublisher, OwnershipDataDataWriter);
   deletePublisher(OwnershipDataPublisher);
   deleteTopic(g_StockTopic);
   deleteParticipant();

   // Cleanup C allocations,
   // recursively freeing the allocated structures and sequences using the OpenSplice API.
   DDS_free(OwnershipDataSample);

   // Print out an empty line, just to let behind a clean new line for the shell..
   printf("\n\r");
   return 0;
}

