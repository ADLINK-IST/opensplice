package org.opensplice.dds.dcps.policy;

/**@brief The Qos policies for a DataWriter.
 */
public class DataWriterQos extends QosPolicy {
    private UserDataQosPolicy userdata;
    private DurabilityQosPolicy durability;
    private DeadlineQosPolicy deadline;
    private LatencyBudgetQosPolicy latency;
    private OwnershipStrengthQosPolicy strength;
    private LivelinessQosPolicy liveliness;
    private ReliabilityQosPolicy reliability;
    private HistoryQosPolicy history;
    private ResourceLimitsQosPolicy resource;
}
    
