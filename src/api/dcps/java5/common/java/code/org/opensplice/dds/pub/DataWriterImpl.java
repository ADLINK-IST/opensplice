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
package org.opensplice.dds.pub;

import java.util.Collection;
import java.util.Set;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.omg.dds.core.Duration;
import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.StatusCondition;
import org.omg.dds.core.Time;
import org.omg.dds.core.status.LivelinessLostStatus;
import org.omg.dds.core.status.OfferedDeadlineMissedStatus;
import org.omg.dds.core.status.OfferedIncompatibleQosStatus;
import org.omg.dds.core.status.PublicationMatchedStatus;
import org.omg.dds.core.status.Status;
import org.omg.dds.pub.DataWriter;
import org.omg.dds.pub.DataWriterListener;
import org.omg.dds.pub.DataWriterQos;
import org.omg.dds.topic.SubscriptionBuiltinTopicData;
import org.omg.dds.topic.Topic;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.IllegalOperationExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.StatusConditionImpl;
import org.opensplice.dds.core.TimeImpl;
import org.opensplice.dds.core.Utilities;
import org.opensplice.dds.core.status.StatusConverter;
import org.opensplice.dds.topic.TopicImpl;

public class DataWriterImpl<TYPE> extends AbstractDataWriter<TYPE> {
    private final TopicImpl<TYPE> topic;
    private final ReflectionDataWriter<TYPE> reflectionWriter;

    public DataWriterImpl(OsplServiceEnvironment environment,
            PublisherImpl parent, TopicImpl<TYPE> topic, DataWriterQos qos,
            DataWriterListener<TYPE> listener,
            Collection<Class<? extends Status>> statuses) {
        super(environment, parent);
        this.topic = topic;

        if (qos == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied DataWriterQos is null.");
        }
        if (topic == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied Topic is null.");
        }
        DDS.DataWriterQos oldQos;

        try {
            oldQos = ((DataWriterQosImpl) qos).convert();
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Cannot create DataWriter with non-OpenSplice qos");
        }

        if (listener != null) {
            this.listener = new DataWriterListenerImpl<TYPE>(this.environment,
                    this, listener, true);
        } else {
            this.listener = null;
        }
        DDS.DataWriter old = this.parent.getOld().create_datawriter(
                topic.getOld(), oldQos, this.listener,
                StatusConverter.convertMask(this.environment, statuses));

        if (old == null) {
            Utilities.throwLastErrorException(this.environment);
        }
        this.setOld(old);
        this.reflectionWriter = new ReflectionDataWriter<TYPE>(
                this.environment, this.getOld(), this.topic.getTypeSupport()
                        .getType());
        this.topic.retain();

        if (this.listener != null) {
            this.listener.setInitialised();
        }
    }

    private void setListener(DataWriterListener<TYPE> listener, int mask) {
        DataWriterListenerImpl<TYPE> wrapperListener;
        int rc;

        if (listener != null) {
            wrapperListener = new DataWriterListenerImpl<TYPE>(
                    this.environment, this, listener);
        } else {
            wrapperListener = null;
        }
        rc = this.getOld().set_listener(wrapperListener, mask);
        Utilities.checkReturnCode(rc, this.environment,
                "DataWriter.setListener() failed.");

        this.listener = wrapperListener;
    }

    @Override
    public void setListener(DataWriterListener<TYPE> listener) {
        this.setListener(listener, StatusConverter.getAnyMask());
    }

    @Override
    public void setListener(DataWriterListener<TYPE> listener,
            Collection<Class<? extends Status>> statuses) {
        this.setListener(listener,
                StatusConverter.convertMask(this.environment, statuses));
    }

    @Override
    public void setListener(DataWriterListener<TYPE> listener,
            Class<? extends Status>... statuses) {
        this.setListener(listener,
                StatusConverter.convertMask(this.environment, statuses));
    }

    @Override
    public DataWriterQos getQos() {
        return this.reflectionWriter.getQos();
    }

    @Override
    public void setQos(DataWriterQos qos) {
        this.reflectionWriter.setQos(qos);
    }

    @SuppressWarnings("unchecked")
    @Override
    public <OTHER> DataWriter<OTHER> cast() {
        DataWriter<OTHER> other;
        try {
            other = (DataWriter<OTHER>) this;
        } catch (ClassCastException cce) {
            throw new IllegalOperationExceptionImpl(this.environment,
                    "Unable to perform requested cast.");
        }
        return other;
    }

    @Override
    public Topic<TYPE> getTopic() {
        return this.topic;
    }

    @Override
    public void waitForAcknowledgments(Duration maxWait)
            throws TimeoutException {
        this.reflectionWriter.waitForAcknowledgments(maxWait);
    }

    @Override
    public void waitForAcknowledgments(long maxWait, TimeUnit unit)
            throws TimeoutException {
        this.reflectionWriter.waitForAcknowledgments(maxWait, unit);
    }

    @Override
    public LivelinessLostStatus getLivelinessLostStatus() {
        return this.reflectionWriter.getLivelinessLostStatus();
    }

    @Override
    public OfferedDeadlineMissedStatus getOfferedDeadlineMissedStatus() {
        return this.reflectionWriter.getOfferedDeadlineMissedStatus();
    }

    @Override
    public OfferedIncompatibleQosStatus getOfferedIncompatibleQosStatus() {
        return this.reflectionWriter.getOfferedIncompatibleQosStatus();
    }

    @Override
    public PublicationMatchedStatus getPublicationMatchedStatus() {
        return this.reflectionWriter.getPublicationMatchedStatus();
    }

    @Override
    public void assertLiveliness() {
        this.reflectionWriter.assertLiveliness();
    }

    @Override
    public Set<InstanceHandle> getMatchedSubscriptions() {
        return this.reflectionWriter.getMatchedSubscriptions();
    }

    @Override
    public SubscriptionBuiltinTopicData getMatchedSubscriptionData(
            InstanceHandle subscriptionHandle) {
        return this.reflectionWriter
                .getMatchedSubscriptionData(subscriptionHandle);
    }

    @Override
    public InstanceHandle registerInstance(TYPE instanceData)
            throws TimeoutException {
        return this.reflectionWriter.registerInstance(instanceData);
    }

    @Override
    public InstanceHandle registerInstance(TYPE instanceData,
            Time sourceTimestamp) throws TimeoutException {
        return this.reflectionWriter.registerInstance(instanceData,
                sourceTimestamp);
    }

    @Override
    public InstanceHandle registerInstance(TYPE instanceData,
            long sourceTimestamp, TimeUnit unit) throws TimeoutException {
        return this.reflectionWriter.registerInstance(instanceData,
                sourceTimestamp, unit);
    }

    @Override
    public void unregisterInstance(InstanceHandle handle)
            throws TimeoutException {
        this.reflectionWriter.unregisterInstance(handle);
    }

    @Override
    public void unregisterInstance(InstanceHandle handle, TYPE instanceData)
            throws TimeoutException {
        this.reflectionWriter.unregisterInstance(handle, instanceData);
    }

    @Override
    public void unregisterInstance(InstanceHandle handle, TYPE instanceData,
            Time sourceTimestamp) throws TimeoutException {
        this.reflectionWriter.unregisterInstance(handle, instanceData,
                sourceTimestamp);
    }

    @Override
    public void unregisterInstance(InstanceHandle handle, TYPE instanceData,
            long sourceTimestamp, TimeUnit unit) throws TimeoutException {
        this.reflectionWriter.unregisterInstance(handle, instanceData,
                sourceTimestamp, unit);
    }

    @Override
    public void write(TYPE instanceData) throws TimeoutException {
        this.reflectionWriter.write(instanceData, this.environment.getSPI()
                .nilHandle());
    }

    @Override
    public void write(TYPE instanceData, Time sourceTimestamp)
            throws TimeoutException {
        this.reflectionWriter.write(instanceData, this.environment.getSPI()
                .nilHandle(), sourceTimestamp);
    }

    @Override
    public void write(TYPE instanceData, long sourceTimestamp, TimeUnit unit)
            throws TimeoutException {
        this.reflectionWriter.write(instanceData, new TimeImpl(
                this.environment, sourceTimestamp, unit));
    }

    @Override
    public void write(TYPE instanceData, InstanceHandle handle)
            throws TimeoutException {
        this.reflectionWriter.write(instanceData, handle);
    }

    @Override
    public void write(TYPE instanceData, InstanceHandle handle,
            Time sourceTimestamp) throws TimeoutException {
        this.reflectionWriter.write(instanceData, handle, sourceTimestamp);
    }

    @Override
    public void write(TYPE instanceData, InstanceHandle handle,
            long sourceTimestamp, TimeUnit unit) throws TimeoutException {
        this.reflectionWriter
                .write(instanceData, handle, sourceTimestamp, unit);
    }

    @Override
    public void dispose(InstanceHandle instanceHandle) throws TimeoutException {
        this.reflectionWriter.dispose(instanceHandle, null);
    }

    @Override
    public void dispose(InstanceHandle instanceHandle, TYPE instanceData)
            throws TimeoutException {
        this.reflectionWriter.dispose(instanceHandle, instanceData);
    }

    @Override
    public void dispose(InstanceHandle instanceHandle, TYPE instanceData,
            Time sourceTimestamp) throws TimeoutException {
        this.reflectionWriter.dispose(instanceHandle, instanceData,
                sourceTimestamp);
    }

    @Override
    public void dispose(InstanceHandle instanceHandle, TYPE instanceData,
            long sourceTimestamp, TimeUnit unit) throws TimeoutException {
        this.reflectionWriter.dispose(instanceHandle, instanceData,
                sourceTimestamp, unit);
    }

    @Override
    public TYPE getKeyValue(TYPE keyHolder, InstanceHandle handle) {
        return this.reflectionWriter.getKeyValue(keyHolder, handle);
    }

    @Override
    public TYPE getKeyValue(InstanceHandle handle) {
        return this.reflectionWriter.getKeyValue(handle);
    }

    @Override
    public InstanceHandle lookupInstance(TYPE keyHolder) {
        return this.reflectionWriter.lookupInstance(keyHolder);
    }

    @Override
    public StatusCondition<DataWriter<TYPE>> getStatusCondition() {
        DDS.StatusCondition oldCondition = this.getOld().get_statuscondition();

        if (oldCondition == null) {
            Utilities.throwLastErrorException(this.environment);
        }
        return new StatusConditionImpl<DataWriter<TYPE>>(this.environment,
                oldCondition, this);
    }

    @Override
    public void writeDispose(TYPE instanceData) throws TimeoutException {
        this.reflectionWriter.writeDispose(instanceData);

    }

    @Override
    public void writeDispose(TYPE instanceData, Time sourceTimestamp)
            throws TimeoutException {
        this.reflectionWriter.writeDispose(instanceData, sourceTimestamp);
    }

    @Override
    public void writeDispose(TYPE instanceData, long sourceTimestamp,
            TimeUnit unit) throws TimeoutException {
        this.reflectionWriter.writeDispose(instanceData, sourceTimestamp, unit);
    }

    @Override
    public void writeDispose(TYPE instanceData, InstanceHandle handle)
            throws TimeoutException {
        this.reflectionWriter.writeDispose(instanceData, handle);
    }

    @Override
    public void writeDispose(TYPE instanceData, InstanceHandle handle,
            Time sourceTimestamp) throws TimeoutException {
        this.reflectionWriter.writeDispose(instanceData, handle,
                sourceTimestamp);
    }

    @Override
    public void writeDispose(TYPE instanceData, InstanceHandle handle,
            long sourceTimestamp, TimeUnit unit) throws TimeoutException {
        this.reflectionWriter.writeDispose(instanceData, handle,
                sourceTimestamp, unit);

    }

    @Override
    protected void destroy() {
        super.destroy();
        this.topic.close();
    }
}
