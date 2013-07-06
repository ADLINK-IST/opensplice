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
    // Types & Pre-defined values
    // ----------------------------------------------------------------------
    [StructLayout(LayoutKind.Sequential)]
    public struct Duration
    {
        [MarshalAs(UnmanagedType.I4)]
        private int sec;
        public int Sec
        {
            get { return sec; }
            set { sec = value; }
        }

        [MarshalAs(UnmanagedType.U4)]
        private uint nanosec;
        public uint NanoSec
        {
            get { return nanosec; }
            set { nanosec = value; }
        }

        public const int InfiniteSec = 0x7fffffff;
        public const uint InfiniteNanoSec = 0x7fffffff;

        public const int ZeroSec = 0;
        public const uint ZeroNanoSec = 0;

        public static readonly Duration Infinite = new Duration(InfiniteSec, InfiniteNanoSec);
        public static readonly Duration Zero = new Duration(ZeroSec, ZeroNanoSec);

        public static Duration FromTimeSpan(TimeSpan value)
        {
            return FromMilliseconds(value.TotalMilliseconds);
        }

        public static Duration FromMilliseconds(double value)
        {
            return new Duration((int)(value / 1000),
                (uint)((Math.Abs(value) % 1000) * 1000000));
        }

        public Duration(int seconds, uint nanoSeconds)
        {
            sec = seconds;
            nanosec = nanoSeconds;
        }

        public override bool Equals(object obj)
        {
            if (!(obj is Duration))
                return false;

            return ((Duration)obj).sec == sec && ((Duration)obj).nanosec == nanosec;
        }

        public override int GetHashCode()
        {
            return (int)nanosec;
        }

        public static bool operator ==(Duration duration1, Duration duration2)
        {
            return (duration1.sec == duration2.sec && duration1.nanosec == duration2.nanosec);
        }

        public static bool operator !=(Duration duration1, Duration duration2)
        {
            return !(duration1 == duration2);
        }

        public override string ToString()
        {
            return string.Format("{0}.{1}", sec, nanosec);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Time
    {
        [MarshalAs(UnmanagedType.I4)]
        private int sec;
        public int Sec
        {
            get { return sec; }
            set { sec = value; }
        }

        [MarshalAs(UnmanagedType.U4)]
        private uint nanosec;
        public uint NanoSec
        {
            get { return nanosec; }
            set { nanosec = value; }
        }

        public const int InvalidSec = -1;
        public const uint InvalidNanoSec = 0xffffffff;

        public static readonly Time Invalid = new Time(InvalidSec, InvalidNanoSec);

        public static Time FromDateTime(DateTime value)
        {
            DateTime origin = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc);
            TimeSpan diff = value - origin;
            return new Time((int)Math.Floor(diff.TotalSeconds), (uint)(diff.Milliseconds * 1000000));
        }

        public DateTime ToDatetime()
        {
            DateTime origin = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc);
            return origin.AddSeconds(sec).AddTicks(nanosec / 100);
        }

        public Time(int _sec, uint _nanosec)
        {
            sec = _sec;
            nanosec = _nanosec;
        }

        public override bool Equals(object obj)
        {
            if (!(obj is Time))
                return false;

            return ((Time)obj).sec == sec && ((Time)obj).nanosec == nanosec;
        }

        public override int GetHashCode()
        {
            return (int)nanosec;
        }

        public static bool operator ==(Time left, Time right)
        {
            return (left.sec == right.sec && left.nanosec == right.nanosec);
        }

        public static bool operator !=(Time left, Time right)
        {
            return !(left == right);
        }

        public static bool operator >(Time left, Time right)
        {
            return (left.sec > right.sec) || (left.sec == right.sec && left.nanosec > right.nanosec);
        }

        public static bool operator <(Time left, Time right)
        {
            return right > left;
        }

        public static bool operator >=(Time left, Time right)
        {
            return (left.sec >= right.sec) || (left.sec == right.sec && left.nanosec >= right.nanosec);
        }

        public static bool operator <=(Time left, Time right)
        {
            return right >= left;
        }

        public override string ToString()
        {
            return string.Format("{0}.{1}", sec, nanosec);
        }
    }
    
    // This marshals as an int or 32-bit integer
    public struct DomainId
    {
        private int value;

        public static readonly DomainId Default = new DomainId(0x7fffffff);

        public DomainId(int id)
        {
            value = id;
        }

        public override bool Equals(object obj)
        {
            if (!(obj is DomainId))
                return false;

            return ((DomainId)obj).value == value;
        }

        public override int GetHashCode()
        {
            return (int)value;
        }

        public long ToInt32()
        {
            return value;
        }

        public static bool operator ==(DomainId id1, DomainId id2)
        {
            return (id1.value == id2.value);
        }

        public static bool operator !=(DomainId id1, DomainId id2)
        {
            return (id1.value != id2.value);
        }

        public static implicit operator DomainId(int id)
        {
            return new DomainId(id);
        }

        public static implicit operator int(DomainId id)
        {
            return id.value;
        }

        public override string ToString()
        {
            return ToString(null);
        }

        public string ToString(string format)
        {
            return value.ToString(format);
        }
    }

    // This marshals as a long or 64-bit integer
    public struct InstanceHandle
    {
        private long value;

        public static readonly InstanceHandle Nil = new InstanceHandle(0);

        public InstanceHandle(long handleVal)
        {
            value = handleVal;
        }

        public override bool Equals(object obj)
        {
            if (!(obj is InstanceHandle))
                return false;

            return ((InstanceHandle)obj).value == value;
        }

        public override int GetHashCode()
        {
            return (int)value;
        }

        public long ToInt64()
        {
            return value;
        }

        public static bool operator ==(InstanceHandle handle1, InstanceHandle handle2)
        {
            return (handle1.value == handle2.value);
        }

        public static bool operator !=(InstanceHandle handle1, InstanceHandle handle2)
        {
            return (handle1.value != handle2.value);
        }

        public static implicit operator InstanceHandle(long handleVal)
        {
            return new InstanceHandle(handleVal);
        }

        public static implicit operator long(InstanceHandle handle)
        {
            return handle.value;
        }

        public override string ToString()
        {
            return ToString(null);
        }

        public string ToString(string format)
        {
            return value.ToString(format);
        }
    }

    public struct Length
    {
        public const int Unlimited = -1;
    }

    // ----------------------------------------------------------------------
    // Status structs
    // ----------------------------------------------------------------------
    [StructLayout(LayoutKind.Sequential)]
    public class InconsistentTopicStatus
    {
        public int TotalCount;
        public int TotalCountChange;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class SampleLostStatus
    {
        public int TotalCount;
        public int TotalCountChange;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class SampleRejectedStatus
    {
        public int TotalCount;
        public int TotalCountChange;
        public SampleRejectedStatusKind LastReason;
        public InstanceHandle LastInstanceHandle;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class LivelinessLostStatus
    {
        public int TotalCount;
        public int TotalCountChange;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class LivelinessChangedStatus
    {
        public int AliveCount;
        public int NotAliveCount;
        public int AliveCountChange;
        public int NotAliveCountChange;
        public InstanceHandle LastPublicationHandle;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class OfferedDeadlineMissedStatus
    {
        public int TotalCount;
        public int TotalCountChange;
        public InstanceHandle LastInstanceHandle;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class RequestedDeadlineMissedStatus
    {
        public int TotalCount;
        public int TotalCountChange;
        public InstanceHandle LastInstanceHandle;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class OfferedIncompatibleQosStatus
    {
        public int TotalCount;
        public int TotalCountChange;
        public QosPolicyId LastPolicyId;
        public QosPolicyCount[] Policies;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class RequestedIncompatibleQosStatus
    {
        public int TotalCount;
        public int TotalCountChange;
        public QosPolicyId LastPolicyId;
        public QosPolicyCount[] Policies;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class PublicationMatchedStatus
    {
        public int TotalCount;
        public int TotalCountChange;
        public int CurrentCount;
        public int CurrentCountChange;
        public long LastSubscriptionHandle;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class SubscriptionMatchedStatus
    {
        public int TotalCount;
        public int TotalCountChange;
        public int CurrentCount;
        public int CurrentCountChange;
        public long LastPublicationHandle;
    }

    public struct QosPolicyCount
    {
        public QosPolicyId PolicyId;
        public int Count;
    }

    // ----------------------------------------------------------------------
    // Qos
    // ----------------------------------------------------------------------
    public struct QosPolicyName
    {
        public const string UserData = "UserData";
        public const string Durability = "Durability";
        public const string Presentation = "Presentation";
        public const string Deadline = "Deadline";
        public const string LatencyBudget = "LatencyBudget";
        public const string Ownership = "Ownership";
        public const string OwnershipStrength = "OwnershipStrength";
        public const string Liveliness = "Liveliness";
        public const string TimeBasedFilter = "TimeBasedFilter";
        public const string Partition = "Partition";
        public const string Reliability = "Reliability";
        public const string DestinationOrder = "DestinationOrder";
        public const string History = "History";
        public const string ResourceLimits = "ResourceLimits";
        public const string EntityFactory = "EntityFactory";
        public const string WriterDataLifecycle = "WriterDataLifecycle";
        public const string ReaderDataLifecycle = "ReaderDataLifecycle";
        public const string TopicData = "TopicData";
        public const string GroupData = "GroupData";
        public const string TransportPriority = "TransportPriority";
        public const string Lifespan = "Lifespan";
        public const string DurabilityService = "DurabilityService";
        public const string Scheduling = "Scheduling";
    }

/*    [StructLayout(LayoutKind.Sequential)]
    public struct DomainParticipantQos
    {
        public UserDataQosPolicy UserData;
        public EntityFactoryQosPolicy EntityFactory;
        public SchedulingQosPolicy WatchdogScheduling;
        public SchedulingQosPolicy ListenerScheduling;
    }

    [StructLayout(LayoutKind.Auto, Size = 200)]
    public struct UserDataQosPolicy
    {
        public byte[] Value;
    }


    public struct TopicDataQosPolicy
    {
        public byte[] Value;
    }

    public struct GroupDataQosPolicy
    {
        public byte[] Value;
    }


    public struct TransportPriorityQosPolicy
    {
        public int Value;
    }


    public struct LifespanQosPolicy
    {
        public Duration Duration;
    }

    public struct DurabilityQosPolicy
    {
        public DurabilityQosPolicyKind Kind;
    }

    public struct PresentationQosPolicy
    {
        public PresentationQosPolicyAccessScopeKind AccessScope;
        public bool CoherentAccess;
        public bool OrderedAccess;
    }


    public struct DeadlineQosPolicy
    {
        public Duration Period;
    }


    public struct LatencyBudgetQosPolicy
    {
        public Duration Duration;
    }

    public struct OwnershipQosPolicy
    {
        public OwnershipQosPolicyKind Kind;
    }

    public struct OwnershipStrengthQosPolicy
    {
        public int Value;
    }

    public struct LivelinessQosPolicy
    {
        public LivelinessQosPolicyKind Kind;
        public Duration LeaseDuration;
    }

    public struct TimeBasedFilterQosPolicy
    {
        public Duration MinimumSeparation;
    }

    public struct PartitionQosPolicy
    {
        public string[] Name;
    }

    public struct ReliabilityQosPolicy
    {
        public ReliabilityQosPolicyKind Kind;
        public Duration MaxBlockingTime;
        public bool synchronous;
    }

    public struct DestinationOrderQosPolicy
    {
        public DestinationOrderQosPolicyKind Kind;
    }

    public struct HistoryQosPolicy
    {
        public HistoryQosPolicyKind Kind;
        public int Depth;
    }

    public struct ResourceLimitsQosPolicy
    {
        public int MaxSamples;
        public int MaxInstances;
        public int MaxSamplesPerInstance;
    }

    public struct EntityFactoryQosPolicy
    {
        [MarshalAs(UnmanagedType.U1)]
        public bool AutoEnableCreatedEntities;
    }

    public struct ShareQosPolicy
    {
        public string Name;
        public bool Enable;
    }

    public struct WriterDataLifecycleQosPolicy
    {
        [MarshalAs(UnmanagedType.U1)]
        public bool AutoDisposeUnregisteredInstances;
        public Duration AutopurgeSuspendedSamplesDelay;
        public Duration AutounregisterInstanceDelay;
        
    }

    public struct ReaderDataLifecycleQosPolicy
    {
        public Duration AutoPurgeNoWriterSamplesDelay;
        public Duration AutoPurgeDisposedSamplesDelay;
        public bool EnableInvalidSamples;
    }

    public struct SubscriptionKeyQosPolicy
    {
        public bool UseKeyList;
        public string[] KeyList;
    }

    public struct ReaderLifespanQosPolicy
    {
        public bool UseLifespan;
        public Duration Duration;
    }

    public struct DurabilityServiceQosPolicy
    {
        public Duration ServiceCleanupDelay;
        public HistoryQosPolicyKind HistoryKind;
        public int HistoryDepth;
        public int MaxSamples;
        public int MaxInstances;
        public int MaxSamplesPerInstance;
    }

    public struct SchedulingClassQosPolicy
    {
        public SchedulingClassQosPolicyKind Kind;
    }

    public struct SchedulingPriorityQosPolicy
    {
        public SchedulingPriorityQosPolicyKind Kind;
    }

    public struct SchedulingQosPolicy
    {
        public SchedulingClassQosPolicy SchedulingClass;
        public SchedulingPriorityQosPolicy SchedulingPriorityKind;
        public int SchedulingPriority;
    }

    public struct DomainParticipantFactoryQos
    {
        public EntityFactoryQosPolicy EntityFactory;
    }

    public struct TopicQos
    {
        public TopicDataQosPolicy TopicData;
        public DurabilityQosPolicy Durability;
        public DurabilityServiceQosPolicy DurabilityService;
        public DeadlineQosPolicy Deadline;
        public LatencyBudgetQosPolicy LatencyBudget;
        public LivelinessQosPolicy Liveliness;
        public ReliabilityQosPolicy Reliability;
        public DestinationOrderQosPolicy DestinationOrder;
        public HistoryQosPolicy History;
        public ResourceLimitsQosPolicy ResourceLimits;
        public TransportPriorityQosPolicy TransportPriority;
        public LifespanQosPolicy Lifespan;
        public OwnershipQosPolicy Ownership;
    }

    public struct DataWriterQos
    {
        public DurabilityQosPolicy Durability;
        public DeadlineQosPolicy Deadline;
        public LatencyBudgetQosPolicy LatencyBudget;
        public LivelinessQosPolicy Liveliness;
        public ReliabilityQosPolicy Reliability;
        public DestinationOrderQosPolicy DestinationOrder;
        public HistoryQosPolicy History;
        public ResourceLimitsQosPolicy ResourceLimits;
        public TransportPriorityQosPolicy TransportPriority;
        public LifespanQosPolicy Lifespan;
        public UserDataQosPolicy UserData;
        public OwnershipQosPolicy Ownership;
        public OwnershipStrengthQosPolicy OwnershipStrength;
        public WriterDataLifecycleQosPolicy WriterDataLifecycle;
    }

    public struct PublisherQos
    {
        public PresentationQosPolicy Presentation;
        public PartitionQosPolicy Partition;
        public GroupDataQosPolicy GroupData;
        public EntityFactoryQosPolicy EntityFactory;
    }

    public struct DataReaderQos
    {
        public DurabilityQosPolicy Durability;
        public DeadlineQosPolicy Deadline;
        public LatencyBudgetQosPolicy LatencyBudget;
        public LivelinessQosPolicy Liveliness;
        public ReliabilityQosPolicy Reliability;
        public DestinationOrderQosPolicy DestinationOrder;
        public HistoryQosPolicy History;
        public ResourceLimitsQosPolicy ResourceLimits;
        public UserDataQosPolicy UserData;
        public OwnershipQosPolicy Ownership;
        public TimeBasedFilterQosPolicy TimeBasedFilter;
        public ReaderDataLifecycleQosPolicy ReaderDataLifecycle;
        public SubscriptionKeyQosPolicy SubscriptionKeys;
        public ReaderLifespanQosPolicy ReaderLifespan;
        public ShareQosPolicy Share;
    }

    public struct SubscriberQos
    {
        public PresentationQosPolicy Presentation;
        public PartitionQosPolicy Partition;
        public GroupDataQosPolicy GroupData;
        public EntityFactoryQosPolicy EntityFactory;
        public ShareQosPolicy Share;
    }

    // ----------------------------------------------------------------------
    // BuiltinTopicData
    // ----------------------------------------------------------------------
    public class ParticipantBuiltinTopicData
    {
        public BuiltinTopicKey Key;
        public UserDataQosPolicy UserData;
    }

    public class TopicBuiltinTopicData
    {
        public BuiltinTopicKey Key;
        public string Name;
        public string TypeName;
        public DurabilityQosPolicy Durability;
        public DurabilityServiceQosPolicy DurabilityService;
        public DeadlineQosPolicy Deadline;
        public LatencyBudgetQosPolicy LatencyBudget;
        public LivelinessQosPolicy Liveliness;
        public ReliabilityQosPolicy Reliability;
        public TransportPriorityQosPolicy TransportPriority;
        public LifespanQosPolicy Lifespan;
        public DestinationOrderQosPolicy DestinationOrder;
        public HistoryQosPolicy History;
        public ResourceLimitsQosPolicy ResourceLimits;
        public OwnershipQosPolicy Ownership;
        public TopicDataQosPolicy TopicData;
    }

    public class PublicationBuiltinTopicData
    {
        public BuiltinTopicKey Key;
        public BuiltinTopicKey ParticipantKey;
        public string TopicName;
        public string TypeName;
        public DurabilityQosPolicy Durability;
        public DeadlineQosPolicy Deadline;
        public LatencyBudgetQosPolicy LatencyBudget;
        public LivelinessQosPolicy Liveliness;
        public ReliabilityQosPolicy Reliability;
        public LifespanQosPolicy Lifespan;
        public UserDataQosPolicy UserData;
        public OwnershipQosPolicy Ownership;
        public OwnershipStrengthQosPolicy OwnershipStrength;
        public PresentationQosPolicy Presentation;
        public PartitionQosPolicy Partition;
        public TopicDataQosPolicy TopicData;
        public GroupDataQosPolicy GroupData;
    }

    public class SubscriptionBuiltinTopicData
    {
        public BuiltinTopicKey Key;
        public BuiltinTopicKey ParticipantKey;
        public string TopicName;
        public string TypeName;
        public DurabilityQosPolicy Durability;
        public DeadlineQosPolicy Deadline;
        public LatencyBudgetQosPolicy LatencyBudget;
        public LivelinessQosPolicy Liveliness;
        public ReliabilityQosPolicy Reliability;
        public OwnershipQosPolicy Ownership;
        public DestinationOrderQosPolicy DestinationOrder;
        public UserDataQosPolicy UserData;
        public TimeBasedFilterQosPolicy TimeBasedFilter;
        public PresentationQosPolicy Presentation;
        public PartitionQosPolicy Partition;
        public TopicDataQosPolicy TopicData;
        public GroupDataQosPolicy GroupData;
    }*/

    [StructLayoutAttribute(LayoutKind.Sequential)]
    public class SampleInfo
    {
        public SampleStateKind SampleState;
        public ViewStateKind ViewState;
        public InstanceStateKind InstanceState;
        [MarshalAs(UnmanagedType.U1)]
        public bool ValidData;
        public Time SourceTimestamp;
        public InstanceHandle InstanceHandle;
        public InstanceHandle PublicationHandle;
        public int DisposedGenerationCount;
        public int NoWritersGenerationCount;
        public int SampleRank;
        public int GenerationRank;
        public int AbsoluteGenerationRank;
        public Time ReceptionTimestamp;
    }

} // end namespace DDS
