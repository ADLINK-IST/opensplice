package org.opensplice.dds.dcps.policy;

/**@brief This policy expresses if data should outlive their writing time.
 * 
 * - Concerns:    DataWriter/DataReader/Topic
 * - RxO:         Yes
 * - Changable:   No
 */
public class DurabilityQosPolicy {
    private DurabilityQosKind kind;
    
    public DurabilityQosPolicy(DurabilityQosKind _kind){
        kind = _kind;
    }
}

