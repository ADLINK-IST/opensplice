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
using System.Collections.Generic;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
{
    internal class DomainParticipantListenerHelper
    {
        private Gapi.gapi_listener_InconsistentTopicListener onInconsistentTopicDelegate;

        private Gapi.gapi_listener_OfferedDeadlineMissedListener onOfferedDeadlineMissedDelegate;
        private Gapi.gapi_listener_LivelinessLostListener onLivelinessLostDelegate;
        private Gapi.gapi_listener_OfferedIncompatibleQosListener onOfferedIncompatibleQosDelegate;
        private Gapi.gapi_listener_PublicationMatchedListener onPublicationMatchDelegate;

        private Gapi.gapi_listener_RequestedDeadlineMissedListener onRequestedDeadlineMissedDelegate;
        private Gapi.gapi_listener_RequestedIncompatibleQosListener onRequestedIncompatibleQosDelegate;
        private Gapi.gapi_listener_SampleRejectedListener onSampleRejectedDelegate;
        private Gapi.gapi_listener_LivelinessChangedListener onLivelinessChangedDelegate;
        private Gapi.gapi_listener_DataAvailableListener onDataAvailableDelegate;
        private Gapi.gapi_listener_SubscriptionMatchedListener onSubscriptionMatchDelegate;
        private Gapi.gapi_listener_SampleLostListener onSampleLostDelegate;

        private Gapi.gapi_listener_DataOnReadersListener onDataOnReadersDelegate;

        private IDomainParticipantListener listener;

        public IDomainParticipantListener Listener
        {
            get { return listener; }
            set { listener = value; }
        }

        // ITopicListener
        private void Topic_PrivateOnInconsistentTopic(
                IntPtr entityData, IntPtr topicPtr, 
                InconsistentTopicStatus status)
        {
            if (listener != null)
            {
                ITopic topic = (ITopic)OpenSplice.SacsSuperClass.fromUserData(topicPtr);
                listener.OnInconsistentTopic(topic, status);
            }
        }

        // IDataWriterListener
        private void PrivateOfferedDeadlineMissed(
                IntPtr entityData, 
                IntPtr writerPtr, 
                OfferedDeadlineMissedStatus status)
        {
            if (listener != null)
            {
                IDataWriter dataWriter = (IDataWriter)OpenSplice.SacsSuperClass.fromUserData(writerPtr);
                listener.OnOfferedDeadlineMissed(dataWriter, status);
            }
        }

        private void PrivateLivelinessLost(
                IntPtr entityData, 
                IntPtr writerPtr, 
                LivelinessLostStatus status)
        {
            if (listener != null)
            {
                IDataWriter dataWriter = (IDataWriter)OpenSplice.SacsSuperClass.fromUserData(writerPtr);
                listener.OnLivelinessLost(dataWriter, status);
            }
        }

        private void PrivateOfferedIncompatibleQos(
                IntPtr entityData, 
                IntPtr writerPtr, 
                IntPtr gapi_status)
        {
            if (listener != null)
            {
                IDataWriter dataWriter = (IDataWriter)OpenSplice.SacsSuperClass.fromUserData(writerPtr);
                OfferedIncompatibleQosStatus status = new OfferedIncompatibleQosStatus();
                OfferedIncompatibleQosStatusMarshaler.CopyOut(gapi_status, ref status, 0);
                listener.OnOfferedIncompatibleQos(dataWriter, status);
            }
        }

        private void PrivatePublicationMatched(
                IntPtr entityData, 
                IntPtr writerPtr, 
                PublicationMatchedStatus status)
        {
            if (listener != null)
            {
                IDataWriter dataWriter = (IDataWriter)OpenSplice.SacsSuperClass.fromUserData(writerPtr);
                listener.OnPublicationMatched(dataWriter, status);
            }
        }

        // ISubscriberListener
        private void PrivateDataOnReaders(IntPtr entityData, IntPtr enityPtr)
        {
            if (listener != null)
            {
                ISubscriber subscriber = (ISubscriber)OpenSplice.SacsSuperClass.fromUserData(enityPtr);
                listener.OnDataOnReaders(subscriber);
            }
        }

        // IDataReaderListener
        private void PrivateRequestedDeadlineMissed(
                IntPtr entityData, 
                IntPtr enityPtr, 
                RequestedDeadlineMissedStatus status)
        {
            if (listener != null)
            {
                IDataReader dataReader = (IDataReader)OpenSplice.SacsSuperClass.fromUserData(enityPtr);
                listener.OnRequestedDeadlineMissed(dataReader, status);
            }
        }

        private void PrivateRequestedIncompatibleQos(
                IntPtr entityData, 
                IntPtr enityPtr, 
                IntPtr gapi_status)
        {
            if (listener != null)
            {
                IDataReader dataReader = (IDataReader)OpenSplice.SacsSuperClass.fromUserData(enityPtr);
                RequestedIncompatibleQosStatus status = new RequestedIncompatibleQosStatus();
                RequestedIncompatibleQosStatusMarshaler.CopyOut(gapi_status, ref status, 0);
                listener.OnRequestedIncompatibleQos(dataReader, status);
            }
        }

        private void PrivateSampleRejected(
                IntPtr entityData, 
                IntPtr enityPtr, 
                SampleRejectedStatus status)
        {
            if (listener != null)
            {
                IDataReader dataReader = (IDataReader)OpenSplice.SacsSuperClass.fromUserData(enityPtr);
                listener.OnSampleRejected(dataReader, status);
            }
        }

        private void PrivateLivelinessChanged(
                IntPtr entityData, 
                IntPtr enityPtr, 
                LivelinessChangedStatus status)
        {
            if (listener != null)
            {
                IDataReader dataReader = (IDataReader)OpenSplice.SacsSuperClass.fromUserData(enityPtr);
                listener.OnLivelinessChanged(dataReader, status);
            }
        }

        private void PrivateDataAvailable(IntPtr entityData, IntPtr enityPtr)
        {
            if (listener != null)
            {
                IDataReader dataReader = (IDataReader)OpenSplice.SacsSuperClass.fromUserData(enityPtr);
                listener.OnDataAvailable(dataReader);
            }
        }

        private void PrivateSubscriptionMatched(
                IntPtr entityData, 
                IntPtr enityPtr, 
                SubscriptionMatchedStatus status)
        {
            if (listener != null)
            {
                IDataReader dataReader = (IDataReader)OpenSplice.SacsSuperClass.fromUserData(enityPtr);
                listener.OnSubscriptionMatched(dataReader, status);
            }
        }

        private void PrivateSampleLost(
                IntPtr entityData, 
                IntPtr enityPtr, 
                SampleLostStatus status)
        {
            if (listener != null)
            {
                IDataReader dataReader = (IDataReader)OpenSplice.SacsSuperClass.fromUserData(enityPtr);
                listener.OnSampleLost(dataReader, status);
            }
        }

        internal void CreateListener(out OpenSplice.Gapi.gapi_domainParticipantListener gapiListener)
        {
            onInconsistentTopicDelegate = Topic_PrivateOnInconsistentTopic;

            onOfferedDeadlineMissedDelegate = PrivateOfferedDeadlineMissed;
            onOfferedIncompatibleQosDelegate = PrivateOfferedIncompatibleQos;
            onLivelinessLostDelegate = PrivateLivelinessLost;
            onPublicationMatchDelegate = PrivatePublicationMatched;

            onRequestedDeadlineMissedDelegate = PrivateRequestedDeadlineMissed;
            onRequestedIncompatibleQosDelegate = PrivateRequestedIncompatibleQos;
            onSampleRejectedDelegate = PrivateSampleRejected;
            onLivelinessChangedDelegate = PrivateLivelinessChanged;
            onDataAvailableDelegate = PrivateDataAvailable;
            onSubscriptionMatchDelegate = PrivateSubscriptionMatched;
            onSampleLostDelegate = PrivateSampleLost;

            onDataOnReadersDelegate = PrivateDataOnReaders;

            gapiListener = new DDS.OpenSplice.Gapi.gapi_domainParticipantListener();

            gapiListener.listener_data = IntPtr.Zero;
            gapiListener.on_inconsistent_topic = onInconsistentTopicDelegate;

            gapiListener.on_offered_deadline_missed = onOfferedDeadlineMissedDelegate;
            gapiListener.on_offered_incompatible_qos = onOfferedIncompatibleQosDelegate;
            gapiListener.on_liveliness_lost = onLivelinessLostDelegate;
            gapiListener.on_publication_match = onPublicationMatchDelegate;
            gapiListener.on_requested_deadline_missed = onRequestedDeadlineMissedDelegate;
            gapiListener.on_requested_incompatible_qos = onRequestedIncompatibleQosDelegate;
            gapiListener.on_sample_rejected = onSampleRejectedDelegate;
            gapiListener.on_liveliness_changed = onLivelinessChangedDelegate;
            gapiListener.on_data_available = onDataAvailableDelegate;
            gapiListener.on_subscription_match = onSubscriptionMatchDelegate;
            gapiListener.on_sample_lost = onSampleLostDelegate;
            gapiListener.on_data_on_readers = onDataOnReadersDelegate;
        }

    }
}
