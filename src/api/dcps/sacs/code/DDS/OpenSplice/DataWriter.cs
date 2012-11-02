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
                    using (PublisherDataWriterListenerMarshaler marshaler = 
                            new PublisherDataWriterListenerMarshaler(ref gapiListener))
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
        
        public ReturnCode SetQos(DataWriterQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.DataWriterQosMarshaler marshaler = 
                    new OpenSplice.CustomMarshalers.DataWriterQosMarshaler())
            {
                result = marshaler.CopyIn(qos);
                if (result == ReturnCode.Ok)
                {
                    result = Gapi.DataWriter.set_qos(GapiPeer, marshaler.GapiPtr);
                }
            }

            return result;
        }
        
        public ReturnCode GetQos(ref DataWriterQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.DataWriterQosMarshaler marshaler = 
                    new OpenSplice.CustomMarshalers.DataWriterQosMarshaler())
            {
                result = Gapi.DataWriter.get_qos(GapiPeer, marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref qos);
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
            return Gapi.DataWriter.wait_for_acknowledgments(GapiPeer, ref maxWait);
        }
        
        public ReturnCode GetLivelinessLostStatus(
                ref LivelinessLostStatus status)
        {
            if (status == null) status = new LivelinessLostStatus();
            return Gapi.DataWriter.get_liveliness_lost_status(GapiPeer, status);
        }
        
        public ReturnCode GetOfferedDeadlineMissedStatus(
                ref OfferedDeadlineMissedStatus status)
        {
            if (status == null) status = new OfferedDeadlineMissedStatus();
            return Gapi.DataWriter.get_offered_deadline_missed_status(GapiPeer, status);
        }
        
        public ReturnCode GetOfferedIncompatibleQosStatus(
                ref OfferedIncompatibleQosStatus status)
        {
            ReturnCode result;

            using (OfferedIncompatibleQosStatusMarshaler marshaler = 
                    new OfferedIncompatibleQosStatusMarshaler())
            {
                if (status == null) status = new OfferedIncompatibleQosStatus();
                if (status.Policies == null) status.Policies = new QosPolicyCount[28];
                marshaler.CopyIn(status);
                
                result = Gapi.DataWriter.get_offered_incompatible_qos_status(
                        GapiPeer, marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref status);
                }
            }

            return result;
        }
        
        public ReturnCode GetPublicationMatchedStatus(
                ref PublicationMatchedStatus status)
        {
            if (status == null) status = new PublicationMatchedStatus();
            return Gapi.DataWriter.get_publication_matched_status(GapiPeer, status);
        }
        
        public ReturnCode AssertLiveliness()
        {
            return Gapi.DataWriter.assert_liveliness(GapiPeer);
        }
        
        public ReturnCode GetMatchedSubscriptions(ref InstanceHandle[] subscriptionHandles)
        {
            ReturnCode result;

            using (SequenceInstanceHandleMarshaler marshaler = 
                    new SequenceInstanceHandleMarshaler())
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
        
        public ReturnCode GetMatchedSubscriptionData(
                ref SubscriptionBuiltinTopicData subscriptionData, 
                InstanceHandle subscriptionHandle)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.SubscriptionBuiltinTopicDataMarshaler marshaler = 
                    new OpenSplice.CustomMarshalers.SubscriptionBuiltinTopicDataMarshaler())
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
