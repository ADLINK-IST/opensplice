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
    internal class Publisher : Entity, IPublisher
    {
        private readonly PublisherDataWriterListenerHelper listenerHelper;

        internal Publisher(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
            listenerHelper = new PublisherDataWriterListenerHelper();
        }

        internal Publisher(IntPtr gapiPtr, PublisherDataWriterListenerHelper listenerHelper)
            : base(gapiPtr)
        {
            // Base class handles everything.
            this.listenerHelper = listenerHelper;
        }

        public ReturnCode SetListener(PublisherListener listener, StatusKind mask)
        {
            ReturnCode result = ReturnCode.Error;

            Gapi.gapi_publisherDataWriterListener gapiListener;
            listenerHelper.Listener = listener;
            listenerHelper.CreateListener(out gapiListener);
            using (PublisherDataWriterListenerMarshaler marshaler = new PublisherDataWriterListenerMarshaler(ref gapiListener))
            {
                result = Gapi.Publisher.set_listener(
                    GapiPeer,
                    marshaler.GapiPtr,
                    mask);
            }

            return result;
        }

        public IDataWriter CreateDataWriter(ITopic topic)
        {
            IDataWriter dataWriter = null;
            Topic topicObj = (Topic)topic;

            IntPtr gapiPtr = Gapi.Publisher.create_datawriter(
                GapiPeer,
                topicObj.GapiPeer,
                IntPtr.Zero,
                IntPtr.Zero,
                StatusKind.Any);
            if (gapiPtr != IntPtr.Zero)
            {
                // TODO: check this and maybe do some error handling
                IDomainParticipant participant = topic.Participant;
                string name = topic.TypeName;
                ITypeSupport support = participant.GetTypeSupport(name);
                TypeSupport typeSupport = (TypeSupport)support;
                dataWriter = typeSupport.CreateDataWriter(gapiPtr);
            }
            return dataWriter;
        }

        public IDataWriter CreateDataWriter(ITopic topic,
            IDataWriterListener listener, StatusKind mask)
        {
            // just in case they use this with null
            if (listener == null)
            {
                return CreateDataWriter(topic);
            }

            DataWriter dataWriter = null;
            Topic topicObj = (Topic)topic;

            // Note: we use the same gapi lister as the DataWriter since the
            // publisher doesn't add anything unique
            OpenSplice.Gapi.gapi_publisherDataWriterListener gapiListener;
            PublisherDataWriterListenerHelper listenerHelper = new PublisherDataWriterListenerHelper();
            listenerHelper.Listener = listener;
            listenerHelper.CreateListener(out gapiListener);
            using (PublisherDataWriterListenerMarshaler listenerMarshaler = new PublisherDataWriterListenerMarshaler(ref gapiListener))
            {
                IntPtr gapiPtr = Gapi.Publisher.create_datawriter(
                    GapiPeer,
                    topicObj.GapiPeer,
                    IntPtr.Zero,
                    listenerMarshaler.GapiPtr,
                    mask);
                if (gapiPtr != IntPtr.Zero)
                {
                    // TODO: check this and maybe do some error handling
                    IDomainParticipant participant = topic.Participant;
                    string name = topic.TypeName;
                    ITypeSupport support = participant.GetTypeSupport(name);
                    TypeSupport typeSupport = (TypeSupport)support;
                    dataWriter = typeSupport.CreateDataWriter(gapiPtr);
                    dataWriter.SetListener(listenerHelper);
                }
            }

            return dataWriter;
        }

        public IDataWriter CreateDataWriter(ITopic topic, ref DataWriterQos qos)
        {
            IDataWriter dataWriter = null;
            Topic topicObj = (Topic)topic;

            using (IMarshaler marshaler = new DataWriterQosMarshaler(ref qos))
            {
                IntPtr gapiPtr = Gapi.Publisher.create_datawriter(
                    GapiPeer,
                    topicObj.GapiPeer,
                    marshaler.GapiPtr,
                    IntPtr.Zero,
                    StatusKind.Any);
                if (gapiPtr != IntPtr.Zero)
                {
                    // TODO: check this and maybe do some error handling
                    IDomainParticipant participant = topic.Participant;
                    string name = topic.TypeName;
                    ITypeSupport support = participant.GetTypeSupport(name);
                    TypeSupport typeSupport = (TypeSupport)support;
                    dataWriter = typeSupport.CreateDataWriter(gapiPtr);
                }
            }

            return dataWriter;
        }

        public IDataWriter CreateDataWriter(ITopic topic, ref DataWriterQos qos,
            IDataWriterListener listener, StatusKind mask)
        {
            // just in case they use this with null
            if (listener == null)
            {
                return CreateDataWriter(topic, ref qos);
            }

            DataWriter dataWriter = null;
            Topic topicObj = (Topic)topic;

            using (IMarshaler marshaler = new DataWriterQosMarshaler(ref qos))
            {
                // Note: we use the same gapi lister as the DataWriter since the
                // publisher doesn't add anything unique
                OpenSplice.Gapi.gapi_publisherDataWriterListener gapiListener;
                PublisherDataWriterListenerHelper listenerHelper = new PublisherDataWriterListenerHelper();
                listenerHelper.Listener = listener;
                listenerHelper.CreateListener(out gapiListener);
                using (PublisherDataWriterListenerMarshaler listenerMarshaler = new PublisherDataWriterListenerMarshaler(ref gapiListener))
                {
                    IntPtr gapiPtr = Gapi.Publisher.create_datawriter(
                        GapiPeer,
                        topicObj.GapiPeer,
                        marshaler.GapiPtr,
                        listenerMarshaler.GapiPtr,
                        mask);
                    if (gapiPtr != IntPtr.Zero)
                    {
                        // TODO: check this and maybe do some error handling
                        IDomainParticipant participant = topic.Participant;
                        string name = topic.TypeName;
                        ITypeSupport support = participant.GetTypeSupport(name);
                        TypeSupport typeSupport = (TypeSupport)support;
                        dataWriter = typeSupport.CreateDataWriter(gapiPtr);
                        dataWriter.SetListener(listenerHelper);
                    }
                }
            }

            return dataWriter;
        }

        public ReturnCode DeleteDataWriter(IDataWriter dataWriter)
        {
            DataWriter dataWriterObj = (DataWriter)dataWriter;

            return Gapi.Publisher.delete_datawriter(
                GapiPeer,
                dataWriterObj.GapiPeer);
        }

        public IDataWriter LookupDataWriter(string topicName)
        {
            IntPtr gapiPtr = Gapi.Publisher.lookup_datawriter(
                GapiPeer,
                topicName);

            IDataWriter dataWriter = SacsSuperClass.fromUserData(gapiPtr) as IDataWriter;
            return dataWriter;
        }

        public ReturnCode DeleteContainedEntities()
        {
            return Gapi.Publisher.delete_contained_entities(
                GapiPeer,
                null,
                IntPtr.Zero);
        }

        public ReturnCode SetQos(ref PublisherQos qos)
        {
            using (IMarshaler marshaler = new PublisherQosMarshaler(ref qos))
            {
                return Gapi.Publisher.set_qos(
                GapiPeer,
                marshaler.GapiPtr);
            }
        }

        public ReturnCode GetQos(out PublisherQos qos)
        {
            qos = new PublisherQos();
            ReturnCode result = ReturnCode.Error;

            using (PublisherQosMarshaler marshaler = new PublisherQosMarshaler())
            {
                result = Gapi.Publisher.get_qos(
                GapiPeer,
                marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(out qos);
                }
            }

            return result;
        }

        public ReturnCode SuspendPublications()
        {
            return Gapi.Publisher.suspend_publications(GapiPeer);
        }

        public ReturnCode ResumePublications()
        {
            return Gapi.Publisher.resume_publications(GapiPeer);
        }

        public ReturnCode BeginCoherentChanges()
        {
            return Gapi.Publisher.begin_coherent_changes(GapiPeer);
        }

        public ReturnCode EndCoherentChanges()
        {
            return Gapi.Publisher.end_coherent_changes(GapiPeer);
        }

        public ReturnCode WaitForAcknowledgments(Duration maxWait)
        {
            return Gapi.Publisher.wait_for_acknowledgments(
                GapiPeer,
                ref maxWait);
        }

        public IDomainParticipant GetParticipant()
        {
            IntPtr gapiPtr = Gapi.Publisher.get_participant(GapiPeer);

            IDomainParticipant domainParticipant = SacsSuperClass.fromUserData(gapiPtr) as IDomainParticipant;
            return domainParticipant;
        }

        public ReturnCode SetDefaultDataWriterQos(ref DataWriterQos qos)
        {
            using (IMarshaler marshaler = new DataWriterQosMarshaler(ref qos))
            {
                return Gapi.Publisher.set_default_datawriter_qos(
                GapiPeer,
                marshaler.GapiPtr);
            }
        }

        public ReturnCode GetDefaultDataWriterQos(out DataWriterQos qos)
        {
            qos = new DataWriterQos();
            ReturnCode result = ReturnCode.Error;

            using (DataWriterQosMarshaler marshaler = new DataWriterQosMarshaler())
            {
                result = Gapi.Publisher.get_default_datawriter_qos(
                GapiPeer,
                marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(out qos);
                }
            }

            return result;
        }

        public ReturnCode CopyFromTopicQos(out DataWriterQos dataWriterQos, ref TopicQos topicQos)
        {
            dataWriterQos = new DataWriterQos();
            ReturnCode result = ReturnCode.Error;

            using (IMarshaler marshaler = new TopicQosMarshaler(ref topicQos))
            {
                using (DataWriterQosMarshaler dataWriterMarshaler = new
                    DataWriterQosMarshaler())
                {
                    result = Gapi.Publisher.copy_from_topic_qos(
                    GapiPeer,
                    dataWriterMarshaler.GapiPtr,
                    marshaler.GapiPtr);

                    if (result == ReturnCode.Ok)
                    {
                        dataWriterMarshaler.CopyOut(out dataWriterQos);
                    }
                }
            }

            return result;
        }
    }
}
