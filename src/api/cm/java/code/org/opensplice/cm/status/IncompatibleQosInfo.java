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
package org.opensplice.cm.status;

import java.util.ArrayList;

/**
 * This class is applicable for two kinds of statusses and therefore has two
 * meanings:
 * 
 * 1. WriterStatus: A QosPolicy value was incompatible with what was 
 *    requested. (OFFEREED_INCOMPATBIBLE_QOS)
 * 2. ReaderStatus: A QosPolicy value was incompatible with what is offered.
 *    (REQUESTED_INCOMPATBIBLE_QOS)
 * 
 * @date Oct 12, 2004 
 */
public class IncompatibleQosInfo{
    private final long total_count;
    private final long total_count_change;
    private long last_policy_id;
    private ArrayList policies;
    private final String[] policyNames = {
        "INVALID_QOS_POLICY_ID", "USERDATA_QOS_POLICY_ID",
        "DURABILITY_QOS_POLICY_ID", "PRESENTATION_QOS_POLICY_ID",
        "DEADLINE_QOS_POLICY_ID", "LATENCYBUDGET_QOS_POLICY_ID", 
        "OWNERSHIP_QOS_POLICY_ID", "OWNERSHIPSTRENGTH_QOS_POLICY_ID", 
        "LIVELINESS_QOS_POLICY_ID", "TIMEBASEDFILTER_QOS_POLICY_ID", 
        "PARTITION_QOS_POLICY_ID", "RELIABILITY_QOS_POLICY_ID", 
        "DESTINATIONORDER_QOS_POLICY_ID", "HISTORY_QOS_POLICY_ID", 
        "RESOURCELIMITS_QOS_POLICY_ID", "ENTITYFACTORY_QOS_POLICY_ID", 
        "WRITERDATALIFECYCLE_QOS_POLICY_ID", "READERDATALIFECYCLE_QOS_POLICY_ID", 
        "TOPICDATA_QOS_POLICY_ID", "GROUPDATA__QOS_POLICY_ID", 
        "TRANSPORTPRIORITY_QOS_POLICY_ID", "LIFESPAN_QOS_POLICY_ID", 
        "DURABILITYSERVICE_QOS_POLICY_ID", "USERKEY_QOS_POLICY_ID", 
        "VIEWKEY_QOS_POLICY_ID", "READERLIFESPAN_QOS_POLICY_ID", 
        "SHAREPOLICY_QOS_POLICY_ID", "SCHEDULINGPOLICY_QOS_POLICY_ID"};
    
    /**
     * Constructs a new IncompatibleQosInfo according to the supplied arguments.
     *
     * @param _total_count Total cumulative count the concerned DataReader 
     *                     discovered a DataWriter for the same Topic with an 
     *                     offered QoS that was incompatible with that requested
     *                     by the DataReade or total cumulative number of times
     *                     the concerned DataWriter discovered a DataReader for
     *                     the same Topic with a requested QoS that is
     *                     incompatible with that offered by the DataWriter.
     * @param _total_count_change The change in total_count since the last time
     *                            the listener was called or the status was
     *                            read.
     */
    public IncompatibleQosInfo(long _total_count, long _total_count_change){
        total_count = _total_count;
        total_count_change = _total_count_change;
        policies = null;
    }
    
    /**
     * Provides access to total_count.
     * 
     * @return Returns the total_count.
     */
    public long getTotalCount() {
        return total_count;
    }
    
    /**
     * Provides access to total_count_change.
     * 
     * @return Returns the total_count_change.
     */
    public long getTotalCountChange() {
        return total_count_change;
    }
    /**
     * Provides access to last_policy_id.
     * 
     * @return Returns the last_policy_id.
     */
    public long getLastPolicyId() {
        return last_policy_id;
    }
    
    public String getLastPolicyIdName(){
        String result;
        
        if(this.policyNames.length >= this.last_policy_id+1){
            result = policyNames[(int) (this.last_policy_id)];
        } else {
            result = "!CM API MISMATCH!"; 
        }
        return result;
    }
    
    /**
     * Provides access to policies.
     * 
     * @todo TODO: change return type from Long[] to long[].
     * @return Returns the policies.
     */
    public Long[] getPolicies() {
        Long[] result = null;
        
        if(policies != null){
            result = (Long[])
                    (policies.toArray(new Long[policies.size()]));
        }
        return result;
    }
    
    /**
     * Adds the supplied policy to the list of policies.
     * 
     * @param policy
     */
    public void addPolicy(long policy){
        if(policies == null){
            policies = new ArrayList();
        }
        policies.add(new Long(policy));
        this.last_policy_id = policy;
    }
    
    /**
     * Sets the last_policy_id to the supplied value.
     *
     * @param last_policy_id The last_policy_id to set.
     */
    public void setLastPolicyId(long last_policy_id) {
        this.last_policy_id = last_policy_id;
    }
}
