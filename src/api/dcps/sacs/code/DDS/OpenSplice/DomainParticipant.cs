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
using DDS;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate bool CopyInstanceHandleDelegate(IntPtr instance, IntPtr arg);

    public class DomainParticipant : Entity, IDomainParticipant
    {
        private readonly DomainParticipantListenerHelper listenerHelper;
        private CopyInstanceHandleDelegate CopyInstanceHandleAttr;
        private DatabaseMarshaler ParticipantDataMarshaler;
        private DatabaseMarshaler TopicDataMarshaler;

        /**
         * Constructor is only called by DDS.DomainParticipantFactory.
         */
        internal DomainParticipant(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            listenerHelper = new DomainParticipantListenerHelper();
            BuiltinTopicRegisterTypeSupport(this);
            CopyInstanceHandleAttr = CopyInstanceHandle;
        }

        /**
         * Constructor is only called by DDS.DomainParticipantFactory.
         */
        internal DomainParticipant(IntPtr gapiPtr, DomainParticipantListenerHelper listenerHelper)
            : base(gapiPtr)
        {
            this.listenerHelper = listenerHelper;
            BuiltinTopicRegisterTypeSupport(this);
        }

        /**
         * Register the four builtin topics:
         *     "DDS::ParticipantBuiltinTopicData"
         *     "DDS::TopicBuiltinTopicData"
         *     "DDS::PublicationBuiltinTopicData"
         *     "DDS::SubscriptionBuiltinTopicData"
         */
        internal static ReturnCode BuiltinTopicRegisterTypeSupport(DomainParticipant participant)
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
            } else {
               participant.ParticipantDataMarshaler = DatabaseMarshaler.GetMarshaler(
                       participant, DDSParticipant.TypeSpec);
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
            } else {
               participant.TopicDataMarshaler = DatabaseMarshaler.GetMarshaler(
                       participant, DDSTopic.TypeSpec);
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
         * Determine by name whether a topic is a builtin topic.
         */
        internal static string[] BuiltinTopicNames = {
                "DCPSParticipant",
                "DCPSTopic",
                "DCPSPublication",
                "DCPSSubscription" };

        internal static bool IsBuiltinTopic(string name)
        {
            bool result = false;
            for (int i = 0; i < BuiltinTopicNames.Length && !result; i++)
            {
                if (name == BuiltinTopicNames[i])
                {
                    result = true;
                }
            }
            return result;
        }

        /**
         * Wrap the selected builtin topics in a C# object.
         */
        internal Topic BuiltinTopicCreateWrapper(string biTopicName)
        {
            Topic wrapper = null;

            // Lookup the "DCPSParticipant" topic.
            IntPtr gapiPtr = Gapi.DomainParticipant.lookup_topicdescription(
                    this.GapiPeer, biTopicName);
            if (gapiPtr == IntPtr.Zero)
            {
                string msg = string.Format("Failed to wrap builtin topic: {0}", biTopicName);
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.DomainParticipant.builtinTopicCreateWrapper",
                        "DDS/OpenSplice/DomainParticipant.cs",
                        DDS.ErrorCode.Error,
                        msg);
            }
            else
            {
                // Wrap the gapi topic in a C# object.
                wrapper = new Topic(gapiPtr);
            }
            return wrapper;
        }

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
                using (PublisherDataWriterListenerMarshaler listenerMarshaler =
                        new PublisherDataWriterListenerMarshaler(ref gapiListener))
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

            if (publisher != null)
            {
                DomainParticipantQos dpQos = null;
                ReturnCode result = GetQos(ref dpQos);
                if (result == ReturnCode.Ok)
                {
                    if (dpQos.EntityFactory.AutoenableCreatedEntities)
                    {
                        publisher.Enable();
                    }
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
        /// <remarks>
        /// This operation creates a Publisher with the desired QosPolicy settings and if
        /// applicable, attaches the optionally specified PublisherListener to it. When the
        /// PublisherListener is not applicable, the NULL pointer must be supplied instead.
        /// To delete the Publisher the operation DeletePublisher or
        /// DeleteContainedEntities must be used.
        /// In case the specified QosPolicy settings are not consistent, no Publisher is
        /// created and the NULL pointer is returned.
        /// </remarks>
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

            using (OpenSplice.CustomMarshalers.PublisherQosMarshaler marshaler =
                    new OpenSplice.CustomMarshalers.PublisherQosMarshaler())
            {
                // Note: we use the same gapi lister as the DataWriter since the
                // publisher doesn't add anything unique
                if (marshaler.CopyIn(qos) == ReturnCode.Ok)
                {
                    if (listener != null)
                    {
                        Gapi.gapi_publisherDataWriterListener gapiListener;
                        PublisherDataWriterListenerHelper listenerHelper =
                                new PublisherDataWriterListenerHelper();
                        listenerHelper.Listener = listener;
                        listenerHelper.CreateListener(out gapiListener);
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

            if (publisher != null)
            {
                DomainParticipantQos dpQos = null;
                ReturnCode result = GetQos(ref dpQos);
                if (result == ReturnCode.Ok)
                {
                    if (dpQos.EntityFactory.AutoenableCreatedEntities)
                    {
                        publisher.Enable();
                    }
                }
            }

            return publisher;
        }

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

            if (subscriber != null)
            {
                DomainParticipantQos dpQos = null;
                ReturnCode result = GetQos(ref dpQos);
                if (result == ReturnCode.Ok)
                {
                    if (dpQos.EntityFactory.AutoenableCreatedEntities)
                    {
                        subscriber.Enable();
                    }
                }
            }

            return subscriber;
        }

        public ISubscriber CreateSubscriber(SubscriberQos qos)
        {
            return CreateSubscriber(qos, null, 0);
        }

        public ISubscriber CreateSubscriber(SubscriberQos qos, ISubscriberListener listener, StatusKind mask)
        {
            ISubscriber subscriber = null;

            using (OpenSplice.CustomMarshalers.SubscriberQosMarshaler marshaler =
                    new OpenSplice.CustomMarshalers.SubscriberQosMarshaler())
            {
                if (marshaler.CopyIn(qos) == ReturnCode.Ok)
                {
                    if (listener != null)
                    {
                        OpenSplice.Gapi.gapi_subscriberListener gapiListener;
                        SubscriberListenerHelper listenerHelper = new SubscriberListenerHelper();
                        listenerHelper.Listener = listener;
                        listenerHelper.CreateListener(out gapiListener);
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

            if (subscriber != null)
            {
                DomainParticipantQos dpQos = null;
                ReturnCode result = GetQos(ref dpQos);
                if (result == ReturnCode.Ok)
                {
                    if (dpQos.EntityFactory.AutoenableCreatedEntities)
                    {
                        subscriber.Enable();
                    }
                }
            }

            return subscriber;
        }

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
                        TypeSupportFactory tsFactory = GetTypeSupportFactory("DDS::TopicBuiltinTopicData");
                        tsFactory.CreateDataReader(reader_gapiPtr);
                        IntPtr topic_gapiPtr = Gapi.DataReader.get_topicdescription(reader_gapiPtr);
                        if (topic_gapiPtr != IntPtr.Zero)
                        {
                            new OpenSplice.Topic(topic_gapiPtr);
                        }
                    }
                    reader_gapiPtr = Gapi.Subscriber.lookup_datareader(subscriber_gapiPtr, "DCPSParticipant");
                    if (reader_gapiPtr != IntPtr.Zero){
                        TypeSupportFactory tsFactory = GetTypeSupportFactory("DDS::ParticipantBuiltinTopicData");
                        tsFactory.CreateDataReader(reader_gapiPtr);
                        IntPtr topic_gapiPtr = Gapi.DataReader.get_topicdescription(reader_gapiPtr);
                        if (topic_gapiPtr != IntPtr.Zero)
                        {
                            new OpenSplice.Topic(topic_gapiPtr);
                        }
                    }
                    reader_gapiPtr = Gapi.Subscriber.lookup_datareader(subscriber_gapiPtr, "DCPSPublication");
                    if (reader_gapiPtr != IntPtr.Zero){
                        TypeSupportFactory tsFactory = GetTypeSupportFactory("DDS::PublicationBuiltinTopicData");
                        tsFactory.CreateDataReader(reader_gapiPtr);
                        IntPtr topic_gapiPtr = Gapi.DataReader.get_topicdescription(reader_gapiPtr);
                        if (topic_gapiPtr != IntPtr.Zero)
                        {
                            new OpenSplice.Topic(topic_gapiPtr);
                        }
                    }
                    reader_gapiPtr = Gapi.Subscriber.lookup_datareader(subscriber_gapiPtr, "DCPSSubscription");
                    if (reader_gapiPtr != IntPtr.Zero){
                        TypeSupportFactory tsFactory = GetTypeSupportFactory("DDS::SubscriptionBuiltinTopicData");
                        tsFactory.CreateDataReader(reader_gapiPtr);
                        IntPtr topic_gapiPtr = Gapi.DataReader.get_topicdescription(reader_gapiPtr);
                        if (topic_gapiPtr != IntPtr.Zero)
                        {
                            new OpenSplice.Topic(topic_gapiPtr);
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

        public ITopic CreateTopic(
                string topicName,
                string typeName,
                ITopicListener listener,
                StatusKind mask)
        {
            ITopic topic = null;

            if (listener != null)
            {
                OpenSplice.Gapi.gapi_topicListener gapiListener;
                TopicListenerHelper listenerHelper = new TopicListenerHelper();
                listenerHelper.Listener = listener;
                listenerHelper.CreateListener(out gapiListener);
                using (TopicListenerMarshaler listenerMarshaler =
                        new TopicListenerMarshaler(ref gapiListener))
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

            if (topic != null)
            {
                DomainParticipantQos dpQos = null;
                ReturnCode result = GetQos(ref dpQos);
                if (result == ReturnCode.Ok)
                {
                    if (dpQos.EntityFactory.AutoenableCreatedEntities)
                    {
                        topic.Enable();
                    }
                }
            }

            return topic;
        }

        public ITopic CreateTopic(string topicName, string typeName, TopicQos qos)
        {
            return CreateTopic(topicName, typeName, qos, null, 0);
        }

        public ITopic CreateTopic(
                string topicName,
                string typeName,
                TopicQos qos,
                ITopicListener listener,
                StatusKind mask)
        {
            ITopic topic = null;

            using (OpenSplice.CustomMarshalers.TopicQosMarshaler marshaler =
                    new OpenSplice.CustomMarshalers.TopicQosMarshaler())
            {
                if (marshaler.CopyIn(qos) == ReturnCode.Ok)
                {
                    if (listener != null)
                    {
                        OpenSplice.Gapi.gapi_topicListener gapiListener;
                        TopicListenerHelper listenerHelper = new TopicListenerHelper();
                        listenerHelper.Listener = listener;
                        listenerHelper.CreateListener(out gapiListener);
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

            if (topic != null)
            {
                DomainParticipantQos dpQos = null;
                ReturnCode result = GetQos(ref dpQos);
                if (result == ReturnCode.Ok)
                {
                    if (dpQos.EntityFactory.AutoenableCreatedEntities)
                    {
                        topic.Enable();
                    }
                }
            }

            return topic;
        }

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
                    if (IsBuiltinTopic(name))
                    {
                        topicDesc = BuiltinTopicCreateWrapper(name);
                    } else {
                        DDS.OpenSplice.OS.Report(
                                DDS.OpenSplice.ReportType.OS_ERROR,
                                "DDS.OpenSplice.DomainParticipant.LookupTopicDescription",
                                "DDS/OpenSplice/DomainParticipant.cs",
                                DDS.ErrorCode.EntityUnknown,
                                "TopicDescription Entity has no C# wrapper.");
                    }
                }
            }

            return topicDesc;
        }

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

        public ReturnCode DeleteContainedEntities()
        {
            return Gapi.DomainParticipant.delete_contained_entities(GapiPeer);
        }

        public ReturnCode SetQos(DomainParticipantQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler marshaler =
                    new OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler())
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

        public ReturnCode GetQos(ref DomainParticipantQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler marshaler =
                    new OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler())
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

        public ReturnCode IgnoreParticipant(InstanceHandle handle)
        {
            return Gapi.DomainParticipant.ignore_participant(
                    GapiPeer,
                    handle);
        }

        public ReturnCode IgnoreTopic(InstanceHandle handle)
        {
            return Gapi.DomainParticipant.ignore_topic(
                    GapiPeer,
                    handle);
        }

        public ReturnCode IgnorePublication(InstanceHandle handle)
        {
            return Gapi.DomainParticipant.ignore_publication(
                    GapiPeer,
                    handle);
        }

        public ReturnCode IgnoreSubscription(InstanceHandle handle)
        {
            return Gapi.DomainParticipant.ignore_subscription(
                    GapiPeer,
                    handle);
        }


        public DomainId DomainId
        {
            get
            {
                DomainId result = Gapi.DomainParticipant.get_domain_id(GapiPeer);
                return result;
            }
        }

        public ReturnCode AssertLiveliness()
        {
            return Gapi.DomainParticipant.assert_liveliness(GapiPeer);
        }

        public ReturnCode SetDefaultPublisherQos(PublisherQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.PublisherQosMarshaler marshaler =
                    new OpenSplice.CustomMarshalers.PublisherQosMarshaler())
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

        public ReturnCode GetDefaultPublisherQos(ref PublisherQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.PublisherQosMarshaler marshaler =
                    new OpenSplice.CustomMarshalers.PublisherQosMarshaler())
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

        public ReturnCode SetDefaultSubscriberQos(SubscriberQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.SubscriberQosMarshaler marshaler =
                    new OpenSplice.CustomMarshalers.SubscriberQosMarshaler())
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

        public ReturnCode GetDefaultSubscriberQos(ref SubscriberQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.SubscriberQosMarshaler marshaler =
                    new OpenSplice.CustomMarshalers.SubscriberQosMarshaler())
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

        public ReturnCode SetDefaultTopicQos(TopicQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.TopicQosMarshaler marshaler =
                    new OpenSplice.CustomMarshalers.TopicQosMarshaler())
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

        public ReturnCode GetDefaultTopicQos(ref TopicQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.TopicQosMarshaler marshaler =
                    new OpenSplice.CustomMarshalers.TopicQosMarshaler())
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

        internal class InstanceHandleUserData
        {
            internal InstanceHandleUserData(int Index, InstanceHandle[] Seq)
            {
                this.Index = Index;
                this.Seq = Seq;
            }

            internal int Index;
            internal InstanceHandle[] Seq;
        }

        bool
        CopyInstanceHandle(IntPtr instance, IntPtr arg)
        {
            bool result = true;
            InstanceHandle ghandle;
            uint length;
            GCHandle tmpGCHandleData = GCHandle.FromIntPtr(arg);
            InstanceHandleUserData a = tmpGCHandleData.Target as InstanceHandleUserData;

            if (a.Index == 0) {
                length = Kernel.DataReaderInstance.GetNotEmptyInstanceCount(instance);
                if (a.Seq == null || length != a.Seq.Length) {
                    a.Seq = new InstanceHandle[length]; /* potentially reallocate */
                }
            }

            ghandle = Kernel.InstanceHandle.New(instance);
            if (a.Index < a.Seq.Length) {
                a.Seq[a.Index++] = ghandle;
            } else {
                /* error index out of bounds */
            }

            return result;
        }

        public ReturnCode GetDiscoveredParticipants (ref InstanceHandle[] participantHandles)
        {
            ReturnCode result;
            InstanceHandleUserData a = new InstanceHandleUserData(0, participantHandles);
            GCHandle iHUserDataHandle = GCHandle.Alloc(a, GCHandleType.Normal);

            result = (ReturnCode)  Gapi.DomainParticipant.get_discovered_participants(
                    GapiPeer,
                    CopyInstanceHandleAttr,
                    GCHandle.ToIntPtr(iHUserDataHandle));

            participantHandles = a.Seq;
            iHUserDataHandle.Free();

            return result;
        }

        public ReturnCode GetDiscoveredParticipantData (ref ParticipantBuiltinTopicData data, InstanceHandle handle)
        {
            ReturnCode result;
            GCHandle dataHandle = GCHandle.Alloc(data, GCHandleType.Normal);

            result = (ReturnCode)  Gapi.DomainParticipant.get_discovered_participant_data(
                    GapiPeer,
                    GCHandle.ToIntPtr(dataHandle),
                    handle,
                    ParticipantDataMarshaler.CopyOutDelegate);

            data = dataHandle.Target as ParticipantBuiltinTopicData;
            dataHandle.Free();

            return result;
        }
        
        public ReturnCode GetDiscoveredTopics (ref InstanceHandle[] topicHandles)
        {
            ReturnCode result;
            InstanceHandleUserData a = new InstanceHandleUserData(0, topicHandles);
            GCHandle iHUserDataHandle = GCHandle.Alloc(a, GCHandleType.Normal);

            result = (ReturnCode)  Gapi.DomainParticipant.get_discovered_topics(
                    GapiPeer,
                    CopyInstanceHandleAttr,
                    GCHandle.ToIntPtr(iHUserDataHandle));

            topicHandles = a.Seq;
            iHUserDataHandle.Free();

            return result;
        }

        public ReturnCode GetDiscoveredTopicData (ref TopicBuiltinTopicData data, InstanceHandle handle)
        {
            ReturnCode result;
            GCHandle dataHandle = GCHandle.Alloc(data, GCHandleType.Normal);

            result = (ReturnCode)  Gapi.DomainParticipant.get_discovered_topic_data(
                    GapiPeer,
                    GCHandle.ToIntPtr(dataHandle),
                    handle,
                    TopicDataMarshaler.CopyOutDelegate);

            data = dataHandle.Target as TopicBuiltinTopicData;
            dataHandle.Free();

            return result;
        }

        public bool ContainsEntity(InstanceHandle handle)
        {
            byte result = Gapi.DomainParticipant.contains_entity(
                    GapiPeer,
                    handle);
            return result != 0;
        }

        public ReturnCode GetCurrentTime(out Time currentTime)
        {
            return Gapi.DomainParticipant.get_current_time(
                    GapiPeer,
                    out currentTime);
        }

        internal TypeSupportFactory GetTypeSupportFactory(string registeredName)
        {
            TypeSupportFactory tsFactory = null;

            IntPtr gapiPtr = Gapi.DomainParticipant.get_typesupport(
                    GapiPeer,
                    registeredName);

            if (gapiPtr != IntPtr.Zero)
            {
                tsFactory = TypeSupportFactory.fromUserData(gapiPtr);
            }

            return tsFactory;
        }

        public ITypeSupport GetTypeSupport(string registeredTypeName)
        {
            ITypeSupport typeSupport = null;

            IntPtr gapiPtr = Gapi.DomainParticipant.get_typesupport(
                GapiPeer,
                registeredTypeName);

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
