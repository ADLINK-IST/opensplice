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

/**
 * A Condition is a root interface for all the conditions that may be
 * attached to a {@link org.omg.dds.core.WaitSet}. This basic interface is specialized in the
 * following interfaces that are known by the middleware:
 * {@link org.omg.dds.core.GuardCondition}, {@link org.omg.dds.core.StatusCondition}, and
 * {@link org.omg.dds.sub.ReadCondition}.
 * <p>
 * A Condition has a triggerValue that can be true or false and is set
 * automatically by the Service.
 */
public interface Condition extends DDSObject {
    /**
     * @return  the triggerValue of the Condition.
     */
    public boolean getTriggerValue();
}
