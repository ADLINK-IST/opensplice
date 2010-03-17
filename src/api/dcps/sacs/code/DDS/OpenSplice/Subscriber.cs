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
    /// A Subscriber is the object responsible for the actual reception of the data
    /// resulting from its subscriptions.
    /// A Subscriber acts on behalf of one or more DataReader objects that are related
    /// to it. When it receives data (from the other parts of the system), it indicates to the
    /// application that data is available through its DataReaderListener and by
    /// enabling related Conditions. The application can access the list of concerned
    /// DataReader objects through the operation get_datareaders and then access the
    /// data available through operations on the DataReader.
    /// </summary>
    internal class Subscriber : Entity, ISubscriber
    {
        private readonly SubscriberListenerHelper listenerHelper;

        internal Subscriber(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
            listenerHelper = new SubscriberListenerHelper();
        }

        internal Subscriber(IntPtr gapiPtr, SubscriberListenerHelper listenerHelper)
            : base(gapiPtr)
        {
            // Base class handles everything.
            this.listenerHelper = listenerHelper;
        }

        /// <summary>
        /// This operation attaches a SubscriberListener to the Subscriber.
        /// </summary>
        /// <param name="listener">The SubscriberListener instance, which will be attached to the Subscriber.</param>
        /// <param name="mask">A bit mask in which each bit enables the invocation of the SubscriberListener 
        /// for a certain status.</param>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode SetListener(ISubscriberListener listener, StatusKind mask)
        {
            ReturnCode result = ReturnCode.Error;

            if (listener != null)
            {
                Gapi.gapi_subscriberListener gapiListener;
                listenerHelper.Listener = listener;
                listenerHelper.CreateListener(out gapiListener);
                lock (listener)
                {
                    using (SubscriberListenerMarshaler marshaler =
                            new SubscriberListenerMarshaler(ref gapiListener))
                    {
                        result = Gapi.Subscriber.set_listener(
                                GapiPeer,
                                marshaler.GapiPtr,
                                mask);
                    }
                }
            }
            else
            {
                result = Gapi.Subscriber.set_listener(
                        GapiPeer,
                        IntPtr.Zero,
                        mask);
            }

            return result;
        }

        public IDataReader CreateDataReader(ITopicDescription topic)
        {
            return CreateDataReader(topic, null, 0);
        }

        public IDataReader CreateDataReader(
                ITopicDescription topic,
                IDataReaderListener listener, 
                StatusKind mask)
        {

            DataReader dataReader = null;

            if (topic != null)
            {
                SacsSuperClass superObj = (SacsSuperClass)topic;

                if (listener != null)
                {
                    OpenSplice.Gapi.gapi_dataReaderListener gapiListener;
                    DataReaderListenerHelper listenerHelper = new DataReaderListenerHelper();
                    listenerHelper.Listener = listener;
                    listenerHelper.CreateListener(out gapiListener);
                    lock (listener)
                    {
                        using (DataReaderListenerMarshaler listenerMarshaler =
                                new DataReaderListenerMarshaler(ref gapiListener))
                        {
                            IntPtr gapiPtr = Gapi.Subscriber.create_datareader(
                                    GapiPeer,
                                    superObj.GapiPeer,
                                    IntPtr.Zero,
                                    listenerMarshaler.GapiPtr,
                                    mask);
                            if (gapiPtr != IntPtr.Zero)
                            {
                                TypeSupport typeSupport = topic.Participant.GetTypeSupport(topic.TypeName)
                                        as OpenSplice.TypeSupport;
                                dataReader = typeSupport.CreateDataReader(gapiPtr);
                                dataReader.SetListener(listenerHelper);
                            }
                        }
                    }
                }
                else
                {
                    IntPtr gapiPtr = Gapi.Subscriber.create_datareader(
                            GapiPeer,
                            superObj.GapiPeer,
                            IntPtr.Zero,
                            IntPtr.Zero,
                            StatusKind.Any);
                    if (gapiPtr != IntPtr.Zero)
                    {
                        TypeSupport typeSupport = topic.Participant.GetTypeSupport(topic.TypeName)
                                as OpenSplice.TypeSupport;
                        dataReader = typeSupport.CreateDataReader(gapiPtr);
                    }
                }
            }
            return dataReader;
        }

        public IDataReader CreateDataReader(ITopicDescription topic, DataReaderQos qos)
        {
            return CreateDataReader(topic, qos, null, 0);
        }

        /// <summary>
        /// This operation creates a DataReader with the desired QosPolicy settings, for the
        /// desired TopicDescription and attaches the optionally specified DataWriterListener to it.
        /// </summary>
        /// <param name="topic">The TopicDescription for which the DataReader is created. 
        /// This may be a Topic, MultiTopic or ContentFilteredTopic.</param>
        /// <param name="qos">The struct with the QosPolicy settings for the new DataReader, 
        /// when these QosPolicy settings are not self consistent, no DataReader is created.</param>
        /// <param name="listener">The DataReaderListener instance which will be attached to the new
        /// DataReader. It is permitted to use NULL as the value of the listener: this
        /// behaves as a DataWriterListener whose operations perform no action.</param>
        /// <param name="mask">A bit-mask in which each bit enables the invocation of 
        /// the DataReaderListener for a certain status.</param>
        /// <returns>The newly created DataReader, or in case of an error a null value one.</returns>
        public IDataReader CreateDataReader(
                ITopicDescription topic, 
                DataReaderQos qos,
                IDataReaderListener listener, 
                StatusKind mask)
        {
            DataReader dataReader = null;
            
            if (topic != null)
            {
                SacsSuperClass superObj = (SacsSuperClass)topic;

                using (DataReaderQosMarshaler marshaler = new DataReaderQosMarshaler())
                {
                    if (marshaler.CopyIn(qos) == ReturnCode.Ok)
                    {
                        if (listener != null)
                        {
                            OpenSplice.Gapi.gapi_dataReaderListener gapiListener;
                            DataReaderListenerHelper listenerHelper = new DataReaderListenerHelper();
                            listenerHelper.Listener = listener;
                            listenerHelper.CreateListener(out gapiListener);
                            lock (listener)
                            {
                                using (DataReaderListenerMarshaler listenerMarshaler =
                                        new DataReaderListenerMarshaler(ref gapiListener))
                                {
                                    IntPtr gapiPtr = Gapi.Subscriber.create_datareader(
                                            GapiPeer,
                                            superObj.GapiPeer,
                                            marshaler.GapiPtr,
                                            listenerMarshaler.GapiPtr,
                                            mask);
                                    if (gapiPtr != IntPtr.Zero)
                                    {
                                        TypeSupport typeSupport = topic.Participant.GetTypeSupport(topic.TypeName)
                                                as OpenSplice.TypeSupport;
                                        dataReader = typeSupport.CreateDataReader(gapiPtr);
                                        dataReader.SetListener(listenerHelper);
                                    }
                                }
                            }
                        }
                        else
                        {
                            IntPtr gapiPtr = Gapi.Subscriber.create_datareader(
                                    GapiPeer,
                                    superObj.GapiPeer,
                                    marshaler.GapiPtr,
                                    IntPtr.Zero,
                                    mask);
                            if (gapiPtr != IntPtr.Zero)
                            {
                                TypeSupport typeSupport = topic.Participant.GetTypeSupport(topic.TypeName)
                                        as OpenSplice.TypeSupport;
                                dataReader = typeSupport.CreateDataReader(gapiPtr);
                            }
                        }
                    }
                }
            }

            return dataReader;
        }

        /// <summary>
        /// This operation deletes a DataReader that belongs to the Subscriber.
        /// </summary>
        /// <param name="dataReader">The DataReader which is to be deleted.</param>
        /// <returns>Return codes are:Ok,Error,BadParameter.AlreadyDeleted,OutOfResources or PreconditionNotMet.</returns>
        public ReturnCode DeleteDataReader(IDataReader dataReader)
        {
            ReturnCode result = ReturnCode.BadParameter;
            DataReader dataReaderObj = dataReader as DataReader;

            if (dataReaderObj != null)
            {
                result = Gapi.Subscriber.delete_datareader(
                        GapiPeer,
                        dataReaderObj.GapiPeer);
            }
            return result;
        }

        /// <summary>
        /// This operation returns a previously created DataReader belonging to the
        /// Subscriber which is attached to a Topic with the matching topic_name.
        /// </summary>
        /// <param name="topicName">The name of the Topic, which is attached to the DataReader to look for.</param>
        /// <returns>A reference to the DataReader found. If no such DataReader is found the a null value one is 
        /// returned.</returns>
        public IDataReader LookupDataReader(string topicName)
        {
            IntPtr gapiPtr = Gapi.Subscriber.lookup_datareader(
                    GapiPeer,
                    topicName);

            IDataReader dataReader = SacsSuperClass.fromUserData(gapiPtr) as IDataReader;
            return dataReader;
        }

        /// <summary>
        /// This operation deletes all the DataReader objects that were created by means of
        /// the create_datareader operation on the Subscriber.
        /// </summary>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode DeleteContainedEntities()
        {
            return Gapi.Subscriber.delete_contained_entities(
                GapiPeer,
                null,
                IntPtr.Zero);
        }

        public ReturnCode GetDataReaders(ref IDataReader[] readers)
        {
            return GetDataReaders(ref readers, SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="readers"></param>
        /// <param name="sampleStates"></param>
        /// <param name="viewStates"></param>
        /// <param name="instanceStates"></param>
        /// <returns></returns>
        public ReturnCode GetDataReaders(
                ref IDataReader[] readers, 
                SampleStateKind sampleStates,
                ViewStateKind viewStates, 
                InstanceStateKind instanceStates)
        {
            ReturnCode result = ReturnCode.Error;

            using (SequenceMarshaler<IDataReader, DataReader> marshaler = 
                    new SequenceMarshaler<IDataReader, DataReader>())
            {
                result = Gapi.Subscriber.get_datareaders(
                        GapiPeer,
                        marshaler.GapiPtr,
                        sampleStates,
                        viewStates,
                        instanceStates);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref readers);
                }
            }

            return result;
        }

        /// <summary>
        /// This operation invokes the on_data_available operation on
        /// DataReaderListener objects which are attached to the contained DataReader
        /// entities having new, available data.
        /// </summary>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode NotifyDataReaders()
        {
            return Gapi.Subscriber.notify_datareaders(GapiPeer);
        }

        /// <summary>
        /// This operation replaces the existing set of QosPolicy settings for a Subscriber.
        /// </summary>
        /// <param name="qos">The new set of QosPolicy settings for the Subscriber.</param>
        /// <returns>Return codes are:Ok,Error,BadParameter,Unsupported,AlreadyDeleted,
        /// OutOfResources or ImmutablePolicy</returns>
        public ReturnCode SetQos(SubscriberQos qos)
        {
            DDS.ReturnCode result;
            
            using (SubscriberQosMarshaler marshaler = new SubscriberQosMarshaler())
            {
                result = marshaler.CopyIn(qos);
                if (result == DDS.ReturnCode.Ok)
                {
                    result = OpenSplice.Gapi.Subscriber.set_qos(
                            GapiPeer,
                            marshaler.GapiPtr);
                }
            }

            return result;
        }

        /// <summary>
        /// This operation allows access to the existing set of QoS policies for a Subscriber.
        /// </summary>
        /// <param name="qos">A reference to the destination SubscriberQos struct in which the QosPolicy 
        /// settings will be copied.</param>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources</returns>
        public ReturnCode GetQos(ref SubscriberQos qos)
        {
            ReturnCode result;

            using (SubscriberQosMarshaler marshaler = new SubscriberQosMarshaler())
            {
                result = OpenSplice.Gapi.Subscriber.get_qos(
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
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <returns></returns>
        public ReturnCode BeginAccess()
        {
            return OpenSplice.Gapi.Subscriber.begin_access(GapiPeer);
        }

        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <returns></returns>
        public ReturnCode EndAccess()
        {
            return OpenSplice.Gapi.Subscriber.end_access(GapiPeer);
        }

        /// <summary>
        /// This property allows access to the DomainParticipant associated with the Subscriber.
        /// </summary>
        public IDomainParticipant Participant
        {
            get
            {
                IntPtr gapiPtr = Gapi.Subscriber.get_participant(GapiPeer);

                IDomainParticipant domainParticipant = SacsSuperClass.fromUserData(gapiPtr) 
                        as IDomainParticipant;
                return domainParticipant;
            }
        }

        /// <summary>
        /// This operation sets the default DataReaderQos of the DataReader.
        /// </summary>
        /// <param name="qos">The DataReaderQos struct, which containsthe new default QosPolicy 
        /// settings for the newly created DataReaders.</param>
        /// <returns>Return codes are:Ok,Error,BadParameter,Unsupported,AlreadyDeleted,OutOfResources or 
        /// InconsistentPolicy.</returns>
        public ReturnCode SetDefaultDataReaderQos(DataReaderQos qos)
        {
            DDS.ReturnCode result;
            
            using (DataReaderQosMarshaler marshaler = new DataReaderQosMarshaler())
            {
                result = marshaler.CopyIn(qos);
                if (result == DDS.ReturnCode.Ok)
                {
                    return OpenSplice.Gapi.Subscriber.set_default_datareader_qos(
                            GapiPeer,
                            marshaler.GapiPtr);
                }
            }

            return result;
        }

        /// <summary>
        /// This operation gets the default QosPolicy settings of the DataReader.
        /// </summary>
        /// <param name="qos">A reference to the DataReaderQos struct(provided by the application) 
        /// in which the default QosPolicy settings for the DataReader are written.</param>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode GetDefaultDataReaderQos(ref DataReaderQos qos)
        {
            ReturnCode result;

            using (DataReaderQosMarshaler marshaler = new DataReaderQosMarshaler())
            {
                result = OpenSplice.Gapi.Subscriber.get_default_datareader_qos(
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
        /// This operation will copy the policies in a_topic_qos to the corresponding policies in datareaderQos.
        /// </summary>
        /// <param name="dataReaderQos">The destination DataReaderQos struct to which the QosPolicy settings 
        /// will be copied.</param>
        /// <param name="topicQos">The source TopicQos, which will be copied.</param>
        /// <returns>Return codes are: Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode CopyFromTopicQos(ref DataReaderQos dataReaderQos, TopicQos topicQos)
        {
            ReturnCode result = ReturnCode.Ok;

            if (dataReaderQos == null) 
            {
                result = GetDefaultDataReaderQos(ref dataReaderQos);
            }

            if (result == ReturnCode.Ok)
            {
                using (TopicQosMarshaler marshaler = new TopicQosMarshaler())
                {
                    result = marshaler.CopyIn(topicQos);
                    if (result == ReturnCode.Ok)
                    {
                        using (DataReaderQosMarshaler dataReaderMarshaler = 
                                new DataReaderQosMarshaler())
                        {
                            result = dataReaderMarshaler.CopyIn(dataReaderQos);
                            if (result == ReturnCode.Ok)
                            {
                                result = OpenSplice.Gapi.Subscriber.copy_from_topic_qos(
                                        GapiPeer,
                                        dataReaderMarshaler.GapiPtr,
                                        marshaler.GapiPtr);

                                if (result == ReturnCode.Ok)
                                {
                                    dataReaderMarshaler.CopyOut(ref dataReaderQos);
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
