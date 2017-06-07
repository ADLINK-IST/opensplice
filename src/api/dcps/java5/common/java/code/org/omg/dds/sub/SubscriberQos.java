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

package org.omg.dds.sub;

import org.omg.dds.core.EntityQos;
import org.omg.dds.core.policy.EntityFactory;
import org.omg.dds.core.policy.GroupData;
import org.omg.dds.core.policy.Partition;
import org.omg.dds.core.policy.Presentation;
import org.omg.dds.core.policy.QosPolicy;


public interface SubscriberQos
extends EntityQos<QosPolicy.ForSubscriber>
{
    /**
     * @return the presentation QosPolicy
     */
    public Presentation getPresentation();

    /**
     * @return the partition QosPolicy
     */
    public Partition getPartition();

    /**
     * @return the groupData QosPolicy
     */
    public GroupData getGroupData();

    /**
     * @return the entityFactory QosPolicy
     */
    public EntityFactory getEntityFactory();


    // --- Modification: -----------------------------------------------------

    @Override
    public SubscriberQos withPolicy(QosPolicy.ForSubscriber policy);

    @Override
    public SubscriberQos withPolicies(QosPolicy.ForSubscriber... policy);
}
