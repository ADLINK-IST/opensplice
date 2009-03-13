package org.opensplice.dds.dcps.policy;

/**@brief This policy is meant to be able to specifiy a deadline.
 * 
 * - Concerns:    DataReader/DataWriter/Topic
 * - RxO:         Yes
 * - Changable:   Yes
 */
public class DeadlineQosPolicy {
    private Duration period;
    
    /**@brief DataReader expects a new sample updating the value of each instance
     * at least once every _period. 
     * 
     * DataWriter indicates that the application commits to write a new value
     * for each instance managed by the DataWriter at least each _period.
     * 
     * The default value is infinite.
     * 
     * @param _period The timespan an update must be received. 
     */    
    public DeadlineQosPolicy(Duration _period){
        period = _period;
    }
}

