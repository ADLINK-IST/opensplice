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
    internal class TopicListenerHelper
    {
        private ITopicListener listener;

        private Gapi.gapi_listener_InconsistentTopicListener onInconsistentTopicDelegate;

        public ITopicListener Listener
        {
            get { return listener; }
            set { listener = value; }
        }

        private void Topic_PrivateOnInconsistentTopic(IntPtr entityData, IntPtr topicPtr, InconsistentTopicStatus status)
        {
            if (listener != null)
            {
                ITopic topic = (ITopic)OpenSplice.SacsSuperClass.fromUserData(topicPtr);
                listener.OnInconsistentTopic(topic, status);
            }
        }

        internal void CreateListener(out OpenSplice.Gapi.gapi_topicListener listener)
        {
            onInconsistentTopicDelegate = Topic_PrivateOnInconsistentTopic;

            listener = new DDS.OpenSplice.Gapi.gapi_topicListener();
            listener.listener_data = IntPtr.Zero;
            listener.on_inconsistent_topic = onInconsistentTopicDelegate;
        }

    }
}
