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
 * LOGICAL_NAME:    OwnershipDataPublisher.c
 * FUNCTION:        Publisher's main for the OwnershipData OpenSplice example.
 * MODULE:          OpenSplice OwnershipData example for the C programming language.
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

void usage()
{
   fprintf(stderr, "\n\n *** ERROR");
   fprintf(stderr, "\n *** [Publisher] usage: \n \t Publisher <publisher_name> <ownership_strength> <nb_iterations>  <is_stopping_subscriber = 0 OR 1>");
   exit(-1);
}

int main(int argc, char *argv[])
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
   DDS_InstanceHandle_t userHandle;
   char* ticker = "MSFT";
   int stringLength;

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
   g_StockTypeName = (char*) OwnershipData_StockTypeSupport_get_type_name(g_StockTypeSupport);
   g_StockTopic = createTopic("StockTrackerExclusive", g_StockTypeName);
   DDS_free(g_StockTypeName);
   DDS_free(g_StockTypeSupport);

   // Create the Publisher's in the DDS Domain.
   OwnershipDataPublisher = createPublisher();

   // Request a Writer from the the Publisher.
   OwnershipDataDataWriter = createDataWriter(OwnershipDataPublisher, g_StockTopic, FALSE);

   setOwnershipStrength(OwnershipDataDataWriter, ownership_strength);

   // Publish a Stock Sample reflecting the state of the Msg DataWriter.
   OwnershipDataSample = OwnershipData_Stock__alloc();

   stringLength = strlen(ticker);
   OwnershipDataSample->ticker = DDS_string_alloc(stringLength);
   snprintf(OwnershipDataSample->ticker, stringLength + 1, "%s", ticker);
   OwnershipDataSample->price = 0.0;
   OwnershipDataSample->publisher = DDS_string_alloc(strlen(publisher_name)+1);
   snprintf(OwnershipDataSample->publisher, strlen(publisher_name) + 1, "%s", publisher_name);
   OwnershipDataSample->strength = ownership_strength;

   userHandle = OwnershipData_StockDataWriter_register_instance(OwnershipDataDataWriter, OwnershipDataSample);

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
      price = price + 0.5;
   }
   if( isStoppingSubscriber == TRUE )
   {
      // This special price gives the end signal to the Subscriber:
      OwnershipDataSample->price =  (DDS_float) -1.0f;
      writeStockSample(OwnershipDataDataWriter, userHandle, OwnershipDataSample);
   }
   // This to make sure the Subscriber will get all the Samples.
   os_nanoSleep(os_delay2000);

   OwnershipData_StockDataWriter_unregister_instance (OwnershipDataDataWriter, OwnershipDataSample, userHandle);
   

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

