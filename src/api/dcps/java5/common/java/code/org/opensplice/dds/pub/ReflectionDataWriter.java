/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.eclipse.cyclone.dds.core.AbstractDDSObject;
import org.omg.dds.core.DDSObject;
import org.omg.dds.core.Duration;
import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.Time;
import org.omg.dds.core.status.LivelinessLostStatus;
import org.omg.dds.core.status.OfferedDeadlineMissedStatus;
import org.omg.dds.core.status.OfferedIncompatibleQosStatus;
import org.omg.dds.core.status.PublicationMatchedStatus;
import org.omg.dds.pub.DataWriterQos;
import org.omg.dds.topic.SubscriptionBuiltinTopicData;
import org.opensplice.dds.core.DDSExceptionImpl;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.PreconditionNotMetExceptionImpl;
import org.opensplice.dds.core.TimeImpl;
import org.opensplice.dds.core.Utilities;
import org.opensplice.dds.core.status.StatusConverter;
import org.opensplice.dds.topic.SubscriptionBuiltinTopicDataImpl;

public class ReflectionDataWriter<TYPE> extends AbstractDDSObject implements
        DDSObject {
    private final OsplServiceEnvironment environment;
    private final DDS.DataWriter old;
    private final Method registerInstance;
    private final Method registerInstanceTimestamp;
    private final Method unregisterInstance;
    private final Method unregisterInstanceTimestamp;
    private final Method write;
    private final Method writeTimestamp;
    private final Method dispose;
    private final Method disposeTimestamp;
    private final Method writeDispose;
    private final Method writeDisposeTimestamp;
    private final Method getKeyValue;
    private final Method lookupInstance;
    private final Class<?> sampleHolderClz;
    private final Field samplHolderValueField;

    public ReflectionDataWriter(OsplServiceEnvironment environment,
            DDS.DataWriter writer, Class<TYPE> typeClz) {
        Class<?> typedWriterClz;
        String typedWriterClzName;

        this.old = writer;
        this.environment = environment;

        typedWriterClzName = typeClz.getName() + "DataWriterImpl";

        try {
            typedWriterClz = Class.forName(typedWriterClzName);

            this.sampleHolderClz = Class.forName(typeClz.getName() + "Holder");
            this.samplHolderValueField = this.sampleHolderClz
                    .getDeclaredField("value");

            this.registerInstance = typedWriterClz.getMethod(
                    "register_instance", typeClz);
            this.registerInstanceTimestamp = typedWriterClz.getMethod(
                    "register_instance_w_timestamp", typeClz, DDS.Time_t.class);
            this.unregisterInstance = typedWriterClz.getMethod(
                    "unregister_instance", typeClz, long.class);
            this.unregisterInstanceTimestamp = typedWriterClz.getMethod(
                    "unregister_instance_w_timestamp", typeClz, long.class,
                    DDS.Time_t.class);
            this.write = typedWriterClz.getMethod("write", typeClz, long.class);
            this.writeTimestamp = typedWriterClz.getMethod("write_w_timestamp",
                    typeClz, long.class, DDS.Time_t.class);
            this.dispose = typedWriterClz.getMethod("dispose", typeClz,
                    long.class);
            this.disposeTimestamp = typedWriterClz.getMethod(
                    "dispose_w_timestamp", typeClz, long.class,
                    DDS.Time_t.class);
            this.writeDispose = typedWriterClz.getMethod("writedispose",
                    typeClz, long.class);
            this.writeDisposeTimestamp = typedWriterClz.getMethod(
                    "writedispose_w_timestamp", typeClz, long.class,
                    DDS.Time_t.class);
            this.getKeyValue = typedWriterClz.getMethod("get_key_value",
                    this.sampleHolderClz, long.class);
            this.lookupInstance = typedWriterClz.getMethod("lookup_instance",
                    typeClz);

        } catch (ClassNotFoundException e) {
            throw new IllegalArgumentExceptionImpl(
                    this.environment,
                    "Cannot find Typed DataWriter '"
                            + typedWriterClzName
                            + "' that should be generated with OpenSplice idlpp");
        } catch (NoSuchMethodException e) {
            throw new IllegalArgumentExceptionImpl(
                    this.environment,
                    "Cannot find correct methods in '"
                            + typedWriterClzName
                            + "' that should be generated with OpenSplice idlpp ( "
                            + e.getMessage() + ").");
        } catch (NoSuchFieldException e) {
            throw new IllegalArgumentExceptionImpl(
                    this.environment,
                    "Cannot find 'value' field in "
                            + "the typed sampleHolderClass "
                            + "that should be generated with OpenSplice idlpp ( "
                            + e.getMessage() + ").");
        } catch (SecurityException e) {
            throw new IllegalArgumentExceptionImpl(
                    this.environment,
                    "Cannot find 'value' field in "
                            + "the typed sampleHolderClass "
                            + "that should be generated with OpenSplice idlpp ( "
                            + e.getMessage() + ").");
        }
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    public InstanceHandle registerInstance(TYPE instanceData)
            throws TimeoutException {
        long handle;

        if (instanceData == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal instanceData (null) provided.");
        }
        try {
            handle = (Long) this.registerInstance
                    .invoke(this.old, instanceData);
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.registerInstance() failed (" + e.getMessage()
                            + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.registerInstance() failed (" + e.getMessage()
                            + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.registerInstance() failed (" + e.getMessage()
                            + ").");
        }
        return Utilities.convert(this.environment, handle);
    }

    public InstanceHandle registerInstance(TYPE instanceData,
            Time sourceTimestamp) throws TimeoutException {
        long handle;

        if (instanceData == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal instanceData (null) provided.");
        }
        try {
            handle = (Long) this.registerInstanceTimestamp.invoke(this.old,
                    instanceData,
                    Utilities.convert(this.environment, sourceTimestamp));
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.registerInstance() failed (" + e.getMessage()
                            + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.registerInstance() failed (" + e.getMessage()
                            + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.registerInstance() failed (" + e.getMessage()
                            + ").");
        }
        return Utilities.convert(this.environment, handle);
    }

    public InstanceHandle registerInstance(TYPE instanceData,
            long sourceTimestamp, TimeUnit unit) throws TimeoutException {
        return this.registerInstance(instanceData, new TimeImpl(
                this.environment, sourceTimestamp, unit));
    }

    public void unregisterInstance(InstanceHandle handle)
            throws TimeoutException {
        this.unregisterInstance(handle, null);
    }

    public void unregisterInstance(InstanceHandle handle, TYPE instanceData)
            throws TimeoutException {
        try {
            int rc = (Integer) this.unregisterInstance.invoke(this.old,
                    instanceData, Utilities.convert(this.environment, handle));
            Utilities.checkReturnCodeWithTimeout(rc, this.environment,
                    "Datawriter.unregisterInstance() failed.");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.unregisterInstance() failed (" + e.getMessage()
                            + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.unregisterInstance() failed (" + e.getMessage()
                            + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.unregisterInstance() failed (" + e.getMessage()
                            + ").");
        }
    }

    public void unregisterInstance(InstanceHandle handle, TYPE instanceData,
            Time sourceTimestamp) throws TimeoutException {
        try {
            int rc = (Integer) this.unregisterInstanceTimestamp.invoke(
                    this.old, instanceData,
                    Utilities.convert(this.environment, handle),
                    Utilities.convert(this.environment, sourceTimestamp));
            Utilities.checkReturnCodeWithTimeout(rc, this.environment,
                    "Datawriter.unregisterInstance() failed.");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.unregisterInstance() failed (" + e.getMessage()
                            + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.unregisterInstance() failed (" + e.getMessage()
                            + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.unregisterInstance() failed (" + e.getMessage()
                            + ").");
        }
    }

    public void unregisterInstance(InstanceHandle handle, TYPE instanceData,
            long sourceTimestamp, TimeUnit unit) throws TimeoutException {
        this.unregisterInstance(handle, instanceData, new TimeImpl(
                this.environment, sourceTimestamp, unit));

    }

    public void write(TYPE instanceData) throws TimeoutException {
        this.write(instanceData, this.environment.getSPI().nilHandle());
    }

    public void write(TYPE instanceData, Time sourceTimestamp)
            throws TimeoutException {
        this.write(instanceData, this.environment.getSPI().nilHandle(),
                sourceTimestamp);
    }

    public void write(TYPE instanceData, long sourceTimestamp, TimeUnit unit)
            throws TimeoutException {
        this.write(instanceData, new TimeImpl(this.environment,
                sourceTimestamp, unit));

    }

    public void write(TYPE instanceData, InstanceHandle handle)
            throws TimeoutException {
        try {
            int rc = (Integer) this.write.invoke(this.old, instanceData,
                    Utilities.convert(this.environment, handle));
            Utilities.checkReturnCodeWithTimeout(rc, this.environment,
                    "DataWriter.write() failed.");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.write() failed (" + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.write() failed (" + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.write() failed (" + e.getMessage() + ").");
        }
    }

    public void write(TYPE instanceData, InstanceHandle handle,
            Time sourceTimestamp) throws TimeoutException {
        try {
            int rc = (Integer) this.writeTimestamp.invoke(this.old,
                    instanceData, Utilities.convert(this.environment, handle),
                    Utilities.convert(this.environment, sourceTimestamp));
            Utilities.checkReturnCodeWithTimeout(rc, this.environment,
                    "DataWriter.write() failed.");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.write() failed (" + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.write() failed (" + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.write() failed (" + e.getMessage() + ").");
        }
    }

    public void write(TYPE instanceData, InstanceHandle handle,
            long sourceTimestamp, TimeUnit unit) throws TimeoutException {
        this.write(instanceData, handle, new TimeImpl(this.environment,
                sourceTimestamp, unit));
    }

    public void dispose(InstanceHandle instanceHandle) throws TimeoutException {
        this.dispose(instanceHandle, null);
    }

    public void dispose(InstanceHandle instanceHandle, TYPE instanceData)
            throws TimeoutException {
        try {
            int rc = (Integer) this.dispose.invoke(this.old, instanceData,
                    Utilities.convert(this.environment, instanceHandle));
            Utilities.checkReturnCodeWithTimeout(rc, this.environment,
                    "DataWriter.dispose() failed.");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.dispose() failed (" + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.dispose() failed (" + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.dispose() failed (" + e.getMessage() + ").");
        }
    }

    public void dispose(InstanceHandle instanceHandle, TYPE instanceData,
            Time sourceTimestamp) throws TimeoutException {
        try {
            int rc = (Integer) this.disposeTimestamp.invoke(this.old,
                    instanceData,
                    Utilities.convert(this.environment, instanceHandle),
                    Utilities.convert(this.environment, sourceTimestamp));
            Utilities.checkReturnCodeWithTimeout(rc, this.environment,
                    "DataWriter.dispose() failed.");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.dispose() failed (" + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.dispose() failed (" + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.dispose() failed (" + e.getMessage() + ").");
        }

    }

    public void dispose(InstanceHandle instanceHandle, TYPE instanceData,
            long sourceTimestamp, TimeUnit unit) throws TimeoutException {
        this.dispose(instanceHandle, instanceData, new TimeImpl(
                this.environment, sourceTimestamp, unit));

    }

    @SuppressWarnings("unchecked")
    public TYPE getKeyValue(TYPE keyHolder, InstanceHandle handle) {
        Object holder;
        TYPE result;

        if (keyHolder == null) {
            return this.getKeyValue(handle);
        }

        try {
            holder = this.sampleHolderClz.newInstance();
            this.samplHolderValueField.set(holder, keyHolder);
            int rc = (Integer) this.getKeyValue.invoke(this.old, holder,
                    Utilities.convert(this.environment, handle));
            Utilities.checkReturnCode(rc, this.environment,
                    "DataWriter.getKeyValue() failed.");
            result = (TYPE) this.samplHolderValueField.get(holder);
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.getKeyValue() failed (" + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.getKeyValue() failed (" + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.getKeyValue() failed (" + e.getMessage() + ").");
        } catch (InstantiationException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.getKeyValue() failed (" + e.getMessage() + ").");
        }
        return result;
    }

    @SuppressWarnings("unchecked")
    public TYPE getKeyValue(InstanceHandle handle) {
        Object holder;
        TYPE result;

        try {
            holder = this.sampleHolderClz.newInstance();
            int rc = (Integer) this.getKeyValue.invoke(this.old, holder,
                    Utilities.convert(this.environment, handle));
            Utilities.checkReturnCode(rc, this.environment,
                    "DataWriter.getKeyValue() failed.");
            result = (TYPE) this.samplHolderValueField.get(holder);
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.getKeyValue() failed (" + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.getKeyValue() failed (" + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.getKeyValue() failed (" + e.getMessage() + ").");
        } catch (InstantiationException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.getKeyValue() failed (" + e.getMessage() + ").");
        }
        return result;
    }

    public InstanceHandle lookupInstance(TYPE keyHolder) {
        InstanceHandle handle;

        if (keyHolder == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal keyHolder (null) provided.");
        }
        try {
            long result = (Long) this.lookupInstance
                    .invoke(this.old, keyHolder);
            handle = Utilities.convert(this.environment, result);
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.lookupInstance() failed (" + e.getMessage()
                            + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.lookupInstance() failed (" + e.getMessage()
                            + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.lookupInstance() failed (" + e.getMessage()
                            + ").");
        }
        return handle;
    }

    public void writeDispose(TYPE instanceData) throws TimeoutException {
        this.writeDispose(instanceData, this.environment.getSPI().nilHandle());

    }

    public void writeDispose(TYPE instanceData, Time sourceTimestamp)
            throws TimeoutException {
        this.writeDispose(instanceData, this.environment.getSPI().nilHandle(),
                sourceTimestamp);
    }

    public void writeDispose(TYPE instanceData, long sourceTimestamp,
            TimeUnit unit) throws TimeoutException {
        this.writeDispose(instanceData, this.environment.getSPI().nilHandle(),
                new TimeImpl(this.environment, sourceTimestamp, unit));
    }

    public void writeDispose(TYPE instanceData, InstanceHandle handle)
            throws TimeoutException {
        try {
            int rc = (Integer) this.writeDispose.invoke(this.old, instanceData,
                    Utilities.convert(this.environment, handle));
            Utilities.checkReturnCodeWithTimeout(rc, this.environment,
                    "DataWriter.writeDispose() failed.");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.writeDispose() failed (" + e.getMessage()
                            + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.writeDispose() failed (" + e.getMessage()
                            + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.writeDispose() failed (" + e.getMessage()
                            + ").");
        }

    }

    public void writeDispose(TYPE instanceData, InstanceHandle handle,
            Time sourceTimestamp) throws TimeoutException {
        try {
            int rc = (Integer) this.writeDisposeTimestamp.invoke(this.old,
                    instanceData, Utilities.convert(this.environment, handle),
                    Utilities.convert(this.environment, sourceTimestamp));
            Utilities.checkReturnCodeWithTimeout(rc, this.environment,
                    "DataWriter.writeDispose() failed.");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.writeDispose() failed (" + e.getMessage()
                            + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.writeDispose() failed (" + e.getMessage()
                            + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(this.environment,
                    "DataWriter.writeDispose() failed (" + e.getMessage()
                            + ").");
        }
    }

    public void writeDispose(TYPE instanceData, InstanceHandle handle,
            long sourceTimestamp, TimeUnit unit) throws TimeoutException {
        this.writeDispose(instanceData, handle, new TimeImpl(this.environment,
                sourceTimestamp, unit));

    }

    public void assertLiveliness() {
        int rc = this.old.assert_liveliness();
        Utilities.checkReturnCode(rc, this.environment,
                "DataWriter.assertLiveliness() failed.");
    }

    public Set<InstanceHandle> getMatchedSubscriptions() {
        DDS.InstanceHandleSeqHolder holder = new DDS.InstanceHandleSeqHolder();
        Set<InstanceHandle> handles;

        int rc = this.old.get_matched_subscriptions(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "DataWriter.getMatchedSubscriptions() failed.");

        handles = new HashSet<InstanceHandle>();

        for (long handle : holder.value) {
            handles.add(Utilities.convert(this.environment, handle));
        }
        return handles;
    }

    public SubscriptionBuiltinTopicData getMatchedSubscriptionData(
            InstanceHandle subscriptionHandle) {
        DDS.SubscriptionBuiltinTopicDataHolder holder = new DDS.SubscriptionBuiltinTopicDataHolder();
        int rc = this.old.get_matched_subscription_data(holder,
                Utilities.convert(this.environment, subscriptionHandle));
        Utilities.checkReturnCode(rc, this.environment,
                "DataWriter.getMatchedSubscriptionData() failed.");
        if (holder.value != null) {
            return new SubscriptionBuiltinTopicDataImpl(this.environment,
                    holder.value);
        }
        throw new PreconditionNotMetExceptionImpl(this.environment,
                    "No data for this instanceHandle.");
    }

    public DataWriterQos getQos() {
        DDS.DataWriterQosHolder holder = new DDS.DataWriterQosHolder();
        int rc = this.old.get_qos(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "DataWriter.getQos() failed.");

        return DataWriterQosImpl.convert(this.environment, holder.value);
    }

    public void setQos(DataWriterQos qos) {
        DataWriterQosImpl q;

        if (qos == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied DataWriterQos is null.");
        }
        try {
            q = (DataWriterQosImpl) qos;
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Setting non-OpenSplice Qos not supported.");
        }
        int rc = this.old.set_qos(q.convert());
        Utilities.checkReturnCode(rc, this.environment,
                "DataWriter.setQos() failed.");
    }

    public void waitForAcknowledgments(Duration maxWait)
            throws TimeoutException {
        int rc = this.old.wait_for_acknowledgments(Utilities.convert(this.environment,
                maxWait));
        Utilities.checkReturnCodeWithTimeout(rc, this.environment,
                "DataWriter.waitForAcknowledgments() failed.");
    }

    public void waitForAcknowledgments(long maxWait, TimeUnit unit)
            throws TimeoutException {
        this.waitForAcknowledgments(this.environment.getSPI().newDuration(
                maxWait, unit));
    }

    public LivelinessLostStatus getLivelinessLostStatus() {
        DDS.LivelinessLostStatusHolder holder = new DDS.LivelinessLostStatusHolder();

        int rc = this.old.get_liveliness_lost_status(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "DataWriter.getLivelinessLostStatus() failed.");

        return StatusConverter.convert(this.environment, holder.value);
    }

    public OfferedDeadlineMissedStatus getOfferedDeadlineMissedStatus() {
        DDS.OfferedDeadlineMissedStatusHolder holder = new DDS.OfferedDeadlineMissedStatusHolder();

        int rc = this.old.get_offered_deadline_missed_status(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "DataWriter.getOfferedDeadlineMissedStatus() failed.");

        return StatusConverter.convert(this.environment, holder.value);
    }

    public OfferedIncompatibleQosStatus getOfferedIncompatibleQosStatus() {
        DDS.OfferedIncompatibleQosStatusHolder holder = new DDS.OfferedIncompatibleQosStatusHolder();

        int rc = this.old.get_offered_incompatible_qos_status(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "DataWriter.getOfferedIncompatibleQosStatus() failed.");

        return StatusConverter.convert(this.environment, holder.value);
    }

    public PublicationMatchedStatus getPublicationMatchedStatus() {
        DDS.PublicationMatchedStatusHolder holder = new DDS.PublicationMatchedStatusHolder();

        int rc = this.old.get_publication_matched_status(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "DataWriter.getPublicationMatchedStatus() failed.");

        return StatusConverter.convert(this.environment, holder.value);
    }
}
