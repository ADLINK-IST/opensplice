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
 *
 */
package org.opensplice.dds.sub;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.omg.dds.core.AlreadyClosedException;
import org.omg.dds.core.Duration;
import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.StatusCondition;
import org.omg.dds.core.Time;
import org.omg.dds.core.policy.ResourceLimits;
import org.omg.dds.core.status.LivelinessChangedStatus;
import org.omg.dds.core.status.RequestedDeadlineMissedStatus;
import org.omg.dds.core.status.RequestedIncompatibleQosStatus;
import org.omg.dds.core.status.SampleLostStatus;
import org.omg.dds.core.status.SampleRejectedStatus;
import org.omg.dds.core.status.Status;
import org.omg.dds.core.status.SubscriptionMatchedStatus;
import org.omg.dds.sub.DataReader;
import org.omg.dds.sub.DataReaderListener;
import org.omg.dds.sub.DataReaderQos;
import org.omg.dds.sub.QueryCondition;
import org.omg.dds.sub.ReadCondition;
import org.omg.dds.sub.Sample;
import org.omg.dds.sub.Sample.Iterator;
import org.omg.dds.sub.Subscriber.DataState;
import org.omg.dds.topic.PublicationBuiltinTopicData;
import org.omg.dds.topic.TopicDescription;
import org.opensplice.dds.core.DomainEntityImpl;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.IllegalOperationExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.StatusConditionImpl;
import org.opensplice.dds.core.Utilities;
import org.opensplice.dds.core.policy.ResourceLimitsImpl;
import org.opensplice.dds.core.status.StatusConverter;
import org.opensplice.dds.topic.TopicDescriptionExt;

public abstract class AbstractDataReader<TYPE>
        extends
        DomainEntityImpl<DDS.DataReader, SubscriberImpl, DDS.Subscriber, DataReaderQos, DataReaderListener<TYPE>, DataReaderListenerImpl<TYPE>>
        implements org.opensplice.dds.sub.DataReader<TYPE> {

    protected final TopicDescriptionExt<TYPE> topicDescription;
    protected final ConcurrentHashMap<DDS.Condition, ReadConditionImpl<TYPE>> conditions;
    protected final HashSet<AbstractIterator<TYPE>> iterators;
    protected final Selector<TYPE> selector;


    public AbstractDataReader(OsplServiceEnvironment environment,
            SubscriberImpl parent, TopicDescriptionExt<TYPE> topicDescription) {
        super(environment, parent, parent.getOld());

        this.topicDescription = topicDescription;
        this.conditions = new ConcurrentHashMap<DDS.Condition, ReadConditionImpl<TYPE>>();
        this.iterators = new HashSet<AbstractIterator<TYPE>>();
        this.selector = new SelectorImpl<TYPE>(environment, this);
    }

    public void registerIterator(AbstractIterator<TYPE> iterator) {
        synchronized (this.iterators) {
            this.iterators.add(iterator);
        }
    }

    public void deregisterIterator(AbstractIterator<TYPE> iterator) {
        synchronized (this.iterators) {
            this.iterators.remove(iterator);
        }
    }

    protected abstract ReflectionDataReader<?, TYPE> getReflectionReader();

    @Override
    public ReadCondition<TYPE> createReadCondition(DataState states) {
        ReadConditionImpl<TYPE> condition;

        synchronized (this.conditions) {
            try {
                condition = new ReadConditionImpl<TYPE>(this.environment, this,
                        (DataStateImpl) states);
            } catch (ClassCastException e) {
                throw new IllegalArgumentExceptionImpl(this.environment,
                        "Non-OpenSplice DataState not supported.");
            }
            this.conditions.put(condition.getOld(), condition);
        }
        return condition;
    }

    @Override
    public QueryCondition<TYPE> createQueryCondition(String queryExpression,
            List<String> queryParameters) {
        return this.createQueryCondition(
                DataStateImpl.getAnyStateDataState(this.environment),
                queryExpression, queryParameters);
    }

    @Override
    public QueryCondition<TYPE> createQueryCondition(DataState states,
            String queryExpression, List<String> queryParameters) {
        QueryConditionImpl<TYPE> query;

        try {
            synchronized (this.conditions) {
                query = new QueryConditionImpl<TYPE>(this.environment, this,
                        (DataStateImpl) states, queryExpression,
                        queryParameters);

                this.conditions.put(query.getOld(), query);
            }
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Non-OpenSplice DataState not supported.");
        }

        return query;
    }

    @Override
    public QueryCondition<TYPE> createQueryCondition(String queryExpression,
            String... queryParameters) {
        return createQueryCondition(queryExpression,
                Arrays.asList(queryParameters));
    }

    @Override
    public QueryCondition<TYPE> createQueryCondition(DataState states,
            String queryExpression, String... queryParameters) {
        QueryCondition<TYPE> result;

        if (queryParameters == null) {
            result = createQueryCondition(states, queryExpression,
                    new ArrayList<String>());
        } else {
            result = createQueryCondition(states, queryExpression,
                    Arrays.asList(queryParameters));
        }
        return result;
    }

    public void destroyReadCondition(ReadConditionImpl<TYPE> condition) {
        synchronized (this.conditions) {
            DDS.ReadCondition old = condition.getOld();
            int rc = this.getOld().delete_readcondition(old);
            this.conditions.remove(old);
            Utilities.checkReturnCode(rc, this.environment,
                    "Condition already closed.");
        }

    }

    @SuppressWarnings("unchecked")
    @Override
    public void closeContainedEntities() {
        for (ReadConditionImpl<TYPE> condition : this.conditions.values()) {
            /*
             * Intentionally ignoring potential errors during deletion as
             * application may concurrently close conditions.
             */
            this.getOld().delete_readcondition(condition.getOld());
        }
        HashSet<AbstractIterator<TYPE>> clones;

        synchronized (this.iterators) {
            clones = (HashSet<AbstractIterator<TYPE>>) this.iterators.clone();
        }
        for (AbstractIterator<TYPE> iterator : clones) {
            try {
                iterator.close();
            } catch (AlreadyClosedException a) {
                /* Entity may be closed concurrently by application */
            }
        }
    }

    @Override
    protected void destroy() {
        this.closeContainedEntities();
        this.parent.destroyDataReader(this);
    }

    @Override
    public SubscriberImpl getParent() {
        return this.parent;
    }

    private void setListener(DataReaderListener<TYPE> listener, int mask) {
        DataReaderListenerImpl<TYPE> wrapperListener;
        int rc;

        if (listener != null) {
            wrapperListener = new DataReaderListenerImpl<TYPE>(
                    this.environment, this, listener);
        } else {
            wrapperListener = null;
        }
        rc = this.getOld().set_listener(wrapperListener, mask);
        Utilities.checkReturnCode(rc, this.environment,
                "DataReader.setListener() failed.");

        this.listener = wrapperListener;
    }

    @Override
    public void setListener(DataReaderListener<TYPE> listener) {
        this.setListener(listener, StatusConverter.getAnyMask());
    }

    @Override
    public void setListener(DataReaderListener<TYPE> listener,
            Collection<Class<? extends Status>> statuses) {
        this.setListener(listener,
                StatusConverter.convertMask(this.environment, statuses));
    }

    @Override
    public void setListener(DataReaderListener<TYPE> listener,
            Class<? extends Status>... statuses) {
        this.setListener(listener,
                StatusConverter.convertMask(this.environment, statuses));
    }

    @Override
    public TopicDescription<TYPE> getTopicDescription() {
        return this.topicDescription;
    }

    @Override
    public void setProperty(String key, String value) {
        int rc = this.getOld().set_property(new DDS.Property(key, value));
        Utilities.checkReturnCode(rc, this.environment,
                "DataReader.setProperty() failed.");
    }

    @Override
    public String getProperty(String key) {
        DDS.PropertyHolder holder = new DDS.PropertyHolder();
        int rc = this.getOld().get_property(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "DataReader.getProperty() failed.");

        return holder.value.value;
    }

    @SuppressWarnings("unchecked")
    @Override
    public <OTHER> DataReader<OTHER> cast() {
        DataReader<OTHER> other;
        try {
            other = (DataReader<OTHER>) this;
        } catch (ClassCastException cce) {
            throw new IllegalOperationExceptionImpl(this.environment,
                    "Unable to perform requested cast.");
        }
        return other;
    }

    @Override
    public DataReaderQos getQos() {
        return this.getReflectionReader().getQos();
    }

    @Override
    public void setQos(DataReaderQos qos) {
        this.getReflectionReader().setQos(qos);
    }

    @Override
    public SampleRejectedStatus getSampleRejectedStatus() {
        return this.getReflectionReader().getSampleRejectedStatus();
    }

    @Override
    public LivelinessChangedStatus getLivelinessChangedStatus() {
        return this.getReflectionReader().getLivelinessChangedStatus();
    }

    @Override
    public RequestedDeadlineMissedStatus getRequestedDeadlineMissedStatus() {
        return this.getReflectionReader().getRequestedDeadlineMissedStatus();
    }

    @Override
    public RequestedIncompatibleQosStatus getRequestedIncompatibleQosStatus() {
        return this.getReflectionReader().getRequestedIncompatibleQosStatus();
    }

    @Override
    public SubscriptionMatchedStatus getSubscriptionMatchedStatus() {
        return this.getReflectionReader().getSubscriptionMatchedStatus();
    }

    @Override
    public SampleLostStatus getSampleLostStatus() {
        return this.getReflectionReader().getSampleLostStatus();
    }

    @Override
    public void waitForHistoricalData(Duration maxWait) throws TimeoutException {
        this.getReflectionReader().waitForHistoricalData(maxWait);
    }

    @Override
    public void waitForHistoricalData(long maxWait, TimeUnit unit)
            throws TimeoutException {
        this.getReflectionReader().waitForHistoricalData(maxWait, unit);
    }

    @Override
    public void waitForHistoricalData(String filterExpression,
            List<String> filterParameters, Time minSourceTimestamp,
            Time maxSourceTimestamp, ResourceLimits resourceLimits,
            Duration maxWait) throws TimeoutException {
        this.getReflectionReader().waitForHistoricalData(filterExpression,
                filterParameters, minSourceTimestamp, maxSourceTimestamp,
                resourceLimits, maxWait);
    }

    @Override
    public void waitForHistoricalData(String filterExpression,
            List<String> filterParameters, Time minSourceTimestamp,
            Time maxSourceTimestamp, Duration maxWait) throws TimeoutException {
        this.getReflectionReader().waitForHistoricalData(filterExpression,
                filterParameters, minSourceTimestamp, maxSourceTimestamp,
                new ResourceLimitsImpl(this.environment), maxWait);
    }

    @Override
    public void waitForHistoricalData(String filterExpression,
            List<String> filterParameters, ResourceLimits resourceLimits,
            Duration maxWait) throws TimeoutException {
        this.getReflectionReader().waitForHistoricalData(filterExpression,
                filterParameters, Time.invalidTime(this.environment),
                Time.invalidTime(this.environment), resourceLimits, maxWait);
    }

    @Override
    public void waitForHistoricalData(String filterExpression,
            List<String> filterParameters, Duration maxWait)
            throws TimeoutException {
        this.getReflectionReader().waitForHistoricalData(filterExpression,
                filterParameters, Time.invalidTime(this.environment),
                Time.invalidTime(this.environment),
                new ResourceLimitsImpl(this.environment), maxWait);
    }

    @Override
    public void waitForHistoricalData(Time minSourceTimestamp,
            Time maxSourceTimestamp, ResourceLimits resourceLimits,
            Duration maxWait) throws TimeoutException {
        this.getReflectionReader()
                .waitForHistoricalData(null, null, minSourceTimestamp,
                        maxSourceTimestamp, resourceLimits, maxWait);

    }

    @Override
    public void waitForHistoricalData(Time minSourceTimestamp,
            Time maxSourceTimestamp, Duration maxWait) throws TimeoutException {
        this.getReflectionReader().waitForHistoricalData(null, null,
                minSourceTimestamp, maxSourceTimestamp,
                new ResourceLimitsImpl(this.environment), maxWait);

    }

    @Override
    public void waitForHistoricalData(ResourceLimits resourceLimits,
            Duration maxWait) throws TimeoutException {
        this.getReflectionReader().waitForHistoricalData(null, null,
                Time.invalidTime(this.environment),
                Time.invalidTime(this.environment), resourceLimits, maxWait);

    }

    @Override
    public void waitForHistoricalData(String filterExpression,
            List<String> filterParameters, Time minSourceTimestamp,
            Time maxSourceTimestamp, ResourceLimits resourceLimits,
            long maxWait, TimeUnit unit) throws TimeoutException {
        this.getReflectionReader().waitForHistoricalData(filterExpression,
                filterParameters, minSourceTimestamp, maxSourceTimestamp,
                resourceLimits, maxWait, unit);
    }

    @Override
    public void waitForHistoricalData(String filterExpression,
            List<String> filterParameters, Time minSourceTimestamp,
            Time maxSourceTimestamp, long maxWait, TimeUnit unit)
            throws TimeoutException {
        this.getReflectionReader().waitForHistoricalData(filterExpression,
                filterParameters, minSourceTimestamp, maxSourceTimestamp,
                new ResourceLimitsImpl(this.environment), maxWait, unit);
    }

    @Override
    public void waitForHistoricalData(String filterExpression,
            List<String> filterParameters, ResourceLimits resourceLimits,
            long maxWait, TimeUnit unit) throws TimeoutException {
        this.getReflectionReader().waitForHistoricalData(filterExpression,
                filterParameters, Time.invalidTime(this.environment),
                Time.invalidTime(this.environment), resourceLimits, maxWait,
                unit);
    }

    @Override
    public void waitForHistoricalData(String filterExpression,
            List<String> filterParameters, long maxWait, TimeUnit unit)
            throws TimeoutException {
        this.getReflectionReader().waitForHistoricalData(filterExpression,
                filterParameters, Time.invalidTime(this.environment),
                Time.invalidTime(this.environment),
                new ResourceLimitsImpl(this.environment), maxWait, unit);
    }

    @Override
    public void waitForHistoricalData(Time minSourceTimestamp,
            Time maxSourceTimestamp, ResourceLimits resourceLimits,
            long maxWait, TimeUnit unit) throws TimeoutException {
        this.getReflectionReader().waitForHistoricalData(null, null,
                minSourceTimestamp, maxSourceTimestamp, resourceLimits,
                maxWait, unit);
    }

    @Override
    public void waitForHistoricalData(Time minSourceTimestamp,
            Time maxSourceTimestamp, long maxWait, TimeUnit unit)
            throws TimeoutException {
        this.getReflectionReader().waitForHistoricalData(null, null,
                minSourceTimestamp, maxSourceTimestamp,
                new ResourceLimitsImpl(this.environment), maxWait, unit);
    }

    @Override
    public void waitForHistoricalData(ResourceLimits resourceLimits,
            long maxWait, TimeUnit unit) throws TimeoutException {
        this.getReflectionReader().waitForHistoricalData(null, null,
                Time.invalidTime(this.environment),
                Time.invalidTime(this.environment), resourceLimits, maxWait,
                unit);
    }

    @Override
    public Set<InstanceHandle> getMatchedPublications() {
        return this.getReflectionReader().getMatchedPublications();
    }

    @Override
    public PublicationBuiltinTopicData getMatchedPublicationData(
            InstanceHandle publicationHandle) {
        return this.getReflectionReader()
                .getMatchedPublicationData(publicationHandle);
    }

    @Override
    public org.omg.dds.sub.DataReader.Selector<TYPE> select() {
        return this.selector;
    }

    public void returnLoan(Object sampleSeqHolder,
            DDS.SampleInfoSeqHolder infoSeqHolder) {
        this.getReflectionReader().returnLoan(sampleSeqHolder, infoSeqHolder);
    }

    @Override
    public Iterator<TYPE> read() {
        return this.getReflectionReader().read();
    }

    @Override
    public Iterator<TYPE> read(org.omg.dds.sub.DataReader.Selector<TYPE> query) {
        return this.getReflectionReader().read(query);
    }

    @Override
    public Iterator<TYPE> read(int maxSamples) {
        return this.getReflectionReader().read(maxSamples);
    }

    @Override
    public List<Sample<TYPE>> read(List<Sample<TYPE>> samples) {
        return this.getReflectionReader().read(samples);
    }

    @Override
    public List<Sample<TYPE>> read(List<Sample<TYPE>> samples,
            org.omg.dds.sub.DataReader.Selector<TYPE> selector) {
        return this.getReflectionReader().read(samples, selector);
    }

    @Override
    public Iterator<TYPE> take() {
        return this.getReflectionReader().take();
    }

    @Override
    public Iterator<TYPE> take(int maxSamples) {
        return this.getReflectionReader().take(maxSamples);
    }

    @Override
    public Iterator<TYPE> take(org.omg.dds.sub.DataReader.Selector<TYPE> query) {
        return this.getReflectionReader().take(query);
    }

    @Override
    public List<Sample<TYPE>> take(List<Sample<TYPE>> samples) {
        return this.getReflectionReader().take(samples);
    }

    @Override
    public List<Sample<TYPE>> take(List<Sample<TYPE>> samples,
            org.omg.dds.sub.DataReader.Selector<TYPE> selector) {
        return this.getReflectionReader().take(samples, selector);
    }

    @Override
    public StatusCondition<DataReader<TYPE>> getStatusCondition() {
        DDS.StatusCondition oldCondition = this.getOld().get_statuscondition();

        if (oldCondition == null) {
            Utilities.throwLastErrorException(this.environment);
        }
        return new StatusConditionImpl<DataReader<TYPE>>(this.environment,
                oldCondition, this);
    }

    public abstract PreAllocator<TYPE> getPreAllocator(
            List<Sample<TYPE>> samples, Class<?> sampleSeqHolderClz,
            Field sampleSeqHolderValueField);

    public abstract Sample.Iterator<?> createIterator(
            Object sampleSeqHolder,
            Field sampleSeqHolderValueField, DDS.SampleInfoSeqHolder info);
}
