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
    /// <summary>
    /// A DataReader allows the application:
    /// to declare data it wishes to receive (i.e., make a subscription)
    /// to access data received by the associated Subscriber. 
    /// A DataReader refers to exactly one TopicDescription (either a Topic, a ContentFilteredTopic or a MultiTopic) 
    /// that identifies the samples to be read. The DataReader may give access to several instances of the data type, 
    /// which are distinguished from each other by their key. 
    /// DataReader is an abstract class. It is specialized for each particular application data type. 
    /// For a fictional application data type “Foo” (defined in the module SPACE) 
    /// the specialized class would be SPACE.FooDataReader.
    /// </summary>
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

        /// <summary>
        /// This operation attaches a DataReaderListener to the DataReader.
        /// </summary>
        /// <param name="listener">The DataReaderListener which will be attached to the DataReader.</param>
        /// <param name="mask">A bit mask in which each bit enables the invocation of the DataReaderListener 
        /// for a certain status.</param>
        /// <returns></returns>
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
                    using (DataReaderListenerMarshaler marshaler = new DataReaderListenerMarshaler(ref gapiListener))
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

        /// <summary>
        /// This operation creates a new ReadCondition for the DataReader.
        /// </summary>
        /// <param name="sampleStates">a mask, which selects only those samples with the desired sample states.</param>
        /// <param name="viewStates">a mask, which selects only those samples with the desired view states.</param>
        /// <param name="instanceStates">a mask, which selects only those samples with the desired instance states.</param>
        /// <returns></returns>
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

        /// <summary>
        /// This operation creates a new QueryCondition for the DataReader.
        /// </summary>
        /// <param name="sampleStates">a mask, which selects only those samples with the desired sample states.</param>
        /// <param name="viewStates">a mask, which selects only those samples with the desired view states.</param>
        /// <param name="instanceStates">a mask, which selects only those samples with the desired instance states.</param>
        /// <param name="queryExpression">the query string, which must be a subset of the SQL query language.</param>
        /// <param name="queryParameters">a sequence of strings which are the parameter values used in the 
        /// SQL query string (i.e., the “%n” tokens in the expression). The number of values in queryParameters 
        /// must be equal or greater than the highest referenced %n token in the queryExpression 
        /// (e.g.if %1 and %8 are used as parameters in the queryExpression, the queryParameters 
        /// should at least contain n+1 = 9 values).</param>
        /// <returns>Returns the QueryCondition. When it fails it returns a null QueryCondition.</returns>
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

        /// <summary>
        /// This operation deletes a ReadCondition or QueryCondition which is attached to the DataReader.
        /// </summary>
        /// <param name="condition">The ReadCondition or QueryCondition which is to be deleted.</param>
        /// <returns>Return codes are: Ok,Error,BadParameter,AlreadyDeleted,OutOfResources or PreconditionNotMet</returns>
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

        /// <summary>
        /// This operation deletes all the Entity objects that were created by means of one of the 
        /// “create_” operations on the DataReader.
        /// </summary>
        /// <returns>Return codes are: Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode DeleteContainedEntities()
        {
            return Gapi.DataReader.delete_contained_entities(
                    GapiPeer,
                    null,
                    IntPtr.Zero);
        }

        /// <summary>
        /// This operation replaces the existing set of QosPolicy settings for a DataReader.
        /// </summary>
        /// <param name="qos">the new set of QosPolicy settings for the DataReader.</param>
        /// <returns>Return codes are:Ok,Error,BadParameter,Unsupported,AlreadyDeleted,OutOfResources,
        /// ImmutablePolicy or InconsistentPolicy.</returns>
        public ReturnCode SetQos(DataReaderQos qos)
        {
            ReturnCode result;

            using (DataReaderQosMarshaler marshaler = new DataReaderQosMarshaler())
            {
                result = marshaler.CopyIn(qos);
                if (result == ReturnCode.Ok)
                {
                    result = Gapi.DataReader.set_qos(GapiPeer, marshaler.GapiPtr);
                }
            }

            return result;
        }

        /// <summary>
        /// This operation allows access to the existing set of QoS policies for a DataReader.
        /// </summary>
        /// <param name="qos">a reference to DataReaderQos, where the QosPolicy settings of the DataReader
        /// are to be copied into.</param>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode GetQos(ref DataReaderQos qos)
        {
            ReturnCode result;

            using (DataReaderQosMarshaler marshaler = new DataReaderQosMarshaler())
            {
                result = Gapi.DataReader.get_qos(GapiPeer, marshaler.GapiPtr);
                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref qos);
                }
            }

            return result;
        }

        /// <summary>
        /// This operation returns the TopicDescription which is associated with the DataReader.
        /// </summary>
        /// <returns>Returns the TopicDescription associated with the DataReader.</returns>
        public ITopicDescription GetTopicDescription()
        {
            IntPtr gapiPtr = Gapi.DataReader.get_topicdescription(GapiPeer);

            ITopicDescription topicDesc = SacsSuperClass.fromUserData(gapiPtr) as ITopicDescription;
            return topicDesc;
        }

        /// <summary>
        /// This property returns the Subscriber to which the DataReader belongs.
        /// </summary>
        public ISubscriber Subscriber
        {
            get
            {
                IntPtr gapiPtr = Gapi.DataReader.get_subscriber(GapiPeer);

                ISubscriber subscriber = SacsSuperClass.fromUserData(gapiPtr) as ISubscriber;
                return subscriber;
            }
        }

        /// <summary>
        /// This operation obtains the SampleRejectedStatus of the DataReader.
        /// </summary>
        /// <param name="status">A reference to SampleRejectedStatus where the contents of the 
        /// SampleRejectedStatus of the DataReader will be copied into.</param>
        /// <returns>Return codes are: Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode GetSampleRejectedStatus(ref SampleRejectedStatus status)
        {
            if (status == null) status = new SampleRejectedStatus();
            return Gapi.DataReader.get_sample_rejected_status(GapiPeer, status);
        }

        /// <summary>
        /// This operation obtains the LivelinessChangedStatus struct of the DataReader.
        /// </summary>
        /// <param name="status">A reference to LivelinessChangedStatus where the contents of the 
        ///  LivelinessChangedStatus of the DataReader will be copied into.</param>
        /// <returns>Return codes are: Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode GetLivelinessChangedStatus(ref LivelinessChangedStatus status)
        {
            if (status == null) status = new LivelinessChangedStatus();
            return Gapi.DataReader.get_liveliness_changed_status(GapiPeer, status);
        }

        /// <summary>
        /// This operation obtains the RequestedDeadlineMissedStatus struct of the DataReader.        
        /// </summary>
        /// <param name="status">A reference to RequestedDeadlineMissedStatus where the contents of the 
        /// RequestedDeadlineMissedStatus  of the DataReader will be copied into.</param>
        /// <returns>Return codes are: Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode GetRequestedDeadlineMissedStatus(ref RequestedDeadlineMissedStatus status)
        {
            if (status == null) status = new RequestedDeadlineMissedStatus();
            return Gapi.DataReader.get_requested_deadline_missed_status(GapiPeer, status);
        }

        /// <summary>
        /// This operation obtains the RequestedIncompatibleQosStatus struct of the DataReader.
        /// </summary>
        /// <param name="status">A reference to RequestedIncompatibleQosStatus where the contents of the 
        ///  RequestedIncompatibleQosStatus of the DataReader will be copied into.</param>
        /// <returns>Return codes are: Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode GetRequestedIncompatibleQosStatus(ref RequestedIncompatibleQosStatus status)
        {
            if (status == null) status = new RequestedIncompatibleQosStatus();
            return Gapi.DataReader.get_requested_incompatible_qos_status(GapiPeer, status);
        }

        /// <summary>
        /// This operation obtains the SubscriptionMatchedStatus struct of the DataReader.
        /// </summary>
        /// <param name="status">A reference to SubscriptionMatchedStatus where the contents of the 
        ///  SubscriptionMatchedStatus of the DataReader will be copied into.</param>
        /// <returns>Return codes are: Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode GetSubscriptionMatchedStatus(ref SubscriptionMatchedStatus status)
        {
            if (status == null) status = new SubscriptionMatchedStatus();
            return Gapi.DataReader.get_subscription_matched_status(GapiPeer, status);
        }

        /// <summary>
        /// This operation obtains the SampleLostStatus struct of the DataReader.
        /// </summary>
        /// <param name="status"> reference to SampleLostStatus where the contents of the 
        ///  SampleLostStatus of the DataReader will be copied into.</param>
        /// <returns>Return codes are: Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode GetSampleLostStatus(ref SampleLostStatus status)
        {
            if (status == null) status = new SampleLostStatus();
            return Gapi.DataReader.get_sample_lost_status(GapiPeer, status);
        }

        /// <summary>
        /// This operation will block the application thread until all “historical” data is received.
        /// </summary>
        /// <param name="maxWait">the maximum duration to block for the operation, after which 
        /// the application thread is unblocked. The special constant Duration Infinite can be used when 
        /// the maximum waiting time does not need to be bounded.</param>
        /// <returns>Return codes are: Ok,Error,AlreadyDeleted,OutOfResources,NotEnabled or Timeout.</returns>
        public ReturnCode WaitForHistoricalData(Duration maxWait)
        {
            return Gapi.DataReader.wait_for_historical_data(GapiPeer, ref maxWait);
        }

        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="publicationHandles"></param>
        /// <returns></returns>
        public ReturnCode GetMatchedPublications(ref InstanceHandle[] publicationHandles)
        {
            ReturnCode result;

            using (SequenceInstanceHandleMarshaler marshaler = new SequenceInstanceHandleMarshaler())
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

        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="publicationData"></param>
        /// <param name="publicationHandle"></param>
        /// <returns></returns>
        public ReturnCode GetMatchedPublicationData(ref PublicationBuiltinTopicData publicationData, InstanceHandle publicationHandle)
        {
            ReturnCode result;

            using (PublicationBuiltinTopicDataMarshaler marshaler = new PublicationBuiltinTopicDataMarshaler())
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
