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

import java.util.List;


/**
 * QueryCondition objects are specialized {@link org.omg.dds.sub.ReadCondition} objects that
 * allow the application to also specify a filter on the locally available
 * data.
 * <p>
 * The query (queryExpression) is similar to an SQL WHERE clause can be
 * parameterized by arguments that are dynamically changeable by the
 * {@link #setQueryParameters(List)} operation.
 * <p>
 * This feature is optional. In the cases where it is not supported, the
 * {@link org.omg.dds.sub.DataReader#createQueryCondition(String, List)} will return null.
 * <p>
 * The triggerValue of a QueryCondition is like that of a ReadCondition with
 * the additional condition that the data associated with at least one sample
 * must be such that the queryExpression evaluates to true.
 * <p>
 * Example:
 * <pre>
 * <code>
 * // Create a QueryCondition with a DataState with a sample state of not read, new view state and alive instance state.
 * // And apply a condition that only samples with age &gt; 0 and name equals BILL are shown.
 * DataState ds = subscriber.createDataState();
 * ds = ds.with(SampleState.NOT_READ)
 *        .with(ViewState.NEW)
 *        .with(InstanceState.ALIVE);
 * String expr = "age &gt; %0 AND name = %1");
 * List&lt;String&gt; params = new ArrayList&lt;String&gt;();
 * params.add("0");
 * params.add("BILL");
 * QueryCondition&lt;Foo&gt; queryCond = fooDR.createQueryCondition(ds,expr,params);
 * //Instantiate a WaitSet and attach the condition.
 * WaitSet myWS = WaitSet.newWaitSet(env);
 * myWS.attachCondition(queryCond);
 * Duration waitTimeout = Duration.newDuration(60, TimeUnit.SECONDS, env);
 * // Wait for the querycondition to trigger.
 * try
 * {
 *     myWS.waitForConditions(waitTimeout);
 *     List&lt;Sample&lt;Foo&gt;&gt; samples = new ArrayList&lt;Sample&lt;Foo&gt;&gt;();
 *     fooDR.select().dataState(ds).Content(expr,params).read(samples);
 * }
 * catch (TimeoutException e) {}
 * </code>
 * </pre>
 *
 * @param <TYPE>    The concrete type of the data that can be read using the
 *                  the {@link org.omg.dds.sub.DataReader} that created this QueryCondition.
 */
public interface QueryCondition<TYPE> extends ReadCondition<TYPE> {
    /**
     * This operation returns the queryExpression associated with the
     * QueryCondition. That is, the expression specified when the
     * QueryCondition was created.
     * @see     #getQueryParameters()
     */
    public String getQueryExpression();

    /**
     * This operation returns the queryParameters associated with the
     * QueryCondition. That is, the parameters specified on the last
     * successful call to {@link #setQueryParameters(List)}, or if
     * {@link #setQueryParameters(List)} was never called, the arguments
     * specified when the QueryCondition was created.
     *
     * @return  an unmodifiable list of the current query parameters.
     * @see     #setQueryParameters(List)
     * @see     #getQueryExpression()
     */
    public List<String> getQueryParameters();

    /**
     * This operation changes the queryParameters associated with the
     * QueryCondition.
     *
     * @param   queryParameters a container, into which this method will
     *          place its result.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #getQueryParameters()
     */
    public void setQueryParameters(List<String> queryParameters);

    /**
     * This operation changes the queryParameters associated with the
     * QueryCondition.
     *
     * @param   queryParameters a container, into which this method will
     *          place its result.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #getQueryParameters()
     */
    public void setQueryParameters(String... queryParameters);
}
