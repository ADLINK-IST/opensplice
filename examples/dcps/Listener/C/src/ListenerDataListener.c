/************************************************************************
 *  
 * Copyright (c) 2010
 * PrismTech Ltd.
 * All rights Reserved.
 * 
 * LOGICAL_NAME:    ListenerListener.c
 * FUNCTION:        Implementation of the DDS Listener's call-back methods.
 * MODULE:          Listener Example for the C programming language.
 * DATE             August 2010.
 ************************************************************************/

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
          if( message_infoSeq->_buffer[0].valid_data == TRUE )
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
   int i;

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


