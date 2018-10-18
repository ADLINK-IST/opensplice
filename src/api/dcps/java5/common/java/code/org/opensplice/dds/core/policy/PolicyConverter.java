/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
package org.opensplice.dds.core.policy;

import java.util.Set;

import org.omg.dds.core.Duration;
import org.omg.dds.core.policy.Deadline;
import org.omg.dds.core.policy.DestinationOrder;
import org.omg.dds.core.policy.DestinationOrder.Kind;
import org.omg.dds.core.policy.Durability;
import org.omg.dds.core.policy.DurabilityService;
import org.omg.dds.core.policy.EntityFactory;
import org.omg.dds.core.policy.GroupData;
import org.omg.dds.core.policy.History;
import org.omg.dds.core.policy.LatencyBudget;
import org.omg.dds.core.policy.Lifespan;
import org.omg.dds.core.policy.Liveliness;
import org.omg.dds.core.policy.Ownership;
import org.omg.dds.core.policy.OwnershipStrength;
import org.omg.dds.core.policy.Partition;
import org.omg.dds.core.policy.Presentation;
import org.omg.dds.core.policy.Presentation.AccessScopeKind;
import org.opensplice.dds.core.DDSExceptionImpl;
import org.opensplice.dds.core.DurationImpl;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.Utilities;
import org.opensplice.dds.core.policy.Scheduling.SchedulingClass;
import org.opensplice.dds.core.policy.Scheduling.SchedulingKind;
import org.omg.dds.core.policy.QosPolicy;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.core.policy.ResourceLimits;
import org.omg.dds.core.policy.TimeBasedFilter;
import org.omg.dds.core.policy.TopicData;
import org.omg.dds.core.policy.TransportPriority;
import org.omg.dds.core.policy.UserData;
import org.omg.dds.core.policy.WriterDataLifecycle;

public class PolicyConverter {
    public static DDS.UserDataQosPolicy convert(OsplServiceEnvironment env,
            UserData p) {
        return new DDS.UserDataQosPolicy(p.getValue());
    }

    public static UserData convert(OsplServiceEnvironment env,
            DDS.UserDataQosPolicy old) {
        return new UserDataImpl(env, old.value);
    }

    public static EntityFactory convert(OsplServiceEnvironment env,
            DDS.EntityFactoryQosPolicy old) {
        return new EntityFactoryImpl(env, old.autoenable_created_entities);
    }

    public static DDS.EntityFactoryQosPolicy convert(
            OsplServiceEnvironment env, EntityFactory p) {
        return new DDS.EntityFactoryQosPolicy(p.isAutoEnableCreatedEntities());
    }

    public static Scheduling convert(OsplServiceEnvironment env,
            DDS.SchedulingQosPolicy old) {
        Scheduling ls = new SchedulingImpl(env);

        switch (old.scheduling_class.kind.value()) {
        case DDS.SchedulingClassQosPolicyKind._SCHEDULE_DEFAULT:
            ls = ls.withSchedulingClass(SchedulingClass.DEFAULT);
            break;
        case DDS.SchedulingClassQosPolicyKind._SCHEDULE_REALTIME:
            ls = ls.withSchedulingClass(SchedulingClass.REALTIME);
            break;
        case DDS.SchedulingClassQosPolicyKind._SCHEDULE_TIMESHARING:
            ls = ls.withSchedulingClass(SchedulingClass.TIMESHARING);
            break;
        default:
            throw new DDSExceptionImpl(env,
                    "Failed to convert listenerSchedulingClass");
        }
        switch (old.scheduling_priority_kind.kind.value()) {
        case DDS.SchedulingPriorityQosPolicyKind._PRIORITY_ABSOLUTE:
            ls = ls.withKind(SchedulingKind.ABSOLUTE);
            break;
        case DDS.SchedulingPriorityQosPolicyKind._PRIORITY_RELATIVE:
            ls = ls.withKind(SchedulingKind.RELATIVE);
            break;
        default:
            throw new DDSExceptionImpl(env,
                    "Failed to convert listenerSchedulingKind");
        }
        return ls;
    }

    public static DDS.SchedulingQosPolicy convert(OsplServiceEnvironment env,
            Scheduling p) {
        DDS.SchedulingQosPolicy old = new DDS.SchedulingQosPolicy();

        switch (p.getSchedulingClass()) {
        case DEFAULT:
            old.scheduling_class = new DDS.SchedulingClassQosPolicy(
                    DDS.SchedulingClassQosPolicyKind.SCHEDULE_DEFAULT);
            break;
        case REALTIME:
            old.scheduling_class = new DDS.SchedulingClassQosPolicy(
                    DDS.SchedulingClassQosPolicyKind.SCHEDULE_REALTIME);
            break;
        case TIMESHARING:
            old.scheduling_class = new DDS.SchedulingClassQosPolicy(
                    DDS.SchedulingClassQosPolicyKind.SCHEDULE_TIMESHARING);
            break;
        default:
            throw new DDSExceptionImpl(env,
                    "Failed to convert listenerSchedulingClass");
        }

        switch (p.getKind()) {
        case ABSOLUTE:
            old.scheduling_priority_kind = new DDS.SchedulingPriorityQosPolicy(
                    DDS.SchedulingPriorityQosPolicyKind.PRIORITY_ABSOLUTE);
            break;
        case RELATIVE:
            old.scheduling_priority_kind = new DDS.SchedulingPriorityQosPolicy(
                    DDS.SchedulingPriorityQosPolicyKind.PRIORITY_RELATIVE);
            break;
        default:
            throw new DDSExceptionImpl(env,
                    "Failed to convert listenerSchedulingKind");
        }
        old.scheduling_priority = p.getPriority();

        return old;
    }

    public static Duration convert(OsplServiceEnvironment env,
            DDS.Duration_t old) {
        return new DurationImpl(env, old.sec, old.nanosec);
    }

    public static DDS.Duration_t convert(OsplServiceEnvironment env, Duration p) {
        return Utilities.convert(env, p);
    }

    public static Deadline convert(OsplServiceEnvironment env,
            DDS.DeadlineQosPolicy old) {
        return new DeadlineImpl(env, convert(env, old.period));
    }

    public static DDS.DeadlineQosPolicy convert(OsplServiceEnvironment env,
            Deadline p) {
        return new DDS.DeadlineQosPolicy(convert(env, p.getPeriod()));
    }

    public static DestinationOrder convert(OsplServiceEnvironment env,
            DDS.DestinationOrderQosPolicy old) {
        switch (old.kind.value()) {
        case DDS.DestinationOrderQosPolicyKind._BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS:
            return new DestinationOrderImpl(env, Kind.BY_RECEPTION_TIMESTAMP);
        case DDS.DestinationOrderQosPolicyKind._BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS:
            return new DestinationOrderImpl(env, Kind.BY_SOURCE_TIMESTAMP);
        default:
            throw new IllegalArgumentExceptionImpl(env,
                    "Unknown DestinationOrder kind.");
        }
    }

    public static DDS.DestinationOrderQosPolicy convert(
            OsplServiceEnvironment env, DestinationOrder p) {
        switch (p.getKind()) {
        case BY_RECEPTION_TIMESTAMP:
            return new DDS.DestinationOrderQosPolicy(
                    DDS.DestinationOrderQosPolicyKind.BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS);
        case BY_SOURCE_TIMESTAMP:
            return new DDS.DestinationOrderQosPolicy(
                    DDS.DestinationOrderQosPolicyKind.BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS);
        default:
            throw new IllegalArgumentExceptionImpl(env,
                    "Unknown DestinationOrder kind.");
        }
    }

    public static Durability convert(OsplServiceEnvironment env,
            DDS.DurabilityQosPolicy old) {
        switch (old.kind.value()) {
        case DDS.DurabilityQosPolicyKind._VOLATILE_DURABILITY_QOS:
            return new DurabilityImpl(env, Durability.Kind.VOLATILE);
        case DDS.DurabilityQosPolicyKind._TRANSIENT_LOCAL_DURABILITY_QOS:
            return new DurabilityImpl(env, Durability.Kind.TRANSIENT_LOCAL);
        case DDS.DurabilityQosPolicyKind._TRANSIENT_DURABILITY_QOS:
            return new DurabilityImpl(env, Durability.Kind.TRANSIENT);
        case DDS.DurabilityQosPolicyKind._PERSISTENT_DURABILITY_QOS:
            return new DurabilityImpl(env, Durability.Kind.PERSISTENT);
        default:
            throw new IllegalArgumentExceptionImpl(env,
                    "Unknown Durability kind.");
        }
    }

    public static DDS.DurabilityQosPolicy convert(OsplServiceEnvironment env,
            Durability p) {
        switch (p.getKind()) {
        case VOLATILE:
            return new DDS.DurabilityQosPolicy(
                    DDS.DurabilityQosPolicyKind.VOLATILE_DURABILITY_QOS);
        case TRANSIENT_LOCAL:
            return new DDS.DurabilityQosPolicy(
                    DDS.DurabilityQosPolicyKind.TRANSIENT_LOCAL_DURABILITY_QOS);
        case TRANSIENT:
            return new DDS.DurabilityQosPolicy(
                    DDS.DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS);
        case PERSISTENT:
            return new DDS.DurabilityQosPolicy(
                    DDS.DurabilityQosPolicyKind.PERSISTENT_DURABILITY_QOS);
        default:
            throw new IllegalArgumentExceptionImpl(env,
                    "Unknown Durability kind.");
        }
    }

    public static DurabilityService convert(OsplServiceEnvironment env,
            DDS.DurabilityServiceQosPolicy old) {
        History.Kind kind;

        switch (old.history_kind.value()) {
        case DDS.HistoryQosPolicyKind._KEEP_ALL_HISTORY_QOS:
            kind = History.Kind.KEEP_ALL;
            break;
        case DDS.HistoryQosPolicyKind._KEEP_LAST_HISTORY_QOS:
            kind = History.Kind.KEEP_LAST;
            break;
        default:
            throw new IllegalArgumentExceptionImpl(env, "Unknown History kind.");
        }

        return new DurabilityServiceImpl(env, convert(env,
                old.service_cleanup_delay), kind, old.history_depth,
                old.max_samples, old.max_instances,
                old.max_samples_per_instance);
    }

    public static DDS.DurabilityServiceQosPolicy convert(
            OsplServiceEnvironment env, DurabilityService p) {
        DDS.HistoryQosPolicyKind kind;

        switch (p.getHistoryKind()) {
        case KEEP_ALL:
            kind = DDS.HistoryQosPolicyKind.KEEP_ALL_HISTORY_QOS;
            break;
        case KEEP_LAST:
            kind = DDS.HistoryQosPolicyKind.KEEP_LAST_HISTORY_QOS;
            break;
        default:
            throw new IllegalArgumentExceptionImpl(
                    (OsplServiceEnvironment) p.getEnvironment(),
                    "Unknown History kind.");
        }
        return new DDS.DurabilityServiceQosPolicy(convert(env,
                p.getServiceCleanupDelay()), kind, p.getHistoryDepth(),
                p.getMaxSamples(), p.getMaxInstances(),
                p.getMaxSamplesPerInstance());
    }

    public static History convert(OsplServiceEnvironment env,
            DDS.HistoryQosPolicy old) {
        History.Kind kind;

        switch (old.kind.value()) {
        case DDS.HistoryQosPolicyKind._KEEP_ALL_HISTORY_QOS:
            kind = History.Kind.KEEP_ALL;
            break;
        case DDS.HistoryQosPolicyKind._KEEP_LAST_HISTORY_QOS:
            kind = History.Kind.KEEP_LAST;
            break;
        default:
            throw new IllegalArgumentExceptionImpl(env, "Unknown History kind.");
        }
        return new HistoryImpl(env, kind, old.depth);
    }

    public static DDS.HistoryQosPolicy convert(OsplServiceEnvironment env,
            History p) {
        DDS.HistoryQosPolicyKind kind;

        switch (p.getKind()) {
        case KEEP_ALL:
            kind = DDS.HistoryQosPolicyKind.KEEP_ALL_HISTORY_QOS;
            break;
        case KEEP_LAST:
            kind = DDS.HistoryQosPolicyKind.KEEP_LAST_HISTORY_QOS;
            break;
        default:
            throw new IllegalArgumentExceptionImpl(env, "Unknown History kind.");
        }
        return new DDS.HistoryQosPolicy(kind, p.getDepth());
    }

    public static LatencyBudget convert(OsplServiceEnvironment env,
            DDS.LatencyBudgetQosPolicy old) {
        return new LatencyBudgetImpl(env, convert(env, old.duration));
    }

    public static DDS.LatencyBudgetQosPolicy convert(
            OsplServiceEnvironment env, LatencyBudget p) {
        return new DDS.LatencyBudgetQosPolicy(convert(env, p.getDuration()));
    }

    public static Lifespan convert(OsplServiceEnvironment env,
            DDS.LifespanQosPolicy old) {
        return new LifespanImpl(env, convert(env, old.duration));
    }

    public static DDS.LifespanQosPolicy convert(OsplServiceEnvironment env,
            Lifespan p) {
        return new DDS.LifespanQosPolicy(convert(env, p.getDuration()));
    }

    public static ReaderLifespan convert(OsplServiceEnvironment env,
            DDS.ReaderLifespanQosPolicy old) {
        if (old.use_lifespan == false) {
            return null;
        }
        return new ReaderLifespanImpl(env, convert(env, old.duration));
    }

    public static DDS.ReaderLifespanQosPolicy convert(
            OsplServiceEnvironment env, ReaderLifespan p) {
        if (p == null) {
            return new DDS.ReaderLifespanQosPolicy(false,
                    DDS.DURATION_ZERO.value);
        }
        return new DDS.ReaderLifespanQosPolicy(true, convert(env,
                p.getDuration()));
    }

    public static Share convert(OsplServiceEnvironment env,
            DDS.ShareQosPolicy old) {
        if (old.enable == false) {
            return null;
        }
        return new ShareImpl(env, old.name);
    }

    public static DDS.ShareQosPolicy convert(OsplServiceEnvironment env, Share p) {
        if (p == null) {
            return new DDS.ShareQosPolicy("", false);
        }
        return new DDS.ShareQosPolicy(p.getName(), true);
    }

    public static SubscriptionKeys convert(OsplServiceEnvironment env,
            DDS.SubscriptionKeyQosPolicy old) {
        if (old.use_key_list == false) {
            return null;
        }
        return new SubscriptionKeysImpl(env, old.key_list);
    }

    public static DDS.SubscriptionKeyQosPolicy convert(
            OsplServiceEnvironment env, SubscriptionKeys p) {
        if (p == null) {
            return new DDS.SubscriptionKeyQosPolicy(false, new String[0]);
        }
        return new DDS.SubscriptionKeyQosPolicy(true, p.getKey().toArray(
                new String[p.getKey().size()]));
    }

    public static TimeBasedFilter convert(OsplServiceEnvironment env,
            DDS.TimeBasedFilterQosPolicy old) {
        return new TimeBasedFilterImpl(env, Utilities.convert(env,
                old.minimum_separation));
    }

    public static DDS.TimeBasedFilterQosPolicy convert(
            OsplServiceEnvironment env, TimeBasedFilter p) {
        return new DDS.TimeBasedFilterQosPolicy(Utilities.convert(env,
                p.getMinimumSeparation()));
    }

    public static Liveliness convert(OsplServiceEnvironment env,
            DDS.LivelinessQosPolicy old) {
        Liveliness.Kind kind;

        switch (old.kind.value()) {
        case DDS.LivelinessQosPolicyKind._AUTOMATIC_LIVELINESS_QOS:
            kind = Liveliness.Kind.AUTOMATIC;
            break;
        case DDS.LivelinessQosPolicyKind._MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
            kind = Liveliness.Kind.MANUAL_BY_PARTICIPANT;
            break;
        case DDS.LivelinessQosPolicyKind._MANUAL_BY_TOPIC_LIVELINESS_QOS:
            kind = Liveliness.Kind.MANUAL_BY_TOPIC;
            break;
        default:
            throw new IllegalArgumentExceptionImpl(env,
                    "Unknown Liveliness kind.");
        }
        return new LivelinessImpl(env, kind, convert(env, old.lease_duration));
    }

    public static DDS.LivelinessQosPolicy convert(OsplServiceEnvironment env,
            Liveliness p) {
        DDS.LivelinessQosPolicyKind kind;

        switch (p.getKind()) {
        case AUTOMATIC:
            kind = DDS.LivelinessQosPolicyKind.AUTOMATIC_LIVELINESS_QOS;
            break;
        case MANUAL_BY_PARTICIPANT:
            kind = DDS.LivelinessQosPolicyKind.MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
            break;
        case MANUAL_BY_TOPIC:
            kind = DDS.LivelinessQosPolicyKind.MANUAL_BY_TOPIC_LIVELINESS_QOS;
            break;
        default:
            throw new IllegalArgumentExceptionImpl(env,
                    "Unknown Liveliness kind.");
        }
        return new DDS.LivelinessQosPolicy(kind, convert(env,
                p.getLeaseDuration()));
    }

    public static Ownership convert(OsplServiceEnvironment env,
            DDS.OwnershipQosPolicy old) {
        Ownership.Kind kind;

        switch (old.kind.value()) {
        case DDS.OwnershipQosPolicyKind._SHARED_OWNERSHIP_QOS:
            kind = Ownership.Kind.SHARED;
            break;
        case DDS.OwnershipQosPolicyKind._EXCLUSIVE_OWNERSHIP_QOS:
            kind = Ownership.Kind.EXCLUSIVE;
            break;
        default:
            throw new IllegalArgumentExceptionImpl(env,
                    "Unknown Ownership kind.");
        }
        return new OwnershipImpl(env, kind);
    }

    public static DDS.OwnershipQosPolicy convert(OsplServiceEnvironment env,
            Ownership p) {
        DDS.OwnershipQosPolicyKind kind;

        switch (p.getKind()) {
        case SHARED:
            kind = DDS.OwnershipQosPolicyKind.SHARED_OWNERSHIP_QOS;
            break;
        case EXCLUSIVE:
            kind = DDS.OwnershipQosPolicyKind.EXCLUSIVE_OWNERSHIP_QOS;
            break;
        default:
            throw new IllegalArgumentExceptionImpl(env,
                    "Unknown Ownership kind.");
        }
        return new DDS.OwnershipQosPolicy(kind);
    }

    public static Reliability convert(OsplServiceEnvironment env,
            DDS.ReliabilityQosPolicy old) {
        Reliability.Kind kind;

        switch (old.kind.value()) {
        case DDS.ReliabilityQosPolicyKind._BEST_EFFORT_RELIABILITY_QOS:
            kind = Reliability.Kind.BEST_EFFORT;
            break;
        case DDS.ReliabilityQosPolicyKind._RELIABLE_RELIABILITY_QOS:
            kind = Reliability.Kind.RELIABLE;
            break;
        default:
            throw new IllegalArgumentExceptionImpl(env,
                    "Unknown Reliability kind.");
        }
        return new ReliabilityImpl(env, kind, convert(env,
                old.max_blocking_time), old.synchronous);
    }

    public static DDS.ReliabilityQosPolicy convert(OsplServiceEnvironment env,
            Reliability p) {
        org.opensplice.dds.core.policy.Reliability r;
        boolean synchronous;
        DDS.ReliabilityQosPolicyKind kind;

        try {
            r = (org.opensplice.dds.core.policy.Reliability) p;
            synchronous = r.isSynchronous();
        } catch (ClassCastException e) {
            synchronous = false;
        }
        switch (p.getKind()) {
        case RELIABLE:
            kind = DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
            break;
        case BEST_EFFORT:
            kind = DDS.ReliabilityQosPolicyKind.BEST_EFFORT_RELIABILITY_QOS;
            break;
        default:
            throw new IllegalArgumentExceptionImpl(env,
                    "Unknown Reliability kind.");
        }
        return new DDS.ReliabilityQosPolicy(kind, convert(env,
                p.getMaxBlockingTime()), synchronous);
    }

    public static ResourceLimits convert(OsplServiceEnvironment env,
            DDS.ResourceLimitsQosPolicy old) {
        return new ResourceLimitsImpl(env, old.max_samples, old.max_instances,
                old.max_samples_per_instance);
    }

    public static DDS.ResourceLimitsQosPolicy convert(
            OsplServiceEnvironment env, ResourceLimits p) {
        return new DDS.ResourceLimitsQosPolicy(p.getMaxSamples(),
                p.getMaxInstances(), p.getMaxSamplesPerInstance());
    }

    public static TopicData convert(OsplServiceEnvironment env,
            DDS.TopicDataQosPolicy old) {
        return new TopicDataImpl(env, old.value);
    }

    public static DDS.TopicDataQosPolicy convert(OsplServiceEnvironment env,
            TopicData p) {
        return new DDS.TopicDataQosPolicy(p.getValue());
    }

    public static TransportPriority convert(OsplServiceEnvironment env,
            DDS.TransportPriorityQosPolicy old) {
        return new TransportPriorityImpl(env, old.value);
    }

    public static DDS.TransportPriorityQosPolicy convert(
            OsplServiceEnvironment env, TransportPriority p) {
        return new DDS.TransportPriorityQosPolicy(p.getValue());
    }

    public static GroupData convert(OsplServiceEnvironment env,
            DDS.GroupDataQosPolicy old) {
        return new GroupDataImpl(env, old.value);
    }

    public static DDS.GroupDataQosPolicy convert(OsplServiceEnvironment env,
            GroupData p) {
        return new DDS.GroupDataQosPolicy(p.getValue());
    }

    public static Partition convert(OsplServiceEnvironment env,
            DDS.PartitionQosPolicy old) {
        return new PartitionImpl(env, old.name);
    }

    public static DDS.PartitionQosPolicy convert(OsplServiceEnvironment env,
            Partition p) {
        Set<String> partitions = p.getName();
        String[] pArray = p.getName().toArray(new String[partitions.size()]);

        return new DDS.PartitionQosPolicy(pArray);
    }

    public static Presentation convert(OsplServiceEnvironment env,
            DDS.PresentationQosPolicy old) {
        AccessScopeKind kind;

        switch (old.access_scope.value()) {
        case DDS.PresentationQosPolicyAccessScopeKind._INSTANCE_PRESENTATION_QOS:
            kind = AccessScopeKind.INSTANCE;
            break;
        case DDS.PresentationQosPolicyAccessScopeKind._TOPIC_PRESENTATION_QOS:
            kind = AccessScopeKind.TOPIC;
            break;
        case DDS.PresentationQosPolicyAccessScopeKind._GROUP_PRESENTATION_QOS:
            kind = AccessScopeKind.GROUP;
            break;
        default:
            throw new IllegalArgumentExceptionImpl(env,
                    "Unknown Presentation AccessScope kind.");
        }
        return new PresentationImpl(env, kind, old.coherent_access,
                old.ordered_access);
    }

    public static DDS.PresentationQosPolicy convert(OsplServiceEnvironment env,
            Presentation p) {
        DDS.PresentationQosPolicyAccessScopeKind kind;

        switch (p.getAccessScope()) {
        case INSTANCE:
            kind = DDS.PresentationQosPolicyAccessScopeKind.INSTANCE_PRESENTATION_QOS;
            break;
        case TOPIC:
            kind = DDS.PresentationQosPolicyAccessScopeKind.TOPIC_PRESENTATION_QOS;
            break;
        case GROUP:
            kind = DDS.PresentationQosPolicyAccessScopeKind.GROUP_PRESENTATION_QOS;
            break;
        default:
            throw new IllegalArgumentExceptionImpl(env,
                    "Unknown Presentation AccessScope kind.");
        }
        return new DDS.PresentationQosPolicy(kind, p.isCoherentAccess(),
                p.isOrderedAccess());
    }

    public static OwnershipStrength convert(OsplServiceEnvironment env,
            DDS.OwnershipStrengthQosPolicy old) {
        return new OwnershipStrengthImpl(env, old.value);
    }

    public static DDS.OwnershipStrengthQosPolicy convert(
            OsplServiceEnvironment env, OwnershipStrength p) {
        return new DDS.OwnershipStrengthQosPolicy(p.getValue());
    }

    public static WriterDataLifecycle convert(OsplServiceEnvironment env,
            DDS.WriterDataLifecycleQosPolicy old) {
        return new WriterDataLifecycleImpl(env,
                old.autodispose_unregistered_instances, convert(env,
                        old.autopurge_suspended_samples_delay), convert(env,
                        old.autounregister_instance_delay));
    }

    public static DDS.WriterDataLifecycleQosPolicy convert(
            OsplServiceEnvironment env, WriterDataLifecycle p) {
        Duration autoPurgeSuspendedSamplesDelay, autoUnregisterInstanceDelay;
        org.opensplice.dds.core.policy.WriterDataLifecycle w;

        try {
            w = (org.opensplice.dds.core.policy.WriterDataLifecycle) p;
            autoPurgeSuspendedSamplesDelay = w
                    .getAutoPurgeSuspendedSamplesDelay();
            autoUnregisterInstanceDelay = w.getAutoUnregisterInstanceDelay();
        } catch (ClassCastException e) {
            autoPurgeSuspendedSamplesDelay = p.getEnvironment().getSPI()
                    .infiniteDuration();
            autoUnregisterInstanceDelay = p.getEnvironment().getSPI()
                    .infiniteDuration();
        }

        return new DDS.WriterDataLifecycleQosPolicy(
                p.isAutDisposeUnregisteredInstances(), convert(env,
                        autoPurgeSuspendedSamplesDelay), convert(env,
                        autoUnregisterInstanceDelay));
    }

    public static ReaderDataLifecycle convert(OsplServiceEnvironment env,
            DDS.ReaderDataLifecycleQosPolicy old) {
        org.opensplice.dds.core.policy.ReaderDataLifecycle.Kind kind;

        switch (old.invalid_sample_visibility.kind.value()) {
        case DDS.InvalidSampleVisibilityQosPolicyKind._ALL_INVALID_SAMPLES:
            kind = org.opensplice.dds.core.policy.ReaderDataLifecycle.Kind.ALL;
            break;
        case DDS.InvalidSampleVisibilityQosPolicyKind._MINIMUM_INVALID_SAMPLES:
            kind = org.opensplice.dds.core.policy.ReaderDataLifecycle.Kind.MINIMUM;
            break;
        case DDS.InvalidSampleVisibilityQosPolicyKind._NO_INVALID_SAMPLES:
            kind = org.opensplice.dds.core.policy.ReaderDataLifecycle.Kind.NONE;
            break;
        default:
            throw new IllegalArgumentExceptionImpl(env,
                    "Unknown ReaderDataLifecycle InvalidSampleVisibilityKind.");
        }
        return new ReaderDataLifecycleImpl(env, Utilities.convert(env,
                old.autopurge_nowriter_samples_delay), Utilities.convert(env,
                old.autopurge_disposed_samples_delay),
                old.autopurge_dispose_all, kind);
    }

    public static DDS.ReaderDataLifecycleQosPolicy convert(
            OsplServiceEnvironment env,
            org.omg.dds.core.policy.ReaderDataLifecycle p) {
        org.opensplice.dds.core.policy.ReaderDataLifecycle r;
        boolean autoPurgeDisposeAll;
        DDS.InvalidSampleVisibilityQosPolicy invalidSampleVisibility;

        try {
            r = (org.opensplice.dds.core.policy.ReaderDataLifecycle) p;
            autoPurgeDisposeAll = r.getAutoPurgeDisposeAll();
            switch (r.getInvalidSampleInvisibility()) {
            case ALL:
                invalidSampleVisibility = new DDS.InvalidSampleVisibilityQosPolicy(
                        DDS.InvalidSampleVisibilityQosPolicyKind.ALL_INVALID_SAMPLES);
                break;
            case MINIMUM:
                invalidSampleVisibility = new DDS.InvalidSampleVisibilityQosPolicy(
                        DDS.InvalidSampleVisibilityQosPolicyKind.MINIMUM_INVALID_SAMPLES);
                break;
            case NONE:
                invalidSampleVisibility = new DDS.InvalidSampleVisibilityQosPolicy(
                        DDS.InvalidSampleVisibilityQosPolicyKind.NO_INVALID_SAMPLES);
                break;
            default:
                throw new IllegalArgumentExceptionImpl(env,
                        "Unknown ReaderDataLifecycle InvalidSampleVisibilityKind.");
            }

        } catch (ClassCastException e) {
            autoPurgeDisposeAll = false;
            invalidSampleVisibility = new DDS.InvalidSampleVisibilityQosPolicy(
                    DDS.InvalidSampleVisibilityQosPolicyKind.MINIMUM_INVALID_SAMPLES);
        }

        return new DDS.ReaderDataLifecycleQosPolicy(convert(env,
                p.getAutoPurgeNoWriterSamplesDelay()), convert(env,
                p.getAutoPurgeDisposedSamplesDelay()), autoPurgeDisposeAll,
                true, invalidSampleVisibility);
    }

    public static Class<? extends QosPolicy> convert(
            OsplServiceEnvironment env, int policyId) {
        Class<? extends QosPolicy> policy;

        switch (policyId) {
        case DDS.DEADLINE_QOS_POLICY_ID.value:
            policy = Deadline.class;
            break;
        case DDS.DESTINATIONORDER_QOS_POLICY_ID.value:
            policy = DestinationOrder.class;
            break;
        case DDS.DURABILITY_QOS_POLICY_ID.value:
            policy = Durability.class;
            break;
        case DDS.DURABILITYSERVICE_QOS_POLICY_ID.value:
            policy = DurabilityService.class;
            break;
        case DDS.ENTITYFACTORY_QOS_POLICY_ID.value:
            policy = EntityFactory.class;
            break;
        case DDS.GROUPDATA_QOS_POLICY_ID.value:
            policy = GroupData.class;
            break;
        case DDS.HISTORY_QOS_POLICY_ID.value:
            policy = History.class;
            break;
        case DDS.LATENCYBUDGET_QOS_POLICY_ID.value:
            policy = LatencyBudget.class;
            break;
        case DDS.LIFESPAN_QOS_POLICY_ID.value:
            policy = Lifespan.class;
            break;
        case DDS.LIVELINESS_QOS_POLICY_ID.value:
            policy = Liveliness.class;
            break;
        case DDS.OWNERSHIP_QOS_POLICY_ID.value:
            policy = Ownership.class;
            break;
        case DDS.OWNERSHIPSTRENGTH_QOS_POLICY_ID.value:
            policy = OwnershipStrength.class;
            break;
        case DDS.PARTITION_QOS_POLICY_ID.value:
            policy = Partition.class;
            break;
        case DDS.PRESENTATION_QOS_POLICY_ID.value:
            policy = Presentation.class;
            break;
        case DDS.READERDATALIFECYCLE_QOS_POLICY_ID.value:
            policy = ReaderDataLifecycle.class;
            break;
        case DDS.RELIABILITY_QOS_POLICY_ID.value:
            policy = Reliability.class;
            break;
        case DDS.RESOURCELIMITS_QOS_POLICY_ID.value:
            policy = ResourceLimits.class;
            break;
        case DDS.SCHEDULING_QOS_POLICY_ID.value:
            policy = Scheduling.class;
            break;
        case DDS.TIMEBASEDFILTER_QOS_POLICY_ID.value:
            policy = TimeBasedFilter.class;
            break;
        case DDS.TOPICDATA_QOS_POLICY_ID.value:
            policy = TopicData.class;
            break;
        case DDS.TRANSPORTPRIORITY_QOS_POLICY_ID.value:
            policy = TransportPriority.class;
            break;
        case DDS.USERDATA_QOS_POLICY_ID.value:
            policy = UserData.class;
            break;
        case DDS.WRITERDATALIFECYCLE_QOS_POLICY_ID.value:
            policy = WriterDataLifecycle.class;
            break;
        case 23: /* TODO: Add SUBSCRIPTIONKEY_QOS_POLICY_ID to classic Java PSM */
            policy = SubscriptionKeys.class;
            break;
        case 24: /* TODO: Add VIEWKEY_QOS_POLICY_ID to classic Java PSM */
            policy = ViewKeys.class;
            break;
        case 25:/* TODO: Add READERLIFESPAN_QOS_POLICY_ID to classic Java PSM */
            policy = ReaderLifespan.class;
            break;
        case 26:/* TODO: Add SHARE_QOS_POLICY_ID to classic Java PSM */
            policy = Share.class;
            break;
        case DDS.INVALID_QOS_POLICY_ID.value:
            policy = null;
            break;
        default:
            throw new IllegalArgumentExceptionImpl(env,
                    "Found unknown QoSPolicy id: " + policyId);
        }
        return policy;
    }
}
