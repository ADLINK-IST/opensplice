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
 * A span of elapsed time expressed with nanosecond precision.
 * <p>
 * Instances of this type are immutable.
 */
@Extensibility(Extensibility.Kind.FINAL_EXTENSIBILITY)
@Nested
public abstract class Duration
implements Comparable<Duration>, Serializable, DDSObject
{
    // -----------------------------------------------------------------------
    // Private Constants
    // -----------------------------------------------------------------------

    private static final long serialVersionUID = 6926514364942353575L;



    // -----------------------------------------------------------------------
    // Factory Methods
    // -----------------------------------------------------------------------

    /**
     * Construct a time duration of the given magnitude.
     * <p>
     * A duration of magnitude {@link Long#MAX_VALUE} indicates an infinite
     * duration, regardless of the units specified.
     *
     * @param env       Identifies the Service instance to which the new
     *                  object will belong.
     *
     * @see     #isInfinite()
     * @see     #infiniteDuration(ServiceEnvironment)
     */
    public static Duration newDuration(
            long duration,
            TimeUnit unit,
            ServiceEnvironment env)
    {
        if (env == null) {
            throw new IllegalArgumentException(
                    "Invalid ServiceEnvironment (null) provided.");
        }
        return env.getSPI().newDuration(duration, unit);
    }


    /**
     * @param env       Identifies the Service instance to which the
     *                  object will belong.
     *
     * @return  An unmodifiable {@link org.omg.dds.core.Duration} of infinite length.
     */
    public static Duration infiniteDuration(ServiceEnvironment env)
    {
        if (env == null) {
            throw new IllegalArgumentException(
                    "Invalid ServiceEnvironment (null) provided.");
        }
        return env.getSPI().infiniteDuration();
    }


    /**
     * @param env       Identifies the Service instance to which the
     *                  object will belong.
     *
     * @return  A {@link org.omg.dds.core.Duration} of zero length.
     */
    public static Duration zeroDuration(ServiceEnvironment env)
    {
        if (env == null) {
            throw new IllegalArgumentException(
                    "Invalid ServiceEnvironment (null) provided.");
        }
        return env.getSPI().zeroDuration();
    }



    // -----------------------------------------------------------------------
    // Instance Methods
    // -----------------------------------------------------------------------

    // --- Data access: ------------------------------------------------------

    /**
     * Truncate this duration to a whole-number quantity of the given time
     * unit. For example, if this duration is equal to one second plus 100
     * nanoseconds, calling this method with an argument of
     * {@link TimeUnit#SECONDS} will result in the value <code>1</code>.
     * <p>
     * If this duration is infinite, this method shall return
     * {@link Long#MAX_VALUE}, regardless of the units given.
     *  <p>
     * If this duration cannot be expressed in the given units without
     * overflowing, this method shall return {@link Long#MAX_VALUE}. In such
     * a case, the caller may wish to use this method in combination with
     * {@link #getRemainder(TimeUnit, TimeUnit)} to obtain the full duration
     * without lack of precision.
     *
     * @param   inThisUnit  The time unit in which the return result will
     *                      be measured.
     *
     * @see     #getRemainder(TimeUnit, TimeUnit)
     * @see     Long#MAX_VALUE
     * @see     TimeUnit
     */
    public abstract long getDuration(TimeUnit inThisUnit);

    /**
     * If getting the magnitude of this duration in the given
     * <code>primaryUnit</code> would cause truncation with respect to the
     * given <code>remainderUnit</code>, return the magnitude of the
     * truncation in the latter (presumably finer-grained) unit. For example,
     * if this duration is equal to one second plus 100 nanoseconds, calling
     * this method with arguments of {@link TimeUnit#SECONDS} and
     * {@link TimeUnit#NANOSECONDS} respectively will result in the value
     * <code>100</code>.
     * <p>
     * This method is equivalent to the following pseudo-code:
     *  <p>
     * <code>
     * (this - getDuration(primaryUnit)).getDuration(remainderUnit)
     * </code>
     *  <p>
     * If <code>remainderUnit</code> is represents a coarser granularity than
     * <code>primaryUnit</code> (for example, the former is
     * {@link TimeUnit#MILLISECONDS} but the latter is {@link TimeUnit#SECONDS}),
     * this method shall return <code>0</code>.
     * <p>
     * If the resulting duration cannot be expressed in the given units
     * without overflowing, this method shall return {@link Long#MAX_VALUE}.
     *
     * @param   primaryUnit
     * @param   remainderUnit   The time unit in which the return result will
     *                          be measured.
     *
     * @see     #getDuration(TimeUnit)
     * @see     Long#MAX_VALUE
     * @see     TimeUnit
     */
    public abstract long getRemainder(
            TimeUnit primaryUnit, TimeUnit remainderUnit);


    // --- Query: ------------------------------------------------------------

    /**
     * Report whether this duration lasts no time at all. The result of this
     * method is equivalent to the following:
     * <p>
     * <code>this.getDuration(TimeUnit.NANOSECONDS) == 0;</code>
     *
     * @see     #getDuration(TimeUnit)
     */
    public abstract boolean isZero();

    /**
     * Report whether this duration lasts forever.
     *  <p>
     * If this duration is infinite, the following relationship shall be
     * true:
     *  <p>
     * <code>this.equals(infiniteDuration(this.getEnvironment()))</code>
     *
     * @see     #infiniteDuration(ServiceEnvironment)
     */
    public abstract boolean isInfinite();


    // --- Manipulation: -----------------------------------------------------

    /**
     * @return  a new Duration that is the sum of this Duration and the
     *          given Duration.
     */
    public abstract Duration add(Duration duration);

    /**
     * @return  a new Duration that is the sum of this Duration and the
     *          given duration.
     */
    public abstract Duration add(long duration, TimeUnit unit);

    /**
     * @return  a new Duration that is the difference after subtracting the
     *          given Duration.
     */
    public abstract Duration subtract(Duration duration);

    /**
     * @return  a new Duration that is the difference after subtracting the
     *          given Duration.
     */
    public abstract Duration subtract(long duration, TimeUnit unit);
}
