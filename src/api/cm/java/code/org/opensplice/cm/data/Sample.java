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
package org.opensplice.cm.data;

/**
 * Represents a Splice Sample.
 * 
 * @date May 13, 2004
 */
public class Sample {
    /**
     * Constructs a new Sample.
     * 
     * @param _insertTimeSec The time the data was inserted in the reader database(seconds).
     * @param _insertTimeNanoSec The time the data was inserted in the reader database(nanoseconds).
     * @param _state The state of the sample.
     * @param _disposeCount Indicates the number of times the instance had
     *                      become alive after it was disposed explicitly by a 
     *                      Writer, at the time the Sample was received.
     * @param _noWritersCount Indicates the number of times the instance had 
     *                        become alive after it was disposed because there 
     *                        were no writers, at the time the Sample was 
     *                        received.
     * @param _previous Reference to the previous Sample.
     * @param _message Message in the Sample.
     */
    public Sample(
            long _insertTimeSec, 
            long _insertTimeNanoSec, 
            State _state, 
            long _disposeCount, 
            long _noWritersCount, 
            Sample _previous, 
            Message _message)
    {
        insertTimeSec = _insertTimeSec;
        insertTimeNanoSec = _insertTimeNanoSec;
        state = _state;
        disposeCount = _disposeCount;
        noWritersCount = _noWritersCount;
        previous = _previous;
        message = _message;        
    }
    
    /**
     * Constructs a new Sample.
     * 
     * @param _insertTimeSec The time the data was inserted in the reader database(seconds).
     * @param _insertTimeNanoSec The time the data was inserted in the reader database(nanoseconds).
     * @param _state The state of the sample.
     * @param _disposeCount Indicates the number of times the instance had
     *                      become alive after it was disposed explicitly by a 
     *                      Writer, at the time the Sample was received.
     * @param _noWritersCount Indicates the number of times the instance had 
     *                        become alive after it was disposed because there 
     *                        were no writers, at the time the Sample was 
     *                        received.   
     * @param _message Message in the Sample.
     */
    public Sample(
            long _insertTimeSec, 
            long _insertTimeNanoSec, 
            State _state, 
            long _disposeCount, 
            long _noWritersCount, 
            Message _message)
    {
        insertTimeSec = _insertTimeSec;
        insertTimeNanoSec = _insertTimeNanoSec;
        state = _state;
        disposeCount = _disposeCount;
        noWritersCount = _noWritersCount;
        previous = null;
        message = _message;        
    }
    
    /**
     * Provides access to the insert time(nanoseconds).
     * 
     * @return The insert time (nanoseconds)
     */
    public long getInsertTimeNanoSec() {
        return insertTimeNanoSec;
    }

    /**
     * Provides access to the insert time(seconds).
     * 
     * @return The insert time (seconds)
     */
    public long getInsertTimeSec() {
        return insertTimeSec;
    }

    /**
     * Provides access to the Message.
     * 
     * @return The message in this Sample.
     */
    public Message getMessage() {
        return message;
    }

    /**
     * Provides access to the previous Sample.
     * 
     * @return The previous Sample of this Sample, or null if not available.
     */
    public Sample getPrevious() {
        return previous;
    }

    /**
     * Provides access to the Sample state.
     * 
     * @return The state of this Sample.
     */
    public State getState() {
        return state;
    }

    /**
     * Sets the insert time (nanosec) to the supplied value.
     * 
     * @param l The value to set.
     */
    public void setInsertTimeNanoSec(long l) {
        insertTimeNanoSec = l;
    }

    /**
     * Sets the insert time (sec) to the supplied value.
     * 
     * @param l The value to set.
     */
    public void setInsertTimeSec(long l) {
        insertTimeSec = l;
    }

    /**
     * Sets the message in this Sample to the supplied value.
     * 
     * @param message The value to set.
     */
    public void setMessage(Message message) {
        this.message = message;
    }

    /**
     * Sets the previous Sample to the supplied value.
     * 
     * @param sample The value to set.
     */
    public void setPrevious(Sample sample) {
        previous = sample;
    }

    /**
     * Sets the sample state to the supplied value.
     * 
     * @param l The value to set.
     */
    public void setState(State state) {
        this.state = state;
    }
    
    /**
     * Provides access to disposeCount.
     * 
     * @return Returns the disposeCount.
     */
    public long getDisposeCount() {
        return disposeCount;
    }
    
    /**
     * Sets the disposeCount to the supplied value.
     *
     * @param disposeCount The disposeCount to set.
     */
    public void setDisposeCount(long disposeCount) {
        this.disposeCount = disposeCount;
    }
    
    /**
     * Provides access to noWritersCount.
     * 
     * @return Returns the noWritersCount.
     */
    public long getNoWritersCount() {
        return noWritersCount;
    }
    
    /**
     * Sets the noWritersCount to the supplied value.
     *
     * @param noWritersCount The noWritersCount to set.
     */
    public void setNoWritersCount(long noWritersCount) {
        this.noWritersCount = noWritersCount;
    }
    
    /**
     * Time the data was inserted in the reader database (seconds).
     */
    private long insertTimeSec;
    
    /**
     * Time the data was inserted in the reader database (nanoseconds).
     */
    private long insertTimeNanoSec;
    
    /**
     * Indicates the number of times the instance had become alive after it was 
     * disposed explicitly by a Writer, at the time the Sample was received
     */
    private long disposeCount;
    
    /**
     * Indicates the number of times the instance had become alive after it was 
     * disposed because there were no writers, at the time the sample was 
     * received.
     */
    private long noWritersCount;
    
    /**
     * The sample state.
     */
    private State state;
    
    /**
     * Reference to the previous sample.
     */
    private Sample previous;
    
    /**
     * The message in the Sample.
     */
    private Message message;
}
