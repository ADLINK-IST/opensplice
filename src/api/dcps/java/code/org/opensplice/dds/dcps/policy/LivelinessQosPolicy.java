package org.opensplice.dds.dcps.policy;

/**@brief Determines the mechanism and parameteres used by the application to determine
 * whether an Entity is "active" (alive). The liveliness status of an Entity is used
 * to maintain instance ownership in combination with the setting of OwnerShipQosPolicy.
 * 
 * - Concerns:    DataWriter/DataReader/Topic
 * - RxO:         Yes
 * - Changable:   No
 */ 
public class LivelinessQosPolicy {
    private Duration lease_duration;
    private LivelinessQosKind kind;
    
    /**@brief Creates new LivelinessQosPolicy
     * 
     * @param _lease_duration The lease period.
     * @param _kind The liveliness kind.
     */
    public LivelinessQosPolicy(Duration _lease_duration, LivelinessQosKind _kind){
        lease_duration = _lease_duration;
        kind = _kind;
    }
}
