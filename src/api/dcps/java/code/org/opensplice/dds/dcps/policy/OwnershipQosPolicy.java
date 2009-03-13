package org.opensplice.dds.dcps.policy;

/**@brief Specifies whether it is allowed for multiple DataWriters to write the same instance
 * of the data and if so how these modifications should be arbitrated.
 * 
 * - Concerns:    Topic
 * - RxO:         Yes
 * - Changable:   No
 */
public class OwnershipQosPolicy {
    private OwnershipQosKind kind;
    
    public OwnershipQosPolicy(OwnershipQosKind _kind){
        kind = _kind;
    }
}
