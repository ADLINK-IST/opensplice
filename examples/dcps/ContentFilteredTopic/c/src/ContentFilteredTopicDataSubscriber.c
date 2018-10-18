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
 * LOGICAL_NAME:    ContentFilteredTopicDataSubscriber.c
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          OpenSplice ContentFilteredTopic example for the C programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation for the 'ContentFilteredTopicDataQuerySubscriber' executable.
 *
 ***/
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
   fprintf(stderr, "\n *** usage: \n \t ContentFilteredTopicDataSubscriber <topic's content filtering string>");

   exit(-1);
}

int OSPL_MAIN (int argc, char *argv[])
{
   char* partition_name = "ContentFilteredTopic example";
   DDS_Subscriber contentFilteredTopicDataSubscriber;
   DDS_DataReader contentFilteredTopicDataDataReader;
   StockMarket_Stock* contentFilteredTopicDataSample;
   DDS_sequence_StockMarket_Stock* msgList = DDS_sequence_StockMarket_Stock__alloc();
   DDS_SampleInfoSeq* infoSeq = DDS_SampleInfoSeq__alloc();
   c_bool isClosed = FALSE;
   unsigned long j;
   const char *filterValueToSubscribe;
   DDS_char* contentFilteredStockTopicName = "MyStockTopic";
   DDS_ContentFilteredTopic contentFilteredTopic;
   DDS_unsigned_long filter_expressionLength;
   const char filter_expressionPrefix [] = "ticker = '";
   const char filter_expressionSuffix [] = "'";
   DDS_char *filter_expression;
   const DDS_StringSeq* filter_parameters = DDS_StringSeq__alloc();
   os_time delay_200ms = { 0, 200000000 };
   int count = 0;

   // usage : ContentFilteredTopicDataSubscriber <topic's content filtering string>
   if( argc < 2 )
   {
      usage();
   }
   printf("=== ContentFilteredTopicDataSubscriber");
   // Removed to comply with expected results
   //printf("\n\n Parameter is \"%s\"", argv[1]);
   filterValueToSubscribe = argv[1];

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

   // Create the Subscriber's in the DDS Domain.
   contentFilteredTopicDataSubscriber = createSubscriber();

   // create subscription filter

   filter_expressionLength = sizeof(filter_expressionPrefix) - 1;
   filter_expressionLength += (DDS_unsigned_long) strlen(filterValueToSubscribe);
   filter_expressionLength += sizeof(filter_expressionSuffix) - 1;
   filter_expression = DDS_string_alloc(filter_expressionLength);
   snprintf(filter_expression, filter_expressionLength + 1, "%s%s%s", filter_expressionPrefix, filterValueToSubscribe, filter_expressionSuffix);

   // create ContentFilteredTopic to retrieve only the samples with the given Stock.
   contentFilteredTopic = createContentFilteredTopic(contentFilteredStockTopicName, g_StockTopic, filter_expression, filter_parameters);
   // create Filtered DataReader

   printf( "\n=== [ContentFilteredTopicDataSubscriber] Subscription filter : %s", filter_expression );

   // Request a Reader from the the Subscriber.
   contentFilteredTopicDataDataReader = createContentFilteredDataReader(contentFilteredTopicDataSubscriber, contentFilteredTopic);

   printf( "\n=== [ContentFilteredTopicDataSubscriber] Ready ..." );

   do
   {
      g_status = StockMarket_StockDataReader_take(contentFilteredTopicDataDataReader, msgList, infoSeq, DDS_LENGTH_UNLIMITED, DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
      checkStatus(g_status, "StockMarket_StockDataReaderView_take");
      if( msgList->_length > 0 )
      {
         j = 0;
         do
         {
            contentFilteredTopicDataSample = &msgList->_buffer[j];
            if( infoSeq->_buffer[j].valid_data )
            {
               if( contentFilteredTopicDataSample->price != (DDS_float) -1.0f )
               {
                 int floatWidth = ( ((DDS_float) contentFilteredTopicDataSample->price) - ((long) contentFilteredTopicDataSample->price) ) ? 1 : 0;
                   printf("\n=== [ContentFilteredTopicDataSubscriber] receives stockQuote :  (%s, %.*f)", contentFilteredTopicDataSample->ticker, floatWidth, contentFilteredTopicDataSample->price);
               }
               else
               {
                  isClosed = TRUE;
               }
            }
         }
         while( ++j < msgList->_length );

         g_status = StockMarket_StockDataReader_return_loan(contentFilteredTopicDataDataReader, msgList, infoSeq);
         checkStatus(g_status, "StockMarket_StockDataReader_return_loan");

         if(isClosed == FALSE)
         {
            os_nanoSleep(delay_200ms);
            ++count;
         }
      }
   }
   while( isClosed == FALSE && count < 1500); // We dont want the example to run indefinitely

   printf("\n=== [ContentFilteredTopicDataSubscriber] Market Closed\n");

   // Cleanup DDS from the created Entities.
   deleteDataReader(contentFilteredTopicDataSubscriber, contentFilteredTopicDataDataReader);
   deleteSubscriber(contentFilteredTopicDataSubscriber);
   deleteContentFilteredTopic(contentFilteredTopic);
   deleteTopic(g_StockTopic);
   deleteParticipant();

   // Cleanup C allocations,
   // recursively freeing the allocated structures and sequences using the OpenSplice API.
   DDS_free(filter_parameters);
   DDS_free(filter_expression);
   DDS_free(msgList);
   DDS_free(infoSeq);

   // Print out an empty line, just to let behind a clean new line for the shell..
   // Removed to comply with expected results
   //printf("\n\r");
   return 0;
}
