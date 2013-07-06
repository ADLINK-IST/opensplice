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

/**
 * Only used in PublicationMatchInfo and SubscriptionMatchInfo, which are 
 * deprecated.
 * 
 * @date Oct 12, 2004 
 */
public class QosPolicyCount {
    long policy_id;
    long count;
    
    public QosPolicyCount(long _policy_id, long _count){
        policy_id = _policy_id;
        count = _count;
    }
    /**
     * Provides access to policy_id.
     * 
     * @return Returns the policy_id.
     */
    public long getPolicyId() {
        return policy_id;
    }
    /**
     * Provides access to count.
     * 
     * @return Returns the count.
     */
    public long getCount() {
        return count;
    }
}
