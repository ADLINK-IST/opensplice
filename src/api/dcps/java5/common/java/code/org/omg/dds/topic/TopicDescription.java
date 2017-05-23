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

package org.omg.dds.topic;

import java.io.Closeable;

import org.omg.dds.core.DDSObject;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.type.TypeSupport;


/**
 * This interface is the base for {@link org.omg.dds.topic.Topic}, {@link org.omg.dds.topic.ContentFilteredTopic},
 * and {@link org.omg.dds.topic.MultiTopic}.
 * <p>
 * TopicDescription represents the fact that both publications and
 * subscriptions are tied to a single data type. Its attribute typeName
 * defines a unique resulting type for the publication or the subscription
 * and therefore creates an implicit association with a {@link org.omg.dds.type.TypeSupport}.
 * TopicDescription has also a name that allows it to be retrieved locally.
 *
 * @param <TYPE>    The concrete type of the data that will be published and/
 *                  or subscribed by the readers and writers that use this
 *                  topic description.
 */
public interface TopicDescription<TYPE> extends Closeable, DDSObject
{
    /**
     * Returns the {@link org.omg.dds.type.TypeSupport} used to create this TopicDescription.
     * @return  the {@link org.omg.dds.type.TypeSupport}.
     */
    public TypeSupport<TYPE> getTypeSupport();

    /**
     * Cast this topic description to the given type, or throw an exception if
     * the cast fails.
     *
     * @param <OTHER>   The type of the data exchanged on this topic,
     *                  according to the caller.
     * @return          this topic description
     * @throws          ClassCastException if the cast fails
     */
    public <OTHER> TopicDescription<OTHER> cast();

    /**
     * Returns the type name used to create the TopicDescription.
     * @return  the type name
     */
    public String getTypeName();

    /**
     * Returns the name used to create the TopicDescription.
     * @return  the name.
     */
    public String getName();

    /**
     * Returns the {@link org.omg.dds.domain.DomainParticipant} to which the TopicDescription belongs.
     * @return  the {@link org.omg.dds.domain.DomainParticipant}
     */
    public DomainParticipant getParent();

    /**
     * Dispose the resources held by this object.
     * <p>
     * A TopicDescription cannot be closed if it is in use by any
     * {@link org.omg.dds.pub.DataWriter}s or {@link org.omg.dds.sub.DataReader}s. With respect to
     * {@link org.omg.dds.topic.Topic}s specifically: a Topic cannot be closed if it has any
     * remaining {@link org.omg.dds.topic.ContentFilteredTopic}s or {@link org.omg.dds.topic.MultiTopic}s related
     * to it.
     *
     * @see     org.omg.dds.core.Entity#close()
     */
    @Override
    public void close();
}
