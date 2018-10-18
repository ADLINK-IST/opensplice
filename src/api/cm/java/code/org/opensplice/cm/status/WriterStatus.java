/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
 * Concrete descendant of Status, which represents the Status of a Writer. 
 * 
 * @date Oct 19, 2004 
 */
public class WriterStatus extends Status {
    private LivelinessLostInfo livelinessLost;
    private DeadlineMissedInfo deadlineMissed;
    private IncompatibleQosInfo incompatibleQos;
    private TopicMatchInfo publicationMatch;
    
    /**
     * Constructs a new WriterStatus from the supplied arguments.
     *  
     * @param _state The state of the Status which has no meaning in this
     *               context.
     * @param _livelinessLost The liveliness that the Writer has committed
     *                        through its QosPolicy LIVELINESS was not 
     *                        respected; thus DataReader entities will consider
     *                        the Writer as no longer "active".
     * @param _deadlineMissed The deadline that the Writer has committed 
     *                        through its QosPolicy DEADLINE was not respected
     *                        for a specific instance.
     * @param _incompatibleQos A QosPolicy value was incompatible with what was
     *                         requested.
     * @param _publicationMatch The Writer has found DataReader that matches the
     *                          Topic and has compatible QoS.
     */
    public WriterStatus(
            String _state, 
            LivelinessLostInfo _livelinessLost,
            DeadlineMissedInfo _deadlineMissed,
            IncompatibleQosInfo _incompatibleQos,
            TopicMatchInfo _publicationMatch) {
        super(_state);
        livelinessLost = _livelinessLost;
        deadlineMissed = _deadlineMissed;
        incompatibleQos = _incompatibleQos;
        publicationMatch = _publicationMatch;
    }

    /**
     * Provides access to publicationMatch.
     * 
     * @return Returns the publicationMatch.
     */
    public TopicMatchInfo getPublicationMatch() {
        return publicationMatch;
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
     * Provides access to livelinessLost.
     * 
     * @return Returns the livelinessLost.
     */
    public LivelinessLostInfo getLivelinessLost() {
        return livelinessLost;
    }
}
