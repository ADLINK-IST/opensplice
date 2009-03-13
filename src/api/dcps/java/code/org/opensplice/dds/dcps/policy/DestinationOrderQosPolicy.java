package org.opensplice.dds.dcps.policy;

/**@brief Controls the criteria used to determine the logical order among 
 * changes made by Publisher entities to the same instance of data
 * (i.e. matching Topic and key).
 *
 * - Concerns:    DataReader/Topic
 * - RxO:         No
 * - Changable:   No
 */
public class DestinationOrderQosPolicy {
    private DestinationOrderQosKind kind;
    
    /**@brief Controls the criteria used to determine the logical
     * order among changes made by Publisher entities to the same
     * instance of data(i.e. matching Topic and key).
     * 
     * @param _kind The kind of the destination order.
     */ 
    public DestinationOrderQosPolicy(DestinationOrderQosKind _kind){
        kind = _kind;   
    }
}

