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

package org.omg.dds.type;

import org.omg.dds.core.DDSObject;
import org.omg.dds.core.ServiceEnvironment;


/**
 * TypeSupport is an abstract interface that has to be specialized for each
 * concrete type that will be used by the application to publish and/or
 * subscribe to data over DDS.
 *
 * @param <TYPE>    The type to be supported for publication and/or
 *                  subscription.
 */
public abstract class TypeSupport<TYPE> implements DDSObject
{
    // -----------------------------------------------------------------------
    // Factory Methods
    // -----------------------------------------------------------------------

    // --- Types: ------------------------------------------------------------

    /**
     * Create a new TypeSupport object for the given physical type.
     * This method is equivalent to:
     * <p>
     * <code>newTypeSupport(type, type.getClass().getName(), bootstrap)</code>
     *
     * @see #newTypeSupport(Class, String, ServiceEnvironment)
     */
    public static <TYPE> TypeSupport<TYPE> newTypeSupport(
            Class<TYPE> type,
            ServiceEnvironment env)
    {
        if (type == null) {
            throw new IllegalArgumentException("Invalid type provided");
        }
        return newTypeSupport(type, type.getName(), env);
    }


    /**
     * Create a new TypeSupport object for the given physical type.
     * The Service will register this type under the given name with any
     * participant with which the TypeSupport is used.
     *
     * @param <TYPE>    The physical type of all samples read or written by
     *                  any {@link org.omg.dds.sub.DataReader} or
     *                  {@link org.omg.dds.pub.DataWriter} typed by the
     *                  resulting <code>TypeSupport</code>.
     * @param type      The physical type of all samples read or written by
     *                  any {@link org.omg.dds.sub.DataReader} or
     *                  {@link org.omg.dds.pub.DataWriter} typed by the
     *                  resulting <code>TypeSupport</code>.
     * @param registeredName    The logical name under which this type will
     *                          be registered with any
     *                          {@link org.omg.dds.domain.DomainParticipant}
     *                          with which the resulting
     *                          <code>TypeSupport</code> is used.
     * @param env       Identifies the Service instance to which the new
     *                  object will belong.
     *
     * @return          A new <code>TypeSupport</code> object, which can
     *                  subsequently be used to create one or more
     *                  {@link org.omg.dds.topic.Topic}s.
     *
     * @see #newTypeSupport(Class, ServiceEnvironment)
     */
    public static <TYPE> TypeSupport<TYPE> newTypeSupport(
            Class<TYPE> type,
            String registeredName,
            ServiceEnvironment env)
    {
        if (env == null) {
            throw new IllegalArgumentException("Invalid environment provided");
        }
        if (type == null) {
            throw new IllegalArgumentException("Invalid type provided");
        }
        return env.getSPI().newTypeSupport(type, registeredName);
    }



    // -----------------------------------------------------------------------
    // Instance Methods
    // -----------------------------------------------------------------------

    /**
     * @return  a new object of the type supported by this TypeSupport.
     */
    public abstract TYPE newData();

    /**
     * @return  the class of the type supported by this TypeSupport.
     */
    public abstract Class<TYPE> getType();

    /**
     * @return  the registered name for the data type represented by the
     *          TypeSupport.
     */
    public abstract String getTypeName();
}
