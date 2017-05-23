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

package org.omg.dds.domain;

import org.omg.dds.core.EntityQos;
import org.omg.dds.core.policy.EntityFactory;
import org.omg.dds.core.policy.QosPolicy;
import org.omg.dds.core.policy.UserData;


public interface DomainParticipantQos
extends EntityQos<QosPolicy.ForDomainParticipant>
{
    /**
     * @return the userData QosPolicy
     */
    public UserData getUserData();

    /**
     * @return the entityFactory QosPolicy
     */
    public EntityFactory getEntityFactory();


    // --- Modification: -----------------------------------------------------

    @Override
    public DomainParticipantQos withPolicy(
            QosPolicy.ForDomainParticipant policy);

    @Override
    public DomainParticipantQos withPolicies(
            QosPolicy.ForDomainParticipant... policy);
}
