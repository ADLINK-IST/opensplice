// The OpenSplice DDS Community Edition project.
//
// Copyright (C) 2006 to 2009 PrismTech Limited and its licensees.
// Copyright (C) 2009  L-3 Communications / IS
// 
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License Version 3 dated 29 June 2007, as published by the
//  Free Software Foundation.
// 
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
// 
//  You should have received a copy of the GNU Lesser General Public
//  License along with OpenSplice DDS Community Edition; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

using System;
using DDS;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
{
    public class DataReader : Entity, IDataReader
    {
        private DataReaderListenerHelper listenerHelper;

        public DataReader(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
        }

        internal void SetListener(DataReaderListenerHelper listenerHelper)
        {
            this.listenerHelper = listenerHelper;
        }

        public ReturnCode SetListener(IDataReaderListener listener, StatusKind mask)
        {
            ReturnCode result = ReturnCode.Error;

            Gapi.gapi_dataReaderListener gapiListener;
            if (listenerHelper == null)
            {
                // Since the real DataReader is created from the TypeSupport,
                // the listenerHelper is not "readonly"
                listenerHelper = new DataReaderListenerHelper();
            }
            listenerHelper.Listener = listener;
            listenerHelper.CreateListener(out gapiListener);

            using (DataReaderListenerMarshaler marshaler = new DataReaderListenerMarshaler(ref gapiListener))
            {
                result = Gapi.DataReader.set_listener(
                    GapiPeer,
                    marshaler.GapiPtr,
                    mask);
            }

            return result;
        }

        public IReadCondition CreateReadCondition()
        {
            return CreateReadCondition(SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public IReadCondition CreateReadCondition(SampleStateKind sampleStates, ViewStateKind viewStates,
            InstanceStateKind instanceStates)
        {
            IReadCondition readCondition = null;

            IntPtr gapiPtr = Gapi.DataReader.create_readcondition(
                GapiPeer,
                sampleStates,
                viewStates,
                instanceStates);

            if (gapiPtr != IntPtr.Zero)
            {
                readCondition = new ReadCondition(gapiPtr);
            }

            return readCondition;
        }

        public IQueryCondition CreateQueryCondition(string queryExpression, params string[] queryParameters)
        {
            return CreateQueryCondition(SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any,
                queryExpression, queryParameters);
        }

        public IQueryCondition CreateQueryCondition(SampleStateKind sampleStates, ViewStateKind viewStates,
            InstanceStateKind instanceStates, string queryExpression, params string[] queryParameters)
        {
            IQueryCondition queryCondition = null;

            using (IMarshaler marshaler = new SequenceStringMarshaler(queryParameters))
            {
                IntPtr gapiPtr = Gapi.DataReader.create_querycondition(
                    GapiPeer,
                    sampleStates,
                    viewStates,
                    instanceStates,
                    queryExpression,
                    marshaler.GapiPtr);

                if (gapiPtr != IntPtr.Zero)
                {
                    queryCondition = new QueryCondition(gapiPtr);
                }
            }

            return queryCondition;
        }

        public ReturnCode DeleteReadCondition(IReadCondition condition)
        {
            ReadCondition conditionObj = (ReadCondition)condition;
            return Gapi.DataReader.delete_readcondition(
                GapiPeer,
                conditionObj.GapiPeer);
        }

        public ReturnCode DeleteContainedEntities()
        {
            return Gapi.DataReader.delete_contained_entities(
                GapiPeer,
                null,
                IntPtr.Zero);
        }

        public ReturnCode SetQos(ref DataReaderQos qos)
        {
            using (IMarshaler marshaler = new DataReaderQosMarshaler(ref qos))
            {
                return Gapi.DataReader.set_qos(
                GapiPeer,
                marshaler.GapiPtr);
            }
        }

        public ReturnCode GetQos(out DataReaderQos qos)
        {
            qos = new DataReaderQos();
            ReturnCode result = ReturnCode.Error;

            using (DataReaderQosMarshaler marshaler = new DataReaderQosMarshaler())
            {
                result = Gapi.DataReader.get_qos(
                GapiPeer,
                marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(out qos);
                }
            }

            return result;
        }

        public ITopicDescription GetTopicDescription()
        {
            IntPtr gapiPtr = Gapi.DataReader.get_topicdescription(GapiPeer);

            ITopicDescription topicDesc = SacsSuperClass.fromUserData(gapiPtr) as ITopicDescription;
            return topicDesc;
        }

        public ISubscriber Subscriber
        {
            get
            {
                IntPtr gapiPtr = Gapi.DataReader.get_subscriber(GapiPeer);

                ISubscriber subscriber = SacsSuperClass.fromUserData(gapiPtr) as ISubscriber;
                return subscriber;
            }
        }

        public ReturnCode GetSampleRejectedStatus(SampleRejectedStatus status)
        {
            return Gapi.DataReader.get_sample_rejected_status(
                GapiPeer,
                status);
        }

        public ReturnCode GetLivelinessChangedStatus(LivelinessChangedStatus status)
        {
            return Gapi.DataReader.get_liveliness_changed_status(
                 GapiPeer,
                 status);
        }

        public ReturnCode GetRequestedDeadlineMissedStatus(RequestedDeadlineMissedStatus status)
        {
            return Gapi.DataReader.get_requested_deadline_missed_status(
                 GapiPeer,
                 status);
        }

        public ReturnCode GetRequestedIncompatibleQosStatus(RequestedIncompatibleQosStatus status)
        {
            return Gapi.DataReader.get_requested_incompatible_qos_status(
                 GapiPeer,
                 status);
        }

        public ReturnCode GetSubscriptionMatchedStatus(SubscriptionMatchedStatus status)
        {
            return Gapi.DataReader.get_subscription_matched_status(
                 GapiPeer,
                 status);
        }

        public ReturnCode GetSampleLostStatus(SampleLostStatus status)
        {
            return Gapi.DataReader.get_sample_lost_status(
                 GapiPeer,
                 status);
        }

        public ReturnCode WaitForHistoricalData(Duration maxWait)
        {
            return Gapi.DataReader.wait_for_historical_data(
                GapiPeer,
                ref maxWait);
        }

        public ReturnCode GetMatchedPublications(out InstanceHandle[] publicationHandles)
        {
            publicationHandles = null;
            ReturnCode result = ReturnCode.Error;

            using (SequenceInstanceHandleMarshaler marshaler = new SequenceInstanceHandleMarshaler())
            {
                result = Gapi.DataReader.get_matched_publications(
                    GapiPeer,
                    marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(out publicationHandles);
                }
            }

            return result;
        }

        public ReturnCode GetMatchedPublicationData(out PublicationBuiltinTopicData publicationData, InstanceHandle publicationHandle)
        {
            publicationData = new PublicationBuiltinTopicData();
            ReturnCode result = ReturnCode.Error;

            using (PublicationBuiltinTopicDataMarshaler marshaler = new PublicationBuiltinTopicDataMarshaler())
            {
                result = Gapi.DataReader.get_matched_publication_data(
                    GapiPeer,
                    marshaler.GapiPtr,
                    publicationHandle);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(out publicationData);
                }
            }

            return result;
        }
    }
}
