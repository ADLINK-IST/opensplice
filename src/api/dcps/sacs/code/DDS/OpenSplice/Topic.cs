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
using System.Runtime.InteropServices;
using System.Collections.Generic;
using DDS;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
{
    internal class Topic : Entity, ITopic
    {
        private readonly TopicListenerHelper listenerHelper;

        internal Topic(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
            listenerHelper = new TopicListenerHelper();
        }

        internal Topic(IntPtr gapiPtr, TopicListenerHelper listenerHelper)
            : base(gapiPtr)
        {
            // Base class handles everything.
            this.listenerHelper = listenerHelper;
        }
        
        public ReturnCode SetListener(ITopicListener listener, StatusKind mask)
        {
            ReturnCode result = ReturnCode.Error;

            Gapi.gapi_topicListener gapiListener;
            listenerHelper.Listener = listener;
            listenerHelper.CreateListener(out gapiListener);
            if (listener != null)
            {
                lock (listener)
                {
                    using (TopicListenerMarshaler marshaler = new TopicListenerMarshaler(ref gapiListener))
                    {
                        result = Gapi.Topic.set_listener(
                            GapiPeer,
                            marshaler.GapiPtr,
                            mask);
                    }
                }
            }
            else
            {
                result = Gapi.Topic.set_listener(
                    GapiPeer,
                    IntPtr.Zero,
                    mask);
            }
            return result;
        }
        
        public ReturnCode GetInconsistentTopicStatus(ref InconsistentTopicStatus status)
        {
            if (status == null) status = new InconsistentTopicStatus();
            return Gapi.Topic.get_inconsistent_topic_status(GapiPeer, status);
        }
        
        public ReturnCode GetQos(ref TopicQos qos)
        {
            ReturnCode result;
            
            using (OpenSplice.CustomMarshalers.TopicQosMarshaler marshaler = 
                    new OpenSplice.CustomMarshalers.TopicQosMarshaler())
            {
                result = Gapi.Topic.get_qos(GapiPeer, marshaler.GapiPtr);
                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref qos);
                }
            }

            return result;
        }
        
        public ReturnCode SetQos(TopicQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.TopicQosMarshaler marshaler = 
                    new OpenSplice.CustomMarshalers.TopicQosMarshaler())
            {
                result = marshaler.CopyIn(qos);
                if (result == DDS.ReturnCode.Ok)
                {
                    result = Gapi.Topic.set_qos(GapiPeer, marshaler.GapiPtr);
                }
            }

            return result;
        }

        /// <summary>
        /// This property returns the registered name of the data type associated with the TopicDescription 
        /// (inherited from TopicDescription)
        /// </summary>
        public string TypeName
        {
            get
            {
                IntPtr ptr = Gapi.TopicDescription.get_type_name(GapiPeer);
                string result = Marshal.PtrToStringAnsi(ptr);
                Gapi.GenericAllocRelease.Free(ptr);

                return result;
            }
        }

        /// <summary>
        /// This property returns the name used to create the TopicDescription.
        /// (inherited from TopicDescription)
        /// </summary>
        public string Name
        {
            get
            {
                IntPtr ptr = Gapi.TopicDescription.get_name(GapiPeer);
                string result = Marshal.PtrToStringAnsi(ptr);
                Gapi.GenericAllocRelease.Free(ptr);

                return result;
            }
        }

        /// <summary>
        /// This property returns the DomainParticipant associated with the TopicDescription or 
        /// a null DomainParticipant.
        /// (inherited from TopicDescription)
        /// </summary>
        public IDomainParticipant Participant
        {
            get
            {
                IntPtr gapiPtr = Gapi.TopicDescription.get_participant(GapiPeer);
                IDomainParticipant domainParticipant = SacsSuperClass.fromUserData(gapiPtr) as IDomainParticipant;

                return domainParticipant;
            }
        }
    }
}
