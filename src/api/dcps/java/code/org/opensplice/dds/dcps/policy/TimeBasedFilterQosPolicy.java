package org.opensplice.dds.dcps.policy;

/**@brief Filter that allows a DataReader to specify that it is interested in (potentially)
 * a subset of the values of the data.
 * 
 * - Concerns:    DataReader
 * - RxO:         N/A
 * - Changable:   Yes 
 */
public class TimeBasedFilterQosPolicy {
    private Duration minimum_separation;
    
    /**@brief Creates a new TimeBasedFilterQosPolicy.
     * 
     * @param _minimum_separation States that the DataReader does not want to receive more
     * than one value each _minimum_separation regardless of how fast changes occur. 
     */
    public TimeBasedFilterQosPolicy(Duration _minimum_separation){
        minimum_separation = _minimum_separation;
    }
}

