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

import java.io.Serializable;
import java.util.concurrent.TimeUnit;

import org.omg.dds.type.Extensibility;
import org.omg.dds.type.Nested;


/**
 * A moment in time expressed with nanosecond precision (though not
 * necessarily nanosecond accuracy).
 */
@Extensibility(Extensibility.Kind.FINAL_EXTENSIBILITY)
@Nested
public abstract class Time
implements Comparable<Time>, Serializable, DDSObject
{
    // -----------------------------------------------------------------------
    // Private Constants
    // -----------------------------------------------------------------------

    private static final long serialVersionUID = -132361141453190372L;



    // -----------------------------------------------------------------------
    // Factory Methods
    // -----------------------------------------------------------------------

    /**
     * Construct a specific instant in time.
     * <p>
     * Negative values are considered invalid and will result in the
     * construction of a time <code>t</code> such that:
     * <p>
     * <code>t.isValid() == false</code>
     *
     * @param env       Identifies the Service instance to which the new
     *                  object will belong.
     *
     * @see     #isValid()
     */
    public static ModifiableTime newTime(
            long time,
            TimeUnit units,
            ServiceEnvironment env)
    {
        if (env == null) {
            throw new IllegalArgumentException(
                    "Invalid ServiceEnvironment (null) provided.");
        }
        return env.getSPI().newTime(time, units);
    }


    /**
     * @param env       Identifies the Service instance to which the
     *                  object will belong.
     *
     * @return      An unmodifiable {@link org.omg.dds.core.Time} that is not valid.
     */
    public static Time invalidTime(ServiceEnvironment env)
    {
        if (env == null) {
            throw new IllegalArgumentException(
                    "Invalid ServiceEnvironment (null) provided.");
        }
        return env.getSPI().invalidTime();
    }



    // -----------------------------------------------------------------------
    // Instance Methods
    // -----------------------------------------------------------------------

    // --- Data access: ------------------------------------------------------

    /**
     * Truncate this time to a whole-number quantity of the given time
     * unit. For example, if this time is equal to one second plus 100
     * nanoseconds since the start of the epoch, calling this method with an
     * argument of {@link TimeUnit#SECONDS} will result in the value
     * <code>1</code>.
     * <p>
     * If this time is invalid, this method shall return
     * a negative value, regardless of the units given.
     * <p>
     * If this time cannot be expressed in the given units without
     * overflowing, this method shall return {@link Long#MAX_VALUE}. In such
     * a case, the caller may wish to use this method in combination with
     * {@link #getRemainder(TimeUnit, TimeUnit)} to obtain the full time
     * without lack of precision.
     *
     * @param   inThisUnit  The time unit in which the return result will
     *                      be measured.
     *
     * @see     #getRemainder(TimeUnit, TimeUnit)
     * @see     Long#MAX_VALUE
     * @see     TimeUnit
     */
    public abstract long getTime(TimeUnit inThisUnit);

    /**
     * If getting the magnitude of this time in the given
     * <code>primaryUnit</code> would cause truncation with respect to the
     * given <code>remainderUnit</code>, return the magnitude of the
     * truncation in the latter (presumably finer-grained) unit. For example,
     * if this time is equal to one second plus 100 nanoseconds since the
     * start of the epoch, calling this method with arguments of
     * {@link TimeUnit#SECONDS} and {@link TimeUnit#NANOSECONDS} respectively
     * will result in the value <code>100</code>.
     * <p>
     * This method is equivalent to the following pseudo-code:
     * <p>
     * <code>(this - getTime(primaryUnit)).getTime(remainderUnit)</code>
     *  <p>
     * If <code>remainderUnit</code> is represents a coarser granularity than
     * <code>primaryUnit</code> (for example, the former is
     * {@link TimeUnit#MILLISECONDS} but the latter is {@link TimeUnit#SECONDS}),
     * this method shall return <code>0</code>.
     * <p>
     * If the resulting time cannot be expressed in the given units
     * without overflowing, this method shall return {@link Long#MAX_VALUE}.
     *
     * @param   primaryUnit
     * @param   remainderUnit   The time unit in which the return result will
     *                          be measured.
     *
     * @see     #getTime(TimeUnit)
     * @see     Long#MAX_VALUE
     * @see     TimeUnit
     */
    public abstract long getRemainder(
            TimeUnit primaryUnit, TimeUnit remainderUnit);


    // --- Query: ------------------------------------------------------------

    /**
     * @return  whether this time represents a meaningful instant in time.
     */
    public abstract boolean isValid();


    // --- Modification: -----------------------------------------------------

    /**
     * @return  a modifiable copy of this object's state.
     */
    public abstract ModifiableTime modifiableCopy();
}
