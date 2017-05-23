/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

package org.opensplice.dds.dcps;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;

/**
 *
 *
 * @date Jun 9, 2005
 */
public class Utilities {
    public static final int EXCEPTION_TYPE_BAD_PARAM     = 0;
    public static final int EXCEPTION_TYPE_NO_MEMORY     = 1;
    public static final int EXCEPTION_TYPE_MARSHAL       = 2;
    public static final int EXCEPTION_TYPE_BAD_OPERATION = 3;

    public static final DDS.Time_t DDS_TIMESTAMP_CURRENT = new DDS.Time_t(DDS.TIMESTAMP_INVALID_SEC.value,DDS.TIMESTAMP_INVALID_NSEC.value-1);

    public static final boolean USE_OLD_ReaderDataLifecycleQosPolicy = true;

/**
 *
 * Default OSPL Qos Policy values.
 *
 *************************************/
    public static final DDS.Duration_t ZeroDuration =
                        new DDS.Duration_t(
                            DDS.DURATION_ZERO_SEC.value,
                            DDS.DURATION_ZERO_NSEC.value);

    public static final DDS.Duration_t InfiniteDuration =
                        new DDS.Duration_t(
                            DDS.DURATION_INFINITE_SEC.value,
                            DDS.DURATION_INFINITE_NSEC.value);

    public static final DDS.EntityFactoryQosPolicy defaultEntityFactoryQosPolicy =
                        new DDS.EntityFactoryQosPolicy(
                            true);

    public static final DDS.UserDataQosPolicy defaultUserDataQosPolicy =
                        new DDS.UserDataQosPolicy(
                            new byte[0]);

    public static final DDS.SchedulingClassQosPolicy defaultSchedulingClassQosPolicy =
                        new DDS.SchedulingClassQosPolicy(
                            DDS.SchedulingClassQosPolicyKind.SCHEDULE_DEFAULT);

    public static final DDS.SchedulingPriorityQosPolicy defaultSchedulingPriorityQosPolicy =
                        new DDS.SchedulingPriorityQosPolicy(
                            DDS.SchedulingPriorityQosPolicyKind.PRIORITY_RELATIVE);

    public static final DDS.SchedulingQosPolicy defaultSchedulingQosPolicy =
                        new DDS.SchedulingQosPolicy(
                            Utilities.defaultSchedulingClassQosPolicy,
                            Utilities.defaultSchedulingPriorityQosPolicy,
                            0);

    public static final DDS.PresentationQosPolicy defaultPresentationQosPolicy =
                        new DDS.PresentationQosPolicy(
                            DDS.PresentationQosPolicyAccessScopeKind.INSTANCE_PRESENTATION_QOS,
                            false,
                            false);

    public static final DDS.PartitionQosPolicy defaultPartitionQosPolicy =
                        new DDS.PartitionQosPolicy(
                            new String[0]);

    public static final DDS.GroupDataQosPolicy defaultGroupDataQosPolicy =
                        new DDS.GroupDataQosPolicy(
                            new byte[0]);

    public static final DDS.TopicDataQosPolicy defaultTopicDataQosPolicy =
                        new DDS.TopicDataQosPolicy(
                            new byte[0]);

    public static final DDS.DurabilityQosPolicy defaultDurabilityQosPolicy =
                        new DDS.DurabilityQosPolicy(
                            DDS.DurabilityQosPolicyKind.VOLATILE_DURABILITY_QOS);

    public static final DDS.DurabilityServiceQosPolicy defaultDurabilityServiceQosPolicy =
                        new DDS.DurabilityServiceQosPolicy(
                            Utilities.ZeroDuration,
                            DDS.HistoryQosPolicyKind.KEEP_LAST_HISTORY_QOS,
                            1, -1, -1, -1);

    public static final DDS.DeadlineQosPolicy defaultDeadlineQosPolicy =
                        new DDS.DeadlineQosPolicy(
                            Utilities.InfiniteDuration);

    public static final DDS.LatencyBudgetQosPolicy defaultLatencyBudgetQosPolicy =
                        new DDS.LatencyBudgetQosPolicy(
                            Utilities.ZeroDuration);

    public static final DDS.LivelinessQosPolicy defaultLivelinessQosPolicy =
                        new DDS.LivelinessQosPolicy(
                            DDS.LivelinessQosPolicyKind.AUTOMATIC_LIVELINESS_QOS,
                            Utilities.InfiniteDuration);

    public static final DDS.ReliabilityQosPolicy defaultDataReaderReliabilityQosPolicy =
                        new DDS.ReliabilityQosPolicy(
                            DDS.ReliabilityQosPolicyKind.BEST_EFFORT_RELIABILITY_QOS,
                            new DDS.Duration_t(0,100000000),
                            false);

    public static final DDS.ReliabilityQosPolicy defaultDataWriterReliabilityQosPolicy =
                        new DDS.ReliabilityQosPolicy(
                            DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS,
                            new DDS.Duration_t(0,100000000),
                            false);

    public static final DDS.DestinationOrderQosPolicy defaultDestinationOrderQosPolicy =
                        new DDS.DestinationOrderQosPolicy(
                            DDS.DestinationOrderQosPolicyKind.BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS);

    public static final DDS.HistoryQosPolicy defaultHistoryQosPolicy =
                        new DDS.HistoryQosPolicy(
                            DDS.HistoryQosPolicyKind.KEEP_LAST_HISTORY_QOS,
                            1);

    public static final DDS.ResourceLimitsQosPolicy defaultResourceLimitsQosPolicy =
                        new DDS.ResourceLimitsQosPolicy(
                            -1, -1, -1);

    public static final DDS.TransportPriorityQosPolicy defaultTransportPriorityQosPolicy =
                        new DDS.TransportPriorityQosPolicy(
                            0);

    public static final DDS.LifespanQosPolicy defaultLifespanQosPolicy =
                        new DDS.LifespanQosPolicy(
                            Utilities.InfiniteDuration);

    public static final DDS.OwnershipQosPolicy defaultOwnershipQosPolicy =
                        new DDS.OwnershipQosPolicy(
                            DDS.OwnershipQosPolicyKind.SHARED_OWNERSHIP_QOS);

    protected static DDS.OwnershipQosPolicy deepCopy(DDS.OwnershipQosPolicy o)
    {
        return new DDS.OwnershipQosPolicy(o.kind);
    }

    public static final DDS.OwnershipStrengthQosPolicy defaultOwnershipStrengthQosPolicy =
                        new DDS.OwnershipStrengthQosPolicy(
                            0);

    public static final DDS.WriterDataLifecycleQosPolicy defaultWriterDataLifecycleQosPolicy =
                        new DDS.WriterDataLifecycleQosPolicy(
                            true,
                            Utilities.InfiniteDuration,
                            Utilities.InfiniteDuration);

    public static final DDS.TimeBasedFilterQosPolicy defaultTimeBasedFilterQosPolicy =
                        new DDS.TimeBasedFilterQosPolicy(
                            Utilities.ZeroDuration);

    public static final DDS.InvalidSampleVisibilityQosPolicy defaultInvalidSampleVisibilityQosPolicy =
                        new DDS.InvalidSampleVisibilityQosPolicy(
                            DDS.InvalidSampleVisibilityQosPolicyKind.MINIMUM_INVALID_SAMPLES);

    public static final DDS.ReaderDataLifecycleQosPolicy defaultReaderDataLifecycleQosPolicy =
                        new DDS.ReaderDataLifecycleQosPolicy(
                            Utilities.InfiniteDuration,
                            Utilities.InfiniteDuration,
                            false,
                            true,
                            Utilities.defaultInvalidSampleVisibilityQosPolicy);

    public static final DDS.ShareQosPolicy defaultShareQosPolicy =
                        new DDS.ShareQosPolicy(
                            "",
                            false);

    public static final DDS.ReaderLifespanQosPolicy defaultReaderLifespanQosPolicy =
                        new DDS.ReaderLifespanQosPolicy(
                            false,
                            Utilities.InfiniteDuration);

    public static final DDS.SubscriptionKeyQosPolicy defaultSubscriptionKeyQosPolicy =
                        new DDS.SubscriptionKeyQosPolicy(
                            false,
                            new String[]{});

    public static final DDS.ViewKeyQosPolicy defaultViewKeyQosPolicy =
                        new DDS.ViewKeyQosPolicy(
                            false,
                            new String[]{});


/**
 *
 * Default OSPL Qos values.
 *
 *************************************/
    public static final DDS.DomainParticipantFactoryQos defaultDomainParticipantFactoryQos =
                        new DDS.DomainParticipantFactoryQos(
                                Utilities.defaultEntityFactoryQosPolicy);

    public static final DDS.DomainParticipantQos defaultDomainParticipantQos =
                        new DDS.DomainParticipantQos(
                                Utilities.defaultUserDataQosPolicy,
                                Utilities.defaultEntityFactoryQosPolicy,
                                Utilities.defaultSchedulingQosPolicy,
                                Utilities.defaultSchedulingQosPolicy);

    public static final DDS.SubscriberQos defaultSubscriberQos =
                        new DDS.SubscriberQos(
                                Utilities.defaultPresentationQosPolicy,
                                Utilities.defaultPartitionQosPolicy,
                                Utilities.defaultGroupDataQosPolicy,
                                Utilities.defaultEntityFactoryQosPolicy,
                                Utilities.defaultShareQosPolicy);

    public static final DDS.PublisherQos defaultPublisherQos =
                        new DDS.PublisherQos(
                                Utilities.defaultPresentationQosPolicy,
                                Utilities.defaultPartitionQosPolicy,
                                Utilities.defaultGroupDataQosPolicy,
                                Utilities.defaultEntityFactoryQosPolicy);

    public static final DDS.TopicQos defaultTopicQos =
                        new DDS.TopicQos(
                                Utilities.defaultTopicDataQosPolicy,
                                Utilities.defaultDurabilityQosPolicy,
                                Utilities.defaultDurabilityServiceQosPolicy,
                                Utilities.defaultDeadlineQosPolicy,
                                Utilities.defaultLatencyBudgetQosPolicy,
                                Utilities.defaultLivelinessQosPolicy,
                                Utilities.defaultDataReaderReliabilityQosPolicy,
                                Utilities.defaultDestinationOrderQosPolicy,
                                Utilities.defaultHistoryQosPolicy,
                                Utilities.defaultResourceLimitsQosPolicy,
                                Utilities.defaultTransportPriorityQosPolicy,
                                Utilities.defaultLifespanQosPolicy,
                                Utilities.defaultOwnershipQosPolicy);

    public static final DDS.DataWriterQos defaultDataWriterQos =
                        new DDS.DataWriterQos(
                                Utilities.defaultDurabilityQosPolicy,
                                Utilities.defaultDeadlineQosPolicy,
                                Utilities.defaultLatencyBudgetQosPolicy,
                                Utilities.defaultLivelinessQosPolicy,
                                Utilities.defaultDataWriterReliabilityQosPolicy,
                                Utilities.defaultDestinationOrderQosPolicy,
                                Utilities.defaultHistoryQosPolicy,
                                Utilities.defaultResourceLimitsQosPolicy,
                                Utilities.defaultTransportPriorityQosPolicy,
                                Utilities.defaultLifespanQosPolicy,
                                Utilities.defaultUserDataQosPolicy,
                                Utilities.defaultOwnershipQosPolicy,
                                Utilities.defaultOwnershipStrengthQosPolicy,
                                Utilities.defaultWriterDataLifecycleQosPolicy);

    public static final DDS.DataReaderQos defaultDataReaderQos =
                        new DDS.DataReaderQos(
                                Utilities.defaultDurabilityQosPolicy,
                                Utilities.defaultDeadlineQosPolicy,
                                Utilities.defaultLatencyBudgetQosPolicy,
                                Utilities.defaultLivelinessQosPolicy,
                                Utilities.defaultDataReaderReliabilityQosPolicy,
                                Utilities.defaultDestinationOrderQosPolicy,
                                Utilities.defaultHistoryQosPolicy,
                                Utilities.defaultResourceLimitsQosPolicy,
                                Utilities.defaultUserDataQosPolicy,
                                Utilities.defaultOwnershipQosPolicy,
                                Utilities.defaultTimeBasedFilterQosPolicy,
                                Utilities.defaultReaderDataLifecycleQosPolicy,
                                Utilities.defaultSubscriptionKeyQosPolicy,
                                Utilities.defaultReaderLifespanQosPolicy,
                                Utilities.defaultShareQosPolicy);


    public static final DDS.DataReaderViewQos defaultDataReaderViewQos =
                        new DDS.DataReaderViewQos(
                                Utilities.defaultViewKeyQosPolicy);

/**
 *
 * OSPL Qos deepCopy operations.
 *
 *************************************/
    protected static byte[] deepCopy(byte[] o)
    {
        byte[] n = new byte[o.length];
        for (int i=0; i<o.length; i++) {
            n[i] = o[i];
        }
        return n;
    }

    protected static String[] deepCopy(String[] o)
    {
        String[] name = new String[o.length];
        for (int i=0; i<o.length; i++) {
            name[i] = new String(o[i]);
        }
        return name;
    }

    protected static DDS.Duration_t deepCopy(DDS.Duration_t o)
    {
        return new DDS.Duration_t(o.sec, o.nanosec);
    }

    protected static DDS.EntityFactoryQosPolicy deepCopy(DDS.EntityFactoryQosPolicy o)
    {
        return new DDS.EntityFactoryQosPolicy(o.autoenable_created_entities);
    }

    protected static DDS.UserDataQosPolicy deepCopy(DDS.UserDataQosPolicy o)
    {
        return new DDS.UserDataQosPolicy(deepCopy(o.value));
    }

    protected static DDS.SchedulingClassQosPolicy deepCopy(DDS.SchedulingClassQosPolicy o)
    {
        return new DDS.SchedulingClassQosPolicy(o.kind);
    }

    protected static DDS.SchedulingPriorityQosPolicy deepCopy(DDS.SchedulingPriorityQosPolicy o)
    {
        return new DDS.SchedulingPriorityQosPolicy(o.kind);
    }

    protected static DDS.SchedulingQosPolicy deepCopy(DDS.SchedulingQosPolicy o)
    {
        return new DDS.SchedulingQosPolicy(deepCopy(o.scheduling_class),
                                           deepCopy(o.scheduling_priority_kind),
                                           o.scheduling_priority);
    }

    protected static DDS.PresentationQosPolicy deepCopy(DDS.PresentationQosPolicy o)
    {
        return new DDS.PresentationQosPolicy(o.access_scope,
                                             o.coherent_access,
                                             o.ordered_access);
    }

    protected static DDS.PartitionQosPolicy deepCopy(DDS.PartitionQosPolicy o)
    {
        return new DDS.PartitionQosPolicy(deepCopy(o.name));
    }

    protected static DDS.GroupDataQosPolicy deepCopy(DDS.GroupDataQosPolicy o)
    {
        return new DDS.GroupDataQosPolicy(deepCopy(o.value));
    }

    protected static DDS.TopicDataQosPolicy deepCopy(DDS.TopicDataQosPolicy o)
    {
        return new DDS.TopicDataQosPolicy(deepCopy(o.value));
    }

    protected static DDS.DurabilityQosPolicy deepCopy(DDS.DurabilityQosPolicy o)
    {
        return new DDS.DurabilityQosPolicy(o.kind);
    }

    protected static DDS.DurabilityServiceQosPolicy deepCopy(DDS.DurabilityServiceQosPolicy o)
    {
        return new DDS.DurabilityServiceQosPolicy(deepCopy(o.service_cleanup_delay),
                                                  o.history_kind,
                                                  o.history_depth,
                                                  o.max_samples,
                                                  o.max_instances,
                                                  o.max_samples_per_instance);
    }

    protected static DDS.DeadlineQosPolicy deepCopy(DDS.DeadlineQosPolicy o)
    {
        return new DDS.DeadlineQosPolicy(deepCopy(o.period));
    }

    protected static DDS.LatencyBudgetQosPolicy deepCopy(DDS.LatencyBudgetQosPolicy o)
    {
        return new DDS.LatencyBudgetQosPolicy(deepCopy(o.duration));
    }

    protected static DDS.LivelinessQosPolicy deepCopy(DDS.LivelinessQosPolicy o)
    {
        return new DDS.LivelinessQosPolicy(o.kind, deepCopy(o.lease_duration));
    }

    protected static DDS.ReliabilityQosPolicy deepCopy(DDS.ReliabilityQosPolicy o)
    {
        return new DDS.ReliabilityQosPolicy(o.kind, deepCopy(o.max_blocking_time), o.synchronous);
    }

    protected static DDS.DestinationOrderQosPolicy deepCopy(DDS.DestinationOrderQosPolicy o)
    {
        return new DDS.DestinationOrderQosPolicy(o.kind);
    }

    protected static DDS.HistoryQosPolicy deepCopy(DDS.HistoryQosPolicy o)
    {
        return new DDS.HistoryQosPolicy(o.kind, o.depth);
    }

    protected static DDS.ResourceLimitsQosPolicy deepCopy(DDS.ResourceLimitsQosPolicy o)
    {
        return new DDS.ResourceLimitsQosPolicy(o.max_samples,
                                               o.max_instances,
                                               o.max_samples_per_instance);
    }

    protected static DDS.TransportPriorityQosPolicy deepCopy(DDS.TransportPriorityQosPolicy o)
    {
        return new DDS.TransportPriorityQosPolicy(o.value);
    }

    protected static DDS.LifespanQosPolicy deepCopy(DDS.LifespanQosPolicy o)
    {
        return new DDS.LifespanQosPolicy(deepCopy(o.duration));
    }

    protected static DDS.OwnershipStrengthQosPolicy deepCopy(DDS.OwnershipStrengthQosPolicy o)
    {
        return new DDS.OwnershipStrengthQosPolicy(o.value);
    }

    protected static DDS.WriterDataLifecycleQosPolicy deepCopy(DDS.WriterDataLifecycleQosPolicy o)
    {
        return new DDS.WriterDataLifecycleQosPolicy(o.autodispose_unregistered_instances,
                                                    deepCopy(o.autopurge_suspended_samples_delay),
                                                    deepCopy(o.autounregister_instance_delay));
    }

    protected static DDS.TimeBasedFilterQosPolicy deepCopy(DDS.TimeBasedFilterQosPolicy o)
    {
        return new DDS.TimeBasedFilterQosPolicy(deepCopy(o.minimum_separation));
    }

    protected static DDS.InvalidSampleVisibilityQosPolicy deepCopy(DDS.InvalidSampleVisibilityQosPolicy o)
    {
        return new DDS.InvalidSampleVisibilityQosPolicy(o.kind);
    }

    protected static DDS.ReaderDataLifecycleQosPolicy deepCopy(DDS.ReaderDataLifecycleQosPolicy o)
    {
        return new DDS.ReaderDataLifecycleQosPolicy(deepCopy(o.autopurge_nowriter_samples_delay),
                                                    deepCopy(o.autopurge_disposed_samples_delay),
                                                    o.autopurge_dispose_all,
                                                    o.enable_invalid_samples,
                                                    o.invalid_sample_visibility);
    }

    protected static DDS.ShareQosPolicy deepCopy(DDS.ShareQosPolicy o)
    {
        return new DDS.ShareQosPolicy(o.name, o.enable);
    }

    protected static DDS.ReaderLifespanQosPolicy deepCopy(DDS.ReaderLifespanQosPolicy o)
    {
        return new DDS.ReaderLifespanQosPolicy(o.use_lifespan, deepCopy(o.duration));
    }

    protected static DDS.SubscriptionKeyQosPolicy deepCopy(DDS.SubscriptionKeyQosPolicy o)
    {
        return new DDS.SubscriptionKeyQosPolicy(o.use_key_list, deepCopy(o.key_list));
    }

    protected static DDS.DomainParticipantFactoryQos deepCopy(DDS.DomainParticipantFactoryQos o)
    {
        return new DDS.DomainParticipantFactoryQos(deepCopy(o.entity_factory));
    }

    protected static DDS.DomainParticipantQos deepCopy(DDS.DomainParticipantQos o)
    {
        return new DDS.DomainParticipantQos(deepCopy(o.user_data),
                                            deepCopy(o.entity_factory),
                                            deepCopy(o.watchdog_scheduling),
                                            deepCopy(o.listener_scheduling));
    }

    protected static DDS.TopicQos deepCopy(DDS.TopicQos o)
    {
        return new DDS.TopicQos(deepCopy(o.topic_data),
                                deepCopy(o.durability),
                                deepCopy(o.durability_service),
                                deepCopy(o.deadline),
                                deepCopy(o.latency_budget),
                                deepCopy(o.liveliness),
                                deepCopy(o.reliability),
                                deepCopy(o.destination_order),
                                deepCopy(o.history),
                                deepCopy(o.resource_limits),
                                deepCopy(o.transport_priority),
                                deepCopy(o.lifespan),
                                deepCopy(o.ownership));
    }

    public static DDS.PublisherQos deepCopy(DDS.PublisherQos o)
    {
        return new DDS.PublisherQos(deepCopy(o.presentation),
                                    deepCopy(o.partition),
                                    deepCopy(o.group_data),
                                    deepCopy(o.entity_factory));
    }

    public static DDS.SubscriberQos deepCopy(DDS.SubscriberQos o)
    {
        return new DDS.SubscriberQos(deepCopy(o.presentation),
                                     deepCopy(o.partition),
                                     deepCopy(o.group_data),
                                     deepCopy(o.entity_factory),
                                     deepCopy(o.share));
    }

    public static DDS.DataWriterQos deepCopy(DDS.DataWriterQos o)
    {
        return new DDS.DataWriterQos(deepCopy(o.durability),
                                     deepCopy(o.deadline),
                                     deepCopy(o.latency_budget),
                                     deepCopy(o.liveliness),
                                     deepCopy(o.reliability),
                                     deepCopy(o.destination_order),
                                     deepCopy(o.history),
                                     deepCopy(o.resource_limits),
                                     deepCopy(o.transport_priority),
                                     deepCopy(o.lifespan),
                                     deepCopy(o.user_data),
                                     deepCopy(o.ownership),
                                     deepCopy(o.ownership_strength),
                                     deepCopy(o.writer_data_lifecycle));
    }

    public static DDS.DataReaderQos deepCopy(DDS.DataReaderQos o)
    {
        return new DDS.DataReaderQos(deepCopy(o.durability),
                                     deepCopy(o.deadline),
                                     deepCopy(o.latency_budget),
                                     deepCopy(o.liveliness),
                                     deepCopy(o.reliability),
                                     deepCopy(o.destination_order),
                                     deepCopy(o.history),
                                     deepCopy(o.resource_limits),
                                     deepCopy(o.user_data),
                                     deepCopy(o.ownership),
                                     deepCopy(o.time_based_filter),
                                     deepCopy(o.reader_data_lifecycle),
                                     deepCopy(o.subscription_keys),
                                     deepCopy(o.reader_lifespan),
                                     deepCopy(o.share));
    }

    public static DDS.DataReaderViewQos deepCopy(DDS.DataReaderViewQos o)
    {
        return new DDS.DataReaderViewQos(deepCopy(o.view_keys));
    }

    protected static DDS.ViewKeyQosPolicy deepCopy(DDS.ViewKeyQosPolicy o) {
        return new DDS.ViewKeyQosPolicy(o.use_key_list, deepCopy(o.key_list));
    }

    protected static int checkDuration(DDS.Duration_t o)
    {
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                "Duration_t 'null' is invalid");
        } else if (!((o.sec == DDS.DURATION_INFINITE_SEC.value &&
                      o.nanosec == DDS.DURATION_INFINITE_NSEC.value) ||
                     (o.sec != DDS.DURATION_INFINITE_SEC.value &&
                      o.nanosec < 1000000000)))
        {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                "Duration_t is invalid, seconds '" + o.sec +
                "', nanoseconds '" + o.nanosec + "'.");
        } else if (o.sec != DDS.DURATION_INFINITE_SEC.value && o.sec < 0 )
        {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,"Negative Duration_t seconds is not allowed");
        }

        return result;
    }

    public static int checkTime (DDS.Time_t o)
    {
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                "Time_t 'null' is invalid.");
        } else {
            try {
                if (o.getClass().getField("sec").getType() != Long.TYPE) {
                    if (!((o.sec == Integer.MAX_VALUE &&
                           o.nanosec == Integer.MAX_VALUE) ||
                          (o.sec != Integer.MAX_VALUE &&
                           o.nanosec < 1000000000)))
                    {
                        result = DDS.RETCODE_BAD_PARAMETER.value;
                        ReportStack.report(result,
                                           "Time_t is invalid, seconds '" + o.sec +
                                           "', nanoseconds '" + o.nanosec + "'.");
                    }
                } else {
                    if (!(((long)o.sec == Long.MAX_VALUE &&
                            o.nanosec == Integer.MAX_VALUE) ||
                           ((long)o.sec != Long.MAX_VALUE &&
                            o.nanosec < 1000000000)))
                    {
                         result = DDS.RETCODE_BAD_PARAMETER.value;
                         ReportStack.report(result,
                                            "Time_t is invalid, seconds '" + o.sec +
                                            "', nanoseconds '" + o.nanosec + "'.");
                    }
                }
            } catch (NoSuchFieldException e) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                ReportStack.report(result,
                    "Time_t is invalid.");
            } catch (SecurityException e) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                ReportStack.report(result,
                    "Time_t is invalid.");
            }
        }
        return result;
    }

    protected static int checkPolicy(DDS.EntityFactoryQosPolicy o)
    {
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                "EntityFactoryQosPolicy 'null' is invalid.");
        }

        return result;
    }

    protected static int checkPolicy(DDS.UserDataQosPolicy o)
    {
        String policy = "UserDataQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.value == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".value 'null' is invalid.");
        }

        return result;
    }

    protected static int checkPolicy(DDS.SchedulingClassQosPolicy o)
    {
        String policy = "SchedulingClassQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.kind != DDS.SchedulingClassQosPolicyKind.SCHEDULE_DEFAULT &&
                   o.kind != DDS.SchedulingClassQosPolicyKind.SCHEDULE_TIMESHARING &&
                   o.kind != DDS.SchedulingClassQosPolicyKind.SCHEDULE_REALTIME)
        {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".kind is invalid.");
        }

        return result;
    }

    protected static int checkPolicy(DDS.SchedulingPriorityQosPolicy o)
    {
        String policy = "SchedulingPriorityQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.kind != DDS.SchedulingPriorityQosPolicyKind.PRIORITY_RELATIVE &&
                   o.kind != DDS.SchedulingPriorityQosPolicyKind.PRIORITY_ABSOLUTE)
        {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".kind is invalid.");
        }

        return result;
    }

    protected static int checkPolicy(DDS.SchedulingQosPolicy o)
    {
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                "SchedulingQosPolicy 'null' is invalid.");
        } else {
            result = checkPolicy(o.scheduling_class);
            if (result == DDS.RETCODE_OK.value) {
                result = checkPolicy(o.scheduling_priority_kind);
            }
        }

        return result;
    }

    protected static int checkPolicy(DDS.PresentationQosPolicy o)
    {
        String policy = "PresentationQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.access_scope != DDS.PresentationQosPolicyAccessScopeKind.TOPIC_PRESENTATION_QOS &&
                   o.access_scope != DDS.PresentationQosPolicyAccessScopeKind.GROUP_PRESENTATION_QOS &&
                   o.access_scope != DDS.PresentationQosPolicyAccessScopeKind.INSTANCE_PRESENTATION_QOS)
        {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".access_scope is invalid.");
        }

        return result;
    }

    protected static int checkPolicy(DDS.PartitionQosPolicy o)
    {
        String policy = "PartitionQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.name == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".name 'null' is invalid.");
        }

        return result;
    }

    protected static int checkPolicy(DDS.GroupDataQosPolicy o)
    {
        String policy = "GroupDataQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.value == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".value 'null' is invalid.");
        }

        return result;
    }

    protected static int checkPolicy(DDS.TopicDataQosPolicy o)
    {
        String policy = "TopicDataQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.value == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".value 'null' is invalid.");
        }

        return result;
    }

    protected static int checkPolicy(DDS.DurabilityQosPolicy o)
    {
        String policy = "DurabilityQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.kind != DDS.DurabilityQosPolicyKind.VOLATILE_DURABILITY_QOS &&
                   o.kind != DDS.DurabilityQosPolicyKind.TRANSIENT_LOCAL_DURABILITY_QOS &&
                   o.kind != DDS.DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS &&
                   o.kind != DDS.DurabilityQosPolicyKind.PERSISTENT_DURABILITY_QOS)
        {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".kind is invalid.");
        }

        return result;
    }

    protected static int checkPolicy(DDS.DurabilityServiceQosPolicy o)
    {
        String policy = "DurabilityServiceQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.history_kind == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, policy + ".history_kind 'null' is invalid.");
        } else if (o.history_kind.value() != DDS.HistoryQosPolicyKind.KEEP_ALL_HISTORY_QOS.value() &&
                   o.history_kind.value() != DDS.HistoryQosPolicyKind.KEEP_LAST_HISTORY_QOS.value())
        {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, policy + ".history_kind is invalid.");
        } else if (o.history_kind.value() == DDS.HistoryQosPolicyKind.KEEP_LAST_HISTORY_QOS.value() &&
                   o.history_depth <= 0)
        {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, policy + ".history_depth '" +
                                       o.history_depth + "' is invalid.");
        } else {
            result = checkDuration(o.service_cleanup_delay);
            if (result == DDS.RETCODE_OK.value) {
                if (o.max_samples < 0 &&
                    o.max_samples != DDS.LENGTH_UNLIMITED.value)
                {
                    result = DDS.RETCODE_BAD_PARAMETER.value;
                    ReportStack.report(result,
                        policy + ".max_samples '" +
                        o.max_samples + "' is invalid.");
                } else if (o.max_instances < 0 &&
                           o.max_instances != DDS.LENGTH_UNLIMITED.value)
                {
                    result = DDS.RETCODE_BAD_PARAMETER.value;
                    ReportStack.report(result,
                       policy + ".max_instances '" +
                       o.max_instances + "' is invalid.");
                } else if (o.max_samples_per_instance < 0 &&
                           o.max_samples_per_instance != DDS.LENGTH_UNLIMITED.value)
                {
                    result = DDS.RETCODE_BAD_PARAMETER.value;
                    ReportStack.report(result,
                        policy + ".max_samples_per_instance '" +
                        o.max_samples_per_instance + "' is invalid.");
                } else if (o.history_depth > o.max_samples_per_instance &&
                           o.max_samples_per_instance != DDS.LENGTH_UNLIMITED.value)
                {
                    result = DDS.RETCODE_INCONSISTENT_POLICY.value;
                    ReportStack.report(result,
                        policy + ".history_depth is greater than " +
                        policy + ".max_samples_per_instance.");
                }
            }
        }

        return result;
    }

    protected static int checkPolicy(DDS.DeadlineQosPolicy o)
    {
        String policy = "DeadlineQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else {
            result = checkDuration(o.period);
        }

        return result;
    }

    protected static int checkPolicy(DDS.LatencyBudgetQosPolicy o)
    {
        String policy = "LatencyBudgetQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else {
            result = checkDuration(o.duration);
        }

        return result;
    }

    protected static int checkPolicy(DDS.LivelinessQosPolicy o)
    {
        String policy = "LivelinessQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.kind != DDS.LivelinessQosPolicyKind.AUTOMATIC_LIVELINESS_QOS &&
                   o.kind != DDS.LivelinessQosPolicyKind.MANUAL_BY_PARTICIPANT_LIVELINESS_QOS &&
                   o.kind != DDS.LivelinessQosPolicyKind.MANUAL_BY_TOPIC_LIVELINESS_QOS)
        {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".kind is invalid.");
        } else {
            result = checkDuration(o.lease_duration);
        }

        return result;
    }

    protected static int checkPolicy(DDS.ReliabilityQosPolicy o)
    {
        String policy = "ReliabilityQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.kind == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".kind 'null' is invalid.");
        } else if (o.synchronous) {
            result = checkDuration(o.max_blocking_time);
        }

        return result;
    }

    protected static int checkPolicy(DDS.DestinationOrderQosPolicy o)
    {
        String policy = "DestinationOrderQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.kind != DDS.DestinationOrderQosPolicyKind.BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS &&
                   o.kind != DDS.DestinationOrderQosPolicyKind.BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS)
        {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".kind is invalid.");
        }

        return result;
    }

    protected static int checkPolicy(DDS.OwnershipQosPolicy o)
    {
        String policy = "OwnershipQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.kind != DDS.OwnershipQosPolicyKind.SHARED_OWNERSHIP_QOS &&
                   o.kind != DDS.OwnershipQosPolicyKind.EXCLUSIVE_OWNERSHIP_QOS)
        {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".kind is invalid.");
        }

        return result;
    }

    protected static int checkPolicy(DDS.HistoryQosPolicy o)
    {
        String policy = "HistoryQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.kind == DDS.HistoryQosPolicyKind.KEEP_LAST_HISTORY_QOS) {
            if (o.depth <= 0) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                ReportStack.report(result,
                    policy + ".depth '" + o.depth + "' is invalid.");
            }
        } else if (o.kind != DDS.HistoryQosPolicyKind.KEEP_ALL_HISTORY_QOS) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".kind is invalid.");
        }

        return result;
    }

    protected static int checkPolicy(DDS.ResourceLimitsQosPolicy o)
    {
        String policy = "ResourceLimitsQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.max_samples < 0 &&
                   o.max_samples != DDS.LENGTH_UNLIMITED.value)
        {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".max_samples '" +
                o.max_samples + "' is invalid.");
        } else if (o.max_instances < 0 &&
                   o.max_instances != DDS.LENGTH_UNLIMITED.value)
        {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".max_instances '" +
                o.max_instances + "' is invalid.");
        } else if (o.max_samples_per_instance < 0 &&
                   o.max_samples_per_instance != DDS.LENGTH_UNLIMITED.value)
        {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".max_samples_per_instance '" +
                o.max_samples_per_instance + "' is invalid.");
        }

        return result;
    }

    protected static int checkPolicy(DDS.TransportPriorityQosPolicy o)
    {
        String policy = "TransportPriorityQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        }

        return result;
    }

    protected static int checkPolicy(DDS.LifespanQosPolicy o)
    {
        String policy = "LifespanQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else {
            result = checkDuration(o.duration);
        }

        return result;
    }

    protected static int checkPolicy(DDS.OwnershipStrengthQosPolicy o)
    {
        String policy = "OwnershipStrengthQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        }

        return result;
    }

    protected static int checkPolicy(DDS.WriterDataLifecycleQosPolicy o)
    {
        String policy = "WriterDataLifecycleQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        }

        return result;
    }

    protected static int checkPolicy(DDS.TimeBasedFilterQosPolicy o)
    {
        String policy = "TimeBasedFilterQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_OK.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else {
            result = checkDuration(o.minimum_separation);
        }

        return result;
    }

    protected static int checkPolicy(DDS.InvalidSampleVisibilityQosPolicy o)
    {
        String policy = "InvalidSampleVisibilityQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.kind == DDS.InvalidSampleVisibilityQosPolicyKind.ALL_INVALID_SAMPLES) {
            result = DDS.RETCODE_UNSUPPORTED.value;
            ReportStack.report(result,
                policy + ".kind 'ALL_INVALID_SAMPLES' is unsupported.");
        } else if (o.kind != DDS.InvalidSampleVisibilityQosPolicyKind.NO_INVALID_SAMPLES &&
                   o.kind != DDS.InvalidSampleVisibilityQosPolicyKind.MINIMUM_INVALID_SAMPLES)
        {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".kind is invalid.");
        }

        return result;
    }

    protected static int checkPolicy(DDS.ReaderDataLifecycleQosPolicy o)
    {
        String policy = "ReaderDataLifecycleQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if ((result = checkDuration(o.autopurge_nowriter_samples_delay))
                        == DDS.RETCODE_OK.value &&
                   (result = checkDuration(o.autopurge_disposed_samples_delay))
                        == DDS.RETCODE_OK.value &&
                   (result = checkPolicy(o.invalid_sample_visibility))
                        == DDS.RETCODE_OK.value)
        {
            if (USE_OLD_ReaderDataLifecycleQosPolicy) {
                if (o.enable_invalid_samples == false) {
                    ReportStack.deprecated("DataReaderQos.ReaderDataLifecycle.enable_invalid_samples " +
                                           "is deprecated and will be replaced by " +
                                           "DataReaderQos.ReaderDataLifecycle.invalid_sample_visibility.");
                    if (o.invalid_sample_visibility.kind !=
                            DDS.InvalidSampleVisibilityQosPolicyKind.MINIMUM_INVALID_SAMPLES)
                    {
                        result = DDS.RETCODE_INCONSISTENT_POLICY.value;
                        ReportStack.report(result,
                            policy + " invalid, " + policy + ".enable_invalid_samples inconsistent with " +
                            policy + ".invalid_sample_visibility.");
                    }
                }
            }
        }

        return result;
    }

    protected static int checkPolicy(DDS.ShareQosPolicy o)
    {
        String policy = "ShareQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.enable == true) {
            if (o.name == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                ReportStack.report(result,
                    policy + ".name 'null' is invalid.");
            }
        }

        return result;
    }

    protected static int checkPolicy(DDS.ReaderLifespanQosPolicy o)
    {
        String policy = "ReaderLifespanQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.use_lifespan == true) {
            result = checkDuration(o.duration);
        }

        return result;
    }

    protected static int checkPolicy(DDS.SubscriptionKeyQosPolicy o)
    {
        String policy = "SubscriptionKeyQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.use_key_list == true && o.key_list == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".key_list 'null' is invalid.");
        }

        return result;
    }

    protected static int checkPolicy(DDS.ViewKeyQosPolicy o)
    {
        String policy = "ViewKeyQosPolicy";
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + " 'null' is invalid.");
        } else if (o.use_key_list == true && o.key_list == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                policy + ".key_list 'null' is invalid.");
        }

        return result;
    }

    protected static int checkPolicyConsistency(
        DDS.HistoryQosPolicy history,
        DDS.ResourceLimitsQosPolicy resource_limits)
    {
        int result = DDS.RETCODE_OK.value;

        if (history == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                "HistoryQosPolicy 'null' is invalid.");
        } else if (resource_limits == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                "ResourceLimitsQosPolicy 'null' is invalid.");
        } else if (history.kind == DDS.HistoryQosPolicyKind.KEEP_LAST_HISTORY_QOS) {
            if (resource_limits.max_samples_per_instance != DDS.LENGTH_UNLIMITED.value &&
                history.depth > resource_limits.max_samples_per_instance)
            {
                result = DDS.RETCODE_INCONSISTENT_POLICY.value;
                ReportStack.report(result,
                    "HistoryQosPolicy.depth is greater than " +
                    "ResourceLimitsQosPolicy.max_samples_per_instance.");
            }
        }

        return result;
    }

    protected static int checkPolicyConsistency(
        DDS.DeadlineQosPolicy deadline,
        DDS.TimeBasedFilterQosPolicy time_based_filter)
    {
        int result = DDS.RETCODE_OK.value;

        if (deadline == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                "DeadlineQosPolicy 'null' is invalid.");
        } else if (time_based_filter == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                "TimeBasedFilterQosPolicy 'null' is invalid.");
        } else if ((deadline.period.sec < time_based_filter.minimum_separation.sec) ||
                   (deadline.period.sec == time_based_filter.minimum_separation.sec &&
                    deadline.period.nanosec < time_based_filter.minimum_separation.nanosec))
        {
            result = DDS.RETCODE_INCONSISTENT_POLICY.value;
            ReportStack.report(result,
                "DeadlineQosPolicy.period is less than " +
                "TimeBasedFilterQosPolicy.separation");
        }

        return result;
    }

    protected static int checkQos(DDS.DomainParticipantFactoryQos o)
    {
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                "DomainParticipantFactoryQos 'null' is invalid.");
        } else if (o != DDS.PARTICIPANTFACTORY_QOS_DEFAULT.value) {
            result = checkPolicy(o.entity_factory);
        }

        return result;
    }

    protected static int checkQos(DDS.DomainParticipantQos o)
    {
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                "DomainParticipantQos 'null' is invalid.");
        } else if (o != DDS.PARTICIPANT_QOS_DEFAULT.value) {
            if ((result = checkPolicy(o.user_data))             == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.entity_factory))        == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.watchdog_scheduling))   == DDS.RETCODE_OK.value)
            {
                result = checkPolicy(o.listener_scheduling);
            }
        }

        return result;
    }

    protected static int checkQos(DDS.TopicQos o)
    {
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                "TopicQos 'null' is invalid.");
        } else if (o != DDS.TOPIC_QOS_DEFAULT.value) {
            if ((result = checkPolicy(o.topic_data))         == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.durability))         == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.durability_service)) == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.deadline))           == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.latency_budget))     == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.liveliness))         == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.reliability))        == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.destination_order))  == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.history))            == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.resource_limits))    == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.transport_priority)) == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.lifespan))           == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.ownership))          == DDS.RETCODE_OK.value)
            {
                result = checkPolicyConsistency(o.history, o.resource_limits);
            }
        }

        return result;
    }

    protected static int checkQos(DDS.PublisherQos o)
    {
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                "PublisherQos 'null' is invalid.");
        } else if (o != DDS.PUBLISHER_QOS_DEFAULT.value) {
            if ((result = checkPolicy(o.presentation)) == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.partition))    == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.group_data))   == DDS.RETCODE_OK.value)
            {
                result = checkPolicy(o.entity_factory);
            }
        }

        return result;
    }

    protected static int checkQos(DDS.SubscriberQos o)
    {
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_OK.value;
            ReportStack.report(result,
                "SubscriberQos 'null' is invalid.");
        } else if (o != DDS.SUBSCRIBER_QOS_DEFAULT.value) {
            if ((result = checkPolicy(o.presentation)) == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.partition))    == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.group_data))   == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.share))        == DDS.RETCODE_OK.value)
            {
                result = checkPolicy(o.entity_factory);
            }
        }

        return result;
    }

    protected static int checkQos(DDS.DataWriterQos o)
    {
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                "DataWriterQos 'null' is invalid.");
        } else if (o != DDS.DATAWRITER_QOS_DEFAULT.value &&
                   o != DDS.DATAWRITER_QOS_USE_TOPIC_QOS.value)
        {
            if ((result = checkPolicy(o.durability))            == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.deadline))              == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.latency_budget))        == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.liveliness))            == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.reliability))           == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.destination_order))     == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.history))               == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.resource_limits))       == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.transport_priority))    == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.lifespan))              == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.user_data))             == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.ownership))             == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.ownership_strength))    == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.writer_data_lifecycle)) == DDS.RETCODE_OK.value)
            {
                result = checkPolicyConsistency(o.history, o.resource_limits);
            }
        }

        return result;
    }

    protected static int checkQos(DDS.DataReaderQos o)
    {
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                "DataReaderQos 'null' is invalid.");
        } else if (o != DDS.DATAREADER_QOS_DEFAULT.value &&
                   o != DDS.DATAREADER_QOS_USE_TOPIC_QOS.value)
        {
            if ((result = checkPolicy(o.durability))            == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.deadline))              == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.latency_budget))        == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.liveliness))            == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.reliability))           == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.destination_order))     == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.history))               == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.resource_limits))       == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.user_data))             == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.ownership))             == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.time_based_filter))     == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.reader_data_lifecycle)) == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.subscription_keys))     == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.reader_lifespan))       == DDS.RETCODE_OK.value &&
                (result = checkPolicy(o.share))                 == DDS.RETCODE_OK.value)
            {
                result = checkPolicyConsistency(o.history, o.resource_limits);
                if (result == DDS.RETCODE_OK.value) {
                    result = checkPolicyConsistency(o.deadline, o.time_based_filter);
                }
            }
        }

        return result;
    }

    protected static int checkQos(DDS.DataReaderViewQos o)
    {
        int result = DDS.RETCODE_OK.value;

        if (o == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                "DataReaderViewQos 'null' is invalid.");
        } else if (o != DDS.DATAREADERVIEW_QOS_DEFAULT.value) {
            result = checkPolicy(o.view_keys);
        }

        return result;
    }



    private Utilities(){}

    static protected Object deepCopy(Object oldObj)
    {
        ObjectOutputStream oos = null;
        ObjectInputStream ois = null;
        try
        {
           ByteArrayOutputStream bos = new ByteArrayOutputStream();
           oos = new ObjectOutputStream(bos);
           // serialize and pass the object
           oos.writeObject(oldObj);
           oos.flush();
           ByteArrayInputStream bin = new ByteArrayInputStream(bos.toByteArray());
           ois = new ObjectInputStream(bin);
           // return the new object
           return ois.readObject();
        } catch(Exception e) {
           System.out.println("Exception in Utilities.deepCopy = " + e + "Object = " + oldObj);
           return null;
        }
        finally
        {
            try
            {
               oos.close();
               ois.close();
            } catch(Exception e) {
               System.out.println("Exception in Utilities.deepCopy = " + e);
            }
        }
    }

    public static RuntimeException createException(int errorCode, String errorMessage){
        RuntimeException exc = null;

        switch(errorCode){
        case EXCEPTION_TYPE_BAD_PARAM:
            exc = new org.omg.CORBA.BAD_PARAM(errorMessage);
            break;
        case EXCEPTION_TYPE_NO_MEMORY:
            exc = new org.omg.CORBA.NO_MEMORY(errorMessage);
            break;
        case EXCEPTION_TYPE_MARSHAL:
            exc = new org.omg.CORBA.MARSHAL(errorMessage);
            break;
        case EXCEPTION_TYPE_BAD_OPERATION:
            exc = new org.omg.CORBA.BAD_OPERATION(errorMessage);
            break;
        default:
            assert false: "Unknown error code.";
        }
        return exc;
    }

    protected static int checkSampleStateMask(int sample_states)
    {
        int result = DDS.RETCODE_OK.value;
        int mask = DDS.READ_SAMPLE_STATE.value |
                   DDS.NOT_READ_SAMPLE_STATE.value;

        if (sample_states != DDS.ANY_SAMPLE_STATE.value) {
            if ((sample_states & mask) == 0 ||
                (sample_states & ~mask) > 0)
            {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                ReportStack.report(result,
                    "SampleStateMask '" +
                    Integer.toString(sample_states, 2) + "' is invalid.");
            }
        }

        return result;
    }

    protected static int checkViewStateMask(int view_states)
    {
        int result = DDS.RETCODE_OK.value;
        int mask = DDS.NEW_VIEW_STATE.value |
                   DDS.NOT_NEW_VIEW_STATE.value;

        if (view_states != DDS.ANY_VIEW_STATE.value) {
            if ((view_states & mask) == 0 ||
                (view_states & ~mask) > 0)
            {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                ReportStack.report(result,
                    "ViewStateMask '" +
                    Integer.toString(view_states, 2) + "' is invalid.");
            }
        }

        return result;
    }

    protected static int checkInstanceStateMask(int instance_states)
    {
        int result = DDS.RETCODE_OK.value;
        int mask = DDS.ALIVE_INSTANCE_STATE.value |
                   DDS.NOT_ALIVE_NO_WRITERS_INSTANCE_STATE.value |
                   DDS.NOT_ALIVE_DISPOSED_INSTANCE_STATE.value;

        if (instance_states != DDS.ANY_INSTANCE_STATE.value) {
            if ((instance_states & mask) == 0 ||
                (instance_states & ~mask) > 0)
            {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                ReportStack.report(result,
                    "InstanceStateMask '" +
                    Integer.toString(instance_states, 2) + "' is invalid.");
            }
        }

        return result;
    }
}
