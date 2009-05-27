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
    // Return codes
    // ----------------------------------------------------------------------
    public enum ReturnCode
    {
        Ok = 0,
        Error = 1,
        Unsupported = 2,
        BadParameter = 3,
        PreconditionNotMet = 4,
        OutOfResources = 5,
        NotEnabled = 6,
        ImmutablePolicy = 7,
        InconsistentPolicy = 8,
        AlreadyDeleted = 9,
        Timeout = 10,
        NoData = 11,
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
    [Flags]
    public enum StatusKind
    {
        InconsistentTopic = 0x0001 << 0,
        OfferedDeadlineMissed = 0x0001 << 1,
        RequestedDeadlineMissed = 0x0001 << 2,
        OfferedIncompatibleQos = 0x0001 << 5,
        RequestedIncompatibleQos = 0x0001 << 6,
        SampleLost = 0x0001 << 7,
        SampleRejected = 0x0001 << 8,
        DataOnReaders = 0x0001 << 9,
        DataAvailable = 0x0001 << 10,
        LivelinessLost = 0x0001 << 11,
        LivelinessChanged = 0x0001 << 12,
        PublicationMatched = 0x0001 << 13,
        SubscriptionMatched = 0x0001 << 14,

        Any = 0x7fffffff,
    }

    public enum SampleRejectedStatusKind
    {
        NotRejected,
        RejectedByInstanceLimit,
        RejectedBySamplesLimit,
        RejectedBySamplesPerInstanceLimit,
    };

    public enum QosPolicyId
    {
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

    [Flags]
    public enum SampleStateKind
    {
        Read = 0x0001 << 0,
        NotRead = 0x0001 << 1,

        Any = 0xffff,
    }

    [Flags]
    public enum ViewStateKind
    {
        New = 0x0001 << 0,
        NotNew = 0x0001 << 1,

        Any = 0xffff,
    }

    [Flags]
    public enum InstanceStateKind
    {
        Alive = 0x0001 << 0,
        NotAliveDisposed = 0x0001 << 1,
        NotAliveNoWriters = 0x0001 << 2,

        NotAlive = 0x006,
        Any = 0xffff,
    }

    public enum DurabilityQosPolicyKind
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
    }
} // end namespace DDS
