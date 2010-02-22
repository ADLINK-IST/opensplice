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

// Csharp backend
// PTF C# mapping for IDL
// File /Users/Jcm/Documents/Ecllipse_WS/CSharpDDS/generated/dds_dcps.cs
// Generated on 2008-11-11 13:36:00
// from dds_dcps.idl

using System;
using System.Runtime.InteropServices;

namespace DDS
{
    // ----------------------------------------------------------------------
    // Conditions
    // ----------------------------------------------------------------------
    public interface ICondition
    {
        bool GetTriggerValue();
    }

    public interface IWaitSet
    {
        ReturnCode Wait(ref ICondition[] activeConditions, Duration timeout);
        ReturnCode AttachCondition(ICondition condition);
        ReturnCode DetachCondition(ICondition condition);
        ReturnCode GetConditions(ref ICondition[] attachedConditions);
    }

    public interface IGuardCondition : ICondition
    {
        ReturnCode SetTriggerValue(bool value);
    }

    public interface IStatusCondition : ICondition
    {
        StatusKind GetEnabledStatuses();
        ReturnCode SetEnabledStatuses(StatusKind mask);
        IEntity GetEntity();
    }

    public interface IReadCondition : ICondition
    {
        SampleStateKind GetSampleStateMask();
        ViewStateKind GetViewStateMask();
        InstanceStateKind GetInstanceStateMask();
        IDataReader GetDataReader();
    }

    public interface IQueryCondition : IReadCondition
    {
        string GetQueryExpression();
        ReturnCode GetQueryParameters(ref string[] queryParameters);
        ReturnCode SetQueryParameters(params string[] queryParameters);
    }

    // ----------------------------------------------------------------------
    // Factory
    // ----------------------------------------------------------------------
    public interface IDomainParticipantFactory
    {
        IDomainParticipant CreateParticipant(string domainId);
        IDomainParticipant CreateParticipant(string domainId,
            IDomainParticipantListener listener, StatusKind mask);
        IDomainParticipant CreateParticipant(string domainId, DomainParticipantQos qos);
        IDomainParticipant CreateParticipant(string domainId, DomainParticipantQos qos,
            IDomainParticipantListener listener, StatusKind mask);
        ReturnCode DeleteParticipant(IDomainParticipant participant);
        IDomainParticipant LookupParticipant(string domainId);
        ReturnCode SetDefaultParticipantQos(DomainParticipantQos qos);
        ReturnCode GetDefaultParticipantQos(ref DomainParticipantQos qos);
        ReturnCode SetQos(DomainParticipantFactoryQos qos);
        ReturnCode GetQos(ref DomainParticipantFactoryQos qos);
    }

    // ----------------------------------------------------------------------
    // Entities
    // ----------------------------------------------------------------------
    public interface IEntity
    {
        ReturnCode Enable();
        IStatusCondition StatusCondition { get; }
        StatusKind StatusChanges { get; }
        InstanceHandle InstanceHandle { get; }
    }

    public interface IDomainParticipant : IEntity
    {
        IPublisher CreatePublisher();
        IPublisher CreatePublisher(
                IPublisherListener listener, 
                StatusKind mask);
        IPublisher CreatePublisher(PublisherQos qos);
        IPublisher CreatePublisher(
                PublisherQos qos,
                IPublisherListener listener, 
                StatusKind mask);
        ReturnCode DeletePublisher(IPublisher p);
        ISubscriber CreateSubscriber();
        ISubscriber CreateSubscriber(
                ISubscriberListener listener, 
                StatusKind mask);
        ISubscriber CreateSubscriber(SubscriberQos qos);
        ISubscriber CreateSubscriber(
                SubscriberQos qos,
                ISubscriberListener listener, 
                StatusKind mask);
        ReturnCode DeleteSubscriber(ISubscriber s);
        ISubscriber BuiltInSubscriber { get; }
        ITopic CreateTopic(string topicName, string typeName);
        ITopic CreateTopic(
                string topicName, 
                string typeName,
                ITopicListener listener, 
                StatusKind mask);
        ITopic CreateTopic(string topicName, string typeName, TopicQos qos);
        ITopic CreateTopic(
                string topicName, 
                string typeName, 
                TopicQos qos,
                ITopicListener listener, 
                StatusKind mask);
        ReturnCode DeleteTopic(ITopic topic);
        ITopic FindTopic(string topicName, Duration timeout);
        ITopicDescription LookupTopicDescription(string name);
        IContentFilteredTopic CreateContentFilteredTopic(
                string name,
                ITopic relatedTopic,
                string filterExpression,
                params string[] expressionParameters);
        ReturnCode DeleteContentFilteredTopic(IContentFilteredTopic aContentFilteredTopic);
        IMultiTopic CreateMultiTopic(
                string name,
                string typeName,
                string subscriptionExpression,
                params string[] expressionParameters);
        ReturnCode DeleteMultiTopic(IMultiTopic multiTopic);
        ReturnCode DeleteContainedEntities();
        ReturnCode SetQos(DomainParticipantQos qos);
        ReturnCode GetQos(ref DomainParticipantQos qos);
        ReturnCode SetListener(IDomainParticipantListener listener, StatusKind mask);
        ReturnCode IgnoreParticipant(InstanceHandle handle);
        ReturnCode IgnoreTopic(InstanceHandle handle);
        ReturnCode IgnorePublication(InstanceHandle handle);
        ReturnCode IgnoreSubscription(InstanceHandle handle);
        string DomainId { get; }
        ReturnCode AssertLiveliness();
        ReturnCode SetDefaultPublisherQos(PublisherQos qos);
        ReturnCode GetDefaultPublisherQos(ref PublisherQos qos);
        ReturnCode SetDefaultSubscriberQos(SubscriberQos qos);
        ReturnCode GetDefaultSubscriberQos(ref SubscriberQos qos);
        ReturnCode SetDefaultTopicQos(TopicQos qos);
        ReturnCode GetDefaultTopicQos(ref TopicQos qos);
        bool ContainsEntity(InstanceHandle handle);
        ReturnCode GetCurrentTime(out Time currentTime);
        ITypeSupport GetTypeSupport(string registeredName);
        ITypeSupport LookupTypeSupport(string registeredTypeName);
    }

    public interface ITypeSupport
    {
        ReturnCode RegisterType(IDomainParticipant domain, string typeName);
        string TypeName { get; }
        string Description { get; }
        string KeyList { get; }
    }

    // ----------------------------------------------------------------------
    // Topics
    // ----------------------------------------------------------------------
    public interface ITopicDescription
    {
        string TypeName { get; }
        string Name { get; }
        IDomainParticipant Participant { get; }
    }

    public interface ITopic : IEntity, ITopicDescription
    {
        ReturnCode GetInconsistentTopicStatus(ref InconsistentTopicStatus aStatus);
        ReturnCode GetQos(ref TopicQos qos);
        ReturnCode SetQos(TopicQos qos);
        ReturnCode SetListener(ITopicListener listener, StatusKind mask);
    }

    public interface IContentFilteredTopic : ITopicDescription
    {
        string GetFilterExpression();
        ReturnCode GetExpressionParameters(ref string[] expressionParameters);
        ReturnCode SetExpressionParameters(params string[] expressionParameters);
        ITopic RelatedTopic { get; }
    }

    public interface IMultiTopic : ITopicDescription
    {
        string SubscriptionExpression { get; }
        ReturnCode GetExpressionParameters(ref string[] expressionParameters);
        ReturnCode SetExpressionParameters(params string[] expressionParameters);
    }

    // ----------------------------------------------------------------------  
    // Publisher/Subscriber, DataWriter/DataReader
    // ----------------------------------------------------------------------  
    public interface IPublisher : IEntity
    {
        IDataWriter CreateDataWriter(ITopic topic);
        IDataWriter CreateDataWriter(
                ITopic topic,
                IDataWriterListener listener, StatusKind mask);
        IDataWriter CreateDataWriter(ITopic topic, DataWriterQos qos);
        IDataWriter CreateDataWriter(
                ITopic topic, 
                DataWriterQos qos,
                IDataWriterListener listener, 
                StatusKind mask);
        ReturnCode DeleteDataWriter(IDataWriter dataWriter);
        IDataWriter LookupDataWriter(string topicName);
        ReturnCode DeleteContainedEntities();
        ReturnCode SetQos(PublisherQos qos);
        ReturnCode GetQos(ref PublisherQos qos);
        ReturnCode SetListener(IPublisherListener listener, StatusKind mask);
        ReturnCode SuspendPublications();
        ReturnCode ResumePublications();
        ReturnCode BeginCoherentChanges();
        ReturnCode EndCoherentChanges();
        ReturnCode WaitForAcknowledgments(Duration maxWait);
        IDomainParticipant GetParticipant();
        ReturnCode SetDefaultDataWriterQos(DataWriterQos qos);
        ReturnCode GetDefaultDataWriterQos(ref DataWriterQos qos);
        ReturnCode CopyFromTopicQos(ref DataWriterQos dataWriterQos, TopicQos topicQos);
    }

    public interface IDataWriter : IEntity
    {
        ReturnCode SetQos(DataWriterQos qos);
        ReturnCode GetQos(ref DataWriterQos qos);
        ReturnCode SetListener(IDataWriterListener listener, StatusKind mask);
        ITopic Topic { get; }
        IPublisher Publisher { get; }
        ReturnCode WaitForAcknowledgments(Duration maxWait);
        ReturnCode GetLivelinessLostStatus(ref LivelinessLostStatus status);
        ReturnCode GetOfferedDeadlineMissedStatus(ref OfferedDeadlineMissedStatus status);
        ReturnCode GetOfferedIncompatibleQosStatus(ref OfferedIncompatibleQosStatus status);
        ReturnCode GetPublicationMatchedStatus(ref PublicationMatchedStatus status);
        ReturnCode AssertLiveliness();
        ReturnCode GetMatchedSubscriptions(ref InstanceHandle[] subscriptionHandles);
        ReturnCode GetMatchedSubscriptionData(
                ref SubscriptionBuiltinTopicData subscriptionData,
                InstanceHandle subscriptionHandle);
    }

    public interface ISubscriber : IEntity
    {
        IDataReader CreateDataReader(ITopicDescription topic);
        IDataReader CreateDataReader(
                ITopicDescription topic,
                IDataReaderListener listener, StatusKind mask);
        IDataReader CreateDataReader(ITopicDescription topic, DataReaderQos qos);
        IDataReader CreateDataReader(
                ITopicDescription topic, 
                DataReaderQos qos,
                IDataReaderListener listener, 
                StatusKind mask);
        ReturnCode DeleteDataReader(IDataReader dataReader);
        ReturnCode DeleteContainedEntities();
        IDataReader LookupDataReader(string topicName);
        ReturnCode GetDataReaders(ref IDataReader[] readers);
        ReturnCode GetDataReaders(
                ref IDataReader[] readers,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates);
        ReturnCode NotifyDataReaders();
        ReturnCode SetQos(SubscriberQos qos);
        ReturnCode GetQos(ref SubscriberQos qos);
        ReturnCode SetListener(ISubscriberListener listener, StatusKind mask);
        ReturnCode BeginAccess();
        ReturnCode EndAccess();
        IDomainParticipant Participant { get; }
        ReturnCode SetDefaultDataReaderQos(DataReaderQos qos);
        ReturnCode GetDefaultDataReaderQos(ref DataReaderQos qos);
        ReturnCode CopyFromTopicQos(ref DataReaderQos dataReaderQos, TopicQos topicQos);
    }

    public interface IDataReader : IEntity
    {
        IReadCondition CreateReadCondition();
        IReadCondition CreateReadCondition(
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates);
        IQueryCondition CreateQueryCondition(
                string queryExpression,
                params string[] queryParameters);
        IQueryCondition CreateQueryCondition(
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates,
                string queryExpression,
                params string[] queryParameters);
        ReturnCode DeleteReadCondition(IReadCondition condition);
        ReturnCode DeleteContainedEntities();
        ReturnCode SetQos(DataReaderQos qos);
        ReturnCode GetQos(ref DataReaderQos qos);
        ReturnCode SetListener(IDataReaderListener listener, StatusKind mask);
        ITopicDescription GetTopicDescription();
        ISubscriber Subscriber { get; }
        ReturnCode GetSampleRejectedStatus(ref SampleRejectedStatus status);
        ReturnCode GetLivelinessChangedStatus(ref LivelinessChangedStatus status);
        ReturnCode GetRequestedDeadlineMissedStatus(ref RequestedDeadlineMissedStatus status);
        ReturnCode GetRequestedIncompatibleQosStatus(ref RequestedIncompatibleQosStatus status);
        ReturnCode GetSubscriptionMatchedStatus(ref SubscriptionMatchedStatus status);
        ReturnCode GetSampleLostStatus(ref SampleLostStatus status);
        ReturnCode WaitForHistoricalData(Duration maxWait);
        ReturnCode GetMatchedPublications(ref InstanceHandle[] publicationHandles);
        ReturnCode GetMatchedPublicationData(
                ref PublicationBuiltinTopicData publicationData,
                InstanceHandle publicationHandle);
    }

    // listener interfaces

    public interface ITopicListener
    {
        void OnInconsistentTopic(ITopic entityInterface, InconsistentTopicStatus status);
    }

    public interface IDataWriterListener
    {
        void OnOfferedDeadlineMissed(IDataWriter entityInterface, OfferedDeadlineMissedStatus status);
        void OnOfferedIncompatibleQos(IDataWriter entityInterface, OfferedIncompatibleQosStatus status);
        void OnLivelinessLost(IDataWriter entityInterface, LivelinessLostStatus status);
        void OnPublicationMatched(IDataWriter entityInterface, PublicationMatchedStatus status);
    }

    public interface IPublisherListener : IDataWriterListener
    {
    }

    public interface IDataReaderListener
    {
        void OnRequestedDeadlineMissed(IDataReader entityInterface, RequestedDeadlineMissedStatus status);
        void OnRequestedIncompatibleQos(IDataReader entityInterface, RequestedIncompatibleQosStatus status);
        void OnSampleRejected(IDataReader entityInterface, SampleRejectedStatus status);
        void OnLivelinessChanged(IDataReader entityInterface, LivelinessChangedStatus status);
        void OnDataAvailable(IDataReader entityInterface);
        void OnSubscriptionMatched(IDataReader entityInterface, SubscriptionMatchedStatus status);
        void OnSampleLost(IDataReader entityInterface, SampleLostStatus status);
    }

    public interface ISubscriberListener : IDataReaderListener
    {
        void OnDataOnReaders(ISubscriber entityInterface);
    }

    public interface IDomainParticipantListener : ITopicListener, IPublisherListener, ISubscriberListener
    {
    }


} // end namespace DDS
