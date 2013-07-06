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
 * LOGICAL_NAME:    HelloWorldDataSubscriber.cpp
 * FUNCTION:        HelloWorldDataSubscriber's main for the HelloWorld OpenSplice programming example.
 * MODULE:          OpenSplice HelloWorld example for the C programming language.
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

#include "example_main.h"

/* entry point exported so symbol can be found in shared library */
OS_API_EXPORT
int HelloWorldDataSubscriber(int argc, const char *argv[])
{
   DDS_DomainParticipant domainParticipant;
   DDS_sequence_HelloWorldData_Msg* message_seq = DDS_sequence_HelloWorldData_Msg__alloc();
   DDS_SampleInfoSeq* message_infoSeq = DDS_SampleInfoSeq__alloc();
   DDS_Subscriber message_Subscriber;
   DDS_DataReader message_DataReader;
   DDS_Topic messageTopic;
   DDS_TypeSupport messageTypeSupport;
   char * messageTypeName;
   c_bool isClosed = FALSE;
   int count = 0;
   os_time os_delay200 = { 0, 200000000 };

   // Create DDS DomainParticipant
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

   // Create the Subscriber's in the DDS Domain.
   message_Subscriber = createSubscriber(domainParticipant);

   // Request a Reader from the the Subscriber.
   message_DataReader = createDataReader(message_Subscriber, messageTopic);

   printf("=== [Subscriber] Ready ...");

   do
   {
      g_status = HelloWorldData_MsgDataReader_take(
          message_DataReader,
          message_seq,
          message_infoSeq,
          1,
          DDS_ANY_SAMPLE_STATE,
          DDS_ANY_VIEW_STATE,
          DDS_ANY_INSTANCE_STATE );
      checkStatus(g_status, "HelloWorldData_MsgDataReader_take");

      if( message_seq->_length > 0 && message_infoSeq->_buffer[0].valid_data )
      {
         isClosed = TRUE;
         printf("\n=== [Subscriber] message received :" );
         printf( "\n    userID  : %d", message_seq->_buffer[0].userID );
         printf( "\n    Message : \"%s\"\n", message_seq->_buffer[0].message );
         fflush(stdout);
         HelloWorldData_MsgDataReader_return_loan (message_DataReader, message_seq, message_infoSeq);
      }

      if(isClosed == FALSE)
      {
         os_nanoSleep(os_delay200);
         ++count;
      }
   }
   while( isClosed == FALSE && count < 1500 );

   os_nanoSleep(os_delay200);

   // Cleanup DDS from the created Entities.
   deleteDataReader(message_Subscriber, message_DataReader);
   deleteSubscriber(domainParticipant, message_Subscriber);
   deleteTopic(domainParticipant, messageTopic);
   deleteParticipant(domainParticipant);

   // Cleanup C allocations
   // Recursively free the instances sequence using the OpenSplice API.
   DDS_free(message_seq);
   DDS_free(message_infoSeq);

   return 0;
}

int OSPL_MAIN (int argc, const char *argv[])
{
   return HelloWorldDataSubscriber (argc, argv);
}
