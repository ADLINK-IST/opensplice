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
 * LOGICAL_NAME:    DurabilityDataPublisher.c
 * FUNCTION:        Publisher's main for the Durability OpenSplice programming example.
 * MODULE:          OpenSplice Durability example for the C programming language.
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
#include "DurabilityData.h"

#include "example_main.h"

void usage()
{
   printf("\n*** ERROR");
   printf("\n*** usage : \"DurabilityDataPublisher <durability_kind> <autodispose_flag>\"");
   printf("\n***    durability_kind = transient | persistent");
   printf("\n***    autodispose_flag = false | true");
   printf("\n***    automatic_flag = false | true");
   exit(-1);
}

int OSPL_MAIN (int argc, const char *argv[])
{
   int x;
   DDS_InstanceHandle_t userHandles[10];
   DDS_sequence_DurabilityData_Msg* instances = DDS_sequence_DurabilityData_Msg__alloc();
   DDS_boolean isTransient;
   DDS_boolean isPersistent;
   DDS_boolean isAutodisposeTrue;
   DDS_boolean isAutodisposeFalse;
   DDS_boolean isAutomated;
   os_time delay = { 30, 0 };

   if( argc < 4 )
   {
      usage();
   }

   isTransient = (strcmp(argv[1], "transient") == 0) ? TRUE : FALSE;
   isPersistent = (strcmp(argv[1], "persistent") == 0) ? TRUE : FALSE;
   isAutodisposeTrue = (strcmp(argv[2], "true") == 0) ? TRUE : FALSE;
   isAutodisposeFalse = (strcmp(argv[2], "false") == 0) ? TRUE : FALSE;
   if( !((isTransient || isPersistent) && (isAutodisposeTrue || isAutodisposeFalse)) )
   {
      usage();
   }
   isAutomated = (strcmp(argv[3], "true") == 0) ? TRUE : FALSE;

   g_durability_kind = argv[1];

   g_autodispose_unregistered_instances = isAutodisposeTrue ? (DDS_boolean) TRUE : (DDS_boolean) FALSE;

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

   // Create the Publisher's in the DDS Domain.
   createPublisher();

   // Request a Writer from the the Publisher.
   createWriter();

   instances->_release = TRUE;
   instances->_buffer = DDS_sequence_DurabilityData_Msg_allocbuf(10);

   for( x = 0; x < 10 ; x++ )
   {
      instances->_buffer[x].id = x;

      instances->_buffer[x].content = DDS_string_alloc(1);

      snprintf(instances->_buffer[x].content, 2, "%d", x);

      userHandles[x] = DurabilityData_MsgDataWriter_register_instance(g_DataWriter, &instances->_buffer[x]);

      printf("\n%s", instances->_buffer[x].content);
      g_status = DurabilityData_MsgDataWriter_write(g_DataWriter, &instances->_buffer[x], userHandles[x]);
      checkStatus(g_status, "DurabilityData_MsgDataWriter_write");
   }

   if( isAutomated == FALSE )
   {
      int c = 0;
      printf("\n Type 'E' + Enter to exit:\n");
      do
      {
         c = getchar();
         printf("\n You Typed %c", c);
      }
      while( c != (int) 'E' );
      printf("\n Exiting.\n\n");
   }
   else
   {
      //printf( "\n === sleeping 30s..." );;
      os_nanoSleep(delay);

   }

   // Cleanup DDS from the created Entities.
   deleteDataWriter();
   deletePublisher();
   deleteTopic();
   deleteParticipant();

   // Cleanup C allocations, recursively freeing the allocated structures and sequences using the OpenSplice API.
   DDS_free(instances);

   return 0;
}
