/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
