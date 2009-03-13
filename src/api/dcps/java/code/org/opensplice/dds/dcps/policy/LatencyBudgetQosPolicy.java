package org.opensplice.dds.dcps.policy;

/**@brief Provides a hint as to the maximum acceptable delay from the time the data is written
 * to the time the data is received by its subscribing applications. This policy is a hint
 * and not something that will be monitored or enforced.
 * 
 * - Concerns:    DataWriter/DataReader/Topic
 * - RxO:         Yes
 * - Changable:   Yes
 */
public class LatencyBudgetQosPolicy {
    private Duration duration;
    
    public LatencyBudgetQosPolicy(Duration _duration){
        duration = _duration;
    }
}

