/**@package org.opensplice.dds.dcps.policy
 * @brief Provides the functionality for Quality of Services.
 * 
 * A QoS is a set of characteristics that controls some aspect of the
 * behaviour of the DDS service. QoS is compromised of individual QoS
 * policies.
 */
package org.opensplice.dds.dcps.policy;

/**@brief The Qos policies for a DataReader.
 */
public class DataReaderQos extends QosPolicy {
    private UserDataQosPolicy userdata;
    private DurabilityQosPolicy durability;
    private DeadlineQosPolicy deadline;
    private LatencyBudgetQosPolicy latency;
    private LivelinessQosPolicy liveliness;
    private TimeBasedFilterQosPolicy pacing;
    private ReliabilityQosPolicy reliability;
    private DestinationOrderQosPolicy orderby;
    private HistoryQosPolicy history;
    private ResourceLimitsQosPolicy resource;
}
