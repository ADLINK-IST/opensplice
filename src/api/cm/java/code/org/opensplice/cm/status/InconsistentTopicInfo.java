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
 * Another topic exists with the same name but different characteristics.
 * 
 * @date Oct 12, 2004 
 */
public class InconsistentTopicInfo{
    private long total_count;
    private long total_count_change;
    
    /**
     * Constructs a new InconsistentTopicInfo according to the supplied 
     * arguments.
     *
     * @param _total_count Total cumulative count of the Topics discovered whose
     *                     name matches the Topic to which this status is 
     *                     attached and whose type is inconsistent with the 
     *                     Topic.
     * @param _total_count_change The incremental number of inconsistent topics
     *                            discovered since the last time the listener
     *                            was called or the status was read.
     */
    public InconsistentTopicInfo(long _total_count, long _total_count_change){
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
