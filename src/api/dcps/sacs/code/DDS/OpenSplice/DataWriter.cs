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

        public ReturnCode SetListener(IDataWriterListener listener, StatusKind mask)
        {
            ReturnCode result = ReturnCode.Error;

            Gapi.gapi_publisherDataWriterListener gapiListener;
            if (listenerHelper == null)
            {
                // Since the real DataWriter is created from the TypeSupport,
                // the listenerHelper is not "readonly"
                listenerHelper = new PublisherDataWriterListenerHelper();
            }
            listenerHelper.Listener = listener;
            listenerHelper.CreateListener(out gapiListener);

            using (PublisherDataWriterListenerMarshaler marshaler = new PublisherDataWriterListenerMarshaler(ref gapiListener))
            {
                result = Gapi.DataWriter.set_listener(
                    GapiPeer,
                    marshaler.GapiPtr,
                    mask);
            }

            return result;
        }

        public ReturnCode SetQos(ref DataWriterQos qos)
        {
            using (IMarshaler marshaler = new DataWriterQosMarshaler(ref qos))
            {
                return Gapi.DataWriter.set_qos(
                GapiPeer,
                marshaler.GapiPtr);
            }
        }

        public ReturnCode GetQos(out DataWriterQos qos)
        {
            qos = new DataWriterQos();
            ReturnCode result = ReturnCode.Error;

            using (DataWriterQosMarshaler marshaler = new DataWriterQosMarshaler())
            {
                result = Gapi.DataWriter.get_qos(
                GapiPeer,
                marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(out qos);
                }
            }

            return result;
        }

        public ITopic Topic
        {
            get
            {
                IntPtr gapiPtr = Gapi.DataWriter.get_topic(GapiPeer);

                ITopic topic = SacsSuperClass.fromUserData(gapiPtr) as ITopic;
                return topic;
            }
        }

        public IPublisher Publisher
        {
            get
            {
                IntPtr gapiPtr = Gapi.DataWriter.get_publisher(GapiPeer);

                IPublisher publisher = SacsSuperClass.fromUserData(gapiPtr) as IPublisher;
                return publisher;
            }
        }

        public ReturnCode WaitForAcknowledgments(Duration maxWait)
        {
            return Gapi.DataWriter.wait_for_acknowledgments(
                GapiPeer,
                ref maxWait);
        }

        public ReturnCode GetLivelinessLostStatus(out LivelinessLostStatus status)
        {
            return Gapi.DataWriter.get_liveliness_lost_status(
                 GapiPeer,
                 out status);
        }

        public ReturnCode GetOfferedDeadlineMissedStatus(out OfferedDeadlineMissedStatus status)
        {
            return Gapi.DataWriter.get_offered_deadline_missed_status(
                 GapiPeer,
                 out status);
        }

        public ReturnCode GetOfferedIncompatibleQosStatus(out OfferedIncompatibleQosStatus status)
        {
            return Gapi.DataWriter.get_offered_incompatible_qos_status(
                 GapiPeer,
                 out status);
        }

        public ReturnCode GetPublicationMatchedStatus(out PublicationMatchedStatus status)
        {
            return Gapi.DataWriter.get_publication_matched_status(
                 GapiPeer,
                 out status);
        }

        public ReturnCode AssertLiveliness()
        {
            return Gapi.DataWriter.assert_liveliness(GapiPeer);
        }

        public ReturnCode GetMatchedSubscriptions(out InstanceHandle[] subscriptionHandles)
        {
            subscriptionHandles = null;
            ReturnCode result = ReturnCode.Error;

            using (SequenceInstanceHandleMarshaler marshaler = new SequenceInstanceHandleMarshaler())
            {
                result = Gapi.DataWriter.get_matched_subscriptions(
                    GapiPeer,
                    marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(out subscriptionHandles);
                }
            }

            return result;
        }

        public ReturnCode GetMatchedSubscriptionData(out SubscriptionBuiltinTopicData subscriptionData, InstanceHandle subscriptionHandle)
        {
            subscriptionData = new SubscriptionBuiltinTopicData();
            ReturnCode result = ReturnCode.Error;

            using (SubscriptionBuiltinTopicDataMarshaler marshaler = new SubscriptionBuiltinTopicDataMarshaler())
            {
                result = Gapi.DataWriter.get_matched_subscription_data(
                    GapiPeer,
                    marshaler.GapiPtr,
                    subscriptionHandle);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(out subscriptionData);
                }
            }

            return result;
        }

        protected virtual void CopyIn() { }

        protected virtual void Cleanup() { }

        protected virtual void CopyOut() { }
    }
}
