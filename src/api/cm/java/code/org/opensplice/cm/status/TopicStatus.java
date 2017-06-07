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
