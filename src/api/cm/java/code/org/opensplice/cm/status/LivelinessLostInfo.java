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
 * The liveliness that the DataWriter has committed through its QosPolicy
 * LIVELINESS was not respected; thus DataReader entities will consider 
 * the Writer as no longer "active".
 * 
 * @date Oct 12, 2004 
 */
public class LivelinessLostInfo{
    private long total_count;
    private long total_count_change;
    
    /**
     * Constructs a new LivelinessLostInfo according to the supplied arguments.
     *
     * @param _total_count Total cumulative count of the number of times the 
     *                     Writer failed to actively signal its liveliness
     *                     within the offered liveliness period.
     * @param _total_count_change The change in total_count since the last time
     *                            the listener was called or the status was 
     *                            read.
     */
    public LivelinessLostInfo(long _total_count, long _total_count_change){
        total_count = _total_count;
        total_count_change = _total_count_change;
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
}
