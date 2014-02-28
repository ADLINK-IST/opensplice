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

namespace DDS.OpenSplice.Gapi
{
    static internal class GenericAllocRelease
    {
        /* Generic allocation/release functions */
        [DllImport("ddskernel", EntryPoint = "gapi_free")]
        public static extern void Free(IntPtr buffer);

        // it is actually a uint not an int...but all are
        // lengths are ints, so this saves a cast.
        [DllImport("ddskernel", EntryPoint = "gapi_alloc")]
        public static extern IntPtr Alloc(int length);


        [DllImport("ddskernel", EntryPoint = "gapi_sequence_malloc")]
        public static extern IntPtr sequence_alloc();

        [DllImport("ddskernel", EntryPoint = "gapi_string_alloc")]
        public static extern IntPtr string_alloc(int length);

        [DllImport("ddskernel", EntryPoint = "gapi_string_dup")]
        public static extern IntPtr string_dup(string stringPtr);

        // QOS alloc's
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipantFactoryQos__alloc")]
        public static extern IntPtr domainParticipantFactoryQos_alloc();

        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipantQos__alloc")]
        public static extern IntPtr domainParticipantQos_alloc();

        [DllImport("ddskernel", EntryPoint = "gapi_topicQos__alloc")]
        public static extern IntPtr topicQos_alloc();

        [DllImport("ddskernel", EntryPoint = "gapi_dataWriterQos__alloc")]
        public static extern IntPtr dataWriterQos_alloc();

        [DllImport("ddskernel", EntryPoint = "gapi_publisherQos__alloc")]
        public static extern IntPtr publisherQos_alloc();

        [DllImport("ddskernel", EntryPoint = "gapi_dataReaderQos__alloc")]
        public static extern IntPtr dataReaderQos_alloc();

        [DllImport("ddskernel", EntryPoint = "gapi_subscriberQos__alloc")]
        public static extern IntPtr subscriberQos_alloc();

        //// sequence alloc's
        //[DllImport("ddskernel", EntryPoint = "gapi_stringSeq__alloc")]
        //public static extern IntPtr stringSeq_alloc();

        // TODO: More seqs

        // sequence buffer alloc's
        //[DllImport("ddskernel", EntryPoint = "gapi_instanceHandleSeq_allocbuf")]
        //public static extern IntPtr instanceHandleSeq_allocbuf(uint length);

        //[DllImport("ddskernel", EntryPoint = "gapi_stringSeq_allocbuf")]
        //public static extern IntPtr stringSeq_allocbuf(uint length);

        //[DllImport("ddskernel", EntryPoint = "gapi_qosPolicyCountSeq_allocbuf")]
        //public static extern IntPtr qosPolicyCountSeq_allocbuf(uint length);

        //[DllImport("ddskernel", EntryPoint = "gapi_topicSeq_allocbuf")]
        //public static extern IntPtr topicSeq_allocbuf(uint length);

        //[DllImport("ddskernel", EntryPoint = "gapi_dataReaderSeq_allocbuf")]
        //public static extern IntPtr dataReaderSeq_allocbuf(uint length);

        //[DllImport("ddskernel", EntryPoint = "gapi_dataReaderViewSeq_allocbuf")]
        //public static extern IntPtr dataReaderViewSeq_allocbuf(uint length);

        //[DllImport("ddskernel", EntryPoint = "gapi_conditionSeq_allocbuf")]
        //public static extern IntPtr conditionSeq_allocbuf(uint length);

        //[DllImport("ddskernel", EntryPoint = "gapi_sampleStateSeq_allocbuf")]
        //public static extern IntPtr sampleStateSeq_allocbuf(uint length);

        //[DllImport("ddskernel", EntryPoint = "gapi_viewStateSeq_allocbuf")]
        //public static extern IntPtr viewStateSeq_allocbuf(uint length);

        //[DllImport("ddskernel", EntryPoint = "gapi_instanceStateSeq_allocbuf")]
        //public static extern IntPtr instanceStateSeq_allocbuf(uint length);

        //[DllImport("ddskernel", EntryPoint = "gapi_octetSeq_allocbuf")]
        //public static extern IntPtr octetSeq_allocbuf(uint length);

        //[DllImport("ddskernel", EntryPoint = "gapi_participantBuiltinTopicDataSeq_allocbuf")]
        //public static extern IntPtr participantBuiltinTopicDataSeq_allocbuf(uint length);

        //[DllImport("ddskernel", EntryPoint = "gapi_topicBuiltinTopicDataSeq_allocbuf")]
        //public static extern IntPtr topicBuiltinTopicDataSeq_allocbuf(uint length);

        //[DllImport("ddskernel", EntryPoint = "gapi_publicationBuiltinTopicDataSeq_allocbuf")]
        //public static extern IntPtr publicationBuiltinTopicDataSeq_allocbuf(uint length);

        //[DllImport("ddskernel", EntryPoint = "gapi_subscriptionBuiltinTopicDataSeq_allocbuf")]
        //public static extern IntPtr subscriptionBuiltinTopicDataSeq_allocbuf(uint length);

        //[DllImport("ddskernel", EntryPoint = "gapi_sampleInfoSeq_allocbuf")]
        //public static extern IntPtr sampleInfoSeq_allocbuf(uint length);
    }

    /*
     * // ----------------------------------------------------------------------
     * // gapi Constants & Macro's
     * // ----------------------------------------------------------------------
      */
    
    internal static class NativeConstants
    {
        public static IntPtr GapiParticipantQosDefault
        {
            get
            {
                return IntPtr.Zero;
            }
        }

        public static IntPtr GapiPublisherQosDefault
        {
            get
            {
                return IntPtr.Zero;
            }
        }

        public static IntPtr GapiSubscriberQosDefault
        {
            get
            {
                return IntPtr.Zero;
            }
        }

        public static IntPtr GapiTopicQosDefault
        {
            get
            {
                return IntPtr.Zero;
            }
        }

        public static IntPtr GapiDataWriterQosDefault
        {
            get
            {
                return IntPtr.Zero;
            }
        }
    }

    /*
     * // ----------------------------------------------------------------------
     * // Listeners
     * // ----------------------------------------------------------------------
     * interface Listener;
     * interface Entity;
     * interface TopicDescription;
     * interface Topic;
     * interface ContentFilteredTopic;
     * interface MultiTopic;
     * interface DataWriter;
     * interface DataReader;
     * interface Subscriber;
     * interface Publisher;
     * interface TypeSupport;
     */
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void gapi_listenerThreadAction(System.IntPtr listener_data);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void gapi_deleteEntityAction(IntPtr entityData, IntPtr userData);

    // topic listener
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void gapi_listener_InconsistentTopicListener(IntPtr entityData, IntPtr topicPtr, InconsistentTopicStatus status);

    internal struct gapi_topicListener
    {
        public IntPtr listener_data;
        public gapi_listener_InconsistentTopicListener on_inconsistent_topic;
    }

    // datawriter listener
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void gapi_listener_OfferedDeadlineMissedListener(IntPtr entityData, IntPtr writerPtr, OfferedDeadlineMissedStatus status);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void gapi_listener_LivelinessLostListener(IntPtr entityData, IntPtr writerPtr, LivelinessLostStatus status);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void gapi_listener_OfferedIncompatibleQosListener(IntPtr entityData, IntPtr writerPtr, IntPtr gapi_status);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void gapi_listener_PublicationMatchedListener(IntPtr entityData, IntPtr writerPtr, PublicationMatchedStatus status);

    internal struct gapi_publisherDataWriterListener
    {
        public IntPtr listener_data;
        public gapi_listener_OfferedDeadlineMissedListener on_offered_deadline_missed;
        public gapi_listener_OfferedIncompatibleQosListener on_offered_incompatible_qos;
        public gapi_listener_LivelinessLostListener on_liveliness_lost;
        public gapi_listener_PublicationMatchedListener on_publication_match;
    }

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void gapi_listener_RequestedDeadlineMissedListener(IntPtr entityData, IntPtr readerPtr, RequestedDeadlineMissedStatus status);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void gapi_listener_LivelinessChangedListener(IntPtr entityData, IntPtr readerPtr, LivelinessChangedStatus status);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void gapi_listener_RequestedIncompatibleQosListener(IntPtr entityData, IntPtr readerPtr, IntPtr gapi_status);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void gapi_listener_SampleRejectedListener(IntPtr entityData, IntPtr readerPtr, SampleRejectedStatus status);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void gapi_listener_DataAvailableListener(IntPtr entityData, IntPtr readerPtr);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void gapi_listener_SubscriptionMatchedListener(IntPtr entityData, IntPtr readerPtr, SubscriptionMatchedStatus status);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void gapi_listener_SampleLostListener(IntPtr entityData, IntPtr readerPtr, SampleLostStatus status);

    internal struct gapi_dataReaderListener
    {
        public IntPtr listener_data;
        public gapi_listener_RequestedDeadlineMissedListener on_requested_deadline_missed;
        public gapi_listener_RequestedIncompatibleQosListener on_requested_incompatible_qos;
        public gapi_listener_SampleRejectedListener on_sample_rejected;
        public gapi_listener_LivelinessChangedListener on_liveliness_changed;
        public gapi_listener_DataAvailableListener on_data_available;
        public gapi_listener_SubscriptionMatchedListener on_subscription_match;
        public gapi_listener_SampleLostListener on_sample_lost;
    }

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void gapi_listener_DataOnReadersListener(IntPtr entityData, IntPtr subscriberPtr);

    internal struct gapi_subscriberListener
    {
        public gapi_dataReaderListener dataReader;
        public gapi_listener_DataOnReadersListener on_data_on_readers;
    }

    internal struct gapi_domainParticipantListener
    {
        public IntPtr listener_data;
        public gapi_listener_InconsistentTopicListener on_inconsistent_topic;
        public gapi_listener_OfferedDeadlineMissedListener on_offered_deadline_missed;
        public gapi_listener_OfferedIncompatibleQosListener on_offered_incompatible_qos;
        public gapi_listener_LivelinessLostListener on_liveliness_lost;
        public gapi_listener_PublicationMatchedListener on_publication_match;
        public gapi_listener_RequestedDeadlineMissedListener on_requested_deadline_missed;
        public gapi_listener_RequestedIncompatibleQosListener on_requested_incompatible_qos;
        public gapi_listener_SampleRejectedListener on_sample_rejected;
        public gapi_listener_LivelinessChangedListener on_liveliness_changed;
        public gapi_listener_DataAvailableListener on_data_available;
        public gapi_listener_SubscriptionMatchedListener on_subscription_match;
        public gapi_listener_SampleLostListener on_sample_lost;
        public gapi_listener_DataOnReadersListener on_data_on_readers;
    }

    /*
     * struct QosPolicyCount {
     *     QosPolicyId_t policy_id;
     *     long count;
     * };
     */
    //typedef C_STRUCT(gapi_qosPolicyCount) {
    //    gapi_qosPolicyId_t policy_id;
    //    gapi_long count;
    //} gapi_qosPolicyCount;
    [StructLayoutAttribute(LayoutKind.Sequential)]
    public struct gapi_qosPolicyCount
    {
        public int policy_id;
        public int count;
    }

    // NOTE: Both gapi_requestedIncompatibleQosStatus and gapi_offeredIncompatibleQosStatus
    // use the same basic type, so we create gapi_offeredRequestedIncompatibleQosStatus 
    //typedef C_STRUCT(gapi_requestedIncompatibleQosStatus) {
    //    gapi_long total_count;
    //    gapi_long total_count_change;
    //    gapi_qosPolicyId_t last_policy_id;
    //    gapi_qosPolicyCountSeq policies;
    //} gapi_requestedIncompatibleQosStatus;
    [StructLayoutAttribute(LayoutKind.Sequential)]
    public struct gapi_offeredRequestedIncompatibleQosStatus
    {
        public int total_count;
        public int total_count_change;
        public int last_policy_id;

        // gapi_qosPolicyCountSeq
        public gapi_Seq policies;
    }


    /*
     * // ----------------------------------------------------------------------
     * // Qos
     * // ----------------------------------------------------------------------
     */
#pragma warning disable 169
    [StructLayoutAttribute(LayoutKind.Sequential)]
    public class gapi_Seq
    {
        public uint _maximum;
        public uint _length;
        public IntPtr _buffer;
        [MarshalAs(UnmanagedType.U1)]
        public bool _release;
    }

    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_userDataQosPolicy
    {
        // octetSeq
        gapi_Seq value;
    }

    /* struct EntityFactoryQosPolicy {
     *     boolean autoenable_created_entities;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_entityFactoryQosPolicy
    {
        [MarshalAs(UnmanagedType.U1)]
        bool autoenable_created_entities;
    }

    /* struct ShareQosPolicy {
     *     String name;
     *     Boolean enable;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_shareQosPolicy
    {
        IntPtr name;
        [MarshalAs(UnmanagedType.U1)]
        bool enable;
    }

    /* struct SchedulingClassQosPolicy {
     *     SchedulingClassQosPolicyKind kind;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_schedulingClassQosPolicy
    {
        SchedulingClassQosPolicyKind kind;
    }

    /* struct SchedulingPriorityQosPolicy {
     *     SchedulingPriorityQosPolicyKind kind;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_schedulingPriorityQosPolicy
    {
        SchedulingPriorityQosPolicyKind kind;
    }

    /* struct SchedulingQosPolicy {
     *     SchedulingClassQosPolicy    scheduling_class;
     *     SchedulingPriorityQosPolicy scheduling_priority_kind;
     *     long                        scheduling_priority;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_schedulingQosPolicy
    {
        gapi_schedulingClassQosPolicy scheduling_class;
        gapi_schedulingPriorityQosPolicy scheduling_priority_kind;
        int scheduling_priority;
    }

    /*
     * struct DomainParticipantFactoryQos {
     *     UserDataQosPolicy user_data;
     *     EntityFactoryQosPolicy entity_factory;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_domainParticipantFactoryQos
    {
        gapi_entityFactoryQosPolicy entity_factory;
    }

    /*
     * struct DomainParticipantQos {
     *     UserDataQosPolicy user_data;
     *     EntityFactoryQosPolicy entity_factory;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_domainParticipantQos
    {
        gapi_userDataQosPolicy user_data;
        gapi_entityFactoryQosPolicy entity_factory;
        gapi_schedulingQosPolicy watchdog_scheduling;
        gapi_schedulingQosPolicy listener_scheduling;
    }

    /* struct PresentationQosPolicy {
     *     PresentationQosPolicyAccessScopeKind access_scope;
     *     boolean coherent_access;
     *     boolean ordered_access;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_presentationQosPolicy
    {
        int access_scope;
        [MarshalAs(UnmanagedType.U1)]
        bool coherent_access;
        [MarshalAs(UnmanagedType.U1)]
        bool ordered_access;
    }

    /* struct GroupDataQosPolicy {
     *     sequence<octet> value;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_groupDataQosPolicy
    {
        // octetSeq
        gapi_Seq value;
    }

    /* struct PartitionQosPolicy {
     *     StringSeq name;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_partitionQosPolicy
    {
        // stringSeq
        gapi_Seq name;
    }

    /*
     * struct PublisherQos {
     *     PresentationQosPolicy presentation;
     *     PartitionQosPolicy partition;
     *     GroupDataQosPolicy group_data;
     *     EntityFactoryQosPolicy entity_factory;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_publisherQos
    {
        gapi_presentationQosPolicy presentation;
        gapi_partitionQosPolicy partition;
        gapi_groupDataQosPolicy group_data;
        gapi_entityFactoryQosPolicy entity_factory;
    }

    /*
     * struct SubscriberQos {
     *     PresentationQosPolicy presentation;
     *     PartitionQosPolicy partition;
     *     GroupDataQosPolicy group_data;
     *     EntityFactoryQosPolicy entity_factory;
     *     ShareQosPolicy share;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_subscriberQos
    {
        gapi_presentationQosPolicy presentation;
        gapi_partitionQosPolicy partition;
        gapi_groupDataQosPolicy group_data;
        gapi_entityFactoryQosPolicy entity_factory;
        gapi_shareQosPolicy share;
    }

    /*
     * struct TopicDataQosPolicy {
     *     sequence<octet> value;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_topicDataQosPolicy
    {
        // octetSeq
        gapi_Seq value;
    }

    /* struct DurabilityQosPolicy {
     *     DurabilityQosPolicyKind kind;
     *     Duration_t service_cleanup_delay;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_durabilityQosPolicy
    {
        DurabilityQosPolicyKind kind;
    }

    /* struct DurabilityServiceQosPolicy {
     *     HistoryQosPolicyKind history_kind;
     *     long history_depth;
     *     long max_samples;
     *     long max_instances;
     *     long max_samples_per_instance;
     *     Duration_t service_cleanup_delay;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_durabilityServiceQosPolicy
    {
        Duration service_cleanup_delay;
        HistoryQosPolicyKind history_kind;
        int history_depth;
        int max_samples;
        int max_instances;
        int max_samples_per_instance;
    }

    /* struct DeadlineQosPolicy {
     *     Duration_t period;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_deadlineQosPolicy
    {
        Duration period;
    }

    /* struct LatencyBudgetQosPolicy {
     *     Duration_t duration;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_latencyBudgetQosPolicy
    {
        Duration duration;
    }

    /* struct LivelinessQosPolicy {
     *     LivelinessQosPolicyKind kind;
     *     Duration_t lease_duration;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_livelinessQosPolicy
    {
        LivelinessQosPolicyKind kind;
        Duration lease_duration;
    }

    /* struct ReliabilityQosPolicy {
     *     ReliabilityQosPolicyKind kind;
     *     Duration_t max_blocking_time;
     *     boolean synchronous;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_reliabilityQosPolicy
    {
        ReliabilityQosPolicyKind kind;
        Duration max_blocking_time;
        [MarshalAs(UnmanagedType.U1)]
        bool synchronous;
    }

    /* struct DestinationOrderQosPolicy {
     *     DestinationOrderQosPolicyKind kind;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_destinationOrderQosPolicy
    {
        DestinationOrderQosPolicyKind kind;
    }

    /* struct HistoryQosPolicy {
     *     HistoryQosPolicyKind kind;
     *     long depth;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_historyQosPolicy
    {
        HistoryQosPolicyKind kind;
        int depth;
    }

    /* struct ResourceLimitsQosPolicy {
     *     long max_samples;
     *     long max_instances;
     *     long max_samples_per_instance;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_resourceLimitsQosPolicy
    {
        int max_samples;
        int max_instances;
        int max_samples_per_instance;
    }

    /* struct TransportPriorityQosPolicy {
     *     long value;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_transportPriorityQosPolicy
    {
        int value;
    }

    /* struct LifespanQosPolicy {
     *     Duration_t duration;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_lifespanQosPolicy
    {
        Duration duration;
    }

    /* struct OwnershipQosPolicy {
     *     OwnershipQosPolicyKind kind;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_ownershipQosPolicy
    {
        OwnershipQosPolicyKind kind;
    }

    /*
     * struct TopicQos {
     *     TopicDataQosPolicy topic_data;
     *     DurabilityQosPolicy durability;
     *     DeadlineQosPolicy deadline;
     *     LatencyBudgetQosPolicy latency_budget;
     *     LivelinessQosPolicy liveliness;
     *     ReliabilityQosPolicy reliability;
     *     DestinationOrderQosPolicy destination_order;
     *     HistoryQosPolicy history;
     *     ResourceLimitsQosPolicy resource_limits;
     *     TransportPriorityQosPolicy transport_priority;
     *     LifespanQosPolicy lifespan;
     *     OwnershipQosPolicy ownership;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_topicQos
    {
        gapi_topicDataQosPolicy topic_data;
        gapi_durabilityQosPolicy durability;
        gapi_durabilityServiceQosPolicy durability_service;
        gapi_deadlineQosPolicy deadline;
        gapi_latencyBudgetQosPolicy latency_budget;
        gapi_livelinessQosPolicy liveliness;
        gapi_reliabilityQosPolicy reliability;
        gapi_destinationOrderQosPolicy destination_order;
        gapi_historyQosPolicy history;
        gapi_resourceLimitsQosPolicy resource_limits;
        gapi_transportPriorityQosPolicy transport_priority;
        gapi_lifespanQosPolicy lifespan;
        gapi_ownershipQosPolicy ownership;
    }

    /* struct TimeBasedFilterQosPolicy {
     *     Duration_t minimum_separation;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_timeBasedFilterQosPolicy
    {
        Duration minimum_separation;
    }
    /*
    
        enum InvalidSampleVisibilityQosPolicyKind {
        NO_INVALID_SAMPLES,
        MINIMUM_INVALID_SAMPLES,
        ALL_INVALID_SAMPLES
    };
    */
    
    /* struct InvalidSampleVisibilityQosPolicy {
     *     InvalidSampleVisibilityQosPolicyKind kind;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_invalidSampleVisibilityQosPolicy
    {
        InvalidSampleVisibilityQosPolicyKind kind;
    }


    /* struct ReaderDataLifecycleQosPolicy {
     *     Duration_t autopurge_nowriter_samples_delay;
     *     Duration_t autopurge_disposed_samples_delay;
     *     boolean enable_invalid_samples;
     *     InvalidSampleVisibilityQosPolicy invalid_sample_visibility;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_readerDataLifecycleQosPolicy
    {
        Duration autopurge_nowriter_samples_delay;
        Duration autopurge_disposed_samples_delay;
        [MarshalAs(UnmanagedType.U1)]
        bool enable_invalid_samples;
        gapi_invalidSampleVisibilityQosPolicy invalid_sample_visibility;
    }

    /* struct SubscriptionKeyQosPolicy {
     *     Boolean   use_key_list;
     *     StringSeq key_list;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_subscriptionKeyQosPolicy
    {
        [MarshalAs(UnmanagedType.U1)]
        bool use_key_list;
        // string seq
        gapi_Seq key_list;
    }

    /* struct ReaderLifespanQosPolicy {
     *     Boolean    use_lifespan;
     *     Duration_t duration;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_readerLifespanQosPolicy
    {
        [MarshalAs(UnmanagedType.U1)]
        bool use_lifespan;
        Duration duration;
    }

    /*
     * struct DataReaderQos {
     *     DurabilityQosPolicy durability;
     *     DeadlineQosPolicy deadline;
     *     LatencyBudgetQosPolicy latency_budget;
     *     LivelinessQosPolicy liveliness;
     *     ReliabilityQosPolicy reliability;
     *     DestinationOrderQosPolicy destination_order;
     *     HistoryQosPolicy history;
     *     ResourceLimitsQosPolicy resource_limits;
     *     UserDataQosPolicy user_data;
     *     OwnershipQosPolicy ownership;
     *     TimeBasedFilterQosPolicy time_based_filter;
     *     ReaderDataLifecycleQosPolicy reader_data_lifecycle;
     *     SubscriptionKeyQosPolicy subscription_keys;
     *     ReaderLifespanQosPolicy reader_lifespan;
     *     ShareQosPolicy share;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_dataReaderQos
    {
        gapi_durabilityQosPolicy durability;
        gapi_deadlineQosPolicy deadline;
        gapi_latencyBudgetQosPolicy latency_budget;
        gapi_livelinessQosPolicy liveliness;
        gapi_reliabilityQosPolicy reliability;
        gapi_destinationOrderQosPolicy destination_order;
        gapi_historyQosPolicy history;
        gapi_resourceLimitsQosPolicy resource_limits;
        gapi_userDataQosPolicy user_data;
        gapi_ownershipQosPolicy ownership;
        gapi_timeBasedFilterQosPolicy time_based_filter;
        gapi_readerDataLifecycleQosPolicy reader_data_lifecycle;
        gapi_subscriptionKeyQosPolicy subscription_keys;
        gapi_readerLifespanQosPolicy reader_lifespan;
        gapi_shareQosPolicy share;
    } ;

    /* struct OwnershipStrengthQosPolicy {
     *     long value;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_ownershipStrengthQosPolicy
    {
        int value;
    }

    /* struct WriterDataLifecycleQosPolicy {
     *     boolean autodispose_unregistered_instances;
     *     Duration_t autopurge_suspended_samples_delay;
     *     Duration_t autounregister_instance_delay;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_writerDataLifecycleQosPolicy
    {
        [MarshalAs(UnmanagedType.U1)]
        bool autodispose_unregistered_instances;
        Duration autopurge_suspended_samples_delay;
        Duration autounregister_instance_delay;
    }

    /*
     * struct DataWriterQos {
     *     DurabilityQosPolicy durability;
     *     DeadlineQosPolicy deadline;
     *     LatencyBudgetQosPolicy latency_budget;
     *     LivelinessQosPolicy liveliness;
     *     ReliabilityQosPolicy reliability;
     *     DestinationOrderQosPolicy destination_order;
     *     HistoryQosPolicy history;
     *     ResourceLimitsQosPolicy resource_limits;
     *     TransportPriorityQosPolicy transport_priority;
     *     LifespanQosPolicy lifespan;
     *     UserDataQosPolicy user_data;
     *     OwnershipQosPolicy ownership;
     *     OwnershipStrengthQosPolicy ownership_strength;
     *     WriterDataLifecycleQosPolicy writer_data_lifecycle;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_dataWriterQos
    {
        gapi_durabilityQosPolicy durability;
        gapi_deadlineQosPolicy deadline;
        gapi_latencyBudgetQosPolicy latency_budget;
        gapi_livelinessQosPolicy liveliness;
        gapi_reliabilityQosPolicy reliability;
        gapi_destinationOrderQosPolicy destination_order;
        gapi_historyQosPolicy history;
        gapi_resourceLimitsQosPolicy resource_limits;
        gapi_transportPriorityQosPolicy transport_priority;
        gapi_lifespanQosPolicy lifespan;
        gapi_userDataQosPolicy user_data;
        gapi_ownershipQosPolicy ownership;
        gapi_ownershipStrengthQosPolicy ownership_strength;
        gapi_writerDataLifecycleQosPolicy writer_data_lifecycle;
    }

    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct BuiltinTopicKey
    {
        public uint SystemId;
        public uint LocalId;
        public uint Serial;
    }

    /*
     * struct TopicBuiltinTopicData {
     *     BuiltinTopicKey_t key;
     *     string name;
     *     string type_name;
     *     DurabilityQosPolicy durability;
     *     DeadlineQosPolicy deadline;
     *     LatencyBudgetQosPolicy latency_budget;
     *     LivelinessQosPolicy liveliness;
     *     ReliabilityQosPolicy reliability;
     *     TransportPriorityQosPolicy transport_priority;
     *     LifespanQosPolicy lifespan;
     *     DestinationOrderQosPolicy destination_order;
     *     HistoryQosPolicy history;
     *     ResourceLimitsQosPolicy resource_limits;
     *     OwnershipQosPolicy ownership;
     *     TopicDataQosPolicy topic_data;
     * };
     */

    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_topicBuiltinTopicData
    {
        BuiltinTopicKey key;
        IntPtr name;
        IntPtr type_name;
        gapi_durabilityQosPolicy durability;
        gapi_durabilityServiceQosPolicy durability_service;
        gapi_deadlineQosPolicy deadline;
        gapi_latencyBudgetQosPolicy latency_budget;
        gapi_livelinessQosPolicy liveliness;
        gapi_reliabilityQosPolicy reliability;
        gapi_transportPriorityQosPolicy transport_priority;
        gapi_lifespanQosPolicy lifespan;
        gapi_destinationOrderQosPolicy destination_order;
        gapi_historyQosPolicy history;
        gapi_resourceLimitsQosPolicy resource_limits;
        gapi_ownershipQosPolicy ownership;
        gapi_topicDataQosPolicy topic_data;
        IntPtr meta_data;
        IntPtr key_list;
    }

    /*
     * struct PublicationBuiltinTopicData {
     *     BuiltinTopicKey_t key;
     *     BuiltinTopicKey_t participant_key;
     *     string topic_name;
     *     string type_name;
     *     DurabilityQosPolicy durability;
     *     DeadlineQosPolicy deadline;
     *     LatencyBudgetQosPolicy latency_budget;
     *     LivelinessQosPolicy liveliness;
     *     ReliabilityQosPolicy reliability;
     *     LifespanQosPolicy lifespan;
     *     UserDataQosPolicy user_data;
     *     OwnershipQosPolicy ownership;
     *     OwnershipStrengthQosPolicy ownership_strength;
     *     PresentationQosPolicy presentation;
     *     PartitionQosPolicy partition;
     *     TopicDataQosPolicy topic_data;
     *     GroupDataQosPolicy group_data;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_publicationBuiltinTopicData
    {
        BuiltinTopicKey key;
        BuiltinTopicKey participant_key;
        IntPtr topic_name;
        IntPtr type_name;
        gapi_durabilityQosPolicy durability;
        gapi_deadlineQosPolicy deadline;
        gapi_latencyBudgetQosPolicy latency_budget;
        gapi_livelinessQosPolicy liveliness;
        gapi_reliabilityQosPolicy reliability;
        gapi_lifespanQosPolicy lifespan;
        gapi_destinationOrderQosPolicy destination_order;
        gapi_userDataQosPolicy user_data;
        gapi_ownershipQosPolicy ownership;
        gapi_ownershipStrengthQosPolicy ownership_strength;
        gapi_presentationQosPolicy presentation;
        gapi_partitionQosPolicy partition;
        gapi_topicDataQosPolicy topic_data;
        gapi_groupDataQosPolicy group_data;
    }

    /*
     * struct SubscriptionBuiltinTopicData {
     *     BuiltinTopicKey_t key;
     *     BuiltinTopicKey_t participant_key;
     *     string topic_name;
     *     string type_name;
     *     DurabilityQosPolicy durability;
     *     DeadlineQosPolicy deadline;
     *     LatencyBudgetQosPolicy latency_budget;
     *     LivelinessQosPolicy liveliness;
     *     ReliabilityQosPolicy reliability;
     *     DestinationOrderQosPolicy destination_order;
     *     UserDataQosPolicy user_data;
     *     OwnershipQosPolicy ownership;
     *     TimeBasedFilterQosPolicy time_based_filter;
     *     PresentationQosPolicy presentation;
     *     PartitionQosPolicy partition;
     *     TopicDataQosPolicy topic_data;
     *     GroupDataQosPolicy group_data;
     * };
     */
    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct gapi_subscriptionBuiltinTopicData
    {
        BuiltinTopicKey key;
        BuiltinTopicKey participant_key;
        IntPtr topic_name;
        IntPtr type_name;
        gapi_durabilityQosPolicy durability;
        gapi_deadlineQosPolicy deadline;
        gapi_latencyBudgetQosPolicy latency_budget;
        gapi_livelinessQosPolicy liveliness;
        gapi_reliabilityQosPolicy reliability;
        gapi_ownershipQosPolicy ownership;
        gapi_destinationOrderQosPolicy destination_order;
        gapi_userDataQosPolicy user_data;
        gapi_timeBasedFilterQosPolicy time_based_filter;
        gapi_presentationQosPolicy presentation;
        gapi_partitionQosPolicy partition;
        gapi_topicDataQosPolicy topic_data;
        gapi_groupDataQosPolicy group_data;
    }

    //typedef struct gapi_readerInfo_s {
    //    gapi_unsigned_long    max_samples;
    //    gapi_unsigned_long    num_samples;
    //    gapi_copyOut          copy_out;
    //    gapi_copyCache        copy_cache;
    //    gapi_unsigned_long    alloc_size;
    //    gapi_topicAllocBuffer alloc_buffer;
    //    void                  *data_buffer;
    //    void                  *info_buffer;
    //    void                  **loan_registry;
    //} gapi_readerInfo;

    [StructLayoutAttribute(LayoutKind.Sequential)]
    public class gapi_readerInfo
    {
        public int max_samples;
        public int num_samples;
        public IntPtr copy_out;
        IntPtr copy_cache;
        uint alloc_size;
        IntPtr alloc_buffer;
        public IntPtr data_buffer;
        public IntPtr info_buffer;
        IntPtr loan_registry;
    }

    //    typedef C_STRUCT(gapi_dataSample) {
    //    void           *data;
    //    gapi_sampleInfo info;
    //} gapi_dataSample;
    [StructLayoutAttribute(LayoutKind.Sequential)]
    public struct gapi_dataSample
    {
        public IntPtr data;
        public gapi_sampleInfo sampleInfo;
		// _NOL_
		private IntPtr message;
    }

    //typedef C_STRUCT(gapi_sampleInfo) {
    //    gapi_sampleStateKind sample_state;
    //    gapi_viewStateKind view_state;
    //    gapi_instanceStateKind instance_state;
    //    gapi_boolean valid_data;
    //    gapi_time_t source_timestamp;
    //    gapi_instanceHandle_t instance_handle;
    //    gapi_instanceHandle_t publication_handle;
    //    gapi_long disposed_generation_count;
    //    gapi_long no_writers_generation_count;
    //    gapi_long sample_rank;
    //    gapi_long generation_rank;
    //    gapi_long absolute_generation_rank;
    //    gapi_time_t arrival_timestamp;
    //} gapi_sampleInfo;

    //We have one defined that matches exacly 
    // in the DdsDcpsStruct.cs

    [StructLayoutAttribute(LayoutKind.Sequential)]
    public struct gapi_sampleInfo
    {
        public SampleStateKind sample_state;
        public ViewStateKind view_state;
        public InstanceStateKind instance_state;
        [MarshalAs(UnmanagedType.U1)]
        public bool valid_data;
        public Time source_timestamp;
        public InstanceHandle instance_handle;
        public InstanceHandle publication_handle;
        public int disposed_generation_count;
        public int no_writers_generation_count;
        public int sample_rank;
        public int generation_rank;
        public int absolute_generation_rank;
        public Time arrival_timestamp;
    }
}
#pragma warning restore 169
