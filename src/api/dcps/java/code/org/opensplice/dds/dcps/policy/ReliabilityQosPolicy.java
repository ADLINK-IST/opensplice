package org.opensplice.dds.dcps.policy;

/**@brief Indicates the level of reliability offered by the Service.
 * 
 * - Concerns:    DataWriter/DataReader/Topic
 * - RxO:         Yes
 * - Changable:   No
 */
public class ReliabilityQosPolicy {
    private ReliabilityQosKind kind;
    
    /**@brief Creates new ReliabilityQosPolicy
     * 
     * @param _kind The reliability kind.
     */
    public ReliabilityQosPolicy(ReliabilityQosKind _kind){
        kind = _kind;
    }
}

