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
 * LOGICAL_NAME:    ListenerDataPublisher.c
 * FUNCTION:        Publisher's main for the Listener OpenSplice programming example.
 * MODULE:          OpenSplice Listener example for the C programming language.
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

#include "example_main.h"

int OSPL_MAIN (int argc, const char *argv[])
{
   DDS_Publisher message_Publisher;
   DDS_DataWriter message_DataWriter;
   ListenerData_Msg* message_Sample;
   const DDS_char listener [] = "Hello World";
   os_time delay_2s = { 2, 0 };
   DDS_unsigned_long listenerLength = sizeof(listener) - 1;

   printf("\n Starting ListenerPublisher...");

   // Create DDS DomainParticipant
   printf("\n create Participant...");
   createParticipant("Listener example");

   // Register the Topic's type in the DDS Domain.
   g_MessageTypeSupport = ListenerData_MsgTypeSupport__alloc();
   checkHandle(g_MessageTypeSupport, "ListenerData_MsgTypeSupport__alloc");
   registerMessageType(g_MessageTypeSupport);
   // Create the Topic's in the DDS Domain.
   g_MessageTypeName = ListenerData_MsgTypeSupport_get_type_name(g_MessageTypeSupport);
   g_MessageTopic = createTopic("ListenerData_Msg", g_MessageTypeName);
   DDS_free(g_MessageTypeName);
   DDS_free(g_MessageTypeSupport);

   // Create the Publisher's in the DDS Domain.
   message_Publisher = createPublisher();

   // Request a Writer from the the Publisher.
   message_DataWriter = createDataWriter(message_Publisher, g_MessageTopic);

   message_Sample = ListenerData_Msg__alloc();
   message_Sample->userID = 1;
   message_Sample->message = DDS_string_alloc(listenerLength);
   strncpy(message_Sample->message, listener, listenerLength);

   printf("\n=== [ListenerPublisher] writing a message containing :");
   printf("\n    userID  : %d", message_Sample->userID);
   printf("\n    Message : \"%s\"", message_Sample->message);

   g_status = ListenerData_MsgDataWriter_write(message_DataWriter, message_Sample, DDS_HANDLE_NIL);
   checkStatus(g_status, "ListenerData_MsgDataWriter_write");
   os_nanoSleep(delay_2s);
   // Cleanup DDS

   deleteDataWriter(message_Publisher, message_DataWriter);
   deletePublisher(message_Publisher);
   deleteTopic(g_MessageTopic);
   deleteParticipant();

   // Cleanup C allocations
   DDS_free(message_Sample);

   printf("\n=== [ListenerPublisher] Exiting.\n\n");
   return 0;
}
