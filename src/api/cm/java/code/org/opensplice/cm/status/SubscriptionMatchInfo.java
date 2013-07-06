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
 * 
 * 
 * @date Oct 12, 2004
 * @deprecated Use TopicMatchStatus instead. 
 */
public class SubscriptionMatchInfo {
    private long total_count;
    private long total_count_change;
    private String last_subscription_handle;
    
    public SubscriptionMatchInfo(
            long _total_count, 
            long _total_count_change, 
            String _last_subscription_handle)
    {
        total_count = _total_count;
        total_count_change = _total_count_change;
        last_subscription_handle = _last_subscription_handle;
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
     * Provides access to last_subscription_handle.
     * 
     * @return Returns the last_subscription_handle.
     */
    public String getLastSubscriptionHandle() {
        return last_subscription_handle;
    }
}
