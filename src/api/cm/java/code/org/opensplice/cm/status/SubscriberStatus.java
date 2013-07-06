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
