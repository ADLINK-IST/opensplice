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
    // Return codes
    // ----------------------------------------------------------------------
    /// <summary>
    /// This is the enum that represents the various ReturnCode values that DDS operations return.
    /// </summary>
    public enum ReturnCode
    {
        /// <summary>
        /// Successful return
        /// </summary>
        Ok = 0,
        /// <summary>
        /// Generic, unspecified error
        /// </summary>
        Error = 1,
        /// <summary>
        /// Unsupported operation or QosPolicy setting. Can only
        /// be returned by operations that are optional or operations
        /// that uses an optional &ltEntity&gtQoS as a parameter
        /// </summary>
        Unsupported = 2,
        /// <summary>
        /// Illegal parameter value
        /// </summary>
        BadParameter = 3,
        /// <summary>
        /// A pre-condition for the operation was not met
        /// </summary>
        PreconditionNotMet = 4,
        /// <summary>
        /// Service ran out of the resources needed to complete the operation
        /// </summary>
        OutOfResources = 5,
        /// <summary>
        /// Operation invoked on an Entity that is not yet enabled
        /// </summary>
        NotEnabled = 6,
        /// <summary>
        /// Application attempted to modify an immutable QosPolicy
        /// </summary>
        ImmutablePolicy = 7,
        /// <summary>
        /// Application specified a set of policies that are not consistent with each other
        /// </summary>
        InconsistentPolicy = 8,
        /// <summary>
        /// The object target of this operation has already been deleted
        /// </summary>
        AlreadyDeleted = 9,
        /// <summary>
        /// The operation timed out
        /// </summary>
        Timeout = 10,
        /// <summary>
        /// Indicates a situation where the operation did not return any data
        /// </summary>
        NoData = 11,
        /// <summary>
        /// An operation was invoked on an inappropriate object or
        /// at an inappropriate time (as determined by QosPolicies
        /// that control the behaviour of the object in question).
        /// There is no precondition that could be changed to make
        /// the operation succeed. This code can not be returned in C#.
        /// </summary>
        IllegalOperation = 12,
    }

    // ----------------------------------------------------------------------
    // Error codes
    // ----------------------------------------------------------------------
    public enum ErrorCode
    {
        Undefined = 0,
        Error = 1,
        OutOfResources = 2,
        CreationKernelEntityFailed = 3,
        InvalidValue = 4,
        InvalidDuration = 5,
        InvalidTime = 6,
        EntityInUse = 7,
        ContainsEntities = 8,
        EntityUnknown = 9,
        HandleNotRegistered = 10,
        HandleNotMatched = 11,
        HandleInvalid = 12,
        InvalidSequence = 13,
        UnsupportedValue = 14,
        InconsistentValue = 15,
        ImmutableQosPolicy = 16,
        InconsistentQos = 17,
        UnsupportedQosPolicy = 18,
        ContainsConditions = 19,
        ContainsLoans = 20,
        InconsistentTopic = 21,
    }

    // ----------------------------------------------------------------------
    // Status to support listeners and conditions
    // ----------------------------------------------------------------------
    /// <summary>
    /// Each concrete Entity class has a set of Status attributes and for each attribute the
    /// Entity class provides an operation to read the value. Changes to Status attributes
    /// will affect associated StatusCondition and (invoked and associated) Listener
    /// objects.The communication statuses whose changes can be communicated to the application
    /// depend on the Entity.
    /// </summary>
    [Flags]
    public enum StatusKind
    {
        /// <summary>
        /// Another Topic exists with the same name but with different characteristics.
        /// </summary>
        InconsistentTopic = 0x0001 << 0,
        /// <summary>
        /// The deadline that the DataWriter has committed through its DeadlineQosPolicy
        /// was not respected for a specific instance.
        /// </summary>
        OfferedDeadlineMissed = 0x0001 << 1,
        /// <summary>
        /// The deadline that the DataReader was expecting through its DeadlineQosPolicy
        /// was not respected for a specific instance.
        /// </summary>
        RequestedDeadlineMissed = 0x0001 << 2,
        /// <summary>
        /// A QosPolicy setting was incompatible with what was requested.
        /// </summary>
        OfferedIncompatibleQos = 0x0001 << 5,
        /// <summary>
        /// A QosPolicy setting was incompatible with what is offered.
        /// </summary>
        RequestedIncompatibleQos = 0x0001 << 6,
        /// <summary>
        /// A sample has been lost (never received).
        /// </summary>
        SampleLost = 0x0001 << 7,
        /// <summary>
        /// A (received) sample has been rejected.
        /// </summary>
        SampleRejected = 0x0001 << 8,
        /// <summary>
        /// New information is available.
        /// </summary>
        DataOnReaders = 0x0001 << 9,
        /// <summary>
        /// New information is available.
        /// </summary>
        DataAvailable = 0x0001 << 10,
        /// <summary>
        /// The liveliness that the DataWriter has committed through its LivelinessQosPolicy 
        /// was not respected; thus DataReader objects will consider the DataWriter as no longer "alive".
        /// </summary>
        LivelinessLost = 0x0001 << 11,
        /// <summary>
        /// The liveliness of one or more DataWriter objects that were writing instances read
        /// through the DataReader has changed. Some DataWriter have become “alive” or “not alive”.
        /// </summary>
        LivelinessChanged = 0x0001 << 12,
        /// <summary>
        /// The DataWriter has found DataReader that matches the Topic and has compatible QoS.
        /// </summary>
        PublicationMatched = 0x0001 << 13,
        /// <summary>
        /// The DataReader has found a DataWriter that matches the Topic and has compatible QoS.
        /// </summary>
        SubscriptionMatched = 0x0001 << 14,
        /// <summary>
        /// Any status.
        /// </summary>
        Any = 0x7fffffff,
    }

    /// <summary>
    /// This struct contains the statistics about samples that have been rejected.
    /// </summary>
    public enum SampleRejectedStatusKind
    {
        /// <summary>
        /// no sample has been rejected yet
        /// </summary>
        NotRejected,
        /// <summary>
        /// the sample was rejected because it would exceed the maximum number of instances set by the ResourceLimitsQosPolicy.
        /// </summary>
        RejectedByInstanceLimit,
        /// <summary>
        /// the sample was rejected because it would exceed the maximum number of samples set by the ResourceLimits QosPolicy.
        /// </summary>
        RejectedBySamplesLimit,
        /// <summary>
        /// the sample was rejected because it would exceed the maximum number of samples per instance set by the ResourceLimitsQosPolicy.
        /// </summary>
        RejectedBySamplesPerInstanceLimit,
    };

    /// <summary>
    /// QosPolicyId enum
    /// </summary>
    public enum QosPolicyId
    {
        /// <summary>
        /// 
        /// </summary>
        InvalidQos = 0,
        UserDataQos = 1,
        DurabilityQos = 2,
        PresentationQos = 3,
        DeadlineQos = 4,
        LatencyBudgetQos = 5,
        OwnershipQos = 6,
        OwnershipStrengthQos = 7,
        LivelinessQos = 8,
        TimeBasedFilterQos = 9,
        PartitionQos = 10,
        ReliabilityQos = 11,
        DestinationOrderQos = 12,
        HistoryQos = 13,
        ResourceLimitsQos = 14,
        EntityFactoryQos = 15,
        WriterDataLifecycleQos = 16,
        ReaderDataLifecycleQos = 17,
        TopicDataQos = 18,
        GroupDataQos = 19,
        TransportPriorityQos = 20,
        LifespanQos = 21,
        DurabilityServiceQos = 22,
        SchedulingQos = 27,
    }

    /// <summary>
    /// For each sample, the Data Distribution Service internally maintains a
    /// sample_state specific to each DataReader. The sample_state can either be
    /// Read or NotRead.
    /// </summary>
    [Flags]
    public enum SampleStateKind
    {
        /// <summary>
        /// Read - indicates that the DataReader has already accessed that
        /// sample by means of read. Had the sample been accessed by take it would no
        /// longer be available to the DataReader.
        /// </summary>
        Read = 0x0001 << 0,
        /// <summary>
        /// Notread - indicates that the DataReader has not accessed that sample before.
        /// </summary>
        NotRead = 0x0001 << 1,

        Any = 0xffff,
    }

    /// <summary>
    /// For each instance (identified by the key), the Data Distribution Service internally
    /// maintains a view_state relative to each DataReader. The view_state can
    /// either be New or NotNew
    /// </summary>
    [Flags]
    public enum ViewStateKind
    {
        /// <summary>
        /// New - indicates that either this is the first time that the DataReader
        /// has ever accessed samples of that instance, or else that the DataReader has
        /// accessed previous samples of the instance, but the instance has since been reborn
        /// (i.e. becomes not-alive and then alive again);
        /// </summary>
        New = 0x0001 << 0,
        /// <summary>
        /// NotNew - indicates that the DataReader has already accessed samples of the same instance 
        /// and that the instance has not been reborn since.
        /// </summary>
        NotNew = 0x0001 << 1,

        Any = 0xffff,
    }

    /// <summary>
    /// For each instance the Data Distribution Service internally maintains an instance_state.
    /// </summary>
    [Flags]
    public enum InstanceStateKind
    {
        /// <summary>
        /// Alive indicates that:
        /// <list type="bullet">
        /// <item>samples have been received for the instance</item>
        /// <item>there are live DataWriter objects writing the instance</item>
        /// <item>the instance has not been explicitly disposed of 
        /// (or else samples have been received after it was disposed of)</item>
        /// </list>
        /// </summary>
        Alive = 0x0001 << 0,
        /// <summary>
        /// NotAliveDisposed indicates the instance was explicitly disposed of by a 
        /// DataWriter by means of the dispose operation and no new samples has been written.
        /// </summary>
        NotAliveDisposed = 0x0001 << 1,
        /// <summary>
        /// NotAliveNoWriters - indicates the instance has been declared as not-alive by the 
        /// DataReader because it detected that there are no live DataWriter objects writing that instance.
        /// </summary>
        NotAliveNoWriters = 0x0001 << 2,

        NotAlive = 0x006,
        Any = 0xffff,
    }

/*    public enum DurabilityQosPolicyKind
    {
        VolatileDurabilityQos,
        TransientLocalDurabilityQos,
        TransientDurabilityQos,
        PersistentDurabilityQos
    }

    public enum PresentationQosPolicyAccessScopeKind
    {
        InstancePresentationQos,
        TopicPresentationQos,
        GroupPresentationQos
    }

    public enum OwnershipQosPolicyKind
    {
        SharedOwnershipQos,
        ExclusiveOwnershipQos
    }

    public enum LivelinessQosPolicyKind
    {
        AutomaticLivelinessQos,
        ManualByParticipantLivelinessQos,
        ManualByTopicLivelinessQos
    }

    public enum ReliabilityQosPolicyKind
    {
        BestEffortReliabilityQos,
        ReliableReliabilityQos
    }

    public enum DestinationOrderQosPolicyKind
    {
        ByReceptionTimestampDestinationOrderQos,
        BySourceTimestampDestinationOrderQos
    }

    public enum HistoryQosPolicyKind
    {
        KeepLastHistoryQos,
        KeepAllHistoryQos
    }

    public enum SchedulingClassQosPolicyKind
    {
        ScheduleDefault,
        ScheduleTimesharing,
        ScheduleRealtime
    }

    public enum SchedulingPriorityQosPolicyKind
    {
        PriorityRelative,
        PriorityAbsolute
    }*/
} // end namespace DDS
