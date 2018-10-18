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

#include "ListenerDataListener.h"
#include "CheckStatus.h"
#include "dds_dcps.h"
#include "ListenerDataSacDcps.h"


// --------------------------------------------------------------------------------------------------------
//                                              ListenerListener                                     --
// --------------------------------------------------------------------------------------------------------
/* Implementation for the call-back function "on_data_available". */
void on_data_available (
    void *Listener_data,
    DDS_DataReader reader )
{
    struct Listener_data        *pListener_data = (struct Listener_data*) Listener_data;
    DDS_ReturnCode_t status;
    DDS_sequence_ListenerData_Msg* message_seq = DDS_sequence_ListenerData_Msg__alloc();
    DDS_SampleInfoSeq* message_infoSeq = DDS_SampleInfoSeq__alloc();
    unsigned long i;
    //printf( "\n=== [ListenerListener::on_data_available] : triggered" );

    status = ListenerData_MsgDataReader_read(*pListener_data->message_DataReader,
             message_seq,
             message_infoSeq,
             DDS_LENGTH_UNLIMITED,
             DDS_ANY_SAMPLE_STATE,
             DDS_NEW_VIEW_STATE,
             DDS_ANY_INSTANCE_STATE);
    checkStatus(status, "ListenerData_MsgDataReader_read");

    if ( message_seq->_length > 0 )
    {
       printf( "\n=== [ListenerListener::on_data_available] - message_seq->length : %d", message_seq->_length );

       i = 0;
       do
       {
          if( message_infoSeq->_buffer[i].valid_data == TRUE )
          {
             printf( "\n    --- message received ---" );
             printf( "\n    userID  : %d", message_seq->_buffer[i].userID );
             printf( "\n    Message : \"%s\"", message_seq->_buffer[i].message );
          }
       }
       while ( ++i < message_seq->_length );
    }

    status = ListenerData_MsgDataReader_return_loan(*pListener_data->message_DataReader, message_seq, message_infoSeq);
    checkStatus(status, "ListenerData_MsgDataReader_return_loan");

    // unblock the waitset in Subscriber main loop
    status = DDS_GuardCondition_set_trigger_value(*pListener_data->guardCondition, TRUE);
    checkStatus(status, "DDS_GuardCondition_set_trigger_value");
}

void on_requested_deadline_missed (void *Listener_data,
   DDS_DataReader reader,
   const DDS_RequestedDeadlineMissedStatus *status)
{
   DDS_ReturnCode_t returnCode;
   struct Listener_data        *pListener_data = (struct Listener_data*) Listener_data;

   //printf( "\n=== [ListenerListener::on_requested_deadline_missed] : triggered" );

   if(*pListener_data->isClosed == FALSE)
   {
      printf( "\n=== [ListenerListener::on_requested_deadline_missed] : triggering the end of the ListenerDataSubscriber process." );
      *pListener_data->isClosed = TRUE;

      // unblock the waitset in Subscriber main loop
      returnCode = DDS_GuardCondition_set_trigger_value(*pListener_data->guardCondition, TRUE);
      checkStatus(returnCode, "DDS_GuardCondition_set_trigger_value");
   }
}


