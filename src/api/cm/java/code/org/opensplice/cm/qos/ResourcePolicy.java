/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
package org.opensplice.cm.qos;

/**
 * Specifies the resources that the service can consume in order to meet the
 * requested QoS.
 * 
 * @date Jan 10, 2005 
 */
public class ResourcePolicy {
    /**
     * Specifies the maximum number of datasamples the Writer (or DataReader)
     * can manage across all the instances associated with it. Represents the 
     * maximum samples the middleware can store for any one Writer (or 
     * DataReader) By default, unlimited.
     */
    public int max_samples;
    
    /**
     * Represents the maximum number of instances Writer (or DataReader) can 
     * manage. By default, unlimited.
     */
    public int max_instances;
    
    /**
     * Represents the maximum number of samples of any one instance a Writer (or 
     * DataReader) can manage. By default, unlimited.
     */
    public int max_samples_per_instance;
    
    public static final ResourcePolicy DEFAULT = new ResourcePolicy(-1, -1, -1);
    
    /**
     * Constructs a new ResourcePolicy.  
     *
     * @param _max_samples The maximum amount of samples.
     * @param _max_instances The maximum amount of instances.
     * @param _max_samples_per_instance The maximum amount of samples per 
     *                                  instance.
     */
    public ResourcePolicy(int _max_samples, int _max_instances, int _max_samples_per_instance){
        max_samples = _max_samples;
        max_instances = _max_instances;
        max_samples_per_instance = _max_samples_per_instance;
    }
    
    public ResourcePolicy copy(){
        return new ResourcePolicy(this.max_samples, this.max_instances, this.max_samples_per_instance);
    }
}
