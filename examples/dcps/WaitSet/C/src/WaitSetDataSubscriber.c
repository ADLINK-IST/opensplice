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
 * LOGICAL_NAME:    WaitSetDataSubscriber.cpp
 * FUNCTION:        WaitSetDataSubscriber's main for the WaitSetData OpenSplice programming example.
 * MODULE:          OpenSplice WaitSet example for the C programming language.
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

int main(int argc, char *argv[])
{
   DDS_sequence_WaitSetData_Msg* message_seq = DDS_sequence_WaitSetData_Msg__alloc();
   DDS_SampleInfoSeq* message_infoSeq = DDS_SampleInfoSeq__alloc();
   DDS_Subscriber message_Subscriber;
   DDS_DataReader message_DataReader;
   DDS_char* query_parameter = "Hello again";
   DDS_char *query_string = "message=%0";
   DDS_ReadCondition readCondition;
   DDS_QueryCondition queryCondition;
   DDS_StatusCondition statusCondition;
   DDS_WaitSet waitSet;
   DDS_GuardCondition escape;
   DDS_LivelinessChangedStatus livelinessChangedStatus;
   DDS_ConditionSeq *guardList = NULL;
   DDS_Duration_t timeout = { 20, 0 };
   int count = 0;

   DDS_long previousLivelinessCount;
   int os_result;
   c_bool isClosed = FALSE;
   c_bool isEscaped = FALSE;
   c_bool writerLeft = FALSE;
   unsigned long i, j;

   // Force the output to be unbuffered.
   setbuf(stdout, (char *) 0);

   // Create DDS DomainParticipant
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

   // Create the Subscriber's in the DDS Domain.
   message_Subscriber = createSubscriber();

   // Request a Reader from the the Subscriber.
   message_DataReader = createDataReader(message_Subscriber, g_MessageTopic);

   // 1- Create a ReadCondition that will contain new Msg only.
   readCondition = DDS_DataReader_create_readcondition(message_DataReader, DDS_NOT_READ_SAMPLE_STATE, DDS_NEW_VIEW_STATE, DDS_ALIVE_INSTANCE_STATE);
   checkHandle(readCondition, "DDS_DataReader_create_readcondition (newUser)");

   // 2- Create QueryCondition.
   queryCondition = createQueryCondition(message_DataReader, query_string, (DDS_char*) query_parameter);
   printf("=== [WaitSetDataSubscriber] Query : message = \"%s\"", query_parameter);

   // 3- Obtain a StatusCondition associated to a Writer
   // and that triggers only when the Writer changes Liveliness.
   statusCondition = DDS_DataReader_get_statuscondition(message_DataReader);
   checkHandle(statusCondition, "DDS_DataReader_get_statuscondition");
   g_status = DDS_StatusCondition_set_enabled_statuses(statusCondition, DDS_LIVELINESS_CHANGED_STATUS);
   checkStatus(g_status, "DDS_StatusCondition_set_enabled_statuses");

   // 4- Create a GuardCondition which will be used to close the subscriber.
   escape = DDS_GuardCondition__alloc();
   checkHandle(escape, "DDS_GuardCondition__alloc");

   // Create a waitset and add the 4 Conditions created above :
   // ReadCondition, QueryCondition, StatusCondition, GuardCondition.
   waitSet = DDS_WaitSet__alloc();
   checkHandle(waitSet, "DDS_WaitSet__alloc");
   g_status = DDS_WaitSet_attach_condition(waitSet, readCondition);
   checkStatus(g_status, "DDS_WaitSet_attach_condition (readCondition)");
   g_status = DDS_WaitSet_attach_condition(waitSet, queryCondition);
   checkStatus(g_status, "DDS_WaitSet_attach_condition (queryCondition)");
   g_status = DDS_WaitSet_attach_condition(waitSet, statusCondition);
   checkStatus(g_status, "DDS_WaitSet_attach_condition (statusCondition)");
   g_status = DDS_WaitSet_attach_condition(waitSet, escape);
   checkStatus(g_status, "DDS_WaitSet_attach_condition (escape)");

   // Initialize and pre-allocate the GuardList used to obtain the triggered Conditions.
   guardList = DDS_ConditionSeq__alloc();
   checkHandle(guardList, "DDS_ConditionSeq__alloc");
   guardList->_maximum = 4;
   guardList->_length = 0;
   guardList->_buffer = DDS_ConditionSeq_allocbuf(4);
   checkHandle(guardList->_buffer, "DDS_ConditionSeq_allocbuf");

   printf("\n=== [WaitSetDataSubscriber] Ready ...");

   // Used to store the livelinessChangedStatus.alive_count value (StatusCondition).
   previousLivelinessCount = 0;
   do
   {
      // Wait until at least one of the Conditions in the waitset triggers.
      g_status = DDS_WaitSet_wait(waitSet, guardList, &timeout);
      
      if (g_status == DDS_RETCODE_OK) {
         i = 0;
         do
         {
            if( guardList->_buffer[i] == statusCondition )
            {
               // StatusCondition triggered.
               // Some liveliness has changed (either a DataWriter joined or a DataWriter left).
               // Check whether a new Writer appeared.
               g_status = DDS_DataReader_get_liveliness_changed_status(message_DataReader, &livelinessChangedStatus);
               checkStatus(g_status, "DDS_DataReader_get_liveliness_changed_status");

               if( livelinessChangedStatus.alive_count > previousLivelinessCount )
               {
                  // a DataWriter joined.
                  printf("\n\n!!! a MsgWriter joined");
                  g_status = DDS_StatusCondition_set_enabled_statuses(statusCondition, DDS_LIVELINESS_CHANGED_STATUS);
                  checkStatus(g_status, "DDS_StatusCondition_set_enabled_statuses");
               }
               else if( livelinessChangedStatus.alive_count < previousLivelinessCount )
               {
                  // A MsgWriter lost its liveliness.
                  printf("\n\n!!! a MsgWriter lost its liveliness");
                  g_status = DDS_StatusCondition_set_enabled_statuses(statusCondition, DDS_LIVELINESS_CHANGED_STATUS);
                  checkStatus(g_status, "DDS_StatusCondition_set_enabled_statuses");
                  writerLeft = TRUE;
                  printf("\n === Triggering escape condition");
 	          g_status = DDS_GuardCondition_set_trigger_value(escape, TRUE);
                  checkStatus(g_status, "DDS_GuardCondition_set_trigger_value");
               }
               else if ( livelinessChangedStatus.alive_count == previousLivelinessCount )
               {
                  // The status has changed, though alive_count is still zero,
                  // this implies that both events have occurred,
                  // and that the value is back to zero already.
                  printf("\n\n!!! a MsgWriter joined\n");
                  printf("\n\n!!! a MsgWriter lost its liveliness\n");
                  writerLeft = TRUE;
                  printf("\n === Triggering escape condition");
 	          g_status = DDS_GuardCondition_set_trigger_value(escape, TRUE);
                  checkStatus(g_status, "DDS_GuardCondition_set_trigger_value");

                  // NB: we do not need to activate this Condition anymore;
                  // both events occurred.
               }
               // NB: Here we record the count a in order to compare it next time this condition triggers (?).
               previousLivelinessCount = livelinessChangedStatus.alive_count;

            }
            else if( guardList->_buffer[i] == readCondition )
            {
               /* The newMsg ReadCondition contains data */
               g_status = WaitSetData_MsgDataReader_read_w_condition(message_DataReader, message_seq, message_infoSeq, DDS_LENGTH_UNLIMITED, readCondition);
               checkStatus(g_status, "WaitSetData_MsgDataReader_read_w_condition");

               for( j = 0; j < message_seq->_length ; j++ )
               {
                  printf("\n    --- New message received ---");
                  if( message_infoSeq->_buffer[j].valid_data == TRUE )
                  {
                     printf("\n    userID  : %d", message_seq->_buffer[j].userID);
                     printf("\n    Message : \"%s\"", message_seq->_buffer[j].message);
                  }
                  else
                  {
                     printf("\n    Data is invalid!");
                  }
               }
               g_status = WaitSetData_MsgDataReader_return_loan(message_DataReader, message_seq, message_infoSeq);
               checkStatus(g_status, "WaitSetData_MsgDataReader_return_loan");
            }
            else if( guardList->_buffer[i] == queryCondition )
            {
               /* The queryCond QueryCondition contains data  */
               g_status = WaitSetData_MsgDataReader_take_w_condition(message_DataReader, message_seq, message_infoSeq, DDS_LENGTH_UNLIMITED, queryCondition);
               checkStatus(g_status, "WaitSetData_MsgDataReader_take_w_condition");

               for( j = 0; j < message_seq->_length ; j++ )
               {
                  printf("\n\n    --- message received (with QueryCOndition on message field) ---");
                  if( message_infoSeq->_buffer[j].valid_data == TRUE )
                  {
                     printf("\n    userID  : %d", message_seq->_buffer[j].userID);
                     printf("\n    Message : \"%s\"", message_seq->_buffer[j].message);
                  }
                  else
                  {
                     printf("\n    Data is invalid!");
                  }
               }
               g_status = WaitSetData_MsgDataReader_return_loan(message_DataReader, message_seq, message_infoSeq);
               checkStatus(g_status, "WaitSetData_MsgDataReader_return_loan");
            }
            // This checks whether the Timeout Thread has resumed waiting.
            else if( guardList->_buffer[i] == escape )
            {
               printf("\n escape condition triggered! - count = %d\n ", count);
               isEscaped = TRUE;
	       g_status = DDS_GuardCondition_set_trigger_value(escape, FALSE);
               checkStatus(g_status, "DDS_GuardCondition_set_trigger_value");
            }
         }
         while( ++i < guardList->_length );
      }
      else if (g_status != DDS_RETCODE_TIMEOUT) {
        // DDS_RETCODE_TIMEOUT is considered as an error
        // only after it has occurred count times
        checkStatus(g_status, "DDS_WaitSet_wait");
      } else {
        printf("!!! [INFO] WaitSet timedout - count = %d\n ", count);
      }
 
      ++count;
      isClosed = isEscaped && writerLeft;

   }   while( isClosed == FALSE && count < 20);
   if (count >= 20)  {
      printf( "*** Error : Timed out - count = %d ***\n", count);
   }
   /* Remove all Conditions from the WaitSet. */
   g_status = DDS_WaitSet_detach_condition(waitSet, escape);
   checkStatus(g_status, "DDS_WaitSet_detach_condition (escape)");
   g_status = DDS_WaitSet_detach_condition(waitSet, statusCondition);
   checkStatus(g_status, "DDS_WaitSet_detach_condition (statusCondition)");
   g_status = DDS_WaitSet_detach_condition(waitSet, queryCondition);
   checkStatus(g_status, "DDS_WaitSet_detach_condition (queryCondition)");
   g_status = DDS_WaitSet_detach_condition(waitSet, readCondition);
   checkStatus(g_status, "DDS_WaitSet_detach_condition (readCondition)");

   // Cleanup DDS from the created Entities.
   deleteQueryCondition(message_DataReader, readCondition);
   deleteQueryCondition(message_DataReader, queryCondition);
   deleteDataReader(message_Subscriber, message_DataReader);
   deleteSubscriber(message_Subscriber);
   deleteTopic(g_MessageTopic);
   deleteParticipant();

   // Cleanup C allocations
   // Recursively free the instances sequence using the OpenSplice API.
   DDS_free(guardList);
   DDS_free(message_seq);
   DDS_free(message_infoSeq);
   DDS_free(waitSet);
   DDS_free(escape);

   printf("\n");
   return 0;
}
