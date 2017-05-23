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

import java.io.Closeable;
import java.util.Collection;
import java.util.EventListener;
import java.util.Set;

import org.omg.dds.core.status.Status;


/**
 * This class is the abstract base class for all the DCPS objects that
 * support QoS policies, a listener and a status condition.
 *
 * @param <LISTENER>    The listener interface appropriate for this entity.
 * @param <QOS>         The QoS interface appropriate for this entity.
 */
public interface Entity<LISTENER extends EventListener,
                        QOS extends EntityQos<?>>
extends Closeable, DDSObject
{
    /**
     * This operation allows access to the existing Listener attached to the
     * Entity.
     * 
     * @return The attached listener if set, or null otherwise.
     *
     * @see #setListener(EventListener)
     * @see #setListener(EventListener, Collection)
     */
    public LISTENER getListener();

    /**
     * This operation installs a Listener on the Entity. The listener will only
     * be invoked on all communication statuses pertaining to the concrete type
     * of this entity.
     *
     * It is permitted to use null as the value of the listener. The null
     * listener behaves as a Listener whose operations perform no action.
     *
     * Only one listener can be attached to each Entity. If a listener was
     * already set, the operation will replace it with the new one. Consequently
     * if the value null is passed for the listener parameter, any existing
     * listener will be removed.
     * 
     * @param listener
     *            the listener to attach.
     *
     * @see #getListener()
     * @see #setListener(EventListener, Collection)
     */
    public void setListener(LISTENER listener);

    /**
     * This operation installs a Listener on the Entity. The listener will
     * only be invoked on the changes of communication status indicated by
     * the specified mask.
     *
     * It is permitted to use null as the value of the listener. The null
     * listener behaves as a Listener whose operations perform no action.
     *
     * Only one listener can be attached to each Entity. If a listener was
     * already set, the operation will replace it with the new one.
     * Consequently if the value null is passed for the listener parameter,
     * any existing listener will be removed.
     *
     * @see     #getListener()
     * @see     #setListener(EventListener)
     */
    public void setListener(
            LISTENER listener,
            Collection<Class<? extends Status>> statuses);

    /**
     * This operation installs a Listener on the Entity. The listener will
     * only be invoked on the changes of communication status indicated by
     * the specified mask.
     *
     * It is permitted to use null as the value of the listener. The null
     * listener behaves as a Listener whose operations perform no action.
     *
     * Only one listener can be attached to each Entity. If a listener was
     * already set, the operation will replace it with the new one.
     * Consequently if the value null is passed for the listener parameter,
     * any existing listener will be removed.
     *
     * @see     #getListener()
     * @see     #setListener(EventListener)
     */
    public void setListener(
            LISTENER listener,
            Class<? extends Status>... statuses);

    /**
     * This operation allows access to the existing set of QoS policies for
     * the Entity. This operation must be provided by each of the derived
     * Entity classes ({@link org.omg.dds.domain.DomainParticipant}, {@link org.omg.dds.topic.Topic},
     * {@link org.omg.dds.pub.Publisher}, {@link org.omg.dds.pub.DataWriter}, {@link org.omg.dds.sub.Subscriber},
     * {@link org.omg.dds.sub.DataReader}) so that the policies meaningful to the particular
     * Entity are retrieved.
     *
     * @see     #setQos(EntityQos)
     */
    public QOS getQos();

    /**
     * This operation is used to set the QoS policies of the Entity. This
     * operation must be provided by each of the derived Entity classes
     * ({@link org.omg.dds.domain.DomainParticipant}, {@link org.omg.dds.topic.Topic}, {@link org.omg.dds.pub.Publisher},
     * {@link org.omg.dds.pub.DataWriter}, {@link org.omg.dds.sub.Subscriber}, {@link org.omg.dds.sub.DataReader}) so that
     * the policies that are meaningful to each Entity can be set.
     *
     * The set of policies specified as the parameter are applied on top of
     * the existing QoS, replacing the values of any policies previously set.
     *
     * Certain policies are "immutable"; they can only be set at Entity
     * creation time, or before the entity is made enabled. If setQos is
     * invoked after the Entity is enabled and it attempts to change the
     * value of an "immutable" policy, the operation will fail with
     * {@link org.omg.dds.core.ImmutablePolicyException}.
     *
     * Certain values of QoS policies can be incompatible with the settings
     * of the other policies. The setQos operation will also fail if it
     * specifies a set of values that once combined with the existing values
     * would result in an inconsistent set of policies. In this case, it
     * shall fail with {@link org.omg.dds.core.InconsistentPolicyException}.
     *
     * If the application supplies a non-default value for a QoS policy that
     * is not supported by the implementation of the service, the setQos
     * operation will fail with {@link UnsupportedOperationException}.
     *
     * The existing set of policies are only changed if the setQos operation
     * succeeds. In all other cases, none of the policies is modified.
     *
     * @throws  ImmutablePolicyException    if an immutable policy changes
     *                                      its value.
     * @throws  InconsistentPolicyException if a combination of policies is
     *                                      inconsistent with one another.
     * @throws  UnsupportedOperationException   if an unsupported policy
     *                                          has a non-default value.
     *
     * @see     #getQos()
     */
    public void setQos(QOS qos);

    /**
     * This operation enables the Entity. Entity objects can be created
     * either enabled or disabled. This is controlled by the value of the
     * {@link org.omg.dds.core.policy.EntityFactory} on the corresponding factory for the
     * Entity.
     *
     * The default setting of {@link org.omg.dds.core.policy.EntityFactory} is such that, by
     * default, it is not necessary to explicitly call enable on newly
     * created entities.
     *
     * The enable operation is idempotent. Calling enable on an already
     * enabled Entity has no effect.
     *
     * If an Entity has not yet been enabled, the following kinds of
     * operations may be invoked on it:
     *
     * <ul>
     *     <li>Operations to set or get an Entity's QoS policies (including
     *         default QoS policies) and listener</li>
     *     <li>{@link #getStatusCondition()}</li>
     *     <li>'factory' operations and {@link #close()}</li>
     *     <li>{@link #getStatusChanges()} and other get status
     *         operations (although the status of a disabled entity never
     *         changes)</li>
     *     <li>'lookup' operations</li>
     * </ul>
     *
     * Other operations may explicitly state that they may be called on
     * disabled entities; those that do not will fail with
     *  {@link org.omg.dds.core.NotEnabledException}.
     *
     *  It is legal to delete an Entity that has not been enabled by calling
     *  {@link #close()}. Entities created from a factory that is disabled
     *  are created disabled regardless of the setting of
     *  {@link org.omg.dds.core.policy.EntityFactory}.
     *
     *  Calling enable on an Entity whose factory is not enabled will fail
     *  with {@link org.omg.dds.core.PreconditionNotMetException}.
     *
     *  If {@link org.omg.dds.core.policy.EntityFactory#isAutoEnableCreatedEntities()} is
     *  true, the enable operation on the factory will automatically enable
     *  all entities created from the factory.
     *
     *  The Listeners associated with an entity are not called until the
     *  entity is enabled. {@link org.omg.dds.core.Condition}s associated with an entity that
     *  is not enabled are "inactive," that is, have a triggerValue == false.
     */
    public void enable();

    /**
     * This operation allows access to the {@link org.omg.dds.core.StatusCondition} associated
     * with the Entity. The returned condition can then be added to a
     * {@link org.omg.dds.core.WaitSet} so that the application can wait for specific status
     * changes that affect the Entity.
     */
    public StatusCondition<?> getStatusCondition();

    /**
     * This operation retrieves the list of communication statuses in the
     * Entity that are 'triggered.' That is, the list of statuses whose value
     * has changed since the last time the application read the status.
     *
     * When the entity is first created or if the entity is not enabled, all
     * communication statuses are in the "untriggered" state so the list
     * returned will be empty.
     *
     * The list of statuses returned refers to the statuses that are
     * triggered on the Entity itself and does not include statuses that
     * apply to contained entities.
     *
     * @return  a new Set containing the triggered statuses.
     */
    public Set<Class<? extends Status>> getStatusChanges();

    /**
     * @return  the {@link org.omg.dds.core.InstanceHandle} that represents the Entity.
     */
    public InstanceHandle getInstanceHandle();

    /**
     * Halt communication and dispose the resources held by this Entity.
     *
     * Closing an Entity implicitly closes all of its contained objects, if
     * any. For example, closing a Publisher also closes all of its contained
     * DataWriters.
     *
     * An Entity cannot be closed if it has any unclosed dependent objects,
     * not including contained objects. These include the following:
     *
     * <ul>
     *     <li>A {@link org.omg.dds.topic.Topic} cannot be closed if it is still in use by any
     *         {@link org.omg.dds.topic.ContentFilteredTopic}s or {@link org.omg.dds.topic.MultiTopic}s.</li>
     *     <li>A Topic cannot be closed if any {@link org.omg.dds.pub.DataWriter}s or
     *         {@link org.omg.dds.sub.DataReader} is still using it.</li>
     *     <li>A DataReader cannot be closed if it has any outstanding loans
     *         as a result of a call to {@link org.omg.dds.sub.DataReader#read()},
     *         {@link org.omg.dds.sub.DataReader#take()}, or one of the variants thereof.
     *         </li>
     * </ul>
     *
     * The deletion of a {@link org.omg.dds.pub.DataWriter} will automatically unregister all
     * instances. Depending on the settings of the
     * {@link org.omg.dds.core.policy.WriterDataLifecycle}, the deletion of the DataWriter
     * may also dispose all instances.
     *
     * @throws  PreconditionNotMetException     if close is called on an
     *          Entity with unclosed dependent object(s), not including
     *          contained objects.
     *
     * @see     org.omg.dds.topic.TopicDescription#close()
     */
    @Override
    public abstract void close();

    /**
     * Indicates that references to this object may go out of scope but that
     * the application expects to look it up again later. Therefore, the
     * Service must consider this object to be still in use and may not
     * close it automatically.
     */
    public void retain();
}
