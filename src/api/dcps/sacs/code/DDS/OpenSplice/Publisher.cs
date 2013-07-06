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
        
        public ReturnCode SetListener(IPublisherListener listener, StatusKind mask)
        {
            ReturnCode result = ReturnCode.Error;

            if (listener != null)
            {
                Gapi.gapi_publisherDataWriterListener gapiListener;
                listenerHelper.Listener = listener;
                listenerHelper.CreateListener(out gapiListener);
                lock (listener)
                {
                    using (PublisherDataWriterListenerMarshaler marshaler =
                            new PublisherDataWriterListenerMarshaler(ref gapiListener))
                    {
                        result = Gapi.Publisher.set_listener(
                                GapiPeer,
                                marshaler.GapiPtr,
                                mask);
                    }
                }
            }
            else
            {
                result = Gapi.Publisher.set_listener(
                        GapiPeer,
                        IntPtr.Zero,
                        mask);
            }
            return result;
        }

        public IDataWriter CreateDataWriter(ITopic topic)
        {
            return CreateDataWriter(topic, null, 0);
        }

        public IDataWriter CreateDataWriter(
                ITopic topic,
                IDataWriterListener listener, 
                StatusKind mask)
        {
            DataWriter dataWriter = null;
            Topic topicObj = topic as Topic;

            if (listener != null)
            {
                // Note: we use the same gapi lister as the DataWriter since the
                // publisher doesn't add anything unique
                OpenSplice.Gapi.gapi_publisherDataWriterListener gapiListener;
                PublisherDataWriterListenerHelper listenerHelper = new PublisherDataWriterListenerHelper();
                listenerHelper.Listener = listener;
                listenerHelper.CreateListener(out gapiListener);
                using (PublisherDataWriterListenerMarshaler listenerMarshaler =
                        new PublisherDataWriterListenerMarshaler(ref gapiListener))
                {
                    IntPtr gapiPtr = Gapi.Publisher.create_datawriter(
                            GapiPeer,
                            topicObj.GapiPeer,
                            Gapi.NativeConstants.GapiDataWriterQosDefault,
                            listenerMarshaler.GapiPtr,
                            mask);
                    if (gapiPtr != IntPtr.Zero)
                    {
                        DDS.OpenSplice.DomainParticipant participantImpl;
                    
                        participantImpl = topic.Participant as DDS.OpenSplice.DomainParticipant;
                        TypeSupportFactory tsFactory = participantImpl.GetTypeSupportFactory(topic.TypeName);
                        dataWriter = tsFactory.CreateDataWriter(gapiPtr);
                        dataWriter.SetListener(listenerHelper);
                    }
                }
            }
            else
            {
                IntPtr gapiPtr = Gapi.Publisher.create_datawriter(
                            GapiPeer,
                            topicObj.GapiPeer,
                            Gapi.NativeConstants.GapiDataWriterQosDefault,
                            IntPtr.Zero,
                            mask);
                if (gapiPtr != IntPtr.Zero)
                {
                    DDS.OpenSplice.DomainParticipant participantImpl;
                    
                    participantImpl = topic.Participant as DDS.OpenSplice.DomainParticipant;
                    TypeSupportFactory tsFactory = participantImpl.GetTypeSupportFactory(topic.TypeName);
                    dataWriter = tsFactory.CreateDataWriter(gapiPtr);
                }
            }

            if (dataWriter != null)
            {
                PublisherQos pubQos = null;
                ReturnCode result = GetQos(ref pubQos);
                if (result == ReturnCode.Ok)
                {
                    if (pubQos.EntityFactory.AutoenableCreatedEntities)
                    {
                        dataWriter.Enable();
                    }
                }
            }       
                
            return dataWriter;
        }

        public IDataWriter CreateDataWriter(ITopic topic, DataWriterQos qos)
        {
            return CreateDataWriter(topic, qos, null, 0);
        }
        
        public IDataWriter CreateDataWriter(
                ITopic topic, 
                DataWriterQos qos,
                IDataWriterListener listener, 
                StatusKind mask)
        {
            DataWriter dataWriter = null;
            Topic topicObj = topic as Topic;

            using (OpenSplice.CustomMarshalers.DataWriterQosMarshaler marshaler = 
                    new OpenSplice.CustomMarshalers.DataWriterQosMarshaler())
            {
                if (marshaler.CopyIn(qos) == ReturnCode.Ok)
                {
                    if (listener != null)
                    {
                        // Note: we use the same gapi lister as the DataWriter since the
                        // publisher doesn't add anything unique
                        OpenSplice.Gapi.gapi_publisherDataWriterListener gapiListener;
                        PublisherDataWriterListenerHelper listenerHelper = new PublisherDataWriterListenerHelper();
                        listenerHelper.Listener = listener;
                        listenerHelper.CreateListener(out gapiListener);
                        using (PublisherDataWriterListenerMarshaler listenerMarshaler =
                                new PublisherDataWriterListenerMarshaler(ref gapiListener))
                        {
                            IntPtr gapiPtr = Gapi.Publisher.create_datawriter(
                                    GapiPeer,
                                    topicObj.GapiPeer,
                                    marshaler.GapiPtr,
                                    listenerMarshaler.GapiPtr,
                                    mask);
                            if (gapiPtr != IntPtr.Zero)
                            {
                                DDS.OpenSplice.DomainParticipant participantImpl;
                    
                                participantImpl = topic.Participant as DDS.OpenSplice.DomainParticipant;
                                TypeSupportFactory tsFactory = participantImpl.GetTypeSupportFactory(topic.TypeName);
                                dataWriter = tsFactory.CreateDataWriter(gapiPtr);
                                dataWriter.SetListener(listenerHelper);
                            }
                        }
                    }
                    else
                    {
                        // Invoke the corresponding gapi function.
                        IntPtr gapiPtr = Gapi.Publisher.create_datawriter(
                                GapiPeer,
                                topicObj.GapiPeer,
                                marshaler.GapiPtr,
                                IntPtr.Zero,
                                mask);
                        if (gapiPtr != IntPtr.Zero)
                        {
                            DDS.OpenSplice.DomainParticipant participantImpl;
                
                            participantImpl = topic.Participant as DDS.OpenSplice.DomainParticipant;
                            TypeSupportFactory tsFactory = participantImpl.GetTypeSupportFactory(topic.TypeName);
                            dataWriter = tsFactory.CreateDataWriter(gapiPtr);
                        }
                    }
                }
            }

            if (dataWriter != null)
            {
                PublisherQos pubQos = null;
                ReturnCode result = GetQos(ref pubQos);
                if (result == ReturnCode.Ok)
                {
                    if (pubQos.EntityFactory.AutoenableCreatedEntities)
                    {
                        dataWriter.Enable();
                    }
                }
            }       
                
            return dataWriter;
        }
        
        public ReturnCode DeleteDataWriter(IDataWriter dataWriter)
        {
            ReturnCode result = ReturnCode.BadParameter;
            DataWriter dataWriterObj = dataWriter as DataWriter;

            if (dataWriterObj != null)
            {
                result = Gapi.Publisher.delete_datawriter(
                        GapiPeer,
                        dataWriterObj.GapiPeer);
            }
            return result;
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
            return Gapi.Publisher.delete_contained_entities(GapiPeer);
        }
        
        public ReturnCode SetQos(PublisherQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.PublisherQosMarshaler marshaler = 
                    new OpenSplice.CustomMarshalers.PublisherQosMarshaler())
            {
                result = marshaler.CopyIn(qos);
                if (result == ReturnCode.Ok)
                {
                    // Invoke the corresponding gapi function.
                    result = Gapi.Publisher.set_qos(
                            GapiPeer,
                            marshaler.GapiPtr);
                }
            }
            return result;
        }
        
        public ReturnCode GetQos(ref PublisherQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.PublisherQosMarshaler marshaler = 
                    new OpenSplice.CustomMarshalers.PublisherQosMarshaler())
            {
                result = Gapi.Publisher.get_qos(
                        GapiPeer,
                        marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref qos);
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

            IDomainParticipant domainParticipant = SacsSuperClass.fromUserData(gapiPtr) 
                    as IDomainParticipant;
            return domainParticipant;
        }
        
        public ReturnCode SetDefaultDataWriterQos(DataWriterQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.DataWriterQosMarshaler marshaler = 
                    new OpenSplice.CustomMarshalers.DataWriterQosMarshaler())
            {
                result = marshaler.CopyIn(qos);
                if (result == ReturnCode.Ok)
                {
                    result = Gapi.Publisher.set_default_datawriter_qos(
                            GapiPeer,
                            marshaler.GapiPtr);
                }
            }
            return result;
        }
        
        public ReturnCode GetDefaultDataWriterQos(ref DataWriterQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.DataWriterQosMarshaler marshaler = 
                    new OpenSplice.CustomMarshalers.DataWriterQosMarshaler())
            {
                result = Gapi.Publisher.get_default_datawriter_qos(
                        GapiPeer,
                        marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref qos);
                }
            }

            return result;
        }
        
        public ReturnCode CopyFromTopicQos(ref DataWriterQos dataWriterQos, TopicQos topicQos)
        {
            ReturnCode result = ReturnCode.Ok;

            if (dataWriterQos == null) 
            {
                result = GetDefaultDataWriterQos(ref dataWriterQos);
            }

            if (result == ReturnCode.Ok)
            {
                using (OpenSplice.CustomMarshalers.TopicQosMarshaler marshaler = 
                        new OpenSplice.CustomMarshalers.TopicQosMarshaler())
                {
                    result = marshaler.CopyIn(topicQos);
                    if (result == ReturnCode.Ok)
                    {
                        using (OpenSplice.CustomMarshalers.DataWriterQosMarshaler dataWriterMarshaler = 
                                new OpenSplice.CustomMarshalers.DataWriterQosMarshaler())
                        {
                            result = dataWriterMarshaler.CopyIn(dataWriterQos);
                            if (result == ReturnCode.Ok)
                            {
                                result = Gapi.Publisher.copy_from_topic_qos(
                                        GapiPeer,
                                        dataWriterMarshaler.GapiPtr,
                                        marshaler.GapiPtr);

                                if (result == ReturnCode.Ok)
                                {
                                    dataWriterMarshaler.CopyOut(ref dataWriterQos);
                                }
                            }
                        }
                    }
                }
            }

            return result;
        }
    }
}
