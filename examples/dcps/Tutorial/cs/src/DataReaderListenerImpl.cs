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
using System;
using DDS;
using Chat;

namespace Chatroom
{
    class DataReaderListenerImpl : IDataReaderListener
    {
        /*
         * Attributes
         */

        /* Caching variables */
        private int previous = -1;
        private String userName;
        private NamedMessage joinedSample = new NamedMessage();

        private ChatMessage[] messages;
        private NameService[] names;
        private SampleInfo[] infoSeq1;
        private SampleInfo[] infoSeq2;

        /* Type-specific DDS entities */
        private ChatMessageDataReader chatMessageDR;

        public ChatMessageDataReader ChatMessageDR
        {
            get { return chatMessageDR; }
            set { chatMessageDR = value; }
        }

        private NameServiceDataReader nameServiceDR;

        public NameServiceDataReader NameServiceDR
        {
            get { return nameServiceDR; }
            set { nameServiceDR = value; }
        }

        private NamedMessageDataWriter namedMessageDW;

        public NamedMessageDataWriter NamedMessageDW
        {
            get { return namedMessageDW; }
            set { namedMessageDW = value; }
        }


        /* Query related stuff */
        private IQueryCondition nameFinder;

        public IQueryCondition NameFinder
        {
            get { return nameFinder; }
            set { nameFinder = value; }
        }

        private String[] nameFinderParams;

        public String[] NameFinderParams
        {
            get { return nameFinderParams; }
            set { nameFinderParams = value; }
        }


        /***
         * Operations
         ***/
        public void OnRequestedDeadlineMissed(
                IDataReader the_reader,
                RequestedDeadlineMissedStatus status) { }

        public void OnRequestedIncompatibleQos(
                IDataReader the_reader,
                RequestedIncompatibleQosStatus status) { }

        public void OnSampleRejected(
            IDataReader the_reader, SampleRejectedStatus status) { }

        public void OnLivelinessChanged(
            IDataReader the_reader, LivelinessChangedStatus status) { }

        public void OnDataAvailable(IDataReader the_reader)
        {
            int count = 0;
            previous = -1;

            /* Take all messages. */
            ReturnCode status = chatMessageDR.Take(
                ref messages,
                ref infoSeq1,
                SampleStateKind.Any,
                ViewStateKind.Any,
                InstanceStateKind.Any);
            ErrorHandler.checkStatus(
                status, "Chat.ChatMessageDataReader.take");

            /* For each message, extract the key-field and find
               the corresponding name. */
            for (int i = 0; i < messages.Length; i++)
            {
                if (infoSeq1[i].ValidData)
                {
                    /* Find the corresponding named message. */
                    if (messages[i].userID != previous)
                    {
                        previous = messages[i].userID;
                        nameFinderParams[0] = previous.ToString();
                        status = nameFinder.SetQueryParameters(nameFinderParams);
                        ErrorHandler.checkStatus(
                            status, "DDS.QueryCondition.SetQueryParameters");

                        /* To ensure the identifier for the sender is presented with the message
                         * content, it may be necessary to try reading again if the NameService
                         * sample is not initially available.  This is because the arrival order
                         * of samples representing different topics cannot be guaranteeed.
                         */
                        status = ReturnCode.NoData;
                        while (status == ReturnCode.NoData && count < 10)
                        {
                            status = nameServiceDR.ReadWithCondition(
                                ref names,
                                ref infoSeq2,
                                nameFinder);
                            ErrorHandler.checkStatus(
                                status, "Chat.NameServiceDataReader.ReadWithCondition");

                            /* Extract Name (there should only be one result). */
                            if (status == ReturnCode.NoData)
                            {
                                userName = "Name not found!! id = " + previous;
                                /* Sleep for some amount of time, as not to consume too much CPU cycles. */
                                System.Threading.Thread.Sleep(100);
                                count++;
                            }
                            else
                            {
                                userName = names[0].name;
                                /* Release the name sample again. */
                                status = nameServiceDR.ReturnLoan(ref names, ref infoSeq2);
                                ErrorHandler.checkStatus(
                                    status, "Chat.NameServiceDataReader.return_loan");
                            }
                        }
                    }
                    /* Write merged Topic with userName instead of userID. */
                    joinedSample.userName = userName;
                    joinedSample.userID = messages[i].userID;
                    joinedSample.index = messages[i].index;
                    joinedSample.content = messages[i].content;
                    status = namedMessageDW.Write(joinedSample);
                    ErrorHandler.checkStatus(
                        status, "Chat.NamedMessageDataWriter.write");
                }
            }
            status = chatMessageDR.ReturnLoan(ref messages, ref infoSeq1);
            ErrorHandler.checkStatus(
                status, "Chat.ChatMessageDataReader.return_loan");

        }

        public void OnSubscriptionMatched(
            IDataReader the_reader, SubscriptionMatchedStatus status) { }

        public void OnSampleLost(
            IDataReader the_reader, SampleLostStatus status) { }

    }
}
