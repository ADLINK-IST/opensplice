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

import java.util.List;

/**
 * MultiTopic is a specialization of TopicDescription that allows subscriptions
 * to combine/filter/rearrange data coming from several
 * {@link org.omg.dds.topic.Topic}s.
 * <p>
 * MultiTopic allows a more sophisticated subscription that can select and
 * combine data received from multiple topics into a single resulting type
 * (specified by the inherited typeName). The data will then be filtered
 * (selection) and possibly rearranged (aggregation/projection) according to a
 * subscriptionExpression with parameters expressionParameters.
 *
 * <ul>
 * <li>The subscriptionExpression is a string that identifies the selection and
 * rearrangement of data from the associated topics. It is similar to an SQL
 * clause where the SELECT part provides the fields to be kept, the FROM part
 * provides the names of the topics that are searched for those fields, and the
 * WHERE clause gives the content filter. The Topics combined may have different
 * types but they are restricted in that the type of the fields used for the
 * NATURAL JOIN operation must be the same.</li>
 * <li>The expressionParameters attribute is a sequence of strings that give
 * values to the "parameters" (i.e., "%n" tokens) in the subscriptionExpression.
 * The number of supplied parameters must fit with the requested values in the
 * subscriptionExpression (i.e., the number of "%n" tokens).</li>
 * <li>{@link org.omg.dds.sub.DataReader} entities associated with a MultiTopic
 * are alerted of data modifications by the usual Listener or
 * {@link org.omg.dds.core.Condition} mechanisms whenever modifications occur to
 * the data associated with any of the topics relevant to the MultiTopic.</li>
 * <li>DataReader entities associated with a MultiTopic access instances that
 * are "constructed" at the DataReader side from the instances written by
 * multiple DataWriter entities. The MultiTopic access instance will begin to
 * exist as soon as all the constituting Topic instances are in existence. The
 * {@link org.omg.dds.sub.ViewState} and {@link org.omg.dds.sub.InstanceState}
 * are computed from the corresponding states of the constituting instances:
 * <ul>
 * <li>The viewState of the MultiTopic instance is NEW if at least one of the
 * constituting instances has viewState = NEW, otherwise it will be NOT_NEW.</li>
 * <li>The instanceState of the MultiTopic instance is "ALIVE" if the
 * instanceStates of all the constituting Topic instances are ALIVE. It is
 * "NOT_ALIVE_DISPOSED" if at least one of the constituting Topic instances is
 * NOT_ALIVE_DISPOSED. Otherwise, it is NOT_ALIVE_NO_WRITERS.</li>
 * </ul>
 * </li>
 * </ul>
 * <b>NOTE: MultiTopic is currently not supported by OpenSplice!</b>
 *
 * @param <TYPE>
 *            The concrete type of the data that will be published and/ or
 *            subscribed by the readers and writers that use this topic
 *            description.
 */
public interface MultiTopic<TYPE> extends TopicDescription<TYPE> {
    /**
     * @return the subscription expression associated with the MultiTopic, that
     *         is, the expression specified when the MultiTopic was created.
     */
    public String getSubscriptionExpression();

    /**
     * This operation returns the expression parameters associated with the
     * MultiTopic, that is, the parameters specified on the last successful call
     * to {@link #setExpressionParameters(List)}, or if
     * {@link #setExpressionParameters(List)} was never called, the parameters
     * specified when the MultiTopic was created.
     *
     * @return an unmodifiable list.
     *
     * @see #setExpressionParameters(List)
     */
    public List<String> getExpressionParameters();

    /**
     * This operation changes the expression parameters associated with the
     * MultiTopic.
     * @param expressionParameters  The expressionParameters attribute is a collection of strings that give
     *                              values to the "parameters" (i.e., "%n" tokens) in the queryExpression.
     *                              The number of supplied parameters must fit with the requested values
     *                              in the queryExpression (i.e., the number of "%n" tokens).
     * @see #getExpressionParameters()
     */
    public void setExpressionParameters(List<String> expressionParameters);

    /**
     * This operation changes the expression parameters associated with the
     * MultiTopic.
     * @param expressionParameters  The expressionParameters attribute is a sequence of strings that give
     *                              values to the "parameters" (i.e., "%n" tokens) in the queryExpression.
     *                              The number of supplied parameters must fit with the requested values
     *                              in the queryExpression (i.e., the number of "%n" tokens).
     * @see #getExpressionParameters()
     */
    public void setExpressionParameters(String... expressionParameters);
}
