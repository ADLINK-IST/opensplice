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
    internal class DomainParticipant : Entity, IDomainParticipant
    {
        private readonly DomainParticipantListenerHelper listenerHelper;

        internal DomainParticipant(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
            listenerHelper = new DomainParticipantListenerHelper();
        }

        internal DomainParticipant(IntPtr gapiPtr, DomainParticipantListenerHelper listenerHelper)
            : base(gapiPtr)
        {
            // Base class handles everything.
            this.listenerHelper = listenerHelper;
        }

        public ReturnCode SetListener(IDomainParticipantListener listener, StatusKind mask)
        {
            ReturnCode result = ReturnCode.Error;

            Gapi.gapi_domainParticipantListener gapiListener;
            listenerHelper.Listener = listener;
            listenerHelper.CreateListener(out gapiListener);

            using (DomainParticipantListenerMarshaler marshaler = new DomainParticipantListenerMarshaler(ref gapiListener))
            {
                result = Gapi.DomainParticipant.set_listener(
                    GapiPeer,
                    marshaler.GapiPtr,
                    mask);
            }

            return result;
        }

        public IPublisher CreatePublisher()
        {
            IPublisher publisher = null;
            IntPtr gapiPtr = Gapi.DomainParticipant.create_publisher(
                GapiPeer,
                IntPtr.Zero,
                IntPtr.Zero,
                StatusKind.Any);
            if (gapiPtr != IntPtr.Zero)
            {
                publisher = new Publisher(gapiPtr);
            }
            return publisher;
        }

        public IPublisher CreatePublisher(IPublisherListener listener, StatusKind mask)
        {
            // just in case they use this with null
            if (listener == null)
            {
                return CreatePublisher();
            }

            IPublisher publisher = null;

            // Note: we use the same gapi lister as the DataWriter since the
            // publisher doesn't add anything unique
            OpenSplice.Gapi.gapi_publisherDataWriterListener gapiListener;
            PublisherDataWriterListenerHelper listenerHelper = new PublisherDataWriterListenerHelper();
            listenerHelper.Listener = listener;
            listenerHelper.CreateListener(out gapiListener);
            using (PublisherDataWriterListenerMarshaler listenerMarshaler = new PublisherDataWriterListenerMarshaler(ref gapiListener))
            {
                IntPtr gapiPtr = Gapi.DomainParticipant.create_publisher(
                    GapiPeer,
                    IntPtr.Zero,
                    listenerMarshaler.GapiPtr,
                    mask);
                if (gapiPtr != IntPtr.Zero)
                {
                    publisher = new Publisher(gapiPtr, listenerHelper);
                }
            }

            return publisher;
        }

        public IPublisher CreatePublisher(ref PublisherQos qos)
        {
            IPublisher publisher = null;

            using (IMarshaler marshaler = new PublisherQosMarshaler(ref qos))
            {
                IntPtr gapiPtr = Gapi.DomainParticipant.create_publisher(
                    GapiPeer,
                    marshaler.GapiPtr,
                    IntPtr.Zero,
                    0);
                if (gapiPtr != IntPtr.Zero)
                {
                    publisher = new Publisher(gapiPtr);
                }
            }

            return publisher;
        }

        public IPublisher CreatePublisher(ref PublisherQos qos, IPublisherListener listener, StatusKind mask)
        {
            // just in case they use this with null
            if (listener == null)
            {
                return CreatePublisher(ref qos);
            }

            IPublisher publisher = null;

            using (IMarshaler marshaler = new PublisherQosMarshaler(ref qos))
            {
                // Note: we use the same gapi lister as the DataWriter since the
                // publisher doesn't add anything unique
                OpenSplice.Gapi.gapi_publisherDataWriterListener gapiListener;
                PublisherDataWriterListenerHelper listenerHelper = new PublisherDataWriterListenerHelper();
                listenerHelper.Listener = listener;
                listenerHelper.CreateListener(out gapiListener);
                using (PublisherDataWriterListenerMarshaler listenerMarshaler = new PublisherDataWriterListenerMarshaler(ref gapiListener))
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

            return publisher;
        }

        public ReturnCode DeletePublisher(IPublisher p)
        {
            Publisher publisher = (Publisher)p;
            ReturnCode result = OpenSplice.Gapi.DomainParticipant.delete_publisher(
                GapiPeer,
                publisher.GapiPeer);

            return result;
        }

        public ISubscriber CreateSubscriber()
        {
            ISubscriber subscriber = null;
            IntPtr gapiPtr = Gapi.DomainParticipant.create_subscriber(
                GapiPeer,
                IntPtr.Zero,
                IntPtr.Zero,
                StatusKind.Any);
            if (gapiPtr != IntPtr.Zero)
            {
                subscriber = new Subscriber(gapiPtr);
            }
            return subscriber;
        }

        public ISubscriber CreateSubscriber(ISubscriberListener listener, StatusKind mask)
        {
            // just in case they use this with null
            if (listener == null)
            {
                return CreateSubscriber();
            }

            ISubscriber subscriber = null;

            OpenSplice.Gapi.gapi_subscriberListener gapiListener;
            SubscriberListenerHelper listenerHelper = new SubscriberListenerHelper();
            listenerHelper.Listener = listener;
            listenerHelper.CreateListener(out gapiListener);
            using (SubscriberListenerMarshaler listenerMarshaler = new SubscriberListenerMarshaler(ref gapiListener))
            {
                IntPtr gapiPtr = Gapi.DomainParticipant.create_subscriber(
                    GapiPeer,
                    IntPtr.Zero,
                    listenerMarshaler.GapiPtr,
                    mask);
                if (gapiPtr != IntPtr.Zero)
                {
                    subscriber = new Subscriber(gapiPtr, listenerHelper);
                }
            }

            return subscriber;
        }

        public ISubscriber CreateSubscriber(ref SubscriberQos qos)
        {
            ISubscriber subscriber = null;

            using (IMarshaler marshaler = new SubscriberQosMarshaler(ref qos))
            {
                IntPtr gapiPtr = Gapi.DomainParticipant.create_subscriber(
                    GapiPeer,
                    marshaler.GapiPtr,
                    IntPtr.Zero,
                    0);
                if (gapiPtr != IntPtr.Zero)
                {
                    subscriber = new Subscriber(gapiPtr);
                }
            }
            return subscriber;
        }

        public ISubscriber CreateSubscriber(ref SubscriberQos qos, ISubscriberListener listener, StatusKind mask)
        {
            // just in case they use this with null
            if (listener == null)
            {
                return CreateSubscriber(ref qos);
            }

            ISubscriber subscriber = null;

            using (IMarshaler marshaler = new SubscriberQosMarshaler(ref qos))
            {
                OpenSplice.Gapi.gapi_subscriberListener gapiListener;
                SubscriberListenerHelper listenerHelper = new SubscriberListenerHelper();
                listenerHelper.Listener = listener;
                listenerHelper.CreateListener(out gapiListener);
                using (SubscriberListenerMarshaler listenerMarshaler = new SubscriberListenerMarshaler(ref gapiListener))
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

            return subscriber;
        }


        public ReturnCode DeleteSubscriber(ISubscriber s)
        {
            Subscriber subscriber = (Subscriber)s;
            return OpenSplice.Gapi.DomainParticipant.delete_subscriber(
                GapiPeer,
                subscriber.GapiPeer);
        }

        public ISubscriber BuiltInSubscriber
        {
            get
            {
                IntPtr gapiPtr = OpenSplice.Gapi.DomainParticipant.get_builtin_subscriber(GapiPeer);
                ISubscriber subscriber = SacsSuperClass.fromUserData(gapiPtr) as ISubscriber;

                // TODO: Verify the assumption that there is only one gapiptr for builtin_subscriber

                // TODO: Verify whether this is needed:

                // if the lookup fails, then we don't have a managed object for subscriber yet
                if (gapiPtr != IntPtr.Zero && subscriber == null)
                {
                    subscriber = new Subscriber(gapiPtr);
                }

                return subscriber;
            }
        }

        public ITopic CreateTopic(string topicName, string typeName)
        {
            ITopic topic = null;

            IntPtr gapiPtr = OpenSplice.Gapi.DomainParticipant.create_topic(
                GapiPeer,
                topicName,
                typeName,
                IntPtr.Zero,
                IntPtr.Zero,
                StatusKind.Any);

            if (gapiPtr != IntPtr.Zero)
            {
                topic = new Topic(gapiPtr);
            }

            return topic;
        }

        public ITopic CreateTopic(string topicName, string typeName,
            ITopicListener listener, StatusKind mask)
        {
            // just in case they use this with null
            if (listener == null)
            {
                return CreateTopic(topicName, typeName);
            }

            ITopic topic = null;

            OpenSplice.Gapi.gapi_topicListener gapiListener;
            TopicListenerHelper listenerHelper = new TopicListenerHelper();
            listenerHelper.Listener = listener;
            listenerHelper.CreateListener(out gapiListener);
            using (TopicListenerMarshaler listenerMarshaler = new TopicListenerMarshaler(ref gapiListener))
            {
                IntPtr gapiPtr = Gapi.DomainParticipant.create_topic(
                    GapiPeer,
                    topicName,
                    typeName,
                    IntPtr.Zero,
                    listenerMarshaler.GapiPtr,
                    mask);
                if (gapiPtr != IntPtr.Zero)
                {
                    topic = new Topic(gapiPtr, listenerHelper);
                }
            }

            return topic;
        }

        public ITopic CreateTopic(string topicName, string typeName, ref TopicQos qos)
        {
            ITopic topic = null;

            using (IMarshaler marshaler = new TopicQosMarshaler(ref qos))
            {
                IntPtr gapiPtr = OpenSplice.Gapi.DomainParticipant.create_topic(
                    GapiPeer,
                    topicName,
                    typeName,
                    marshaler.GapiPtr,
                    IntPtr.Zero,
                    StatusKind.Any);

                if (gapiPtr != IntPtr.Zero)
                {
                    topic = new Topic(gapiPtr);
                }
            }
            return topic;
        }

        public ITopic CreateTopic(string topicName, string typeName, ref TopicQos qos,
            ITopicListener listener, StatusKind mask)
        {
            // just in case they use this with null
            if (listener == null)
            {
                return CreateTopic(topicName, typeName, ref qos);
            }

            ITopic topic = null;

            using (IMarshaler marshaler = new TopicQosMarshaler(ref qos))
            {
                OpenSplice.Gapi.gapi_topicListener gapiListener;
                TopicListenerHelper listenerHelper = new TopicListenerHelper();
                listenerHelper.Listener = listener;
                listenerHelper.CreateListener(out gapiListener);
                using (TopicListenerMarshaler listenerMarshaler = new TopicListenerMarshaler(ref gapiListener))
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

            return topic;
        }

        public ReturnCode DeleteTopic(ITopic t)
        {
            Topic topic = (Topic)t;
            return OpenSplice.Gapi.DomainParticipant.delete_topic(
                GapiPeer,
                topic.GapiPeer);
        }

        public ITopic FindTopic(string topicName, Duration timeout)
        {
            IntPtr gapiPtr = OpenSplice.Gapi.DomainParticipant.find_topic(
                GapiPeer,
                topicName,
                ref timeout);

            ITopic topic = SacsSuperClass.fromUserData(gapiPtr) as ITopic;

            return topic;
        }

        public ITopicDescription LookupTopicDescription(string name)
        {
            ITopicDescription topicDesc = null;

            IntPtr gapiPtr = OpenSplice.Gapi.DomainParticipant.lookup_topicdescription(
                GapiPeer,
                name);

            if (gapiPtr != IntPtr.Zero)
            {
                topicDesc = SacsSuperClass.fromUserData(gapiPtr) as ITopicDescription;

                // if the lookup fails, then we don't have a managed object for gapi object yet
                if (topicDesc == null)
                {
                    topicDesc = new TopicDescription(gapiPtr);
                }
            }

            return topicDesc;
        }

        public IContentFilteredTopic CreateContentFilteredTopic(string name, ITopic relatedTopic,
            string filterExpression, params string[] expressionParameters)
        {
            IContentFilteredTopic contentFilteredTopic = null;

            Topic related = (Topic)relatedTopic;

            using (IMarshaler marshaler = new SequenceStringMarshaler(expressionParameters))
            {
                IntPtr gapiPtr = OpenSplice.Gapi.DomainParticipant.create_contentfilteredtopic(
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

            return contentFilteredTopic;
        }

        public ReturnCode DeleteContentFilteredTopic(IContentFilteredTopic t)
        {
            ContentFilteredTopic contentFilteredTopic = (ContentFilteredTopic)t;
            return OpenSplice.Gapi.DomainParticipant.delete_contentfilteredtopic(
                GapiPeer,
                contentFilteredTopic.GapiPeer);
        }


        public IMultiTopic CreateMultiTopic(string name, ITypeSupport typeSupport,
            string subscriptionExpression, params string[] expressionParameters)
        {
            TypeSupport typeSupportObj = (TypeSupport)typeSupport;
            IMultiTopic multiTopic = null;

            using (IMarshaler marshaler = new SequenceStringMarshaler(expressionParameters))
            {
                IntPtr gapiPtr = OpenSplice.Gapi.DomainParticipant.create_multitopic(
                    GapiPeer,
                    name,
                    typeSupport.TypeName,
                    subscriptionExpression,
                    marshaler.GapiPtr);

                if (gapiPtr != IntPtr.Zero)
                {
                    multiTopic = new MultiTopic(gapiPtr);
                }
            }

            return multiTopic;
        }

        public ReturnCode DeleteMultiTopic(IMultiTopic t)
        {
            MultiTopic multiTopic = (MultiTopic)t;
            return OpenSplice.Gapi.DomainParticipant.delete_multitopic(
                GapiPeer,
                multiTopic.GapiPeer);
        }

        public ReturnCode DeleteContainedEntities()
        {
            return OpenSplice.Gapi.DomainParticipant.delete_contained_entities(
                GapiPeer,
                null,
                IntPtr.Zero);
        }

        public ReturnCode SetQos(ref DomainParticipantQos qos)
        {
            using (IMarshaler marshaler = new DomainParticipantQosMarshaler(ref qos))
            {
                return OpenSplice.Gapi.DomainParticipant.set_qos(
                GapiPeer,
                marshaler.GapiPtr);
            }
        }

        public ReturnCode GetQos(out DomainParticipantQos qos)
        {
            qos = new DomainParticipantQos();
            ReturnCode result = ReturnCode.Error;

            using (DomainParticipantQosMarshaler marshaler = new DomainParticipantQosMarshaler())
            {
                result = OpenSplice.Gapi.DomainParticipant.get_qos(
                GapiPeer,
                marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(out qos);
                }
            }

            return result;
        }

        public ReturnCode IgnoreParticipant(InstanceHandle handle)
        {
            return OpenSplice.Gapi.DomainParticipant.ignore_participant(
                GapiPeer,
                handle);
        }

        public ReturnCode IgnoreTopic(InstanceHandle handle)
        {
            return OpenSplice.Gapi.DomainParticipant.ignore_topic(
                GapiPeer,
                handle);
        }

        public ReturnCode IgnorePublication(InstanceHandle handle)
        {
            return OpenSplice.Gapi.DomainParticipant.ignore_publication(
                GapiPeer,
                handle);
        }

        public ReturnCode IgnoreSubscription(InstanceHandle handle)
        {
            return OpenSplice.Gapi.DomainParticipant.ignore_subscription(
                GapiPeer,
                handle);
        }

        public string DomainId
        {
            get
            {
                IntPtr ptr = OpenSplice.Gapi.DomainParticipant.get_domain_id(GapiPeer);
                string result = Marshal.PtrToStringAnsi(ptr);
                Gapi.GenericAllocRelease.Free(ptr);

                return result;
            }
        }

        public ReturnCode AssertLiveliness()
        {
            return OpenSplice.Gapi.DomainParticipant.assert_liveliness(
                GapiPeer);
        }

        public ReturnCode SetDefaultPublisherQos(ref PublisherQos qos)
        {
            using (PublisherQosMarshaler marshaler = new PublisherQosMarshaler())
            {
                return OpenSplice.Gapi.DomainParticipant.set_default_publisher_qos(
                GapiPeer,
                marshaler.GapiPtr);
            }
        }

        public ReturnCode GetDefaultPublisherQos(out PublisherQos qos)
        {
            qos = new PublisherQos();
            ReturnCode result = ReturnCode.Error;

            using (PublisherQosMarshaler marshaler = new PublisherQosMarshaler())
            {
                result = OpenSplice.Gapi.DomainParticipant.get_default_publisher_qos(
                GapiPeer,
                marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(out qos);
                }
            }

            return result;
        }

        public ReturnCode SetDefaultSubscriberQos(ref SubscriberQos qos)
        {
            using (SubscriberQosMarshaler marshaler = new SubscriberQosMarshaler())
            {
                return OpenSplice.Gapi.DomainParticipant.set_default_subscriber_qos(
                GapiPeer,
                marshaler.GapiPtr);
            }
        }

        public ReturnCode GetDefaultSubscriberQos(out SubscriberQos qos)
        {
            qos = new SubscriberQos();
            ReturnCode result = ReturnCode.Error;

            using (SubscriberQosMarshaler marshaler = new SubscriberQosMarshaler())
            {
                result = OpenSplice.Gapi.DomainParticipant.get_default_subscriber_qos(
                GapiPeer,
                marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(out qos);
                }
            }

            return result;
        }

        public ReturnCode SetDefaultTopicQos(ref TopicQos qos)
        {
            using (TopicQosMarshaler marshaler = new TopicQosMarshaler())
            {
                return OpenSplice.Gapi.DomainParticipant.set_default_topic_qos(
                GapiPeer,
                marshaler.GapiPtr);
            }
        }

        public ReturnCode GetDefaultTopicQos(out TopicQos qos)
        {
            qos = new TopicQos();
            ReturnCode result = ReturnCode.Error;

            using (TopicQosMarshaler marshaler = new TopicQosMarshaler())
            {
                result = OpenSplice.Gapi.DomainParticipant.get_default_topic_qos(
                GapiPeer,
                marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(out qos);
                }
            }

            return result;
        }

        public bool ContainsEntity(InstanceHandle handle)
        {
            return OpenSplice.Gapi.DomainParticipant.contains_entity(
                GapiPeer,
                handle);
        }

        public ReturnCode GetCurrentTime(out Time currentTime)
        {
            return OpenSplice.Gapi.DomainParticipant.get_current_time(
                GapiPeer,
                out currentTime);
        }

        public ITypeSupport GetTypeSupport(string registeredName)
        {
            ITypeSupport typeSupport = null;

            IntPtr gapiPtr = OpenSplice.Gapi.DomainParticipant.get_typesupport(
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

            IntPtr gapiPtr = OpenSplice.Gapi.DomainParticipant.lookup_typesupport(
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
            return OpenSplice.Gapi.DomainParticipant.get_type_metadescription(
                GapiPeer,
                typeName);
        }
    }
}