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

main(int argc, const char *argv[])
{
   int x, i;
   DDS_InstanceHandle_t userHandle;
   DDS_Publisher message_Publisher;
   DDS_DataWriter message_DataWriter;
   ListenerData_Msg* message_Sample;
   DDS_char* listener = "Hello World";
   int listenerLength;

   printf("\n Starting ListenerPublisher...");

   // Create DDS DomainParticipant
   printf("\n create Participant...");
   createParticipant("Listener example");

   // Register the Topic's type in the DDS Domain.
   g_MessageTypeSupport = ListenerData_MsgTypeSupport__alloc();
   checkHandle(g_MessageTypeSupport, "ListenerData_MsgTypeSupport__alloc");
   registerMessageType(g_MessageTypeSupport);
   // Create the Topic's in the DDS Domain.
   g_MessageTypeName = (char*) ListenerData_MsgTypeSupport_get_type_name(g_MessageTypeSupport);
   g_MessageTopic = createTopic("ListenerData_Msg", g_MessageTypeName);
   DDS_free(g_MessageTypeName);
   DDS_free(g_MessageTypeSupport);

   // Create the Publisher's in the DDS Domain.
   message_Publisher = createPublisher();

   // Request a Writer from the the Publisher.
   message_DataWriter = createDataWriter(message_Publisher, g_MessageTopic);

   message_Sample = ListenerData_Msg__alloc();

   userHandle = ListenerData_MsgDataWriter_register_instance(message_DataWriter, message_Sample);

   message_Sample->userID = 1;
   listenerLength = strlen(listener);
   message_Sample->message = DDS_string_alloc(listenerLength);
   strcpy(message_Sample->message, listener);

   printf("\n=== [ListenerPublisher] writing a message containing :");
   printf("\n    userID  : %d", message_Sample->userID);
   printf("\n    Message : \"%s\"", message_Sample->message);

   g_status = ListenerData_MsgDataWriter_write(message_DataWriter, message_Sample, 0);
   checkStatus(g_status, "ListenerData_MsgDataWriter_write");

   ListenerData_MsgDataWriter_unregister_instance (message_DataWriter, message_Sample, userHandle);

   // Cleanup DDS

   deleteDataWriter(message_Publisher, message_DataWriter);
   deletePublisher(message_Publisher);
   deleteTopic(g_MessageTopic);
   deleteParticipant();

   // Cleanup C allocations
   // Recursively free the instances sequence using the OpenSplice API.
   DDS_free(message_Sample);

   printf("\n=== [ListenerPublisher] Exiting.\n\n");
   return 0;
}
