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
 * LOGICAL_NAME:    DataReaderListenerImpl.java
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the Java programming language.
 * DATE             june 2007.
 ************************************************************************
 * 
 * This file contains the implementation for a DataReader listener, that 
 * simulates MultiTopic behavior by writing a NamedMessage sample (which 
 * contains the merged information from both the ChatMessage and NameService
 * topics) for each incoming ChatMessage.
 * 
 ***/

package chatroom;

import DDS.*;
import Chat.*;

public class DataReaderListenerImpl implements DataReaderListener {
    
    /***
     * Attributes
     ***/     
    /* Caching variables */
    private int                   previous      = 0x80000000;
    private String                userName;
    private ChatMessageSeqHolder  msgSeq        = new ChatMessageSeqHolder();
    private NameServiceSeqHolder  nameSeq       = new NameServiceSeqHolder();
    private SampleInfoSeqHolder   infoSeq1      = new SampleInfoSeqHolder();
    private SampleInfoSeqHolder   infoSeq2      = new SampleInfoSeqHolder();
    private NamedMessage          joinedSample  = new NamedMessage();
    
        
    /* Type-specific DDS entities */
    public ChatMessageDataReader        chatMessageDR;
    public NameServiceDataReader        nameServiceDR;
    public NamedMessageDataWriter       namedMessageDW;

    /* Query related stuff */
    public QueryCondition               nameFinder;
    public String[]                     nameFinderParams;

    /***
     * Operations
     ***/
    public void on_requested_deadline_missed(
            DataReader the_reader, 
            RequestedDeadlineMissedStatus status) { }

    public void on_requested_incompatible_qos(
            DataReader the_reader,
            RequestedIncompatibleQosStatus status) { }

    public void on_sample_rejected(
        DataReader the_reader, SampleRejectedStatus status) { }

    public void on_liveliness_changed(
        DataReader the_reader, LivelinessChangedStatus status) { }

    public void on_data_available(DataReader the_reader) {
        previous = 0x80000000;
        
        /* Take all messages. */
        int status = chatMessageDR.take( 
            msgSeq, 
            infoSeq1, 
            LENGTH_UNLIMITED.value, 
            ANY_SAMPLE_STATE.value, 
            ANY_VIEW_STATE.value, 
            ANY_INSTANCE_STATE.value);
        ErrorHandler.checkStatus(
            status, "Chat.ChatMessageDataReader.take");
        
        /* For each message, extract the key-field and find 
           the corresponding name. */
        for (int i = 0; i < msgSeq.value.length; i++)
        {
            if (infoSeq1.value[i].valid_data)
            {
                /* Find the corresponding named message. */
                if (msgSeq.value[i].userID != previous)
                {
                    previous = msgSeq.value[i].userID;
                    nameFinderParams[0] = Integer.toString(previous);
                    status = nameFinder.set_query_parameters(nameFinderParams);
                    ErrorHandler.checkStatus(
                        status, "DDS.QueryCondition.set_query_parameters");
                    status = nameServiceDR.read_w_condition( 
                        nameSeq, 
                        infoSeq2, 
                        LENGTH_UNLIMITED.value, 
                        nameFinder);
                    ErrorHandler.checkStatus(
                        status, "Chat.NameServiceDataReader.read_w_condition");
                    
                    /* Extract Name (there should only be one result). */
                    if (status == RETCODE_NO_DATA.value)
                    {
                        userName = new String(
                                "Name not found!! id = " + previous);
                    }
                    else
                    {
                        userName = nameSeq.value[0].name;
                    }
    
                    /* Release the name sample again. */                
                    status = nameServiceDR.return_loan(nameSeq, infoSeq2);
                    ErrorHandler.checkStatus(
                        status, "Chat.NameServiceDataReader.return_loan");
                }
                /* Write merged Topic with userName instead of userID. */
                joinedSample.userName = userName;
                joinedSample.userID = msgSeq.value[i].userID;
                joinedSample.index = msgSeq.value[i].index;
                joinedSample.content = msgSeq.value[i].content;
                status = namedMessageDW.write(joinedSample, HANDLE_NIL.value);
                ErrorHandler.checkStatus(
                    status, "Chat.NamedMessageDataWriter.write");
            }
        }
        status = chatMessageDR.return_loan(msgSeq, infoSeq1);
        ErrorHandler.checkStatus(
            status, "Chat.ChatMessageDataReader.return_loan");

    }

    public void on_subscription_matched(
        DataReader the_reader, SubscriptionMatchedStatus status) { }

    public void on_sample_lost(
        DataReader the_reader, SampleLostStatus status) { }

}
