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
 * LOGICAL_NAME:    HelloWorldPublisher.c
 * FUNCTION:        Publisher's main for the HelloWorld OpenSplice programming example.
 * MODULE:          OpenSplice HelloWorld example for the C programming language.
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
   HelloWorldData_Msg* message_Sample;
   DDS_char* HelloWorld = (DDS_char*) "Hello World";
   int HelloWorldLength;

   printf("=== HelloWorldPublisher");

   // Create DDS DomainParticipant
   // Removed to comply with expected results
   //printf("\n create Participant...");
   createParticipant("HelloWorld example");

   // Register the Topic's type in the DDS Domain.
   g_MessageTypeSupport = HelloWorldData_MsgTypeSupport__alloc();
   checkHandle(g_MessageTypeSupport, (char*) "HelloWorldData_MsgTypeSupport__alloc");
   registerMessageType(g_MessageTypeSupport);
   // Create the Topic's in the DDS Domain.
   g_MessageTypeName = (char*) HelloWorldData_MsgTypeSupport_get_type_name(g_MessageTypeSupport);
   g_MessageTopic = createTopic("HelloWorldData_Msg", g_MessageTypeName);
   DDS_free(g_MessageTypeName);
   DDS_free(g_MessageTypeSupport);

   // Create the Publisher's in the DDS Domain.
   message_Publisher = createPublisher();

   // Request a Writer from the the Publisher.
   message_DataWriter = createDataWriter(message_Publisher, g_MessageTopic);

   message_Sample = HelloWorldData_Msg__alloc();

   message_Sample->userID = 1;
   HelloWorldLength = strlen(HelloWorld);
   message_Sample->message = DDS_string_alloc(HelloWorldLength);
   strcpy(message_Sample->message, HelloWorld);

   printf("\n=== [Publisher] writing a message containing :");
   printf("\n    userID  : %d", message_Sample->userID);
   printf("\n    Message : \"%s\"\n", message_Sample->message);

   g_status = HelloWorldData_MsgDataWriter_write(message_DataWriter, message_Sample, 0);
   checkStatus(g_status, "HelloWorldData_MsgDataWriter_write");

   // Removed to comply with expected results
   //printf("\n=== [HelloWorldDataPublisher] Exiting.\n\n");

   // Cleanup DDS
   deleteDataWriter(message_Publisher, message_DataWriter);
   deletePublisher(message_Publisher);
   deleteTopic(g_MessageTopic);
   deleteParticipant();

   // Cleanup C allocations
   // Recursively free the instances sequence using the OpenSplice API.
   DDS_free(message_Sample);

   return 0;
}
