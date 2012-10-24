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
    // NOTE: There is no PublisherListener and DataWriterListener share the same helper
    internal class PublisherDataWriterListenerHelper
    {
        private IDataWriterListener listener;

        private Gapi.gapi_listener_OfferedDeadlineMissedListener onOfferedDeadlineMissedDelegate;
        private Gapi.gapi_listener_LivelinessLostListener onLivelinessLostDelegate;
        private Gapi.gapi_listener_OfferedIncompatibleQosListener onOfferedIncompatibleQosDelegate;
        private Gapi.gapi_listener_PublicationMatchedListener onPublicationMatchDelegate;

        public IDataWriterListener Listener
        {
            get { return listener; }
            set { listener = value; }
        }

        private void PrivateOfferedDeadlineMissed(IntPtr entityData, IntPtr writerPtr, OfferedDeadlineMissedStatus status)
        {
            if (listener != null)
            {
                IDataWriter dataWriter = (IDataWriter)OpenSplice.SacsSuperClass.fromUserData(writerPtr);
                listener.OnOfferedDeadlineMissed(dataWriter, status);
            }
        }

        private void PrivateLivelinessLost(IntPtr entityData, IntPtr writerPtr, LivelinessLostStatus status)
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

        private void PrivatePublicationMatched(IntPtr entityData, IntPtr writerPtr, PublicationMatchedStatus status)
        {
            if (listener != null)
            {
                IDataWriter dataWriter = (IDataWriter)OpenSplice.SacsSuperClass.fromUserData(writerPtr);
                listener.OnPublicationMatched(dataWriter, status);
            }
        }

        internal void CreateListener(out OpenSplice.Gapi.gapi_publisherDataWriterListener listener)
        {
            onOfferedDeadlineMissedDelegate = PrivateOfferedDeadlineMissed;
            onLivelinessLostDelegate = PrivateLivelinessLost;
            onOfferedIncompatibleQosDelegate = PrivateOfferedIncompatibleQos;
            onPublicationMatchDelegate = PrivatePublicationMatched;

            listener = new DDS.OpenSplice.Gapi.gapi_publisherDataWriterListener();
            listener.listener_data = IntPtr.Zero;
            listener.on_offered_deadline_missed = onOfferedDeadlineMissedDelegate;
            listener.on_liveliness_lost = onLivelinessLostDelegate;
            listener.on_offered_incompatible_qos = onOfferedIncompatibleQosDelegate;
            listener.on_publication_match = onPublicationMatchDelegate;
        }

    }
}
