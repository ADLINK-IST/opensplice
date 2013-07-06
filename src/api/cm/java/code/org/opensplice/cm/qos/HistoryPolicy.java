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
 * Specifies the behavior of the service in the case where the value of a sample
 * changes (one or more times) before it can be successfully communicated to one 
 * or more existing subscribers. This QoS policy controls whether the Service
 * should deliver only the most recent value, attempt to deliver all 
 * intermediate values, or do something in between. On the publishing side this 
 * policy controls the samples that should be maintained by the Writer on behalf
 * of existing DataReader entities. The behavior with regards to a DataReader
 * entities discovered after a sample is written is controlled by the 
 * DurabilityPolicy. On the subscribing side it controls the samples that should 
 * be maintained until the application "takes" them from the service.
 * 
 * @date Jan 10, 2005 
 */
public class HistoryPolicy {
    /**
     * Determines how samples are kept on the publishing and subscribing side.
     */
    public HistoryQosKind kind;
    
    /**
     * The number of samples to keep for an instance. The default value of depth 
     * is 1.If a value other than 1 is specified, it should be consistent with 
     * the settings of the ResourcePolicy.
     */
    public int depth;
    
    public static final HistoryPolicy DEFAULT = new HistoryPolicy(HistoryQosKind.KEEPLAST, 1);
    
    /**
     * Constructs a new HistoryPolicy.
     *  
     *
     * @param _kind The history kind.
     * @param _depth The history depth.
     */
    public HistoryPolicy(HistoryQosKind _kind, int _depth){
        kind = _kind;
        depth = _depth;
    }
    
    public HistoryPolicy copy(){
        return new HistoryPolicy(this.kind, this.depth);
    }
}
