// The OpenSplice DDS Community Edition project.
//
// Copyright (C) 2006 to 2011 PrismTech Limited and its licensees.
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

            if (listener != null)
            {
                Gapi.gapi_dataReaderListener gapiListener;
                if (listenerHelper == null)
                {
                    // Since the real DataReader is created from the TypeSupport,
                    // the listenerHelper is not "readonly"
                    listenerHelper = new DataReaderListenerHelper();
                }
                listenerHelper.Listener = listener;
                listenerHelper.CreateListener(out gapiListener);
                lock (listener)
                {
                    using (DataReaderListenerMarshaler marshaler = 
                            new DataReaderListenerMarshaler(ref gapiListener))
                    {
                        result = Gapi.DataReader.set_listener(
                                GapiPeer,
                                marshaler.GapiPtr,
                                mask);
                    }
                }
            }
            else
            {
                result = Gapi.DataReader.set_listener(
                        GapiPeer,
                        IntPtr.Zero,
                        mask);
            }

            return result;
        }

        
        public IReadCondition CreateReadCondition()
        {
            return CreateReadCondition(SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }
        
        public IReadCondition CreateReadCondition(
                SampleStateKind sampleStates, 
                ViewStateKind viewStates,
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

        public IQueryCondition CreateQueryCondition(
                string queryExpression, 
                params string[] queryParameters)
        {
            return CreateQueryCondition(
                    SampleStateKind.Any, 
                    ViewStateKind.Any, 
                    InstanceStateKind.Any,
                    queryExpression, queryParameters);
        }
        
        public IQueryCondition CreateQueryCondition(
                SampleStateKind sampleStates, 
                ViewStateKind viewStates,
                InstanceStateKind instanceStates, 
                string queryExpression, 
                params string[] queryParameters)
        {
            IQueryCondition queryCondition = null;

            using (SequenceStringMarshaler marshaler = new SequenceStringMarshaler())
            {
                if (marshaler.CopyIn(queryParameters) == DDS.ReturnCode.Ok)
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
            }

            return queryCondition;
        }
        
        public ReturnCode DeleteReadCondition(IReadCondition condition)
        {
            ReturnCode result = ReturnCode.BadParameter;

            ReadCondition conditionObj = condition as ReadCondition;
            if (conditionObj != null)
            {
                result = Gapi.DataReader.delete_readcondition(
                        GapiPeer,
                        conditionObj.GapiPeer);
            }

            return result;
        }
        
        public ReturnCode DeleteContainedEntities()
        {
            return Gapi.DataReader.delete_contained_entities(GapiPeer);
        }
        
        public ReturnCode SetQos(DataReaderQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.DataReaderQosMarshaler marshaler = 
                    new OpenSplice.CustomMarshalers.DataReaderQosMarshaler())
            {
                result = marshaler.CopyIn(qos);
                if (result == ReturnCode.Ok)
                {
                    result = Gapi.DataReader.set_qos(GapiPeer, marshaler.GapiPtr);
                }
            }

            return result;
        }
        
        public ReturnCode GetQos(ref DataReaderQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.DataReaderQosMarshaler marshaler = 
                    new OpenSplice.CustomMarshalers.DataReaderQosMarshaler())
            {
                result = Gapi.DataReader.get_qos(GapiPeer, marshaler.GapiPtr);
                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref qos);
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
        
        public ReturnCode GetSampleRejectedStatus(
                ref SampleRejectedStatus status)
        {
            if (status == null) status = new SampleRejectedStatus();
            return Gapi.DataReader.get_sample_rejected_status(GapiPeer, status);
        }
        
        public ReturnCode GetLivelinessChangedStatus(
                ref LivelinessChangedStatus status)
        {
            if (status == null) status = new LivelinessChangedStatus();
            return Gapi.DataReader.get_liveliness_changed_status(GapiPeer, status);
        }
        
        public ReturnCode GetRequestedDeadlineMissedStatus(
                ref RequestedDeadlineMissedStatus status)
        {
            if (status == null) status = new RequestedDeadlineMissedStatus();
            return Gapi.DataReader.get_requested_deadline_missed_status(GapiPeer, status);
        }
        
        public ReturnCode GetRequestedIncompatibleQosStatus(
                ref RequestedIncompatibleQosStatus status)
        {
            ReturnCode result;

            using (RequestedIncompatibleQosStatusMarshaler marshaler = 
                    new RequestedIncompatibleQosStatusMarshaler())
            {
                if (status == null) status = new RequestedIncompatibleQosStatus();
                if (status.Policies == null) status.Policies = new QosPolicyCount[28];
                marshaler.CopyIn(status);
                
                result = Gapi.DataReader.get_requested_incompatible_qos_status(
                        GapiPeer, marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref status);
                }
            }

            return result;
        }
        
        public ReturnCode GetSubscriptionMatchedStatus(
                ref SubscriptionMatchedStatus status)
        {
            if (status == null) status = new SubscriptionMatchedStatus();
            return Gapi.DataReader.get_subscription_matched_status(GapiPeer, status);
        }
        
        public ReturnCode GetSampleLostStatus(ref SampleLostStatus status)
        {
            if (status == null) status = new SampleLostStatus();
            return Gapi.DataReader.get_sample_lost_status(GapiPeer, status);
        }
        
        public ReturnCode WaitForHistoricalData(Duration maxWait)
        {
            return Gapi.DataReader.wait_for_historical_data(GapiPeer, ref maxWait);
        }
        
        public ReturnCode GetMatchedPublications(ref InstanceHandle[] publicationHandles)
        {
            ReturnCode result;

            using (SequenceInstanceHandleMarshaler marshaler = 
                    new SequenceInstanceHandleMarshaler())
            {
                result = Gapi.DataReader.get_matched_publications(
                        GapiPeer,
                        marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref publicationHandles);
                }
            }

            return result;
        }

        public ReturnCode GetMatchedPublicationData(
                ref PublicationBuiltinTopicData publicationData, 
                InstanceHandle publicationHandle)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.PublicationBuiltinTopicDataMarshaler marshaler = 
                    new OpenSplice.CustomMarshalers.PublicationBuiltinTopicDataMarshaler())
            {
                result = Gapi.DataReader.get_matched_publication_data(
                        GapiPeer,
                        marshaler.GapiPtr,
                        publicationHandle);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref publicationData);
                }
            }

            return result;
        }
    }
}
