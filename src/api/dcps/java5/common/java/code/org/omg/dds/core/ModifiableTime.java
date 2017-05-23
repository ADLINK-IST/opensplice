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

package org.omg.dds.core;

import java.util.concurrent.TimeUnit;


public abstract class ModifiableTime extends Time
{
    // -----------------------------------------------------------------------
    // Private Constants
    // -----------------------------------------------------------------------

    private static final long serialVersionUID = -881589038981141796L;



    // -----------------------------------------------------------------------
    // Methods
    // -----------------------------------------------------------------------

    /**
     * Overwrite the state of this object with that of the given object.
     */
    public abstract void copyFrom(Time src);

    /**
     * @return  an immutable copy of this object's state.
     */
    public abstract Time immutableCopy();


    // --- Data access: ------------------------------------------------------

    public abstract void setTime(long time, TimeUnit unit);


    // --- Manipulation: -----------------------------------------------------

    /**
     * Increment this time by the given amount.
     */
    public abstract void add(Duration duration);

    /**
     * Increment this time by the given amount.
     */
    public abstract void add(long duration, TimeUnit unit);

    /**
     * Decrement this time by the given amount.
     */
    public abstract void subtract(Duration duration);

    /**
     * Decrement this time by the given amount.
     */
    public abstract void subtract(long duration, TimeUnit unit);
}
