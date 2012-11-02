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
 * LOGICAL_NAME:    WaitSetDataPublisher.c
 * FUNCTION:        Publisher's main for the WaitSet OpenSplice programming example.
 * MODULE:          OpenSplice WaitSet example for the C programming language.
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
   WaitSetData_Msg* message_Sample;
   DDS_char* firstHello = "First Hello";
   int firstHelloLength = strlen(firstHello);
   DDS_char* helloAgain = "Hello again";
   int helloAgainLength = strlen(helloAgain);
   os_time os_delay300ms = { 0, 300000000 };

   printf("\n Starting WaitSetDataPublisher...");

   // Create DDS DomainParticipant
   printf("\n create Participant...");
   createParticipant("WaitSet example");

   // Register the Topic's type in the DDS Domain.
   g_MessageTypeSupport = WaitSetData_MsgTypeSupport__alloc();
   checkHandle(g_MessageTypeSupport, "WaitSetData_MsgTypeSupport__alloc");
   registerMessageType(g_MessageTypeSupport);
   // Create the Topic's in the DDS Domain.
   g_MessageTypeName = (char*) WaitSetData_MsgTypeSupport_get_type_name(g_MessageTypeSupport);
   g_MessageTopic = createTopic("WaitSetData_Msg", g_MessageTypeName);
   DDS_free(g_MessageTypeName);
   DDS_free(g_MessageTypeSupport);

   // Create the Publisher's in the DDS Domain.
   message_Publisher = createPublisher();

   // Request a Writer from the the Publisher.
   message_DataWriter = createDataWriter(message_Publisher, g_MessageTopic);

   message_Sample = WaitSetData_Msg__alloc();

   userHandle = WaitSetData_MsgDataWriter_register_instance(message_DataWriter, message_Sample);

   message_Sample->userID = 1;
   message_Sample->message = DDS_string_alloc(firstHelloLength);
   strcpy(message_Sample->message, firstHello);

   printf("\n=== [WaitSetDataPublisher] writing a message containing :");
   printf("\n    userID  : %d", message_Sample->userID);
   printf("\n    Message : \"%s\"", message_Sample->message);

   g_status = WaitSetData_MsgDataWriter_write(message_DataWriter, message_Sample, 0);
   checkStatus(g_status, "WaitSetData_MsgDataWriter_write");
   DDS_free(message_Sample->message);

   // Let the time for the new StatusCondition to be handled by the Subscriber.
   os_nanoSleep(os_delay300ms);

   message_Sample->message = DDS_string_alloc(helloAgainLength);
   strcpy(message_Sample->message, helloAgain);

   printf("\n=== [WaitSetDataPublisher] writing a message containing :");
   printf("\n    userID  : %d", message_Sample->userID);
   printf("\n    Message : \"%s\"", message_Sample->message);

   g_status = WaitSetData_MsgDataWriter_write(message_DataWriter, message_Sample, 0);
   checkStatus(g_status, "WaitSetData_MsgDataWriter_write");
   // This time, no need to free message_Sample->message as it will be freed by DDS_free(message_Sample);

   // Let the time for the new StatusCondition to be handled by the Subscriber.
   os_nanoSleep(os_delay300ms);

   WaitSetData_MsgDataWriter_unregister_instance (message_DataWriter, message_Sample, userHandle);

   printf("\n=== [WaitSetDataPublisher] Exiting.\n\n");

   // Cleanup DDS
   deleteDataWriter(message_Publisher, message_DataWriter);

   // Let the time for the new StatusCondition to be handled by the Subscriber.
   os_nanoSleep(os_delay300ms);

   deletePublisher(message_Publisher);
   deleteTopic(g_MessageTopic);
   deleteParticipant();

   // Cleanup C allocations
   // Recursively free the instances sequence using the OpenSplice API.
   DDS_free(message_Sample);
   return 0;
}
