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
 * LOGICAL_NAME:    QueryConditionSubscriber.c
 * FUNCTION:        Subscriber's main for the QueryCondition OpenSplice example.
 * MODULE:          OpenSplice QueryCondition example for the C programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation for the 'QueryConditionSubscriber' executable.
 *
 ***/
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

void usage()
{
   fprintf(stderr, "\n\n *** ERROR");
   fprintf(stderr, "\n *** usage: \n \t QueryConditionSubscriber <query string>");

   exit(-1);
}

int OSPL_MAIN (int argc, char *argv[])
{
   DDS_Subscriber QueryConditionDataSubscriber;
   DDS_DataReader QueryConditionDataDataReader;
   StockMarket_Stock* QueryConditionDataSample;
   DDS_sequence_StockMarket_Stock* msgList = DDS_sequence_StockMarket_Stock__alloc();
   DDS_SampleInfoSeq* infoSeq = DDS_SampleInfoSeq__alloc();
   c_bool isClosed = FALSE;
   unsigned long j;
   DDS_char* QueryConditionDataToSubscribe;
   DDS_char *query_string = "ticker=%0";
   DDS_QueryCondition queryCondition;
   os_time delay_200ms = { 0, 200000000 };
   int count = 0;

   // usage : QueryConditionSubscriber <topic's content filtering string>
   if( argc < 2 )
   {
      usage();
   }
   printf("\n\n Starting QueryConditionSubscriber...");
   printf("\n\n Parameter is \"%s\"", argv[1]);
   QueryConditionDataToSubscribe = argv[1];

   createParticipant("QueryCondition example");

   // Register Stock Topic's type in the DDS Domain.
   g_StockTypeSupport = (DDS_TypeSupport) StockMarket_StockTypeSupport__alloc();
   checkHandle(g_StockTypeSupport, "StockMarket_StockTypeSupport__alloc");
   registerStockType(g_StockTypeSupport);
   // Create Stock Topic in the DDS Domain.
   g_StockTypeName = StockMarket_StockTypeSupport_get_type_name(g_StockTypeSupport);
   g_StockTopic = createTopic("StockTrackerExclusive", g_StockTypeName);
   DDS_free(g_StockTypeName);
   DDS_free(g_StockTypeSupport);

   // Create the Subscriber's in the DDS Domain.
   QueryConditionDataSubscriber = createSubscriber();

   // Request a Reader from the the Subscriber.
   QueryConditionDataDataReader = createDataReader(QueryConditionDataSubscriber, g_StockTopic);

   // Create QueryCondition
   printf( "\n=== [QueryConditionSubscriber] Query : ticker = %s\n", QueryConditionDataToSubscribe );

   queryCondition = createQueryCondition(QueryConditionDataDataReader, query_string, (DDS_char*) QueryConditionDataToSubscribe);


   printf( "\n=== [QueryConditionSubscriber] Ready..." );

   do
   {
      g_status = StockMarket_StockDataReader_take_w_condition(QueryConditionDataDataReader,
               msgList,
               infoSeq,
               DDS_LENGTH_UNLIMITED,
               queryCondition);
      checkStatus(g_status, "StockMarket_StockDataReaderView_take");
      if( msgList->_length > 0 )
      {
         j = 0;
         do
         {
            QueryConditionDataSample = &msgList->_buffer[j];
            if( infoSeq->_buffer[j].valid_data )
            {
               printf("\n\n %s: %f", QueryConditionDataSample->ticker, QueryConditionDataSample->price);
               if( QueryConditionDataSample->price == (DDS_float) -1.0f )
               {
                  printf("\n ===[QueryConditionSubscriber] QueryConditionDataSample->price == -1.0f ");
                  isClosed = TRUE;
               }
            }
         }
         while( ++j < msgList->_length );

         g_status = StockMarket_StockDataReader_return_loan(QueryConditionDataDataReader, msgList, infoSeq);
         checkStatus(g_status, "StockMarket_StockDataReaderView_return_loan");
      }
      os_nanoSleep(delay_200ms);
      ++count;
   }
   while( isClosed == FALSE && count < 1500 );

   printf("\n\n=== [QueryConditionSubscriber] Market Closed");

   // Cleanup DDS from the created Entities.

   deleteQueryCondition(QueryConditionDataDataReader, queryCondition);
   deleteDataReader(QueryConditionDataSubscriber, QueryConditionDataDataReader);
   deleteSubscriber(QueryConditionDataSubscriber);
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
