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
 * LOGICAL_NAME:    LifecycleSubscriber.cpp
 * FUNCTION:        Subscriber's main for the Lifecycle OpenSplice programming example.
 * MODULE:          OpenSplice Lifecycle example for the C programming language.
 * DATE             September 2010.
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <string.h>
#include "os_time.h"
#include "dds_dcps.h"
#include "CheckStatus.h"
#include "DDSEntitiesManager.h"
#include "LifecycleData.h"

char* sSampleState[] =
{ "READ_SAMPLE_STATE", "NOT_READ_SAMPLE_STATE" };
char* sViewState[] =
{ "NEW_VIEW_STATE", "NOT_NEW_VIEW_STATE" };
char* sInstanceState[] =
{ "ALIVE_INSTANCE_STATE", "NOT_ALIVE_DISPOSED_INSTANCE_STATE", "NOT_ALIVE_NO_WRITERS_INSTANCE_STATE" };

int getIndex(int i)
{
   int j = (log10((float) i) / log10((float) 2));
   return j;
}

int main(int argc, char *argv[])
{

   DDS_sequence_LifecycleData_Msg* msgSeq = DDS_sequence_LifecycleData_Msg__alloc();
   DDS_SampleInfoSeq* msgInfoSeq = DDS_SampleInfoSeq__alloc();
   DDS_Subscriber msgSubscriber;
   DDS_DataReader msgDataReader;

   unsigned long j, inputChar;

   DDS_boolean closed = FALSE;
   int nbIter = 1;
   int nbIterMax = 100;

   os_time os_delay20ms = {0, 20000000};
   os_time os_delay200ms = {0, 200000000};

   // Force the output to be unbuffered.
   setbuf(stdout, (char *) 0);

   // First initialize the Topics and their DDS Entities

   // Create DDS DomainParticipant
   createParticipant("Lifecycle example");

   //------------------ Msg topic --------------------//
   // Register Msg Topic's type in the DDS Domain.
   g_msgTypeSupport = LifecycleData_MsgTypeSupport__alloc();
   checkHandle(g_msgTypeSupport, "LifecycleData_MsgTypeSupport__alloc");
   registerMsgType(g_msgTypeSupport);
   // Create Msg Topic in the DDS Domain.
   g_msgTypeName = (char*) LifecycleData_MsgTypeSupport_get_type_name(g_msgTypeSupport);
   g_msgTopic = createTopic("Lifecycle_Msg", g_msgTypeName);
   DDS_free(g_msgTypeName);
   DDS_free(g_msgTypeSupport);

   // Create the Publisher's in the DDS Domain.
   msgSubscriber = createSubscriber();

   // Request a DataReader from the the Subscriber.
   msgDataReader = createDataReader(msgSubscriber, g_msgTopic);
   //End initialization.

   printf("\n=== [Subscriber] Ready...");

   msgInfoSeq->_length = 0;
   while ((closed == FALSE) && (nbIter < nbIterMax))
   {
      g_status = LifecycleData_MsgDataReader_read(msgDataReader, msgSeq, msgInfoSeq, 1, DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
      checkStatus(g_status, "LifecycleData_MsgDataReader_read");

      if( msgInfoSeq->_length > 0 )
      {
         if( msgInfoSeq->_buffer[0].valid_data == TRUE )
         {
            printf("\n\n Message        : %s", msgSeq->_buffer[0].message);
            printf("\n WriterStates : %s", msgSeq->_buffer[0].writerStates);
            printf("\n valid_data     : %d", msgInfoSeq->_buffer[0].valid_data);
            printf("\n sample_state:%s-view_state:%s-instance_state:%s\n", 
	   		       sSampleState[getIndex(msgInfoSeq->_buffer[0].sample_state)],
			       sViewState[getIndex(msgInfoSeq->_buffer[0].view_state)],
			       sInstanceState[getIndex(msgInfoSeq->_buffer[0].instance_state)]);   
         }
         closed = strcmp(msgSeq->_buffer[0].writerStates, "STOPPING_SUBSCRIBER") == 0 ? TRUE : FALSE;
         printf("=== closed=%d - nbIter %d\n", closed, nbIter);
         g_status = LifecycleData_MsgDataReader_return_loan(msgDataReader, msgSeq, msgInfoSeq);
         checkStatus(g_status, "LifecycleData_MsgDataReader_return_loan");
         // Useless to harass the system, so we wait before to retry.
         os_nanoSleep(os_delay200ms);
         msgInfoSeq->_length = 0;
         nbIter++;
      }
   }
   printf("\n=== [Subscriber] stopping after %d iterations ..\n", nbIter);
   if (nbIter == nbIterMax) printf ("*** Error : max %d iterations reached", nbIterMax);
   // Cleanup DDS from the created Entities.
   deleteDataReader(msgSubscriber, msgDataReader);
   deleteSubscriber(msgSubscriber);
   deleteTopic(g_msgTopic);
   deleteParticipant();

   // Cleanup C allocations, recursively freeing the allocated structures and sequences using the OpenSplice API.
   DDS__free(msgSeq);
   DDS__free(msgInfoSeq);

   return 0;
}
