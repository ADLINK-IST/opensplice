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
package org.opensplice.dds.core.event;

import org.omg.dds.core.event.StatusChangedEvent;
import org.omg.dds.topic.Topic;
import org.opensplice.dds.core.status.AllDataDisposedStatus;

/**
 * All instances published for this topic have been disposed by means of a call
 * to {@link org.opensplice.dds.topic.Topic#disposeAllData()}
 *
 * @param <TYPE>
 *            The data type of the source {@link org.omg.dds.topic.Topic}
 *
 * @see AllDataDisposedStatus
 */
public abstract class AllDataDisposedEvent<TYPE> extends
        StatusChangedEvent<Topic<TYPE>> {
    private static final long serialVersionUID = 6035423504123023667L;

    protected AllDataDisposedEvent(Topic<TYPE> source) {
        super(source);
    }

    /**
     * Get access to the corresponding AllDataDisposedStatus.
     *
     * @return The corresponding AllDataDisposedStatus
     */
    public abstract AllDataDisposedStatus getStatus();

    @Override
    public abstract AllDataDisposedEvent<TYPE> clone();
}
