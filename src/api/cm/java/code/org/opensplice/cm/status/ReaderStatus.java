/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package org.opensplice.cm.status;

/**
 * Concrete descendant of Status, which represents the Status of a DataReader. 
 * 
 * @date Oct 19, 2004 
 */
public class ReaderStatus extends Status {
    private LivelinessChangedInfo livelinessChanged;
    private SampleRejectedInfo sampleRejected;
    private DeadlineMissedInfo deadlineMissed;
    private IncompatibleQosInfo incompatibleQos;
    private SampleLostInfo sampleLost;
    private TopicMatchInfo subscriptionMatch;
    
    /**
     * Constructs a new ReaderStatus from the supplied arguments.
     *
     * @param _state Represents DATA_AVAILABLE status of a DataReader.
     * @param _livelinessChanged The liveliness of one or more Writer that were 
     *                           writing instances read through the DataReader
     *                           has changed. Some DataWriter have become 
     *                           "active" or "inactive".
     * @param _sampleRejected A (received) sample has been rejected.
     * @param _deadlineMissed The deadline that the DataReader was expecting 
     *                        through its QosPolicy DEADLINE was not respected
     *                        for a specific instance.
     * @param _incompatibleQos A QosPolicy value was incompatible with what is
     *                         offered.
     * @param _sampleLost A sample has been lost (never received).
     * @param _subscriptionMatch The DataReader has found a Writer that 
     *                           matches the Topic and has compatible QoS.
     */
    public ReaderStatus(
            String _state,
            LivelinessChangedInfo _livelinessChanged,
            SampleRejectedInfo _sampleRejected,
            DeadlineMissedInfo _deadlineMissed,
            IncompatibleQosInfo _incompatibleQos,
            SampleLostInfo _sampleLost,
            TopicMatchInfo _subscriptionMatch) 
    {
        super(_state);
        livelinessChanged = _livelinessChanged;
        sampleRejected = _sampleRejected;
        deadlineMissed = _deadlineMissed;
        incompatibleQos = _incompatibleQos;
        sampleLost = _sampleLost;
        subscriptionMatch = _subscriptionMatch;
    }

    /**
     * Provides access to deadlineMissed.
     * 
     * @return Returns the deadlineMissed.
     */
    public DeadlineMissedInfo getDeadlineMissed() {
        return deadlineMissed;
    }
    
    /**
     * Provides access to incompatibleQos.
     * 
     * @return Returns the incompatibleQos.
     */
    public IncompatibleQosInfo getIncompatibleQos() {
        return incompatibleQos;
    }
    
    /**
     * Provides access to livelinessChanged.
     * 
     * @return Returns the livelinessChanged.
     */
    public LivelinessChangedInfo getLivelinessChanged() {
        return livelinessChanged;
    }
    
    /**
     * Provides access to sampleRejected.
     * 
     * @return Returns the sampleRejected.
     */
    public SampleRejectedInfo getSampleRejected() {
        return sampleRejected;
    }
    
    /**
     * Provides access to sampleLost.
     * 
     * @return Returns the sampleLost.
     */
    public SampleLostInfo getSampleLost() {
        return sampleLost;
    }
    
    /**
     * Provides access to subscriptionMatch.
     * 
     * @return Returns the subscriptionMatch.
     */
    public TopicMatchInfo getSubscriptionMatch() {
        return subscriptionMatch;
    }
}
