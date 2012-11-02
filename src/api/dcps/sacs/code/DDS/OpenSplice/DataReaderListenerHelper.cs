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
    internal class DataReaderListenerHelper
    {
        private Gapi.gapi_listener_RequestedDeadlineMissedListener onRequestedDeadlineMissedDelegate;
        private Gapi.gapi_listener_RequestedIncompatibleQosListener onRequestedIncompatibleQosDelegate;
        private Gapi.gapi_listener_SampleRejectedListener onSampleRejectedDelegate;
        private Gapi.gapi_listener_LivelinessChangedListener onLivelinessChangedDelegate;
        private Gapi.gapi_listener_DataAvailableListener onDataAvailableDelegate;
        private Gapi.gapi_listener_SubscriptionMatchedListener onSubscriptionMatchDelegate;
        private Gapi.gapi_listener_SampleLostListener onSampleLostDelegate;

        private IDataReaderListener listener;

        public IDataReaderListener Listener
        {
            get { return listener; }
            set { listener = value; }
        }

        private void PrivateRequestedDeadlineMissed(IntPtr entityData, IntPtr enityPtr, RequestedDeadlineMissedStatus status)
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

        private void PrivateSampleRejected(IntPtr entityData, IntPtr enityPtr, SampleRejectedStatus status)
        {
            if (listener != null)
            {
                IDataReader dataReader = (IDataReader)OpenSplice.SacsSuperClass.fromUserData(enityPtr);
                listener.OnSampleRejected(dataReader, status);
            }
        }

        private void PrivateLivelinessChanged(IntPtr entityData, IntPtr enityPtr, LivelinessChangedStatus status)
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

        private void PrivateSubscriptionMatched(IntPtr entityData, IntPtr enityPtr, SubscriptionMatchedStatus status)
        {
            if (listener != null)
            {
                IDataReader dataReader = (IDataReader)OpenSplice.SacsSuperClass.fromUserData(enityPtr);
                listener.OnSubscriptionMatched(dataReader, status);
            }
        }

        private void PrivateSampleLost(IntPtr entityData, IntPtr enityPtr, SampleLostStatus status)
        {
            if (listener != null)
            {
                IDataReader dataReader = (IDataReader)OpenSplice.SacsSuperClass.fromUserData(enityPtr);
                listener.OnSampleLost(dataReader, status);
            }
        }

        internal void CreateListener(out OpenSplice.Gapi.gapi_dataReaderListener listener)
        {
            onRequestedDeadlineMissedDelegate = PrivateRequestedDeadlineMissed;
            onRequestedIncompatibleQosDelegate = PrivateRequestedIncompatibleQos;
            onSampleRejectedDelegate = PrivateSampleRejected;
            onLivelinessChangedDelegate = PrivateLivelinessChanged;
            onDataAvailableDelegate = PrivateDataAvailable;
            onSubscriptionMatchDelegate = PrivateSubscriptionMatched;
            onSampleLostDelegate = PrivateSampleLost;

            listener = new DDS.OpenSplice.Gapi.gapi_dataReaderListener();
            listener.listener_data = IntPtr.Zero;
            listener.on_requested_deadline_missed = onRequestedDeadlineMissedDelegate;
            listener.on_requested_incompatible_qos = onRequestedIncompatibleQosDelegate;
            listener.on_sample_rejected = onSampleRejectedDelegate;
            listener.on_liveliness_changed = onLivelinessChangedDelegate;
            listener.on_data_available = onDataAvailableDelegate;
            listener.on_subscription_match = onSubscriptionMatchDelegate;
            listener.on_sample_lost = onSampleLostDelegate;
        }
    }
}
