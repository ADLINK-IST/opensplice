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



/**
 * Specifies how the samples representing changes to data instances are
 * presented to the subscribing application. This policy affects the
 * application's ability to specify and receive coherent changes and to see the
 * relative order of changes. The accessScope determines the largest scope
 * spanning the entities for which the order and coherency of changes can be
 * preserved. The two booleans control whether coherent access and ordered
 * access are supported within the scope accessScope.
 *<p>
 * <b>Concerns:</b> {@link org.omg.dds.pub.Publisher},
 * {@link org.omg.dds.sub.Subscriber}
 *<p>
 * <b>RxO:</b> Yes
 *<p>
 * <b>Changeable:</b> No
 *<p>
 * This QoS policy controls the extent to which changes to data instances can be
 * made dependent on each other and also the kind of dependencies that can be
 * propagated and maintained by the Service.
 * <p>
 * The setting of coherentAccess controls whether the Service will preserve the
 * groupings of changes made by the publishing application by means of the
 * operations {@link org.omg.dds.pub.Publisher#beginCoherentChanges()} and
 * {@link org.omg.dds.pub.Publisher#endCoherentChanges()}.
 * <p>
 * The setting of orderedAccess controls whether the Service will preserve the
 * order of changes.
 * <p>
 * The granularity is controlled by the setting of the accessScope.
 * <p>
 * Note that this QoS policy controls the scope at which related changes are
 * made available to the subscriber. This means the subscriber <em>can</em>
 * access the changes in a coherent manner and in the proper order; however, it
 * does not necessarily imply that the Subscriber <em>will</em> indeed access
 * the changes in the correct order. For that to occur, the application at the
 * subscriber end must use the proper logic in reading the DataReader objects.
 * <p>
 * The value offered is considered compatible with the value requested if and
 * only if the following conditions are met:
 *
 * <ol>
 * <li>The inequality "offered access_scope &gt;= requested access_scope"
 * evaluates to true. For the purposes of this inequality, the values of
 * {@link Presentation.AccessScopeKind} are considered ordered such that
 * INSTANCE &lt; TOPIC &lt; GROUP.</li>
 * <li>Requested coherentAccess is false, or else both offered and requested
 * coherentAccess are true.</li>
 * <li>Requested orderedAccess is false, or else both offered and requested
 * orderedAccess are true.</li>
 * </ol>
 */
public interface Presentation
extends QosPolicy.ForPublisher,
        QosPolicy.ForSubscriber,
        RequestedOffered<Presentation>
{
    // -----------------------------------------------------------------------
    // Methods
    // -----------------------------------------------------------------------

    /**
     * @return the accessScope
     */
    public AccessScopeKind getAccessScope();

    /**
     * If coherentAccess is set, then the accessScope controls the maximum
     * extent of coherent changes. The behavior is as follows:
     *
     * <ul>
     *      <li>If accessAcope is set to
     *          {@link Presentation.AccessScopeKind#INSTANCE}, the
     *          use of {@link org.omg.dds.pub.Publisher#beginCoherentChanges()} and
     *          {@link org.omg.dds.pub.Publisher#endCoherentChanges()} has no effect on how
     *          the subscriber can access the data because with the scope
     *          limited to each instance, changes to separate instances are
     *          considered independent and thus cannot be grouped by a
     *          coherent change.
     *      <li>If accessScope is set to
     *          {@link Presentation.AccessScopeKind#TOPIC}, then
     *          coherent changes (indicated by their enclosure within calls to
     *          {@link org.omg.dds.pub.Publisher#beginCoherentChanges()} and
     *          {@link org.omg.dds.pub.Publisher#endCoherentChanges()}) will be made available
     *          as such to each remote {@link org.omg.dds.sub.DataReader} independently. That
     *          is, changes made to instances within each individual
     *          {@link org.omg.dds.pub.DataWriter} will be available as coherent with respect
     *          to other changes to instances in that same DataWriter, but
     *          will not be grouped with changes made to instances belonging
     *          to a different DataWriter.
     *      <li>If accessScope is set to
     *          {@link Presentation.AccessScopeKind#GROUP}, then
     *          coherent changes made to instances through a DataWriter
     *          attached to a common {@link org.omg.dds.pub.Publisher} are made available as
     *          a unit to remote subscribers.</li>
     * </ul>
     *
     * @see #getAccessScope()
     */
    public boolean isCoherentAccess();

    /**
     * If orderedAccess is set, then the accessScope controls the maximum
     * extent for which order will be preserved by the Service.
     *
     * <ul>
     *  <li>If accessScope is set to
     *      {@link Presentation.AccessScopeKind#INSTANCE} (the
     *      lowest level), then changes to each instance are considered
     *      unordered relative to changes to any other instance. That means
     *      that changes (creations, deletions, modifications) made to two
     *      instances are not necessarily seen in the order they occur. This
     *      is the case even if it is the same application thread making the
     *      changes using the same {@link org.omg.dds.pub.DataWriter}.</li>
     *  <li>If accessScope is set to
     *      {@link Presentation.AccessScopeKind#TOPIC}, changes
     *      (creations, deletions, modifications) made by a single
     *      {@link org.omg.dds.pub.DataWriter} are made available to subscribers in the same
     *      order they occur. Changes made to instances through different
     *      DataWriter entities are not necessarily seen in the order they
     *      occur. This is the case, even if the changes are made by a single
     *      application thread using DataWriter objects attached to the same
     *      {@link org.omg.dds.pub.Publisher}.</li>
     *  <li>Finally, if accessScope is set to
     *      {@link Presentation.AccessScopeKind#GROUP}, changes made
     *      to instances via DataWriter entities attached to the same
     *      Publisher object are made available to subscribers on the same
     *      order they occur.</li>
     * </ul>
     *
     * @see #getAccessScope()
     */
    public boolean isOrderedAccess();


    // --- Modification: -----------------------------------------------------

    /**
     * Copy this policy and override the value of the property.
     * @param accessScope       Specifies the granularity of the changes that needs to be preserved when communicating a set
     *                          of samples and the granularity of the ordering in which these changes need to be presented to the user.
     *
     * @return  a new Presentation policy
     */
    public Presentation withAccessScope(AccessScopeKind accessScope);

    /**
     * Copy this policy and override the value of the property.
     * @param coherentAccess    Controls whether the Data Distribution Service will preserve the groupings of changes, as indicated by
     *                          the access_scope, made by a publishing application by means of the operations begin_coherent_change
     *                          and end_coherent_change.
     *
     * @return  a new Presentation policy
     */
    public Presentation withCoherentAccess(boolean coherentAccess);

    /**
     * Copy this policy and override the value of the property.
     * @param orderedAccess     Controls whether the Data Distribution Service will preserve the order of the changes, as indicated
     *                          by the access_scope.
     *
     * @return  a new Presentation policy
     */
    public Presentation withOrderedAccess(boolean orderedAccess);

    /**
    * @return  a new instance Presentation policy
    */
    public Presentation withInstance();
    /**
     * @return  a new topic Presentation policy
     */
    public Presentation withTopic();
    /**
     * @return  a new group Presentation policy
     */
    public Presentation withGroup();

    // -----------------------------------------------------------------------
    // Types
    // -----------------------------------------------------------------------

    public enum AccessScopeKind {
        /**
         * Scope spans only a single instance. Indicates that changes to one
         * instance need not be coherent nor ordered with respect to changes
         * to any other instance. In other words, order and coherent changes
         * apply to each instance separately. This is the default accessScope.
         */
        INSTANCE,

        /**
         * Scope spans to all instances within the same {@link org.omg.dds.pub.DataWriter}
         * (or {@link org.omg.dds.sub.DataReader}), but not across instances in different
         * DataWriter (or DataReader).
         */
        TOPIC,

        /**
         * [optional] Scope spans to all instances belonging to
         * {@link org.omg.dds.pub.DataWriter} (or {@link org.omg.dds.sub.DataReader}) entities within the
         * same {@link org.omg.dds.pub.Publisher} (or {@link org.omg.dds.sub.Subscriber}).
         */
        GROUP
    }

}
