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
 * Concrete descendant of Status, which represents the Status of a Topic.
 * 
 * @date Oct 19, 2004 
 */
public class TopicStatus extends Status {
    private InconsistentTopicInfo inconsistentTopic;
    
    /**
     * Constructs a new TopicStatus from the supplied arguments.
     *  
     * @param _state State of the status which has no meaning in this context.
     * @param _inconsistentTopic Another topic exists with the same name but
     *                           different characteristics.
     */
    public TopicStatus(String _state, InconsistentTopicInfo _inconsistentTopic) {
        super(_state);
        inconsistentTopic = _inconsistentTopic;
    }

    /**
     * Provides access to inconsistentTopic.
     * 
     * @return Returns the inconsistentTopic.
     */
    public InconsistentTopicInfo getInconsistentTopic() {
        return inconsistentTopic;
    }
}
