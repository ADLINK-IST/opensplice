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
package org.opensplice.dds.core.status;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import org.omg.dds.core.policy.Deadline;
import org.omg.dds.core.policy.DestinationOrder;
import org.omg.dds.core.policy.Durability;
import org.omg.dds.core.policy.EntityFactory;
import org.omg.dds.core.policy.GroupData;
import org.omg.dds.core.policy.History;
import org.omg.dds.core.policy.LatencyBudget;
import org.omg.dds.core.policy.Lifespan;
import org.omg.dds.core.policy.Liveliness;
import org.omg.dds.core.policy.Ownership;
import org.omg.dds.core.policy.Partition;
import org.omg.dds.core.policy.Presentation;
import org.omg.dds.core.policy.QosPolicy;
import org.omg.dds.core.policy.QosPolicyCount;
import org.omg.dds.core.policy.ReaderDataLifecycle;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.core.policy.ResourceLimits;
import org.omg.dds.core.policy.TimeBasedFilter;
import org.omg.dds.core.policy.TopicData;
import org.omg.dds.core.policy.TransportPriority;
import org.omg.dds.core.policy.UserData;
import org.omg.dds.core.policy.WriterDataLifecycle;
import org.omg.dds.core.status.DataAvailableStatus;
import org.omg.dds.core.status.DataOnReadersStatus;
import org.omg.dds.core.status.InconsistentTopicStatus;
import org.omg.dds.core.status.LivelinessChangedStatus;
import org.omg.dds.core.status.LivelinessLostStatus;
import org.omg.dds.core.status.OfferedDeadlineMissedStatus;
import org.omg.dds.core.status.OfferedIncompatibleQosStatus;
import org.omg.dds.core.status.PublicationMatchedStatus;
import org.omg.dds.core.status.RequestedDeadlineMissedStatus;
import org.omg.dds.core.status.RequestedIncompatibleQosStatus;
import org.omg.dds.core.status.SampleLostStatus;
import org.omg.dds.core.status.SampleRejectedStatus;
import org.omg.dds.core.status.SampleRejectedStatus.Kind;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.InstanceHandleImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.Utilities;
import org.opensplice.dds.core.policy.PolicyConverter;
import org.opensplice.dds.core.policy.QosPolicyCountImpl;
import org.opensplice.dds.core.policy.Scheduling;
import org.omg.dds.core.status.Status;
import org.omg.dds.core.status.SubscriptionMatchedStatus;

import DDS.SampleRejectedStatusKind;

public class StatusConverter {
    private static boolean stateTest(int state, int mask) {
        return (((state) & (mask)) == mask) ? true : false;
    }

    public static Set<Class<? extends Status>> convertMask(
            OsplServiceEnvironment environment, int state) {
        HashSet<Class<? extends Status>> statuses = new HashSet<Class<? extends Status>>();

        if (state == DDS.STATUS_MASK_ANY_V1_2.value) {
            statuses.add(AllDataDisposedStatus.class);
            statuses.add(DataAvailableStatus.class);
            statuses.add(DataOnReadersStatus.class);
            statuses.add(InconsistentTopicStatus.class);
            statuses.add(LivelinessChangedStatus.class);
            statuses.add(LivelinessLostStatus.class);
            statuses.add(OfferedDeadlineMissedStatus.class);
            statuses.add(OfferedIncompatibleQosStatus.class);
            statuses.add(PublicationMatchedStatus.class);
            statuses.add(RequestedDeadlineMissedStatus.class);
            statuses.add(RequestedIncompatibleQosStatus.class);
            statuses.add(SampleLostStatus.class);
            statuses.add(SampleRejectedStatus.class);
            statuses.add(SubscriptionMatchedStatus.class);
        } else {
            if (stateTest(state, DDS.ALL_DATA_DISPOSED_TOPIC_STATUS.value)) {
                statuses.add(AllDataDisposedStatus.class);
            }
            if (stateTest(state, DDS.DATA_AVAILABLE_STATUS.value)) {
                statuses.add(DataAvailableStatus.class);
            }
            if (stateTest(state, DDS.DATA_ON_READERS_STATUS.value)) {
                statuses.add(DataOnReadersStatus.class);
            }
            if (stateTest(state, DDS.INCONSISTENT_TOPIC_STATUS.value)) {
                statuses.add(InconsistentTopicStatus.class);
            }
            if (stateTest(state, DDS.LIVELINESS_CHANGED_STATUS.value)) {
                statuses.add(LivelinessChangedStatus.class);
            }
            if (stateTest(state, DDS.LIVELINESS_LOST_STATUS.value)) {
                statuses.add(LivelinessLostStatus.class);
            }
            if (stateTest(state, DDS.OFFERED_DEADLINE_MISSED_STATUS.value)) {
                statuses.add(OfferedDeadlineMissedStatus.class);
            }
            if (stateTest(state, DDS.OFFERED_INCOMPATIBLE_QOS_STATUS.value)) {
                statuses.add(OfferedIncompatibleQosStatus.class);
            }
            if (stateTest(state, DDS.PUBLICATION_MATCHED_STATUS.value)) {
                statuses.add(PublicationMatchedStatus.class);
            }
            if (stateTest(state, DDS.REQUESTED_DEADLINE_MISSED_STATUS.value)) {
                statuses.add(RequestedDeadlineMissedStatus.class);
            }
            if (stateTest(state, DDS.REQUESTED_INCOMPATIBLE_QOS_STATUS.value)) {
                statuses.add(RequestedIncompatibleQosStatus.class);
            }
            if (stateTest(state, DDS.SAMPLE_LOST_STATUS.value)) {
                statuses.add(SampleLostStatus.class);
            }
            if (stateTest(state, DDS.SAMPLE_REJECTED_STATUS.value)) {
                statuses.add(SampleRejectedStatus.class);
            }
            if (stateTest(state, DDS.SUBSCRIPTION_MATCHED_STATUS.value)) {
                statuses.add(SubscriptionMatchedStatus.class);
            }
        }
        return statuses;
    }

    public static int convertMask(OsplServiceEnvironment env,
            Class<? extends Status>... statuses) {
        return StatusConverter.convertMask(env, Arrays.asList(statuses));
    }

    public static int getAnyMask() {
        return DDS.STATUS_MASK_ANY_V1_2.value;
    }

    public static int convertMask(OsplServiceEnvironment env,
            Collection<Class<? extends Status>> statuses) {
        int mask;

        if (statuses == null) {
            return DDS.STATUS_MASK_ANY_V1_2.value;
        } else if (statuses.size() == 0) {
            return DDS.STATUS_MASK_NONE.value;
        } else if (statuses.size() == 1 && statuses.iterator().next() == null) {
            return DDS.STATUS_MASK_ANY_V1_2.value;
        }
        mask = DDS.STATUS_MASK_NONE.value;

        for (Class<? extends Status> clz : statuses) {
            if (clz == null) {
                throw new IllegalArgumentExceptionImpl(env,
                        "Passed illegal <null> status.");
            } else if (clz.equals(DataAvailableStatus.class)) {
                mask |= DDS.DATA_AVAILABLE_STATUS.value;
            } else if (clz.equals(InconsistentTopicStatus.class)) {
                mask |= DDS.INCONSISTENT_TOPIC_STATUS.value;
            } else if (clz.equals(OfferedDeadlineMissedStatus.class)) {
                mask |= DDS.OFFERED_DEADLINE_MISSED_STATUS.value;
            } else if (clz.equals(RequestedDeadlineMissedStatus.class)) {
                mask |= DDS.REQUESTED_DEADLINE_MISSED_STATUS.value;
            } else if (clz.equals(OfferedIncompatibleQosStatus.class)) {
                mask |= DDS.OFFERED_INCOMPATIBLE_QOS_STATUS.value;
            } else if (clz.equals(RequestedIncompatibleQosStatus.class)) {
                mask |= DDS.REQUESTED_INCOMPATIBLE_QOS_STATUS.value;
            } else if (clz.equals(SampleLostStatus.class)) {
                mask |= DDS.SAMPLE_LOST_STATUS.value;
            } else if (clz.equals(SampleRejectedStatus.class)) {
                mask |= DDS.SAMPLE_REJECTED_STATUS.value;
            } else if (clz.equals(DataOnReadersStatus.class)) {
                mask |= DDS.DATA_ON_READERS_STATUS.value;
            } else if (clz.equals(LivelinessLostStatus.class)) {
                mask |= DDS.LIVELINESS_LOST_STATUS.value;
            } else if (clz.equals(LivelinessChangedStatus.class)) {
                mask |= DDS.LIVELINESS_CHANGED_STATUS.value;
            } else if (clz.equals(PublicationMatchedStatus.class)) {
                mask |= DDS.PUBLICATION_MATCHED_STATUS.value;
            } else if (clz.equals(SubscriptionMatchedStatus.class)) {
                mask |= DDS.SUBSCRIPTION_MATCHED_STATUS.value;
            } else if (clz.equals(AllDataDisposedStatus.class)) {
                mask |= DDS.ALL_DATA_DISPOSED_TOPIC_STATUS.value;
            } else if (clz.equals(Status.class)) {
                throw new IllegalArgumentExceptionImpl(env,
                        "Provided class does not extend from the org.omg.dds.core.status.Status class.");
            } else {
                throw new IllegalArgumentExceptionImpl(env,
                        "Found illegal Class<? extends Status>: "
                                + clz.getName());
            }
        }
        return mask;
    }

    public static QosPolicyCount[] convert(OsplServiceEnvironment env,
            DDS.QosPolicyCount[] old) {
        Class<? extends QosPolicy> policyClass;
        ArrayList<QosPolicyCountImpl> policies = new ArrayList<QosPolicyCountImpl>();

        for (int i = 0; i < old.length; i++) {
            policyClass = PolicyConverter.convert(env, old[i].policy_id);

            if (policyClass != null) {
                if (old[i].count != 0) {
                    policies.add(new QosPolicyCountImpl(env, policyClass,
                            old[i].count));
                }
            }
        }
        return policies.toArray(new QosPolicyCountImpl[policies.size()]);
    }

    public static int convert(OsplServiceEnvironment env,
            Class<? extends QosPolicy> policy) {
        int id;

        if (policy.equals(Deadline.class)) {
            id = DDS.DEADLINE_QOS_POLICY_ID.value;
        } else if (policy.equals(DestinationOrder.class)) {
            id = DDS.DESTINATIONORDER_QOS_POLICY_ID.value;
        } else if (policy.equals(Durability.class)) {
            id = DDS.DURABILITYSERVICE_QOS_POLICY_ID.value;
        } else if (policy.equals(EntityFactory.class)) {
            id = DDS.ENTITYFACTORY_QOS_POLICY_ID.value;
        } else if (policy.equals(GroupData.class)) {
            id = DDS.GROUPDATA_QOS_POLICY_ID.value;
        } else if (policy.equals(History.class)) {
            id = DDS.HISTORY_QOS_POLICY_ID.value;
        } else if (policy.equals(LatencyBudget.class)) {
            id = DDS.LATENCYBUDGET_QOS_POLICY_ID.value;
        } else if (policy.equals(Lifespan.class)) {
            id = DDS.LIFESPAN_QOS_POLICY_ID.value;
        } else if (policy.equals(Liveliness.class)) {
            id = DDS.LIVELINESS_QOS_POLICY_ID.value;
        } else if (policy.equals(Ownership.class)) {
            id = DDS.OWNERSHIP_QOS_POLICY_ID.value;
        } else if (policy.equals(Partition.class)) {
            id = DDS.PARTITION_QOS_POLICY_ID.value;
        } else if (policy.equals(Presentation.class)) {
            id = DDS.PRESENTATION_QOS_POLICY_ID.value;
        } else if (policy.equals(ReaderDataLifecycle.class)) {
            id = DDS.READERDATALIFECYCLE_QOS_POLICY_ID.value;
        } else if (policy.equals(Reliability.class)) {
            id = DDS.RELIABILITY_QOS_POLICY_ID.value;
        } else if (policy.equals(ResourceLimits.class)) {
            id = DDS.RESOURCELIMITS_QOS_POLICY_ID.value;
        } else if (policy.equals(Scheduling.class)) {
            id = DDS.SCHEDULING_QOS_POLICY_ID.value;
        } else if (policy.equals(TimeBasedFilter.class)) {
            id = DDS.TIMEBASEDFILTER_QOS_POLICY_ID.value;
        } else if (policy.equals(TopicData.class)) {
            id = DDS.TOPICDATA_QOS_POLICY_ID.value;
        } else if (policy.equals(TransportPriority.class)) {
            id = DDS.TRANSPORTPRIORITY_QOS_POLICY_ID.value;
        } else if (policy.equals(UserData.class)) {
            id = DDS.USERDATA_QOS_POLICY_ID.value;
        } else if (policy.equals(WriterDataLifecycle.class)) {
            id = DDS.WRITERDATALIFECYCLE_QOS_POLICY_ID.value;
        } else {
            throw new IllegalArgumentExceptionImpl(env,
                    "Found illegal QoSPolicy: " + policy.getName());
        }
        return id;
    }

    public static DDS.QosPolicyCount[] convert(OsplServiceEnvironment env,
            Set<QosPolicyCount> count) {
        DDS.QosPolicyCount[] old = new DDS.QosPolicyCount[count.size()];
        Iterator<QosPolicyCount> iter = count.iterator();
        QosPolicyCount current;

        for (int i = 0; i < count.size(); i++) {
            current = iter.next();
            old[i] = new DDS.QosPolicyCount(convert(env,
                    current.getPolicyClass()), current.getCount());
        }
        return old;
    }

    public static InconsistentTopicStatus convert(OsplServiceEnvironment env,
            DDS.InconsistentTopicStatus old) {
        return new InconsistentTopicStatusImpl(env, old.total_count,
                old.total_count_change);
    }

    public static DDS.InconsistentTopicStatus convert(
            OsplServiceEnvironment env, InconsistentTopicStatus status) {
        return new DDS.InconsistentTopicStatus(status.getTotalCount(),
                status.getTotalCountChange());
    }

    public static AllDataDisposedStatus convert(OsplServiceEnvironment env,
            DDS.AllDataDisposedTopicStatus old) {
        return new AllDataDisposedStatusImpl(env, old.total_count,
                old.total_count_change);
    }

    public static DDS.AllDataDisposedTopicStatus convert(
            OsplServiceEnvironment env, AllDataDisposedStatus status) {
        return new DDS.AllDataDisposedTopicStatus(status.getTotalCount(),
                status.getTotalCountChange());
    }

    public static LivelinessChangedStatus convert(OsplServiceEnvironment env,
            DDS.LivelinessChangedStatus old) {
        return new LivelinessChangedStatusImpl(env, old.alive_count,
                old.alive_count_change, old.not_alive_count,
                old.not_alive_count_change, new InstanceHandleImpl(env,
                        old.last_publication_handle));
    }

    public static DDS.LivelinessChangedStatus convert(
            OsplServiceEnvironment env, LivelinessChangedStatus status) {
        return new DDS.LivelinessChangedStatus(status.getAliveCount(),
                status.getAliveCountChange(), status.getNotAliveCount(),
                status.getNotAliveCountChange(), Utilities.convert(env,
                        status.getLastPublicationHandle()));
    }

    public static LivelinessLostStatus convert(OsplServiceEnvironment env,
            DDS.LivelinessLostStatus old) {
        return new LivelinessLostStatusImpl(env, old.total_count,
                old.total_count_change);
    }

    public static DDS.LivelinessLostStatus convert(OsplServiceEnvironment env,
            LivelinessLostStatus status) {
        return new DDS.LivelinessLostStatus(status.getTotalCount(),
                status.getTotalCountChange());
    }

    public static OfferedDeadlineMissedStatus convert(
            OsplServiceEnvironment env, DDS.OfferedDeadlineMissedStatus old) {
        return new OfferedDeadlineMissedStatusImpl(env, old.total_count,
                old.total_count_change, Utilities.convert(env,
                        old.last_instance_handle));
    }

    public static DDS.OfferedDeadlineMissedStatus convert(
            OsplServiceEnvironment env, OfferedDeadlineMissedStatus status) {
        return new DDS.OfferedDeadlineMissedStatus(status.getTotalCount(),
                status.getTotalCountChange(), Utilities.convert(env,
                        status.getLastInstanceHandle()));
    }

    public static OfferedIncompatibleQosStatus convert(
            OsplServiceEnvironment env, DDS.OfferedIncompatibleQosStatus old) {

        return new OfferedIncompatibleQosStatusImpl(env, old.total_count,
                old.total_count_change, PolicyConverter.convert(env,
                        old.last_policy_id), convert(env, old.policies));
    }

    public static DDS.OfferedIncompatibleQosStatus convert(
            OsplServiceEnvironment env, OfferedIncompatibleQosStatus status) {
        return new DDS.OfferedIncompatibleQosStatus(status.getTotalCount(),
                status.getTotalCountChange(), convert(env,
                        status.getLastPolicyClass()), convert(env,
                        status.getPolicies()));
    }

    public static PublicationMatchedStatus convert(OsplServiceEnvironment env,
            DDS.PublicationMatchedStatus old) {

        return new PublicationMatchedStatusImpl(env, old.total_count,
                old.total_count_change, old.current_count,
                old.current_count_change, Utilities.convert(env,
                        old.last_subscription_handle));
    }

    public static DDS.PublicationMatchedStatus convert(
            OsplServiceEnvironment env, PublicationMatchedStatus status) {
        return new DDS.PublicationMatchedStatus(status.getTotalCount(),
                status.getTotalCountChange(), status.getCurrentCount(),
                status.getCurrentCountChange(), Utilities.convert(env,
                        status.getLastSubscriptionHandle()));
    }

    public static RequestedDeadlineMissedStatus convert(
            OsplServiceEnvironment env, DDS.RequestedDeadlineMissedStatus old) {
        return new RequestedDeadlineMissedStatusImpl(env, old.total_count,
                old.total_count_change, Utilities.convert(env,
                        old.last_instance_handle));
    }

    public static DDS.RequestedDeadlineMissedStatus convert(
            OsplServiceEnvironment env, RequestedDeadlineMissedStatus status) {
        return new DDS.RequestedDeadlineMissedStatus(status.getTotalCount(),
                status.getTotalCountChange(), Utilities.convert(env,
                        status.getLastInstanceHandle()));
    }

    public static SampleRejectedStatus convert(OsplServiceEnvironment env,
            DDS.SampleRejectedStatus old) {
        SampleRejectedStatus.Kind kind;

        switch (old.last_reason.value()) {
        case SampleRejectedStatusKind._NOT_REJECTED:
            kind = Kind.NOT_REJECTED;
            break;
        case SampleRejectedStatusKind._REJECTED_BY_INSTANCES_LIMIT:
            kind = Kind.REJECTED_BY_INSTANCES_LIMIT;
            break;
        case SampleRejectedStatusKind._REJECTED_BY_SAMPLES_LIMIT:
            kind = Kind.REJECTED_BY_SAMPLES_LIMIT;
            break;
        case SampleRejectedStatusKind._REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT:
            kind = Kind.REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
            break;
        default:
            throw new IllegalArgumentExceptionImpl(env,
                    "Found illegal SampleRejectedStatus.Kind "
                            + old.last_reason.value());
        }
        return new SampleRejectedStatusImpl(env, old.total_count,
                old.total_count_change, kind, Utilities.convert(env,
                        old.last_instance_handle));
    }

    public static DDS.SampleRejectedStatus convert(OsplServiceEnvironment env,
            SampleRejectedStatus status) {
        DDS.SampleRejectedStatusKind kind;

        switch (status.getLastReason()) {
        case NOT_REJECTED:
            kind = DDS.SampleRejectedStatusKind.NOT_REJECTED;
            break;
        case REJECTED_BY_INSTANCES_LIMIT:
            kind = DDS.SampleRejectedStatusKind.REJECTED_BY_INSTANCES_LIMIT;
            break;
        case REJECTED_BY_SAMPLES_LIMIT:
            kind = DDS.SampleRejectedStatusKind.REJECTED_BY_SAMPLES_LIMIT;
            break;
        case REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT:
            kind = DDS.SampleRejectedStatusKind.REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
            break;
        default:
            throw new IllegalArgumentExceptionImpl(env,
                    "Found illegal SampleRejectedStatus.Kind");
        }
        return new DDS.SampleRejectedStatus(status.getTotalCount(),
                status.getTotalCountChange(), kind, Utilities.convert(env,
                        status.getLastInstanceHandle()));
    }

    public static RequestedIncompatibleQosStatus convert(
            OsplServiceEnvironment env, DDS.RequestedIncompatibleQosStatus old) {

        return new RequestedIncompatibleQosStatusImpl(env, old.total_count,
                old.total_count_change, PolicyConverter.convert(env,
                        old.last_policy_id), convert(env, old.policies));
    }

    public static DDS.RequestedIncompatibleQosStatus convert(
            OsplServiceEnvironment env, RequestedIncompatibleQosStatus status) {
        return new DDS.RequestedIncompatibleQosStatus(status.getTotalCount(),
                status.getTotalCountChange(), convert(env,
                        status.getLastPolicyClass()), convert(env,
                        status.getPolicies()));
    }

    public static SubscriptionMatchedStatus convert(OsplServiceEnvironment env,
            DDS.SubscriptionMatchedStatus old) {

        return new SubscriptionMatchedStatusImpl(env, old.total_count,
                old.total_count_change, old.current_count,
                old.current_count_change, Utilities.convert(env,
                        old.last_publication_handle));
    }

    public static DDS.SubscriptionMatchedStatus convert(
            OsplServiceEnvironment env, SubscriptionMatchedStatus status) {
        return new DDS.SubscriptionMatchedStatus(status.getTotalCount(),
                status.getTotalCountChange(), status.getCurrentCount(),
                status.getCurrentCountChange(), Utilities.convert(env,
                        status.getLastPublicationHandle()));
    }

    public static SampleLostStatus convert(OsplServiceEnvironment env,
            DDS.SampleLostStatus old) {
        return new SampleLostStatusImpl(env, old.total_count,
                old.total_count_change);
    }

    public static DDS.SampleLostStatus convert(SampleLostStatus status) {
        return new DDS.SampleLostStatus(status.getTotalCount(),
                status.getTotalCountChange());
    }
}
