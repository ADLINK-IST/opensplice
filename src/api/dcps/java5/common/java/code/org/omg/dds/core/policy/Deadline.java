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
 * {@link org.omg.dds.sub.DataReader} expects a new sample updating the value of
 * each instance at least once every deadline period. The
 * {@link org.omg.dds.pub.DataWriter} indicates that the application commits to
 * write a new value (using the DataWriter) for each instance managed by the
 * DataWriter at least once every deadline period. It is inconsistent for a
 * DataReader to have a deadline period less than the result of its
 * {@link org.omg.dds.core.policy.TimeBasedFilter#getMinimumSeparation()}. The
 * default value of the deadline period is infinite.
 * <p>
 * <b>Concerns:</b> {@link org.omg.dds.topic.Topic},
 * {@link org.omg.dds.sub.DataReader}, {@link org.omg.dds.pub.DataWriter}
 * <p>
 * <b>RxO:</b> Yes
 * <p>
 * <b>Changeable:</b> Yes
 * <p>
 * This policy is useful for cases where a {@link org.omg.dds.topic.Topic} is
 * expected to have each instance updated periodically. On the publishing side
 * this setting establishes a contract that the application must meet. On the
 * subscribing side the setting establishes a minimum requirement for the remote
 * publishers that are expected to supply the data values.
 * <p>
 * When the Service "matches" a DataWriter and a DataReader it checks whether
 * the settings are compatible (i.e., offered deadline period &lt;= requested
 * deadline period). If they are not, the two entities are informed (via the
 * listener or {@link org.omg.dds.core.Condition} mechanism) of the
 * incompatibility of the QoS settings and communication will not occur.
 * <p>
 * The exact consequences of a missed deadline depend on the Entity in which it
 * occurred, and the OwnershipQosPolicy value of that Entity:
 * <p>
 * In case a DataWriter misses an instance deadline
 * (regardless of its OwnershipQosPolicy setting), an offered_deadline_missed_status
 * is raised, which can be detected by either a Listener or a StatusCondition.
 * There are no further consequences.
 * <p>
 * In case a DataReader misses an instance deadline, a {@link org.omg.dds.core.status.RequestedDeadlineMissedStatus}
 * is raised, which can be detected by either a Listener or a StatusCondition.
 * In case the OwnershipQosPolicy is set to SHARED, there are no further consequences.
 * In case the OwnershipQosPolicy is set to EXCLUSIVE, the ownership of that instance
 * on that particular DataReader is transferred to the next available highest strength
 * DataWriter, but this will have no impact on the instance_state  whatsoever.
 * So even when a deadline is missed for an instance that has no other (lower-strength)
 * DataWriters to transfer ownership to, the instance_state remains unchanged.
 * See also {@link org.omg.dds.core.policy.Ownership}.
 * <p>
 * The value offered is considered compatible with the value requested if and
 * only if the inequality "offered deadline period &lt;= requested deadline
 * period" evaluates to true.
 * <p>
 * Changing an existing deadline period using the set_qos operation on either the
 * DataWriter or DataReader may have consequences for the connectivity between
 * readers and writers, depending on their RxO values. Consider a writer with
 * deadline period <b>Pw</b> and a reader with deadline period <b>Pr</b>,
 * where <b>Pw &lt;= Pr</b>. In this case a connection between that reader and that
 * writer is established. Now suppose <b>Pw</b> is changed so that <b>Pw&gt;Pr</b>,
 * then the existing connection between reader and writer will be lost, and the
 * reader will behave as if the writer unregistered all its instances, transferring
 * the ownership of these instances when appropriate. See also {@link org.omg.dds.core.policy.Ownership}.
 * <p>
 * The setting of the DEADLINE policy must be set consistently with that of the
 * {@link org.omg.dds.core.policy.TimeBasedFilter}. For these two policies to be
 * consistent the settings must be such that "deadline period &gt;=
 * minimum_separation."
 */
public interface Deadline
extends QosPolicy.ForTopic,
        QosPolicy.ForDataReader,
        QosPolicy.ForDataWriter,
        RequestedOffered<Deadline>
{
    public Duration getPeriod();


    // --- Modification: -----------------------------------------------------

    /**
     * Copy this policy and override the value of the property.
     * @param period                 Specifies a duration within which a new sample is expected or to be written.
     *
     * @return  a new Deadline policy
     */
    public Deadline withPeriod(Duration period);

    /**
     * Copy this policy and override the value of the property.
     * @param period                 Specifies the period within which a new sample is expected or to be written.
     *
     * @param unit                   The TimeUnit which the period describes (i.e. TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * @return  a new Deadline policy
     */
    public Deadline withPeriod(long period, TimeUnit unit);
}
