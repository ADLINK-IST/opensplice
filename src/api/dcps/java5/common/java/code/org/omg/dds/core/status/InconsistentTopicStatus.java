/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Inc.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.omg.dds.core.status;




/**
 * Another topic exists with the same name but different characteristics.
 * This class contains the statistics about attempts to create other Topics with the
 * same name but with different characteristics. The attribute totalCount holds the
 * total detected cumulative count of Topic creations, whose name matches the Topic
 * to which this Status is attached and whose characteristics are inconsistent.
 * The attribute totalCountChange holds the incremental number of inconsistent Topics,
 * since the last time the Listener was called or the Status was read.
 *
 * @see org.omg.dds.core.event.InconsistentTopicEvent
 */
public abstract class InconsistentTopicStatus extends Status
{
    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------

    private static final long serialVersionUID = -1695476267550323893L;



    // -----------------------------------------------------------------------
    // Methods
    // -----------------------------------------------------------------------

    /**
     * Total cumulative count of the {@link org.omg.dds.topic.Topic}s discovered whose name
     * matches the Topic to which this status is attached and whose type is
     * inconsistent with the Topic.
     */
    public abstract int getTotalCount();

    /**
     * The incremental number of inconsistent topics discovered since the
     * last time the listener was called or the status was read.
     */
    public abstract int getTotalCountChange();

}
