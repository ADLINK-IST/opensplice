package org.opensplice.dds.dcps.policy;

/**@brief Combination of seconds and nanoseconds.
 */
public class Duration {
    long sec;
    long nanosec;
    
    public Duration(long _sec, long _nanosec){
        sec = _sec;
        nanosec = _nanosec;
    }
}

