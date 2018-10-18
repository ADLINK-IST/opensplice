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
package org.opensplice.dds.sub;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.eclipse.cyclone.dds.core.AbstractDDSObject;
import org.omg.dds.core.DDSObject;
import org.omg.dds.core.Duration;
import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.Time;
import org.omg.dds.core.policy.ResourceLimits;
import org.omg.dds.core.status.LivelinessChangedStatus;
import org.omg.dds.core.status.RequestedDeadlineMissedStatus;
import org.omg.dds.core.status.RequestedIncompatibleQosStatus;
import org.omg.dds.core.status.SampleLostStatus;
import org.omg.dds.core.status.SampleRejectedStatus;
import org.omg.dds.core.status.SubscriptionMatchedStatus;
import org.omg.dds.sub.DataReaderQos;
import org.omg.dds.sub.ReadCondition;
import org.omg.dds.sub.Sample;
import org.omg.dds.sub.Sample.Iterator;
import org.omg.dds.topic.PublicationBuiltinTopicData;
import org.opensplice.dds.core.DDSExceptionImpl;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.InstanceHandleImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.PreconditionNotMetExceptionImpl;
import org.opensplice.dds.core.Utilities;
import org.opensplice.dds.core.policy.PolicyConverter;
import org.opensplice.dds.core.status.StatusConverter;
import org.opensplice.dds.topic.PublicationBuiltinTopicDataImpl;

import DDS.SampleInfoHolder;
import DDS.SampleInfoSeqHolder;

public class ReflectionDataReader<TYPE, OUT_TYPE> extends AbstractDDSObject
        implements
        DDSObject {
    private final OsplServiceEnvironment environment;
    private final AbstractDataReader<OUT_TYPE> reader;
    private final DDS.DataReader old;
    private final Class<?> sampleSeqHolderClz;
    private final Field sampleSeqHolderValueField;

    private final Class<?> sampleHolderClz;
    private final Field sampleHolderValueField;

    private final Method read;
    private final Method take;
    private final Method readCondition;
    private final Method takeCondition;
    private final Method readNextSample;
    private final Method takeNextSample;
    private final Method readInstance;
    private final Method takeInstance;
    private final Method readNextInstance;
    private final Method takeNextInstance;
    private final Method readNextInstanceCondition;
    private final Method takeNextInstanceCondition;
    private final Method returnLoan;
    private final Method getKeyValue;
    private final Method lookupInstance;

    public ReflectionDataReader(OsplServiceEnvironment environment,
            AbstractDataReader<OUT_TYPE> reader, Class<TYPE> ddsTypeClz) {
        this.environment = environment;
        this.reader = reader;
        this.old = reader.getOld();

        Class<?> typedReaderClz;
        String typedReaderClzName = ddsTypeClz.getName() + "DataReaderImpl";

        try {
            typedReaderClz = Class.forName(typedReaderClzName);

            this.sampleHolderClz = Class.forName(ddsTypeClz.getName() + "Holder");
            this.sampleHolderValueField = this.sampleHolderClz
                    .getDeclaredField("value");

            this.sampleSeqHolderClz = Class.forName(ddsTypeClz.getName()
                    + "SeqHolder");
            this.sampleSeqHolderValueField = this.sampleSeqHolderClz
                    .getDeclaredField("value");

            this.read = typedReaderClz.getMethod("read",
                    this.sampleSeqHolderClz, SampleInfoSeqHolder.class,
                    int.class, int.class, int.class, int.class);
            this.take = typedReaderClz.getMethod("take",
                    this.sampleSeqHolderClz, SampleInfoSeqHolder.class,
                    int.class, int.class, int.class, int.class);

            this.readCondition = typedReaderClz.getMethod("read_w_condition",
                    this.sampleSeqHolderClz, SampleInfoSeqHolder.class,
                    int.class, DDS.ReadCondition.class);
            this.takeCondition = typedReaderClz.getMethod("take_w_condition",
                    this.sampleSeqHolderClz, SampleInfoSeqHolder.class,
                    int.class, DDS.ReadCondition.class);

            this.readNextSample = typedReaderClz.getMethod("read_next_sample",
                    this.sampleHolderClz, SampleInfoHolder.class);
            this.takeNextSample = typedReaderClz.getMethod("take_next_sample",
                    this.sampleHolderClz, SampleInfoHolder.class);

            this.readInstance = typedReaderClz.getMethod("read_instance",
                    this.sampleSeqHolderClz, SampleInfoSeqHolder.class,
                    int.class, long.class, int.class, int.class, int.class);
            this.takeInstance = typedReaderClz.getMethod("take_instance",
                    this.sampleSeqHolderClz, SampleInfoSeqHolder.class,
                    int.class, long.class, int.class, int.class, int.class);

            this.readNextInstance = typedReaderClz.getMethod(
                    "read_next_instance", this.sampleSeqHolderClz,
                    SampleInfoSeqHolder.class, int.class, long.class,
                    int.class, int.class, int.class);
            this.takeNextInstance = typedReaderClz.getMethod(
                    "take_next_instance", this.sampleSeqHolderClz,
                    SampleInfoSeqHolder.class, int.class, long.class,
                    int.class, int.class, int.class);

            this.readNextInstanceCondition = typedReaderClz.getMethod(
                    "read_next_instance_w_condition", this.sampleSeqHolderClz,
                    SampleInfoSeqHolder.class, int.class, long.class,
                    DDS.ReadCondition.class);
            this.takeNextInstanceCondition = typedReaderClz.getMethod(
                    "take_next_instance_w_condition", this.sampleSeqHolderClz,
                    SampleInfoSeqHolder.class, int.class, long.class,
                    DDS.ReadCondition.class);

            this.returnLoan = typedReaderClz.getMethod("return_loan",
                    this.sampleSeqHolderClz, SampleInfoSeqHolder.class);

            this.getKeyValue = typedReaderClz.getMethod("get_key_value",
                    this.sampleHolderClz, long.class);
            this.lookupInstance = typedReaderClz.getMethod("lookup_instance",
                    ddsTypeClz);
        } catch (ClassNotFoundException e) {
            throw new PreconditionNotMetExceptionImpl(
                    environment,
                    "Cannot find Typed DataReader '"
                            + typedReaderClzName
                            + "' that should have been generated manually with the OpenSplice IDL pre-processor.("
                            + e.getMessage() + ").");
        } catch (NoSuchMethodException e) {
            throw new DDSExceptionImpl(environment,
                    "Cannot find correct methods in OpenSplice IDL pre-processor generated class: "
                            + typedReaderClzName + " (" + e.getMessage() + ").");
        } catch (NoSuchFieldException e) {
            throw new DDSExceptionImpl(
                    environment,
                    "Cannot find 'value' field in "
                            + "the typed sampleHolderClass "
                            + "that should have been generated by the OpenSplice IDL pre-processor ("
                            + e.getMessage() + ").");
        } catch (SecurityException e) {
            throw new PreconditionNotMetExceptionImpl(
                    environment,
                    "Insufficient rights to find methods/fields in code that has been generated by the OpenSplice IDL pre-processor ("
                            + e.getMessage() + ").");
        }
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    public DataReaderQos getQos() {
        DDS.DataReaderQosHolder holder = new DDS.DataReaderQosHolder();
        int rc = this.old.get_qos(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "DataReader.getQos() failed.");

        return DataReaderQosImpl.convert(this.environment, holder.value);
    }

    public void setQos(DataReaderQos qos) {
        DataReaderQosImpl q;

        if (qos == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied DataReaderQos is null.");
        }
        try {
            q = (DataReaderQosImpl) qos;
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Setting non-OpenSplice Qos not supported.");
        }
        int rc = this.old.set_qos(q.convert());
        Utilities.checkReturnCode(rc, this.environment,
                "DataReader.setQos() failed.");
    }

    public SampleRejectedStatus getSampleRejectedStatus() {
        DDS.SampleRejectedStatusHolder holder = new DDS.SampleRejectedStatusHolder();

        int rc = this.old.get_sample_rejected_status(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "DataReader.getSampleRejectedStatus() failed.");

        return StatusConverter.convert(this.environment, holder.value);
    }

    public LivelinessChangedStatus getLivelinessChangedStatus() {
        DDS.LivelinessChangedStatusHolder holder = new DDS.LivelinessChangedStatusHolder();

        int rc = this.old.get_liveliness_changed_status(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "DataReader.getLivelinessChangedStatus() failed.");

        return StatusConverter.convert(this.environment, holder.value);
    }

    public RequestedDeadlineMissedStatus getRequestedDeadlineMissedStatus() {
        DDS.RequestedDeadlineMissedStatusHolder holder = new DDS.RequestedDeadlineMissedStatusHolder();

        int rc = this.old.get_requested_deadline_missed_status(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "DataReader.getRequestedDeadlineMissedStatus() failed.");

        return StatusConverter.convert(this.environment, holder.value);
    }

    public RequestedIncompatibleQosStatus getRequestedIncompatibleQosStatus() {
        DDS.RequestedIncompatibleQosStatusHolder holder = new DDS.RequestedIncompatibleQosStatusHolder();

        int rc = this.old.get_requested_incompatible_qos_status(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "DataReader.getRequestedIncompatibleQosStatus() failed.");

        return StatusConverter.convert(this.environment, holder.value);
    }

    public SubscriptionMatchedStatus getSubscriptionMatchedStatus() {
        DDS.SubscriptionMatchedStatusHolder holder = new DDS.SubscriptionMatchedStatusHolder();

        int rc = this.old.get_subscription_matched_status(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "DataReader.getSubscriptionMatchedStatus() failed.");

        return StatusConverter.convert(this.environment, holder.value);
    }

    public SampleLostStatus getSampleLostStatus() {
        DDS.SampleLostStatusHolder holder = new DDS.SampleLostStatusHolder();

        int rc = this.old.get_sample_lost_status(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "DataReader.getSampleLostStatus() failed.");

        return StatusConverter.convert(this.environment, holder.value);
    }

    public void waitForHistoricalData(Duration maxWait) throws TimeoutException {
        int rc = this.old.wait_for_historical_data(Utilities.convert(this.environment,
                maxWait));
        Utilities.checkReturnCodeWithTimeout(rc, this.environment,
                "DataReader.waitForHistoricalData() failed.");

    }

    public void waitForHistoricalData(long maxWait, TimeUnit unit)
            throws TimeoutException {
        this.waitForHistoricalData(this.environment.getSPI().newDuration(
                maxWait, unit));
    }

    public void waitForHistoricalData(String filterExpression,
            List<String> filterParameters, Time minSourceTimestamp,
            Time maxSourceTimestamp, ResourceLimits resourceLimits,
            Duration maxWait) throws TimeoutException {
        if (resourceLimits == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Invalid resourceLimits (null) supplied.");
        }
        String[] params;

        if(filterParameters != null){
            params = filterParameters.toArray(new String[filterParameters
                    .size()]);
        } else {
            params = null;
        }
        int rc = this.old.wait_for_historical_data_w_condition(
                filterExpression, params,
                Utilities.convert(this.environment, minSourceTimestamp),
                Utilities.convert(this.environment, maxSourceTimestamp),
                PolicyConverter.convert(this.environment, resourceLimits),
                Utilities.convert(this.environment, maxWait));
        Utilities.checkReturnCodeWithTimeout(rc, this.environment,
                "DataReader.waitForHistoricalData() failed.");
    }

    public void waitForHistoricalData(String filterExpression,
            List<String> filterParameters, Time minSourceTimestamp,
            Time maxSourceTimestamp, ResourceLimits resourceLimits,
            long maxWait, TimeUnit unit) throws TimeoutException {
        this.waitForHistoricalData(filterExpression, filterParameters,
                minSourceTimestamp, maxSourceTimestamp, resourceLimits,
                this.environment.getSPI().newDuration(maxWait, unit));
    }

    public Set<InstanceHandle> getMatchedPublications() {
        DDS.InstanceHandleSeqHolder holder = new DDS.InstanceHandleSeqHolder();
        Set<InstanceHandle> handles;

        int rc = this.old.get_matched_publications(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "DataReader.getMatchedPublications() failed.");

        handles = new HashSet<InstanceHandle>();

        for (long handle : holder.value) {
            handles.add(Utilities.convert(this.environment, handle));
        }
        return handles;
    }

    public PublicationBuiltinTopicData getMatchedPublicationData(
            InstanceHandle publicationHandle) {
        DDS.PublicationBuiltinTopicDataHolder holder = new DDS.PublicationBuiltinTopicDataHolder();
        int rc = this.old.get_matched_publication_data(holder,
                Utilities.convert(this.environment, publicationHandle));
        Utilities.checkReturnCode(rc, this.environment,
                "DataReader.getMatchedPublicationData() failed.");
        if (holder.value != null) {
            return new PublicationBuiltinTopicDataImpl(this.environment,
                    holder.value);
        }
        throw new PreconditionNotMetExceptionImpl(this.environment,
                    "No data for this instanceHandle.");
    }

    public TYPE getKeyValue(TYPE keyHolder, InstanceHandle handle) {
        Object sampleHolder;

        if (keyHolder == null) {
            throw new IllegalArgumentException(
                    "Invalid key holder (null) provided.");
        }

        try {
            sampleHolder = this.sampleHolderClz.newInstance();
            this.sampleHolderValueField.set(sampleHolder, keyHolder);
            int rc = (Integer) this.getKeyValue.invoke(this.old, sampleHolder,
                    Utilities.convert(this.environment, handle));
            Utilities.checkReturnCode(rc, this.environment,
                    "DataReader.getKeyValue() failed.");
        } catch (InstantiationException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (ClassCastException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        }
        return keyHolder;
    }

    @SuppressWarnings("unchecked")
    public TYPE getKeyValue(InstanceHandle handle) {
        Object sampleHolder;


        try {
            sampleHolder = this.sampleHolderClz.newInstance();
            int rc = (Integer) this.getKeyValue.invoke(this.old, sampleHolder,
                    Utilities.convert(this.environment, handle));
            Utilities.checkReturnCode(rc, this.environment,
                    "DataReader.getKeyValue() failed.");

            return (TYPE) this.sampleHolderValueField.get(sampleHolder);
        } catch (InstantiationException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (ClassCastException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        }
    }

    public InstanceHandle lookupInstance(TYPE keyHolder) {
        long oldHandle;

        if (keyHolder == null) {
            throw new IllegalArgumentException(
                    "Invalid key holder (null) provided.");
        }

        try {
            oldHandle = (Long) this.lookupInstance.invoke(this.old, keyHolder);
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (ClassCastException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        }
        return Utilities.convert(this.environment, oldHandle);
    }

    public void returnLoan(Object sampleSeqHolder,
            DDS.SampleInfoSeqHolder infoSeqHolder) {
        try {
            int rc = (Integer) this.returnLoan.invoke(this.old,
                    sampleSeqHolder, infoSeqHolder);
            Utilities.checkReturnCode(rc, this.environment,
                    "Return loan failed.");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        }
    }

    public Iterator<OUT_TYPE> read() {
        return this.read(DDS.LENGTH_UNLIMITED.value);
    }

    @SuppressWarnings("unchecked")
    public Iterator<OUT_TYPE> read(
            org.omg.dds.sub.DataReader.Selector<OUT_TYPE> query) {
        DDS.SampleInfoSeqHolder info = new DDS.SampleInfoSeqHolder();
        Object sampleSeqHolder;

        if (query == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Invalid Selector (null) provided.");
        }
        ReadCondition<OUT_TYPE> condition = null;

        if (query.getQueryExpression() != null) {
            condition = query.getCondition();
        }
        InstanceHandle instance = query.getInstance();

        try {
            DataStateImpl state = (DataStateImpl) query.getDataState();
            InstanceHandleImpl handle = (InstanceHandleImpl) instance;
            sampleSeqHolder = this.sampleSeqHolderClz.newInstance();
            int rc;

            if (condition != null) {
                DDS.ReadCondition oldCondition = ((ReadConditionImpl<OUT_TYPE>) condition)
                        .getOld();

                if (query.retrieveNextInstance()) {
                    // read_next_instance_w_condition
                    rc = (Integer) this.readNextInstanceCondition.invoke(
                            this.old, sampleSeqHolder, info,
                            query.getMaxSamples(), handle.getValue(),
                            oldCondition);
                } else {
                    // read_w_condition
                    rc = (Integer) this.readCondition.invoke(this.old,
                            sampleSeqHolder, info, query.getMaxSamples(),
                            oldCondition);
                }
            } else {
                // read_next_instance
                if (query.retrieveNextInstance()) {
                    rc = (Integer) this.readNextInstance.invoke(this.old,
                            sampleSeqHolder, info, query.getMaxSamples(),
                            handle.getValue(), state.getOldSampleState(),
                            state.getOldViewState(),
                            state.getOldInstanceState());
                } else {
                    // read
                    if (instance.isNil()) {
                        rc = (Integer) this.read.invoke(this.old,
                                sampleSeqHolder, info, query.getMaxSamples(),
                                state.getOldSampleState(),
                                state.getOldViewState(),
                                state.getOldInstanceState());
                    }
                    // read_instance
                    else {
                        rc = (Integer) this.readInstance.invoke(this.old,
                                sampleSeqHolder, info, query.getMaxSamples(),
                                handle.getValue(), state.getOldSampleState(),
                                state.getOldViewState(),
                                state.getOldInstanceState());
                    }
                }
            }
            Utilities.checkReturnCode(rc, this.environment,
                    "DataReader.read() failed.");
        } catch (InstantiationException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(
                    this.environment,
                    "Reading with non-OpenSplice DataState, InstanceHandle or ReadCondition not supported");
        }
        return (Iterator<OUT_TYPE>) this.reader.createIterator(
                sampleSeqHolder, this.sampleSeqHolderValueField, info);
    }

    @SuppressWarnings("unchecked")
    public Iterator<OUT_TYPE> read(int maxSamples) {
        DDS.SampleInfoSeqHolder info = new DDS.SampleInfoSeqHolder();
        Object sampleSeqHolder;

        try {
            sampleSeqHolder = this.sampleSeqHolderClz.newInstance();
            int rc = (Integer) this.read.invoke(this.old, sampleSeqHolder,
                    info, maxSamples, DDS.ANY_SAMPLE_STATE.value,
                    DDS.ANY_VIEW_STATE.value, DDS.ANY_INSTANCE_STATE.value);
            Utilities.checkReturnCode(rc, this.environment,
                    "DataReader.read() failed.");
        } catch (InstantiationException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        }
        return (Iterator<OUT_TYPE>) this.reader.createIterator(
                sampleSeqHolder, this.sampleSeqHolderValueField, info);
    }

    public Iterator<OUT_TYPE> take() {
        return this.take(DDS.LENGTH_UNLIMITED.value);
    }

    @SuppressWarnings("unchecked")
    public Iterator<OUT_TYPE> take(int maxSamples) {
        DDS.SampleInfoSeqHolder info = new DDS.SampleInfoSeqHolder();
        Object sampleSeqHolder;

        try {
            sampleSeqHolder = this.sampleSeqHolderClz.newInstance();
            int rc = (Integer) this.take.invoke(this.old, sampleSeqHolder,
                    info, maxSamples, DDS.ANY_SAMPLE_STATE.value,
                    DDS.ANY_VIEW_STATE.value, DDS.ANY_INSTANCE_STATE.value);
            Utilities.checkReturnCode(rc, this.environment,
                    "DataReader.take() failed.");
        } catch (InstantiationException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        }
        return (Iterator<OUT_TYPE>) this.reader.createIterator(
                sampleSeqHolder, this.sampleSeqHolderValueField, info);
    }

    public Field getSampleSeqHolderValueField() {
        return this.sampleSeqHolderValueField;
    }

    @SuppressWarnings("unchecked")
    public Iterator<OUT_TYPE> take(
            org.omg.dds.sub.DataReader.Selector<OUT_TYPE> query) {
        DDS.SampleInfoSeqHolder info = new DDS.SampleInfoSeqHolder();
        Object sampleSeqHolder;

        if (query == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Invalid Selector (null) provided.");
        }
        ReadCondition<OUT_TYPE> condition = null;

        if (query.getQueryExpression() != null) {
            condition = query.getCondition();
        }
        InstanceHandle instance = query.getInstance();

        try {
            DataStateImpl state = (DataStateImpl) query.getDataState();
            InstanceHandleImpl handle = (InstanceHandleImpl) instance;
            sampleSeqHolder = this.sampleSeqHolderClz.newInstance();
            int rc;

            if (condition != null) {
                DDS.ReadCondition oldCondition = ((ReadConditionImpl<OUT_TYPE>) condition)
                        .getOld();

                if (query.retrieveNextInstance()) {
                    // take_next_instance_w_condition
                    rc = (Integer) this.takeNextInstanceCondition.invoke(
                            this.old, sampleSeqHolder, info,
                            query.getMaxSamples(), handle.getValue(),
                            oldCondition);
                } else {
                    // take_w_condition
                    rc = (Integer) this.takeCondition.invoke(this.old,
                            sampleSeqHolder, info, query.getMaxSamples(),
                            oldCondition);
                }
            } else {
                // take_next_instance
                if (query.retrieveNextInstance()) {
                    rc = (Integer) this.takeNextInstance.invoke(this.old,
                            sampleSeqHolder, info, query.getMaxSamples(),
                            handle.getValue(), state.getOldSampleState(),
                            state.getOldViewState(),
                            state.getOldInstanceState());
                } else {
                    // take
                    if (instance.isNil()) {
                        rc = (Integer) this.take.invoke(this.old,
                                sampleSeqHolder, info, query.getMaxSamples(),
                                state.getOldSampleState(),
                                state.getOldViewState(),
                                state.getOldInstanceState());
                    }
                    // take_instance
                    else {
                        rc = (Integer) this.takeInstance.invoke(this.old,
                                sampleSeqHolder, info, query.getMaxSamples(),
                                handle.getValue(), state.getOldSampleState(),
                                state.getOldViewState(),
                                state.getOldInstanceState());
                    }
                }
            }
            Utilities.checkReturnCode(rc, this.environment,
                    "DataReader.take() failed.");
        } catch (InstantiationException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(
                    this.environment,
                    "Taking with non-OpenSplice DataState, InstanceHandle or ReadCondition not supported");
        }
        return (Iterator<OUT_TYPE>) this.reader.createIterator(
                sampleSeqHolder, this.sampleSeqHolderValueField, info);
    }

    @SuppressWarnings("unchecked")
    public boolean readNextSample(SampleImpl<TYPE> sample) {
        DDS.SampleInfoHolder info;
        Object sampleHolder;
        boolean result;

        if (sample == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Provided an invalid null sample.");
        }
        try {
            info = new DDS.SampleInfoHolder();
            sampleHolder = this.sampleHolderClz.newInstance();

            this.sampleHolderValueField.set(sampleHolder, sample.getData());
            int rc = (Integer) this.readNextSample.invoke(this.old,
                    sampleHolder, info);
            Utilities.checkReturnCode(rc, this.environment,
                    "DataReader.readNextSample() failed.");

            if (rc == DDS.RETCODE_OK.value) {
                sample.setContent(
                        (TYPE) this.sampleHolderValueField.get(sampleHolder),
                        info.value);
                result = true;
            } else {
                result = false;
            }

        } catch (InstantiationException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (ClassCastException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        }
        return result;
    }

    @SuppressWarnings("unchecked")
    public boolean takeNextSample(SampleImpl<TYPE> sample) {
        DDS.SampleInfoHolder info;
        Object sampleHolder;
        boolean result;

        if (sample == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Provided an invalid null sample.");
        }
        try {
            info = new DDS.SampleInfoHolder();
            sampleHolder = this.sampleHolderClz.newInstance();
            this.sampleHolderValueField.set(sampleHolder, sample.getData());
            int rc = (Integer) this.takeNextSample.invoke(this.old,
                    sampleHolder, info);
            Utilities.checkReturnCode(rc, this.environment,
                    "DataReader.readNextSample() failed.");

            if (rc == DDS.RETCODE_OK.value) {
                sample.setContent(
                        (TYPE) this.sampleHolderValueField.get(sampleHolder),
                        info.value);
                result = true;
            } else {
                result = false;
            }

        } catch (InstantiationException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (ClassCastException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        }
        return result;
    }

    public List<Sample<OUT_TYPE>> take(List<Sample<OUT_TYPE>> samples) {
        PreAllocator<OUT_TYPE> pa = this.reader.getPreAllocator(samples,
                this.sampleSeqHolderClz, this.sampleSeqHolderValueField);

        try {
            int rc = (Integer) this.take.invoke(this.old,
                    pa.getDataSeqHolder(), pa.getInfoSeqHolder(),
                    DDS.LENGTH_UNLIMITED.value, DDS.ANY_SAMPLE_STATE.value,
                    DDS.ANY_VIEW_STATE.value, DDS.ANY_INSTANCE_STATE.value);
            Utilities.checkReturnCode(rc, this.environment,
                    "DataReader.read() failed.");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        }
        pa.updateReferences();

        return pa.getSampleList();
    }

    public List<Sample<OUT_TYPE>> take(List<Sample<OUT_TYPE>> samples,
            org.omg.dds.sub.DataReader.Selector<OUT_TYPE> selector) {
        if (selector == null) {
            return this.take(samples);
        }

        PreAllocator<OUT_TYPE> pa = this.reader.getPreAllocator(samples,
                this.sampleSeqHolderClz, this.sampleSeqHolderValueField);
        ReadCondition<OUT_TYPE> condition = null;

        if (selector.getQueryExpression() != null) {
            condition = selector.getCondition();
        }
        InstanceHandle instance = selector.getInstance();

        try {
            DataStateImpl state = (DataStateImpl) selector.getDataState();
            InstanceHandleImpl handle = (InstanceHandleImpl) instance;
            int rc;

            if (condition != null) {
                DDS.ReadCondition oldCondition = ((ReadConditionImpl<OUT_TYPE>) condition)
                        .getOld();

                if (selector.retrieveNextInstance()) {
                    // take_next_instance_w_condition
                    rc = (Integer) this.takeNextInstanceCondition.invoke(
                            this.old, pa.getDataSeqHolder(),
                            pa.getInfoSeqHolder(), selector.getMaxSamples(),
                            handle.getValue(), oldCondition);
                } else {
                    // take_w_condition
                    rc = (Integer) this.takeCondition.invoke(this.old,
                            pa.getDataSeqHolder(), pa.getInfoSeqHolder(),
                            selector.getMaxSamples(), oldCondition);
                }
            } else {
                // take_next_instance
                if (selector.retrieveNextInstance()) {
                    rc = (Integer) this.takeNextInstance.invoke(this.old,
                            pa.getDataSeqHolder(), pa.getInfoSeqHolder(),
                            selector.getMaxSamples(), handle.getValue(),
                            state.getOldSampleState(), state.getOldViewState(),
                            state.getOldInstanceState());
                } else {
                    // take
                    if (instance.isNil()) {
                        rc = (Integer) this.take.invoke(this.old,
                                pa.getDataSeqHolder(), pa.getInfoSeqHolder(),
                                selector.getMaxSamples(),
                                state.getOldSampleState(),
                                state.getOldViewState(),
                                state.getOldInstanceState());
                    }
                    // take_instance
                    else {
                        rc = (Integer) this.takeInstance.invoke(this.old,
                                pa.getDataSeqHolder(), pa.getInfoSeqHolder(),
                                selector.getMaxSamples(), handle.getValue(),
                                state.getOldSampleState(),
                                state.getOldViewState(),
                                state.getOldInstanceState());
                    }
                }
            }
            Utilities.checkReturnCode(rc, this.environment,
                    "DataReader.take() failed.");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(
                    this.environment,
                    "Reading with non-OpenSplice DataState, InstanceHandle or ReadCondition not supported");
        }
        pa.updateReferences();

        return pa.getSampleList();
    }

    public List<Sample<OUT_TYPE>> read(List<Sample<OUT_TYPE>> samples,
            org.omg.dds.sub.DataReader.Selector<OUT_TYPE> selector) {

        if (selector == null) {
            return this.read(samples);
        }

        PreAllocator<OUT_TYPE> pa = this.reader.getPreAllocator(samples,
                this.sampleSeqHolderClz, this.sampleSeqHolderValueField);

        ReadCondition<OUT_TYPE> condition = null;

        if (selector.getQueryExpression() != null) {
            condition = selector.getCondition();
        }
        InstanceHandle instance = selector.getInstance();

        try {
            DataStateImpl state = (DataStateImpl) selector.getDataState();
            InstanceHandleImpl handle = (InstanceHandleImpl) instance;
            int rc;

            if (condition != null) {
                DDS.ReadCondition oldCondition = ((ReadConditionImpl<OUT_TYPE>) condition)
                        .getOld();

                if (selector.retrieveNextInstance()) {
                    // read_next_instance_w_condition
                    rc = (Integer) this.readNextInstanceCondition.invoke(
                            this.old, pa.getDataSeqHolder(),
                            pa.getInfoSeqHolder(), selector.getMaxSamples(),
                            handle.getValue(), oldCondition);
                } else {
                    // read_w_condition
                    rc = (Integer) this.readCondition.invoke(this.old,
                            pa.getDataSeqHolder(), pa.getInfoSeqHolder(),
                            selector.getMaxSamples(), oldCondition);
                }
            } else {
                // read_next_instance
                if (selector.retrieveNextInstance()) {
                    rc = (Integer) this.readNextInstance.invoke(this.old,
                            pa.getDataSeqHolder(), pa.getInfoSeqHolder(),
                            selector.getMaxSamples(), handle.getValue(),
                            state.getOldSampleState(), state.getOldViewState(),
                            state.getOldInstanceState());
                } else {
                    // read
                    if (instance.isNil()) {
                        rc = (Integer) this.read.invoke(this.old,
                                pa.getDataSeqHolder(), pa.getInfoSeqHolder(),
                                selector.getMaxSamples(),
                                state.getOldSampleState(),
                                state.getOldViewState(),
                                state.getOldInstanceState());
                    }
                    // read_instance
                    else {
                        rc = (Integer) this.readInstance.invoke(this.old,
                                pa.getDataSeqHolder(), pa.getInfoSeqHolder(),
                                selector.getMaxSamples(), handle.getValue(),
                                state.getOldSampleState(),
                                state.getOldViewState(),
                                state.getOldInstanceState());
                    }
                }
            }
            Utilities.checkReturnCode(rc, this.environment,
                    "DataReader.read() failed.");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(
                    this.environment,
                    "Reading with non-OpenSplice DataState, InstanceHandle or ReadCondition not supported");
        }
        pa.updateReferences();

        return pa.getSampleList();
    }

    public List<Sample<OUT_TYPE>> read(List<Sample<OUT_TYPE>> samples) {
        PreAllocator<OUT_TYPE> pa = this.reader.getPreAllocator(samples,
                this.sampleSeqHolderClz, this.sampleSeqHolderValueField);

        try {
            int rc = (Integer) this.read.invoke(this.old,
                    pa.getDataSeqHolder(), pa.getInfoSeqHolder(),
                    DDS.LENGTH_UNLIMITED.value, DDS.ANY_SAMPLE_STATE.value,
                    DDS.ANY_VIEW_STATE.value, DDS.ANY_INSTANCE_STATE.value);
            Utilities.checkReturnCode(rc, this.environment,
                    "DataReader.read() failed.");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalArgumentExceptionImpl e) {
            throw e;
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (InvocationTargetException e) {
            throw new DDSExceptionImpl(environment, "Internal error ("
                    + e.getMessage() + ").");
        }
        pa.updateReferences();

        return pa.getSampleList();
    }
}
