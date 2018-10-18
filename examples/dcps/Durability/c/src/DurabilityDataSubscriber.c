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
 * LOGICAL_NAME:    DurabilityDataSubscriber.cpp
 * FUNCTION:        Subscriber's main for the Durability OpenSplice programming example.
 * MODULE:          OpenSplice Durability example for the C programming language.
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
#include "DurabilityData.h"

#include "example_main.h"

void usage()
{
   printf("\n*** ERROR");
   printf("\n*** usage : \"DurabilityDataSubscriber <durability_kind>\"");
   printf("\n***    durability_kind = transient | persistent");
   exit(-1);
}

int OSPL_MAIN (int argc, char *argv[])
{
   c_bool isTransient, isPersistent;
   c_bool isClosed = FALSE;
   DDS_Duration_t waitTime = {40, 0};
   unsigned long i, j;
   os_time os_delay2000 = { 2, 0 };

   DDS_sequence_DurabilityData_Msg* DurabilityData_Msg_Seq = DDS_sequence_DurabilityData_Msg__alloc();
   DDS_SampleInfoSeq* DurabilityData_infoSeq = DDS_SampleInfoSeq__alloc();

   if( argc < 2 )
   {
      usage();
   }

   isTransient = (strcmp(argv[1], "transient") == 0) ? TRUE : FALSE;
   isPersistent = (strcmp(argv[1], "persistent") == 0) ? TRUE : FALSE;
   if( ! (isTransient || isPersistent) )
   {
      usage();
   }

   g_durability_kind = argv[1];

   // Create DDS DomainParticipant
   createParticipant("Durability example");

   // Register the Topic's type in the DDS Domain.
   g_MsgTypeSupport = DurabilityData_MsgTypeSupport__alloc();
   checkHandle(g_MsgTypeSupport, "DurabilityData_MsgTypeSupport__alloc");
   registerType(g_MsgTypeSupport);
   // Create the Topic's in the DDS Domain.
   g_MsgTypeName = DurabilityData_MsgTypeSupport_get_type_name(g_MsgTypeSupport);
   if (isPersistent) {
      createTopic("PersistentCDurabilityData_Msg", g_MsgTypeName);
   } else {
      createTopic("CDurabilityData_Msg", g_MsgTypeName);
   }
   DDS_free(g_MsgTypeName);
   DDS_free(g_MsgTypeSupport);

   // Create the Subscriber's in the DDS Domain.
   createSubscriber();

   // Request a Reader from the the Subscriber.
   createReader();

   // Wait 40 seconds for historical data
   g_status = DurabilityData_MsgDataReader_wait_for_historical_data(g_DataReader, &waitTime);
   checkStatus(g_status, "DurabilityData_MsgDataReader_wait_for_historical_data");

   printf("=== [Subscriber] Ready ...");

   // Added a max iteration threshold in order to avoid looping infinitely.
   // This is in the case of "persistent" + auto_dispose == TRUE,
   // if the user tries to use persistence feature, it won't succeed:
   // with this setting value, the Instance is still deleted upon the delete of the Writer,
   // even though the persistent setting has been passed on both processes.
   i = 0;
   do
   {
      g_status = DurabilityData_MsgDataReader_take(
          g_DataReader,
          DurabilityData_Msg_Seq,
          DurabilityData_infoSeq,
          DDS_LENGTH_UNLIMITED,
          DDS_ANY_SAMPLE_STATE,
          DDS_ANY_VIEW_STATE,
          DDS_ANY_INSTANCE_STATE );

      checkStatus(g_status, "DurabilityData_MsgDataReader_take");

      if( DurabilityData_Msg_Seq->_length > 0 )
      {
         j = 0;
         do
         {
            if( DurabilityData_infoSeq->_buffer[j].valid_data )
            {
               printf("\n%s", DurabilityData_Msg_Seq->_buffer[j].content);
               if( strcmp(DurabilityData_Msg_Seq->_buffer[j].content, "9") == 0 )
               {
                  isClosed = TRUE;
               }
            }
         }
         while( ++j < DurabilityData_Msg_Seq->_length );

         DurabilityData_MsgDataReader_return_loan (g_DataReader, DurabilityData_Msg_Seq, DurabilityData_infoSeq);
      }
      os_nanoSleep(os_delay2000);
   }
   while( isClosed == FALSE && i++ < 1000);

   // For single process mode wait some time to ensure persistent data is stored to disk
   os_nanoSleep(os_delay2000);

   // Cleanup DDS from the created Entities.
   deleteDataReader();
   deleteSubscriber();
   deleteTopic();
   deleteParticipant();

   // Cleanup C allocations, recursively freeing the allocated structures and sequences using the OpenSplice API.
   DDS_free(DurabilityData_Msg_Seq);
   DDS_free(DurabilityData_infoSeq);

   return 0;
}
