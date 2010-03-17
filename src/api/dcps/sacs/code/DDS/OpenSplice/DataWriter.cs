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
    /// DataWriter allows the application to set the value of the sample to be published
    /// under a given Topic.
    /// A DataWriter is attached to exactly one Publisher which acts as a factory for it.
    /// A DataWriter is bound to exactly one Topic and therefore to exactly one data
    /// type. The Topic must exist prior to the DataWriter's creation.
    /// DataWriter is an abstract class. It must be specialized for each particular
    /// application data type. For a fictional application data type Foo (defined in the
    /// module SPACE) the specialized class would be SPACE.FooDataWriter.
    /// </summary>
    public class DataWriter : Entity, IDataWriter
    {
        private PublisherDataWriterListenerHelper listenerHelper;

        public DataWriter(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
        }

        internal void SetListener(PublisherDataWriterListenerHelper listenerHelper)
        {
            this.listenerHelper = listenerHelper;
        }

        /// <summary>
        /// This operation attaches a DataWriterListener to the DataWriter.
        /// </summary>
        /// <param name="listener"> The DataWriterListener instance that will be attached to the
        /// DataWriter.</param>
        /// <param name="mask">a bit mask in which each bit enables the invocation of the DataWriterListener 
        /// for a certain status.</param>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode SetListener(IDataWriterListener listener, StatusKind mask)
        {
            ReturnCode result = ReturnCode.Error;

            if (listener != null)
            {
                Gapi.gapi_publisherDataWriterListener gapiListener;
                if (listenerHelper == null)
                {
                    // Since the real DataWriter is created from the TypeSupport,
                    // the listenerHelper is not "readonly"
                    listenerHelper = new PublisherDataWriterListenerHelper();
                }
                listenerHelper.Listener = listener;
                listenerHelper.CreateListener(out gapiListener);
                lock (listener)
                {
                    using (PublisherDataWriterListenerMarshaler marshaler = new PublisherDataWriterListenerMarshaler(ref gapiListener))
                    {
                        result = Gapi.DataWriter.set_listener(
                            GapiPeer,
                            marshaler.GapiPtr,
                            mask);
                    }
                }
            }
            else
            {
                result = Gapi.DataWriter.set_listener(
                    GapiPeer,
                    IntPtr.Zero,
                    mask);
            }

            return result;
        }

        /// <summary>
        /// This operation replaces the existing set of QosPolicy settings for a DataWriter.
        /// </summary>
        /// <param name="qos">new set of QosPolicy settings for the DataWriter.</param>
        /// <returns>Return codes are: Ok,Error,Badparameter,Unsupported,
        /// AlreadyDeleted,OutOfResources,ImmutablePolicy or InconsistentPolicy. </returns>
        public ReturnCode SetQos(DataWriterQos qos)
        {
            ReturnCode result;

            using (DataWriterQosMarshaler marshaler = new DataWriterQosMarshaler())
            {
                result = marshaler.CopyIn(qos);
                if (result == ReturnCode.Ok)
                {
                    result = Gapi.DataWriter.set_qos(GapiPeer, marshaler.GapiPtr);
                }
            }

            return result;
        }

        /// <summary>
        /// This operation allows access to the existing list of QosPolicy settings for a DataWriter.
        /// </summary>
        /// <param name="qos">A reference to the destination DataWriterQos struct in which the 
        /// QosPolicy settings will be copied into.</param>
        /// <returns>Return codes are: Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode GetQos(ref DataWriterQos qos)
        {
            ReturnCode result;

            using (DataWriterQosMarshaler marshaler = new DataWriterQosMarshaler())
            {
                result = Gapi.DataWriter.get_qos(GapiPeer, marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref qos);
                }
            }

            return result;
        }

        /// <summary>
        /// This property returns the Topic which is associated with the DataWriter.
        /// </summary>
        public ITopic Topic
        {
            get
            {
                IntPtr gapiPtr = Gapi.DataWriter.get_topic(GapiPeer);

                ITopic topic = SacsSuperClass.fromUserData(gapiPtr) as ITopic;
                return topic;
            }
        }

        /// <summary>
        /// This property returns the Publisher to which the DataWriter belongs.
        /// </summary>
        public IPublisher Publisher
        {
            get
            {
                IntPtr gapiPtr = Gapi.DataWriter.get_publisher(GapiPeer);

                IPublisher publisher = SacsSuperClass.fromUserData(gapiPtr) as IPublisher;
                return publisher;
            }
        }

        /// <summary>
        /// This operation is not yet implemented It is scheduled for a future release.
        /// </summary>
        /// <param name="maxWait">the maximum duration to block for the WaitForAcknowledgments, 
        /// after which the application thread is unblocked. The special constant 
        /// Duration Infinite can be used when the maximum waiting time does not need to be bounded.</param>
        /// <returns>Return codes are: Unsupported</returns>
        public ReturnCode WaitForAcknowledgments(Duration maxWait)
        {
            return Gapi.DataWriter.wait_for_acknowledgments(GapiPeer, ref maxWait);
        }

        /// <summary>
        /// This operation obtains the LivelinessLostStatus struct of the DataWriter.
        /// </summary>
        /// <param name="status">A reference to LivelinessLostStatus where the contents of the LivelinessLostStatus  
        /// struct of the DataWriter will be copied into.</param>
        /// <returns>Return codes are: Ok,Error,AlreadyDeleted,OutOfResources.</returns>
        public ReturnCode GetLivelinessLostStatus(ref LivelinessLostStatus status)
        {
            if (status == null) status = new LivelinessLostStatus();
            return Gapi.DataWriter.get_liveliness_lost_status(GapiPeer, status);
        }

        /// <summary>
        /// This operation obtains the OfferedDeadlineMissedStatus struct of the DataWriter.
        /// </summary>
        /// <param name="status">A reference to OfferedDeadlineMissedStatus  where the contents of the   
        /// OfferedDeadlineMissedStatus struct of the DataWriter will be copied into.</param>
        /// <returns>Return codes are: Ok,Error,AlreadyDeleted,OutOfResources.</returns>
        public ReturnCode GetOfferedDeadlineMissedStatus(ref OfferedDeadlineMissedStatus status)
        {
            if (status == null) status = new OfferedDeadlineMissedStatus();
            return Gapi.DataWriter.get_offered_deadline_missed_status(GapiPeer, status);
        }

        /// <summary>
        /// This operation obtains the OfferedIncompatibleQosStatus struct of the DataWriter.
        /// </summary>
        /// <param name="status">A reference to  OfferedIncompatibleQosStatus where the contents of the   
        ///  OfferedIncompatibleQosStatus struct of the DataWriter will be copied into.</param>
        /// <returns>Return codes are: Ok,Error,AlreadyDeleted,OutOfResources.</returns>
        public ReturnCode GetOfferedIncompatibleQosStatus(ref OfferedIncompatibleQosStatus status)
        {
            if (status == null) status = new OfferedIncompatibleQosStatus();
            return Gapi.DataWriter.get_offered_incompatible_qos_status(GapiPeer, status);
        }

        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="status"></param>
        /// <returns></returns>
        public ReturnCode GetPublicationMatchedStatus(ref PublicationMatchedStatus status)
        {
            if (status == null) status = new PublicationMatchedStatus();
            return Gapi.DataWriter.get_publication_matched_status(GapiPeer, status);
        }

        /// <summary>
        /// This operation asserts the liveliness for the DataWriter.
        /// </summary>
        /// <returns>Return codes are: Ok,Error,AlreadyDeleted,OutOfResources or NotEnabled.</returns>
        public ReturnCode AssertLiveliness()
        {
            return Gapi.DataWriter.assert_liveliness(GapiPeer);
        }

        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="subscriptionHandles"></param>
        /// <returns></returns>
        public ReturnCode GetMatchedSubscriptions(ref InstanceHandle[] subscriptionHandles)
        {
            ReturnCode result;

            using (SequenceInstanceHandleMarshaler marshaler = new SequenceInstanceHandleMarshaler())
            {
                result = Gapi.DataWriter.get_matched_subscriptions(
                        GapiPeer,
                        marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref subscriptionHandles);
                }
            }

            return result;
        }

        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="subscriptionData"></param>
        /// <param name="subscriptionHandle"></param>
        /// <returns></returns>
        public ReturnCode GetMatchedSubscriptionData(
                ref SubscriptionBuiltinTopicData subscriptionData, 
                InstanceHandle subscriptionHandle)
        {
            ReturnCode result;

            using (SubscriptionBuiltinTopicDataMarshaler marshaler = new SubscriptionBuiltinTopicDataMarshaler())
            {
                result = Gapi.DataWriter.get_matched_subscription_data(
                        GapiPeer,
                        marshaler.GapiPtr,
                        subscriptionHandle);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref subscriptionData);
                }
            }

            return result;
        }

        protected virtual void CopyIn() { }

        protected virtual void Cleanup() { }

        protected virtual void CopyOut() { }
    }
}
