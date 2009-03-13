package org.opensplice.dds.dcps.policy;

/**@brief Specifies the value of the "strength" used to arbitrate among multiple DataWriter
 * objects that attempt to modify the same instance of a data object.
 * 
 * - Concerns:    DataWriter
 * - RxO:         N/A
 * - Changable:   Yes
 */
public class OwnershipStrengthQosPolicy extends QosPolicy{
    long value;
    
    public OwnershipStrengthQosPolicy(long _value){
        value = _value;
    }
}

