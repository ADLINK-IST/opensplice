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

import java.io.Serializable;
import java.util.Set;

import org.omg.dds.core.DDSObject;
import org.omg.dds.core.ServiceEnvironment;


/**
 * Status is the abstract root class for all communication status objects.
 * All concrete kinds of Status classes extend this class.
 * 
 * Each concrete {@link org.omg.dds.core.Entity} is associated with a set of Status objects
 * whose value represents the "communication status" of that entity. These
 * status values can be accessed with corresponding methods on the Entity.
 * The changes on these status values are the ones that both cause activation
 * of the corresponding {@link org.omg.dds.core.StatusCondition} objects and trigger invocation
 * of the proper Listener objects to asynchronously inform the application.
 * 
 * @see org.omg.dds.core.event.StatusChangedEvent
 */
public abstract class Status implements Serializable, DDSObject
{
    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------

    private static final long serialVersionUID = 8294883446033723160L;



    // -----------------------------------------------------------------------
    // Object Life Cycle
    // -----------------------------------------------------------------------

    /**
     * @param env       Identifies the Service instance to which the
     *                  object will belong.
     */
    public static Set<Class<? extends Status>> allStatuses(
            ServiceEnvironment env)
    {
        return env.getSPI().allStatusKinds();
    }


    /**
     * @param env Identifies the Service instance to which the
     *                  object will belong.
     */
    public static Set<Class<? extends Status>> noStatuses(
            ServiceEnvironment env)
    {
        return env.getSPI().noStatusKinds();
    }
}
