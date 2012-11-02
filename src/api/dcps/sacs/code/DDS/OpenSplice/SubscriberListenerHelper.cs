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
    internal class SubscriberListenerHelper : DataReaderListenerHelper
    {
        private ISubscriberListener listener;

        private Gapi.gapi_listener_DataOnReadersListener onDataOnReadersDelegate;

        public new ISubscriberListener Listener
        {
            get { return listener; }
            set { listener = value; }
        }

        private void PrivateDataOnReaders(IntPtr entityData, IntPtr enityPtr)
        {
            if (listener != null)
            {
                ISubscriber subscriber = (ISubscriber)OpenSplice.SacsSuperClass.fromUserData(enityPtr);
                listener.OnDataOnReaders(subscriber);
            }
        }

        internal void CreateListener(out OpenSplice.Gapi.gapi_subscriberListener listener)
        {
            onDataOnReadersDelegate = PrivateDataOnReaders;

            listener = new DDS.OpenSplice.Gapi.gapi_subscriberListener();
            base.CreateListener(out listener.dataReader);
            listener.on_data_on_readers = onDataOnReadersDelegate;
        }

    }
}
