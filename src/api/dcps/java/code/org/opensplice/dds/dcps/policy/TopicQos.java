package org.opensplice.dds.dcps.policy;

/**@brief The Qos policies for a Topic.
 */
public class TopicQos extends QosPolicy {
    private DurabilityQosPolicy durability;
    private DeadlineQosPolicy deadline;
    private LatencyBudgetQosPolicy latency;
    private OwnershipQosPolicy ownership;
    private LivelinessQosPolicy liveliness;
    private ReliabilityQosPolicy reliability;
    private DestinationOrderQosPolicy orderby;
    private HistoryQosPolicy history;
    private ResourceLimitsQosPolicy resource;
}

