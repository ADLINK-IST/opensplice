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
using DDS.OpenSplice;

namespace DDS.OpenSplice.CustomMarshalers
{
    internal class DomainParticipantQosMarshaler : GapiMarshaler
    {
        private static Type type = typeof(OpenSplice.Gapi.gapi_domainParticipantQos);
        public static readonly int Size = Marshal.SizeOf(type);

        private static int offset_user_data = (int)Marshal.OffsetOf(type, "user_data");
        private static int offset_entity_factory = (int)Marshal.OffsetOf(type, "entity_factory");
        private static int offset_watchdog_scheduling = (int)Marshal.OffsetOf(type, "watchdog_scheduling");
        private static int offset_listener_scheduling = (int)Marshal.OffsetOf(type, "listener_scheduling");

        public DomainParticipantQosMarshaler() : 
                base(Gapi.GenericAllocRelease.domainParticipantQos_alloc())
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(DomainParticipantQos from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr);
        }

        internal static DDS.ReturnCode CopyIn(DomainParticipantQos from, IntPtr to)
        {
            DDS.ReturnCode result;
            if (from != null) {
                result = UserDataQosPolicyMarshaler.CopyIn(from.UserData, to, offset_user_data);
                if (result == DDS.ReturnCode.Ok) {
                    result = EntityFactoryQosPolicyMarshaler.CopyIn(
                            from.EntityFactory, to, offset_entity_factory);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = SchedulingQosPolicyMarshaler.CopyIn(
                            from.WatchdogScheduling, to, offset_watchdog_scheduling);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = SchedulingQosPolicyMarshaler.CopyIn(
                            from.ListenerScheduling, to, offset_listener_scheduling);
                }
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosToplevelMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "DomainParticipantQos attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(IntPtr nativePtr)
        {
            UserDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_user_data);
            EntityFactoryQosPolicyMarshaler.CleanupIn(nativePtr, offset_entity_factory);
            SchedulingQosPolicyMarshaler.CleanupIn(nativePtr, offset_watchdog_scheduling);
            SchedulingQosPolicyMarshaler.CleanupIn(nativePtr, offset_listener_scheduling);
        }

        internal void CopyOut(ref DomainParticipantQos to)
        {
            CopyOut(GapiPtr, ref to);
        }

        internal static void CopyOut(IntPtr from, ref DomainParticipantQos to)
        {
            if (to == null) to = new DomainParticipantQos();
            UserDataQosPolicyMarshaler.CopyOut(from, ref to.UserData, offset_user_data);
            EntityFactoryQosPolicyMarshaler.CopyOut(from, ref to.EntityFactory, offset_entity_factory);
            SchedulingQosPolicyMarshaler.CopyOut(from, ref to.WatchdogScheduling, offset_watchdog_scheduling);
            SchedulingQosPolicyMarshaler.CopyOut(from, ref to.ListenerScheduling, offset_listener_scheduling);
        }
    }

    internal class DomainParticipantFactoryQosMarshaler : GapiMarshaler
    {
        private static Type type = typeof(OpenSplice.Gapi.gapi_domainParticipantFactoryQos);
        public static readonly int Size = Marshal.SizeOf(type);

        private static int offset_entity_factory = (int)Marshal.OffsetOf(type, "entity_factory");

        public DomainParticipantFactoryQosMarshaler() :
                base(Gapi.GenericAllocRelease.domainParticipantFactoryQos_alloc())
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(DomainParticipantFactoryQos from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr);
        }

        internal static DDS.ReturnCode CopyIn(DomainParticipantFactoryQos from, IntPtr to)
        {
            DDS.ReturnCode result;
            if (from != null) {
                result = EntityFactoryQosPolicyMarshaler.CopyIn(from.EntityFactory, to, offset_entity_factory);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.DomainParticipantFactoryQosMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosToplevelMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "DomainParticipantFactoryQos attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(IntPtr nativePtr)
        {
            EntityFactoryQosPolicyMarshaler.CleanupIn(nativePtr, offset_entity_factory);
        }

        internal void CopyOut(ref DomainParticipantFactoryQos to)
        {
            CopyOut(GapiPtr, ref to);
        }

        internal static void CopyOut(IntPtr from, ref DomainParticipantFactoryQos to)
        {
            if (to == null) to = new DomainParticipantFactoryQos();
            EntityFactoryQosPolicyMarshaler.CopyOut(from, ref to.EntityFactory, offset_entity_factory);
        }
    }

    internal class PublisherQosMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_publisherQos);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_presentation = (int)Marshal.OffsetOf(type, "presentation");
        private static readonly int offset_partition = (int)Marshal.OffsetOf(type, "partition");
        private static readonly int offset_group_data = (int)Marshal.OffsetOf(type, "group_data");
        private static readonly int offset_entity_factory = (int)Marshal.OffsetOf(type, "entity_factory");

        public PublisherQosMarshaler() :
                base(Gapi.GenericAllocRelease.publisherQos_alloc())
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(PublisherQos from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr);
        }

        internal static DDS.ReturnCode CopyIn(PublisherQos from, IntPtr to)
        {
            DDS.ReturnCode result;
            if (from != null) {
                result = PresentationQosPolicyMarshaler.CopyIn(
                        from.Presentation, to, offset_presentation);
                if (result == DDS.ReturnCode.Ok) {
                    result = PartitionQosPolicyMarshaler.CopyIn(
                            from.Partition, to, offset_partition);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = GroupDataQosPolicyMarshaler.CopyIn(
                            from.GroupData, to, offset_group_data);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = EntityFactoryQosPolicyMarshaler.CopyIn(
                            from.EntityFactory, to, offset_entity_factory);
                }
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.PublisherQosMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosToplevelMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "PublisherQos attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(IntPtr nativePtr)
        {
            PresentationQosPolicyMarshaler.CleanupIn(nativePtr, offset_presentation);
            PartitionQosPolicyMarshaler.CleanupIn(nativePtr, offset_partition);
            GroupDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_group_data);
            EntityFactoryQosPolicyMarshaler.CleanupIn(nativePtr, offset_entity_factory);
        }

        internal void CopyOut(ref PublisherQos to)
        {
            CopyOut(GapiPtr, ref to);
        }

        internal static void CopyOut(IntPtr from, ref PublisherQos to)
        {
            if (to == null) to = new PublisherQos();
            PresentationQosPolicyMarshaler.CopyOut(from, ref to.Presentation, offset_presentation);
            PartitionQosPolicyMarshaler.CopyOut(from, ref to.Partition, offset_partition);
            GroupDataQosPolicyMarshaler.CopyOut(from, ref to.GroupData, offset_group_data);
            EntityFactoryQosPolicyMarshaler.CopyOut(from, ref to.EntityFactory, offset_entity_factory);
        }
    }

    internal class SubscriberQosMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_subscriberQos);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_presentation = (int)Marshal.OffsetOf(type, "presentation");
        private static readonly int offset_partition = (int)Marshal.OffsetOf(type, "partition");
        private static readonly int offset_group_data = (int)Marshal.OffsetOf(type, "group_data");
        private static readonly int offset_entity_factory = (int)Marshal.OffsetOf(type, "entity_factory");
        private static readonly int offset_share = (int)Marshal.OffsetOf(type, "share");

        public SubscriberQosMarshaler() :
                base(Gapi.GenericAllocRelease.subscriberQos_alloc())
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(SubscriberQos from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr);
        }

        internal static DDS.ReturnCode CopyIn(SubscriberQos from, IntPtr to)
        {
            DDS.ReturnCode result;
            if (from != null) {
                result = PresentationQosPolicyMarshaler.CopyIn(
                        from.Presentation, to, offset_presentation);
                if (result == DDS.ReturnCode.Ok) {
                    result = PartitionQosPolicyMarshaler.CopyIn(
                            from.Partition, to, offset_partition);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = GroupDataQosPolicyMarshaler.CopyIn(
                            from.GroupData, to, offset_group_data);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = EntityFactoryQosPolicyMarshaler.CopyIn(
                            from.EntityFactory, to, offset_entity_factory);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ShareQosPolicyMarshaler.CopyIn(
                            from.Share, to, offset_share);
                }
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.SubscriberQosMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosToplevelMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "SubscriberQos attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(IntPtr nativePtr)
        {
            PresentationQosPolicyMarshaler.CleanupIn(nativePtr, offset_presentation);
            PartitionQosPolicyMarshaler.CleanupIn(nativePtr, offset_partition);
            GroupDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_group_data);
            EntityFactoryQosPolicyMarshaler.CleanupIn(nativePtr, offset_entity_factory);
            ShareQosPolicyMarshaler.CleanupIn(nativePtr, offset_share);

        }

        internal void CopyOut(ref SubscriberQos to)
        {
            CopyOut(GapiPtr, ref to);
        }

        internal static void CopyOut(IntPtr from, ref SubscriberQos to)
        {
            if (to == null) to = new SubscriberQos();
            PresentationQosPolicyMarshaler.CopyOut(from, ref to.Presentation, offset_presentation);
            PartitionQosPolicyMarshaler.CopyOut(from, ref to.Partition, offset_partition);
            GroupDataQosPolicyMarshaler.CopyOut(from, ref to.GroupData, offset_group_data);
            EntityFactoryQosPolicyMarshaler.CopyOut(from, ref to.EntityFactory, offset_entity_factory);
            ShareQosPolicyMarshaler.CopyOut(from, ref to.Share, offset_share);
        }
    }

    internal class TopicQosMarshaler : GapiMarshaler
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

        public TopicQosMarshaler() :
                base(Gapi.GenericAllocRelease.topicQos_alloc())
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }


        internal DDS.ReturnCode CopyIn(TopicQos from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr);
        }

        internal static DDS.ReturnCode CopyIn(TopicQos from, IntPtr to)
        {
            DDS.ReturnCode result;
            if (from != null) {
                result = TopicDataQosPolicyMarshaler.CopyIn(
                        from.TopicData, to, offset_topic_data);
                if (result == DDS.ReturnCode.Ok) {
                    result = DurabilityQosPolicyMarshaler.CopyIn(
                            from.Durability, to, offset_durability);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = DurabilityServiceQosPolicyMarshaler.CopyIn(
                            from.DurabilityService, to, offset_durability_service);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = DeadlineQosPolicyMarshaler.CopyIn(
                            from.Deadline, to, offset_deadline);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LatencyBudgetQosPolicyMarshaler.CopyIn(
                            from.LatencyBudget, to, offset_latency_budget);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LivelinessQosPolicyMarshaler.CopyIn(
                            from.Liveliness, to, offset_liveliness);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ReliabilityQosPolicyMarshaler.CopyIn(
                            from.Reliability, to, offset_reliability);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = DestinationOrderQosPolicyMarshaler.CopyIn(
                            from.DestinationOrder, to, offset_destination_order);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = HistoryQosPolicyMarshaler.CopyIn(
                            from.History, to, offset_history);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ResourceLimitsQosPolicyMarshaler.CopyIn(
                            from.ResourceLimits, to, offset_resource_limits);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = TransportPriorityQosPolicyMarshaler.CopyIn(
                            from.TransportPriority, to, offset_transport_priority);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LifespanQosPolicyMarshaler.CopyIn(
                            from.Lifespan, to, offset_lifespan);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = OwnershipQosPolicyMarshaler.CopyIn(
                            from.Ownership, to, offset_ownership);
                }
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.TopicQosMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosToplevelMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "TopicQos attribute may not be a null pointer.");
            }
            return result;
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

        internal void CopyOut(ref TopicQos to)
        {
            CopyOut(GapiPtr, ref to);
        }

        internal static void CopyOut(IntPtr from, ref TopicQos to)
        {
            if (to == null) to = new TopicQos();
            TopicDataQosPolicyMarshaler.CopyOut(from, ref to.TopicData, offset_topic_data);
            DurabilityQosPolicyMarshaler.CopyOut(from, ref to.Durability, offset_durability);
            DurabilityServiceQosPolicyMarshaler.CopyOut(from, ref to.DurabilityService, offset_durability_service);
            DeadlineQosPolicyMarshaler.CopyOut(from, ref to.Deadline, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyOut(from, ref to.LatencyBudget, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CopyOut(from, ref to.Liveliness, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyOut(from, ref to.Reliability, offset_reliability);
            DestinationOrderQosPolicyMarshaler.CopyOut(from, ref to.DestinationOrder, offset_destination_order);
            HistoryQosPolicyMarshaler.CopyOut(from, ref to.History, offset_history);
            ResourceLimitsQosPolicyMarshaler.CopyOut(from, ref to.ResourceLimits, offset_resource_limits);
            TransportPriorityQosPolicyMarshaler.CopyOut(from, ref to.TransportPriority, offset_transport_priority);
            LifespanQosPolicyMarshaler.CopyOut(from, ref to.Lifespan, offset_lifespan);
            OwnershipQosPolicyMarshaler.CopyOut(from, ref to.Ownership, offset_ownership);
        }
    }

    internal class DataReaderQosMarshaler : GapiMarshaler
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

        public DataReaderQosMarshaler() :
                base(Gapi.GenericAllocRelease.dataReaderQos_alloc())
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(DataReaderQos from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr);
        }

        internal static DDS.ReturnCode CopyIn(DataReaderQos from, IntPtr to)
        {
            DDS.ReturnCode result;
            if (from != null) {
                result = DurabilityQosPolicyMarshaler.CopyIn(
                        from.Durability, to, offset_durability);
                if (result == DDS.ReturnCode.Ok) {
                    result = DeadlineQosPolicyMarshaler.CopyIn(
                            from.Deadline, to, offset_deadline);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LatencyBudgetQosPolicyMarshaler.CopyIn(
                            from.LatencyBudget, to, offset_latency_budget);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LivelinessQosPolicyMarshaler.CopyIn(
                            from.Liveliness, to, offset_liveliness);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ReliabilityQosPolicyMarshaler.CopyIn(
                            from.Reliability, to, offset_reliability);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = DestinationOrderQosPolicyMarshaler.CopyIn(
                            from.DestinationOrder, to, offset_destination_order);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = HistoryQosPolicyMarshaler.CopyIn(
                            from.History, to, offset_history);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ResourceLimitsQosPolicyMarshaler.CopyIn(
                            from.ResourceLimits, to, offset_resource_limits);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = UserDataQosPolicyMarshaler.CopyIn(
                            from.UserData, to, offset_user_data);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = OwnershipQosPolicyMarshaler.CopyIn(
                            from.Ownership, to, offset_ownership);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = TimeBasedFilterQosPolicyMarshaler.CopyIn(
                            from.TimeBasedFilter, to, offset_time_based_filter);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ReaderDataLifecycleQosPolicyMarshaler.CopyIn(
                            from.ReaderDataLifecycle, to, offset_reader_data_lifecycle);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = SubscriptionKeyQosPolicyMarshaler.CopyIn(
                            from.SubscriptionKeys, to, offset_subscription_keys);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ReaderLifespanQosPolicyMarshaler.CopyIn(
                            from.ReaderLifespan, to, offset_reader_lifespan);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ShareQosPolicyMarshaler.CopyIn(
                            from.Share, to, offset_share);
                }
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.DataReaderQosMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosToplevelMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "DataReaderQos attribute may not be a null pointer.");
            }
            return result;
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

        internal void CopyOut(ref DataReaderQos to)
        {
            CopyOut(GapiPtr, ref to);
        }

        internal static void CopyOut(IntPtr from, ref DataReaderQos to)
        {
            if (to == null) to = new DataReaderQos();
            DurabilityQosPolicyMarshaler.CopyOut(from, ref to.Durability, offset_durability);
            DeadlineQosPolicyMarshaler.CopyOut(from, ref to.Deadline, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyOut(from, ref to.LatencyBudget, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CopyOut(from, ref to.Liveliness, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyOut(from, ref to.Reliability, offset_reliability);
            DestinationOrderQosPolicyMarshaler.CopyOut(from, ref to.DestinationOrder, offset_destination_order);
            HistoryQosPolicyMarshaler.CopyOut(from, ref to.History, offset_history);
            ResourceLimitsQosPolicyMarshaler.CopyOut(from, ref to.ResourceLimits, offset_resource_limits);
            UserDataQosPolicyMarshaler.CopyOut(from, ref to.UserData, offset_user_data);
            OwnershipQosPolicyMarshaler.CopyOut(from, ref to.Ownership, offset_ownership);
            TimeBasedFilterQosPolicyMarshaler.CopyOut(from, ref to.TimeBasedFilter, offset_time_based_filter);
            ReaderDataLifecycleQosPolicyMarshaler.CopyOut(from, ref to.ReaderDataLifecycle, offset_reader_data_lifecycle);
            SubscriptionKeyQosPolicyMarshaler.CopyOut(from, ref to.SubscriptionKeys, offset_subscription_keys);
            ReaderLifespanQosPolicyMarshaler.CopyOut(from, ref to.ReaderLifespan, offset_reader_lifespan);
            ShareQosPolicyMarshaler.CopyOut(from, ref to.Share, offset_share);
        }
    }

    internal class DataWriterQosMarshaler : GapiMarshaler
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

        public DataWriterQosMarshaler() :
                base(Gapi.GenericAllocRelease.dataWriterQos_alloc())
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(DataWriterQos from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr);
        }

        internal static DDS.ReturnCode CopyIn(DataWriterQos from, IntPtr to)
        {
            DDS.ReturnCode result;
            if (from != null) {
                result = DurabilityQosPolicyMarshaler.CopyIn(
                        from.Durability, to, offset_durability);
                if (result == DDS.ReturnCode.Ok) {
                    result = DeadlineQosPolicyMarshaler.CopyIn(
                            from.Deadline, to, offset_deadline);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LatencyBudgetQosPolicyMarshaler.CopyIn(
                            from.LatencyBudget, to, offset_latency_budget);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LivelinessQosPolicyMarshaler.CopyIn(
                            from.Liveliness, to, offset_liveliness);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ReliabilityQosPolicyMarshaler.CopyIn(
                            from.Reliability, to, offset_reliability);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = DestinationOrderQosPolicyMarshaler.CopyIn(
                            from.DestinationOrder, to, offset_destination_order);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = HistoryQosPolicyMarshaler.CopyIn(
                            from.History, to, offset_history);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ResourceLimitsQosPolicyMarshaler.CopyIn(
                            from.ResourceLimits, to, offset_resource_limits);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = TransportPriorityQosPolicyMarshaler.CopyIn(
                            from.TransportPriority, to, offset_transport_priority);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LifespanQosPolicyMarshaler.CopyIn(
                            from.Lifespan, to, offset_lifespan);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = UserDataQosPolicyMarshaler.CopyIn(
                            from.UserData, to, offset_user_data);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = OwnershipQosPolicyMarshaler.CopyIn(
                            from.Ownership, to, offset_ownership);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = OwnershipStrengthQosPolicyMarshaler.CopyIn(
                            from.OwnershipStrength, to, offset_ownership_strength);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = WriterDataLifecycleQosPolicyMarshaler.CopyIn(
                            from.WriterDataLifecycle, to, offset_writer_data_lifecycle);
                }
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.DataWriterQosMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosToplevelMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "DataWriterQos attribute may not be a null pointer.");
            }
            return result;
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

        internal void CopyOut(ref DataWriterQos to)
        {
            CopyOut(GapiPtr, ref to);
        }

        internal static void CopyOut(IntPtr from, ref DataWriterQos to)
        {
            if (to == null) to = new DataWriterQos();
            DurabilityQosPolicyMarshaler.CopyOut(from, ref to.Durability, offset_durability);
            DeadlineQosPolicyMarshaler.CopyOut(from, ref to.Deadline, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyOut(from, ref to.LatencyBudget, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CopyOut(from, ref to.Liveliness, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyOut(from, ref to.Reliability, offset_reliability);
            DestinationOrderQosPolicyMarshaler.CopyOut(from, ref to.DestinationOrder, offset_destination_order);
            HistoryQosPolicyMarshaler.CopyOut(from, ref to.History, offset_history);
            ResourceLimitsQosPolicyMarshaler.CopyOut(from, ref to.ResourceLimits, offset_resource_limits);
            TransportPriorityQosPolicyMarshaler.CopyOut(from, ref to.TransportPriority, offset_transport_priority);
            LifespanQosPolicyMarshaler.CopyOut(from, ref to.Lifespan, offset_lifespan);
            UserDataQosPolicyMarshaler.CopyOut(from, ref to.UserData, offset_user_data);
            OwnershipQosPolicyMarshaler.CopyOut(from, ref to.Ownership, offset_ownership);
            OwnershipStrengthQosPolicyMarshaler.CopyOut(from, ref to.OwnershipStrength, offset_ownership_strength);
            WriterDataLifecycleQosPolicyMarshaler.CopyOut(from, ref to.WriterDataLifecycle, offset_writer_data_lifecycle);
        }
    }

    internal class TopicBulitinTopicDataMarshaler : GapiMarshaler
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

        public TopicBulitinTopicDataMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(TopicBuiltinTopicData from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr);
        }

        internal static DDS.ReturnCode CopyIn(TopicBuiltinTopicData from, IntPtr to)
        {
            DDS.ReturnCode result;
            if (from != null) {
                result = BuiltinTopicKeyMarshaler.CopyIn(from.Key, to, offset_key);

                IntPtr namePtr = Marshal.StringToHGlobalAnsi(from.Name);
                BaseMarshaler.Write(to, offset_name, namePtr);

                IntPtr typeNamePtr = Marshal.StringToHGlobalAnsi(from.TypeName);
                BaseMarshaler.Write(to, offset_type_name, typeNamePtr);

                if (result == DDS.ReturnCode.Ok) {
                    result = DurabilityQosPolicyMarshaler.CopyIn(
                            from.Durability, to, offset_durability);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = DurabilityServiceQosPolicyMarshaler.CopyIn(
                            from.DurabilityService, to, offset_durability_service);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = DeadlineQosPolicyMarshaler.CopyIn(
                            from.Deadline, to, offset_deadline);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LatencyBudgetQosPolicyMarshaler.CopyIn(
                            from.LatencyBudget, to, offset_latency_budget);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LivelinessQosPolicyMarshaler.CopyIn(
                            from.Liveliness, to, offset_liveliness);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ReliabilityQosPolicyMarshaler.CopyIn(
                            from.Reliability, to, offset_reliability);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = DestinationOrderQosPolicyMarshaler.CopyIn(
                            from.DestinationOrder, to, offset_destination_order);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = TransportPriorityQosPolicyMarshaler.CopyIn(
                            from.TransportPriority, to, offset_transport_priority);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LifespanQosPolicyMarshaler.CopyIn(
                            from.Lifespan, to, offset_lifespan);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = DestinationOrderQosPolicyMarshaler.CopyIn(
                            from.DestinationOrder, to, offset_destination_order);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = HistoryQosPolicyMarshaler.CopyIn(
                            from.History, to, offset_history);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ResourceLimitsQosPolicyMarshaler.CopyIn(
                            from.ResourceLimits, to, offset_resource_limits);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = OwnershipQosPolicyMarshaler.CopyIn(
                            from.Ownership, to, offset_ownership);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = TopicDataQosPolicyMarshaler.CopyIn(
                            from.TopicData, to, offset_topic_data);
                }
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.TopicBulitinTopicDataMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosToplevelMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "TopicBulitinTopicData attribute may not be a null pointer.");
            }
            return result;
        }

        internal void CleanupIn(IntPtr nativePtr)
        {
            BuiltinTopicKeyMarshaler.CleanupIn(nativePtr, offset_key);

            IntPtr namePtr = BaseMarshaler.ReadIntPtr(nativePtr, offset_name);
            Marshal.FreeHGlobal(namePtr);
            BaseMarshaler.Write(nativePtr, offset_name, IntPtr.Zero);

            IntPtr typeNamePtr = BaseMarshaler.ReadIntPtr(nativePtr, offset_type_name);
            Marshal.FreeHGlobal(typeNamePtr);
            BaseMarshaler.Write(nativePtr, offset_type_name, IntPtr.Zero);

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

        internal void CopyOut(ref TopicBuiltinTopicData to)
        {
            CopyOut(GapiPtr, ref to);
        }

        internal static void CopyOut(IntPtr from, ref TopicBuiltinTopicData to)
        {
            if (to == null) to = new TopicBuiltinTopicData();

            BuiltinTopicKeyMarshaler.CopyOut(from, ref to.Key, offset_key);

            IntPtr namePtr = BaseMarshaler.ReadIntPtr(from, offset_name);
            to.Name = Marshal.PtrToStringAnsi(namePtr);

            IntPtr typeNamePtr = BaseMarshaler.ReadIntPtr(from, offset_type_name);
            to.TypeName = Marshal.PtrToStringAnsi(typeNamePtr);

            DurabilityQosPolicyMarshaler.CopyOut(from, ref to.Durability, offset_durability);
            DeadlineQosPolicyMarshaler.CopyOut(from, ref to.Deadline, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyOut(from, ref to.LatencyBudget, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CopyOut(from, ref to.Liveliness, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyOut(from, ref to.Reliability, offset_reliability);
            TransportPriorityQosPolicyMarshaler.CopyOut(from, ref to.TransportPriority, offset_transport_priority);
            LifespanQosPolicyMarshaler.CopyOut(from, ref to.Lifespan, offset_lifespan);
            DestinationOrderQosPolicyMarshaler.CopyOut(from, ref to.DestinationOrder, offset_destination_order);
            HistoryQosPolicyMarshaler.CopyOut(from, ref to.History, offset_history);
            ResourceLimitsQosPolicyMarshaler.CopyOut(from, ref to.ResourceLimits, offset_resource_limits);
            OwnershipQosPolicyMarshaler.CopyOut(from, ref to.Ownership, offset_ownership);
            TopicDataQosPolicyMarshaler.CopyOut(from, ref to.TopicData, offset_topic_data);
        }
    }

    internal class PublicationBuiltinTopicDataMarshaler : GapiMarshaler
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
        private static int offset_destination_order = (int)Marshal.OffsetOf(type, "destination_order");
        private static int offset_user_data = (int)Marshal.OffsetOf(type, "user_data");
        private static int offset_ownership = (int)Marshal.OffsetOf(type, "ownership");
        private static int offset_ownership_strength = (int)Marshal.OffsetOf(type, "ownership_strength");
        private static int offset_presentation = (int)Marshal.OffsetOf(type, "presentation");
        private static int offset_partition = (int)Marshal.OffsetOf(type, "partition");
        private static int offset_topic_data = (int)Marshal.OffsetOf(type, "topic_data");
        private static int offset_group_data = (int)Marshal.OffsetOf(type, "group_data");

        public PublicationBuiltinTopicDataMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(PublicationBuiltinTopicData from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr);
        }

        internal static DDS.ReturnCode CopyIn(PublicationBuiltinTopicData from, IntPtr to)
        {
            DDS.ReturnCode result;
            if (from != null) {
                result = BuiltinTopicKeyMarshaler.CopyIn(from.Key, to, offset_key);
                if (result == DDS.ReturnCode.Ok) {
                    result = BuiltinTopicKeyMarshaler.CopyIn(from.ParticipantKey, to, offset_participant_key);
                }
                if (result == DDS.ReturnCode.Ok) {
                    IntPtr namePtr = Marshal.StringToHGlobalAnsi(from.TopicName);
                    BaseMarshaler.Write(to, offset_topic_name, namePtr);

                    IntPtr typeNamePtr = Marshal.StringToHGlobalAnsi(from.TypeName);
                    BaseMarshaler.Write(to, offset_type_name, typeNamePtr);

                    result = DurabilityQosPolicyMarshaler.CopyIn(from.Durability, to, offset_durability);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = DeadlineQosPolicyMarshaler.CopyIn(from.Deadline, to, offset_deadline);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LatencyBudgetQosPolicyMarshaler.CopyIn(from.LatencyBudget, to, offset_latency_budget);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LivelinessQosPolicyMarshaler.CopyIn(from.Liveliness, to, offset_liveliness);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ReliabilityQosPolicyMarshaler.CopyIn(from.Reliability, to, offset_reliability);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LifespanQosPolicyMarshaler.CopyIn(from.Lifespan, to, offset_lifespan);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = DestinationOrderQosPolicyMarshaler.CopyIn(from.DestinationOrder, to, offset_destination_order);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = UserDataQosPolicyMarshaler.CopyIn(from.UserData, to, offset_user_data);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = OwnershipQosPolicyMarshaler.CopyIn(from.Ownership, to, offset_ownership);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = OwnershipStrengthQosPolicyMarshaler.CopyIn(from.OwnershipStrength, to, offset_ownership_strength);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = PresentationQosPolicyMarshaler.CopyIn(from.Presentation, to, offset_presentation);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = PartitionQosPolicyMarshaler.CopyIn(from.Partition, to, offset_partition);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = TopicDataQosPolicyMarshaler.CopyIn(from.TopicData, to, offset_topic_data);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = GroupDataQosPolicyMarshaler.CopyIn(from.GroupData, to, offset_group_data);
                }
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.PublicationBuiltinTopicDataMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosToplevelMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "PublicationBuiltinTopicData attribute may not be a null pointer.");
            }
            return result;
        }

        internal void CleanupIn(IntPtr nativePtr)
        {
            BuiltinTopicKeyMarshaler.CleanupIn(nativePtr, offset_key);
            BuiltinTopicKeyMarshaler.CleanupIn(nativePtr, offset_participant_key);

            IntPtr namePtr = BaseMarshaler.ReadIntPtr(nativePtr, offset_topic_name);
            Marshal.FreeHGlobal(namePtr);
            BaseMarshaler.Write(nativePtr, offset_topic_name, IntPtr.Zero);

            IntPtr typeNamePtr = BaseMarshaler.ReadIntPtr(nativePtr, offset_type_name);
            Marshal.FreeHGlobal(typeNamePtr);
            BaseMarshaler.Write(nativePtr, offset_type_name, IntPtr.Zero);

            DurabilityQosPolicyMarshaler.CleanupIn(nativePtr, offset_durability);
            DeadlineQosPolicyMarshaler.CleanupIn(nativePtr, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CleanupIn(nativePtr, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CleanupIn(nativePtr, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CleanupIn(nativePtr, offset_reliability);
            LifespanQosPolicyMarshaler.CleanupIn(nativePtr, offset_lifespan);
            DestinationOrderQosPolicyMarshaler.CleanupIn(nativePtr, offset_destination_order);
            UserDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_user_data);
            OwnershipQosPolicyMarshaler.CleanupIn(nativePtr, offset_ownership);
            OwnershipStrengthQosPolicyMarshaler.CleanupIn(nativePtr, offset_ownership_strength);
            PresentationQosPolicyMarshaler.CleanupIn(nativePtr, offset_presentation);
            PartitionQosPolicyMarshaler.CleanupIn(nativePtr, offset_partition);
            TopicDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_topic_data);
            GroupDataQosPolicyMarshaler.CleanupIn(nativePtr, offset_group_data);
        }

        internal void CopyOut(ref PublicationBuiltinTopicData to)
        {
            CopyOut(GapiPtr, ref to);
        }

        internal static void CopyOut(IntPtr from, ref PublicationBuiltinTopicData to)
        {
            if (to == null) to = new PublicationBuiltinTopicData();

            BuiltinTopicKeyMarshaler.CopyOut(from, ref to.Key, offset_key);
            BuiltinTopicKeyMarshaler.CopyOut(from, ref to.ParticipantKey, offset_participant_key);

            IntPtr namePtr = BaseMarshaler.ReadIntPtr(from, offset_topic_name);
            to.TopicName = Marshal.PtrToStringAnsi(namePtr);

            IntPtr typeNamePtr = BaseMarshaler.ReadIntPtr(from, offset_type_name);
            to.TypeName = Marshal.PtrToStringAnsi(typeNamePtr);

            DurabilityQosPolicyMarshaler.CopyOut(from, ref to.Durability, offset_durability);
            DeadlineQosPolicyMarshaler.CopyOut(from, ref to.Deadline, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyOut(from, ref to.LatencyBudget, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CopyOut(from, ref to.Liveliness, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyOut(from, ref to.Reliability, offset_reliability);
            LifespanQosPolicyMarshaler.CopyOut(from, ref to.Lifespan, offset_lifespan);
            DestinationOrderQosPolicyMarshaler.CopyOut(from, ref to.DestinationOrder, offset_destination_order);
            UserDataQosPolicyMarshaler.CopyOut(from, ref to.UserData, offset_user_data);
            OwnershipQosPolicyMarshaler.CopyOut(from, ref to.Ownership, offset_ownership);
            OwnershipStrengthQosPolicyMarshaler.CopyOut(from, ref to.OwnershipStrength, offset_ownership_strength);
            PresentationQosPolicyMarshaler.CopyOut(from, ref to.Presentation, offset_presentation);
            PartitionQosPolicyMarshaler.CopyOut(from, ref to.Partition, offset_partition);
            TopicDataQosPolicyMarshaler.CopyOut(from, ref to.TopicData, offset_topic_data);
            GroupDataQosPolicyMarshaler.CopyOut(from, ref to.GroupData, offset_group_data);
        }
    }

    internal class SubscriptionBuiltinTopicDataMarshaler : GapiMarshaler
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

        public SubscriptionBuiltinTopicDataMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(SubscriptionBuiltinTopicData from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr);
        }

        internal static DDS.ReturnCode CopyIn(SubscriptionBuiltinTopicData from, IntPtr to)
        {
            DDS.ReturnCode result;
            if (from != null) {
                result = BuiltinTopicKeyMarshaler.CopyIn(from.Key, to, offset_key);
                if (result == DDS.ReturnCode.Ok) {
                    result = BuiltinTopicKeyMarshaler.CopyIn(from.ParticipantKey, to, offset_participant_key);
                }
                if (result == DDS.ReturnCode.Ok) {
                    IntPtr namePtr = Marshal.StringToHGlobalAnsi(from.TopicName);
                    BaseMarshaler.Write(to, offset_topic_name, namePtr);

                    IntPtr typeNamePtr = Marshal.StringToHGlobalAnsi(from.TypeName);
                    BaseMarshaler.Write(to, offset_type_name, typeNamePtr);

                    result = DurabilityQosPolicyMarshaler.CopyIn(from.Durability, to, offset_durability);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = DeadlineQosPolicyMarshaler.CopyIn(from.Deadline, to, offset_deadline);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LatencyBudgetQosPolicyMarshaler.CopyIn(from.LatencyBudget, to, offset_latency_budget);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LivelinessQosPolicyMarshaler.CopyIn(from.Liveliness, to, offset_liveliness);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ReliabilityQosPolicyMarshaler.CopyIn(from.Reliability, to, offset_reliability);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = OwnershipQosPolicyMarshaler.CopyIn(from.Ownership, to, offset_ownership);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = DestinationOrderQosPolicyMarshaler.CopyIn(from.DestinationOrder, to, offset_destination_order);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = UserDataQosPolicyMarshaler.CopyIn(from.UserData, to, offset_user_data);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = TimeBasedFilterQosPolicyMarshaler.CopyIn(from.TimeBasedFilter, to, offset_time_based_filter);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = PresentationQosPolicyMarshaler.CopyIn(from.Presentation, to, offset_presentation);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = PartitionQosPolicyMarshaler.CopyIn(from.Partition, to, offset_partition);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = TopicDataQosPolicyMarshaler.CopyIn(from.TopicData, to, offset_topic_data);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = GroupDataQosPolicyMarshaler.CopyIn(from.GroupData, to, offset_group_data);
                }
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.SubscriptionBuiltinTopicDataMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosToplevelMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "SubscriptionBuiltinTopicData attribute may not be a null pointer.");
            }
            return result;
        }

        internal void CleanupIn(IntPtr nativePtr)
        {
            BuiltinTopicKeyMarshaler.CleanupIn(nativePtr, offset_key);
            BuiltinTopicKeyMarshaler.CleanupIn(nativePtr, offset_participant_key);

            IntPtr namePtr = BaseMarshaler.ReadIntPtr(nativePtr, offset_topic_name);
            Marshal.FreeHGlobal(namePtr);
            BaseMarshaler.Write(nativePtr, offset_topic_name, IntPtr.Zero);

            IntPtr typeNamePtr = BaseMarshaler.ReadIntPtr(nativePtr, offset_type_name);
            Marshal.FreeHGlobal(typeNamePtr);
            BaseMarshaler.Write(nativePtr, offset_type_name, IntPtr.Zero);

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

        internal void CopyOut(ref SubscriptionBuiltinTopicData to)
        {
            CopyOut(GapiPtr, ref to);
        }

        internal static void CopyOut(IntPtr from, ref SubscriptionBuiltinTopicData to)
        {
            if (to == null) to = new SubscriptionBuiltinTopicData();

            BuiltinTopicKeyMarshaler.CopyOut(from, ref to.Key, offset_key);
            BuiltinTopicKeyMarshaler.CopyOut(from, ref to.ParticipantKey, offset_participant_key);

            IntPtr namePtr = BaseMarshaler.ReadIntPtr(from, offset_topic_name);
            to.TopicName = Marshal.PtrToStringAnsi(namePtr);

            IntPtr typeNamePtr = BaseMarshaler.ReadIntPtr(from, offset_type_name);
            to.TypeName = Marshal.PtrToStringAnsi(typeNamePtr);

            DurabilityQosPolicyMarshaler.CopyOut(from, ref to.Durability, offset_durability);
            DeadlineQosPolicyMarshaler.CopyOut(from, ref to.Deadline, offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyOut(from, ref to.LatencyBudget, offset_latency_budget);
            LivelinessQosPolicyMarshaler.CopyOut(from, ref to.Liveliness, offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyOut(from, ref to.Reliability, offset_reliability);
            OwnershipQosPolicyMarshaler.CopyOut(from, ref to.Ownership, offset_ownership);
            DestinationOrderQosPolicyMarshaler.CopyOut(from, ref to.DestinationOrder, offset_destination_order);
            UserDataQosPolicyMarshaler.CopyOut(from, ref to.UserData, offset_user_data);
            TimeBasedFilterQosPolicyMarshaler.CopyOut(from, ref to.TimeBasedFilter, offset_time_based_filter);
            PresentationQosPolicyMarshaler.CopyOut(from, ref to.Presentation, offset_presentation);
            PartitionQosPolicyMarshaler.CopyOut(from, ref to.Partition, offset_partition);
            TopicDataQosPolicyMarshaler.CopyOut(from, ref to.TopicData, offset_topic_data);
            GroupDataQosPolicyMarshaler.CopyOut(from, ref to.GroupData, offset_group_data);
        }
    }
}