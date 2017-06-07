/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
package org.opensplice.dds.sub;

import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.omg.dds.core.Duration;
import org.omg.dds.core.Time;
import org.omg.dds.core.policy.ResourceLimits;

public interface DataReader<TYPE> extends org.omg.dds.sub.DataReader<TYPE> {
    /**
     * This operation will block the application thread until all historical
     * data that matches the supplied conditions is received.
     * <p>
     * This operation only makes sense when the receiving node has configured
     * its durability service as an On_Request alignee. (See also the
     * description of the
     * //OpenSplice/DurabilityService/NameSpaces/Policy[@alignee] attribute in
     * the Deployment Guide.) Otherwise the Durability Service will not
     * distinguish between separate reader requests and still inject the full
     * historical data set in each reader.
     * <p>
     * Additionally, when creating the DataReader, the DurabilityQos.kind of the
     * DataReaderQos needs to be set to
     * {@link org.omg.dds.core.policy.Durability.Kind#VOLATILE} to ensure that
     * historical data that potentially is available already at creation time is
     * not immediately delivered to the DataReader at that time.
     * 
     * @param filterExpression
     *            the SQL expression (subset of SQL), which defines the
     *            filtering criteria (null when no SQL filtering is needed).
     * @param filterParameters
     *            sequence of strings with the parameter values used in the SQL
     *            expression (i.e., the number of %n tokens in the expression).
     *            The number of values in filterParameters must be equal to or
     *            greater than the highest referenced %n token in the
     *            filterExpression (e.g. if %1 and %8 are used as parameters in
     *            the filterExpression, the filterParameters should contain at
     *            least n + 1 = 9 values).
     * @param minSourceTimestamp
     *            Filter out all data published before this time. The special
     *            org.omg.dds.core.Time.invalidTime() can be used when no
     *            minimum filter is needed.
     * @param maxSourceTimestamp
     *            Filter out all data published after this time. The special
     *            org.omg.dds.core.Time.invalidTime() can be used when no
     *            maximum filter is needed
     * @param resourceLimits
     *            Specifies limits on the maximum amount of historical data that
     *            may be received.
     * @param maxWait
     *            The maximum duration the application thread is blocked during
     *            this operation.
     * 
     * @throws TimeoutException
     *             thrown when maxWait has expired before the applicable
     *             historical data has successfully been obtained.
     * @throws org.omg.dds.core.DDSException
     *             An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *             The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             The Data Distribution Service ran out of resources to
     *             complete this operation.
     */
    public void waitForHistoricalData(String filterExpression,
            List<String> filterParameters, Time minSourceTimestamp,
            Time maxSourceTimestamp, ResourceLimits resourceLimits,
            Duration maxWait) throws TimeoutException;

    /**
     * This operation will block the application thread until all historical
     * data that matches the supplied conditions is received.
     * <p>
     * This operation only makes sense when the receiving node has configured
     * its durability service as an On_Request alignee. (See also the
     * description of the
     * //OpenSplice/DurabilityService/NameSpaces/Policy[@alignee] attribute in
     * the Deployment Guide.) Otherwise the Durability Service will not
     * distinguish between separate reader requests and still inject the full
     * historical data set in each reader.
     * <p>
     * Additionally, when creating the DataReader, the DurabilityQos.kind of the
     * DataReaderQos needs to be set to
     * {@link org.omg.dds.core.policy.Durability.Kind#VOLATILE} to ensure that
     * historical data that potentially is available already at creation time is
     * not immediately delivered to the DataReader at that time.
     * 
     * @param filterExpression
     *            the SQL expression (subset of SQL), which defines the
     *            filtering criteria (null when no SQL filtering is needed).
     * @param filterParameters
     *            sequence of strings with the parameter values used in the SQL
     *            expression (i.e., the number of %n tokens in the expression).
     *            The number of values in filterParameters must be equal to or
     *            greater than the highest referenced %n token in the
     *            filterExpression (e.g. if %1 and %8 are used as parameters in
     *            the filterExpression, the filterParameters should contain at
     *            least n + 1 = 9 values).
     * @param minSourceTimestamp
     *            Filter out all data published before this time. The special
     *            org.omg.dds.core.Time.invalidTime() can be used when no
     *            minimum filter is needed.
     * @param maxSourceTimestamp
     *            Filter out all data published after this time. The special
     *            org.omg.dds.core.Time.invalidTime() can be used when no
     *            maximum filter is needed
     * @param maxWait
     *            The maximum duration the application thread is blocked during
     *            this operation.
     * 
     * @throws TimeoutException
     *             thrown when maxWait has expired before the applicable
     *             historical data has successfully been obtained.
     * @throws org.omg.dds.core.DDSException
     *             An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *             The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             The Data Distribution Service ran out of resources to
     *             complete this operation.
     */
    public void waitForHistoricalData(String filterExpression,
            List<String> filterParameters, Time minSourceTimestamp,
            Time maxSourceTimestamp, Duration maxWait) throws TimeoutException;

    /**
     * This operation will block the application thread until all historical
     * data that matches the supplied conditions is received.
     * <p>
     * This operation only makes sense when the receiving node has configured
     * its durability service as an On_Request alignee. (See also the
     * description of the
     * //OpenSplice/DurabilityService/NameSpaces/Policy[@alignee] attribute in
     * the Deployment Guide.) Otherwise the Durability Service will not
     * distinguish between separate reader requests and still inject the full
     * historical data set in each reader.
     * <p>
     * Additionally, when creating the DataReader, the DurabilityQos.kind of the
     * DataReaderQos needs to be set to
     * {@link org.omg.dds.core.policy.Durability.Kind#VOLATILE} to ensure that
     * historical data that potentially is available already at creation time is
     * not immediately delivered to the DataReader at that time.
     * 
     * @param filterExpression
     *            the SQL expression (subset of SQL), which defines the
     *            filtering criteria (null when no SQL filtering is needed).
     * @param filterParameters
     *            sequence of strings with the parameter values used in the SQL
     *            expression (i.e., the number of %n tokens in the expression).
     *            The number of values in filterParameters must be equal to or
     *            greater than the highest referenced %n token in the
     *            filterExpression (e.g. if %1 and %8 are used as parameters in
     *            the filterExpression, the filterParameters should contain at
     *            least n + 1 = 9 values).
     * @param resourceLimits
     *            Specifies limits on the maximum amount of historical data that
     *            may be received.
     * @param maxWait
     *            The maximum duration the application thread is blocked during
     *            this operation.
     * 
     * @throws TimeoutException
     *             thrown when maxWait has expired before the applicable
     *             historical data has successfully been obtained.
     * @throws org.omg.dds.core.DDSException
     *             An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *             The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             The Data Distribution Service ran out of resources to
     *             complete this operation.
     */
    public void waitForHistoricalData(String filterExpression,
            List<String> filterParameters, ResourceLimits resourceLimits,
            Duration maxWait) throws TimeoutException;

    /**
     * This operation will block the application thread until all historical
     * data that matches the supplied conditions is received.
     * <p>
     * This operation only makes sense when the receiving node has configured
     * its durability service as an On_Request alignee. (See also the
     * description of the
     * //OpenSplice/DurabilityService/NameSpaces/Policy[@alignee] attribute in
     * the Deployment Guide.) Otherwise the Durability Service will not
     * distinguish between separate reader requests and still inject the full
     * historical data set in each reader.
     * <p>
     * Additionally, when creating the DataReader, the DurabilityQos.kind of the
     * DataReaderQos needs to be set to
     * {@link org.omg.dds.core.policy.Durability.Kind#VOLATILE} to ensure that
     * historical data that potentially is available already at creation time is
     * not immediately delivered to the DataReader at that time.
     * 
     * @param filterExpression
     *            the SQL expression (subset of SQL), which defines the
     *            filtering criteria (null when no SQL filtering is needed).
     * @param filterParameters
     *            sequence of strings with the parameter values used in the SQL
     *            expression (i.e., the number of %n tokens in the expression).
     *            The number of values in filterParameters must be equal to or
     *            greater than the highest referenced %n token in the
     *            filterExpression (e.g. if %1 and %8 are used as parameters in
     *            the filterExpression, the filterParameters should contain at
     *            least n + 1 = 9 values).
     * @param maxWait
     *            The maximum duration the application thread is blocked during
     *            this operation.
     * 
     * @throws TimeoutException
     *             thrown when maxWait has expired before the applicable
     *             historical data has successfully been obtained.
     * @throws org.omg.dds.core.DDSException
     *             An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *             The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             The Data Distribution Service ran out of resources to
     *             complete this operation.
     */
    public void waitForHistoricalData(String filterExpression,
            List<String> filterParameters, Duration maxWait)
            throws TimeoutException;

    /**
     * This operation will block the application thread until all historical
     * data that matches the supplied conditions is received.
     * <p>
     * This operation only makes sense when the receiving node has configured
     * its durability service as an On_Request alignee. (See also the
     * description of the
     * //OpenSplice/DurabilityService/NameSpaces/Policy[@alignee] attribute in
     * the Deployment Guide.) Otherwise the Durability Service will not
     * distinguish between separate reader requests and still inject the full
     * historical data set in each reader.
     * <p>
     * Additionally, when creating the DataReader, the DurabilityQos.kind of the
     * DataReaderQos needs to be set to
     * {@link org.omg.dds.core.policy.Durability.Kind#VOLATILE} to ensure that
     * historical data that potentially is available already at creation time is
     * not immediately delivered to the DataReader at that time.
     * 
     * @param minSourceTimestamp
     *            Filter out all data published before this time. The special
     *            org.omg.dds.core.Time.invalidTime() can be used when no
     *            minimum filter is needed.
     * @param maxSourceTimestamp
     *            Filter out all data published after this time. The special
     *            org.omg.dds.core.Time.invalidTime() can be used when no
     *            maximum filter is needed
     * @param resourceLimits
     *            Specifies limits on the maximum amount of historical data that
     *            may be received.
     * @param maxWait
     *            The maximum duration the application thread is blocked during
     *            this operation.
     * 
     * @throws TimeoutException
     *             thrown when maxWait has expired before the applicable
     *             historical data has successfully been obtained.
     * @throws org.omg.dds.core.DDSException
     *             An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *             The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             The Data Distribution Service ran out of resources to
     *             complete this operation.
     */
    public void waitForHistoricalData(Time minSourceTimestamp,
            Time maxSourceTimestamp, ResourceLimits resourceLimits,
            Duration maxWait) throws TimeoutException;

    /**
     * This operation will block the application thread until all historical
     * data that matches the supplied conditions is received.
     * <p>
     * This operation only makes sense when the receiving node has configured
     * its durability service as an On_Request alignee. (See also the
     * description of the
     * //OpenSplice/DurabilityService/NameSpaces/Policy[@alignee] attribute in
     * the Deployment Guide.) Otherwise the Durability Service will not
     * distinguish between separate reader requests and still inject the full
     * historical data set in each reader.
     * <p>
     * Additionally, when creating the DataReader, the DurabilityQos.kind of the
     * DataReaderQos needs to be set to
     * {@link org.omg.dds.core.policy.Durability.Kind#VOLATILE} to ensure that
     * historical data that potentially is available already at creation time is
     * not immediately delivered to the DataReader at that time.
     * 
     * @param minSourceTimestamp
     *            Filter out all data published before this time. The special
     *            org.omg.dds.core.Time.invalidTime() can be used when no
     *            minimum filter is needed.
     * @param maxSourceTimestamp
     *            Filter out all data published after this time. The special
     *            org.omg.dds.core.Time.invalidTime() can be used when no
     *            maximum filter is needed
     * @param maxWait
     *            The maximum duration the application thread is blocked during
     *            this operation.
     * 
     * @throws TimeoutException
     *             thrown when maxWait has expired before the applicable
     *             historical data has successfully been obtained.
     * @throws org.omg.dds.core.DDSException
     *             An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *             The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             The Data Distribution Service ran out of resources to
     *             complete this operation.
     */
    public void waitForHistoricalData(Time minSourceTimestamp,
            Time maxSourceTimestamp, Duration maxWait) throws TimeoutException;

    /**
     * This operation will block the application thread until all historical
     * data that matches the supplied conditions is received.
     * <p>
     * This operation only makes sense when the receiving node has configured
     * its durability service as an On_Request alignee. (See also the
     * description of the
     * //OpenSplice/DurabilityService/NameSpaces/Policy[@alignee] attribute in
     * the Deployment Guide.) Otherwise the Durability Service will not
     * distinguish between separate reader requests and still inject the full
     * historical data set in each reader.
     * <p>
     * Additionally, when creating the DataReader, the DurabilityQos.kind of the
     * DataReaderQos needs to be set to
     * {@link org.omg.dds.core.policy.Durability.Kind#VOLATILE} to ensure that
     * historical data that potentially is available already at creation time is
     * not immediately delivered to the DataReader at that time.
     * 
     * @param resourceLimits
     *            Specifies limits on the maximum amount of historical data that
     *            may be received.
     * @param maxWait
     *            The maximum duration the application thread is blocked during
     *            this operation.
     * 
     * @throws TimeoutException
     *             thrown when maxWait has expired before the applicable
     *             historical data has successfully been obtained.
     * @throws org.omg.dds.core.DDSException
     *             An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *             The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             The Data Distribution Service ran out of resources to
     *             complete this operation.
     */
    public void waitForHistoricalData(
            ResourceLimits resourceLimits, Duration maxWait)
            throws TimeoutException;

    /**
     * This operation will block the application thread until all historical
     * data that matches the supplied conditions is received.
     * <p>
     * This operation only makes sense when the receiving node has configured
     * its durability service as an On_Request alignee. (See also the
     * description of the
     * //OpenSplice/DurabilityService/NameSpaces/Policy[@alignee] attribute in
     * the Deployment Guide.) Otherwise the Durability Service will not
     * distinguish between separate reader requests and still inject the full
     * historical data set in each reader.
     * <p>
     * Additionally, when creating the DataReader, the DurabilityQos.kind of the
     * DataReaderQos needs to be set to
     * {@link org.omg.dds.core.policy.Durability.Kind#VOLATILE} to ensure that
     * historical data that potentially is available already at creation time is
     * not immediately delivered to the DataReader at that time.
     * 
     * @param filterExpression
     *            the SQL expression (subset of SQL), which defines the
     *            filtering criteria (null when no SQL filtering is needed).
     * @param filterParameters
     *            sequence of strings with the parameter values used in the SQL
     *            expression (i.e., the number of %n tokens in the expression).
     *            The number of values in filterParameters must be equal to or
     *            greater than the highest referenced %n token in the
     *            filterExpression (e.g. if %1 and %8 are used as parameters in
     *            the filterExpression, the filterParameters should contain at
     *            least n + 1 = 9 values).
     * @param minSourceTimestamp
     *            Filter out all data published before this time. The special
     *            org.omg.dds.core.Time.invalidTime() can be used when no
     *            minimum filter is needed.
     * @param maxSourceTimestamp
     *            Filter out all data published after this time. The special
     *            org.omg.dds.core.Time.invalidTime() can be used when no
     *            maximum filter is needed
     * @param resourceLimits
     *            Specifies limits on the maximum amount of historical data that
     *            may be received.
     * @param maxWait
     *            The maximum duration the application thread is blocked during
     *            this operation.
     * @param unit
     *            The TimeUnit which the maxWait describes (i.e.
     *            TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * 
     * @throws TimeoutException
     *             thrown when maxWait has expired before the applicable
     *             historical data has successfully been obtained.
     * @throws org.omg.dds.core.DDSException
     *             An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *             The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             The Data Distribution Service ran out of resources to
     *             complete this operation.
     */
    public void waitForHistoricalData(String filterExpression,
            List<String> filterParameters, Time minSourceTimestamp,
            Time maxSourceTimestamp, ResourceLimits resourceLimits,
            long maxWait, TimeUnit unit) throws TimeoutException;

    /**
     * This operation will block the application thread until all historical
     * data that matches the supplied conditions is received.
     * <p>
     * This operation only makes sense when the receiving node has configured
     * its durability service as an On_Request alignee. (See also the
     * description of the
     * //OpenSplice/DurabilityService/NameSpaces/Policy[@alignee] attribute in
     * the Deployment Guide.) Otherwise the Durability Service will not
     * distinguish between separate reader requests and still inject the full
     * historical data set in each reader.
     * <p>
     * Additionally, when creating the DataReader, the DurabilityQos.kind of the
     * DataReaderQos needs to be set to
     * {@link org.omg.dds.core.policy.Durability.Kind#VOLATILE} to ensure that
     * historical data that potentially is available already at creation time is
     * not immediately delivered to the DataReader at that time.
     * 
     * @param filterExpression
     *            the SQL expression (subset of SQL), which defines the
     *            filtering criteria (null when no SQL filtering is needed).
     * @param filterParameters
     *            sequence of strings with the parameter values used in the SQL
     *            expression (i.e., the number of %n tokens in the expression).
     *            The number of values in filterParameters must be equal to or
     *            greater than the highest referenced %n token in the
     *            filterExpression (e.g. if %1 and %8 are used as parameters in
     *            the filterExpression, the filterParameters should contain at
     *            least n + 1 = 9 values).
     * @param minSourceTimestamp
     *            Filter out all data published before this time. The special
     *            org.omg.dds.core.Time.invalidTime() can be used when no
     *            minimum filter is needed.
     * @param maxSourceTimestamp
     *            Filter out all data published after this time. The special
     *            org.omg.dds.core.Time.invalidTime() can be used when no
     *            maximum filter is needed
     * @param maxWait
     *            The maximum duration the application thread is blocked during
     *            this operation.
     * @param unit
     *            The TimeUnit which the maxWait describes (i.e.
     *            TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * 
     * @throws TimeoutException
     *             thrown when maxWait has expired before the applicable
     *             historical data has successfully been obtained.
     * @throws org.omg.dds.core.DDSException
     *             An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *             The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             The Data Distribution Service ran out of resources to
     *             complete this operation.
     */
    public void waitForHistoricalData(String filterExpression,
            List<String> filterParameters, Time minSourceTimestamp,
            Time maxSourceTimestamp, long maxWait, TimeUnit unit)
            throws TimeoutException;

    /**
     * This operation will block the application thread until all historical
     * data that matches the supplied conditions is received.
     * <p>
     * This operation only makes sense when the receiving node has configured
     * its durability service as an On_Request alignee. (See also the
     * description of the
     * //OpenSplice/DurabilityService/NameSpaces/Policy[@alignee] attribute in
     * the Deployment Guide.) Otherwise the Durability Service will not
     * distinguish between separate reader requests and still inject the full
     * historical data set in each reader.
     * <p>
     * Additionally, when creating the DataReader, the DurabilityQos.kind of the
     * DataReaderQos needs to be set to
     * {@link org.omg.dds.core.policy.Durability.Kind#VOLATILE} to ensure that
     * historical data that potentially is available already at creation time is
     * not immediately delivered to the DataReader at that time.
     * 
     * @param filterExpression
     *            the SQL expression (subset of SQL), which defines the
     *            filtering criteria (null when no SQL filtering is needed).
     * @param filterParameters
     *            sequence of strings with the parameter values used in the SQL
     *            expression (i.e., the number of %n tokens in the expression).
     *            The number of values in filterParameters must be equal to or
     *            greater than the highest referenced %n token in the
     *            filterExpression (e.g. if %1 and %8 are used as parameters in
     *            the filterExpression, the filterParameters should contain at
     *            least n + 1 = 9 values).
     * @param resourceLimits
     *            Specifies limits on the maximum amount of historical data that
     *            may be received.
     * @param maxWait
     *            The maximum duration the application thread is blocked during
     *            this operation.
     * @param unit
     *            The TimeUnit which the maxWait describes (i.e.
     *            TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * 
     * @throws TimeoutException
     *             thrown when maxWait has expired before the applicable
     *             historical data has successfully been obtained.
     * @throws org.omg.dds.core.DDSException
     *             An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *             The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             The Data Distribution Service ran out of resources to
     *             complete this operation.
     */
    public void waitForHistoricalData(String filterExpression,
            List<String> filterParameters, ResourceLimits resourceLimits,
            long maxWait, TimeUnit unit) throws TimeoutException;

    /**
     * This operation will block the application thread until all historical
     * data that matches the supplied conditions is received.
     * <p>
     * This operation only makes sense when the receiving node has configured
     * its durability service as an On_Request alignee. (See also the
     * description of the
     * //OpenSplice/DurabilityService/NameSpaces/Policy[@alignee] attribute in
     * the Deployment Guide.) Otherwise the Durability Service will not
     * distinguish between separate reader requests and still inject the full
     * historical data set in each reader.
     * <p>
     * Additionally, when creating the DataReader, the DurabilityQos.kind of the
     * DataReaderQos needs to be set to
     * {@link org.omg.dds.core.policy.Durability.Kind#VOLATILE} to ensure that
     * historical data that potentially is available already at creation time is
     * not immediately delivered to the DataReader at that time.
     * 
     * @param filterExpression
     *            the SQL expression (subset of SQL), which defines the
     *            filtering criteria (null when no SQL filtering is needed).
     * @param filterParameters
     *            sequence of strings with the parameter values used in the SQL
     *            expression (i.e., the number of %n tokens in the expression).
     *            The number of values in filterParameters must be equal to or
     *            greater than the highest referenced %n token in the
     *            filterExpression (e.g. if %1 and %8 are used as parameters in
     *            the filterExpression, the filterParameters should contain at
     *            least n + 1 = 9 values).
     * @param maxWait
     *            The maximum duration the application thread is blocked during
     *            this operation.
     * @param unit
     *            The TimeUnit which the maxWait describes (i.e.
     *            TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * 
     * @throws TimeoutException
     *             thrown when maxWait has expired before the applicable
     *             historical data has successfully been obtained.
     * @throws org.omg.dds.core.DDSException
     *             An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *             The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             The Data Distribution Service ran out of resources to
     *             complete this operation.
     */
    public void waitForHistoricalData(String filterExpression,
            List<String> filterParameters, long maxWait, TimeUnit unit)
            throws TimeoutException;

    /**
     * This operation will block the application thread until all historical
     * data that matches the supplied conditions is received.
     * <p>
     * This operation only makes sense when the receiving node has configured
     * its durability service as an On_Request alignee. (See also the
     * description of the
     * //OpenSplice/DurabilityService/NameSpaces/Policy[@alignee] attribute in
     * the Deployment Guide.) Otherwise the Durability Service will not
     * distinguish between separate reader requests and still inject the full
     * historical data set in each reader.
     * <p>
     * Additionally, when creating the DataReader, the DurabilityQos.kind of the
     * DataReaderQos needs to be set to
     * {@link org.omg.dds.core.policy.Durability.Kind#VOLATILE} to ensure that
     * historical data that potentially is available already at creation time is
     * not immediately delivered to the DataReader at that time.
     * 
     * @param minSourceTimestamp
     *            Filter out all data published before this time. The special
     *            org.omg.dds.core.Time.invalidTime() can be used when no
     *            minimum filter is needed.
     * @param maxSourceTimestamp
     *            Filter out all data published after this time. The special
     *            org.omg.dds.core.Time.invalidTime() can be used when no
     *            maximum filter is needed
     * @param resourceLimits
     *            Specifies limits on the maximum amount of historical data that
     *            may be received.
     * @param maxWait
     *            The maximum duration the application thread is blocked during
     *            this operation.
     * @param unit
     *            The TimeUnit which the maxWait describes (i.e.
     *            TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * 
     * @throws TimeoutException
     *             thrown when maxWait has expired before the applicable
     *             historical data has successfully been obtained.
     * @throws org.omg.dds.core.DDSException
     *             An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *             The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             The Data Distribution Service ran out of resources to
     *             complete this operation.
     */
    public void waitForHistoricalData(Time minSourceTimestamp,
            Time maxSourceTimestamp, ResourceLimits resourceLimits,
            long maxWait, TimeUnit unit) throws TimeoutException;

    /**
     * This operation will block the application thread until all historical
     * data that matches the supplied conditions is received.
     * <p>
     * This operation only makes sense when the receiving node has configured
     * its durability service as an On_Request alignee. (See also the
     * description of the
     * //OpenSplice/DurabilityService/NameSpaces/Policy[@alignee] attribute in
     * the Deployment Guide.) Otherwise the Durability Service will not
     * distinguish between separate reader requests and still inject the full
     * historical data set in each reader.
     * <p>
     * Additionally, when creating the DataReader, the DurabilityQos.kind of the
     * DataReaderQos needs to be set to
     * {@link org.omg.dds.core.policy.Durability.Kind#VOLATILE} to ensure that
     * historical data that potentially is available already at creation time is
     * not immediately delivered to the DataReader at that time.
     * 
     * @param minSourceTimestamp
     *            Filter out all data published before this time. The special
     *            org.omg.dds.core.Time.invalidTime() can be used when no
     *            minimum filter is needed.
     * @param maxSourceTimestamp
     *            Filter out all data published after this time. The special
     *            org.omg.dds.core.Time.invalidTime() can be used when no
     *            maximum filter is needed
     * @param maxWait
     *            The maximum duration the application thread is blocked during
     *            this operation.
     * @param unit
     *            The TimeUnit which the maxWait describes (i.e.
     *            TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * 
     * @throws TimeoutException
     *             thrown when maxWait has expired before the applicable
     *             historical data has successfully been obtained.
     * @throws org.omg.dds.core.DDSException
     *             An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *             The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             The Data Distribution Service ran out of resources to
     *             complete this operation.
     */
    public void waitForHistoricalData(Time minSourceTimestamp,
            Time maxSourceTimestamp, long maxWait, TimeUnit unit)
            throws TimeoutException;

    /**
     * This operation will block the application thread until all historical
     * data that matches the supplied conditions is received.
     * <p>
     * This operation only makes sense when the receiving node has configured
     * its durability service as an On_Request alignee. (See also the
     * description of the
     * //OpenSplice/DurabilityService/NameSpaces/Policy[@alignee] attribute in
     * the Deployment Guide.) Otherwise the Durability Service will not
     * distinguish between separate reader requests and still inject the full
     * historical data set in each reader.
     * <p>
     * Additionally, when creating the DataReader, the DurabilityQos.kind of the
     * DataReaderQos needs to be set to
     * {@link org.omg.dds.core.policy.Durability.Kind#VOLATILE} to ensure that
     * historical data that potentially is available already at creation time is
     * not immediately delivered to the DataReader at that time.
     * 
     * @param resourceLimits
     *            Specifies limits on the maximum amount of historical data that
     *            may be received.
     * @param maxWait
     *            The maximum duration the application thread is blocked during
     *            this operation.
     * @param unit
     *            The TimeUnit which the maxWait describes (i.e.
     *            TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * 
     * @throws TimeoutException
     *             thrown when maxWait has expired before the applicable
     *             historical data has successfully been obtained.
     * @throws org.omg.dds.core.DDSException
     *             An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *             The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             The Data Distribution Service ran out of resources to
     *             complete this operation.
     */
    public void waitForHistoricalData(
            ResourceLimits resourceLimits, long maxWait, TimeUnit unit)
            throws TimeoutException;
}
