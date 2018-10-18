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

/**
 * @file
 */

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

typedef struct {
   DDS_long nodeId;
   DDS_string hostName;
   DDS_long participantCount;

} NodeInfo;

void updateNode ( DDS_ParticipantBuiltinTopicData* newNode,
         NodeInfo* nodeInfoArray,
         unsigned long nodeInfoIndex)
{
   if(newNode->user_data.value._length > 0 && *newNode->user_data.value._buffer != '<' )
   {
        nodeInfoArray[ nodeInfoIndex ].hostName = DDS_string_alloc( newNode->user_data.value._length );
        memcpy( nodeInfoArray[ nodeInfoIndex ].hostName, newNode->user_data.value._buffer, newNode->user_data.value._length );
        nodeInfoArray[ nodeInfoIndex ].hostName[ newNode->user_data.value._length ] = '\0';
   }
   nodeInfoArray[ nodeInfoIndex ].nodeId = newNode->key[ 0 ];
}

NodeInfo* getNodeInfo ( DDS_ParticipantBuiltinTopicData* node,
         NodeInfo** nodeInfoArray,
         unsigned long* nodeInfoArraySize )
{
   unsigned long nodeIndex;

   for( nodeIndex = 0 ; nodeIndex < *nodeInfoArraySize ; ++nodeIndex )
   {
      if( (*nodeInfoArray)[ nodeIndex ].nodeId == node->key[ 0 ] )
      {
          updateNode(node, *nodeInfoArray, nodeIndex);
          break;
      }
   }
   if( nodeIndex == *nodeInfoArraySize )
   {
       NodeInfo* newNodeInfoArray;
       int newByteSize = ++(*nodeInfoArraySize) * sizeof(NodeInfo);
       newNodeInfoArray = (NodeInfo *)realloc(*nodeInfoArray, newByteSize);
       *nodeInfoArray = newNodeInfoArray;
       (*nodeInfoArray)[ nodeIndex ].hostName = NULL;
       (*nodeInfoArray)[ nodeIndex ].participantCount = 0;
       updateNode(node, *nodeInfoArray, nodeIndex);
   }
   return &(*nodeInfoArray)[ nodeIndex ];
}

void freeNodeInfoArray ( NodeInfo** nodeInfoArray,
         unsigned long* nodeInfoArraySize )
{
   unsigned long nodeIndex;

   for( nodeIndex = 0 ; nodeIndex < *nodeInfoArraySize ; ++nodeIndex )
   {
      if( (*nodeInfoArray)[ nodeIndex ].hostName != NULL )
      {
         DDS_free((*nodeInfoArray)[ nodeIndex ].hostName);
      }
   }

   free(*nodeInfoArray);
   *nodeInfoArray = NULL;
   *nodeInfoArraySize = 0;
}

int OSPL_MAIN (int argc, char *argv[])
{
   DDS_Subscriber builtInTopicsSubscriber;
   DDS_DataReader builtInTopicsReader;
   DDS_sequence_DDS_ParticipantBuiltinTopicData* builtInTopicsDataSeq = DDS_sequence_DDS_ParticipantBuiltinTopicData__alloc();
   DDS_SampleInfoSeq* builtInTopicsInfoSeq = DDS_SampleInfoSeq__alloc();
   DDS_ReadCondition readCondition;
   DDS_WaitSet waitSet;
   DDS_Duration_t waitForSamplesToTakeTimeout = DDS_DURATION_INFINITE;
   DDS_Duration_t waitForHistoricalDataTimeout =  { 10, 0 };
   DDS_ConditionSeq *guardList = NULL;

   DDS_boolean isAutomatic = TRUE;
   DDS_boolean isStopping = FALSE;

   // Resizable array to store the nodes' description with the count of participants for each.
   NodeInfo* nodeInfoArray = NULL;
   NodeInfo* nodeInfo;
   unsigned long nodeInfoArraySize = 0;
   unsigned int  j;

   if( argc > 1 )
   {
      isAutomatic = (strcmp(argv[1], "true") == 0);
   }

   // create domain participant
   createParticipant("BuiltInTopics example");

   // Resolve the built-in Subscriber.
   builtInTopicsSubscriber = DDS_DomainParticipant_get_builtin_subscriber(g_domainParticipant);
   checkHandle(builtInTopicsSubscriber, "DDS_DomainParticipant_get_builtin_subscriber(g_domainParticipant)");

   // Lookup the DataReader for the DCPSParticipant built-in Topic.
   builtInTopicsReader = DDS_Subscriber_lookup_datareader(builtInTopicsSubscriber, "DCPSParticipant");
   checkHandle(builtInTopicsReader, "DDS_Subscriber_lookup_datareader(builtInTopicsSubscriber, DCPSParticipant)");

   printf("=== [BuiltInTopicsDataSubscriber] : Waiting for historical data ... ");
   fflush(stdout);

   // Make sure all historical data is delivered in the DataReader.
   g_status = DDS_DataReader_wait_for_historical_data(builtInTopicsReader, &waitForHistoricalDataTimeout);
   if(g_status == DDS_RETCODE_TIMEOUT)
   {
       printf("\n=== [BuiltInTopicsDataSubscriber] : WARNING: No historical data (durability probably not running) ... \n");
   }
   checkStatus(g_status, "DDS_DataReader_wait_for_historical_data (builtInTopicsReader, 1 minute)");

   printf("\n=== [BuiltInTopicsDataSubscriber] : done");

   // Create a new ReadCondition for the reader that matches all samples.
   readCondition = DDS_DataReader_create_readcondition(builtInTopicsReader, DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
   checkHandle(readCondition, "DDS_DataReader_create_readcondition (newUser)");

   // Create a waitset and add the ReadCondition created above.
   waitSet = DDS_WaitSet__alloc();
   checkHandle(waitSet, "DDS_WaitSet__alloc");
   g_status = DDS_WaitSet_attach_condition(waitSet, readCondition);
   checkStatus(g_status, "DDS_WaitSet_attach_condition (readCondition)");

   // Initialize and pre-allocate the GuardList used to obtain the triggered Conditions.
   guardList = DDS_ConditionSeq__alloc();
   checkHandle(guardList, "DDS_ConditionSeq__alloc");
   // Initialize and pre-allocate the GuardList's seq.
   guardList->_maximum = 1;
   guardList->_length = 0;
   guardList->_release = TRUE;
   guardList->_buffer = DDS_ConditionSeq_allocbuf(1);
   checkHandle(guardList->_buffer, "DDS_ConditionSeq_allocbuf");

   printf("\n=== [BuiltInTopicsDataSubscriber] Ready ...");

   // Continue processing until interrupted.
   do
   {
      // Wait
      // Block the current thread until the attached condition becomes true,
      // or the user interrupts.
      printf( "\n=== [BuiltInTopicsDataSubscriber] Waiting ... " );
      g_status = DDS_WaitSet_wait(waitSet, guardList, &waitForSamplesToTakeTimeout);
      checkStatus(g_status, "DDS_WaitSet_wait");

      // Take all available data from the reader.
      g_status = DDS_ParticipantBuiltinTopicDataDataReader_take(
               builtInTopicsReader,
               builtInTopicsDataSeq,
               builtInTopicsInfoSeq,
               DDS_LENGTH_UNLIMITED,
               DDS_ANY_SAMPLE_STATE,
               DDS_ANY_VIEW_STATE,
               DDS_ANY_INSTANCE_STATE);
      checkStatus(g_status, "DDS_ParticipantBuiltinTopicDataDataReader_take");

      isStopping = ( isAutomatic == TRUE || g_status != DDS_RETCODE_OK ) ? TRUE : FALSE;

      // Verify that data has been taken.
      if(g_status == DDS_RETCODE_OK)
      {
         // Iterate the list of taken samples.
         if( builtInTopicsDataSeq->_length > 0 )
         {
            j = 0;
            do
            {
               if( builtInTopicsInfoSeq->_buffer[ j ].valid_data == TRUE )
               {
                   // The splicedaemon publishes the host-name in the user_data field.
                  // Retrieve the informations for the Node.
                  // Check if we saw a participant for the node before.
                  nodeInfo = getNodeInfo( &builtInTopicsDataSeq->_buffer[ j ], &nodeInfoArray, &nodeInfoArraySize );

                  // Check sample info to see whether the instance is ALIVE.
                  if(builtInTopicsInfoSeq->_buffer[ j ].instance_state == DDS_ALIVE_INSTANCE_STATE)
                  {
                    // Increase the number of participants.
                     nodeInfo->participantCount++;


                     // If it's the first participant, report the node is up.
                     if( nodeInfo->participantCount == 1 )
                     {
                        printf ( "\n=== [BuiltInTopicsDataSubscriber] Node '%d' started (Total nodes running: %lu)", nodeInfo->nodeId, nodeInfoArraySize);
                     }
                     if( builtInTopicsDataSeq->_buffer[ j ].user_data.value._length > 0 && *builtInTopicsDataSeq->_buffer[ j ].user_data.value._buffer != '<')
                     {
                         printf( "\n=== [BuiltInTopicsDataSubscriber] Hostname for node '%d' is '%s'.", nodeInfo->nodeId, nodeInfo->hostName );
                     }
                  }
                  else
                  {
                     // Decrease the number of participants.
                     nodeInfo->participantCount--;

                     // If no more participants exist, report the node is down.
                     if (nodeInfo->participantCount == 0)
                     {

                        printf( "\n=== [BuiltInTopicsDataSubscriber] Node %d (%s) stopped (Total nodes running: %lu)", nodeInfo->nodeId, nodeInfo->hostName, nodeInfoArraySize );
                     }
                  }
               }
            }
            while( ++j < builtInTopicsDataSeq->_length );

            // Indicate to reader that data/info is no longer accessed.*/
            g_status = DDS_ParticipantBuiltinTopicDataDataReader_return_loan(
                     builtInTopicsReader,
                     builtInTopicsDataSeq,
                     builtInTopicsInfoSeq );
            checkStatus(g_status, "DDS_ParticipantBuiltinTopicDataDataReader_return_loan");
         }

      }
   }
   while( isStopping == FALSE && g_status == DDS_RETCODE_OK );
   printf( "\n");

   // Delete the read condition
   g_status = DDS_DataReader_delete_readcondition(builtInTopicsReader, readCondition);
   checkStatus(g_status, "DDS_ParticipantBuiltinTopicDataDataReader_return_loan");

   // Cleanup DDS from the created Entities.
   deleteContainedEntities();
   deleteParticipant();

   DDS_free(guardList);
   DDS_free(waitSet);
   DDS_free(builtInTopicsDataSeq);
   DDS_free(builtInTopicsInfoSeq);

   freeNodeInfoArray(&nodeInfoArray, &nodeInfoArraySize);

   return 0;
}


