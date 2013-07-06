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
 * A sample has been lost (never received).
 * 
 * @date Oct 12, 2004 
 */
public class SampleLostInfo {
    private long total_count;
    private long total_count_change;
    
    /**
     * Constructs a new SampleLostInfo from the supplied arguments.
     *
     * @param _total_count Total cumulative count of all samples lost across of
     *                     instances of data published under the Topic.
     * @param _total_count_change The incremental number of samples lost since
     *                            the last time the listener was called or the 
     *                            Status was read.
     */
    public SampleLostInfo(long _total_count, long _total_count_change){
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
