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
using DDS;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
{
    /// <summary>
    /// All the DCPS Entity objects are attached to a DomainParticipant. A DomainParticipant 
    /// represents the local membership of the application in a Domain.
    /// </summary>
    /// <remarks>A Domain is a distributed concept that links all the applications that must be able to 
    /// communicate with each other. It represents a communication plane: only the 
    /// Publishers and the Subscribers attached to the same Domain can interact. 
    /// This class implements several functions:
    /// <list type="bullet">
    /// <item>it acts as a container for all other Entity objects</item>
    /// <item>it acts as a factory for the Publisher, Subscriber, Topic,ContentFilteredTopic 
    /// and MultiTopic objects</item>
    /// <item>it provides access to the built-in Topic objects</item>
    /// <item>it provides information about Topic objects</item>
    /// <item>It isolates applications within the same Domain (sharing the same domainId) 
    /// from other applications in a different Domain on the same set of computers. In this way, 
    /// several independent distributed applications can coexist in the same physical network without interfering, 
    /// or even being aware of each other.</item>
    /// <item>It provides administration services in the Domain, offering operations, which allow the application 
    /// to ignore locally any information about a given Participant, Publication, Subscription or Topic.</item>
    /// </list>
    /// </remarks>
    public class DomainParticipant : Entity, IDomainParticipant
    {
        private readonly DomainParticipantListenerHelper listenerHelper;

        /**
         * Constructor is only called by DDS.DomainParticipantFactory.
         */
        internal DomainParticipant(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            listenerHelper = new DomainParticipantListenerHelper();
            builtinTopicRegisterTypeSupport(this);
            builtinTopicCreateWrappers(this);
        }

        /**
         * Constructor is only called by DDS.DomainParticipantFactory.
         */
        internal DomainParticipant(IntPtr gapiPtr, DomainParticipantListenerHelper listenerHelper)
            : base(gapiPtr)
        {
            this.listenerHelper = listenerHelper;
            builtinTopicRegisterTypeSupport(this);
            builtinTopicCreateWrappers(this);
        }

        /**
         * Register the four builtin topics:
         *     "DDS::ParticipantBuiltinTopicData"
         *     "DDS::TopicBuiltinTopicData"
         *     "DDS::PublicationBuiltinTopicData"
         *     "DDS::SubscriptionBuiltinTopicData"
         */
        internal static ReturnCode builtinTopicRegisterTypeSupport(DomainParticipant participant)
        {
            ReturnCode result;

            DDS.ParticipantBuiltinTopicDataTypeSupport DDSParticipant = 
                    new DDS.ParticipantBuiltinTopicDataTypeSupport();
            result = DDSParticipant.RegisterType(participant, DDSParticipant.TypeName);
            if (result != ReturnCode.Ok)
            {
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.DomainParticipant.builtinTopicRegisterTypeSupport",
                        "DDS/OpenSplice/DomainParticipant.cs",
                        DDS.ErrorCode.Error,
                        "Failed to register builtin topic: DCPSParticipant");
            }

            DDS.TopicBuiltinTopicDataTypeSupport DDSTopic =
                    new DDS.TopicBuiltinTopicDataTypeSupport();
            result = DDSTopic.RegisterType(participant, DDSTopic.TypeName);
            if (result != ReturnCode.Ok)
            {
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.DomainParticipant.builtinTopicRegisterTypeSupport",
                        "DDS/OpenSplice/DomainParticipant.cs",
                        DDS.ErrorCode.Error,
                        "Failed to register builtin topic: DCPSTopic");
            }

            DDS.PublicationBuiltinTopicDataTypeSupport DDSPublication = 
                    new DDS.PublicationBuiltinTopicDataTypeSupport();
            result = DDSPublication.RegisterType(participant, DDSPublication.TypeName);
            if (result != ReturnCode.Ok)
            {
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.DomainParticipant.builtinTopicRegisterTypeSupport",
                        "DDS/OpenSplice/DomainParticipant.cs",
                        DDS.ErrorCode.Error,
                        "Failed to register builtin topic: DCPSPublication");
            }

            DDS.SubscriptionBuiltinTopicDataTypeSupport DDSSubscription = 
                    new DDS.SubscriptionBuiltinTopicDataTypeSupport();
            result = DDSSubscription.RegisterType(participant, DDSSubscription.TypeName);
            if (result != ReturnCode.Ok)
            {
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.DomainParticipant.builtinTopicRegisterTypeSupport",
                        "DDS/OpenSplice/DomainParticipant.cs",
                        DDS.ErrorCode.Error,
                        "Failed to register builtin topic: DCPSSubscription");
            }
            return result;
        }

        /**
         * Wrap the four builtin topics in C# objects:
         *     "DCPSParticipant"
         *     "DCPSTopic"
         *     "DCPSPublication"
         *     "DCPSSubscription"
         */
        internal static void builtinTopicCreateWrappers(DomainParticipant participant)
        {
            // Lookup the "DCPSParticipant" topic.
            IntPtr gapiPtr = Gapi.DomainParticipant.lookup_topicdescription(participant.GapiPeer, "DCPSParticipant");
            if (gapiPtr == IntPtr.Zero)
            {
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.DomainParticipant.builtinTopicCreateWrappers",
                        "DDS/OpenSplice/DomainParticipant.cs",
                        DDS.ErrorCode.Error,
                        "Failed to wrap builtin topic: DCPSParticipant");
            }
            else
            {
                // Wrap the gapi topic in a C# object.
                new Topic(gapiPtr);
                
                // And lookup the "DCPSTopic" topic.
                gapiPtr = Gapi.DomainParticipant.lookup_topicdescription(participant.GapiPeer, "DCPSTopic");
            }
            if (gapiPtr == IntPtr.Zero)
            {
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.DomainParticipant.builtinTopicCreateWrappers",
                        "DDS/OpenSplice/DomainParticipant.cs",
                        DDS.ErrorCode.Error,
                        "Failed to wrap builtin topic: DCPSTopic");
            }
            else
            {
                // Wrap the gapi topic in a C# object.
                new Topic(gapiPtr);
                
                // And lookup the "DCPSTopic" topic.
                gapiPtr = Gapi.DomainParticipant.lookup_topicdescription(participant.GapiPeer, "DCPSPublication");
            }
            if (gapiPtr == IntPtr.Zero)
            {
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.DomainParticipant.builtinTopicCreateWrappers",
                        "DDS/OpenSplice/DomainParticipant.cs",
                        DDS.ErrorCode.Error,
                        "Failed to wrap builtin topic: DCPSPublication");
            }
            else
            {
                // Wrap the gapi topic in a C# object.
                new Topic(gapiPtr);
                
                // And lookup the "DCPSTopic" topic.
                gapiPtr = Gapi.DomainParticipant.lookup_topicdescription(participant.GapiPeer, "DCPSSubscription");
            }
            if (gapiPtr == IntPtr.Zero)
            {
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.DomainParticipant.builtinTopicCreateWrappers",
                        "DDS/OpenSplice/DomainParticipant.cs",
                        DDS.ErrorCode.Error,
                        "Failed to wrap builtin topic: DCPSSubscription");
            }
            else
            {
                // Wrap the gapi topic in a C# object.
                new Topic(gapiPtr);
            }

        }

        /// <summary>
        /// This operation attaches a DomainParticipantListener to the DomainParticipant.
        /// </summary>
        /// <param name="listener">The DomainParticipantListener instance, which will be attached to the 
        /// DomainParticipant.</param>
        /// <param name="mask">a bit mask in which each bit enables the invocation of the DomainParticipantListener 
        /// for a certain status.</param>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode SetListener(IDomainParticipantListener listener, StatusKind mask)
        {
            ReturnCode result = ReturnCode.Error;

            if (listener != null)
            {
                Gapi.gapi_domainParticipantListener gapiListener;
                listenerHelper.Listener = listener;
                listenerHelper.CreateListener(out gapiListener);
                lock (listener)
                {
                    using (DomainParticipantListenerMarshaler marshaler = new DomainParticipantListenerMarshaler(ref gapiListener))
                    {
                        result = Gapi.DomainParticipant.set_listener(
                                GapiPeer,
                                marshaler.GapiPtr,
                                mask);
                    }
                }
            }
            else
            {
                result = Gapi.DomainParticipant.set_listener(
                        GapiPeer,
                        IntPtr.Zero,
                        mask);
            }
            return result;
        }

        public IPublisher CreatePublisher()
        {
            return CreatePublisher(null, 0);
        }

        public IPublisher CreatePublisher(IPublisherListener listener, StatusKind mask)
        {
            IPublisher publisher = null;

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
                    using (PublisherDataWriterListenerMarshaler listenerMarshaler = new PublisherDataWriterListenerMarshaler(ref gapiListener))
                    {
                        IntPtr gapiPtr = Gapi.DomainParticipant.create_publisher(
                                GapiPeer,
                                Gapi.NativeConstants.GapiPublisherQosDefault,
                                listenerMarshaler.GapiPtr,
                                mask);
                        if (gapiPtr != IntPtr.Zero)
                        {
                            publisher = new Publisher(gapiPtr, listenerHelper);
                        }
                    }
                }
            }
            else
            {
                IntPtr gapiPtr = Gapi.DomainParticipant.create_publisher(
                            GapiPeer,
                            Gapi.NativeConstants.GapiPublisherQosDefault,
                            IntPtr.Zero,
                            mask);
                if (gapiPtr != IntPtr.Zero)
                {
                    publisher = new Publisher(gapiPtr);
                }
            }
            return publisher;
        }

        public IPublisher CreatePublisher(PublisherQos qos)
        {
            return CreatePublisher(qos, null, 0);
        }

        /// <summary>
        /// This operation creates a Publisher with the desired QosPolicy settings and if applicable, 
        /// attaches the optionally specified PublisherListener to it.
        /// </summary>
        /// <param name="qos">A collection of QosPolicy settings for the new Publisher.
        /// In case these settings are not self consistent, no Publisher is created.</param>
        /// <param name="listener">The PublisherListener instance which will be attached to the new Publisher.
        /// It is permitted to use null as the value of the listener: this behaves as a PublisherListener 
        /// whose operations perform no action.</param>
        /// <param name="mask">A bit-mask in which each bit enables the invocation of the PublisherListener 
        /// for a certain status.</param>
        /// <returns>The newly created Publisher. In case of an error, a null Publisher is returned.</returns>
        public IPublisher CreatePublisher(PublisherQos qos, IPublisherListener listener, StatusKind mask)
        {
            IPublisher publisher = null;

            using (PublisherQosMarshaler marshaler = new PublisherQosMarshaler())
            {
                // Note: we use the same gapi lister as the DataWriter since the
                // publisher doesn't add anything unique
                if (marshaler.CopyIn(qos) == ReturnCode.Ok)
                {
                    if (listener != null)
                    {
                        Gapi.gapi_publisherDataWriterListener gapiListener;
                        PublisherDataWriterListenerHelper listenerHelper = new PublisherDataWriterListenerHelper();
                        listenerHelper.Listener = listener;
                        listenerHelper.CreateListener(out gapiListener);
                        lock (listener)
                        {
                            using (PublisherDataWriterListenerMarshaler listenerMarshaler =
                                    new PublisherDataWriterListenerMarshaler(ref gapiListener))
                            {
                                IntPtr gapiPtr = Gapi.DomainParticipant.create_publisher(
                                        GapiPeer,
                                        marshaler.GapiPtr,
                                        listenerMarshaler.GapiPtr,
                                        mask);
                                if (gapiPtr != IntPtr.Zero)
                                {
                                    publisher = new Publisher(gapiPtr, listenerHelper);
                                }
                            }
                        }
                    }
                    else
                    {
                        // Invoke the corresponding gapi function.
                        IntPtr gapiPtr = Gapi.DomainParticipant.create_publisher(
                                GapiPeer,
                                marshaler.GapiPtr,
                                IntPtr.Zero,
                                mask);
                        if (gapiPtr != IntPtr.Zero)
                        {
                            publisher = new Publisher(gapiPtr);
                        }
                    }
                }
            }

            return publisher;
        }

        /// <summary>
        /// This operation deletes a Publisher.
        /// </summary>
        /// <param name="p">The publisher to be deleted.</param>
        /// <returns>Ok,Error,BadParameter,Alreadydeleted,OutOfResources or PreconditionNotMet.</returns>
        public ReturnCode DeletePublisher(IPublisher p)
        {
            ReturnCode result = ReturnCode.BadParameter;

            Publisher publisher = (Publisher)p;
            if (publisher != null)
            {
                result = Gapi.DomainParticipant.delete_publisher(
                        GapiPeer,
                        publisher.GapiPeer);
            }

            return result;
        }

        public ISubscriber CreateSubscriber()
        {
            //TODO: JLS: This had been sending a StatusKind.Any before
            return CreateSubscriber(null, 0);
        }

        public ISubscriber CreateSubscriber(ISubscriberListener listener, StatusKind mask)
        {
            ISubscriber subscriber = null;

            if (listener != null)
            {
                OpenSplice.Gapi.gapi_subscriberListener gapiListener;
                SubscriberListenerHelper listenerHelper = new SubscriberListenerHelper();
                listenerHelper.Listener = listener;
                listenerHelper.CreateListener(out gapiListener);
                lock (listener)
                {
                    using (SubscriberListenerMarshaler listenerMarshaler =
                            new SubscriberListenerMarshaler(ref gapiListener))
                    {
                        IntPtr gapiPtr = Gapi.DomainParticipant.create_subscriber(
                                GapiPeer,
                                Gapi.NativeConstants.GapiSubscriberQosDefault,
                                listenerMarshaler.GapiPtr,
                                mask);
                        if (gapiPtr != IntPtr.Zero)
                        {
                            subscriber = new Subscriber(gapiPtr, listenerHelper);
                        }
                    }
                }
            }
            else
            {
                IntPtr gapiPtr = Gapi.DomainParticipant.create_subscriber(
                            GapiPeer,
                            Gapi.NativeConstants.GapiSubscriberQosDefault,
                            IntPtr.Zero,
                            mask);
                if (gapiPtr != IntPtr.Zero)
                {
                    subscriber = new Subscriber(gapiPtr);
                }
            }
            return subscriber;
        }

        public ISubscriber CreateSubscriber(SubscriberQos qos)
        {
            return CreateSubscriber(qos, null, 0);
        }

        /// <summary>
        /// This operation creates a Subscriber with the desired QosPolicy settings and if applicable, 
        /// attaches the optionally specified SubscriberListener to it.
        /// </summary>
        /// <param name="qos">a collection of QosPolicy settings for the new Subscriber. 
        /// In case these settings are not self consistent, no Subscriber is created.</param>
        /// <param name="listener">The SubscriberListener instance which will be attached to the new Subscriber. 
        /// It is permitted to use null as the value of the listener: this behaves as a SubscriberListener 
        /// whose operations perform no action.</param>
        /// <param name="mask">a bit-mask in which each bit enables the invocation of the SubscriberListener 
        /// for a certain status.</param>
        /// <returns>The newly created Subscriber. In case of an error a Subscriber with a null value is returned.</returns>
        public ISubscriber CreateSubscriber(SubscriberQos qos, ISubscriberListener listener, StatusKind mask)
        {
            ISubscriber subscriber = null;

            using (SubscriberQosMarshaler marshaler = new SubscriberQosMarshaler())
            {
                if (marshaler.CopyIn(qos) == ReturnCode.Ok)
                {
                    if (listener != null)
                    {
                        OpenSplice.Gapi.gapi_subscriberListener gapiListener;
                        SubscriberListenerHelper listenerHelper = new SubscriberListenerHelper();
                        listenerHelper.Listener = listener;
                        listenerHelper.CreateListener(out gapiListener);
                        lock (listener)
                        {
                            using (SubscriberListenerMarshaler listenerMarshaler =
                                    new SubscriberListenerMarshaler(ref gapiListener))
                            {
                                IntPtr gapiPtr = Gapi.DomainParticipant.create_subscriber(
                                        GapiPeer,
                                        marshaler.GapiPtr,
                                        listenerMarshaler.GapiPtr,
                                        mask);
                                if (gapiPtr != IntPtr.Zero)
                                {
                                    subscriber = new Subscriber(gapiPtr, listenerHelper);
                                }
                            }
                        }
                    }
                    else
                    {
                        // Invoke the corresponding gapi function.
                        IntPtr gapiPtr = Gapi.DomainParticipant.create_subscriber(
                                GapiPeer,
                                marshaler.GapiPtr,
                                IntPtr.Zero,
                                mask);
                        if (gapiPtr != IntPtr.Zero)
                        {
                            subscriber = new Subscriber(gapiPtr);
                        }
                    }
                }
            }

            return subscriber;
        }

        /// <summary>
        /// This operation deletes a Subscriber.
        /// </summary>
        /// <param name="s"> The subscriber to be deleted.</param>
        /// <returns>Return codes are: Ok,Error,BadParameter,AlreadyDeleted,OutOfResources or PreconditionNotMet.</returns>
        public ReturnCode DeleteSubscriber(ISubscriber s)
        {
            ReturnCode result = ReturnCode.BadParameter;

            Subscriber subscriber = (Subscriber)s;
            if (subscriber != null)
            {
                result = Gapi.DomainParticipant.delete_subscriber(
                        GapiPeer,
                        subscriber.GapiPeer);
            }

            return result;
        }

        /// <summary>
        /// This property returns the built-in Subscriber associated with the DomainParticipant.
        /// </summary>
        public ISubscriber BuiltInSubscriber
        {
            get
            {
                IntPtr subscriber_gapiPtr = Gapi.DomainParticipant.get_builtin_subscriber(GapiPeer);
                ISubscriber subscriber = SacsSuperClass.fromUserData(subscriber_gapiPtr) as ISubscriber;

                // If needed, associate the C# wrapper with the gapiPtr.
                if (subscriber_gapiPtr != IntPtr.Zero && subscriber == null)
                {
                    subscriber = new Subscriber(subscriber_gapiPtr);

                    IntPtr reader_gapiPtr;
                    reader_gapiPtr = Gapi.Subscriber.lookup_datareader(subscriber_gapiPtr, "DCPSTopic");
                    if (reader_gapiPtr != IntPtr.Zero)
                    {
                        DDS.TopicBuiltinTopicDataTypeSupport typeSupport = 
                                GetTypeSupport("DDS::TopicBuiltinTopicData") 
                                        as DDS.TopicBuiltinTopicDataTypeSupport;
                        typeSupport.CreateDataReader(reader_gapiPtr);
                        IntPtr topic_gapiPtr = Gapi.DataReader.get_topicdescription(reader_gapiPtr);
                        if (topic_gapiPtr != IntPtr.Zero)
                        {
                            Topic topic = new OpenSplice.Topic(topic_gapiPtr);
                        }

                    }
                    reader_gapiPtr = Gapi.Subscriber.lookup_datareader(subscriber_gapiPtr, "DCPSParticipant");
                    if (reader_gapiPtr != IntPtr.Zero){
                        DDS.ParticipantBuiltinTopicDataTypeSupport typeSupport = 
                                GetTypeSupport("DDS::ParticipantBuiltinTopicData") 
                                        as DDS.ParticipantBuiltinTopicDataTypeSupport;
                        typeSupport.CreateDataReader(reader_gapiPtr);
                        IntPtr topic_gapiPtr = Gapi.DataReader.get_topicdescription(reader_gapiPtr);
                        if (topic_gapiPtr != IntPtr.Zero)
                        {
                            Topic topic = new OpenSplice.Topic(topic_gapiPtr);
                        }

                    }
                    reader_gapiPtr = Gapi.Subscriber.lookup_datareader(subscriber_gapiPtr, "DCPSPublication");
                    if (reader_gapiPtr != IntPtr.Zero){
                        DDS.PublicationBuiltinTopicDataTypeSupport typeSupport = 
                                GetTypeSupport("DDS::PublicationBuiltinTopicData") 
                                        as DDS.PublicationBuiltinTopicDataTypeSupport;
                        typeSupport.CreateDataReader(reader_gapiPtr);
                        IntPtr topic_gapiPtr = Gapi.DataReader.get_topicdescription(reader_gapiPtr);
                        if (topic_gapiPtr != IntPtr.Zero)
                        {
                            Topic topic = new OpenSplice.Topic(topic_gapiPtr);
                        }

                    }
                    reader_gapiPtr = Gapi.Subscriber.lookup_datareader(subscriber_gapiPtr, "DCPSSubscription");
                    if (reader_gapiPtr != IntPtr.Zero){
                        DDS.SubscriptionBuiltinTopicDataTypeSupport typeSupport = 
                                GetTypeSupport("DDS::SubscriptionBuiltinTopicData") 
                                        as DDS.SubscriptionBuiltinTopicDataTypeSupport;
                        typeSupport.CreateDataReader(reader_gapiPtr);
                        IntPtr topic_gapiPtr = Gapi.DataReader.get_topicdescription(reader_gapiPtr);
                        if (topic_gapiPtr != IntPtr.Zero)
                        {
                            Topic topic = new OpenSplice.Topic(topic_gapiPtr);
                        }

                    }
                }

                return subscriber;
            }
        }

        public ITopic CreateTopic(string topicName, string typeName)
        {
            return CreateTopic(topicName, typeName, null, 0);
        }

        public ITopic CreateTopic(string topicName, string typeName, ITopicListener listener, StatusKind mask)
        {
            ITopic topic = null;

            if (listener != null)
            {
                OpenSplice.Gapi.gapi_topicListener gapiListener;
                TopicListenerHelper listenerHelper = new TopicListenerHelper();
                listenerHelper.Listener = listener;
                listenerHelper.CreateListener(out gapiListener);
                lock (listener)
                {
                    using (TopicListenerMarshaler listenerMarshaler = new TopicListenerMarshaler(ref gapiListener))
                    {
                        IntPtr gapiPtr = Gapi.DomainParticipant.create_topic(
                                GapiPeer,
                                topicName,
                                typeName,
                                Gapi.NativeConstants.GapiTopicQosDefault,
                                listenerMarshaler.GapiPtr,
                                mask);
                        if (gapiPtr != IntPtr.Zero)
                        {
                            topic = new Topic(gapiPtr, listenerHelper);
                        }
                    }
                }
            }
            else
            {
                IntPtr gapiPtr = Gapi.DomainParticipant.create_topic(
                        GapiPeer,
                        topicName,
                        typeName,
                        Gapi.NativeConstants.GapiTopicQosDefault,
                        IntPtr.Zero,
                        mask);

                if (gapiPtr != IntPtr.Zero)
                {
                    topic = new Topic(gapiPtr);
                }
            }

            return topic;
        }

        public ITopic CreateTopic(string topicName, string typeName, TopicQos qos)
        {
            return CreateTopic(topicName, typeName, qos, null, 0);
        }

        /// <summary>
        /// This operation creates a reference to a new or existing Topic under the given name,for a specific type, 
        /// with the desired QosPolicy settings and if applicable, attaches the optionally specified TopicListener to it.
        /// </summary>
        /// <param name="topicName">The name of the Topic to be created. A new Topic will only be created, when no Topic, 
        /// with the same name, is found within the DomainParticipant.</param>
        /// <param name="typeName">A local alias of the data type, which must have been registered before creating 
        /// the Topic.</param>
        /// <param name="qos">The collection of QosPolicy settings for the new Topic. In case these settings are not 
        /// self consistent, no Topic is created.</param>
        /// <param name="listener">The TopicListener instance which will be attached to the new Topic. 
        /// It is permitted to use null as the value of the listener: this behaves as a TopicListener whose operations
        /// perform no action.</param>
        /// <param name="mask">A bit-mask in which each bit enables the invocation of the TopicListener 
        /// for a certain status.</param>
        /// <returns>The new or existing Topic. In case of an error a Topic with a null value is returned.</returns>
        public ITopic CreateTopic(string topicName, string typeName, TopicQos qos, ITopicListener listener, StatusKind mask)
        {
            ITopic topic = null;

            using (TopicQosMarshaler marshaler = new TopicQosMarshaler())
            {
                if (marshaler.CopyIn(qos) == ReturnCode.Ok)
                {
                    if (listener != null)
                    {
                        OpenSplice.Gapi.gapi_topicListener gapiListener;
                        TopicListenerHelper listenerHelper = new TopicListenerHelper();
                        listenerHelper.Listener = listener;
                        listenerHelper.CreateListener(out gapiListener);
                        lock (listener)
                        {
                            using (TopicListenerMarshaler listenerMarshaler =
                                    new TopicListenerMarshaler(ref gapiListener))
                            {
                                IntPtr gapiPtr = Gapi.DomainParticipant.create_topic(
                                        GapiPeer,
                                        topicName,
                                        typeName,
                                        marshaler.GapiPtr,
                                        listenerMarshaler.GapiPtr,
                                        mask);
                                if (gapiPtr != IntPtr.Zero)
                                {
                                    topic = new Topic(gapiPtr, listenerHelper);
                                }
                            }
                        }
                    }
                    else
                    {
                        // Invoke the corresponding gapi function.
                        IntPtr gapiPtr = Gapi.DomainParticipant.create_topic(
                                GapiPeer,
                                topicName,
                                typeName,
                                marshaler.GapiPtr,
                                IntPtr.Zero,
                                mask);

                        if (gapiPtr != IntPtr.Zero)
                        {
                            topic = new Topic(gapiPtr);
                        }
                    }
                }
            }

            return topic;
        }

        /// <summary>
        /// This operation deletes a Topic
        /// </summary>
        /// <param name="t">The Topic which is to be deleted.</param>
        /// <returns>Return codes are:Ok,Error,BadParameter,AlreadyDeleted,OutOfResources or PreconditionNotMet.</returns>
        public ReturnCode DeleteTopic(ITopic t)
        {
            ReturnCode result = ReturnCode.BadParameter;

            Topic topic = t as Topic;
            if (topic != null)
            {
                result = Gapi.DomainParticipant.delete_topic(
                        GapiPeer,
                        topic.GapiPeer);
            }

            return result;
        }

        /// <summary>
        /// This operation gives access to an existing (or ready to exist) enabled Topic, based on its topicName.
        /// </summary>
        /// <param name="topicName">The name of te Topic that the application wants access to.</param>
        /// <param name="timeout">The maximum duration to block for the DomainParticipant FindTopic, 
        /// after which the application thread is unblocked. The special constant Duration Infinite can be used 
        /// when the maximum waiting time does not need to be bounded.</param>
        /// <returns>The Topic that has been found. If an error occurs the operation returns a Topic with a null value. </returns>
        public ITopic FindTopic(string topicName, Duration timeout)
        {
            IntPtr gapiPtr = Gapi.DomainParticipant.find_topic(
                    GapiPeer,
                    topicName,
                    ref timeout);

            ITopic topic = null;

            if (gapiPtr != IntPtr.Zero)
            {
                topic = new OpenSplice.Topic(gapiPtr);
            }

            return topic;
        }

        /// <summary>
        /// This operation gives access to a locally-created TopicDescription, with a matching name.
        /// </summary>
        /// <param name="name">The name of the TopicDescription to look for.</param>
        /// <returns>The TopicDescription it has found.If an error occurs the operation 
        /// returns a TopicDescription with a null value. </returns>
        public ITopicDescription LookupTopicDescription(string name)
        {
            ITopicDescription topicDesc = null;

            IntPtr gapiPtr = Gapi.DomainParticipant.lookup_topicdescription(
                    GapiPeer,
                    name);

            if (gapiPtr != IntPtr.Zero)
            {
                // if the lookup fails, then we don't have a managed object for gapi object yet
                topicDesc = SacsSuperClass.fromUserData(gapiPtr) as ITopicDescription;

                if (topicDesc == null)
                {
                    DDS.OpenSplice.OS.Report(
                            DDS.OpenSplice.ReportType.OS_ERROR,
                            "DDS.OpenSplice.DomainParticipant.LookupTopicDescription",
                            "DDS/OpenSplice/DomainParticipant.cs",
                            DDS.ErrorCode.EntityUnknown,
                            "TopicDescription Entity has no C# wrapper.");
                }
            }

            return topicDesc;
        }

        /// <summary>
        /// This operation creates a ContentFilteredTopic for a DomainParticipant in order to allow 
        /// DataReaders to subscribe to a subset of the topic content.
        /// </summary>
        /// <param name="name">The name of the ContentFilteredTopic.</param>
        /// <param name="relatedTopic">The base topic on which the filtering will be applied. Therefore, 
        /// a filtered topic is based onn an existing Topic.</param>
        /// <param name="filterExpression">The SQL expression (subset of SQL), which defines the filtering.</param>
        /// <param name="expressionParameters">A sequence of strings with the parameter value used in the SQL expression
        /// (i.e., the number of %n tokens in the expression).The number of values in expressionParameters 
        /// must be equal or greater than the highest referenced %n token in the filterExpression 
        /// (e.g. if %1 and %8 are used as parameter in the filterExpression, the expressionParameters should at least
        /// contain n+1 = 9 values)</param>
        /// <returns>The newly created ContentFilteredTopic. In case of an error a ContentFilteredTopic with a null
        /// value is returned.</returns>
        public IContentFilteredTopic CreateContentFilteredTopic(
                string name, 
                ITopic relatedTopic,
                string filterExpression, 
                params string[] expressionParameters)
        {
            IContentFilteredTopic contentFilteredTopic = null;

            Topic related = relatedTopic as Topic;
            if (relatedTopic != null)
            {
                using (SequenceStringMarshaler marshaler = new SequenceStringMarshaler())
                {
                    if (marshaler.CopyIn(expressionParameters) == DDS.ReturnCode.Ok)
                    {
                        IntPtr gapiPtr = Gapi.DomainParticipant.create_contentfilteredtopic(
                                GapiPeer,
                                name,
                                related.GapiPeer,
                                filterExpression,
                                marshaler.GapiPtr);
                        if (gapiPtr != IntPtr.Zero)
                        {
                            contentFilteredTopic = new ContentFilteredTopic(gapiPtr);
                        }
                    }

                }
            }

            return contentFilteredTopic;
        }

        /// <summary>
        /// This operation deletes a ContentFilteredTopic.
        /// </summary>
        /// <param name="t">The ContentFilteredTopic to be deleted.</param>
        /// <returns>Return codes are:Ok,Error,BadParameter,AlreadyDeleted,OutOfResources or PreconditionNotMet.</returns>
        public ReturnCode DeleteContentFilteredTopic(IContentFilteredTopic t)
        {
            ReturnCode result = ReturnCode.BadParameter;

            ContentFilteredTopic contentFilteredTopic = t as ContentFilteredTopic;
            if (contentFilteredTopic != null)
            {
                result = Gapi.DomainParticipant.delete_contentfilteredtopic(
                    GapiPeer,
                    contentFilteredTopic.GapiPeer);
            }

            return result;
        }

        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="typeName"></param>
        /// <param name="subscriptionExpression"></param>
        /// <param name="expressionParameters"></param>
        /// <returns></returns>
        public IMultiTopic CreateMultiTopic(
                string name, 
                string typeName,
                string subscriptionExpression, 
                params string[] expressionParameters)
        {
            IMultiTopic multiTopic = null;

            using (SequenceStringMarshaler marshaler = new SequenceStringMarshaler())
            {
                if (marshaler.CopyIn(expressionParameters) == DDS.ReturnCode.Ok)
                {
                    IntPtr gapiPtr = Gapi.DomainParticipant.create_multitopic(
                            GapiPeer,
                            name,
                            typeName,
                            subscriptionExpression,
                            marshaler.GapiPtr);

                    if (gapiPtr != IntPtr.Zero)
                    {
                        multiTopic = new MultiTopic(gapiPtr);
                    }
                }
            }

            return multiTopic;
        }

        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="t"></param>
        /// <returns></returns>
        public ReturnCode DeleteMultiTopic(IMultiTopic t)
        {
            ReturnCode result = ReturnCode.BadParameter;

            MultiTopic multiTopic = t as MultiTopic;
            if (multiTopic != null)
            {
                result = Gapi.DomainParticipant.delete_multitopic(
                        GapiPeer,
                        multiTopic.GapiPeer);
            }

            return result;
        }

        /// <summary>
        /// This operation deletes all the Entity objects that were created on the DomainParticipant.
        /// </summary>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources</returns>
        public ReturnCode DeleteContainedEntities()
        {
            return Gapi.DomainParticipant.delete_contained_entities(
                    GapiPeer,
                    null,
                    IntPtr.Zero);
        }

        /// <summary>
        /// This operation replaces the existing set of QosPolicy settings for a DomainParticipant.
        /// </summary>
        /// <param name="qos">New set of QosPolicy settings for the DomainParticipant.</param>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources</returns>
        public ReturnCode SetQos(DomainParticipantQos qos)
        {
            ReturnCode result;

            using (DomainParticipantQosMarshaler marshaler = new DomainParticipantQosMarshaler())
            {
                result = marshaler.CopyIn(qos);
                if (result == ReturnCode.Ok)
                {
                    result = Gapi.DomainParticipant.set_qos(
                            GapiPeer,
                            marshaler.GapiPtr);
                }
            }
            return result;
        }

        /// <summary>
        /// This operation allows access to the existing set of QoS policies for a DomainParticipant.
        /// </summary>
        /// <param name="qos">A reference to the destination DomainParticipantQos struct in which the 
        /// QosPolicy settings will be copied.</param>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode GetQos(ref DomainParticipantQos qos)
        {
            ReturnCode result;

            using (DomainParticipantQosMarshaler marshaler = new DomainParticipantQosMarshaler())
            {
                result = Gapi.DomainParticipant.get_qos(
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
        /// <param name="handle"></param>
        /// <returns></returns>
        public ReturnCode IgnoreParticipant(InstanceHandle handle)
        {
            return Gapi.DomainParticipant.ignore_participant(
                    GapiPeer,
                    handle);
        }

        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="handle"></param>
        /// <returns></returns>
        public ReturnCode IgnoreTopic(InstanceHandle handle)
        {
            return Gapi.DomainParticipant.ignore_topic(
                    GapiPeer,
                    handle);
        }

        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="handle"></param>
        /// <returns></returns>
        public ReturnCode IgnorePublication(InstanceHandle handle)
        {
            return Gapi.DomainParticipant.ignore_publication(
                    GapiPeer,
                    handle);
        }

        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="handle"></param>
        /// <returns></returns>
        public ReturnCode IgnoreSubscription(InstanceHandle handle)
        {
            return Gapi.DomainParticipant.ignore_subscription(
                    GapiPeer,
                    handle);
        }

        /// <summary>
        /// This property returns the DomainId of the Domain to which this DomainParticipant is attached.
        /// </summary>
        public string DomainId
        {
            get
            {
                IntPtr ptr = Gapi.DomainParticipant.get_domain_id(GapiPeer);
                string result = Marshal.PtrToStringAnsi(ptr);
                Gapi.GenericAllocRelease.Free(ptr);

                return result;
            }
        }

        /// <summary>
        /// This operation asserts the liveliness for the DomainParticipant.
        /// </summary>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode AssertLiveliness()
        {
            return Gapi.DomainParticipant.assert_liveliness(GapiPeer);
        }

        /// <summary>
        /// This operation sets the default PublisherQos of the DomainParticipant.
        /// </summary>
        /// <param name="qos">A collection of QosPolicy settings, which contains the new default QosPolicy 
        /// settings for the newly created Publishers.</param>
        /// <returns>Return codes are:Ok,Error,BadParameter,Unsupported,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode SetDefaultPublisherQos(PublisherQos qos)
        {
            ReturnCode result;

            using (PublisherQosMarshaler marshaler = new PublisherQosMarshaler())
            {
                result = marshaler.CopyIn(qos);
                if (result == ReturnCode.Ok)
                {
                    result = Gapi.DomainParticipant.set_default_publisher_qos(
                            GapiPeer,
                            marshaler.GapiPtr);
                }
            }
            return result;
        }

        /// <summary>
        /// This operation gets the struct with the default Publisher QosPolicy settings of the DomainParticipant.
        /// </summary>
        /// <param name="qos">A reference to the PublisherQos struct (provided by the application) in which 
        /// the default QosPolicy settings for the Publisher are written.</param>
        /// <returns>Return codes are: Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode GetDefaultPublisherQos(ref PublisherQos qos)
        {
            ReturnCode result;

            using (PublisherQosMarshaler marshaler = new PublisherQosMarshaler())
            {
                result = Gapi.DomainParticipant.get_default_publisher_qos(
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
        /// This operation sets the default SubscriberQos of the DomainParticipant.
        /// </summary>
        /// <param name="qos">A collection of QosPolicy settings, which contains the new default QosPolicy 
        /// settings for the newly created Subscribers.</param>
        /// <returns>Return codes are:Ok,Error,BadParameter,Unsupported,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode SetDefaultSubscriberQos(SubscriberQos qos)
        {
            ReturnCode result;

            using (SubscriberQosMarshaler marshaler = new SubscriberQosMarshaler())
            {
                result = marshaler.CopyIn(qos);
                if (result == ReturnCode.Ok)
                {
                    result = Gapi.DomainParticipant.set_default_subscriber_qos(
                            GapiPeer,
                            marshaler.GapiPtr);
                }
            }
            return result;
        }

        /// <summary>
        /// This operation gets the struct with the default Subscriber QosPolicy settings of the DomainParticipant.
        /// </summary>
        /// <param name="qos">a reference to the QosPolicy struct (provided by the application) in which the default 
        /// QosPolicy settings for the Subscriber is written</param>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode GetDefaultSubscriberQos(ref SubscriberQos qos)
        {
            ReturnCode result;

            using (SubscriberQosMarshaler marshaler = new SubscriberQosMarshaler())
            {
                result = Gapi.DomainParticipant.get_default_subscriber_qos(
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
        /// This operation sets the default TopicQos of the DomainParticipant.
        /// </summary>
        /// <param name="qos">a collection of QosPolicy settings, which contains the new default QosPolicy settings 
        /// for the newly created Topics.</param>
        /// <returns>Return codes are:Ok,Error,BadParameter,Unsupported,AlreadyDeleted,OutOfResources or 
        /// InconsistentPolicy</returns>
        public ReturnCode SetDefaultTopicQos(TopicQos qos)
        {
            ReturnCode result;

            using (TopicQosMarshaler marshaler = new TopicQosMarshaler())
            {
                result = marshaler.CopyIn(qos);
                if (result == ReturnCode.Ok)
                {
                    result = Gapi.DomainParticipant.set_default_topic_qos(
                            GapiPeer,
                            marshaler.GapiPtr);
                }
            }

            return result;
        }

        /// <summary>
        /// This operation gets the struct with the default Topic QosPolicy settings of the DomainParticipant.
        /// </summary>
        /// <param name="qos">A reference to the QosPolicy struct (provided by the application) in which the 
        /// default QosPolicy settings for the Topic is written.</param>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode GetDefaultTopicQos(ref TopicQos qos)
        {
            ReturnCode result;

            using (TopicQosMarshaler marshaler = new TopicQosMarshaler())
            {
                result = Gapi.DomainParticipant.get_default_topic_qos(
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
        /// This operation checks whether or not the given Entity represented by handle is created by the 
        /// DomainParticipant or any of its contained entities.
        /// </summary>
        /// <param name="handle">An Entity in the Data Distribution Service.</param>
        /// <returns>true if handle represents an Entity that is created by the DomainParticipant or any of its
        /// contained Entities. Otherwise the return value is false.</returns>
        public bool ContainsEntity(InstanceHandle handle)
        {
            byte result = Gapi.DomainParticipant.contains_entity(
                    GapiPeer,
                    handle);
            return result != 0;
        }

        /// <summary>
        /// This operation returns the value of the current time that the Data Distribution Service 
        /// uses to time-stamp written data as well as received data in current_time.
        /// </summary>
        /// <param name="currentTime">stores the value of currentTime as used by the Data Distribution Service. 
        /// The input value of currentTime is ignored.</param>
        /// <returns>Return codes are:Ok,Error,BadParameter,AlreadyDeleted,OutOfResources or NotEnabled.</returns>
        public ReturnCode GetCurrentTime(out Time currentTime)
        {
            return Gapi.DomainParticipant.get_current_time(
                    GapiPeer,
                    out currentTime);
        }


        public ITypeSupport GetTypeSupport(string registeredName)
        {
            ITypeSupport typeSupport = null;

            IntPtr gapiPtr = Gapi.DomainParticipant.get_typesupport(
                    GapiPeer,
                    registeredName);

            if (gapiPtr != IntPtr.Zero)
            {
                typeSupport = SacsSuperClass.fromUserData(gapiPtr) as ITypeSupport;
            }

            return typeSupport;
        }


        public ITypeSupport LookupTypeSupport(string registeredTypeName)
        {
            ITypeSupport typeSupport = null;

            IntPtr gapiPtr = Gapi.DomainParticipant.lookup_typesupport(
                GapiPeer,
                registeredTypeName);

            if (gapiPtr != IntPtr.Zero)
            {
                typeSupport = SacsSuperClass.fromUserData(gapiPtr) as ITypeSupport;
            }

            return typeSupport;
        }

        // only for API implementors
        internal IntPtr GetTypeMetaDescription(string typeName)
        {
            return Gapi.DomainParticipant.get_type_metadescription(
                    GapiPeer,
                    typeName);
        }
    }
}
