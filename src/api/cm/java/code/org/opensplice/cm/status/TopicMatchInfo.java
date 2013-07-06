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
 * This class is applicable for two kinds of statusses and therefore has two
 * meanings:
 * 
 * 1. The DataReader has found a Writer that matches the Topic and has 
 *    compatible QoS. (SUBSCRIPTION_MATCH)
 * 2. The Writer has found DataReader that matches the Topic and has compatible 
 *    QoS. (PUBLICATION_MATCH)
 * 
 * @date Nov 15, 2004 
 */
public class TopicMatchInfo {
    private long total_count;
    private long total_count_change;
    private long current_count;
    private long current_count_change;
    private String instance_handle;
    
    /**
     * Constructs a new TopicMatchInfo from its supplied arguments.
     *  
     * @param _total_count Total cumulative count the concerned Writer 
     *                     discovered a "match" with a DataReader. That is, it 
     *                     found a DataReader for the same Topic with a 
     *                     requested QoS that is compatible with that offered by
     *                     the Writer. Or; total cumulative count the concerned 
     *                     DataReader discovered a "match" with a Writer. That 
     *                     is, it found a Writer for the same Topic with a 
     *                     requested QoS that is compatible with that offered by
     *                     the DataReader.
     * @param _total_count_change The change in total_count since the last time
     *                            the listener was called or the status was 
     *                            read.
     * @param _instance_handle Handle to the last DataReader that matched the 
     *                         Writer causing the status to change or, Handle to
     *                         the last Writer that matched the DataReader
     *                         causing the status to change.
     */
    public TopicMatchInfo(
            long _total_count,
            long _total_count_change,
            long _current_count, 
            long _current_count_change,
            String _instance_handle)
    {
        total_count = _total_count;
        total_count_change = _total_count_change;
        current_count = _current_count;
        current_count_change = _current_count_change;
        instance_handle = _instance_handle;
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
     * Provides access to instance_handle.
     * 
     * @return Returns the instance_handle.
     */
    public String getLastInstanceHandle() {
        return instance_handle;
    }
    
    /**
     * Provides access to total_changed.
     * 
     * @return Returns the total_changed.
     */
    public long getTotalCountChange() {
        return total_count_change;
    }
    
    public long getCurrentCount() {
        return this.current_count;
    }

    public long getCurrentCountChange() {
        return this.current_count_change;
    }
}
