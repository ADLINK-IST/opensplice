package org.opensplice.dds.dcps.policy;

/**@brief Specifies the resources that the service can consume in order to meet the 
 * requested QoS.
 * 
 * - Concerns:    DataWriter/DataReader/Topic
 * - RxO:         No
 * - Changable:   No
 */
public class ResourceLimitsQosPolicy {
    private long max_samples;
    private long max_instances;
    private long max_samples_per_instance;
    
    /**@brief Creates new ResourceLimitsQosPolicy
     * 
     * @param _max_samples The maximum number of data samples the DataWriter (or DataReader)
     * can manage across all the instances associated with it.  
     * @param _max_instances Represents the maximum number of instances a DataWriter (or
     * Datareader) can manage.
     * @param _max_samples_per_instance The maximum number of samples of any one instance
     * a DataWriter (or Datareader) can manage.
     */
    public ResourceLimitsQosPolicy( long _max_samples,
                                    long _max_instances,
                                    long _max_samples_per_instance){
        max_samples                 = _max_samples;
        max_instances               = _max_instances;
        max_samples_per_instance    = _max_samples_per_instance;
    }
}

