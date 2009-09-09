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
        ReturnCode GetConditions(out ICondition[] attachedConditions);
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
        IDomainParticipant CreateParticipant(string domainId, ref DomainParticipantQos qos);
        IDomainParticipant CreateParticipant(string domainId, ref DomainParticipantQos qos,
            IDomainParticipantListener listener, StatusKind mask);
        ReturnCode DeleteParticipant(IDomainParticipant participant);
        IDomainParticipant LookupParticipant(string domainId);
        ReturnCode SetDefaultParticipantQos(ref DomainParticipantQos qos);
        ReturnCode GetDefaultParticipantQos(out DomainParticipantQos qos);
        ReturnCode SetQos(ref DomainParticipantFactoryQos qos);
        ReturnCode GetQos(out DomainParticipantFactoryQos qos);
    }

    // ----------------------------------------------------------------------
    // Entities
    // ----------------------------------------------------------------------
    public interface IEntity
    {
        ReturnCode Enable();
        IStatusCondition GetStatusCondition();
        StatusKind GetStatusChanges();
        InstanceHandle GetInstanceHandle();
    }

    public interface IDomainParticipant : IEntity
    {
        IPublisher CreatePublisher();
        IPublisher CreatePublisher(
            IPublisherListener listener, StatusKind mask);
        IPublisher CreatePublisher(ref PublisherQos qos);
        IPublisher CreatePublisher(ref PublisherQos qos,
            IPublisherListener listener, StatusKind mask);
        ReturnCode DeletePublisher(IPublisher p);
        ISubscriber CreateSubscriber();
        ISubscriber CreateSubscriber(
            ISubscriberListener listener, StatusKind mask);
        ISubscriber CreateSubscriber(ref SubscriberQos qos);
        ISubscriber CreateSubscriber(ref SubscriberQos qos,
            ISubscriberListener listener, StatusKind mask);
        ReturnCode DeleteSubscriber(ISubscriber s);
        ISubscriber BuiltInSubscriber { get; }
        ITopic CreateTopic(string topicName, string typeName);
        ITopic CreateTopic(string topicName, string typeName,
            ITopicListener listener, StatusKind mask);
        ITopic CreateTopic(string topicName, string typeName, ref TopicQos qos);
        ITopic CreateTopic(string topicName, string typeName, ref TopicQos qos,
            ITopicListener listener, StatusKind mask);
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
            ITypeSupport typeSupport,
            string subscriptionExpression,
            params string[] expressionParameters);
        ReturnCode DeleteMultiTopic(IMultiTopic multiTopic);
        ReturnCode DeleteContainedEntities();
        ReturnCode SetQos(ref DomainParticipantQos qos);
        ReturnCode GetQos(out DomainParticipantQos qos);
        ReturnCode SetListener(IDomainParticipantListener listener, StatusKind mask);
        ReturnCode IgnoreParticipant(InstanceHandle handle);
        ReturnCode IgnoreTopic(InstanceHandle handle);
        ReturnCode IgnorePublication(InstanceHandle handle);
        ReturnCode IgnoreSubscription(InstanceHandle handle);
        string DomainId { get; }
        ReturnCode AssertLiveliness();
        ReturnCode SetDefaultPublisherQos(ref PublisherQos qos);
        ReturnCode GetDefaultPublisherQos(out PublisherQos qos);
        ReturnCode SetDefaultSubscriberQos(ref SubscriberQos qos);
        ReturnCode GetDefaultSubscriberQos(out SubscriberQos qos);
        ReturnCode SetDefaultTopicQos(ref TopicQos qos);
        ReturnCode GetDefaultTopicQos(out TopicQos qos);
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
        ReturnCode GetQos(out TopicQos qos);
        ReturnCode SetQos(ref TopicQos qos);
        ReturnCode SetListener(ITopicListener listener, StatusKind mask);
    }

    public interface IContentFilteredTopic : ITopicDescription
    {
        string GetFilterExpression();
        ReturnCode GetExpressionParameters(out string[] expressionParameters);
        ReturnCode SetExpressionParameters(params string[] expressionParameters);
        ITopic RelatedTopic { get; }
    }

    public interface IMultiTopic : ITopicDescription
    {
        string SubscriptionExpression { get; }
        ReturnCode GetExpressionParameters(out string[] expressionParameters);
        ReturnCode SetExpressionParameters(params string[] expressionParameters);
    }

    // ----------------------------------------------------------------------  
    // Publisher/Subscriber, DataWriter/DataReader
    // ----------------------------------------------------------------------  
    public interface IPublisher : IEntity
    {
        IDataWriter CreateDataWriter(ITopic topic);
        IDataWriter CreateDataWriter(ITopic topic,
            IDataWriterListener listener, StatusKind mask);
        IDataWriter CreateDataWriter(ITopic topic, ref DataWriterQos qos);
        IDataWriter CreateDataWriter(ITopic topic, ref DataWriterQos qos,
            IDataWriterListener listener, StatusKind mask);
        ReturnCode DeleteDataWriter(IDataWriter dataWriter);
        IDataWriter LookupDataWriter(string topicName);
        ReturnCode DeleteContainedEntities();
        ReturnCode SetQos(ref PublisherQos qos);
        ReturnCode GetQos(out PublisherQos qos);
        ReturnCode SuspendPublications();
        ReturnCode ResumePublications();
        ReturnCode BeginCoherentChanges();
        ReturnCode EndCoherentChanges();
        ReturnCode WaitForAcknowledgments(Duration maxWait);
        IDomainParticipant GetParticipant();
        ReturnCode SetDefaultDataWriterQos(ref DataWriterQos qos);
        ReturnCode GetDefaultDataWriterQos(out DataWriterQos qos);
        ReturnCode CopyFromTopicQos(out DataWriterQos dataWriterQos, ref TopicQos topicQos);
    }

    public interface IDataWriter : IEntity
    {
        ReturnCode SetQos(ref DataWriterQos qos);
        ReturnCode GetQos(out DataWriterQos qos);
        ReturnCode SetListener(IDataWriterListener listener, StatusKind mask);
        ITopic Topic { get; }
        IPublisher Publisher { get; }
        ReturnCode WaitForAcknowledgments(Duration maxWait);
        ReturnCode GetLivelinessLostStatus(out LivelinessLostStatus status);
        ReturnCode GetOfferedDeadlineMissedStatus(out OfferedDeadlineMissedStatus status);
        ReturnCode GetOfferedIncompatibleQosStatus(out OfferedIncompatibleQosStatus status);
        ReturnCode GetPublicationMatchedStatus(out PublicationMatchedStatus status);
        ReturnCode AssertLiveliness();
        ReturnCode GetMatchedSubscriptions(out InstanceHandle[] subscriptionHandles);
        ReturnCode GetMatchedSubscriptionData(
            out SubscriptionBuiltinTopicData subscriptionData,
            InstanceHandle subscriptionHandle);
    }

    public interface ISubscriber : IEntity
    {
        IDataReader CreateDataReader(ITopicDescription topic);
        IDataReader CreateDataReader(ITopicDescription topic,
            IDataReaderListener listener, StatusKind mask);
        IDataReader CreateDataReader(ITopicDescription topic, ref DataReaderQos qos);
        IDataReader CreateDataReader(ITopicDescription topic, ref DataReaderQos qos,
            IDataReaderListener listener, StatusKind mask);
        ReturnCode DeleteDataReader(IDataReader dataReader);
        ReturnCode DeleteContainedEntities();
        IDataReader LookupDataReader(string topicName);
        ReturnCode GetDataReaders(out IDataReader[] readers);
        ReturnCode GetDataReaders(
            out IDataReader[] readers,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);
        ReturnCode NotifyDataReaders();
        ReturnCode SetQos(ref SubscriberQos qos);
        ReturnCode GetQos(out SubscriberQos qos);
        ReturnCode SetListener(ISubscriberListener listener, StatusKind mask);
        ReturnCode BeginAccess();
        ReturnCode EndAccess();
        IDomainParticipant Participant { get; }
        ReturnCode SetDefaultDataReaderQos(ref DataReaderQos qos);
        ReturnCode GetDefaultDataReaderQos(out DataReaderQos qos);
        ReturnCode CopyFromTopicQos(out DataReaderQos dataReaderQos, ref TopicQos topicQos);
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
        ReturnCode SetQos(ref DataReaderQos qos);
        ReturnCode GetQos(out DataReaderQos qos);
        ReturnCode SetListener(IDataReaderListener listener, StatusKind mask);
        ITopicDescription GetTopicDescription();
        ISubscriber Subscriber { get; }
        ReturnCode GetSampleRejectedStatus(out SampleRejectedStatus status);
        ReturnCode GetLivelinessChangedStatus(out LivelinessChangedStatus status);
        ReturnCode GetRequestedDeadlineMissedStatus(out RequestedDeadlineMissedStatus status);
        ReturnCode GetRequestedIncompatibleQosStatus(out RequestedIncompatibleQosStatus status);
        ReturnCode GetSubscriptionMatchedStatus(out SubscriptionMatchedStatus status);
        ReturnCode GetSampleLostStatus(out SampleLostStatus status);
        ReturnCode WaitForHistoricalData(Duration maxWait);
        ReturnCode GetMatchedPublications(out InstanceHandle[] publicationHandles);
        ReturnCode GetMatchedPublicationData(
            out PublicationBuiltinTopicData publicationData,
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
