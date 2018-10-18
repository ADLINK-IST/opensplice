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
 * Concrete descendant of Status, which represents the Status of a Subscriber.
 * 
 * @date Oct 19, 2004 
 */
public class SubscriberStatus extends Status{
    private SampleLostInfo sampleLost;
    
    /**
     * Creates a new SubscriberStatus from its supplied arguments. 
     *
     * @param _state The state, which represents DATA_ON_READERS communication 
     *               status.
     * @param _sampleLost A sample has been lost (never received).
     */
    public SubscriberStatus(String _state, SampleLostInfo _sampleLost) {
        super(_state);
        sampleLost = _sampleLost;
    }

    /**
     * Provides access to sampleLost.
     * 
     * @return Returns the sampleLost.
     */
    public SampleLostInfo getSampleLost() {
        return sampleLost;
    }
}
