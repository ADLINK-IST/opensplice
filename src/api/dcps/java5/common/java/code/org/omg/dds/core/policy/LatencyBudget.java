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

package org.omg.dds.core.policy;

import java.util.concurrent.TimeUnit;

import org.omg.dds.core.Duration;


/**
 * Specifies the maximum acceptable delay from the time the data is written
 * until the data is inserted in the receiver's application cache and the
 * receiving application is notified of the fact. This policy is a hint to the
 * Service, not something that must be monitored or enforced. The Service is not
 * required to track or alert the user of any violation. The default value of
 * the duration is zero indicating that the delay should be minimized.
 *<p>
 * <b>Concerns:</b> {@link org.omg.dds.topic.Topic},
 * {@link org.omg.dds.sub.DataReader}, {@link org.omg.dds.pub.DataWriter}
 *<p>
 * <b>RxO:</b> Yes
 *<p>
 * <b>Changeable:</b> Yes
 *<p>
 * This policy provides a means for the application to indicate to the
 * middleware the "urgency" of the data communication. By having a non-zero
 * duration the Service can optimize its internal operation.
 * <p>
 * This policy is considered a hint. There is no specified mechanism as to how
 * the service should take advantage of this hint.
 * <p>
 * The value offered is considered compatible with the value requested if and
 * only if the inequality "offered duration &lt;= requested duration" evaluates
 * to true.
 */
public interface LatencyBudget
extends QosPolicy.ForTopic,
        QosPolicy.ForDataReader,
        QosPolicy.ForDataWriter,
        RequestedOffered<LatencyBudget>
{
    public Duration getDuration();


    // --- Modification: -----------------------------------------------------

    /**
     * Copy this policy and override the value of the property.
     * @param duration               Specifies a duration for the maximum acceptable additional delay
     *                               from the time the data is written until the data is delivered.
     *
     * @return  a new LatencyBudget policy
     */
    public LatencyBudget withDuration(Duration duration);

    /**
     * Copy this policy and override the value of the property.
     * @param duration               Specifies a long duration for the maximum acceptable additional delay
     *                               from the time the data is written until the data is delivered.
     * @param unit                   The TimeUnit which the duration describes (i.e. TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     *
     * @return  a new policy
     */
    public LatencyBudget withDuration(long duration, TimeUnit unit);
}
