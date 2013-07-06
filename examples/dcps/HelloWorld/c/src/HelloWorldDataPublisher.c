/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
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

#include "example_main.h"

/* entry point exported so symbol can be found in shared library */
OS_API_EXPORT
int HelloWorldDataPublisher(int argc, const char *argv[])
{
   DDS_DomainParticipant domainParticipant;
   DDS_Publisher message_Publisher;
   DDS_DataWriter message_DataWriter;
   DDS_Topic messageTopic;
   DDS_TypeSupport messageTypeSupport;
   HelloWorldData_Msg* message_Sample;
   DDS_char HelloWorld [] = "Hello World";
   os_time delay_1s = { 1, 0 };
   char* messageTypeName;
   DDS_unsigned_long HelloWorldLength = sizeof(HelloWorld) - 1;

   printf("=== HelloWorldPublisher");

   // Create DDS DomainParticipant
   // Removed to comply with expected results
   //printf("\n create Participant...");
   domainParticipant = createParticipant("HelloWorld example");

   // Register the Topic's type in the DDS Domain.
   messageTypeSupport = HelloWorldData_MsgTypeSupport__alloc();
   checkHandle(messageTypeSupport, "HelloWorldData_MsgTypeSupport__alloc");
   registerMessageType(domainParticipant, messageTypeSupport);
   // Create the Topic's in the DDS Domain.
   messageTypeName = HelloWorldData_MsgTypeSupport_get_type_name(messageTypeSupport);
   messageTopic = createTopic(domainParticipant, "HelloWorldData_Msg", messageTypeName);
   DDS_free(messageTypeName);
   DDS_free(messageTypeSupport);

   // Create the Publisher's in the DDS Domain.
   message_Publisher = createPublisher(domainParticipant);

   // Request a Writer from the the Publisher.
   message_DataWriter = createDataWriter(message_Publisher, messageTopic);

   message_Sample = HelloWorldData_Msg__alloc();

   message_Sample->userID = 1;
   message_Sample->message = DDS_string_alloc(HelloWorldLength);
   strncpy(message_Sample->message, HelloWorld, HelloWorldLength);

   printf("\n=== [Publisher] writing a message containing :");
   printf("\n    userID  : %d", message_Sample->userID);
   printf("\n    Message : \"%s\"\n", message_Sample->message);
   fflush(stdout);

   g_status = HelloWorldData_MsgDataWriter_write(message_DataWriter, message_Sample, DDS_HANDLE_NIL);
   checkStatus(g_status, "HelloWorldData_MsgDataWriter_write");
   os_nanoSleep(delay_1s);
   // Removed to comply with expected results
   //printf("\n=== [HelloWorldDataPublisher] Exiting.\n\n");

   // Cleanup DDS
   deleteDataWriter(message_Publisher, message_DataWriter);
   deletePublisher(domainParticipant, message_Publisher);
   deleteTopic(domainParticipant, messageTopic);
   deleteParticipant(domainParticipant);

   // Cleanup C allocations
   // Recursively free the instances sequence using the OpenSplice API.
   DDS_free(message_Sample);

   return 0;
}

int OSPL_MAIN (int argc, const char *argv[])
{
   return HelloWorldDataPublisher (argc, argv);
}
