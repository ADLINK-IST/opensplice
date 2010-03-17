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
using System.Runtime.InteropServices;
using System.Collections.Generic;
using DDS;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
{
    /// <summary>
    /// Topic is the most basic description of the data to be published and subscribed.
    /// A Topic is identified by its name, which must be unique in the whole Domain. In
    /// addition (by virtue of extending TopicDescription) it fully identifies the type of
    /// data that can be communicated when publishing or subscribing to the Topic.
    /// Topic is the only TopicDescription that can be used for publications and
    /// therefore a specialized DataWriter is associated to the Topic.
    /// </summary>
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

        /// <summary>
        /// This operation attaches a TopicListener to the Topic.
        /// </summary>
        /// <param name="listener">The listener to be attached to the Topic.</param>
        /// <param name="mask">A bit mask in which each bit enables the invocation of the TopicListener for a certain status.</param>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources.</returns>
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

        /// <summary>
        /// This operation obtains the InconsistentTopicStatus of the Topic.
        /// </summary>
        /// <param name="status">the contents of the InconsistentTopicStatus struct of the Topic will be 
        /// copied into the location specified by status.</param>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources</returns>
        public ReturnCode GetInconsistentTopicStatus(ref InconsistentTopicStatus status)
        {
            if (status == null) status = new InconsistentTopicStatus();
            return Gapi.Topic.get_inconsistent_topic_status(GapiPeer, status);
        }

        /// <summary>
        /// This operation allows access to the existing set of QoS policies for a Topic.
        /// </summary>
        /// <param name="qos">A reference to the destination TopicQos struct in which the QosPolicy 
        /// settings will be copied.</param>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources</returns>
        public ReturnCode GetQos(ref TopicQos qos)
        {
            ReturnCode result;
            
            using (TopicQosMarshaler marshaler = new TopicQosMarshaler())
            {
                result = Gapi.Topic.get_qos(GapiPeer, marshaler.GapiPtr);
                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref qos);
                }
            }

            return result;
        }

        /// <summary>
        /// This operation replaces the existing set of QosPolicy settings for a Topic.
        /// </summary>
        /// <param name="qos">The new set of QosPolicy settings for a Topic.</param>
        /// <returns>Return codes are:Ok,Error,BadParameter,Unsupported,AlreadyDeleted,
        /// OutOfResources,ImmutablePolicy or InconsistentPolicy.</returns>
        public ReturnCode SetQos(TopicQos qos)
        {
            ReturnCode result;

            using (TopicQosMarshaler marshaler = new TopicQosMarshaler())
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
