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
 * LOGICAL_NAME:    OwnershipDataSubscriber.c
 * FUNCTION:        OwnershipDataSubscriber's main for the Ownership OpenSplice programming example.
 * MODULE:          OpenSplice OwnershipData example for the C programming language.
 * DATE             September 2010.
 ***********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <string.h>
#include "dds_dcps.h"
#include "CheckStatus.h"
#include "DDSEntitiesManager.h"

#include "example_main.h"

int OSPL_MAIN (int argc, char *argv[])
{
   DDS_Subscriber OwnershipDataSubscriber;
   DDS_DataReader OwnershipDataDataReader;
   OwnershipData_Stock* OwnershipDataSample;
   DDS_sequence_OwnershipData_Stock* msgList = DDS_sequence_OwnershipData_Stock__alloc();
   DDS_SampleInfoSeq* infoSeq = DDS_SampleInfoSeq__alloc();
   c_bool isClosed;
   int count = 0;
   unsigned long j;
   os_time os_delay200 = { 0, 200000000 };

   printf("\n\n Starting LifecycleSubscriber...");

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

   // Create the Subscriber's in the DDS Domain.
   OwnershipDataSubscriber = createSubscriber();

   // Request a Reader from the the Subscriber.
   OwnershipDataDataReader = createDataReader(OwnershipDataSubscriber, g_StockTopic);

   printf("\n\n ===[OwnershipDataSubscriber] Ready...");
   printf("\n\n    Ticker   Price   Publisher   ownership strength");
   isClosed = FALSE;

   do
   {
      g_status = OwnershipData_StockDataReader_take(OwnershipDataDataReader, msgList, infoSeq, 1, DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
      checkStatus(g_status, "OwnershipData_StockDataReaderView_take");
      if( msgList->_length > 0 )
      {
         j = 0;
         do
         {
            OwnershipDataSample = &msgList->_buffer[j];
            // When is this supposed to happen?
            // Needs comment...
            if( infoSeq->_buffer[j].valid_data )
            {
               printf("\n   %s %8.1f    %s        %d", OwnershipDataSample->ticker, OwnershipDataSample->price, OwnershipDataSample->publisher, OwnershipDataSample->strength);
               fflush(stdout);
               if( OwnershipDataSample->price < -0.0f )
               {
                  printf("\n ===[OwnershipDataSubscriber] OwnershipDataSample->price == -1.0f ");
                  fflush(stdout);
                  isClosed = TRUE;
                  break;
               }
            }
         }
         while( ++j < msgList->_length );

         g_status = OwnershipData_StockDataReader_return_loan(OwnershipDataDataReader, msgList, infoSeq);
         checkStatus(g_status, "OwnershipData_StockDataReaderView_return_loan");
      }

      os_nanoSleep(os_delay200);
      ++count;
   } while( isClosed == FALSE && count < 1500 );


   printf("\n\n ===[OwnershipDataSubscriber] Market closed");
   fflush(stdout);

   // Cleanup DDS from the created Entities.
   deleteDataReader(OwnershipDataSubscriber, OwnershipDataDataReader);
   deleteSubscriber(OwnershipDataSubscriber);
   deleteTopic(g_StockTopic);
   deleteParticipant();

   // Cleanup C allocations,
   // recursively freeing the allocated structures and sequences using the OpenSplice API.
   DDS_free(msgList);
   DDS_free(infoSeq);

   // Print out an empty line, just to let behind a clean new line for the shell..
   printf("\n\r");
   return 0;
}
