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
    /// <summary>
    /// The Publisher acts on behalf of one or more DataWriter objects that belong to
    /// it. When it is informed of a change to the data associated with one of its
    /// DataWriter objects, it decides when it is appropriate to actually process the
    /// sample-update message. In making this decision, it considers the PublisherQos
    /// and the DataWriterQos.
    /// </summary>
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

        /// <summary>
        /// This operation attaches a PublisherListener to the Publisher.
        /// </summary>
        /// <param name="listener">The PublisherListener instance which will be attached to the publisher.</param>
        /// <param name="mask">a bit mask in which each bit enables the invocation of the PublisherListener 
        /// for a certain status.</param>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources.</returns>
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
                lock (listener)
                {
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
                            TypeSupport typeSupport = topic.Participant.GetTypeSupport(topic.TypeName)
                                    as OpenSplice.TypeSupport;
                            dataWriter = typeSupport.CreateDataWriter(gapiPtr);
                            dataWriter.SetListener(listenerHelper);
                        }
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
                    TypeSupport typeSupport = topic.Participant.GetTypeSupport(topic.TypeName)
                            as OpenSplice.TypeSupport;
                    dataWriter = typeSupport.CreateDataWriter(gapiPtr);
                }
            }
            return dataWriter;
        }

        public IDataWriter CreateDataWriter(ITopic topic, DataWriterQos qos)
        {
            return CreateDataWriter(topic, qos, null, 0);
        }

        /// <summary>
        /// This operation creates a DataWriter with the desired DataWriterQos, for the
        /// desired Topic and attaches the optionally specified DataWriterListener to it.
        /// </summary>
        /// <param name="topic">The topic for which the DataWriter is created.</param>
        /// <param name="qos">The DataWriterQos for the new DataWriter. 
        /// In case these settings are not self consistent, no DataWriter is created.</param>
        /// <param name="listener">The DataWriterListener instance which will be attached to the new
        /// DataWriter. It is permitted to use NULL as the value of the listener: this
        /// behaves as a DataWriterListener whose operations perform no action.</param>
        /// <param name="mask">a bit mask in which each bit enables the invocation of the DataWriterListener  
        /// for a certain status.</param>
        /// <returns>The newly created DataWriter. In case of an error a null value DataWriter is returned.</returns>
        public IDataWriter CreateDataWriter(
                ITopic topic, 
                DataWriterQos qos,
                IDataWriterListener listener, 
                StatusKind mask)
        {
            DataWriter dataWriter = null;
            Topic topicObj = topic as Topic;

            using (DataWriterQosMarshaler marshaler = new DataWriterQosMarshaler())
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
                        lock (listener)
                        {
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
                                    TypeSupport typeSupport = topic.Participant.GetTypeSupport(topic.TypeName)
                                            as OpenSplice.TypeSupport;
                                    dataWriter = typeSupport.CreateDataWriter(gapiPtr);
                                    dataWriter.SetListener(listenerHelper);
                                }
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
                            TypeSupport typeSupport = topic.Participant.GetTypeSupport(topic.TypeName)
                                    as OpenSplice.TypeSupport;
                            dataWriter = typeSupport.CreateDataWriter(gapiPtr);
                        }
                    }
                }
            }

            return dataWriter;
        }

        /// <summary>
        /// This operation deletes a DataWriter that belongs to the Publisher.
        /// </summary>
        /// <param name="dataWriter">The DataWriter which is to be deleted.</param>
        /// <returns>Return codes are: Ok,Error,BadParameter,AlreadyDeleted,OutOfResources or PreconditionNotMet</returns>
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

        /// <summary>
        /// This operation returns a previously created DataWriter belonging to the Publisher 
        /// which is attached to a Topic with the matching topicName.
        /// </summary>
        /// <param name="topicName">the name of the Topic, which is attached to the DataWriter to look for.</param>
        /// <returns>The DataWriter found. When no such DataWriter is found, a DataWriter of null value is returned.</returns>
        public IDataWriter LookupDataWriter(string topicName)
        {
            IntPtr gapiPtr = Gapi.Publisher.lookup_datawriter(
                    GapiPeer,
                    topicName);

            IDataWriter dataWriter = SacsSuperClass.fromUserData(gapiPtr) as IDataWriter;
            return dataWriter;
        }

        /// <summary>
        /// This operation deletes all the DataWriter objects that were created by means of 
        /// one of the CreateDataWriter operations on the Publisher.
        /// </summary>
        /// <returns>Return codes are: Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode DeleteContainedEntities()
        {
            return Gapi.Publisher.delete_contained_entities(
                    GapiPeer,
                    null,
                    IntPtr.Zero);
        }

        /// <summary>
        /// This operation replaces the existing set of QosPolicy settings for a Publisher.
        /// </summary>
        /// <param name="qos">The new set of QosPolicy settings for the Publisher.</param>
        /// <returns>Return codes are:Ok,Error,BadParameter,Unsupported,AlreadyDeleted,OutOfResources or ImmutablePolicy.</returns>
        public ReturnCode SetQos(PublisherQos qos)
        {
            ReturnCode result;

            using (PublisherQosMarshaler marshaler = new PublisherQosMarshaler())
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

        /// <summary>
        /// This operation allows access to the existing set of QoS policies for a Publisher.
        /// </summary>
        /// <param name="qos">A reference to the destination PublisherQos struct in which the 
        /// QosPolicy settings will be copied.</param>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode GetQos(ref PublisherQos qos)
        {
            ReturnCode result;

            using (PublisherQosMarshaler marshaler = new PublisherQosMarshaler())
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

        /// <summary>
        /// This operation will suspend the dissemination of the publications by all contained DataWriter objects.
        /// </summary>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted,OutOfResources or NotEnabled.</returns>
        public ReturnCode SuspendPublications()
        {
            return Gapi.Publisher.suspend_publications(GapiPeer);
        }

        /// <summary>
        /// This operation resumes a previously suspended publication.
        /// </summary>
        /// <returns>Return codes are: Ok,Error,AlreadyDeleted,OutOfResources,NotEnabled or PreconditionNotMet.</returns>
        public ReturnCode ResumePublications()
        {
            return Gapi.Publisher.resume_publications(GapiPeer);
        }

        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <returns></returns>
        public ReturnCode BeginCoherentChanges()
        {
            return Gapi.Publisher.begin_coherent_changes(GapiPeer);
        }

        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <returns></returns>
        public ReturnCode EndCoherentChanges()
        {
            return Gapi.Publisher.end_coherent_changes(GapiPeer);
        }

        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="maxWait"></param>
        /// <returns></returns>
        public ReturnCode WaitForAcknowledgments(Duration maxWait)
        {
            return Gapi.Publisher.wait_for_acknowledgments(
                    GapiPeer,
                    ref maxWait);
        }

        /// <summary>
        /// This operation returns the DomainParticipant associated with the Publisher.
        /// </summary>
        /// <returns>The DomainParticipant associated with the Publisher or a Publisher of null value if an error occurs.</returns>
        public IDomainParticipant GetParticipant()
        {
            IntPtr gapiPtr = Gapi.Publisher.get_participant(GapiPeer);

            IDomainParticipant domainParticipant = SacsSuperClass.fromUserData(gapiPtr) 
                    as IDomainParticipant;
            return domainParticipant;
        }

        /// <summary>
        /// This operation sets the default DataWriterQos of the Publisher.
        /// </summary>
        /// <param name="qos">The DataWriterQos struct, which contains the new default DataWriterQos 
        /// for the newly created DataWriters.</param>
        /// <returns>Return codes are:Ok,Error,BadParameter,Unsupported,AlreadyDeleted,OutOfResources 
        /// or InconsistentPolicy.</returns>
        public ReturnCode SetDefaultDataWriterQos(DataWriterQos qos)
        {
            ReturnCode result;

            using (DataWriterQosMarshaler marshaler = new DataWriterQosMarshaler())
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

        /// <summary>
        /// This operation gets the default DataWriterQos of the Publisher.
        /// </summary>
        /// <param name="qos">A reference to the DataWriterQos struct (provided by the application) in which
        /// the default DataWriterQos for the DataWriter is written.</param>
        /// <returns>Return codes are: Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode GetDefaultDataWriterQos(ref DataWriterQos qos)
        {
            ReturnCode result;

            using (DataWriterQosMarshaler marshaler = new DataWriterQosMarshaler())
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

        /// <summary>
        /// This operation will copy policies in topicQos to the corresponding policies in dataWriterQos.
        /// </summary>
        /// <param name="dataWriterQos">The destination DataWriterQos struct to which the QosPolicy settings 
        /// should be copied.</param>
        /// <param name="topicQos">The source TopicQos struct, which should be copied.</param>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode CopyFromTopicQos(ref DataWriterQos dataWriterQos, TopicQos topicQos)
        {
            ReturnCode result = ReturnCode.Ok;

            if (dataWriterQos == null) 
            {
                result = GetDefaultDataWriterQos(ref dataWriterQos);
            }

            if (result == ReturnCode.Ok)
            {
                using (TopicQosMarshaler marshaler = new TopicQosMarshaler())
                {
                    result = marshaler.CopyIn(topicQos);
                    if (result == ReturnCode.Ok)
                    {
                        using (DataWriterQosMarshaler dataWriterMarshaler = 
                                new DataWriterQosMarshaler())
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
