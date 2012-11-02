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
 * LOGICAL_NAME:    ListenerDataSubscriber.cpp
 * FUNCTION:        ListenerDataSubscriber's main for the Listener OpenSplice programming example.
 * MODULE:          OpenSplice Listener example for the C programming language.
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
#include "ListenerDataListener.h"

c_bool isClosed;
DDS_GuardCondition guardCondition;
DDS_DataReader message_DataReader;

int main(int argc, char *argv[])
{
   DDS_sequence_ListenerData_Msg* message_seq = DDS_sequence_ListenerData_Msg__alloc();
   DDS_SampleInfoSeq* message_infoSeq = DDS_SampleInfoSeq__alloc();
   DDS_Subscriber message_Subscriber;
   DDS_DataReader message_DataReader;
   DDS_WaitSet waitSet;
   DDS_ConditionSeq* guardList = NULL;
   struct DDS_DataReaderListener *message_Listener;
   DDS_Duration_t timeout = { 0, 200000000 };
   struct Listener_data* Listener_data;
   DDS_GuardCondition guardCondition = DDS_GuardCondition__alloc();
   int count = 0;
   DDS_StatusMask mask;

   // Create DDS DomainParticipant
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

   // Create the Subscriber's in the DDS Domain.
   message_Subscriber = createSubscriber("Listener");

   // Request a Reader from the the Subscriber.
   message_DataReader = createDataReader(message_Subscriber, g_MessageTopic);

   /* Allocate the DataReaderListener interface. */
   message_Listener = DDS_DataReaderListener__alloc();
   checkHandle(message_Listener, "DDS_DataReaderListener__alloc");

   Listener_data = malloc(sizeof(struct Listener_data));
   checkHandle(Listener_data, "malloc");
   Listener_data->guardCondition = &guardCondition;
   Listener_data->message_DataReader = &message_DataReader;
   Listener_data->isClosed = &isClosed;
   message_Listener->listener_data = Listener_data;
   message_Listener->on_data_available = on_data_available;
   message_Listener->on_requested_deadline_missed = on_requested_deadline_missed;

   mask =
            DDS_DATA_AVAILABLE_STATUS | DDS_REQUESTED_DEADLINE_MISSED_STATUS;
   g_status = DDS_DataReader_set_listener(message_DataReader, message_Listener, mask);
   checkStatus(g_status, "DDS_DataReader_set_listener");

   // WaitSet is used to avoid spinning in the loop below.
   waitSet = DDS_WaitSet__alloc();
   checkHandle(waitSet, "DDS_WaitSet__alloc");
   g_status = DDS_WaitSet_attach_condition(waitSet, guardCondition);
   checkStatus(g_status, "DDS_WaitSet_attach_condition (readCondition)");

   // Initialize and pre-allocate the GuardList used to obtain the triggered Conditions.
   guardList = DDS_ConditionSeq__alloc();
   checkHandle(guardList, "DDS_ConditionSeq__alloc");
   guardList->_maximum = 1;
   guardList->_length = 0;
   guardList->_buffer = DDS_ConditionSeq_allocbuf(1);
   checkHandle(guardList->_buffer, "DDS_ConditionSeq_allocbuf");

   printf("\n=== [ListenerDataSubscriber] Ready...");

   isClosed = FALSE;
   do
   {
      g_status = DDS_WaitSet_wait(waitSet, guardList, &timeout);
      if(g_status != DDS_RETCODE_TIMEOUT)
      {
          checkStatus(g_status, "DDS_WaitSet_wait");
      }
      else
      {
          printf("\n=== [ListenerDataSubscriber] (timeout)");
      }
      g_status = DDS_GuardCondition_set_trigger_value(guardCondition, FALSE);
      checkStatus(g_status, "DDS_GuardCondition_set_trigger_value");
      ++ count;
   }
   while( isClosed == FALSE && count < 1500 );

   printf("\n\n=== [ListenerDataSubscriber] isClosed");
   // Cleanup DDS from the created Entities.
   deleteDataReader(message_Subscriber, message_DataReader);
   deleteSubscriber(message_Subscriber);
   deleteTopic(g_MessageTopic);
   deleteParticipant();

   // Cleanup C allocations
   // Recursively free the instances sequence using the OpenSplice API.
   DDS_free(message_seq);
   DDS_free(message_infoSeq);
   free(Listener_data);

   // Print out an empty line, just to avoid the shell writing on the same line as our last output.
   printf("\n");
   return 0;
}
