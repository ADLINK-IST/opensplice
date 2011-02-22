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

void usage()
{
   fprintf(stderr, "\n\n *** ERROR");
   fprintf(stderr, "\n *** usage: \n \t QueryConditionSubscriber <query string>");

   exit(-1);
}

int main(int argc, char *argv[])
{
   DDS_Subscriber QueryConditionDataSubscriber;
   DDS_DataReader QueryConditionDataDataReader;
   QueryConditionData_Stock* QueryConditionDataSample;
   DDS_InstanceHandle_t userHandle;
   DDS_sequence_QueryConditionData_Stock* msgList = DDS_sequence_QueryConditionData_Stock__alloc();
   DDS_SampleInfoSeq* infoSeq = DDS_SampleInfoSeq__alloc();
   c_bool isClosed = FALSE;
   unsigned long j, userInput, timeOut;
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
   g_StockTypeSupport = (DDS_TypeSupport) QueryConditionData_StockTypeSupport__alloc();
   checkHandle(g_StockTypeSupport, "QueryConditionData_StockTypeSupport__alloc");
   registerStockType(g_StockTypeSupport);
   // Create Stock Topic in the DDS Domain.
   g_StockTypeName = (char*) QueryConditionData_StockTypeSupport_get_type_name(g_StockTypeSupport);
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
      g_status = QueryConditionData_StockDataReader_take_w_condition(QueryConditionDataDataReader,
               msgList,
               infoSeq,
               DDS_LENGTH_UNLIMITED,
               queryCondition);
      checkStatus(g_status, "QueryConditionData_StockDataReaderView_take");
      if( msgList->_length > 0 )
      {
         j = 0;
         do
         {
            QueryConditionDataSample = &msgList->_buffer[j];
            // When is this supposed to happen?
            // Needs comment...
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

         g_status = QueryConditionData_StockDataReader_return_loan(QueryConditionDataDataReader, msgList, infoSeq);
         checkStatus(g_status, "QueryConditionData_StockDataReaderView_return_loan");
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
   DDS__free(msgList);
   DDS__free(infoSeq);

   // Print out an empty line, just to let behind a clean new line for the shell..
   printf("\n\r");
   return 0;
}
