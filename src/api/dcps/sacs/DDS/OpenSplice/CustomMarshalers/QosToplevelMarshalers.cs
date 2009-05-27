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

namespace DDS.OpenSplice.CustomMarshalers
{
    internal interface IMarshaler : IDisposable
    {
        IntPtr GapiPtr { get; }
    }

    internal class DomainParticipantQosMarshaler : IMarshaler
    {
        private static Type type = typeof(OpenSplice.Gapi.gapi_domainParticipantQos);
        public static readonly int Size = Marshal.SizeOf(type);

        private static int offset_user_data = (int)Marshal.OffsetOf(type, "user_data");
        private static int offset_entity_factory = (int)Marshal.OffsetOf(type, "entity_factory");
        private static int offset_watchdog_scheduling = (int)Marshal.OffsetOf(type, "watchdog_scheduling");
        private static int offset_listener_scheduling = (int)Marshal.OffsetOf(type, "listener_scheduling");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public DomainParticipantQosMarshaler(ref DomainParticipantQos qos)
            : this()
        {
            CopyIn(ref qos, gapiPtr);
            cleanupRequired = true;
        }

        public DomainParticipantQosMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.domainParticipantQos_alloc();
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref DomainParticipantQos from, IntPtr to)
        {
            UserDataQosPolicyMarshaler.CopyIn(ref from.UserData, to, offset_user_data);
            EntityFactoryQosPolicyMarshaler.CopyIn(ref from.EntityFactory, to, offset_entity_factory);
            SchedulingQosPolicyMarshaler.CopyIn(ref from.WatchdogScheduling, to, offset_watchdog_scheduling);
            SchedulingQosPolicyMarshaler.CopyIn(ref from.ListenerScheduling, to, offset_listener_scheduling);
        }

        internal static void CleanupIn(IntPtr nativePtr)
        {
            UserDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_user_data);
            EntityFactoryQosPolicyMarshaler.CleanupIn(nativePtr, offset_entity_factory);
            SchedulingQosPolicyMarshaler.CleanupIn(nativePtr, offset_watchdog_scheduling);
            SchedulingQosPolicyMarshaler.CleanupIn(nativePtr, offset_listener_scheduling);
        }

        internal void CopyOut(out DomainParticipantQos to)
        {
            CopyOut(gapiPtr, out to);
        }

        internal static void CopyOut(IntPtr from, out DomainParticipantQos to)
        {
            UserDataQosPolicyMarshaler.CopyOut(from, out to.UserData, offset_user_data);
            EntityFactoryQosPolicyMarshaler.CopyOut(from, out to.EntityFactory, offset_entity_factory);
            SchedulingQosPolicyMarshaler.CopyOut(from, out to.WatchdogScheduling, offset_watchdog_scheduling);
            SchedulingQosPolicyMarshaler.CopyOut(from, out to.ListenerScheduling, offset_listener_scheduling);
        }
    }

    internal class DomainParticipantFactoryQosMarshaler : IMarshaler
    {
        private static Type type = typeof(OpenSplice.Gapi.gapi_domainParticipantFactoryQos);
        public static readonly int Size = Marshal.SizeOf(type);

        private static int offset_entity_factory = (int)Marshal.OffsetOf(type, "entity_factory");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public DomainParticipantFactoryQosMarshaler(ref DomainParticipantFactoryQos qos)
            : this()
        {
            CopyIn(ref qos, gapiPtr);
            cleanupRequired = true;
        }

        public DomainParticipantFactoryQosMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.domainParticipantFactoryQos_alloc();
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref DomainParticipantFactoryQos from, IntPtr to)
        {
            EntityFactoryQosPolicyMarshaler.CopyIn(ref from.EntityFactory, to, offset_entity_factory);
        }

        internal static void CleanupIn(IntPtr nativePtr)
        {
            EntityFactoryQosPolicyMarshaler.CleanupIn(nativePtr, offset_entity_factory);
        }

        internal void CopyOut(out DomainParticipantFactoryQos to)
        {
            CopyOut(gapiPtr, out to);
        }

        internal static void CopyOut(IntPtr from, out DomainParticipantFactoryQos to)
        {
            EntityFactoryQosPolicyMarshaler.CopyOut(from, out to.EntityFactory, offset_entity_factory);
        }
    }

    internal class PublisherQosMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_publisherQos);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_presentation = (int)Marshal.OffsetOf(type, "presentation");
        private static readonly int offset_partition = (int)Marshal.OffsetOf(type, "partition");
        private static readonly int offset_group_data = (int)Marshal.OffsetOf(type, "group_data");
        private static readonly int offset_entity_factory = (int)Marshal.OffsetOf(type, "entity_factory");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public PublisherQosMarshaler(ref PublisherQos qos)
            : this()
        {
            CopyIn(ref qos, gapiPtr);
            cleanupRequired = true;
        }

        public PublisherQosMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.publisherQos_alloc();
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref PublisherQos from, IntPtr to)
        {
            PresentationQosPolicyMarshaler.CopyIn(ref from.Presentation, to, offset_presentation);
            PartitionQosPolicyMarshaler.CopyIn(ref from.Partition, to, offset_partition);
            GroupDataQosPolicyMarshaler.CopyIn(ref from.GroupData, to, offset_group_data);
            EntityFactoryQosPolicyMarshaler.CopyIn(ref from.EntityFactory, to, offset_entity_factory);
        }

        internal static void CleanupIn(IntPtr nativePtr)
        {
            PresentationQosPolicyMarshaler.CleanupIn(nativePtr, offset_presentation);
            PartitionQosPolicyMarshaler.CleanupIn(nativePtr, offset_partition);
            GroupDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_group_data);
            EntityFactoryQosPolicyMarshaler.CleanupIn(nativePtr, offset_entity_factory);
        }

        internal void CopyOut(out PublisherQos to)
        {
            CopyOut(gapiPtr, out to);
        }

        internal static void CopyOut(IntPtr from, out PublisherQos to)
        {
            PresentationQosPolicyMarshaler.CopyOut(from, out to.Presentation, offset_presentation);
            PartitionQosPolicyMarshaler.CopyOut(from, out to.Partition, offset_partition);
            GroupDataQosPolicyMarshaler.CopyOut(from, out to.GroupData, offset_group_data);
            EntityFactoryQosPolicyMarshaler.CopyOut(from, out to.EntityFactory, offset_entity_factory);
        }
    }

    internal class SubscriberQosMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_subscriberQos);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_presentation = (int)Marshal.OffsetOf(type, "presentation");
        private static readonly int offset_partition = (int)Marshal.OffsetOf(type, "partition");
        private static readonly int offset_group_data = (int)Marshal.OffsetOf(type, "group_data");
        private static readonly int offset_entity_factory = (int)Marshal.OffsetOf(type, "entity_factory");
        private static readonly int offset_share = (int)Marshal.OffsetOf(type, "share");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public SubscriberQosMarshaler(ref SubscriberQos qos)
            : this()
        {
            CopyIn(ref qos, gapiPtr);
            cleanupRequired = true;
        }

        public SubscriberQosMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.subscriberQos_alloc();
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref SubscriberQos from, IntPtr to)
        {
            PresentationQosPolicyMarshaler.CopyIn(ref from.Presentation, to, offset_presentation);
            PartitionQosPolicyMarshaler.CopyIn(ref from.Partition, to, offset_partition);
            GroupDataQosPolicyMarshaler.CopyIn(ref from.GroupData, to, offset_group_data);
            EntityFactoryQosPolicyMarshaler.CopyIn(ref from.EntityFactory, to, offset_entity_factory);
            ShareQosPolicyMarshaler.CopyIn(ref from.Share, to, offset_share);
        }

        internal static void CleanupIn(IntPtr nativePtr)
        {
            PresentationQosPolicyMarshaler.CleanupIn(nativePtr, offset_presentation);
            PartitionQosPolicyMarshaler.CleanupIn(nativePtr, offset_partition);
            GroupDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_group_data);
            EntityFactoryQosPolicyMarshaler.CleanupIn(nativePtr, offset_entity_factory);
            ShareQosPolicyMarshaler.CleanupIn(nativePtr, offset_share);

        }

        internal void CopyOut(out SubscriberQos to)
        {
            CopyOut(gapiPtr, out to);
        }

        internal static void CopyOut(IntPtr from, out SubscriberQos to)
        {
            PresentationQosPolicyMarshaler.CopyOut(from, out to.Presentation, offset_presentation);
            PartitionQosPolicyMarshaler.CopyOut(from, out to.Partition, offset_partition);
            GroupDataQosPolicyMarshaler.CopyOut(from, out to.GroupData, offset_group_data);
            EntityFactoryQosPolicyMarshaler.CopyOut(from, out to.EntityFactory, offset_entity_factory);
            ShareQosPolicyMarshaler.CopyOut(from, out to.Share, offset_share);
        }
    }

    internal class TopicQosMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_topicQos);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_topic_data = (int)Marshal.OffsetOf(type, "topic_data");
        private static readonly int offset_durability = (int)Marshal.OffsetOf(type, "durability");
        private static readonly int offset_durability_service = (int)Marshal.OffsetOf(type, "durability_service");
        private static readonly int offset_deadline = (int)Marshal.OffsetOf(type, "deadline");
        private static readonly int offset_latency_budget = (int)Marshal.OffsetOf(type, "latency_budget");
        private static readonly int offset_liveliness = (int)Marshal.OffsetOf(type, "liveliness");
        private static readonly int offset_reliability = (int)Marshal.OffsetOf(type, "reliability");
        private static readonly int offset_destination_order = (int)Marshal.OffsetOf(type, "destination_order");
        private static readonly int offset_history = (int)Marshal.OffsetOf(type, "history");
        private static readonly int offset_resource_limits = (int)Marshal.OffsetOf(type, "resource_limits");
        private static readonly int offset_transport_priority = (int)Marshal.OffsetOf(type, "transport_priority");
        private static readonly int offset_lifespan = (int)Marshal.OffsetOf(type, "lifespan");
        private static readonly int offset_ownership = (int)Marshal.OffsetOf(type, "ownership");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public TopicQosMarshaler(ref TopicQos qos)
            : this()
        {
            CopyIn(ref qos, gapiPtr);
            cleanupRequired = true;
        }

        public TopicQosMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.topicQos_alloc();
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }


        internal static void CopyIn(ref TopicQos from, IntPtr to)
        {
            TopicDataQosPolicyMarshaler.CopyIn(ref from.TopicData, to, offset_topic_data);
            DurabilityQosPolicyMarshaler.CopyIn(ref from.Durability, to, offset_durability);
            DurabilityServiceQosPolicyMarshaler.CopyIn(ref from.DurabilityService, to, offset_durability_service);
            DeadlineQosPolicyMarshaler.CopyIn(ref from.Deadline, to, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyIn(ref from.LatencyBudget, to, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CopyIn(ref from.Liveliness, to, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyIn(ref from.Reliability, to, offset_reliability);
            DestinationOrderQosPolicyMarshaler.CopyIn(ref from.DestinationOrder, to, offset_destination_order);
            HistoryQosPolicyMarshaler.CopyIn(ref from.History, to, offset_history);
            ResourceLimitsQosPolicyMarshaler.CopyIn(ref from.ResourceLimits, to, offset_resource_limits);
            TransportPriorityQosPolicyMarshaler.CopyIn(ref from.TransportPriority, to, offset_transport_priority);
            LifespanQosPolicyMarshaler.CopyIn(ref from.Lifespan, to, offset_lifespan);
            OwnershipQosPolicyMarshaler.CopyIn(ref from.Ownership, to, offset_ownership);
        }

        internal void CleanupIn(IntPtr nativePtr)
        {
            TopicDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_topic_data);
            DurabilityQosPolicyMarshaler.CleanupIn(nativePtr, offset_durability);
            DurabilityServiceQosPolicyMarshaler.CleanupIn(nativePtr, offset_durability_service);
            DeadlineQosPolicyMarshaler.CleanupIn(nativePtr, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CleanupIn(nativePtr, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CleanupIn(nativePtr, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CleanupIn(nativePtr, offset_reliability);
            DestinationOrderQosPolicyMarshaler.CleanupIn(nativePtr, offset_destination_order);
            HistoryQosPolicyMarshaler.CleanupIn(nativePtr, offset_history);
            ResourceLimitsQosPolicyMarshaler.CleanupIn(nativePtr, offset_resource_limits);
            TransportPriorityQosPolicyMarshaler.CleanupIn(nativePtr, offset_transport_priority);
            LifespanQosPolicyMarshaler.CleanupIn(nativePtr, offset_lifespan);
            OwnershipQosPolicyMarshaler.CleanupIn(nativePtr, offset_ownership);
        }

        internal void CopyOut(out TopicQos to)
        {
            CopyOut(gapiPtr, out to);
        }

        internal static void CopyOut(IntPtr from, out TopicQos to)
        {
            TopicDataQosPolicyMarshaler.CopyOut(from, out to.TopicData, offset_topic_data);
            DurabilityQosPolicyMarshaler.CopyOut(from, out to.Durability, offset_durability);
            DurabilityServiceQosPolicyMarshaler.CopyOut(from, out to.DurabilityService, offset_durability_service);
            DeadlineQosPolicyMarshaler.CopyOut(from, out to.Deadline, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyOut(from, out to.LatencyBudget, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CopyOut(from, out to.Liveliness, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyOut(from, out to.Reliability, offset_reliability);
            DestinationOrderQosPolicyMarshaler.CopyOut(from, out to.DestinationOrder, offset_destination_order);
            HistoryQosPolicyMarshaler.CopyOut(from, out to.History, offset_history);
            ResourceLimitsQosPolicyMarshaler.CopyOut(from, out to.ResourceLimits, offset_resource_limits);
            TransportPriorityQosPolicyMarshaler.CopyOut(from, out to.TransportPriority, offset_transport_priority);
            LifespanQosPolicyMarshaler.CopyOut(from, out to.Lifespan, offset_lifespan);
            OwnershipQosPolicyMarshaler.CopyOut(from, out to.Ownership, offset_ownership);
        }
    }

    internal class DataReaderQosMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_dataReaderQos);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_durability = (int)Marshal.OffsetOf(type, "durability");
        private static readonly int offset_deadline = (int)Marshal.OffsetOf(type, "deadline");
        private static readonly int offset_latency_budget = (int)Marshal.OffsetOf(type, "latency_budget");
        private static readonly int offset_liveliness = (int)Marshal.OffsetOf(type, "liveliness");
        private static readonly int offset_reliability = (int)Marshal.OffsetOf(type, "reliability");
        private static readonly int offset_destination_order = (int)Marshal.OffsetOf(type, "destination_order");
        private static readonly int offset_history = (int)Marshal.OffsetOf(type, "history");
        private static readonly int offset_resource_limits = (int)Marshal.OffsetOf(type, "resource_limits");
        private static readonly int offset_user_data = (int)Marshal.OffsetOf(type, "user_data");
        private static readonly int offset_ownership = (int)Marshal.OffsetOf(type, "ownership");
        private static readonly int offset_time_based_filter = (int)Marshal.OffsetOf(type, "time_based_filter");
        private static readonly int offset_reader_data_lifecycle = (int)Marshal.OffsetOf(type, "reader_data_lifecycle");
        private static readonly int offset_subscription_keys = (int)Marshal.OffsetOf(type, "subscription_keys");
        private static readonly int offset_reader_lifespan = (int)Marshal.OffsetOf(type, "reader_lifespan");
        private static readonly int offset_share = (int)Marshal.OffsetOf(type, "share");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public DataReaderQosMarshaler(ref DataReaderQos qos)
            : this()
        {
            CopyIn(ref qos, gapiPtr);
            cleanupRequired = true;
        }

        public DataReaderQosMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.dataReaderQos_alloc();
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref DataReaderQos from, IntPtr to)
        {
            DurabilityQosPolicyMarshaler.CopyIn(ref from.Durability, to, offset_durability);
            DeadlineQosPolicyMarshaler.CopyIn(ref from.Deadline, to, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyIn(ref from.LatencyBudget, to, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CopyIn(ref from.Liveliness, to, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyIn(ref from.Reliability, to, offset_reliability);
            DestinationOrderQosPolicyMarshaler.CopyIn(ref from.DestinationOrder, to, offset_destination_order);
            HistoryQosPolicyMarshaler.CopyIn(ref from.History, to, offset_history);
            ResourceLimitsQosPolicyMarshaler.CopyIn(ref from.ResourceLimits, to, offset_resource_limits);
            UserDataQosPolicyMarshaler.CopyIn(ref from.UserData, to, offset_user_data);
            OwnershipQosPolicyMarshaler.CopyIn(ref from.Ownership, to, offset_ownership);
            TimeBasedFilterQosPolicyMarshaler.CopyIn(ref from.TimeBasedFilter, to, offset_time_based_filter);
            ReaderDataLifecycleQosPolicyMarshaler.CopyIn(ref from.ReaderDataLifecycle, to, offset_reader_data_lifecycle);
            SubscriptionKeyQosPolicyMarshaler.CopyIn(ref from.SubscriptionKeys, to, offset_subscription_keys);
            ReaderLifespanQosPolicyMarshaler.CopyIn(ref from.ReaderLifespan, to, offset_reader_lifespan);
            ShareQosPolicyMarshaler.CopyIn(ref from.Share, to, offset_share);
        }

        internal void CleanupIn(IntPtr nativePtr)
        {
            DurabilityQosPolicyMarshaler.CleanupIn(nativePtr, offset_durability);
            DeadlineQosPolicyMarshaler.CleanupIn(nativePtr, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CleanupIn(nativePtr, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CleanupIn(nativePtr, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CleanupIn(nativePtr, offset_reliability);
            DestinationOrderQosPolicyMarshaler.CleanupIn(nativePtr, offset_destination_order);
            HistoryQosPolicyMarshaler.CleanupIn(nativePtr, offset_history);
            ResourceLimitsQosPolicyMarshaler.CleanupIn(nativePtr, offset_resource_limits);
            UserDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_user_data);
            OwnershipQosPolicyMarshaler.CleanupIn(nativePtr, offset_ownership);
            TimeBasedFilterQosPolicyMarshaler.CleanupIn(nativePtr, offset_time_based_filter);
            ReaderDataLifecycleQosPolicyMarshaler.CleanupIn(nativePtr, offset_reader_data_lifecycle);
            SubscriptionKeyQosPolicyMarshaler.CleanupIn(nativePtr, offset_subscription_keys);
            ReaderLifespanQosPolicyMarshaler.CleanupIn(nativePtr, offset_reader_lifespan);
            ShareQosPolicyMarshaler.CleanupIn(nativePtr, offset_share);
        }

        internal void CopyOut(out DataReaderQos to)
        {
            CopyOut(gapiPtr, out to);
        }

        internal static void CopyOut(IntPtr from, out DataReaderQos to)
        {
            DurabilityQosPolicyMarshaler.CopyOut(from, out to.Durability, offset_durability);
            DeadlineQosPolicyMarshaler.CopyOut(from, out to.Deadline, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyOut(from, out to.LatencyBudget, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CopyOut(from, out to.Liveliness, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyOut(from, out to.Reliability, offset_reliability);
            DestinationOrderQosPolicyMarshaler.CopyOut(from, out to.DestinationOrder, offset_destination_order);
            HistoryQosPolicyMarshaler.CopyOut(from, out to.History, offset_history);
            ResourceLimitsQosPolicyMarshaler.CopyOut(from, out to.ResourceLimits, offset_resource_limits);
            UserDataQosPolicyMarshaler.CopyOut(from, out to.UserData, offset_user_data);
            OwnershipQosPolicyMarshaler.CopyOut(from, out to.Ownership, offset_ownership);
            TimeBasedFilterQosPolicyMarshaler.CopyOut(from, out to.TimeBasedFilter, offset_time_based_filter);
            ReaderDataLifecycleQosPolicyMarshaler.CopyOut(from, out to.ReaderDataLifecycle, offset_reader_data_lifecycle);
            SubscriptionKeyQosPolicyMarshaler.CopyOut(from, out to.SubscriptionKeys, offset_subscription_keys);
            ReaderLifespanQosPolicyMarshaler.CopyOut(from, out to.ReaderLifespan, offset_reader_lifespan);
            ShareQosPolicyMarshaler.CopyOut(from, out to.Share, offset_share);
        }
    }

    internal class DataWriterQosMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_dataWriterQos);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_durability = (int)Marshal.OffsetOf(type, "durability");
        private static readonly int offset_deadline = (int)Marshal.OffsetOf(type, "deadline");
        private static readonly int offset_latency_budget = (int)Marshal.OffsetOf(type, "latency_budget");
        private static readonly int offset_liveliness = (int)Marshal.OffsetOf(type, "liveliness");
        private static readonly int offset_reliability = (int)Marshal.OffsetOf(type, "reliability");
        private static readonly int offset_destination_order = (int)Marshal.OffsetOf(type, "destination_order");
        private static readonly int offset_history = (int)Marshal.OffsetOf(type, "history");
        private static readonly int offset_resource_limits = (int)Marshal.OffsetOf(type, "resource_limits");
        private static readonly int offset_transport_priority = (int)Marshal.OffsetOf(type, "transport_priority");
        private static readonly int offset_lifespan = (int)Marshal.OffsetOf(type, "lifespan");
        private static readonly int offset_user_data = (int)Marshal.OffsetOf(type, "user_data");
        private static readonly int offset_ownership = (int)Marshal.OffsetOf(type, "ownership");
        private static readonly int offset_ownership_strength = (int)Marshal.OffsetOf(type, "ownership_strength");
        private static readonly int offset_writer_data_lifecycle = (int)Marshal.OffsetOf(type, "writer_data_lifecycle");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public DataWriterQosMarshaler(ref DataWriterQos qos)
            : this()
        {
            CopyIn(ref qos, gapiPtr);
            cleanupRequired = true;
        }

        public DataWriterQosMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.dataWriterQos_alloc();
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref DataWriterQos from, IntPtr to)
        {
            DurabilityQosPolicyMarshaler.CopyIn(ref from.Durability, to, offset_durability);
            DeadlineQosPolicyMarshaler.CopyIn(ref from.Deadline, to, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyIn(ref from.LatencyBudget, to, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CopyIn(ref from.Liveliness, to, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyIn(ref from.Reliability, to, offset_reliability);
            DestinationOrderQosPolicyMarshaler.CopyIn(ref from.DestinationOrder, to, offset_destination_order);
            HistoryQosPolicyMarshaler.CopyIn(ref from.History, to, offset_history);
            ResourceLimitsQosPolicyMarshaler.CopyIn(ref from.ResourceLimits, to, offset_resource_limits);
            TransportPriorityQosPolicyMarshaler.CopyIn(ref from.TransportPriority, to, offset_transport_priority);
            LifespanQosPolicyMarshaler.CopyIn(ref from.Lifespan, to, offset_lifespan);
            UserDataQosPolicyMarshaler.CopyIn(ref from.UserData, to, offset_user_data);
            OwnershipQosPolicyMarshaler.CopyIn(ref from.Ownership, to, offset_ownership);
            OwnershipStrengthQosPolicyMarshaler.CopyIn(ref from.OwnershipStrength, to, offset_ownership_strength);
            WriterDataLifecycleQosPolicyMarshaler.CopyIn(ref from.WriterDataLifecycle, to, offset_writer_data_lifecycle);
        }

        internal void CleanupIn(IntPtr nativePtr)
        {
            DurabilityQosPolicyMarshaler.CleanupIn(nativePtr, offset_durability);
            DeadlineQosPolicyMarshaler.CleanupIn(nativePtr, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CleanupIn(nativePtr, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CleanupIn(nativePtr, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CleanupIn(nativePtr, offset_reliability);
            DestinationOrderQosPolicyMarshaler.CleanupIn(nativePtr, offset_destination_order);
            HistoryQosPolicyMarshaler.CleanupIn(nativePtr, offset_history);
            ResourceLimitsQosPolicyMarshaler.CleanupIn(nativePtr, offset_resource_limits);
            TransportPriorityQosPolicyMarshaler.CleanupIn(nativePtr, offset_transport_priority);
            LifespanQosPolicyMarshaler.CleanupIn(nativePtr, offset_lifespan);
            UserDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_user_data);
            OwnershipQosPolicyMarshaler.CleanupIn(nativePtr, offset_ownership);
            OwnershipStrengthQosPolicyMarshaler.CleanupIn(nativePtr, offset_ownership_strength);
            WriterDataLifecycleQosPolicyMarshaler.CleanupIn(nativePtr, offset_writer_data_lifecycle);
        }

        internal void CopyOut(out DataWriterQos to)
        {
            CopyOut(gapiPtr, out to);
        }

        internal static void CopyOut(IntPtr from, out DataWriterQos to)
        {
            DurabilityQosPolicyMarshaler.CopyOut(from, out to.Durability, offset_durability);
            DeadlineQosPolicyMarshaler.CopyOut(from, out to.Deadline, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyOut(from, out to.LatencyBudget, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CopyOut(from, out to.Liveliness, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyOut(from, out to.Reliability, offset_reliability);
            DestinationOrderQosPolicyMarshaler.CopyOut(from, out to.DestinationOrder, offset_destination_order);
            HistoryQosPolicyMarshaler.CopyOut(from, out to.History, offset_history);
            ResourceLimitsQosPolicyMarshaler.CopyOut(from, out to.ResourceLimits, offset_resource_limits);
            TransportPriorityQosPolicyMarshaler.CopyOut(from, out to.TransportPriority, offset_transport_priority);
            LifespanQosPolicyMarshaler.CopyOut(from, out to.Lifespan, offset_lifespan);
            UserDataQosPolicyMarshaler.CopyOut(from, out to.UserData, offset_user_data);
            OwnershipQosPolicyMarshaler.CopyOut(from, out to.Ownership, offset_ownership);
            OwnershipStrengthQosPolicyMarshaler.CopyOut(from, out to.OwnershipStrength, offset_ownership_strength);
            WriterDataLifecycleQosPolicyMarshaler.CopyOut(from, out to.WriterDataLifecycle, offset_writer_data_lifecycle);
        }
    }

    internal class TopicBulitinTopicDataMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_topicBuiltinTopicData);
        public static readonly int Size = Marshal.SizeOf(type);

        private static int offset_key = (int)Marshal.OffsetOf(type, "key");
        private static int offset_name = (int)Marshal.OffsetOf(type, "name");
        private static int offset_type_name = (int)Marshal.OffsetOf(type, "type_name");
        private static int offset_durability = (int)Marshal.OffsetOf(type, "durability");
        private static int offset_durability_service = (int)Marshal.OffsetOf(type, "durability_service");
        private static int offset_deadline = (int)Marshal.OffsetOf(type, "deadline");
        private static int offset_latency_budget = (int)Marshal.OffsetOf(type, "latency_budget");
        private static int offset_liveliness = (int)Marshal.OffsetOf(type, "liveliness");
        private static int offset_reliability = (int)Marshal.OffsetOf(type, "reliability");
        private static int offset_transport_priority = (int)Marshal.OffsetOf(type, "transport_priority");
        private static int offset_lifespan = (int)Marshal.OffsetOf(type, "lifespan");
        private static int offset_destination_order = (int)Marshal.OffsetOf(type, "destination_order");
        private static int offset_history = (int)Marshal.OffsetOf(type, "history");
        private static int offset_resource_limits = (int)Marshal.OffsetOf(type, "resource_limits");
        private static int offset_ownership = (int)Marshal.OffsetOf(type, "ownership");
        private static int offset_topic_data = (int)Marshal.OffsetOf(type, "topic_data");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public TopicBulitinTopicDataMarshaler(ref TopicBuiltinTopicData qos)
            : this()
        {
            CopyIn(ref qos, gapiPtr);
            cleanupRequired = true;
        }

        public TopicBulitinTopicDataMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref TopicBuiltinTopicData from, IntPtr to)
        {
            BuiltinTopicKeyMarshaler.CopyIn(ref from.Key, to, offset_key);

            IntPtr namePtr = Marshal.StringToHGlobalAnsi(from.Name);
            TypeSupport.Write(to, offset_name, namePtr);

            IntPtr typeNamePtr = Marshal.StringToHGlobalAnsi(from.TypeName);
            TypeSupport.Write(to, offset_type_name, typeNamePtr);

            DurabilityQosPolicyMarshaler.CopyIn(ref from.Durability, to, offset_durability);
            DurabilityServiceQosPolicyMarshaler.CopyIn(ref from.DurabilityService, to, offset_durability_service);
            DeadlineQosPolicyMarshaler.CopyIn(ref from.Deadline, to, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyIn(ref from.LatencyBudget, to, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CopyIn(ref from.Liveliness, to, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyIn(ref from.Reliability, to, offset_reliability);
            DestinationOrderQosPolicyMarshaler.CopyIn(ref from.DestinationOrder, to, offset_destination_order);
            TransportPriorityQosPolicyMarshaler.CopyIn(ref from.TransportPriority, to, offset_transport_priority);
            LifespanQosPolicyMarshaler.CopyIn(ref from.Lifespan, to, offset_lifespan);
            DestinationOrderQosPolicyMarshaler.CopyIn(ref from.DestinationOrder, to, offset_destination_order);
            HistoryQosPolicyMarshaler.CopyIn(ref from.History, to, offset_history);
            ResourceLimitsQosPolicyMarshaler.CopyIn(ref from.ResourceLimits, to, offset_resource_limits);
            OwnershipQosPolicyMarshaler.CopyIn(ref from.Ownership, to, offset_ownership);
            TopicDataQosPolicyMarshaler.CopyIn(ref from.TopicData, to, offset_topic_data);
        }

        internal void CleanupIn(IntPtr nativePtr)
        {
            BuiltinTopicKeyMarshaler.CleanupIn(nativePtr, offset_key);

            IntPtr namePtr = TypeSupport.ReadIntPtr(nativePtr, offset_name);
            Marshal.FreeHGlobal(namePtr);
            TypeSupport.Write(nativePtr, offset_name, IntPtr.Zero);

            IntPtr typeNamePtr = TypeSupport.ReadIntPtr(nativePtr, offset_type_name);
            Marshal.FreeHGlobal(typeNamePtr);
            TypeSupport.Write(nativePtr, offset_type_name, IntPtr.Zero);

            DurabilityQosPolicyMarshaler.CleanupIn(nativePtr, offset_durability);
            DeadlineQosPolicyMarshaler.CleanupIn(nativePtr, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CleanupIn(nativePtr, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CleanupIn(nativePtr, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CleanupIn(nativePtr, offset_reliability);
            DestinationOrderQosPolicyMarshaler.CleanupIn(nativePtr, offset_destination_order);
            TransportPriorityQosPolicyMarshaler.CleanupIn(nativePtr, offset_transport_priority);
            LifespanQosPolicyMarshaler.CleanupIn(nativePtr, offset_lifespan);
            DestinationOrderQosPolicyMarshaler.CleanupIn(nativePtr, offset_destination_order);
            HistoryQosPolicyMarshaler.CleanupIn(nativePtr, offset_history);
            ResourceLimitsQosPolicyMarshaler.CleanupIn(nativePtr, offset_resource_limits);
            OwnershipQosPolicyMarshaler.CleanupIn(nativePtr, offset_ownership);
            TopicDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_topic_data);
        }

        internal void CopyOut(out TopicBuiltinTopicData to)
        {
            CopyOut(gapiPtr, out to);
        }

        internal static void CopyOut(IntPtr from, out TopicBuiltinTopicData to)
        {
            to = new TopicBuiltinTopicData();

            BuiltinTopicKeyMarshaler.CopyOut(from, out to.Key, offset_key);

            IntPtr namePtr = TypeSupport.ReadIntPtr(from, offset_name);
            to.Name = Marshal.PtrToStringAnsi(namePtr);

            IntPtr typeNamePtr = TypeSupport.ReadIntPtr(from, offset_type_name);
            to.TypeName = Marshal.PtrToStringAnsi(typeNamePtr);

            DurabilityQosPolicyMarshaler.CopyOut(from, out to.Durability, offset_durability);
            DeadlineQosPolicyMarshaler.CopyOut(from, out to.Deadline, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyOut(from, out to.LatencyBudget, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CopyOut(from, out to.Liveliness, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyOut(from, out to.Reliability, offset_reliability);
            TransportPriorityQosPolicyMarshaler.CopyOut(from, out to.TransportPriority, offset_transport_priority);
            LifespanQosPolicyMarshaler.CopyOut(from, out to.Lifespan, offset_lifespan);
            DestinationOrderQosPolicyMarshaler.CopyOut(from, out to.DestinationOrder, offset_destination_order);
            HistoryQosPolicyMarshaler.CopyOut(from, out to.History, offset_history);
            ResourceLimitsQosPolicyMarshaler.CopyOut(from, out to.ResourceLimits, offset_resource_limits);
            OwnershipQosPolicyMarshaler.CopyOut(from, out to.Ownership, offset_ownership);
            TopicDataQosPolicyMarshaler.CopyOut(from, out to.TopicData, offset_topic_data);
        }
    }

    internal class PublicationBuiltinTopicDataMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_publicationBuiltinTopicData);
        public static readonly int Size = Marshal.SizeOf(type);

        private static int offset_key = (int)Marshal.OffsetOf(type, "key");
        private static int offset_participant_key = (int)Marshal.OffsetOf(type, "participant_key");
        private static int offset_topic_name = (int)Marshal.OffsetOf(type, "topic_name");
        private static int offset_type_name = (int)Marshal.OffsetOf(type, "type_name");
        private static int offset_durability = (int)Marshal.OffsetOf(type, "durability");
        private static int offset_deadline = (int)Marshal.OffsetOf(type, "deadline");
        private static int offset_latency_budget = (int)Marshal.OffsetOf(type, "latency_budget");
        private static int offset_liveliness = (int)Marshal.OffsetOf(type, "liveliness");
        private static int offset_reliability = (int)Marshal.OffsetOf(type, "reliability");
        private static int offset_lifespan = (int)Marshal.OffsetOf(type, "lifespan");
        private static int offset_user_data = (int)Marshal.OffsetOf(type, "user_data");
        private static int offset_ownership = (int)Marshal.OffsetOf(type, "ownership");
        private static int offset_ownership_strength = (int)Marshal.OffsetOf(type, "ownership_strength");
        private static int offset_presentation = (int)Marshal.OffsetOf(type, "presentation");
        private static int offset_partition = (int)Marshal.OffsetOf(type, "partition");
        private static int offset_topic_data = (int)Marshal.OffsetOf(type, "topic_data");
        private static int offset_group_data = (int)Marshal.OffsetOf(type, "group_data");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public PublicationBuiltinTopicDataMarshaler(ref PublicationBuiltinTopicData qos)
            : this()
        {
            CopyIn(ref qos, gapiPtr);
            cleanupRequired = true;
        }

        public PublicationBuiltinTopicDataMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref PublicationBuiltinTopicData from, IntPtr to)
        {
            BuiltinTopicKeyMarshaler.CopyIn(ref from.Key, to, offset_key);
            BuiltinTopicKeyMarshaler.CopyIn(ref from.ParticipantKey, to, offset_participant_key);

            IntPtr namePtr = Marshal.StringToHGlobalAnsi(from.TopicName);
            TypeSupport.Write(to, offset_topic_name, namePtr);

            IntPtr typeNamePtr = Marshal.StringToHGlobalAnsi(from.TypeName);
            TypeSupport.Write(to, offset_type_name, typeNamePtr);

            DurabilityQosPolicyMarshaler.CopyIn(ref from.Durability, to, offset_durability);
            DeadlineQosPolicyMarshaler.CopyIn(ref from.Deadline, to, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyIn(ref from.LatencyBudget, to, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CopyIn(ref from.Liveliness, to, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyIn(ref from.Reliability, to, offset_reliability);
            LifespanQosPolicyMarshaler.CopyIn(ref from.Lifespan, to, offset_lifespan);
            UserDataQosPolicyMarshaler.CopyIn(ref from.UserData, to, offset_user_data);
            OwnershipQosPolicyMarshaler.CopyIn(ref from.Ownership, to, offset_ownership);
            OwnershipStrengthQosPolicyMarshaler.CopyIn(ref from.OwnershipStrength, to, offset_ownership_strength);
            PresentationQosPolicyMarshaler.CopyIn(ref from.Presentation, to, offset_presentation);
            PartitionQosPolicyMarshaler.CopyIn(ref from.Partition, to, offset_partition);
            TopicDataQosPolicyMarshaler.CopyIn(ref from.TopicData, to, offset_topic_data);
            GroupDataQosPolicyMarshaler.CopyIn(ref from.GroupData, to, offset_group_data);
        }

        internal void CleanupIn(IntPtr nativePtr)
        {
            BuiltinTopicKeyMarshaler.CleanupIn(nativePtr, offset_key);
            BuiltinTopicKeyMarshaler.CleanupIn(nativePtr, offset_participant_key);

            IntPtr namePtr = TypeSupport.ReadIntPtr(nativePtr, offset_topic_name);
            Marshal.FreeHGlobal(namePtr);
            TypeSupport.Write(nativePtr, offset_topic_name, IntPtr.Zero);

            IntPtr typeNamePtr = TypeSupport.ReadIntPtr(nativePtr, offset_type_name);
            Marshal.FreeHGlobal(typeNamePtr);
            TypeSupport.Write(nativePtr, offset_type_name, IntPtr.Zero);

            DurabilityQosPolicyMarshaler.CleanupIn(nativePtr, offset_durability);
            DeadlineQosPolicyMarshaler.CleanupIn(nativePtr, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CleanupIn(nativePtr, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CleanupIn(nativePtr, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CleanupIn(nativePtr, offset_reliability);
            LifespanQosPolicyMarshaler.CleanupIn(nativePtr, offset_lifespan);
            UserDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_user_data);
            OwnershipQosPolicyMarshaler.CleanupIn(nativePtr, offset_ownership);
            OwnershipStrengthQosPolicyMarshaler.CleanupIn(nativePtr, offset_ownership_strength);
            PresentationQosPolicyMarshaler.CleanupIn(nativePtr, offset_presentation);
            PartitionQosPolicyMarshaler.CleanupIn(nativePtr, offset_partition);
            TopicDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_topic_data);
            GroupDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_group_data);
        }

        internal void CopyOut(out PublicationBuiltinTopicData to)
        {
            CopyOut(gapiPtr, out to);
        }

        internal static void CopyOut(IntPtr from, out PublicationBuiltinTopicData to)
        {
            to = new PublicationBuiltinTopicData();

            BuiltinTopicKeyMarshaler.CopyOut(from, out to.Key, offset_key);
            BuiltinTopicKeyMarshaler.CopyOut(from, out to.ParticipantKey, offset_participant_key);

            IntPtr namePtr = TypeSupport.ReadIntPtr(from, offset_topic_name);
            to.TopicName = Marshal.PtrToStringAnsi(namePtr);

            IntPtr typeNamePtr = TypeSupport.ReadIntPtr(from, offset_type_name);
            to.TypeName = Marshal.PtrToStringAnsi(typeNamePtr);

            DurabilityQosPolicyMarshaler.CopyOut(from, out to.Durability, offset_durability);
            DeadlineQosPolicyMarshaler.CopyOut(from, out to.Deadline, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyOut(from, out to.LatencyBudget, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CopyOut(from, out to.Liveliness, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyOut(from, out to.Reliability, offset_reliability);
            LifespanQosPolicyMarshaler.CopyOut(from, out to.Lifespan, offset_lifespan);
            UserDataQosPolicyMarshaler.CopyOut(from, out to.UserData, offset_user_data);
            OwnershipQosPolicyMarshaler.CopyOut(from, out to.Ownership, offset_ownership);
            OwnershipStrengthQosPolicyMarshaler.CopyOut(from, out to.OwnershipStrength, offset_ownership_strength);
            PresentationQosPolicyMarshaler.CopyOut(from, out to.Presentation, offset_presentation);
            PartitionQosPolicyMarshaler.CopyOut(from, out to.Partition, offset_partition);
            TopicDataQosPolicyMarshaler.CopyOut(from, out to.TopicData, offset_topic_data);
            GroupDataQosPolicyMarshaler.CopyOut(from, out to.GroupData, offset_group_data);
        }
    }

    internal class SubscriptionBuiltinTopicDataMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_subscriptionBuiltinTopicData);
        public static readonly int Size = Marshal.SizeOf(type);

        private static int offset_key = (int)Marshal.OffsetOf(type, "key");
        private static int offset_participant_key = (int)Marshal.OffsetOf(type, "participant_key");
        private static int offset_topic_name = (int)Marshal.OffsetOf(type, "topic_name");
        private static int offset_type_name = (int)Marshal.OffsetOf(type, "type_name");
        private static int offset_durability = (int)Marshal.OffsetOf(type, "durability");
        private static int offset_deadline = (int)Marshal.OffsetOf(type, "deadline");
        private static int offset_latency_budget = (int)Marshal.OffsetOf(type, "latency_budget");
        private static int offset_liveliness = (int)Marshal.OffsetOf(type, "liveliness");
        private static int offset_reliability = (int)Marshal.OffsetOf(type, "reliability");
        private static int offset_ownership = (int)Marshal.OffsetOf(type, "ownership");
        private static int offset_destination_order = (int)Marshal.OffsetOf(type, "destination_order");
        private static int offset_user_data = (int)Marshal.OffsetOf(type, "user_data");
        private static int offset_time_based_filter = (int)Marshal.OffsetOf(type, "time_based_filter");
        private static int offset_presentation = (int)Marshal.OffsetOf(type, "presentation");
        private static int offset_partition = (int)Marshal.OffsetOf(type, "partition");
        private static int offset_topic_data = (int)Marshal.OffsetOf(type, "topic_data");
        private static int offset_group_data = (int)Marshal.OffsetOf(type, "group_data");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public SubscriptionBuiltinTopicDataMarshaler(ref SubscriptionBuiltinTopicData qos)
            : this()
        {
            CopyIn(ref qos, gapiPtr);
            cleanupRequired = true;
        }

        public SubscriptionBuiltinTopicDataMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref SubscriptionBuiltinTopicData from, IntPtr to)
        {
            BuiltinTopicKeyMarshaler.CopyIn(ref from.Key, to, offset_key);
            BuiltinTopicKeyMarshaler.CopyIn(ref from.ParticipantKey, to, offset_participant_key);

            IntPtr namePtr = Marshal.StringToHGlobalAnsi(from.TopicName);
            TypeSupport.Write(to, offset_topic_name, namePtr);

            IntPtr typeNamePtr = Marshal.StringToHGlobalAnsi(from.TypeName);
            TypeSupport.Write(to, offset_type_name, typeNamePtr);

            DurabilityQosPolicyMarshaler.CopyIn(ref from.Durability, to, offset_durability);
            DeadlineQosPolicyMarshaler.CopyIn(ref from.Deadline, to, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyIn(ref from.LatencyBudget, to, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CopyIn(ref from.Liveliness, to, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyIn(ref from.Reliability, to, offset_reliability);
            OwnershipQosPolicyMarshaler.CopyIn(ref from.Ownership, to, offset_ownership);
            DestinationOrderQosPolicyMarshaler.CopyIn(ref from.DestinationOrder, to, offset_destination_order);
            UserDataQosPolicyMarshaler.CopyIn(ref from.UserData, to, offset_user_data);
            TimeBasedFilterQosPolicyMarshaler.CopyIn(ref from.TimeBasedFilter, to, offset_time_based_filter);
            PresentationQosPolicyMarshaler.CopyIn(ref from.Presentation, to, offset_presentation);
            PartitionQosPolicyMarshaler.CopyIn(ref from.Partition, to, offset_partition);
            TopicDataQosPolicyMarshaler.CopyIn(ref from.TopicData, to, offset_topic_data);
            GroupDataQosPolicyMarshaler.CopyIn(ref from.GroupData, to, offset_group_data);
        }

        internal void CleanupIn(IntPtr nativePtr)
        {
            BuiltinTopicKeyMarshaler.CleanupIn(nativePtr, offset_key);
            BuiltinTopicKeyMarshaler.CleanupIn(nativePtr, offset_participant_key);

            IntPtr namePtr = TypeSupport.ReadIntPtr(nativePtr, offset_topic_name);
            Marshal.FreeHGlobal(namePtr);
            TypeSupport.Write(nativePtr, offset_topic_name, IntPtr.Zero);

            IntPtr typeNamePtr = TypeSupport.ReadIntPtr(nativePtr, offset_type_name);
            Marshal.FreeHGlobal(typeNamePtr);
            TypeSupport.Write(nativePtr, offset_type_name, IntPtr.Zero);

            DurabilityQosPolicyMarshaler.CleanupIn(nativePtr, offset_durability);
            DeadlineQosPolicyMarshaler.CleanupIn(nativePtr, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CleanupIn(nativePtr, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CleanupIn(nativePtr, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CleanupIn(nativePtr, offset_reliability);
            OwnershipQosPolicyMarshaler.CleanupIn(nativePtr, offset_ownership);
            DestinationOrderQosPolicyMarshaler.CleanupIn(nativePtr, offset_destination_order);
            UserDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_user_data);
            TimeBasedFilterQosPolicyMarshaler.CleanupIn(nativePtr, offset_time_based_filter);
            PresentationQosPolicyMarshaler.CleanupIn(nativePtr, offset_presentation);
            PartitionQosPolicyMarshaler.CleanupIn(nativePtr, offset_partition);
            TopicDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_topic_data);
            GroupDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_group_data);
        }

        internal void CopyOut(out SubscriptionBuiltinTopicData to)
        {
            CopyOut(gapiPtr, out to);
        }

        internal static void CopyOut(IntPtr from, out SubscriptionBuiltinTopicData to)
        {
            to = new SubscriptionBuiltinTopicData();

            BuiltinTopicKeyMarshaler.CopyOut(from, out to.Key, offset_key);
            BuiltinTopicKeyMarshaler.CopyOut(from, out to.ParticipantKey, offset_participant_key);

            IntPtr namePtr = TypeSupport.ReadIntPtr(from, offset_topic_name);
            to.TopicName = Marshal.PtrToStringAnsi(namePtr);

            IntPtr typeNamePtr = TypeSupport.ReadIntPtr(from, offset_type_name);
            to.TypeName = Marshal.PtrToStringAnsi(typeNamePtr);

            DurabilityQosPolicyMarshaler.CopyOut(from, out to.Durability, offset_durability);
            DeadlineQosPolicyMarshaler.CopyOut(from, out to.Deadline, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyOut(from, out to.LatencyBudget, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CopyOut(from, out to.Liveliness, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyOut(from, out to.Reliability, offset_reliability);
            OwnershipQosPolicyMarshaler.CopyOut(from, out to.Ownership, offset_ownership);
            DestinationOrderQosPolicyMarshaler.CopyOut(from, out to.DestinationOrder, offset_destination_order);
            UserDataQosPolicyMarshaler.CopyOut(from, out to.UserData, offset_user_data);
            TimeBasedFilterQosPolicyMarshaler.CopyOut(from, out to.TimeBasedFilter, offset_time_based_filter);
            PresentationQosPolicyMarshaler.CopyOut(from, out to.Presentation, offset_presentation);
            PartitionQosPolicyMarshaler.CopyOut(from, out to.Partition, offset_partition);
            TopicDataQosPolicyMarshaler.CopyOut(from, out to.TopicData, offset_topic_data);
            GroupDataQosPolicyMarshaler.CopyOut(from, out to.GroupData, offset_group_data);
        }
    }
}